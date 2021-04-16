#include <errno.h>
#include <fcntl.h>
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
    if (updated == NULL) {
        perror("Memory allocation failed. Start terminating...");
        program_failure = 1;
        return NULL;
    }
    strcpy(updated, string);
    char *token = strtok(updated, " ");
    if (token == NULL) {
        free(updated);
        return NULL;
    }
    // get the length of the array
    while ((token = strtok(NULL, " ")) != NULL) 
        length_temp++;
    // create an array of the proper
    free(updated);
    char **array = calloc(length_temp+1, sizeof(char *));
    if (array == NULL) {
        perror("Memory allocation failed. Start terminating...");
        program_failure = 1;
        return NULL;
    }
    *length = length_temp;
    length_temp = 0;
    token = strtok(string, " ");
    array[length_temp++] = token;
    while ((token = strtok(NULL, " ")) != NULL) 
        array[length_temp++] = token;
    array[length_temp] = NULL;
    return array;
}

PRINTER *define_printer(char *name, FILE_TYPE *type) {
    // info("%d", printer_count);
    if (printer_count == MAX_PRINTERS)
        return NULL;
    char *name_copy = malloc(strlen(name)+1); // minimum length + 1 for \0
    if (name_copy == NULL) {
        perror("Memory allocation failed. Start terminating...");
        program_failure = 1;
        return NULL;
    }
    strcpy(name_copy, name);
    PRINTER *new_printer = &printers[printer_count++];
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
            char *file_copy = malloc(strlen(file)+1); // length + 1 for \0
            strcpy(file_copy, file);
            if (file_copy == NULL) {
                perror("Memory allocation failed. Start terminating...");
                program_failure = 1;
                return NULL;
            }
            job_count |= ((uint64_t)1) << i;
            JOB *new_job = &jobs[i];
            new_job->file = file_copy;
            new_job->type = type;
            new_job->status = JOB_CREATED;
            new_job->eligible = printer_set;
            return new_job;
        }
    }
    return NULL; // it should never get here
}

pid_t start_job(PRINTER *printer, JOB *job, CONVERSION **path) {
    char **commands; // for sf_job_started
    int length = 0; // length of path array excluding the NULL
    while (path[length] != NULL)
        length++;
    pid_t pid = fork();
    switch (pid) {
        case -1: // can't create master process. ignore job start. 
            // perror("Can't fork to start master process.");
            return -1;
            break;
        case 0: // child - master process
            if (setpgid(0,0) == -1) // set group pid
                perror("setpgid error:");
            int fd_printer = imp_connect_to_printer(printer->name, printer->type->name, PRINTER_NORMAL);
            if (fd_printer == -1) // can't connect to printer -> abort job
                exit(1);
            int input_fd = open(job->file, O_RDONLY); // async
            if (input_fd == -1) // can't open fail -> abort job
                exit(1);

            if (length == 0) {
                int child_status;
                char *cat_command[] = {"/bin/cat", NULL};
                sigset_t sigterm_mask;
                pid_t job_pid = fork();
                switch (job_pid) {
                    case -1: // error
                        exit(1); // can't fork -> abort job
                        break;

                    case 0: // child - pipeline
                        if (sigemptyset(&sigterm_mask) == -1) // async
                                exit(1);
                        if (sigaddset(&sigterm_mask, SIGTERM) == -1) // async
                            exit(1);
                        if (sigprocmask(SIG_UNBLOCK, &sigterm_mask, NULL) == -1) // async
                            exit(1);
                        if (dup2(input_fd, STDIN_FILENO) == -1) // async
                            exit(1); // dup fail -> abort job
                        if (dup2(fd_printer, STDOUT_FILENO) == -1) // async
                            exit(1); // dup fail -> abort job
                        if (execvp(cat_command[0], cat_command) == -1) 
                            exit(1); // exec fail -> abort job
                        // execvp does not return when it runs successfully
                        break;

                    default: // parent
                        if (waitpid(job_pid, &child_status, 0) == -1) // async
                            exit(1); // waitpid fail -> abort job
                        if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0)
                            exit(0); // child exited with 0
                        else 
                            exit(1);
                        break;   
                }
            } else {
                job_process_count = 0;
                exitValue = 0;
                sigset_t sigchld_mask, sigterm_mask;
                if (sigemptyset(&sigchld_mask) == -1) // async
                    exit(1);
                if (sigaddset(&sigchld_mask, SIGCHLD) == -1) // async
                    exit(1);
                if (sigprocmask(SIG_UNBLOCK, &sigchld_mask, NULL) == -1) // async
                    exit(1);
                struct sigaction sig_act;
                memset(&sig_act, 0, sizeof(sig_act));
                sig_act.sa_handler = pipeline_handler;
                sigfillset(&sig_act.sa_mask);
                if (sigaction(SIGCHLD, &sig_act, 0) == -1) { // async
                    perror("Fail to install SIGCHLD handler");
                    exit(1);
                }
                int pipe_fd[2];
                int in_fd = input_fd;
                for (int i = 0; i < length && exitValue != 1; i++) { // stop forking if job is determined to be aborted
                    pipe(pipe_fd);
                    // info("my pipes %d -> %d", pipe_fd[0], pipe_fd[1]);
                    pid_t job_pid = fork(); // async
                    switch (job_pid) {
                        case -1: // error
                            exit(1); // can't fork -> abort job
                            break;
                        case 0: // child
                            if (sigemptyset(&sigterm_mask) == -1) // async
                                exit(1);
                            if (sigaddset(&sigterm_mask, SIGTERM) == -1) // async
                                exit(1);
                            if (sigprocmask(SIG_UNBLOCK, &sigterm_mask, NULL) == -1) // async
                                exit(1);
                            if (close(pipe_fd[0]) == -1) // close read side; // async
                                exit(1);
                            if (i == 0) {
                                if (dup2(input_fd, STDIN_FILENO) == -1) // async
                                    exit(1); 
                            } else {// read in_fd
                                if (dup2(in_fd, STDIN_FILENO) == -1) // async
                                    exit(1);
                                if (close(in_fd) == -1) // async
                                    exit(1);
                            }
                            if (i == length-1) {
                                if (dup2(fd_printer, STDOUT_FILENO) == -1) // async
                                    exit(1);
                            }
                            else { // write to pipe[1]
                                if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) // async
                                    exit(1);
                            }
                            if (execvp(path[i]->cmd_and_args[0], path[i]->cmd_and_args) == -1) 
                                exit(1);
                            // execvp does not return when it runs successfully
                            break;
                        
                        default: // parent
                            if (close(pipe_fd[1]) == -1) // close write side;
                                exit(1);
                            in_fd = pipe_fd[0]; // set next read as the read end of pipe
                            break;
                    }
                }
                sigset_t waitsigchld_mask;
                if (sigfillset(&waitsigchld_mask) == -1) // async
                    exit(1);
                if (sigdelset(&waitsigchld_mask, SIGCHLD) == -1) // async
                    exit(1);
                while (job_process_count != length) {
                    sigsuspend(&waitsigchld_mask); // always return -1 - async
                }
                exit(exitValue); 
            }
            // this point should never be reached but just in case.
            return -1; // shouldn't be return but added in order to compile without warnings
            break;

        default: // parent (fork sucessful. job is running and started now)
            commands = calloc(length+1, sizeof(char *));
            if (commands == NULL) // might need to reconsider how to handle this error
                return -1;
            job->status = JOB_RUNNING;
            sf_job_status(job-jobs, JOB_RUNNING);
            printer->status = PRINTER_BUSY;
            sf_printer_status(printer->name, printer->status);
            for (int i = 0; i < length; i++) {
                commands[i] = path[i]->cmd_and_args[0];
            }
            commands[length] = NULL;
            sf_job_started(job-jobs, printer->name, pid, commands);
            free(commands);
            return pid; 
            break;
    }
}

void job_handler(int sig) {
    int olderrno = errno;
    int child_status;
    pid_t pid;
    while ((pid = waitpid(-1, &child_status, WNOHANG|WSTOPPED|WCONTINUED)) > 0) {
        // might need to block other signals during this process of reading/writing
        // get job id from pid
        // debug("FROM %d", pid);
        // info("%d", id);
        if (WIFEXITED(child_status) || WIFSIGNALED(child_status)) { // exited
            job_process_count--;
            int printer_id, job_id;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (pid == jobs[i].pid) {
                    job_id = i;
                    jobs[i].pid = 0;
                    break;
                }
            }
            for (int i = 0; i < printer_count; i++) {
                if (pid == printers[i].pid) {
                    printer_id = i;
                    printers[i].pid = 0;
                    break;
                }
            }
            if (!WIFSIGNALED(child_status) && WEXITSTATUS(child_status) == EXIT_SUCCESS) {
                // change job status to JOB_FINISHED
                jobs[job_id].status = JOB_FINISHED;
                sf_job_status(job_id, JOB_FINISHED);
                sf_job_finished(job_id, WEXITSTATUS(child_status));
            } else { // EXIT_FAILURE
                // change job status to JOB_ABORT
                jobs[job_id].status = JOB_ABORTED;
                sf_job_status(job_id, JOB_ABORTED);
                sf_job_aborted(job_id, child_status);
            }
            if (printers[printer_id].status == PRINTER_BUSY) {
                printers[printer_id].status = PRINTER_IDLE;
                sf_printer_status(printers[printer_id].name, PRINTER_IDLE);
            }
            // job waits to be deleted
            jobs[job_id].timestamp = time(NULL); // time async safe
        } else if (WIFSTOPPED(child_status)) { // process stopped
            // change job status to JOB_PAUSE if job status is JOB_RUNNING
            int job_id;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (pid == jobs[i].pid) {
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
                if (pid == jobs[i].pid) {
                    job_id = i;
                    break;
                }
            } 
            jobs[job_id].status = JOB_RUNNING;
            sf_job_status(job_id, JOB_RUNNING);
        }
        if (job_process_count == 0) 
            jobs_done = 0;
    }

    errno = olderrno;
}

void scanner() {
    for (int p_id = 0; p_id < printer_count; p_id++) {
        int printer_mask = 1 << p_id;
        for (int i = 0; i < MAX_JOBS; i++) {
            if (((job_count >> i) & 0x1) && ((jobs[i].eligible & printer_mask) != 0) && jobs[i].status == JOB_CREATED && printers[p_id].status == PRINTER_IDLE) {
                // check type
                CONVERSION **path = find_conversion_path(jobs[i].type->name, printers[p_id].type->name);
                if (path != NULL) {
                    pid_t job = start_job(&printers[p_id], &jobs[i], path);
                    if (job != -1) { // if job is started update the appropriate values.
                        job_process_count++;
                        jobs_done = 1;
                        printers[p_id].pid = job;
                        jobs[i].pid = job;
                    }
                }
                free(path);
            }
        }
    }
}

void pipeline_handler(int sig) {
    int olderrno = errno;
    int child_status;
    pid_t pid;
    while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0) {
        job_process_count++;
        if (!(WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0)) {
            exitValue = 1;
        } 
    }
    errno = olderrno;
}