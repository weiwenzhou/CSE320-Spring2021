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
    mb_add_notice(entry->content.message.from, BOUNCE_NOTICE_TYPE, 0);
}

void *chla_mailbox_service(void *arg) {
    CLIENT *client = client_ref((CLIENT *)arg, "for reference being retained by mailbox service thread");
    MAILBOX *mailbox = client_get_mailbox(client, 0);
    MAILBOX_ENTRY *entry;

    // if mailbox is null???? exit

    mb_set_discard_hook(mailbox, discard_hook);
    while ((entry = mb_next_entry(mailbox)) != NULL) {
        CHLA_PACKET_HEADER packet;
        memset(&packet, 0, sizeof(CHLA_PACKET_HEADER));
        if (entry->type == MESSAGE_ENTRY_TYPE) {
            packet.msgid = entry->content.message.msgid;
            struct timespec timestamp;
            if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) { // error
                goto bad_message;
            }
            packet.type = CHLA_MESG_PKT;
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
            free(entry->content.message.body);
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
    CHLA_PACKET_HEADER packet;
    pthread_t mailbox_tid = -1;
    void *payload = NULL;
    int connfd = *((int *)arg);
    free(arg);

    success("Starting client service for fd: %d", connfd);
    CLIENT *self = creg_register(client_registry, connfd);
    
    int status; // for login and logout
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
                if (payload != NULL) {
                    free(payload);
                    payload = NULL;
                }
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
                if (payload != NULL) {
                    free(payload);
                    payload = NULL;
                }
                break;

            case CHLA_USERS_PKT:
                success("USERS");
                CLIENT **clients = creg_all_clients(client_registry);
                int payload_length = 0;
                char *send_payload;
                USER *temp = NULL;
                for (int i = 0; clients[i] != NULL; i++) {
                    // get length of handle + 2 for \r\n
                    temp = client_get_user(clients[i], 1);
                    if (temp != NULL)
                        payload_length += strlen(user_get_handle(temp)) + 2;
                }
                if (payload_length > 0) {
                    send_payload = malloc(payload_length+1);
                    memset(send_payload, 0, payload_length+1);
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
                    client_send_ack(self, packet.msgid, send_payload, payload_length);
                } else {
                    client_send_ack(self, packet.msgid, NULL, 0);
                }
                free(send_payload);
                if (payload != NULL) {
                    free(payload);
                    payload = NULL;
                }
                break;

            case CHLA_SEND_PKT:
                /* code */
                success("SEND");
                char *receiver, *body;
                receiver = strtok_r(payload, "\r\n", &body);
                // info("|%s|%s|", receiver, body+1);
                if (client_get_user(self, 1) != NULL) {
                    CLIENT *endpoint = NULL;
                    USER *temp;
                    CLIENT **clients = creg_all_clients(client_registry);
                    for (int i = 0; clients[i] != NULL; i++) {
                        temp = client_get_user(clients[i], 1);
                        if (temp != NULL) {
                            if (strcmp(user_get_handle(temp), receiver) == 0) {
                                endpoint = clients[i];
                            }
                        }
                        client_unref(clients[i], "for reference in clients list being discarded");
                    }
                    free(clients);
                    if (endpoint == NULL) {
                        client_send_nack(self, packet.msgid);
                    } else {
                        // length of user handle + length of message + \r\n + \0
                        // lenght of message = payload - length of receiver -2 (\r\n)
                        int message_length = ntohl(packet.payload_length)-strlen(receiver)-2;
                        int send_message_length = strlen(user_get_handle(client_get_user(self, 1)))+message_length+3;
                        // warn("%d, %d, %d", ntohl(packet.payload_length), message_length, send_message_length);
                        char *send_message = malloc(send_message_length);
                        memset(send_message, 0, message_length); // set first byte to \0
                        strcat(send_message, user_get_handle(client_get_user(self, 1)));
                        strcat(send_message, "\r\n");
                        strncat(send_message, body+1, message_length);
                        mb_add_message(client_get_mailbox(endpoint, 1), packet.msgid, client_get_mailbox(self, 1), send_message, send_message_length);
                        free(send_message);
                        client_send_ack(self, packet.msgid, NULL, 0);
                    }
                } else {
                    client_send_nack(self, packet.msgid);
                }
                if (payload != NULL) {
                    free(payload);
                    payload = NULL;
                }
                break;
        }
    }
    client_logout(self);
    if (mailbox_tid != -1) {
        pthread_join(mailbox_tid, NULL);
    }
    client_unref(self, "for reference being discarded by terminating client service thread");
    creg_unregister(client_registry, self);
    pthread_exit(NULL);
}