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