#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    char **array = calloc(length_temp, sizeof(char *));
    *length = length_temp;
    length_temp = 0;
    token = strtok(string, " ");
    array[length_temp++] = token;
    while ((token = strtok(NULL, " ")) != NULL) 
        array[length_temp++] = token;
    free(updated);
    return array;
}

PRINTER *define_printer(char *name, FILE_TYPE *type) {
    info("%d", printer_count);
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
    pid_t pid = fork();
    if (pid == 0) { // child (master of the pipeline)
        if (setpgid(0,0) == -1) // set group pid
            perror("setpgid error:");
        // CONVERSION **path = find_conversion_path(job->type->name, printer->type->name);
        // int length = 0;
        // while (path[length] != NULL) {
        //     length++;
        // }
        debug("Starting job %ld", job-jobs);
        job->status = JOB_RUNNING;
        sf_job_status(job-jobs, JOB_RUNNING);
        printer->status = PRINTER_BUSY;
        sf_printer_status(printer->name, printer->status);
        // sf_job_started(job-jobs, printer->name, getpgrp(), NULL);
        int fd_printer = -1;
        if (fd_printer == -1) 
            fd_printer = imp_connect_to_printer(printer->name, printer->type->name, PRINTER_NORMAL);
        FILE *input_file = fopen(job->file, "r");
        if (input_file == NULL) {
            perror("input file");
            exit(1);
        }

        // debug("HERE?");

        pid_t job_pid = fork();
        int child_status;
        if (job_pid == 0) {
            char *cat[] = {
                "/bin/cat",
                NULL,
            };
            info("%d->%d", fileno(input_file), fd_printer);
            dup2(fileno(input_file), STDIN_FILENO);
            dup2(fd_printer, STDOUT_FILENO);
            if (execvp(cat[0], cat) < 0) {
                perror("WHY");
                exit(1);
            }
            exit(0);
        } else {
            waitpid(job_pid, &child_status, 0);
            if (WIFEXITED(child_status)) {
                sf_job_status(job-jobs, JOB_FINISHED);
                sf_job_finished(job-jobs, WEXITSTATUS(child_status));
                printer->status = PRINTER_IDLE;
                sf_printer_status(printer->name, PRINTER_IDLE);
            } else {
                sf_job_status(job-jobs, JOB_ABORTED);
                sf_job_aborted(job-jobs, WEXITSTATUS(child_status));
                printer->status = PRINTER_IDLE;
                sf_printer_status(printer->name, PRINTER_IDLE);
            }
        }
        

        exit(0);
    } 
    return pid;  
}