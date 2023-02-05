#pragma once
/*
 * Definitions for MARS Stream processing inference thread
 */
#include <pthread.h>
#include "decode.h"

typedef struct stream_inference_s{
    int socket;
    pthread_t thread;
    int running;
    packet_t* (*getPacket)(struct stream_inference_s* stream);
    void (*close)(struct stream_inference_s* stream);
    stream_decode_t* decoder;
    #ifdef USE_PACKETS_V1
    packet_queue_t queue;
    pthread_mutex_t lock;
    #else
    packets_t queue;
    #endif
} stream_inference_t;

//void inference_of_raw_frame();

stream_inference_t* init_inference(stream_decode_t *decoder_pointer, const char* type);

//void close_inference(stream_inference_t **stream_inference_pointer_pointer);
