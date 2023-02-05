#pragma once
#include <pthread.h>
#include "encode.h"

typedef struct stream_send_s{
    int socket;
    pthread_t thread;
    int running;
    void (*close)(struct stream_send_s* stream);
    stream_encode_t* encoder;
} stream_send_t;


stream_send_t* init_stream_send(stream_encode_t *encoder_pointer, int port, const char* type);
