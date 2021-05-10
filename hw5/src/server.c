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

void chla_mailbox_service(void *arg) {
    // get client (increase ref)
    // get mailbox of client client_get_mailbox(,0)

    // if mailbox is null???? exit

    // set discard hook
    // while loop with mb_next entry until null
        // packet header 
        // message entry type or notice entry type
            // message send to client_send_packet
            // send ack to sender
            
            // notice entry type 
            // BOUNCE_NOTICE_TYPE
                // send bounce to sender
            // RRCPT_NOTICE_TYPE
                // send to sender   

    // terminate
    // unref client
    // mb unref 
    // pthread exit
}

void *chla_client_service(void *arg) {
    CHLA_PACKET_HEADER packet, sender;
    void *payload;
    int connfd = *((int *)arg);
    free(arg);

    CLIENT *self = creg_register(client_registry, connfd);
    
    int end; // for error handling
    while ((proto_recv_packet(connfd, &packet, &payload)) != -1) {
        switch (packet.type) {
            case CHLA_LOGIN_PKT:
                /* code */
                success("LOGIN");
                // parse string payload
                // client_login()
                    // if sucessful
                    // pthread_create(client) for mailbox thread
                    // send ack to client
                // else send nack
                // free payload
                break;
            
            case CHLA_LOGOUT_PKT:
                /* code */
                success("LOGOUT");
                // client logout
                    // sucess ack
                    // fail nack
                // free payload
                break;

            case CHLA_USERS_PKT:
                success("USERS");
                CLIENT **clients = creg_all_clients(client_registry);
                // while loop
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
                    send_payload = malloc(payload_length-1);
                    for (int i = 0; clients[i] != NULL; i++) {
                        // copy handles
                        temp = client_get_user(clients[i], 1);
                        if (temp != NULL) {
                            if (i != 0)
                                strcat(send_payload, "\r\n");
                            strcat(send_payload, user_get_handle(temp));
                        }
                        client_unref(clients[i], "for reference in clients list being discarded");
                    }
                    free(clients);
                    sender.payload_length = payload_length-1;
                } else {
                    sender.payload_length = 0;
                }
                sender.msgid = packet.msgid;
                struct timespec timestamp;
                if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) { // error
                    end = 1;
                    break;
                }
                sender.timestamp_sec = htonl(timestamp.tv_sec);
                sender.timestamp_nsec = htonl(timestamp.tv_nsec);
                sender.type = CHLA_USERS_PKT;
                client_send_packet(self, &sender, send_payload);
                free(send_payload);
                break;

            case CHLA_SEND_PKT:
                /* code */
                success("SEND");
                // client_send_packet()
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