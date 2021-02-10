/**
 * Modifies global_option to be 0 and returns -1.
 * 
 * @return -1
 */
int invalidargs_return();

/**
 * Returns the length of a string
 * 
 * @param str the string
 * @return the length of the string
 */ 
int len(char *str);

/**
 * Compares two strings.
 * 
 * @param str1 the first string
 * @param str2 the second string
 * @return 0 if the two string are equal, -1 if string 1 is less than string 2, 1 if string 1 is greater than string 2
 */
int compare(char *str1, char *str2);

/**
 * Checks if two strings are equal.
 * 
 * @param str1 the first string
 * @param str2 the second string
 * @return True (1) if equal, otherwise False (0).
 */
int equal(char *str1, char *str2);

/**
 * Converts a non-negative decimal integer encoded string to a non-negative decimal integer between min and max, inclusive.
 * 
 * @param str The string to be converted
 * @param min The min value of the resulting integer (greater than or equal to 0)
 * @param max The max value of the resulting integer 
 * @return The resulting integer if between min and max, inclusive, otherwise -1.
 */
int string_to_int(char *str, int min, int max);