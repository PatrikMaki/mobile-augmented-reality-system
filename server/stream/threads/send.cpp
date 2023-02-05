#include "send.h"
#include "implementations/test0/send_h264_rtp.h"
#include "implementations/test1/send_test1.h"
#include "implementations/tcp_jpg/send_tcp.h"
#include <unistd.h>
#include <string.h>

static void close_send(stream_send_t *stream) {
}

static void *stream_send_thread_0(void *vargp) {
    stream_send_t* stream = (stream_send_t*) vargp;
    init_send(stream);
    while (stream->running) {
        //send:
        packet_t* packet2 = (*stream->encoder->getPacket)(stream->encoder);
        if (packet2==NULL) {
            usleep(1000);
        }else{
            if (packet2->frame.size_of_data == 0) {
                stream->running = false;
            } else {
                send_packet(stream, packet2);
            }
            printf("send %p\n", packet2);
            free(packet2->frame.data);
            free(packet2);
        }
    }
    printf("stream_send_thread_0 done");
    return NULL;
}

static void *stream_send_thread_1(void *vargp) {
    stream_send_t* stream = (stream_send_t*) vargp;
    init_send_test1(stream);
    while (stream->running) {
        //send:
        packet_t* packet2 = (*stream->encoder->getPacket)(stream->encoder);
        if (packet2==NULL) {
            usleep(1000);
        }else{
            if (packet2->frame.size_of_data == 0) {
                stream->running = false;
            } else {
                send_packet_test1(stream, packet2);
                printf("send %p\n", packet2);
            }
            free(packet2->frame.data);
            free(packet2);
        }
    }
    printf("stream_send_thread_1 done");
    return NULL;
}

static void *stream_send_thread_2(void *vargp) {
    stream_send_t* stream = (stream_send_t*) vargp;
    init_send_tcp(stream);
    while (stream->running) {
        //send:
        packet_t* packet2 = (*stream->encoder->getPacket)(stream->encoder);
        if (packet2==NULL) {
            usleep(1000); //this has to be removed
        }else{
            if (packet2->frame.size_of_data == 0) {
                stream->running = false;
            } else {
                send_packet_tcp(stream, packet2);
                //printf("send %p\n", packet2);
            }
            free(packet2->frame.data);
            free(packet2);
        }
    }
    printf("stream_send_thread_2 done");
    return NULL;
}


stream_send_t* init_stream_send(stream_encode_t *encoder_pointer, int sockfd, const char* type) {
    //TODO check stream type,sprint2
    printf("init_stream_send\n");
    stream_send_t *stream_send = (stream_send_t*)calloc(1, sizeof(stream_send_t));
    stream_send->running = 1;
    stream_send->close = close_send;
    stream_send->encoder = encoder_pointer;
    stream_send->socket = sockfd;
    pthread_t thread_id;
    // Note the threads need to be created with detached state to avoid memory leak
    pthread_attr_t attr;
    int rc = pthread_attr_init(&attr);
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (0==strcmp("test1", type)) {
        pthread_create(&thread_id, &attr, stream_send_thread_1, stream_send);
    } else if (0==strcmp("tcpjpg", type)) {
        pthread_create(&thread_id, &attr, stream_send_thread_2, stream_send);
    } else {
        pthread_create(&thread_id, &attr, stream_send_thread_0, stream_send);
    }

    return stream_send;
}
