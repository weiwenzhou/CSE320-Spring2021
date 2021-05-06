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
    // malloc a user registry
    // malloc a mutex
    // init mutex
    // have prev and next point to itself
    // user is NULL;
    return NULL;
}

void ureg_fini(USER_REGISTRY *ureg) {
    // for each USER_REGISTRY 
}

USER *ureg_register(USER_REGISTRY *ureg, char *handle) {
    // check if the handle is register
    // if yes then return the user
    // else
    // create a new user
    // create a new user registry and add to the end of the list

    // do for both cases
    // increase user reference count for the USER * returned
    return NULL;
}

void ureg_unregister(USER_REGISTRY *ureg, char *handle) {
    // check if the handle is register
    // if yes then
    // decrement ref count on user and remove user_registry

}