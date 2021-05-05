#include <pthread.h>
#include <unistd.h>

#include "user.h"

typedef struct user {
    char *handle;
    int referenceCount; // number of reference count
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} USER;