#include <pthread.h>
#include <unistd.h>

#include "user.h"

typedef struct user {
    char *handle;
    int referenceCount; // number of reference count
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} USER;

USER *user_create(char *handle) {
    return NULL;
}

USER *user_ref(USER *user, char *why) {
    return NULL;
}

void user_unref(USER *user, char *why) {

}

char *user_get_handle(USER *user) {
    return NULL;
}