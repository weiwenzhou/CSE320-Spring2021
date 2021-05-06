#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "client_registry.h"

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
    return NULL;
}

CLIENT *client_ref(CLIENT *client, char *why) {
    return client;
}

void client_unref(CLIENT *client, char *why) {

}

int client_login(CLIENT *client, char *handle) {
    return 0;
}

int client_logout(CLIENT *client) {
    return 0;
}

USER *client_get_user(CLIENT *client, int no_ref) {
    return client->user;
}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref) {
    return client->mailbox;
}

int client_get_fd(CLIENT *client) {
    return client->fd;
}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data) {
    return 0;
}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen) {
    return 0;
}

int client_send_nack(CLIENT *client, uint32_t msgid) {
    return 0;
}