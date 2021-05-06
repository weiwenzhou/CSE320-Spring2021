#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h"

typedef struct client_registry {
    CLIENT *client[MAX_CLIENTS]; 
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} CLIENT_REGISTRY;