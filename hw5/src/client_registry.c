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
                return client_ref(client, "for pointer being returned by creg_register()");
            }
        }
    }
    pthread_mutex_unlock(cr->mutex);
    return NULL;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client) {
    pthread_mutex_lock(cr->mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cr->clients[i] == client) {
            client_unref(cr->clients[i], "because client is being unregistered");
            cr->clients[i] = NULL;
            cr->count--;
            pthread_mutex_unlock(cr->mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(cr->mutex);
    return -1;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr) {
    pthread_mutex_lock(cr->mutex);
    CLIENT **clients = calloc(cr->count+1, sizeof(CLIENT *));
    if (clients == NULL) {
        pthread_mutex_unlock(cr->mutex);
        return NULL;
    }
    int client_count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cr->clients[i] != NULL) {
            clients[client_count] = client_ref(cr->clients[i], "for reference being added to clients list");
            client_count++;
        }
        if (client_count == cr->count)
            break;
    }
    clients[cr->count] = NULL;
    pthread_mutex_unlock(cr->mutex);
    return clients;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr) {

}