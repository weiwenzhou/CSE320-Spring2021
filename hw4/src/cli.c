/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"
#include "custom_functions.h"
#include "debug.h"

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    // int stdin_copy = dup(STDIN_FILENO);
    // int stdout_copy = dup(STDOUT_FILENO);
    // int fd_in = fileno(in);
    // int fd_out = fileno(out);
    // dup2(fd_in, STDIN_FILENO);
    // dup2(fd_out, STDOUT_FILENO);
    info("%p", jobs[0].type);
    int returnValue = 0;
    char *prompt = (in == stdin) ? "imp> ":"";
    while (1) {
        char *cmd = sf_readline(prompt); 
        if (cmd == NULL) { // for EOF
            free(cmd);
            if (in == NULL || in == stdin)
                returnValue = -1;
            break;
        }       
        int length = 0;
        char **array = split_string(cmd, &length);

        // decrement length to account for command
        length--;
        if (length < 0) // for an empty line
            goto bad_arg;
        else if (strcmp(*array, "help") == 0) {
            CHECK_ARG(length, 0);
            printf("Commands are: help quit type printer conversion printers jobs print cancel disable enable pause resume\n");
            sf_cmd_ok();
        } else if (strcmp(*array, "quit") == 0) {
            CHECK_ARG(length, 0);
            free(cmd);
            free(array);
            sf_cmd_ok();
            returnValue = -1;
            break;
        } else if (strcmp(*array, "type") == 0) {
            CHECK_ARG(length, 1);
            if (define_type(array[1]) == NULL)
                sf_cmd_error("type");
            else
                sf_cmd_ok();
        } else if (strcmp(*array, "printer") == 0) {
            CHECK_ARG(length, 2);
            FILE_TYPE *type;
            PRINTER *printer;
            if (find_printer_name(array[1]) != NULL) {
                printf("Printer name (%s) already exists\n", array[1]);
                sf_cmd_error("printer - printer name already exists");
                goto bad_arg;
            }
            if ((type = find_type(array[2])) == NULL) {
                printf("Unknown file type: %s\n", array[2]);
                sf_cmd_error("printer - unknown file type");
                goto bad_arg;
            }
            if ((printer = define_printer(array[1], type)) == NULL) {
                printf("Too many printers (32 max)\n");
                sf_cmd_error("printer - too many printers");
                goto bad_arg;
            }
            sf_printer_defined(printer->name, printer->type->name);
            printf("PRINTER: id=%ld, name=%s, type=%s, status=%s\n", printer-printers, printer->name, printer->type->name, printer_status_names[printer->status]);

            sf_cmd_ok();
        } else if (strcmp(*array, "conversion") == 0) {
            CHECK_ARG(length, 3);
            if (find_type(array[1]) == NULL) {
                printf("Undeclared file type: %s\n", array[1]);
                sf_cmd_error("conversion");
                goto bad_arg;
            }
            if (find_type(array[2]) == NULL) {
                printf("Undeclared file type: %s\n", array[2]);
                sf_cmd_error("conversion");
                goto bad_arg;
            }
            CONVERSION *conversion = define_conversion(array[1], array[2], &array[3]);
            if (conversion == NULL)
                sf_cmd_error("conversion");
            else
                sf_cmd_ok();
        } else if (strcmp(*array, "printers") == 0) {
            CHECK_ARG(length, 0);
            for (int i = 0; i < printer_count; i++)
                printf("PRINTER: id=%d, name=%s, type=%s, status=%s\n", i, printers[i].name, printers[i].type->name, printer_status_names[printers[i].status]);
            sf_cmd_ok();
        } else if (strcmp(*array, "jobs") == 0) {
            CHECK_ARG(length, 0);
            for (int i = 0; i < MAX_JOBS; i++) {
                if ((job_count >> i) & 0x1) {
                    printf("JOB[%d]: status=%s, eligible=%x, file=%s\n", i, job_status_names[jobs[i].status], jobs[i].eligible, jobs[i].file);
                }
            }
            sf_cmd_ok();
        } else if (strcmp(*array, "print") == 0) {
            if (length <= 1)
                CHECK_ARG(length, 1);
            FILE_TYPE *type;
            PRINTER *printer;
            JOB *job;
            int printer_set = (length == 1)?~0:0; // if no printers specified every printer is eligible
            if ((type = infer_file_type(array[1])) == NULL) {
                printf("Unknown file type: %s\n", array[1]);
                sf_cmd_error("print - unknown file type");
                goto bad_arg;
            }
            for (int i = 2; i <= length; i++) {
                printer = find_printer_name(array[i]);
                if (printer == NULL) {
                    printf("Unknown printer: %s\n", array[i]);
                    sf_cmd_error("print - unknown printer");
                    goto bad_arg;
                }
                printer_set |= 1 << (printer-printers);
            }
            if ((job = create_job(array[1], type, printer_set)) == NULL) {
                printf("Too many jobs (64 max)\n");
                sf_cmd_error("printer - too many jobs");
                goto bad_arg;
            }
            printf("JOB[%ld]: status=%s, eligible=%08x, file=%s\n", job-jobs, job_status_names[job->status], job->eligible, job->file);
            sf_cmd_ok();
        } else if (strcmp(*array, "cancel") == 0) {
            CHECK_ARG(length, 1);
            
        } else if (strcmp(*array, "pause") == 0) {
            CHECK_ARG(length, 1);
            
        } else if (strcmp(*array, "resume") == 0) {
            CHECK_ARG(length, 1);
            
        } else if (strcmp(*array, "disable") == 0) {
            CHECK_ARG(length, 1);
            
        } else if (strcmp(*array, "enable") == 0) {
            CHECK_ARG(length, 1);
            pid_t pid;
            PRINTER *printer = find_printer_name(array[1]);
            if (printer == NULL) {
                printf("No printer: %s\n", array[1]);
                sf_cmd_error("enable - no printer");
                goto bad_arg;
            }
            printer->status = PRINTER_IDLE;
            sf_printer_status(printer->name, PRINTER_IDLE);
            pid = fork();
            if (pid == 0) { // child
                if (setpgid(0,0) == -1)
                    perror("setpgid error:");
                int iteration = 0;
                while (iteration < 10) { // check the jobs array
                    if (printer->status != PRINTER_IDLE)
                        continue;
                    int printer_mask = 1 << (printer-printers);
                    int fd = -1;
                    iteration++;
                    warn("Checking jobs array %d", iteration);
                    for (int i = 0; i < MAX_JOBS; i++) {
                        if (((job_count >> i) & 0x1) && ((jobs[i].eligible & printer_mask) != 0) && jobs[i].status == JOB_CREATED) {
                            // valid job to process
                            debug("Starting job %d", i);
                            jobs[i].status = JOB_RUNNING;
                            sf_job_status(i, JOB_RUNNING);
                            printer->status = PRINTER_BUSY;
                            sf_printer_status(printer->name, printer->status);
                            // sf_job_started(i, printer->name, getpid(), NULL);
                            if (fd == -1) 
                                fd = imp_connect_to_printer(printer->name, printer->type->name, PRINTER_NORMAL);
                            FILE *input_file = fopen(jobs[i].file, "r");
                            if (input_file == NULL) 
		                        perror("input file");
                            info("here:%d->%d", fileno(input_file), fd);
                            pid_t job_pid = fork();
                            int child_status;
                            if (job_pid == 0) {
                                char *cat[] = {
                                    "/bin/cat",
                                    NULL,
                                };
                                info("%d->%d", fileno(input_file), fd);
                                dup2(fileno(input_file), STDIN_FILENO);
                                dup2(fd, STDOUT_FILENO);
                                if (execvp(cat[0], cat) < 0) {
                                    perror("WHY");
                                    exit(1);
                                }
                                exit(0);
                            } else {
                                waitpid(job_pid, &child_status, 0);
                                if (WIFEXITED(child_status)) {
                                    sf_job_status(i, JOB_FINISHED);
                                    sf_job_finished(i, WEXITSTATUS(child_status));
                                    printer->status = PRINTER_IDLE;
                                    sf_printer_status(printer->name, PRINTER_IDLE);
                                } else {
                                    sf_job_status(i, JOB_ABORTED);
                                    sf_job_aborted(i, WEXITSTATUS(child_status));
                                    printer->status = PRINTER_IDLE;
                                    sf_printer_status(printer->name, PRINTER_IDLE);
                                }
                            }
                        }
                    }
                }
                exit(0);
            } else { // parent
                int scan_status;
                waitpid(pid, &scan_status, 0);
            }
        } else {
            printf("Unrecognized command: %s\n", *array);
            sf_cmd_error("unrecognized command");
        }
        
        bad_arg:
            free(cmd);
            free(array);
            
    }
    // fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
    // abort();

    // dup2(stdin_copy, STDIN_FILENO);
    // dup2(stdout_copy, STDOUT_FILENO);
    for (int i = 0; i < printer_count; i++) 
        free(printers[i].name);
    for (int i = 0; i < MAX_JOBS; i++) {
        if ((job_count >> i) & 0x1) {
            free(jobs[i].file);
        }
    }
    return returnValue;
}