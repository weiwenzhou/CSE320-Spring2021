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
    int start = hash(level, left, right);
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
            return -1;
        } else if (equal_node(&node, hash_current)) {
            return hash_current - bdd_nodes;
        }
        hash_index = (hash_index+1) % BDD_HASH_SIZE;
    } while (hash_index != start);
    return -1;
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    // find the min d for w<=2^d and h<=2^d
    if (w <= 0 || w <= 0)
        return NULL;
    int square = 0;
    while (!(w <= 1<<square && h <= 1<<square)) {
        square++;
    }
    square = 1<<square;
    int nodeIndex = split_raster_data(0, square, 0, square, w, h, raster);
    if (nodeIndex == -1)
        return NULL;
    return bdd_nodes+nodeIndex;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    unsigned char *current = raster;
    int level = node->level;
    int width_bound = 1 << (level-(level/2));
    int height_bound = 1 << (level/2);
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            // info("%i %i %i", row, col, bdd_apply(node, row, col));
            *current++ = bdd_apply(node, row % height_bound, col % width_bound);
        }
    }
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    // TO BE IMPLEMENTED
    if (node == NULL)
        return -1;
    int counter = 1;
    bdd_serialize_helper(node, out, &counter);
    return 0;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    // TO BE IMPLEMENTED
    int character;
    int serial = 0;
    while ((character = fgetc(in)) != EOF) {
        if (character == 64) {
            serial++;
            *(bdd_index_map+serial) = fgetc(in);
        } else if (character >= 'A' && character <= '`') {
            // get left
            int left = fgetc(in);
            left += fgetc(in) << 8;
            left += fgetc(in) << 16;
            left += fgetc(in) << 24;
            // get right
            int right = fgetc(in);
            right += fgetc(in) << 8;
            right += fgetc(in) << 16;
            right += fgetc(in) << 24;
            serial++;
            // info("%i %i %i", character-64, left, right);
            character = character - '@';
            *(bdd_index_map+serial) = bdd_lookup(character, *(bdd_index_map+left), *(bdd_index_map+right));
        }
        // debug("%c %o: %i", character, serial, character);
    }
    return bdd_nodes+*(bdd_index_map+serial);
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    int mid;
    BDD_NODE *next;
    BDD_NODE *current = node;
    int level = node->level;
    int left = 0;
    int top = 0;
    int right = 1 << (level-(level/2));
    int bottom = 1 << (level/2);
    if (level == 0)
        return node-bdd_nodes;
    do {
        int current_level = current->level;
        int left_node = current->left;
        int right_node = current->right;
        // warn("l:%i %i", current_level, level);
        // info("%i %i %i: %i-%i, %i-%i", current_level, left_node, right_node, left, right, top, bottom);
        if (level & 1) {
            // if odd split width | 
            mid = (right+left)/2;
            // debug("width split %i vs %i", mid, c);
            if (c < mid) {
                right = mid;
                if (left_node < BDD_NUM_LEAVES && level == current_level)
                    return left_node;
                next = bdd_nodes+left_node;
            } else {
                left = mid;
                if (right_node < BDD_NUM_LEAVES && level == current_level)
                    return right_node;
                next = bdd_nodes+right_node;
            }
        } else {
            // else even split height -
            mid = (bottom+top)/2;
            // debug("height split %i vs %i", mid, r);
            if (r < mid) {
                bottom = mid;
                if (left_node < BDD_NUM_LEAVES && level == current_level)
                    return left_node;
                next = bdd_nodes+left_node;
            } else {
                top = mid;
                if (right_node < BDD_NUM_LEAVES && level == current_level)
                    return right_node;
                next = bdd_nodes+right_node;
            }
        }
        if (current_level == level) 
            current = next;
        level--;
    } while (1);
    
    return 0;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    int level = node->level;
    if (level == 0)
        return bdd_nodes+func(node-bdd_nodes);
    int left = node->left;
    int right = node->right;
    left = bdd_map(bdd_nodes+left, func)-bdd_nodes;
    right = bdd_map(bdd_nodes+right, func)-bdd_nodes;
    int index = bdd_lookup(level, left, right);
    return bdd_nodes+index;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    int node_level = node->level;
    if (node_level == 0) {
        // info("%li", node-bdd_nodes);
        return node;
    }
    if (node_level == level) {
        int top_left, top_right;
        BDD_NODE *top = bdd_nodes+node->left;
        BDD_NODE *bottom = bdd_nodes+node->right;
        if (node->left < BDD_NUM_LEAVES) {
            top_left = node->left;
            top_right = node->left;
        } else {
            top_left = top->left;
            top_right = top->right;
        }
        int bottom_left, bottom_right;
        if (node->right < BDD_NUM_LEAVES) {
            bottom_left = node->right;
            bottom_right = node->right;
        } else {
            bottom_left = bottom->left;
            bottom_right = bottom->right;

        }
        // debug("index %li", node-bdd_nodes);
        int new_top_left, new_top_right, new_top, new_bottom_left, new_bottom_right, new_bottom;
        if (top->level == level-1) 
            new_top_left = bdd_rotate(bdd_nodes+top_right, level-2)-bdd_nodes;
        else 
            new_top_left = bdd_rotate(top, level-2)-bdd_nodes;
        if (bottom->level == level-1)
            new_top_right = bdd_rotate(bdd_nodes+bottom_right, level-2)-bdd_nodes;
        else
            new_top_right = bdd_rotate(bottom, level-2)-bdd_nodes;
        new_top = bdd_lookup(level-1, new_top_left, new_top_right);
        if (top->level == level-1)
            new_bottom_left = bdd_rotate(bdd_nodes+top_left, level-2)-bdd_nodes;
        else
            new_bottom_left = bdd_rotate(top, level-2)-bdd_nodes;
        if (bottom->level == level-1)
            new_bottom_right = bdd_rotate(bdd_nodes+bottom_left, level-2)-bdd_nodes;
        else
            new_bottom_right = bdd_rotate(bottom, level-2)-bdd_nodes;
        new_bottom = bdd_lookup(level-1, new_bottom_left, new_bottom_right);
        return bdd_nodes+bdd_lookup(level, new_top, new_bottom);
    } else if (node_level == level-1) {
        int new_top = bdd_rotate(bdd_nodes+(node->right), level-2)-bdd_nodes;
        int new_bottom = bdd_rotate(bdd_nodes+(node->left), level-2)-bdd_nodes;
        return bdd_nodes+bdd_lookup(level, new_top, new_bottom);
    } else {
        return bdd_rotate(node, level-2);
    }
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    char f = factor;
    f = 2*f;
    if (node->level == 0)
        return node;

    // zoom left and right
    int current_level = node->level;
    int left = node->left;
    int right = node->right;
    if (f < 0 && current_level+f <= 0) {
        // warn("HERE? %i", current_level);
        return (node == bdd_nodes) ? node:(bdd_nodes+255);
    } else {
        // info("%li %i %i %i", node-bdd_nodes, current_level, left, right);
        left = bdd_zoom(bdd_nodes+left, level, factor)-bdd_nodes;
        right = bdd_zoom(bdd_nodes+right, level, factor)-bdd_nodes;
        // debug("%i %i %i %i", current_level+f, left, right, bdd_lookup(current_level+f, left, right));
        return bdd_nodes+bdd_lookup(current_level+f, left, right);
    }
   return NULL;
}
