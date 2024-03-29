#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
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
    if ((errno = pthread_mutex_init(mutex, NULL)) != 0) { // error
        free(user);
        free(handle_copy);
        free(mutex);
        return NULL;
    }
    user->mutex = mutex;
    info("Increase reference count on user %p [%s] (0 -> 1) for newly created user", user, user->handle);
    return user; 
}

USER *user_ref(USER *user, char *why) {
    // ignore lock and unlock failure (undefined behavior for program)
    pthread_mutex_lock(user->mutex);
    info("Increase reference count on user %p [%s] (%d -> %d) %s", user, user->handle, user->referenceCount, user->referenceCount+1, why);
    user->referenceCount++;
    pthread_mutex_unlock(user->mutex);
    return user;
}

void user_unref(USER *user, char *why) {
    // ignore lock and unlock failure (undefined behavior for program)
    pthread_mutex_lock(user->mutex);
    info("Decrease reference count on user %p [%s] (%d -> %d) %s", user, user->handle, user->referenceCount, user->referenceCount-1, why);
    user->referenceCount--;
    if (user->referenceCount == 0) {
        // this only occurs when user_registry is being freed so it should
        free(user->handle);
        free(user->mutex);
        free(user);
    } else {
        pthread_mutex_unlock(user->mutex);
    }
}

char *user_get_handle(USER *user) {
    // user handle is read-only so do not need to lock
    return user->handle;
}