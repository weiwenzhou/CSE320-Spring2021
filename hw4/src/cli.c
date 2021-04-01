/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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
    // int fd_in = fileno(in);
    // dup2(fd_in, STDIN_FILENO);
    char *prompt = (in == stdin) ? "imp> ":"";
    while (1) {
        char *cmd = sf_readline(prompt);        
        int length = 0;
        char **array = split_string(cmd, &length);

        // decrement length to account for command
        length--;
        if (cmd == NULL)
            break;
        else if (strcmp(*array, "help") == 0) {
            CHECK_ARG(length, 0);
            printf("Commands are: help quit type printer conversion printers jobs print cancel disable enable pause resume\n");
            sf_cmd_ok();
        } else if (strcmp(*array, "quit") == 0) {
            CHECK_ARG(length, 0);
            free(cmd);
            free(array);
            sf_cmd_ok();
            return -1;
        } else if (strcmp(*array, "type") == 0) {
            
        } else if (strcmp(*array, "printer") == 0) {
            
        } else if (strcmp(*array, "conversion") == 0) {
            
        } else if (strcmp(*array, "printers") == 0) {
            
        } else if (strcmp(*array, "jobs") == 0) {
            
        } else if (strcmp(*array, "print") == 0) {
            
        } else if (strcmp(*array, "cancel") == 0) {
            
        } else if (strcmp(*array, "pause") == 0) {
            
        } else if (strcmp(*array, "resume") == 0) {
            
        } else if (strcmp(*array, "disable") == 0) {
            
        } else if (strcmp(*array, "enable") == 0) {
            
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
    return 0;
}
