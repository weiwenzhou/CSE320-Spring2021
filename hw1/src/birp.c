/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"

int pgm_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {
    // TO BE IMPLEMENTED
    // check if there are flags (argc > 1)

    // check if the first flag is -h 
    global_options = HELP_OPTION;

    // loop through the flags to check
    // if the first flag is not -h then the max # of flags is 6
    // ex. bin/birp -i birp -i birp -t 64

    // certain conditional to watch
    
    // -i and -o can appear in either order
    // -i and -o can only appear at most once each
    // -i pgm|birp default birp
    // -o pgm|birp|ascii default birp
    
    // if input and output format are both birp
    // at most one of following is allow
    // -n|-r|-t|-z|-Z 
    // for -t the next flag must be an integer in the range [0,255]
    // for -z & -Z the next flag must be an integer in the range [0,16]

    // for (int index = 0; index < argc; index++) {
        // 
    // }

    // Placeholder for the checking value
    printf("%X\n", global_options);

    return -1;
}
