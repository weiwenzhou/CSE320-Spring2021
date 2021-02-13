#include "const.h"
#include "bdd.h"
#include "custom_functions.h"
#include "debug.h"

int invalidargs_return() {
    global_options = 0;
    return -1;
}

int len(char *str) {
    int size = 0;
    while (*(str+size)) 
        size++;
    return size;
}

int compare(char *str1, char *str2) {
    int max_index = len(str1) > len(str2) ? len(str1):len(str2);
    for (int index = 0; index < max_index; index++) {
        char c1 = *(str1+index);
        char c2 = *(str2+index);
        if (c1 == c2) {
            if (c1 == 0) // both characters is the null terminator(0)
                return 0;
        } else if (c1 < c2) {
            return -1;
        } else {
            return 1;
        }
    }
    return 0;
}

int equal(char *str1, char *str2) {
    return compare(str1, str2) == 0;
}

int string_to_int(char *str, int min, int max) {
    int result = 0;
    int length = len(str);
    for (int index = 0; index < length; index++) {
        char digit = *(str+index);
        if (digit >= '0' && digit <= '9')
            result = result * 10 + (digit - '0'); 
        else 
            return -1;
    }
    if (result >= min && result <= max)
        return result;
    else
        return -1;
}

int hash(int level, int left, int right) {
    // hash (20 bits approximately) 1<<21 = 2097152 
        // bits(15-0) (left and right are indices so they will trend towards 0-255)
            // attempt 1 : 8 bits left 8 bits right (doing this first)
        // bits(19-16)
            // use bits(3-0) of 4 bits of level
    int result = 0;
    result += (left % (1<<8)) << 8;
    result += (right % (1<<8));
    result += (level & 0xf) << 16;

    return result;
}

int equal_node_children(BDD_NODE *node1, BDD_NODE *node2) {
    return node1->left == node2->left && node1->right == node2->right;
}

int equal_node(BDD_NODE *node1, BDD_NODE *node2) {
    return node1->level == node2->level && node1->left == node2->left && node1->right == node2->right;
}

int null_node(BDD_NODE *node) {
    return node->level == 0 && node->left == 0 && node->right == 0;
}

int split_raster_data(int start_width, int end_width, int start_height, int end_height, int w, int h, unsigned char *raster) {
    int width = end_width-start_width;
    int height = end_height-start_height;
    int level = bdd_min_level(width, height);
    if (width * height == 1) {
        if (start_height < h && start_width < w)
            return *(raster+start_height*w+start_width);
        return 0;
    } else if (width > height) {
        width = width/2;
        int left = split_raster_data(start_width, start_width+width, start_height, end_height, w, h, raster);
        int right = split_raster_data(start_width+width, end_width, start_height, end_height, w, h, raster);
        return bdd_lookup(level, left, right);
    } else {
        height = height/2;
        int top = split_raster_data(start_width, end_width, start_height, start_height+height, w, h, raster);
        int bottom = split_raster_data(start_width, end_width, start_height+height, end_height, w, h, raster);
        return bdd_lookup(level, top, bottom);
    }
}

int bdd_serialize_helper(BDD_NODE *node, FILE *out, int *counter) {
    if (node->level == 0) {
        return -1;
    } else {
        int left = node->left;
        int right = node->right;
        if (left < BDD_NUM_LEAVES) {
            if (*(bdd_index_map+left) == 0) {
                int character = '@';
                fputc(character, out);
                fputc(left, out);
                *(bdd_index_map+left) = *counter;
                (*counter)++;
            }
        } else {
            bdd_serialize_helper(bdd_nodes+left,out, counter);
        }
        if (right < BDD_NUM_LEAVES) {
            if (*(bdd_index_map+right) == 0) {
                int character = '@';
                fputc(character, out);
                fputc(right, out);
                *(bdd_index_map+right) = *counter;
                (*counter)++;
            }
        } else {
            bdd_serialize_helper(bdd_nodes+right, out, counter);
        }
        char level = '@' + node->level;
        int index = node-bdd_nodes;
        if (*(bdd_index_map+index) == 0) {
            left = *(bdd_index_map+left);
            right = *(bdd_index_map+right);
            fputc(level, out);
            fputc(left & 0xff, out);
            left = left >> 8;
            fputc(left & 0xff, out);
            left = left >> 8;
            fputc(left & 0xff, out);
            left = left >> 8;
            fputc(left & 0xff, out);

            fputc(right & 0xff, out);
            right = right >> 8;
            fputc(right & 0xff, out);
            right = right >> 8;
            fputc(right & 0xff, out);
            right = right >> 8;
            fputc(right & 0xff, out);
            *(bdd_index_map+index) = *counter;
            (*counter)++;
        }
    }

    return 0;
}