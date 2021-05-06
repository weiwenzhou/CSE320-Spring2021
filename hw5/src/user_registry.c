#include <pthread.h>

#include "user_registry.h"

typedef struct user_registry {
    USER *user;
    pthread_mutex_t mutex; // mutex for thread-safe
    USER_REGISTRY* prev; // links for doubly linked list
    USER_REGISTRY* next;
} USER_REGISTRY;

USER_REGISTRY *ureg_init(void) {
    // initialize the head of doubly linked list
    return NULL;
}

void ureg_fini(USER_REGISTRY *ureg) {

}

USER *ureg_register(USER_REGISTRY *ureg, char *handle) {
    return NULL;
}

void ureg_unregister(USER_REGISTRY *ureg, char *handle) {

}