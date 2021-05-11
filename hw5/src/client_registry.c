#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_registry.h" 
#include "debug.h"
#include "globals.h"

typedef struct client_registry {
    int count; // number of clients registered
    CLIENT *clients[MAX_CLIENTS]; 
    pthread_mutex_t *mutex; // mutex for thread-safe operations
    pthread_mutex_t *terminate;  
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
    pthread_mutex_t *terminate = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) { // error
        free(store);
        free(mutex);
        return NULL;
    }
    if ((errno = pthread_mutex_init(terminate, NULL)) != 0) { // error
        free(store);
        free(mutex);
        free(terminate);
        return NULL;
    }
    store->count = 0;
    store->mutex = mutex;
    store->terminate = terminate;
    info("Initialize client registry");
    return store;
}

void creg_fini(CLIENT_REGISTRY *cr) {
    pthread_mutex_lock(cr->mutex);
    info("Finalize client registry");
    pthread_mutex_unlock(cr->mutex);
    free(cr->mutex);
    free(cr->terminate);
    free(cr);
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
                client_ref(client, "for pointer being returned by creg_register()");
                pthread_mutex_unlock(cr->mutex);
                return client;
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
            info("Unregister client fd %d (total connected: %d)", client_get_fd(client), cr->count-1);
            client_unref(cr->clients[i], "because client is being unregistered");
            cr->clients[i] = NULL;
            cr->count--;
            pthread_mutex_unlock(cr->terminate);
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
    pthread_mutex_lock(cr->mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cr->clients[i] != NULL) {
            shutdown(client_get_fd(cr->clients[i]), SHUT_RDWR);
        }
    }
    pthread_mutex_unlock(cr->mutex);
    // block until count is 0
    while (cr->count != 0) {
        pthread_mutex_lock(cr->terminate);
    }
}