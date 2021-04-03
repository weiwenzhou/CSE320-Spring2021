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

/**
 * Defines a new printer, with a given name and file type.
 * 
 * @param name The name of the printer 
 * @param type A pointer to a FILE_TYPE of the file type
 * 
 * @return a newly created printer object with the sepcific name for 
 * the given file type or NULL, if MAX_PRINTERS have been defined.
 */
PRINTER *define_printer(char *name, FILE_TYPE *type);