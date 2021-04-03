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
    // int stdout_copy = dup(STDOUT_FILENO);
    // int fd_in = fileno(in);
    // int fd_out = fileno(out);
    // dup2(fd_in, STDIN_FILENO);
    // dup2(fd_out, STDOUT_FILENO);
    char *prompt = (in == stdin) ? "imp> ":"";
    while (1) {
        char *cmd = sf_readline(prompt); 
        if (cmd == NULL) {
            free(cmd);
            break;
        }       
        int length = 0;
        char **array = split_string(cmd, &length);

        // decrement length to account for command
        length--;
        if (length < 0)
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
            return -1;
        } else if (strcmp(*array, "type") == 0) {
            CHECK_ARG(length, 1);
            FILE_TYPE *type = define_type(array[1]);
            if (type == NULL)
                sf_cmd_error("type");
            else
                sf_cmd_ok();
        } else if (strcmp(*array, "printer") == 0) {
            CHECK_ARG(length, 2);
            FILE_TYPE *type;
            // check if printer name already exists
            if ((type = find_type(array[2])) == NULL) {
                printf("Unknown file type: %s\n", array[1]);
                sf_cmd_error("printer - unknown file type");
                goto bad_arg;
            }
            if (define_printer(array[1], type) == NULL) {
                printf("Too many printers (32 max)\n");
                sf_cmd_error("printer - too many printers");
                goto bad_arg;
            }
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
            
        } else if (strcmp(*array, "jobs") == 0) {
            CHECK_ARG(length, 0);
            
        } else if (strcmp(*array, "print") == 0) {
            CHECK_ARG(length, 1);
            
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
    return 0;
}
