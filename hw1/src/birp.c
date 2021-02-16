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
    int width, height;
    int code = img_read_pgm(in, &width, &height, raster_data, RASTER_SIZE_MAX);
    if (code)
        return -1;
    BDD_NODE* node = bdd_from_raster(width, height, raster_data);
    if (node == NULL)
        return -1;
    img_write_birp(node, width, height, out);
    return 0;
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width, height;
    BDD_NODE *node = img_read_birp(in, &width, &height);
    if (node == NULL)
        return -1;
    bdd_to_raster(node, width, height, raster_data);
    img_write_pgm(raster_data, width, height, out);
    return 0;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width, height;
    BDD_NODE *node = img_read_birp(in, &width, &height);
    if (node == NULL)
        return -1;
    // reset bdd_index_map
    for (int i = 0; i < BDD_NODES_MAX; i++) {
        *(bdd_index_map+i) = 0;
    }
    int square = 0;
    while (!(width <= 1<<square && height <= 1<<square)) {
        square++;
    }
    square = 1<<square;

    int transformation = (global_options & 0xf00) >> 8;
    switch (transformation) {
        case 1:
            node = bdd_map(node, complement);
            break;
        case 2:
            node = bdd_map(node, cap_pixel);
            break;
        case 4:
            node = bdd_rotate(node, bdd_min_level(square, square));
            width = square;
            height = square;
            break;
    }
    img_write_birp(node, width, height, out);
    return 0;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width, height;
    int code = img_read_pgm(in, &width, &height, raster_data, RASTER_SIZE_MAX);
    if (code)
        return -1;
    raster_to_ascii(out, width, height, raster_data);
    return 0;
}

int birp_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width, height;
    BDD_NODE *node = img_read_birp(in, &width, &height);
    if (node == NULL)
        return -1;
    bdd_to_raster(node, width, height, raster_data);
    raster_to_ascii(out, width, height, raster_data);
    return 0;
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
    // TO BE IMPLEMENTED DONE
    // check if there are flags (argc > 1)
    if (argc <= 1)
        return invalidargs_return();
    // check if the first flag is -h 
    if (equal(*(argv+1), "-h")) {
        global_options = HELP_OPTION;
        return 0;
    }
    // if the first flag is not -h then the max # of flags is 6
    if (argc > 7)
        return invalidargs_return();
    // loop through the flags to check
    int io_search_done = 0;
    int input = 0;
    int output = 0;
    global_options = 0x00000022; // default global_option 0x00000022 (birp birp)
    for (int index = 1; index < argc; index++) {
        char *flag = *(argv+index);
        if (equal(flag, "-i")) {
            index++;
            if (input || io_search_done || index == argc) 
                return invalidargs_return();
            input = 1;
            flag = *(argv+index);
            if (equal(flag, "pgm")) 
                global_options = (global_options & 0xFFFFFFF0) + 0x1;
            else if (equal(flag, "birp")) 
                global_options = (global_options & 0xFFFFFFF0) + 0x2;
            else 
                return invalidargs_return();
        } else if (equal(flag, "-o")) {
            index++;
            if (output || io_search_done || index == argc) 
                return invalidargs_return();
            output = 1;
            flag = *(argv+index);
            if (equal(flag, "pgm")) 
                global_options = (global_options & 0xFFFFFF0F) + 0x10;
            else if (equal(flag, "birp")) 
                global_options = (global_options & 0xFFFFFF0F) + 0x20;
            else if (equal(flag, "ascii"))
                global_options = (global_options & 0xFFFFFF0F) + 0x30;
            else 
                return invalidargs_return();
        } else {
            io_search_done = 1;
            if (global_options != 0x22)
                return invalidargs_return();
            // if input and output format are both birp (global_options == 0x22)
            if (equal(flag, "-n"))
                global_options = (global_options & 0xFFFFF0FF) + 0x100;
            else if (equal(flag, "-t")) {
                global_options = (global_options & 0xFFFFF0FF) + 0x200;
                index++;
                if (index == argc)
                    return invalidargs_return();
                flag = *(argv+index);
                if (len(flag) > 3) 
                    return invalidargs_return();
                int flag_value = string_to_int(flag, 0, 255);
                if (flag_value != -1) 
                    global_options =  (global_options & 0xFF00FFFF) + (flag_value << 16);
                else 
                    return invalidargs_return();
            } else if (equal(flag,"-z")) {
                global_options = (global_options & 0xFFFFF0FF) + 0x300;
                index++;
                if (index == argc)
                    return invalidargs_return();
                flag = *(argv+index);
                if (len(flag) > 2) 
                    return invalidargs_return();
                int flag_value = string_to_int(flag, 0, 16);
                if (flag_value != -1) 
                    global_options =  ((global_options & 0xFF00FFFF) + (-flag_value << 16)) & 0xFFFFFF;
                else 
                    return invalidargs_return();
            } else if (equal(flag, "-Z")) {
                global_options = (global_options & 0xFFFFF0FF) + 0x300;
                index++;
                if (index == argc)
                    return invalidargs_return();
                flag = *(argv+index);
                if (len(flag) > 2) 
                    return invalidargs_return();
                int flag_value = string_to_int(flag, 0, 16);
                if (flag_value != -1)
                    global_options =  (global_options & 0xFF00FFFF) + (flag_value << 16);
                else
                    return invalidargs_return();
            } else if (equal(flag, "-r"))
                global_options = (global_options & 0xFFFFF0FF) + 0x400;
            else
                return invalidargs_return();
        }
    }
    return 0;
}
