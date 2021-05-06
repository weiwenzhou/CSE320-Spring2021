#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h" 
#include "debug.h"
#include "globals.h"

typedef struct client_registry {
    int count; // number of clients registered
    CLIENT *clients[MAX_CLIENTS]; 
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init() {
    CLIENT_REGISTRY *store = malloc(sizeof(CLIENT_REGISTRY));
    memset(store, 0, sizeof(CLIENT_REGISTRY)); // zero out the array to check for NULL 
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
    pthread_mutex_lock(cr->mutex);
    if (cr->count < MAX_CLIENTS) {
        CLIENT *client = client_create(cr, fd);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (cr->clients[i] == NULL) {
                cr->clients[i] = client;
                cr->count++;
                info("Register client fd %d (total connected: %d)", fd, cr->count);
                pthread_mutex_unlock(cr->mutex);
                return client;
            }
        }
    }
    pthread_mutex_unlock(cr->mutex);
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