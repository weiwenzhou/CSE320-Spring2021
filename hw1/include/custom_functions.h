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

/**
 * Returns a hash using level and indices of left and right bdd_nodes
 * 
 * @param left  The index, in the bdd_nodes array, of the left (i.e. "0") child
 * of the BDD node to be looked up.
 * @param right  The index, in the bdd_nodes array, of the right (i.e. "1") child
 * of the BDD node to be looked up.
 * @return 
 */
int hash(int left, int right);

/**
 * Checks if two BDD node have the same left and right values.
 * 
 * @param node1  The first BDD node.
 * @param node2  The second BDD node.
 * @return 1 if equal, otherwise 0.
 */
int equal_node_children(BDD_NODE *node1, BDD_NODE *node2);

/**
 * Checks if two BDD node have the same level, left, and right values.
 * 
 * @param node1  The first BDD node.
 * @param node2  The second BDD node.
 * @return 1 if equal, otherwise 0.
 */
int equal_node(BDD_NODE *node1, BDD_NODE *node2);

/**
 * Checks if a node with level, left, and right values of 0.
 * 
 * @param node The BDD node.
 * @return 1 if True, otherwise 0.
 */
int null_node(BDD_NODE *node);

/**
 * Divides the raster data evenly until we get a single character. A helper method for bdd_from_raster.
 * 
 * @param start_width The starting width position.
 * @param end_width The ending width position.
 * @param start_height The starting height position.
 * @param end_height The ending height position.
 * @param w The width of the raster data.
 * @param h The height of the raster data.
 * @param raster The 2D array containing the raster data.
 * @return index of BDD_NODE for the subset of raster data.
 */
int split_raster_data(int start_width, int end_width, int start_height, int end_height, int w, int h, unsigned char *raster);