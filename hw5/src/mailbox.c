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
    struct mailbox_queue *next;
    struct mailbox_queue *prev;
} MAILBOX_QUEUE;

typedef struct mailbox {
    char *handle;
    MAILBOX_DISCARD_HOOK *hook;
    MAILBOX_STATUS status;
    MAILBOX_QUEUE *box; // holds the messages/notices
    int referenceCount; // number of references
    pthread_mutex_t *mutex; // mutex for thread-safe operations
    sem_t *store; // for producer/consumer model
} MAILBOX;

MAILBOX *mb_init(char *handle) {
    MAILBOX *mailbox = malloc(sizeof(MAILBOX));
    if (mailbox == NULL) // error
        return NULL;
    mailbox->referenceCount = 1;
    mailbox->status = ACTIVE;

    char *handle_copy = malloc(strlen(handle)+1);
    if (handle_copy == NULL) { // error
        free(mailbox);
        return NULL;
    }
    strcpy(handle_copy, handle);
    mailbox->handle = handle_copy;

    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) { // error
        free(mailbox);
        free(handle_copy);
        return NULL;
    }
    if ((errno = pthread_mutex_init(mutex, NULL)) != 0) { // error
        free(mailbox);
        free(handle_copy);
        free(mutex);
        return NULL;
    }
    mailbox->mutex = mutex;

    sem_t *store = malloc(sizeof(sem_t));
    if (store == NULL) { // error
        free(mailbox);
        free(handle_copy);
        free(mutex);
        return NULL;
    }
    if (sem_init(store, 0, 0) == -1) { // error
        free(mailbox);
        free(handle_copy);
        free(mutex);
        free(store);
        return NULL;
    }
    mailbox->store = store;
    
    MAILBOX_QUEUE *head = malloc(sizeof(MAILBOX_QUEUE));
    if (head == NULL) {
        free(mailbox);
        free(handle_copy);
        free(mutex);
        free(store);
        return NULL;
    }
    head->content = NULL;
    head->next = head;
    head->prev = head;
    mailbox->box = head;

    return mailbox;
}

void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *func) {

}

void mb_ref(MAILBOX *mb, char *why) {
    pthread_mutex_lock(mb->mutex);
    info("Increase reference count on mailbox %p (%d -> %d) %s", mb, mb->referenceCount, mb->referenceCount+1, why);
    mb->referenceCount++;
    pthread_mutex_unlock(mb->mutex);
}

void mb_unref(MAILBOX *mb, char *why) {
    pthread_mutex_lock(mb->mutex);
    info("Decrease reference count on mailbox %p (%d -> %d) %s", mb, mb->referenceCount, mb->referenceCount-1, why);
    mb->referenceCount--;
    pthread_mutex_unlock(mb->mutex);
    if (mb->referenceCount == 0) {
        free(mb->handle);
        free(mb->mutex);
        free(mb->box); // need to clean up box content
        free(mb->store);
        free(mb);
    }
}

void mb_shutdown(MAILBOX *mb) {
    // lock mutex
    // modify status
    // sem_post for producer for mb_next_entry to check status, free, and terminate
    // unlock mutex
}

char *mb_get_handle(MAILBOX *mb) {
    // read only handle
    return NULL;
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