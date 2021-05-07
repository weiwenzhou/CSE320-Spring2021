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

MAILBOX *mb_init(char *handle) {
    return NULL;
}

void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *func) {

}

void mb_ref(MAILBOX *mb, char *why) {
    // lock mutex
    // info statement
    // increment reference count
    // unlock mutex
}

void mb_unref(MAILBOX *mb, char *why) {
    // lock mutex
    // info statement
    // decrement reference count
    // unlock mutex
    // free mailbox if referenceCount == 0
}

void mb_shutdown(MAILBOX *mb) {
    // lock mutex
    // modify status
    // sem_post for producer for mb_next_entry to check status, free, and terminate
    // unlock mutex
}

char *mb_get_handle(MAILBOX *mb) {
    // read only handle
}

void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length) {
    // lock mutex
    // malloc message
    // if from != mb then mb_ref mailbox to persist
    // malloc mailbox entry
    // malloc mailbox queue
    // adds message to the end of mb->box
    // sem_post semaphore
    // unlock mutex
}

void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid) {
    // lock mutex
    // malloc notice
    // malloc mailbox entry
    // malloc mailbox queue
    // adds message to the end of mb->box
    // sem_post semaphore
    // unlock mutex
}

MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb) {
    // sem_wait for semaphore
    // check mailbox status
    return NULL;
}