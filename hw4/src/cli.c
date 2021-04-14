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
#include <time.h>
#include <unistd.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"
#include "custom_functions.h"
#include "debug.h"

int run_cli(FILE *in, FILE *out)
{
    sigset_t mask_all, prev_mask;
    sigfillset(&mask_all);

    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = job_handler;
    sigfillset(&sig_act.sa_mask);
    if (sigaction(SIGCHLD, &sig_act, 0) == -1) {
        perror("Fail to install SIGCHLD handler");
        return -1;
    }
    signal(SIGINT, SIG_IGN); // simply to avoid accidental termination by ctrl+c
    sf_set_readline_signal_hook(scanner);
    // TO BE IMPLEMENTED
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int fd_in = fileno(in);
    int fd_out = fileno(out);
    dup2(fd_in, STDIN_FILENO);
    dup2(fd_out, STDOUT_FILENO);
    // info("%p", jobs[0].type);
    int returnValue = 0;
    char *prompt = (in != stdin || out != stdout) ? "":"imp> ";
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
        if (array == NULL) // for an empty line
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
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            if ((printer = define_printer(array[1], type)) == NULL) {
                if (!program_failure) { // not memory allocation error
                    printf("Too many printers (32 max)\n");
                    sf_cmd_error("printer - too many printers");
                }
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                goto bad_arg;
            }
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            sf_printer_defined(printer->name, printer->type->name);
            printf("PRINTER: id=%ld, name=%s, type=%s, status=%s\n", printer-printers, printer->name, printer->type->name, printer_status_names[printer->status]);

            sf_cmd_ok();

        } else if (strcmp(*array, "conversion") == 0) {
            if (length <= 3)
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
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            for (int i = 0; i < printer_count; i++)
                printf("PRINTER: id=%d, name=%s, type=%s, status=%s\n", i, printers[i].name, printers[i].type->name, printer_status_names[printers[i].status]);
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            sf_cmd_ok();

        } else if (strcmp(*array, "jobs") == 0) {
            CHECK_ARG(length, 0);
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            for (int i = 0; i < MAX_JOBS; i++) {
                if ((job_count >> i) & 0x1) {
                    printf("JOB[%d]: status=%s, eligible=%x, file=%s\n", i, job_status_names[jobs[i].status], jobs[i].eligible, jobs[i].file);
                }
            }
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
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
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            if ((job = create_job(array[1], type, printer_set)) == NULL) {
                if (!program_failure) { // not memory allocation error
                    printf("Too many jobs (64 max)\n");
                    sf_cmd_error("printer - too many jobs");
                }
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                goto bad_arg;
            }
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            printf("JOB[%ld]: status=%s, eligible=%08x, file=%s\n", job-jobs, job_status_names[job->status], job->eligible, job->file);
            sf_job_created(job-jobs, job->file, job->type->name);
            sf_cmd_ok();

        } else if (strcmp(*array, "cancel") == 0) {
            CHECK_ARG(length, 1);
            char *leftover;
            int job_id = strtol(array[1], &leftover, 10);
            if (strlen(leftover) != 0 || job_id >= MAX_JOBS || job_id < 0) {
                printf("Invalid job number %s\n", array[1]);
                sf_cmd_error("pause - invalid job number");
                goto bad_arg;
            }
            // info("id %d pid: %d sig term", job_id, job_pids[job_id]);
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            killpg(job_pids[job_id], SIGTERM);
            killpg(job_pids[job_id], SIGCONT);
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            sf_cmd_ok();
            
        } else if (strcmp(*array, "pause") == 0) {
            CHECK_ARG(length, 1);
            char *leftover;
            int job_id = strtol(array[1], &leftover, 10);
            if (strlen(leftover) != 0 || job_id >= MAX_JOBS || job_id < 0) {
                printf("Invalid job number %s\n", array[1]);
                sf_cmd_error("pause - invalid job number");
                goto bad_arg;
            }
            // info("id %d pid: %d", job_id, job_pids[job_id]);
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            killpg(job_pids[job_id], SIGSTOP);
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            sf_cmd_ok();

        } else if (strcmp(*array, "resume") == 0) {
            CHECK_ARG(length, 1);
            char *leftover;
            int job_id = strtol(array[1], &leftover, 10);
            if (strlen(leftover) != 0 || job_id >= MAX_JOBS || job_id < 0) {
                printf("Invalid job number %s\n", array[1]);
                sf_cmd_error("resume - invalid job number");
                goto bad_arg;
            }
            // info("id %d pid: %d", job_id, job_pids[job_id]);
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            killpg(job_pids[job_id], SIGCONT);
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            sf_cmd_ok();

        } else if (strcmp(*array, "disable") == 0) {
            CHECK_ARG(length, 1);
            PRINTER *printer = find_printer_name(array[1]);
            if (printer == NULL) {
                printf("No printer: %s\n", array[1]);
                sf_cmd_error("enable - no printer");
                goto bad_arg;
            }
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            printer->status = PRINTER_DISABLED;
            sf_printer_status(printer->name, PRINTER_DISABLED);
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        } else if (strcmp(*array, "enable") == 0) {
            CHECK_ARG(length, 1);
            // pid_t pid;
            PRINTER *printer = find_printer_name(array[1]);
            if (printer == NULL) {
                printf("No printer: %s\n", array[1]);
                sf_cmd_error("enable - no printer");
                goto bad_arg;
            }
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            // if printer_pid == 0 then go idle else go to busy
            PRINTER_STATUS new_status = (printer_pids[printer-printers]) ? PRINTER_BUSY:PRINTER_IDLE;
            printer->status = new_status;
            sf_printer_status(printer->name, new_status);
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        } else {
            printf("Unrecognized command: %s\n", *array);
            sf_cmd_error("unrecognized command");
        }
        
        bad_arg:
            free(cmd);
            free(array);
            // check for job deletion
            // block signal
            sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
            time_t current = time(NULL);
            for (int i = 0; i < MAX_JOBS; i++) {
                if (job_timestamps[i] != 0 && current - job_timestamps[i] >= 10) {
                    job_timestamps[i] = 0;
                    sf_job_status(i, JOB_DELETED);
                    sf_job_deleted(i);
                    free(jobs[i].file);
                    job_count ^= 1 << i; // flip bit from 1 to 0.
                }
            }
            // end block signal
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            if (program_failure == 1) {
                debug("HERE?");
                break;
            }

    }
    // reset only if in interactive mode
    if (in == stdin || returnValue || program_failure == 1) {
        while (jobs_done != 0)
            ;
        for (int i = 0; i < printer_count; i++) 
            free(printers[i].name);
        for (int i = 0; i < MAX_JOBS; i++) {
            if ((job_count >> i) & 0x1) {
                free(jobs[i].file);
            }
        }
    }
    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    if (program_failure == 1) {
        conversions_fini();
        sf_fini();
        exit(EXIT_FAILURE);
    }
    return returnValue;
}
