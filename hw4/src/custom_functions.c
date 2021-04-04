#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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