#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h" 
#include "globals.h"

typedef struct client_registry {
    int count; // number of clients registered
    CLIENT *client[MAX_CLIENTS]; 
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init() {
    return NULL;
}

void creg_fini(CLIENT_REGISTRY *cr) {
    
}

CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd) {
    return NULL;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client) {
    return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr) {
    return NULL;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr) {

}