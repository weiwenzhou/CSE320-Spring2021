#include <pthread.h>

#include "user_registry.h"

typedef struct user_registry {
    USER *user;
    pthread_mutex_t mutex; // mutex for thread-safe
    USER_REGISTRY* prev; // links for doubly linked list
    USER_REGISTRY* next;
} USER_REGISTRY;