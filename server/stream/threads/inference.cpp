#include "inference.h"
#include <unistd.h>
#include <string.h>
#include "implementations/test0/inference_frame_counter.h"
#include "implementations/test1/inference_test1.h"
#include "implementations/tcp_jpg/inference_tcp.h"
#include "implementations/pytorch/inference_strawberry.h"


#ifdef USE_PACKETS_V1
static void enqueue(stream_inference_t *stream, packet_t *packet) {
    if (packet == NULL) {
        return;
    }
    pthread_mutex_lock(&stream->lock);
    TAILQ_INSERT_TAIL(&stream->queue, packet, nodes);
    pthread_mutex_unlock(&stream->lock);
}

static packet_t* dequeue(stream_inference_t *stream) {
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
// Called by this inference stage to push message to encoder
static void enqueue(stream_inference_t *stream, packet_t *packet) {
    packets_enqueue(&stream->queue, packet);
}

// Called by next stage (encoder)
static packet_t* dequeue(stream_inference_t *stream) {
    packet_t *e = packets_dequeue(&stream->queue, -1); // WAIT FOREVER
    return e;
}
#endif

static void close_inference(stream_inference_t *stream) {
    
}

static void *stream_inference_thread_0(void *vargp) {
    stream_inference_t* stream = (stream_inference_t*) vargp;
    printf("inference test0\n");
    //init_inference_frame_counter(stream);
    while (stream->running) {
        //inference:
        packet_t* packet2 = (*stream->decoder->getPacket)(stream->decoder);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = inference(stream, packet2);
            printf("inference %p -> %p\n", packet2, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            enqueue(stream, packet);
            free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

static void *stream_inference_thread_1(void *vargp) {
    stream_inference_t* stream = (stream_inference_t*) vargp;
    printf("inference test1\n");
    init_inference_test1(stream);
    while (stream->running) {
        //inference:
        packet_t* packet2 = (*stream->decoder->getPacket)(stream->decoder);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = inference_test1(stream, packet2);
	        //printf("inference %p -> %p\n", packet2, packet);
            enqueue(stream, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

static void *stream_inference_thread_2(void *vargp) {
    stream_inference_t* stream = (stream_inference_t*) vargp;
    printf("inference test2\n");
    init_inference_tcpjpg(stream);
    while (stream->running) {
        //inference:
        packet_t* packet2 = (*stream->decoder->getPacket)(stream->decoder);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = inference_tcpjpg(stream, packet2);
	        //printf("inference %p -> %p\n", packet2, packet);
            enqueue(stream, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

static void *stream_inference_thread_3(void *vargp) {
    stream_inference_t* stream = (stream_inference_t*) vargp;
    printf("inference test3\n");
    init_inference_strawberry(stream);
    while (stream->running) {
        //inference:
        packet_t* packet2 = (*stream->decoder->getPacket)(stream->decoder);
        if (packet2==NULL) {
            usleep(1000);
        }else {
            packet_t* packet = inference_strawberry(stream, packet2); //TODO
	        //printf("inference %p -> %p\n", packet2, packet);
            enqueue(stream, packet);
            if (packet2->frame.size_of_data == 0) stream->running = false;
            free(packet2->frame.data);
            free(packet2);
        }
    }
    return NULL;
}

stream_inference_t* init_inference(stream_decode_t *decoder_pointer, const char* type) {
    //TODO check stream type,sprint2
    printf("init_inference\n");
    stream_inference_t *stream_inference = (stream_inference_t*)calloc(1, sizeof(stream_inference_t));
    stream_inference->running = 1;
    stream_inference->getPacket = dequeue;
    stream_inference->close = close_inference;
    stream_inference->decoder = decoder_pointer;
    #ifdef USE_PACKETS_V1
    TAILQ_INIT(&stream_inference->queue);
    #else
    packets_init(&stream_inference->queue);
    #endif
    pthread_t thread_id;
    // Note the threads need to be created with detached state to avoid memory leak
    pthread_attr_t attr;
    int rc = pthread_attr_init(&attr);
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (0==strcmp("test1", type)) {
        pthread_create(&thread_id, &attr, stream_inference_thread_1, stream_inference);
    } else if (0==strcmp("tcpjpg", type)) {
        pthread_create(&thread_id, &attr, stream_inference_thread_2, stream_inference);
    } else if (0==strcmp("tcpjpg c", type)) {
        pthread_create(&thread_id, &attr, stream_inference_thread_3, stream_inference);
    } else {
        init_inference_frame_counter(stream_inference);
        pthread_create(&thread_id, &attr, stream_inference_thread_0, stream_inference);
    }

    return stream_inference;
}
