#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
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
    pthread_mutex_lock(mb->mutex);
    mb->hook = func;
    pthread_mutex_unlock(mb->mutex);
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
        MAILBOX_QUEUE *head, *current;
        MAILBOX_ENTRY *item;
        head = mb->box;
        current = mb->box->next;
        while (current != head) {
            item = current->content;
            if (mb->hook != NULL) {
                mb->hook(item);
            }
            free(item);
            current = current->next;
            free(current->prev);
        }
        free(mb->box); 
        free(mb->store);
        free(mb);
    }
}

void mb_shutdown(MAILBOX *mb) {
    pthread_mutex_lock(mb->mutex);
    mb->status = DEFUNCT;
    sem_post(mb->store);
    pthread_mutex_unlock(mb->mutex);
}

char *mb_get_handle(MAILBOX *mb) {
    // read only handle
    return mb->handle;
}

void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length) {
    // ignore errors because behavior is undefined in program
    pthread_mutex_lock(mb->mutex);
    MAILBOX_ENTRY *entry = malloc(sizeof(MAILBOX_ENTRY));
    entry->type = MESSAGE_ENTRY_TYPE;
    entry->content.message.msgid = msgid;
    entry->content.message.from = from;
    entry->content.message.body = body;
    entry->content.message.length = length;
    if (mb != from)
        mb_ref(from, "for newly created message with mailbox as sender"); 
    MAILBOX_QUEUE *item = malloc(sizeof(MAILBOX_QUEUE));
    item->content = entry;
    item->next = mb->box;
    item->prev = mb->box->prev;
    mb->box->prev->next = item;
    mb->box->prev = item;
    sem_post(mb->store);
    pthread_mutex_unlock(mb->mutex);
}

void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid) {
    // ignore errors because behavior is undefined in program
    pthread_mutex_lock(mb->mutex);
    MAILBOX_ENTRY *entry = malloc(sizeof(MAILBOX_ENTRY));
    entry->type = NOTICE_ENTRY_TYPE;
    entry->content.notice.type = ntype;
    entry->content.notice.msgid = msgid;
    MAILBOX_QUEUE *item = malloc(sizeof(MAILBOX_QUEUE));
    item->content = entry;
    item->next = mb->box;
    item->prev = mb->box->prev;
    mb->box->prev->next = item;
    mb->box->prev = item;
    sem_post(mb->store);
    pthread_mutex_unlock(mb->mutex);
}

MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb) {
    // sem_wait for semaphore
    sem_wait(mb->store);
    pthread_mutex_lock(mb->mutex);
    if (mb->status == ACTIVE) {
        MAILBOX_QUEUE *item = mb->box->next;
        item->next->prev = mb->box;
        mb->box->next = item->next;
        if (item->content->type == MESSAGE_ENTRY_TYPE) {
            if (mb != item->content->content.message.from)
                mb_unref(item->content->content.message.from, "for reference to sender's mailbox held by message being removed"); 
        }
        MAILBOX_ENTRY *entry = item->content;
        free(item);
        pthread_mutex_unlock(mb->mutex);
        return entry;
    } 
    pthread_mutex_unlock(mb->mutex);
    return NULL;
}