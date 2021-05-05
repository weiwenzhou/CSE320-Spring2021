#include <unistd.h>

#include "protocol.h"

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload) {
    uint32_t header_size, payload_size, amount_written;
    // get the payload size (convert from network byte order using ntohl)
    payload_size = ntohl(hdr->payload_length);

    // write the header (keep track of partial writes)
    header_size = sizeof(CHLA_PACKET_HEADER);
    while (header_size != 0) {
        amount_written = write(fd, hdr, header_size);
        if (amount_written <= 0) { // write error or wrote 0 byte 
            return -1;
        }
        header_size -= amount_written;
    }

    // write the payload (keep track of partial writes)
    while (payload_size != 0) {
        amount_written = write(fd, payload, payload_size);
        if (amount_written <= 0) { // write error or wrote 0 bytes
            return -1;
        }
        payload_size -= amount_written;
    }

    return 0;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload) {
    // read the header (keep track of partial reads)

    // get the payload size (convert from network byte order using ntohl)

    // read the payload (keep track of partial reads)

    return 0;
}