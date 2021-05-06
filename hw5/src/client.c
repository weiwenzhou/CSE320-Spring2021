#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h"
#include "debug.h"

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

CLIENT *client_create(CLIENT_REGISTRY *creg, int fd) {
    CLIENT *client = malloc(sizeof(CLIENT));
    if (client == NULL) // error
        return NULL;
    client->fd = fd;
    client->status = NO_USER;
    client->user = NULL;
    client->mailbox = NULL;
    client->referenceCount = 1;
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) { // error
        free(client);
        return NULL;
    }
    if ((errno = pthread_mutex_init(mutex, NULL)) != 0) { // error
        free(client);
        free(mutex);
        return NULL;
    }
    client->mutex = mutex;
    info("Increase reference count on client %p [%d] (0 -> 1) for newly created user", client, client->fd);
    return client;
}

CLIENT *client_ref(CLIENT *client, char *why) {
    pthread_mutex_lock(client->mutex);
    info("Increase reference count on client %p [%d] (%d -> %d) %s", client, client->fd, client->referenceCount, client->referenceCount+1, why);
    client->referenceCount++;
    pthread_mutex_unlock(client->mutex);
    return client;
}

void client_unref(CLIENT *client, char *why) {
    pthread_mutex_lock(client->mutex);
    info("Decrease reference count on client %p [%d] (%d -> %d) %s", client, client->fd, client->referenceCount, client->referenceCount-1, why);
    client->referenceCount--;
    pthread_mutex_unlock(client->mutex);
    if (client->referenceCount == 0) {
        if (client->status == LOGGED_IN) {
            user_unref(client->user, "freeing client");
            mb_unref(client->mailbox, "freeing client");
        }
        free(client->mutex);
        free(client);
    }
}

int client_login(CLIENT *client, char *handle) {
    // check if client is logged in 
    // yes -> return -1
    // else 
    // ureg_register the user with the handle
    // create a mailbox
    return 0;
}

int client_logout(CLIENT *client) {
    return 0;
}

USER *client_get_user(CLIENT *client, int no_ref) {
    // check if client is logged in
    // if yes then check no_ref if nonzero don't increment
    return client->user;
}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref) {
    // check if client is logged in
    // if yes then check no_ref if nonzero don't increment
    return client->mailbox;
}

int client_get_fd(CLIENT *client) {
    // fd is read only so no need to lock
    return client->fd;
}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data) {
    pthread_mutex_lock(user->mutex);
    proto_send_packet(client_get_fd(user), pkt, data);
    pthread_mutex_unlock(user->mutex);
    return 0;
}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen) {
    // lock client
    // malloc CHLA_PACKET_HEADER
    // set type to be ACK
    // set payload_length (htonl)
    // set msgid (htonl)
    // get timestamp in seconds and nanoseconds (htonl)
    // call client_send_packet
    // unlock client
    return 0;
}

int client_send_nack(CLIENT *client, uint32_t msgid) {
    // lock client
    // malloc CHLA_PACKET_HEADER
    // set type to be NACK
    // set payload_length (htonl)
    // set msgid (htonl)
    // get timestamp in seconds and nanoseconds (htonl)
    // call client_send_packet
    // unlock client
    return 0;
}