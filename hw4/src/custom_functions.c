#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "conversions.h"
#include "imprimer.h"
#include "custom_functions.h"
#include "debug.h"

char **split_string(char *string, int *length) {
    if (strlen(string) == 0) 
        return NULL;
    int length_temp = 1;
    char *updated = malloc(strlen(string)+1);
    strcpy(updated, string);
    char *token = strtok(updated, " ");
    // get the length of the array
    while ((token = strtok(NULL, " ")) != NULL) 
        length_temp++;
    // create an array of the proper
    char **array = calloc(length_temp+1, sizeof(char *));
    *length = length_temp;
    length_temp = 0;
    token = strtok(string, " ");
    array[length_temp++] = token;
    while ((token = strtok(NULL, " ")) != NULL) 
        array[length_temp++] = token;
    array[length_temp] = NULL;
    free(updated);
    return array;
}

PRINTER *define_printer(char *name, FILE_TYPE *type) {
    // info("%d", printer_count);
    if (printer_count == MAX_PRINTERS)
        return NULL;
    PRINTER *new_printer = &printers[printer_count++];
    char *name_copy = malloc(strlen(name)+1); // minimum length + 1 for \0
    strcpy(name_copy, name);
    new_printer->name = name_copy;
    new_printer->type = type;
    new_printer->status = PRINTER_DISABLED;
    return new_printer;
}

PRINTER *find_printer_name(char *name) {
    for (int i = 0; i < printer_count; i++) {
        if (strcmp(printers[i].name, name) == 0)
            return &printers[i];
    }
    return NULL;
}

JOB *create_job(char *file, FILE_TYPE *type, int printer_set) {
    if (~job_count == 0) // if every bit is 1 then jobs array is full
        return NULL;
    size_t temp = ~job_count; // bits that are 1 are open slots
    for (int i = 0; i < MAX_JOBS; i++) {
        if ((temp >> i) & 0x1) {
            job_count |= 1 << i;
            JOB *new_job = &jobs[i];
            char *file_copy = malloc(strlen(file)+1); // length + 1 for \0
            strcpy(file_copy, file);
            new_job->file = file_copy;
            new_job->type = type;
            new_job->status = JOB_CREATED;
            new_job->eligible = printer_set;
            return new_job;
        }
    }
    return NULL; // it should never get here
}

pid_t start_job(PRINTER *printer, JOB *job) {
    // debug("Starting job %ld", job-jobs);
    job->status = JOB_RUNNING;
    sf_job_status(job-jobs, JOB_RUNNING);
    printer->status = PRINTER_BUSY;
    sf_printer_status(printer->name, printer->status);
    CONVERSION **path = find_conversion_path(job->type->name, printer->type->name);
    int length = 0;
    while (path[length] != NULL) {
        // debug("%s->%s : %s", path[length]->from->name, path[length]->to->name, path[length]->cmd_and_args[0]);
        length++;
    }
    // info("%d", length);
    char **commands = calloc(length+1, sizeof(char *));
    for (int i = 0; i < length; i++) {
        commands[i] = path[i]->cmd_and_args[0];
    }
    commands[length] = NULL;
    sf_job_started(job-jobs, printer->name, getpgrp(), commands);
    free(commands);
    pid_t pid = fork();
    if (pid == 0) { // child (master of the pipeline)
        if (setpgid(0,0) == -1) // set group pid
            perror("setpgid error:");
        int fd_printer = -1;
        if (fd_printer == -1) 
            fd_printer = imp_connect_to_printer(printer->name, printer->type->name, PRINTER_NORMAL);
        FILE *input_file = fopen(job->file, "r");
        if (input_file == NULL) {
            perror("input file");
            exit(1);
        }

        if (length == 0) {
            pid_t job_pid = fork();
            int child_status;
            if (job_pid == 0) {
                char *cat[] = {
                    "/bin/cat",
                    NULL,
                };
                // info("%d->%d", fileno(input_file), fd_printer);
                dup2(fileno(input_file), STDIN_FILENO);
                dup2(fd_printer, STDOUT_FILENO);
                if (execvp(cat[0], cat) < 0) {
                    perror("WHY");
                    exit(1);
                }
                exit(0);
            } else {
                waitpid(job_pid, &child_status, 0);
                if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0)
                    exit(0);
                else 
                    exit(1);
            }
        } else {
            int pipe_fd[2];
            int in_fd;
            for (int i = 0; i < length; i++) {
                int child_status;
                
                pipe(pipe_fd);
                // info("my pipes %d -> %d", pipe_fd[0], pipe_fd[1]);
                pid_t job_pid = fork();
                if (job_pid == 0) { // child
                    // debug("%d: Executing %s->%s : %s", getpid(),path[i]->from->name, path[i]->to->name, path[i]->cmd_and_args[0]);
                    sigset_t sigterm_mask;
                    sigemptyset(&sigterm_mask);
                    sigaddset(&sigterm_mask, SIGTERM);
                    sigprocmask(SIG_UNBLOCK, &sigterm_mask, NULL);
                    close(pipe_fd[0]); // close read side;
                    if (i == 0)
                        dup2(fileno(input_file), STDIN_FILENO);
                    else {// read in_fd
                        dup2(in_fd, STDIN_FILENO);
                        close(in_fd);
                    }
                    if (i == length-1)
                        dup2(fd_printer, STDOUT_FILENO);
                    else // write to pipe[1]
                        dup2(pipe_fd[1], STDOUT_FILENO);
                    if (execvp(path[i]->cmd_and_args[0], path[i]->cmd_and_args) < 0) {
                        perror("WHY");
                        exit(1);
                    }
                    exit(0);
                } else { // parent
                    close(pipe_fd[1]); // close write side;
                    waitpid(job_pid, &child_status, 0);
                    in_fd = pipe_fd[0]; // set next read as the read end of pipe
                    if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0) {
                        if (i == length-1)
                            exit(0);
                    } else {
                        exit(1);
                    }
                }
            }
        }
        exit(1); // should never be reached by place here just in case
    } 
    return pid;  
}

void job_handler(int sig) {
    int olderrno = errno;
    int child_status;
    pid_t pid = waitpid(-1, &child_status, WNOHANG|WSTOPPED|WCONTINUED);
    if (pid >= 0) {
        // might need to block other signals during this process of reading/writing
        // get job id from pid
        // debug("FROM %d", pid);
        // info("%d", id);
        if (WIFEXITED(child_status) || WIFSIGNALED(child_status)) { // exited
            job_process_count--;
            int printer_id, job_id;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (pid == job_pids[i]) {
                    job_id = i;
                    job_pids[i] = 0;
                    break;
                }
            }
            for (int i = 0; i < printer_count; i++) {
                if (pid == printer_pids[i]) {
                    printer_id = i;
                    printer_pids[i] = 0;
                    break;
                }
            }
            if (!WIFSIGNALED(child_status) && WEXITSTATUS(child_status) == EXIT_SUCCESS) {
                // change job status to JOB_FINISHED
                jobs[job_id].status = JOB_FINISHED;
                sf_job_status(job_id, JOB_FINISHED);
                sf_job_finished(job_id, WEXITSTATUS(child_status));
                printers[printer_id].status = PRINTER_IDLE;
                sf_printer_status(printers[printer_id].name, PRINTER_IDLE);
            } else { // EXIT_FAILURE
                // change job status to JOB_ABORT
                jobs[job_id].status = JOB_ABORTED;
                sf_job_status(job_id, JOB_ABORTED);
                sf_job_aborted(job_id, WEXITSTATUS(child_status));
                printers[printer_id].status = PRINTER_IDLE;
                sf_printer_status(printers[printer_id].name, PRINTER_IDLE);
            }
            // job waits to be deleted
            job_timestamps[job_id] = time(NULL); // time async safe
        } else if (WIFSTOPPED(child_status)) { // process stopped
            // change job status to JOB_PAUSE if job status is JOB_RUNNING
            int job_id;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (pid == job_pids[i]) {
                    job_id = i;
                    break;
                }
            } 
            jobs[job_id].status = JOB_PAUSED;
            sf_job_status(job_id, JOB_PAUSED);
        } else if (WIFCONTINUED(child_status)) { // process continued
            // change job status to JOB_RUNNING if job status is JOB_PAUSE
            int job_id;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (pid == job_pids[i]) {
                    job_id = i;
                    break;
                }
            } 
            jobs[job_id].status = JOB_RUNNING;
            sf_job_status(job_id, JOB_RUNNING);
        }
    }
    if (job_process_count == 0) 
        jobs_done = 0;

    errno = olderrno;
}

void scanner() {
    for (int p_id = 0; p_id < printer_count; p_id++) {
        int printer_mask = 1 << p_id;
        for (int i = 0; i < MAX_JOBS; i++) {
            if (((job_count >> i) & 0x1) && ((jobs[i].eligible & printer_mask) != 0) && jobs[i].status == JOB_CREATED && printers[p_id].status == PRINTER_IDLE) {
                // check type
                if (find_conversion_path(jobs[i].type->name, printers[p_id].type->name) != NULL) {
                    job_process_count++;
                    jobs_done = 1;
                    pid_t job = start_job(&printers[p_id], &jobs[i]);
                    printer_pids[p_id] = job;
                    job_pids[i] = job;
                }
            }
        }
    }
}