#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_registry.h"
#include "debug.h"
#include "globals.h"
#include "mailbox.h"
#include "protocol.h"
#include "server.h"


void discard_hook(MAILBOX_ENTRY *entry) {
    // add bounce notice to the mailbox
}

void *chla_mailbox_service(void *arg) {
    CLIENT *client = client_ref((CLIENT *)arg, "for reference being retained by mailbox service thread");
    MAILBOX *mailbox = client_get_mailbox(client, 0);
    MAILBOX_ENTRY *entry;

    // if mailbox is null???? exit

    mb_set_discard_hook(mailbox, discard_hook);
    while ((entry = mb_next_entry(mailbox)) != NULL) {
        CHLA_PACKET_HEADER packet;
        if (entry->type == MESSAGE_ENTRY_TYPE) {
            packet.msgid = entry->content.message.msgid;
            struct timespec timestamp;
            if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) { // error
                goto bad_message;
            }
            packet.payload_length = htonl(entry->content.message.length);
            packet.timestamp_sec = htonl(timestamp.tv_sec);
            packet.timestamp_nsec = htonl(timestamp.tv_nsec);
            int status = client_send_packet(client, &packet, entry->content.message.body);
            if (status == 0) {
                mb_add_notice(entry->content.message.from, RRCPT_NOTICE_TYPE, entry->content.message.msgid);
            } else {
                bad_message:
                mb_add_notice(entry->content.message.from, BOUNCE_NOTICE_TYPE, entry->content.message.msgid);
            }
        } else { // notice entry type
            packet.msgid = entry->content.notice.msgid;
            struct timespec timestamp;
            if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) { // error
                goto bad_message;
            }
            packet.payload_length = 0;
            packet.timestamp_sec = htonl(timestamp.tv_sec);
            packet.timestamp_nsec = htonl(timestamp.tv_nsec);
            if (entry->content.notice.type == BOUNCE_NOTICE_TYPE) {
                packet.type = CHLA_BOUNCE_PKT;
            } else {
                packet.type = CHLA_RCVD_PKT;
            }
            client_send_packet(client, &packet, NULL);
        }
        free(entry);
    }
    mb_unref(mailbox, "for reference being discarded by terminating mailbox service");
    client_unref(client, "for reference being discarded by terminating mailbox service thread");
    pthread_exit(NULL);
}

void *chla_client_service(void *arg) {
    CHLA_PACKET_HEADER packet, sender;
    pthread_t mailbox_tid;
    void *payload;
    int connfd = *((int *)arg);
    free(arg);

    success("Starting client service for fd: %d", connfd);
    CLIENT *self = creg_register(client_registry, connfd);
    
    int status; // for login and logout
    int end; // for error handling
    while ((proto_recv_packet(connfd, &packet, &payload)) != -1) {
        switch (packet.type) {
            case CHLA_LOGIN_PKT:
                success("LOGIN");
                char *handle, *saved_pointer;
                handle = strtok_r(payload, "\r\n", &saved_pointer);
                // info("|%s,%s|%ld|%s|", (char *)payload, handle, strlen(saved_pointer), saved_pointer);
                // parse string payload
                status = client_login(self, handle);
                if (status == 0) { // success
                    client_send_ack(self, packet.msgid, NULL, 0);
                    success("Starting mailbox service for: %s (fd=%d)", user_get_handle(client_get_user(self, 1)), connfd);
                    pthread_create(&mailbox_tid, NULL, chla_mailbox_service, self);
                } else {
                    client_send_nack(self, packet.msgid);
                }
                free(payload);
                break;
            
            case CHLA_LOGOUT_PKT:
                success("LOGOUT");
                status = client_logout(self);
                if (status == 0) {
                    client_send_ack(self, packet.msgid, NULL, 0);
                    success("Waiting for mailbox service thread (%ld) to terminate", mailbox_tid);
                } else {
                    client_send_nack(self, packet.msgid);
                }
                free(payload);
                break;

            case CHLA_USERS_PKT:
                success("USERS");
                CLIENT **clients = creg_all_clients(client_registry);
                int payload_length = 0;
                char *send_payload;
                USER *temp;
                for (int i = 0; clients[i] != NULL; i++) {
                    // get length of handle + 2 for \r\n
                    temp = client_get_user(clients[i], 1);
                    if (temp != NULL)
                        payload_length += strlen(user_get_handle(temp)) + 2;
                }
                if (payload_length > 0) {
                    send_payload = malloc(payload_length+1);
                    for (int i = 0; clients[i] != NULL; i++) {
                        // copy handles
                        temp = client_get_user(clients[i], 1);
                        if (temp != NULL) {
                            strcat(send_payload, user_get_handle(temp));
                            strcat(send_payload, "\r\n");
                        }
                        client_unref(clients[i], "for reference in clients list being discarded");
                    }
                    free(clients);
                    client_send_ack(self, packet.msgid, payload_length, send_payload);
                } else {
                    client_send_ack(self, packet.msgid, NULL, 0);
                }
                free(send_payload);
                break;

            case CHLA_SEND_PKT:
                /* code */
                success("SEND");
                // add message
                    // client_send_ack
                // free payload
                break;
        }
        if (end == 1) {

        }
    }

    // logout client
    // pthread t
    // pthread join if the mailbox is started
    // client unref
    // creg unregister
    // pthread exit
    return NULL;
}