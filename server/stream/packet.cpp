/*
 * This file contains the packet queue low level functions for packet queue handling.
 */

#include <stdio.h>
#include <string.h>
#include "packet.h"

/**
 * Copy a single packet
 */
packet_t* packet_copy(const packet_t *packet_pointer) {
    packet_t *packet = (packet_t*)calloc(1, sizeof(packet_t));
    if (packet_pointer->frame.size_of_data > 0) {
        packet->frame.data =  (uint8_t*)calloc(1, packet_pointer->frame.size_of_data);
        memcpy(packet->frame.data, packet_pointer->frame.data, packet_pointer->frame.size_of_data);
    }
    packet->frame.size_of_data = packet_pointer->frame.size_of_data;
    packet->frame.frameCount = packet_pointer->frame.frameCount;
    packet->frame.timeStart = packet_pointer->frame.timeStart;
    packet->frame.processTime = packet_pointer->frame.processTime;
    memcpy(&packet->cliaddr, &packet_pointer->cliaddr, sizeof(packet_pointer->cliaddr));
    packet->sockfd = packet_pointer->sockfd;
    return packet;
}

#ifndef USE_PACKETS_V1
/**
 * Free a single packet
 */
void packet_free(packet_t **entry) {
    if (entry == NULL || *entry == NULL) return;
    if ((*entry)->frame.data != NULL) {
        free((*entry)->frame.data);
        (*entry)->frame.data = NULL;
    }
    free(*entry);
    *entry = NULL;
}

/**
 * create new packet queue
 */
packets_t *packets_create() {
    packets_t *queue = (packets_t*)calloc(1, sizeof(packets_t));
    TAILQ_INIT(&queue->queue);
    return queue;
}

/**
 * Initialize existing queue
 */
void packets_init(packets_t *queue) {
    TAILQ_INIT(&queue->queue);
}

/**
 * free packets queue
 */
void packets_free(packets_t **queuep) {
    if (queuep == NULL || *queuep == NULL) return;
    packets_t *queue = *queuep;
    pthread_mutex_lock(&queue->lock);
    // iterate all the entries before deleting the queue
    while (!TAILQ_EMPTY(&queue->queue)) {
        packet_t *e = TAILQ_FIRST(&queue->queue);
        packet_free(&e);
        TAILQ_REMOVE(&queue->queue, e, nodes);
        queue->length--;
    }
    pthread_mutex_unlock(&queue->lock);
    free(queue);
    *queuep = NULL;
}

/**
 * Enqueue a packet to the queue
 */
void packets_enqueue(packets_t *queue, packet_t *message) {
    if (queue == NULL) return;
    if (message == NULL) return;
    pthread_mutex_lock(&queue->lock);
    TAILQ_INSERT_TAIL(&queue->queue, message, nodes);
    pthread_mutex_unlock(&queue->lock);
    queue->length++;
    pthread_cond_signal(&queue->cv);
}

/**
 * Dequeue a packet from the queue with optional timeout
 */
packet_t* packets_dequeue(packets_t *queue, int timeout_millis) {
    if (queue == NULL) return NULL;
    packet_t *e = NULL;
    pthread_mutex_lock(&queue->lock);
    if (queue->length == 0) {
        if (timeout_millis > 0) {
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += timeout_millis / 1000;
            timeout.tv_nsec += (timeout_millis % 1000) * 1000000L;
            if (timeout.tv_nsec >= 1000000000L) {
                timeout.tv_nsec -= 1000000000L;
                timeout.tv_sec++;
            }
            pthread_cond_timedwait(&queue->cv, &queue->lock, &timeout);
        } else if (timeout_millis == -1) {
            pthread_cond_wait(&queue->cv, &queue->lock);
        }
    }
    
    if (!TAILQ_EMPTY(&queue->queue)) {
        e = TAILQ_FIRST(&queue->queue);
        TAILQ_REMOVE(&queue->queue, e, nodes);
        queue->length--;
    }
    pthread_mutex_unlock(&queue->lock);
    return e;
}
#endif
