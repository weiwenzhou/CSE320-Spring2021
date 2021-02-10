/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"
#include "custom_functions.h"

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
    if (argc <= 1)
        return -1;
    // check if the first flag is -h 
    // global_options: bit 31 is 1 -> 0x80000000 (1000...0000)
    if (equal(*(argv+1), "-h")) {
        global_options = HELP_OPTION;
        return 0;
    }
    // if the first flag is not -h then the max # of flags is 6
    // ex. bin/birp -i birp -i birp -t 64 (argc <= 7)
    if (argc > 7)
        return -1;
    // loop through the flags to check
    int io_search_done = 0;
    int input = 0;
    int output = 0;
    // set default global_option 0x00000022 (birp birp)
    global_options = 0x00000022; 
    for (int index = 1; index < argc; index++) {
        char *flag = *(argv+index);
        debug("index %i ,%s", index, flag);
        // certain conditional to watch    
        // -i and -o can appear in either order
        // -i and -o can only appear at most once each
        if (equal(flag, "-i")) {
            if (input || io_search_done) {
                global_options = 0;
                return -1; 
            }
            input = 1;
            index++;
            flag = *(argv+index);
            debug("input: index %i ,%s", index, flag);
            if (equal(flag, "pgm")) 
                global_options = (global_options & 0xFFFFFFF0) + 0x1;
            else if (equal(flag, "birp")) 
                global_options = (global_options & 0xFFFFFFF0) + 0x2;
            else 
                return -1;
            
            // -i pgm|birp default birpx
                // -i is bits 0-3 0x1234567_ 
                    // 0000 (0x0) is not allowed
                    // 0001 (0x1) for pgm
                    // 0010 (0x2) for birp
        } else if (equal(flag, "-o")) {
            if (output || io_search_done) {
                global_options = 0;
                return -1;
            }
            output = 1;
            index++;
            flag = *(argv+index);
            debug("input: index %i ,%s", index, flag);
            if (equal(flag, "pgm")) 
                global_options = (global_options & 0xFFFFFF0F) + 0x10;
            else if (equal(flag, "birp")) 
                global_options = (global_options & 0xFFFFFF0F) + 0x20;
            else if (equal(flag, "ascii"))
                global_options = (global_options & 0xFFFFFF0F) + 0x30;
            else 
                return -1;
            // -o pgm|birp|ascii default birp
                // -o is bits 4-8 0x123456_8
                    // 0000 (0x0) is not allowed
                    // 0001 (0x1) for pgm
                    // 0010 (0x2) for birp
                    // 0011 (0x3) for ascii 
        } else {
            io_search_done = 1;
            if (global_options == 0x22) {
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
                if (equal(flag, "-n"))
                    global_options = (global_options & 0xFFFFF0FF) + 0x100;
                else if (equal(flag, "-t")) {
                    global_options = (global_options & 0xFFFFF0FF) + 0x200;
                    index++;
                    flag = *(argv+index);  
                    debug("transform: index %i ,%s", index, flag);
                    if (len(flag) > 3) {
                        global_options = 0;
                        return -1;
                    }
                    int flag_value = string_to_int(flag, 0, 255);
                    if (flag_value != -1) {
                        global_options =  (global_options & 0xFF00FFFF) + (flag_value << 16);
                    } else {
                        global_options = 0;
                        return -1;
                    }    
                } else if (equal(flag,"-z")) {
                    global_options = (global_options & 0xFFFFF0FF) + 0x300;
                    index++;
                    flag = *(argv+index);
                    debug("transform: index %i ,%s", index, flag);
                    if (len(flag) > 2) {
                        global_options = 0;
                        return -1;
                    }
                    int flag_value = string_to_int(flag, 0, 16);
                    if (flag_value != -1) {
                        global_options =  ((global_options & 0xFF00FFFF) + (-flag_value << 16)) & 0xFFFFFF;
                    } else {
                        global_options = 0;
                        return -1;
                    }
                } else if (equal(flag, "-Z")) {
                    global_options = (global_options & 0xFFFFF0FF) + 0x300;
                    index++;
                    flag = *(argv+index);
                    debug("transform: index %i ,%s", index, flag);
                    if (len(flag) > 2) {
                        global_options = 0;
                        return -1;
                    }
                    int flag_value = string_to_int(flag, 0, 16);
                    if (flag_value != -1) {
                        global_options =  (global_options & 0xFF00FFFF) + (flag_value << 16);
                    } else {
                        global_options = 0;
                        return -1;
                    }
                } else if (equal(flag, "-r"))
                    global_options = (global_options & 0xFFFFF0FF) + 0x400;
                else {
                    global_options = 0;
                    return -1;
                }
                // value for transformation is bits 16-23 0x12__5678
                    // negative for -z (twos complement) [0,16]
                    // positive for -Z (twos complement) [0,16]
                    // positive for -t (unsigned) [0,255]
            } else {
                global_options = 0;
                return -1;
            }
        }
    }


    return 0;
}
