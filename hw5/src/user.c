#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "user.h"

typedef struct user {
    char *handle;
    int referenceCount; // number of reference count
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} USER;

USER *user_create(char *handle) {
    USER *user = malloc(sizeof(USER));
    if (user == NULL) // error
        return NULL;
    user->referenceCount = 1;
    char *handle_copy = malloc(strlen(handle)+1);
    if (handle_copy == NULL) { // error
        free(user);
        return NULL;
    }
    strcpy(handle_copy, handle);
    user->handle = handle_copy;
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) { // error
        free(user);
        free(handle_copy);
        return NULL;
    }
    if (pthread_mutex_init(mutex, NULL) != 0) { // error
        free(user);
        free(handle_copy);
        free(mutex);
        return NULL;
    }
    user->mutex = mutex;
    return user; 
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