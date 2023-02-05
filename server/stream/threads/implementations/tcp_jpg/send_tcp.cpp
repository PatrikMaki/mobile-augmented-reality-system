#include "send_tcp.h"
#include "../../../stream.h"

/**
 * Initialize the TCP Send thread. Does nothing as the receive socket is used for sending.
 */
int init_send_tcp(stream_send_t *stream) {
    return 1;
}

/**
 * Send a MARS Packet containg a frame with metadata
 */
void send_packet_tcp(stream_send_t *stream, packet_t *packet_pointer) {
    // Encode first the message header
    char hexbuffer[FRAME_H_LEN+1] = { 0};
    snprintf(hexbuffer, sizeof(hexbuffer), "img2 %016lx %016lx %016lx %08lx",
        packet_pointer->frame.frameCount, 
        packet_pointer->frame.timeStart, 
        packet_pointer->frame.processTime, 
        packet_pointer->frame.size_of_data
        );
    //printf("sending header: %s\n", hexbuffer);
    ssize_t a = write(packet_pointer->sockfd, hexbuffer, FRAME_H_LEN);
    if (a!=FRAME_H_LEN) {
        printf("Error: did not write all hexbuffer\n");
    }
    //printf("size of image %ld\n", packet_pointer->frame.size_of_data);
    ssize_t b = write(packet_pointer->sockfd, packet_pointer->frame.data, packet_pointer->frame.size_of_data);
    if (b!=packet_pointer->frame.size_of_data) {
        // TODO: we should use a similar function what the readbytes is but for writebytes
        // to guarantee that all bytes are send in case system buffers are running out and the write returns
        // less bytes than it was expected to write.
        printf("Error: did not write all packet \n");
    }
}
