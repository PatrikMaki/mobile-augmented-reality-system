#include "receive.h"
#include "implementations/test0/receive_rtp_h264.h"
#include "implementations/test1/receive_test1.h"
#include "implementations/tcp_jpg/receive_tcp.h"
#include <unistd.h>
#include <string.h>

#ifdef USE_PACKETS_V1
static void enqueue(stream_receive_t *stream, packet_t *packet) {
    if (packet == NULL) {
        return;
    }
    pthread_mutex_lock(&stream->lock);
    TAILQ_INSERT_TAIL(&stream->queue, packet, nodes);
    pthread_mutex_unlock(&stream->lock);
}

static packet_t* dequeue(stream_receive_t *stream) {
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
static void enqueue(stream_receive_t *stream, packet_t *packet) {
    packets_enqueue(&stream->queue, packet);
}

static packet_t* dequeue(stream_receive_t *stream) {
    packet_t *e = packets_dequeue(&stream->queue, -1); // WAIT FOREVER
    return e;
}
#endif

static void *stream_receive_thread_0(void *vargp) {
    stream_receive_t* stream = (stream_receive_t*) vargp;
    init_rtp_h264(stream);
    stream->running = 1;
    while (stream->running) {
        packet_t* packet = receive(stream);
        printf("Received T0: %p\n", packet);
        if (packet != NULL) {
            enqueue(stream, packet);
        }
    }
    return NULL;
}

static void *stream_receive_thread_1(void *vargp) {
    stream_receive_t* stream = (stream_receive_t*) vargp;
    init_rtp_h264(stream);
    stream->running = 1;
    while (stream->running) {
        packet_t* packet = receive_test1(stream);
        printf("Received T1: %p\n", packet);
        if (packet != NULL) {
            enqueue(stream, packet);
        }
    }
    return NULL;
}

static void *stream_receive_thread_2(void *vargp) {
    stream_receive_t* stream = (stream_receive_t*) vargp;
    init_tcp(stream, stream->port); // TODO: Error handling
    stream->running = 1;
    while (stream->running) {
        packet_t* packet = receive_tcp(stream);
        if (packet != NULL) {
            enqueue(stream, packet);
            if (packet->frame.size_of_data == 0) stream->running = false;
        }
    }
    return NULL;
}

stream_receive_t* init_receive_stream(int port, const char* type) {
    printf("init_receive_stream\n");
    stream_receive_t *stream_receive = (stream_receive_t*)calloc(1, sizeof(stream_receive_t));
    // stream_receive->running = 1; // Moved to the implementation so we return only once it's ready
    stream_receive->getPacket = dequeue;
    stream_receive->close = close_receive_h264_rtp_stream;
    stream_receive->port = port;
    #ifdef USE_PACKETS_V1
    TAILQ_INIT(&stream_receive->queue);
    #else
    packets_init(&stream_receive->queue);
    #endif
    pthread_t thread_id;
    // Note the threads need to be created with detached state to avoid memory leak
    pthread_attr_t attr;
    int rc = pthread_attr_init(&attr);
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (0==strcmp("test1", type)) {
        pthread_create(&thread_id, &attr, stream_receive_thread_1, stream_receive);
    } else if (0==strcmp("tcpjpg", type)) {
        pthread_create(&thread_id, &attr, stream_receive_thread_2, stream_receive);
    } else {
        pthread_create(&thread_id, &attr, stream_receive_thread_0, stream_receive);
    }
    int attempts = 1000;
    while (stream_receive->running == 0 && attempts-- > 0) {
        usleep(1000);
    }
    
    return stream_receive;
}
