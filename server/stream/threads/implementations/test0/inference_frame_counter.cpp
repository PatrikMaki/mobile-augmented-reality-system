#include "inference_frame_counter.h"
// This is template for future implementation of frame counter which would just inject the frame number to the image

int init_inference_frame_counter(stream_inference_t *stream) {
    return 1;
}

packet_t* inference(stream_inference_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    return packet;
}
