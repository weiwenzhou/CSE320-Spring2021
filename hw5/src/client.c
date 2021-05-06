#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "mailbox.h"
#include "user.h"

typedef enum {
    NO_USER, LOGGED_IN
} CLIENT_STATUS;

typedef struct client {
    int fd;
    CLIENT_STATUS status; // to keep track if the user is logged in. 
    USER *user;
    MAILBOX *mailbox;
    int referenceCount; // number of references
    pthread_mutex_t *mutex; // mutex for thread-safe operations
} CLIENT;
