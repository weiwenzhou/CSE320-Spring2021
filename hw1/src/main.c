#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv)) 
        USAGE(*argv, EXIT_FAILURE);
    // Placeholder for the checking value
    debug("0x%X",global_options);
    if(global_options & HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    // TO BE IMPLEMENTED
    if (global_options == 0x21) {
        if (pgm_to_birp(stdin, stdout))
            return EXIT_FAILURE;
        else 
            return EXIT_SUCCESS;
    }
    if (global_options == 0x12) {
        if (birp_to_pgm(stdin, stdout))
            return EXIT_FAILURE;
        else
            return EXIT_SUCCESS;
    }
    if ((global_options & 0xff) == 0x22) {
        if (birp_to_birp(stdin, stdout))
            return EXIT_FAILURE;
        else
            return EXIT_SUCCESS;
    }
    if (global_options == 0x31) {
        if (pgm_to_ascii(stdin, stdout))
            return EXIT_FAILURE;
        else 
            return EXIT_SUCCESS;
    }
    if (global_options == 0x32) {
        if (birp_to_ascii(stdin, stdout))
            return EXIT_FAILURE;
        else
            return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
