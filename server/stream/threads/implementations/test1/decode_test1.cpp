#include "decode_test1.h"
int init_decode_test1(stream_decode_t *stream) {
    return 0;
}

packet_t* decode_test1(stream_decode_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    return packet;
}
