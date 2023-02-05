#include "decode_h264.h"
// This is a template for future implementation of H.264 decoder
int init_decode_h264(stream_decode_t *stream) {
    return 0;
}

packet_t* decode(stream_decode_t *stream, packet_t *packet_pointer) {
    //TODO: implement decoder for h264
    packet_t* packet = packet_copy(packet_pointer);
    return packet;
}
