#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "debug.h"
#include "server.h"
#include "globals.h"

static void terminate(int);

/**
 * A SIGHUP signal handler that terminates the program with a successful status.
 */
void sighup_handler(int sig) {
    terminate(EXIT_SUCCESS);
}

/*
 * "Charla" chat server.
 *
 * Usage: charla <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char optval;
    char *leftover;
    int port;
    if (optind >= argc) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    do {
        optval = getopt(argc, argv, "p:");
        switch (optval) {
            case 'p':
                port = strtol(optarg, &leftover, 10);
                if (strlen(leftover) != 0 || port <= 0 || port > 65535) {
                    fprintf(stderr, "Invalid port: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;

            default: // includes '?' invalid option and ':' missing port
                fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    } while (optind < argc);
    // Perform required initializations of the client_registry and
    // player_registry.
    user_registry = ureg_init();
    client_registry = creg_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function charla_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    
    // install SIGHUP handler
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sig_act, 0) == -1) {
        perror("Fail to install SIGHUP handler");
        terminate(EXIT_FAILURE);
    }

    // set up server socket
    int socket_fd, opt, *connfdp;
    struct sockaddr_in address;
    pthread_t tid;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // create socket fd
        perror("Socket failed");
        terminate(EXIT_FAILURE);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(int)) == -1) { // prevent Address already in use
        perror("Setsockopt failed");
        terminate(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) { // bind socket to address, port
        perror("Bind failed");
        terminate(EXIT_FAILURE);
    }

    if (listen(socket_fd, 4) == -1) { // listen on socket
        perror("Listen failed");
        terminate(EXIT_FAILURE);
    }

    while (1) {
        connfdp = malloc(sizeof(int));
        if (connfdp == NULL) {
            perror("Malloc failed");
            terminate(EXIT_FAILURE);
        }
        *connfdp = accept(socket_fd, NULL, NULL);
        if (*connfdp == -1) {
            perror("Accept failure");
            terminate(EXIT_FAILURE);
        }
        pthread_create(&tid, NULL, chla_client_service, connfdp);
    }

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}
