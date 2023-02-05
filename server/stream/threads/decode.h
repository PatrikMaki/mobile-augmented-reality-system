#pragma once
/*
 * Definitions for MARS Stream processing decode thread
 */
#include <pthread.h>
#include "receive.h"

typedef struct stream_decode_s {
    int socket;
    pthread_t thread;
    int running;
    packet_t* (*getPacket)(struct stream_decode_s* stream);
    void (*close)(struct stream_decode_s* stream);
    stream_receive_t* receiver;
    #ifdef USE_PACKETS_V1
    packet_queue_t queue;
    pthread_mutex_t lock;
    #else
    packets_t queue;
    #endif
} stream_decode_t;


//static packet_t* get_raw_packet(stream_decode_t *stream);

//static void *stream_decode_thread(void *vargp);

//void close_decode(stream_decode_t **stream_decode_pointer_pointer);

stream_decode_t* init_decode(stream_receive_t *receiver_pointer, const char* type);
