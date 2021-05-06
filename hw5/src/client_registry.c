#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h" 
#include "debug.h"
#include "globals.h"

typedef struct client_registry {
    int count; // number of clients registered
    CLIENT *client[MAX_CLIENTS]; 
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init() {
    CLIENT_REGISTRY *store = malloc(sizeof(CLIENT_REGISTRY));
    if (store == NULL) // error
        return NULL;
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) { // error
        free(store);
        return NULL;
    }
    if ((errno = pthread_mutex_init(mutex, NULL)) != 0) { // error
        free(store);
        free(mutex);
        return NULL;
    }
    store->count = 0;
    store->mutex = mutex;
    info("Initialize client registry");
    return store;
}

void creg_fini(CLIENT_REGISTRY *cr) {
    // lock mutex
    // loop through array and free every non null entry until count is 0
    // unlock mutex
}

CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd) {
    // lock mutex
    // if count < max_client
        // create client
        // add to next free spot in array
        // increment count
    // unlock mutex
    return NULL;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client) {
    // lock mutex
    // search for client
    // if found unref and remove from array decrement count
    // else error
    // unlock mutex
    return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr) {
    // lock mutex
    // calloc an array of size count + 1 (for null)
    // client_ref each pointer
    // unlock mutex
    return NULL;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr) {

}