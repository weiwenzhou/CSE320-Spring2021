#include "client.h"
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
    CHLA_PACKET_HEADER packet;
    void *payload;
    int connfd = *((int *)arg);
    free(arg);

    creg_register(client_registry, connfd);
    

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
                success("LOGOUT")
                // client logout
                    // sucess ack
                    // fail nack
                // free payload
                break;

            case CHLA_USERS_PKT:
                /* code */
                success("USERS")
                CLIENT **clients = creg_all_clients(client_registry);
                // while loop
                break;

            case CHLA_SEND_PKT:
                /* code */
                success("SEND")
                // client_send_packet()
                    // client_send_ack
                // free payload
                break;
        }
    }

    // logout client
    // pthread t
    // pthread join if the mailbox is started
    // client unref
    // creg unregister
    // pthread exit
}