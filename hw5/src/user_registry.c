#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "user.h"
#include "user_registry.h"

typedef struct user_registry {
    USER *user;
    pthread_mutex_t *mutex; // mutex for thread-safe
    USER_REGISTRY *prev; // links for doubly linked list
    USER_REGISTRY *next;
} USER_REGISTRY;

USER_REGISTRY *ureg_init(void) {
    // initialize the head of doubly linked list
    USER_REGISTRY *head = malloc(sizeof(USER_REGISTRY));
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if ((errno = pthread_mutex_init(mutex, NULL)) != 0) { // error
        free(head);
        free(mutex);
        return NULL;
    }
    head->mutex = mutex;
    head->prev = head;
    head->next = head;
    head->user = NULL;
    info("Initialize user registry");
    return head;
}

void ureg_fini(USER_REGISTRY *ureg) {
    // for each USER_REGISTRY 
}

USER *ureg_register(USER_REGISTRY *ureg, char *handle) {
    pthread_mutex_lock(ureg->mutex);
    USER_REGISTRY *head, *current;
    head = ureg;
    current = ureg;
    while (current->next != head) {
        current = current->next;
        if (strcmp(user_get_handle(current->user), handle) == 0) { // found handle
            user_ref(current->user, "ureg_register return existing user");
            return current->user;
        }
    }
    USER *user = user_create(handle);
    if (user == NULL) {
        return NULL;
    }
    USER_REGISTRY *new = malloc(sizeof(USER_REGISTRY));
    if (new == NULL) {
        user_unref(user, "ureg_register malloc failed"); // free user
        return NULL;
    }
    new->user = user;
    ureg->prev->next = new;
    new->prev = ureg->prev;
    new->next = ureg;
    ureg->prev = new;
    user_ref(user, "ureg_register return new user");
    pthread_mutex_unlock(ureg->mutex);
    return user;
}

void ureg_unregister(USER_REGISTRY *ureg, char *handle) {
    // check if the handle is register
    // if yes then
    // decrement ref count on user and remove user_registry

}