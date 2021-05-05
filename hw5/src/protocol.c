#include "protocol.h"

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload) {
    // get the payload size (convert from network byte order using ntohl)

    // write the header (keep track of partial writes)

    // write the payload (keep track of partial writes)

    return 0;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload) {
    // read the header (keep track of partial reads)

    // get the payload size (convert from network byte order using ntohl)

    // read the payload (keep track of partial reads)

    return 0;
}