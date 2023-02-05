#pragma once
/*
 * Definitions for MARS Stream processing encode thread
 */
#include <pthread.h>
#include "inference.h"

typedef struct stream_encode_s{
    int socket;
    pthread_t thread;
    int running;
    packet_t* (*getPacket)(struct stream_encode_s* stream);
    void (*close)(struct stream_encode_s* stream);
    stream_inference_t* inferencer;
    #ifdef USE_PACKETS_V1
    packet_queue_t queue;
    pthread_mutex_t lock;
    #else
    packets_t queue;
    #endif
} stream_encode_t;

//void encode_raw_frame_to_h264();

stream_encode_t* init_encode(stream_inference_t *inferer_pointer, const char* type);

//void close_encode(stream_encode_t **stream_encode_pointer_pointer);
