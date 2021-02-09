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
    // global_options: bit 31 is 1 -> 0x80000000 (1000...0000)

    // loop through the flags to check
    // if the first flag is not -h then the max # of flags is 6
    // ex. bin/birp -i birp -i birp -t 64

    // certain conditional to watch
    
    // -i and -o can appear in either order
    // -i and -o can only appear at most once each
    // -i pgm|birp default birp
    // -o pgm|birp|ascii default birp
        // -i is bits 0-3 0x1234567_ 
            // 0000 (0x0) is not allowed
            // 0001 (0x1) for pgm
            // 0010 (0x2) for birp
        // -i is bits 4-8 0x123456_8
            // 0000 (0x0) is not allowed
            // 0001 (0x1) for pgm
            // 0010 (0x2) for birp
            // 0011 (0x3) for ascii 

    // if input and output format are both birp
        // global option is 0x00000022 
    // at most one of following is allow
    // -n|-r|-t|-z|-Z 
    // for -t the next flag must be an integer in the range [0,255]
    // for -z & -Z the next flag must be an integer in the range [0,16]

    // transformation is bits 8-11 0x12345_78
        // 0000 (0x0) is no transformation
        // 0001 (0x1) is -n 
        // 0010 (0x2) is -t
        // 0011 (0x3) is -z/-Z
        // 0100 (0x4) is -r
    // value for transformation is bits 16-23 0x12__5678
        // negative for -z (twos complement)
        // positive for -Z (twos complement)
        // positive for -t (unsigned) [0,255]

    // for (int index = 0; index < argc; index++) {
        // 
    // }

    // Placeholder for the checking value
    printf("%X\n", global_options);

    return -1;
}
