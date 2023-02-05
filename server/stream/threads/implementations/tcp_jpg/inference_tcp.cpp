#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inference_tcp.h"

int init_inference_tcpjpg(stream_inference_t *stream) {
    return 1;
}

packet_t* inference_tcpjpg(stream_inference_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    return packet;
}
