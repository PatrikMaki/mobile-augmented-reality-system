#include "decode.h"
#include <unistd.h>
#include <string.h>
#include "implementations/test0/decode_h264.h"
#include "implementations/test1/decode_test1.h"

#ifdef USE_PACKETS_V1
static void enqueue(stream_decode_t *stream, packet_t *packet) {
    if (packet == NULL) {
        return;
    }
    pthread_mutex_lock(&stream->lock);
    TAILQ_INSERT_TAIL(&stream->queue, packet, nodes);
    pthread_mutex_unlock(&stream->lock);
}

static packet_t* dequeue(stream_decode_t *stream) {
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
// Called by this decode stage
static void enqueue(stream_decode_t *stream, packet_t *packet) {
    packets_enqueue(&stream->queue, packet);
}

// Called by next stage (inference)
static packet_t* dequeue(stream_decode_t *stream) {
    packet_t *e = packets_dequeue(&stream->queue, -1); // WAIT FOREVER
    return e;
}
#endif

static void close_decode(stream_decode_t *stream_decode_pointer) {

}

static void *stream_decode_thread_0(void *vargp) {
    stream_decode_t* stream = (stream_decode_t*) vargp;
    init_decode_h264(stream);
    while (stream->running) {
        packet_t* packet2 = (*stream->receiver->getPacket)(stream->receiver);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = decode(stream, packet2);
            //printf("decode %p -> %p\n", packet2, packet);
            enqueue(stream, packet);
            if (packet2->frame.data) free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

static void *stream_decode_thread_1(void *vargp) {
    stream_decode_t* stream = (stream_decode_t*) vargp;
    init_decode_test1(stream);
    while (stream->running) {
        packet_t* packet2 = (*stream->receiver->getPacket)(stream->receiver);
        #ifdef USE_PACKETS_V1
        if (packet2==NULL) {
            usleep(1000);
        }else
        #endif
        {
            packet_t* packet = decode_test1(stream, packet2);
	        //printf("decode %p -> %p\n", packet2, packet);
            enqueue(stream, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            if (packet2->frame.data) free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}


stream_decode_t* init_decode(stream_receive_t *receiver_pointer, const char* type) {
        //TODO check stream type,sprint2
    printf("init_decode\n");
    stream_decode_t *stream_decode = (stream_decode_t*)calloc(1, sizeof(stream_decode_t));
    stream_decode->running = 1;
    stream_decode->getPacket = dequeue;
    stream_decode->close = close_decode;
    stream_decode->receiver = receiver_pointer;
    #ifdef USE_PACKETS_V1
    TAILQ_INIT(&stream_decode->queue);
    #else
    packets_init(&stream_decode->queue);
    #endif
    pthread_t thread_id;

    // Note the threads need to be created with detached state to avoid memory leak
    pthread_attr_t attr;
    int rc = pthread_attr_init(&attr);
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (0==strcmp("test1", type)) {
         pthread_create(&thread_id, &attr, stream_decode_thread_1, stream_decode);
    } else if (0==strcmp("tcpjpg", type)) {
         pthread_create(&thread_id, &attr, stream_decode_thread_1, stream_decode);
    } else {
         pthread_create(&thread_id, &attr, stream_decode_thread_0, stream_decode);
    }
    return stream_decode;
}
