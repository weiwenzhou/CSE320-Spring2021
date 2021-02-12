#include <stdlib.h>
#include <stdio.h>

#include "bdd.h"
#include "custom_functions.h"
#include "debug.h"

/*
 * Macros that take a pointer to a BDD node and obtain pointers to its left
 * and right child nodes, taking into account the fact that a node N at level l
 * also implicitly represents nodes at levels l' > l whose left and right children
 * are equal (to N).
 *
 * You might find it useful to define macros to do other commonly occurring things;
 * such as converting between BDD node pointers and indices in the BDD node table.
 */
#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

int bdd_min_level(int w, int h) {
    int width_level = 0;
    int height_level = 0;
    while (w > 1<<width_level) 
        width_level++;
    while (h > 1<<height_level)
        height_level++;
    return width_level+height_level;
}

/**
 * Look up, in the node table, a BDD node having the specified level and children,
 * inserting a new node if a matching node does not already exist.
 * The returned value is the index of the existing node or of the newly inserted node.
 *
 * The function aborts if the arguments passed are out-of-bounds.
 */
int bdd_lookup(int level, int left, int right) {
    // TO BE IMPLEMENTED
    if (level < 0 || level > BDD_LEVELS_MAX)
        return -1;
    if (left < 0 || left >= BDD_NODES_MAX)
        return -1;
    if (right < 0 || right >= BDD_NODES_MAX)
        return -1;
    if (left == right) 
        return left;
    BDD_NODE node = {level, left, right};
    // Check if BDD node with specified level and children exists bdd_hash_map
    int start = hash(node.left, node.right);
    int hash_index = start;
    do {
        BDD_NODE *hash_current = *(bdd_hash_map+hash_index);
        if (hash_current == NULL) {
            for (int index = BDD_NUM_LEAVES; index < BDD_NODES_MAX; index++) {
                BDD_NODE *current = bdd_nodes+index;
                if (null_node(current)) {
                    current->level = level;
                    current->left = left;
                    current->right = right;
                    *(bdd_hash_map+hash_index) = current;
                    return index;
                }
            }
        } else if (equal_node_children(&node, hash_current)) {
            return hash_current - bdd_nodes;
        }
        hash_index = (hash_index+1) % BDD_HASH_SIZE;
    } while (hash_index != start);
    return -1;
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    return NULL;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    // TO BE IMPLEMENTED
    return NULL;
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    return 0;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    return NULL;
}
