#pragma once
/*
 * Definitions for MARS Stream processing receive thread
 */
#include "../packet.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct stream_receive_s {
    int socket;
    pthread_t thread;
    int running;
    packet_t* (*getPacket)(struct stream_receive_s* stream);
    void (*close)(struct stream_receive_s* stream);
    #ifdef USE_PACKETS_V1
    packet_queue_t queue;
    pthread_mutex_t lock;
    #else
    packets_t queue;
    #endif
    int serverfd;
    int port;
} stream_receive_t;


//void receive_stream_with_h264_over_rtp();

//static packet_t* get_h264_rtp_packet(stream_receive_t *stream);
//static void *stream_receive_thread(void *vargp);
//void close_receive_h264_rtp_stream(stream_receive_t *stream);
stream_receive_t* init_receive_stream(int port, const char* stream_type);
