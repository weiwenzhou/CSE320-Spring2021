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