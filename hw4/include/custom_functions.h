#include <stdio.h>

#include "imprimer.h"

#define WRONG_ARG_COUNT(length, required) printf("Wrong number of args (given: %d, required: %d) for CLI command 'help'\n", length, required); sf_cmd_error("arg count");
#define CHECK_ARG(length, required) if (length != required) {WRONG_ARG_COUNT(length, required); goto bad_arg;}

/*
 * Structure that describes a printer.
 */
typedef struct printer {
    char *name;
    FILE_TYPE *type;
    PRINTER_STATUS status;
} PRINTER;

/*
 * Space to store the PRINTERs.
 */
PRINTER printers[MAX_PRINTERS];
// Global counter to keep track of the number of printers
int printer_count;

/**
 * Splits a string using whitespaces as the delimiter. The 
 * strings in the array do not have whitespaces.
 * 
 * @param string The string to split
 * @param length The int pointer to update to the length 
 * 
 * @return NULL if the string is empty else an array of the strings 
 * split by whitespaces.
 */
char **split_string(char *string, int *length);