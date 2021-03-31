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
#include "debug.h"

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    // int stdin_copy = dup(STDIN_FILENO);
    // int fd_in = fileno(in);
    char *prompt = (in == stdin) ? "imp> ":"";
    // dup2(fd_in, STDIN_FILENO);
    while (1) {
        char *cmd = sf_readline(prompt);
        char *token = strtok(cmd, " ");
        int args = -1;
        while (token != NULL) {
            args++;
            token = strtok(NULL, " ");
        }
        info("%d, %s", args, strtok(cmd, " "));
        
        // info("|%s|", cmd);
        if (cmd == NULL)
            break;
        else if (strcmp(strtok(cmd, " "), "help") == 0) {
            printf("%s", strtok(cmd, " "));
            printf("Commands are: help quit type printer conversion printers jobs print cancel disable enable pause resume\n");
            sf_cmd_ok();
        } else if (strcmp(cmd, "quit") == 0) {
            sf_cmd_ok();
            free(cmd);
            return -1;
        } else if (strcmp(cmd, "type") == 0) {
            
        }

        free(cmd);
    }
    // fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
    // abort();

    // dup2(stdin_copy, STDIN_FILENO);
    return 0;
}
