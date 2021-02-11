#include "const.h"
#include "bdd.h"
#include "custom_functions.h"

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

int hash(int left, int right) {
    // 1<<21 = 2097152 
    // hash (21 bits approximately) 
        // bits(15-0) (left and right are indices so they will trend towards 0-255)
            // attempt 1 : 8 bits left 8 bits right (doing this first)
            // attempt 2 : sum and take 16 bits
            // attempt 3 : ???
        // bits(20-16)
            // use bits(4-0) of the product of left%256 and right%256
    int result = 0;
    result += (left % (1<<8)) << 8;
    result += (right % (1<<8));
    result += (((left % (1<<8)) * right % (1<<8)) & 0x1f) << 16;

    return result;
}

int equal_node_children(BDD_NODE *node1, BDD_NODE *node2) {
    return node1->left == node2->left && node1->right == node2->right;
}

int equal_node(BDD_NODE *node1, BDD_NODE *node2) {
    return node1->level == node2->level && node1->left == node2->left && node1->right == node2->right;
}

int search_node_map(BDD_NODE *node) {
    int start = hash(node->left, node->right);
    int index = start;
    do {
        BDD_NODE *current = *(bdd_hash_map+index);
        if (null_node(current)) {
            // insert and break
            current->level = node-> level;
            current->left = node->left;
            current->right = node->right;
            return 0;
        } else if (equal_node_children(node, current)) {
            return 1;
        }
        index = (index+1) % BDD_HASH_SIZE;
    } while (index != start);
    return 0;
}

int null_node(BDD_NODE *node) {
    return node->level == 0 && node->left == 0 && node->right == 0;
}