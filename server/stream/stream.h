#pragma once
// This file defines MARS Project stream top level internal API.

#include <stdio.h> 
#include <stdint.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <pthread.h>
#include <sys/queue.h>

//include other stream headers
#include "threads/receive.h"
#include "threads/decode.h"
#include "threads/inference.h"
#include "threads/encode.h"
#include "threads/send.h"
    
#define UDPPORT     4321 
#define MAXLINE 64*1024

/**
 * Structure holding the threaded packet processors
 */
typedef struct {
    int socket;
    stream_receive_t* receiver;
    stream_decode_t* decoder;
    stream_inference_t* inferencer;
    stream_encode_t* encoder;
    stream_send_t* sender;
    pthread_t thread; // this is for the stream file to combine the above threads
    int running;
} stream_t;

/**
 * Initialize the stream processors for a particular port with the specific configuration.
 */
stream_t* init_stream(int port, const char* config_string);

/**
 * Close a previously initialized stream.
 */
void close_stream(stream_t **stream_pointer_pointer);


#if 0
//OLD code from fist PoC
packet_t* get_stream_packet(stream_t *stream_pointer);
void *receive_stream_with_x(void *vargp); //Old code
int start_stream_with_x(char* request, char* response); //Old code
#endif
