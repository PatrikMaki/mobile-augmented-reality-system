#include "encode.h"
#include <unistd.h>
#include <string.h>
#include "implementations/test0/encode_h264.h"
#include "implementations/test1/encode_test1.h"

#ifdef USE_PACKETS_V1
static void enqueue(stream_encode_t *stream, packet_t *packet) {
    if (packet == NULL) {
        return;
    }
    pthread_mutex_lock(&stream->lock);
    TAILQ_INSERT_TAIL(&stream->queue, packet, nodes);
    pthread_mutex_unlock(&stream->lock);
}

static packet_t* dequeue(stream_encode_t *stream) {
    packet_t *packet = NULL;
    pthread_mutex_lock(&stream->lock);
    if (!TAILQ_EMPTY(&stream->queue)) {
        packet = TAILQ_FIRST(&stream->queue);
        TAILQ_REMOVE(&stream->queue, packet, nodes);
    }
    pthread_mutex_unlock(&stream->lock);
    return packet;
}
#else
// Called by this encode stage to push message to sender
static void enqueue(stream_encode_t *stream, packet_t *packet) {
    packets_enqueue(&stream->queue, packet);
}

// Called by next stage (sender)
static packet_t* dequeue(stream_encode_t *stream) {
    packet_t *e = packets_dequeue(&stream->queue, -1); // WAIT FOREVER
    return e;
}
#endif

static void close_encode(stream_encode_t *stream) {
    
}

static void *stream_encode_thread_0(void *vargp) {
    stream_encode_t* stream = (stream_encode_t*) vargp;
    init_encode_h264(stream);
    while (stream->running) {
        //encode:
        packet_t* packet2 = (*stream->inferencer->getPacket)(stream->inferencer);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = encode(stream, packet2);
            //printf("encode %p -> %p\n", packet2, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            enqueue(stream, packet);
            free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

static void *stream_encode_thread_1(void *vargp) {
    stream_encode_t* stream = (stream_encode_t*) vargp;
    init_encode_test1(stream);
    while (stream->running) {
        //encode:
        packet_t* packet2 = (*stream->inferencer->getPacket)(stream->inferencer);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = encode_test1(stream, packet2);
   	        //printf("encode %p -> %p\n", packet2, packet);
            enqueue(stream, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

stream_encode_t* init_encode(stream_inference_t *inferencer_pointer, const char* type) {
    //TODO check stream type,sprint2
    printf("init_encode\n");
    stream_encode_t *stream_encode = (stream_encode_t*)calloc(1, sizeof(stream_encode_t));
    stream_encode->running = 1;
    stream_encode->getPacket = dequeue;
    stream_encode->close = close_encode;
    stream_encode->inferencer = inferencer_pointer;
    #ifdef USE_PACKETS_V1
    TAILQ_INIT(&stream_encode->queue);
    #else
    packets_init(&stream_encode->queue);
    #endif
    pthread_t thread_id;
    // Note the threads need to be created with detached state to avoid memory leak
    pthread_attr_t attr;
    int rc = pthread_attr_init(&attr);
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (0==strcmp("test1", type)) {
        pthread_create(&thread_id, &attr, stream_encode_thread_1, stream_encode);
    } else if (0==strcmp("test1", type)) {
        pthread_create(&thread_id, &attr, stream_encode_thread_1, stream_encode);
    } else {
        pthread_create(&thread_id, &attr, stream_encode_thread_0, stream_encode);
    }

    return stream_encode;
}
