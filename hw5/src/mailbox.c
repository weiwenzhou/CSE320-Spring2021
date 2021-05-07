#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mailbox.h"

typedef enum {
    ACTIVE, DEFUNCT
} MAILBOX_STATUS;

typedef struct mailbox_queue {
    MAILBOX_ENTRY *content;
    MAILBOX_QUEUE *next;
    MAILBOX_QUEUE *prev;
} MAILBOX_QUEUE;

typedef struct mailbox {
    char *handle;
    MAILBOX_DISCARD_HOOK *hook;
    MAILBOX_STATUS status;
    MAILBOX_QUEUE box; // holds the 
    int referenceCount; // number of references
    pthread_mutex_t *mutex; // mutex for thread-safe operations
    sem_t *store; // for producer/consumer model
} MAILBOX;