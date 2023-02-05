#include "encode_h264.h"
// This is a template for future implementation of H.264 encoder
int init_encode_h264(stream_encode_t *stream) {
    return 0;
}

packet_t* encode(stream_encode_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    return packet;
}
