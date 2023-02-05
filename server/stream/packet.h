#pragma once

/**
 * Definitions for MARS project stream message queue packets and video/image frames
 */

#include "../mars.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h> 
#include <netinet/in.h>
#include <sys/queue.h>

#define FRAME_H_LEN 64
/*
wire format:
- 8 bytes hex datasize
- data

wire format 2:
field1:
- 4 bytes of format ("img2")
- space //5
- 16 bytes of hex frameCount //21
- space //22
- 16 bytes of hex timeStart //38
- space //39
- 16 bytes of hex processTime //55
- space //56
- 8 bytes of hex datasize //64
- n bytes of binary data
*/

/**
 * The structure for a single jpeg image
 */
typedef struct {
    uint64_t frameCount;  // Client side created incremented frame counter
    uint64_t timeStart;   // Client side timestamp in milliseconds when the image was captured
    uint64_t processTime; // Server side inference processing time
    size_t size_of_data;  // Size of the data (jpeg image)
    uint8_t *data;        // The image data itself
} frame_t;

/**
 * Queued packet
 */
typedef struct packet_node {
    frame_t frame;                   // The frame containing the payload
    TAILQ_ENTRY(packet_node) nodes;  // C-style queue node
    struct sockaddr_in cliaddr;      // Client IP-addresses, used for UDP sending
    int sockfd;                      // Socket used to send response back
} packet_t;

/**
 * A C queueu library based queue structure.
 */
typedef TAILQ_HEAD(packet_head_s, packet_node) packet_queue_t;

/**
 * Function to duplicate a package.
 */
packet_t* packet_copy(const packet_t *packet_pointer);

#ifndef USE_PACKETS_V1

/**
 * A structure to hold a thread-safe queue which supports blocking wait for dequeue.
 */
typedef struct {
    int length;           // number of entries in the queue
    packet_queue_t queue; // the actual queue
    pthread_mutex_t lock; // lock to protect the updates to the queue
    pthread_cond_t cv;    // Condition variable to implement nonpolling wait for packets
} packets_t;

/**
 * Free a single packet.
 */
void packet_free(packet_t **entry);

/**
 * Create and allocate a new packets queue.
 */
packets_t *packets_create();

/**
 * Initialize a previously allocated packets queue.
 */
void packets_init(packets_t *queue);

/**
 * Clear and free a previously allocated packets queue.
 */
void packets_close(packets_t **queuep);

/**
 * Add a packet message to the packets queue.
 */
void packets_enqueue(packets_t *queue, packet_t *message);

/**
 * Take a packet from the packets queue.
 *
 * @param queue is the queue where the packets are read from
 * @param timeout_millis defines the time to wait for a packet to arrive to the queue. Note
 *        if the value is -1 this call will block until a packet is queued with packets_enqueue.
 *        If the value is 0 the request returns NULL if no packets are in the quue.
 *        If the value is greater than 0 then call will wait up to that number of milliseconds
 *        until packets are available or returns NULL if no packets are available.
 * @return a packet from the queue that the caller must free or NULL.
 */
packet_t* packets_dequeue(packets_t *queue, int timeout_millis);
#endif
