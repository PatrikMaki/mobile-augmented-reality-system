#include "encode_test1.h"
int init_encode_test1(stream_encode_t *stream) {
    return 0;
}

packet_t* encode_test1(stream_encode_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    return packet;
}