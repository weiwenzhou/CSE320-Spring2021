#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h"
#include "debug.h"
#include "globals.h"
#include "user_registry.h"

extern char *packet_names[];

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
    if (client->status == LOGGED_IN)
        return -1;
    USER *user = ureg_register(user_registry, handle);
    if (user == NULL) // error
        return -1;
    // check if the handle is unique
    int already_exists = 0;
    CLIENT **clients = creg_all_clients(client_registry);
    for (int i = 0; clients[i] != NULL; i++) {
        if (clients[i]->status == LOGGED_IN)
            if (user == client_get_user(clients[i], 1)) {
                warn("User %s is already logged in", handle);
                already_exists = 1;
            }
        client_unref(client, "for reference in clients list being discarded");
    }
    if (already_exists) { // unref because user_registry entry is not new.
        user_unref(user, "because login could not be completed");
        return -1;
    }
    MAILBOX *mailbox = mb_init(handle);
    if (mailbox == NULL) { // error
        ureg_unregister(user_registry, handle);
        return -1;
    }
    pthread_mutex_lock(client->mutex);
    client->status = LOGGED_IN;
    client->user = user;
    client->mailbox = mailbox;
    pthread_mutex_unlock(client->mutex);
    return 0;
}

int client_logout(CLIENT *client) {
    if (client->status == NO_USER) // not logged in
        return -1;
    pthread_mutex_lock(client->mutex);
    info("Log out client %p", client);
    mb_shutdown(client_get_mailbox(client, 1));
    mb_unref(client_get_mailbox(client, 1), "for reference being removed from now-logged-out client");
    user_unref(client_get_user(client, 1), "for reference being removed from now-logged-out client");
    client->status = NO_USER;
    client->mailbox = NULL;
    client->user = NULL;
    pthread_mutex_unlock(client->mutex);
    return 0;
}

USER *client_get_user(CLIENT *client, int no_ref) {
    if (client->status == NO_USER)
        return NULL;
    if (no_ref == 0)
        user_ref(client->user, "for reference being retained by client_get_user()");
    return client->user;
}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref) {
    // check if client is logged in
    if (client->status == NO_USER)
        return NULL;
    if (no_ref == 0)
        mb_ref(client->mailbox, "for reference being retained by client_get_mailbox()");
    return client->mailbox;
}

int client_get_fd(CLIENT *client) {
    // fd is read only so no need to lock
    return client->fd;
}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data) {
    // pkt is network byte order
    int status;
    pthread_mutex_lock(user->mutex);
    info("Send packet (clientfd=%d, type=%s) for client %p", client_get_fd(user), packet_names[pkt->type], user);
    status = proto_send_packet(client_get_fd(user), pkt, data);
    pthread_mutex_unlock(user->mutex);
    return status;
}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen) {
    CHLA_PACKET_HEADER *header = malloc(sizeof(CHLA_PACKET_HEADER));
    if (header == NULL) // error
        return -1;
    header->type = CHLA_ACK_PKT;
    header->payload_length = htonl(datalen);
    header->msgid = htonl(msgid);
    struct timespec timestamp;
    if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) { // error
        free(header);
        return -1;
    }
    header->timestamp_sec = htonl(timestamp.tv_sec);
    header->timestamp_nsec = htonl(timestamp.tv_nsec);
    int status = client_send_packet(client, header, data);
    free(header);
    return status;
}

int client_send_nack(CLIENT *client, uint32_t msgid) {
    CHLA_PACKET_HEADER *header = malloc(sizeof(CHLA_PACKET_HEADER));
    if (header == NULL) // error
        return -1;
    header->type = CHLA_NACK_PKT;
    header->payload_length = 0;
    header->msgid = htonl(msgid);
    struct timespec timestamp;
    if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) { // error
        free(header);
        return -1;
    }
    header->timestamp_sec = htonl(timestamp.tv_sec);
    header->timestamp_nsec = htonl(timestamp.tv_nsec);
    int status = client_send_packet(client, header, NULL);
    free(header);
    return status;
}