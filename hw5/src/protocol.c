#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "protocol.h"

char *packet_names[] = {
    "",
    "login", "logout", "users", "send",
    "ack", "nack", "mesg", "rcvd", "bounce"
};

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload) {
    info("SEND packet: type=%s, msgid=%d, payload_length=%d, payload=[%s]", packet_names[hdr->type], ntohl(hdr->msgid), ntohl(hdr->payload_length), (char *) payload);
    uint32_t header_size, payload_size;
    ssize_t amount_written;
    char *current_buffer;
    // get the payload size (convert from network byte order using ntohl)
    payload_size = ntohl(hdr->payload_length);

    // write the header (keep track of partial writes)
    header_size = sizeof(CHLA_PACKET_HEADER);
    current_buffer = (char *) hdr;
    while (header_size != 0) {
        amount_written = write(fd, current_buffer, header_size);
        if (amount_written <= 0) { // write error or wrote 0 byte 
            return -1;
        }
        header_size -= amount_written;
        current_buffer += amount_written;
    }
    // write the payload (keep track of partial writes)
    current_buffer = payload;
    while (payload_size != 0) {
        amount_written = write(fd, current_buffer, payload_size);
        if (amount_written <= 0) { // write error or wrote 0 bytes
            return -1;
        }
        payload_size -= amount_written;
        current_buffer += amount_written;
    }
    return 0;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload) {
    uint32_t header_size, payload_size;
    ssize_t amount_read;
    void *payload_buffer;
    char *current_buffer;
    // read the header (keep track of partial reads)
    header_size = sizeof(CHLA_PACKET_HEADER);
    current_buffer = (char *) hdr;
    while (header_size != 0) {
        amount_read = read(fd, current_buffer, header_size);
        if (amount_read <= 0) { // write error or wrote 0 byte 
            return -1;
        }
        header_size -= amount_read;
        current_buffer += amount_read;
    }

    // get the payload size (convert from network byte order using ntohl)
    payload_size = ntohl(hdr->payload_length);
    // create a buffer of size payload
    if (payload_size != 0) {
        payload_buffer = malloc(payload_size);
        if (payload_buffer == NULL) { // malloc error
            return -1;
        }
        memset(payload_buffer, 0, payload_size);

        // read the payload (keep track of partial reads)
        current_buffer = payload_buffer;
        while (payload_size != 0) {
            amount_read = read(fd, current_buffer, payload_size);
            if (amount_read <= 0) { // read error or read EOF
                free(payload_buffer);
                return -1;
            }
            payload_size -= amount_read;
            current_buffer += amount_read;
        }
        // put payload buffer in payload pointer
        *payload = payload_buffer;
    }
    info("packet: type=%s, msgid=%d, payload_length=%d, payload=[%s]", packet_names[hdr->type], ntohl(hdr->msgid), payload_size, (char *) *payload);
    return 0;
}