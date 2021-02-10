#include "custom_functions.h"

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