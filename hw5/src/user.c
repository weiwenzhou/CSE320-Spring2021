#include <pthread.h>
#include <unistd.h>

#include "user.h"

typedef struct user {
    char *handle;
    int referenceCount; // number of reference count
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} USER;

USER *user_create(char *handle) {
    // malloc a user object
    // initialize reference count to 1
    // malloc a buffer for the handle (size of handle + 1)
    // copy the handle
    // malloc a mutex
    // init the mutex
    return NULL; // return the userp
}

USER *user_ref(USER *user, char *why) {
    // lock mutex
    // increment the reference count
    // unlock mutex
    return NULL;
}

void user_unref(USER *user, char *why) {
    // lock the mutex
    // decrement the reference count
    // unlock the mutext
    // if reference count == 0 then free the USER
        // this only occurs when user_registry is being freed so it should
}

char *user_get_handle(USER *user) {
    // user handle is read-only so do not need to lock
    return NULL;
}