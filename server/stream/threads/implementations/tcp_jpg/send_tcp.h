#pragma once
/*
 * Definitions of the custom TCP Image stream sender for MARS Project.
 */
#include "../../../packet.h"
#include "../../send.h"

/**
 * Initialize the send thread
 */
int init_send_tcp(stream_send_t *stream);

/**
 * Send a packet to the client
 */
void send_packet_tcp(stream_send_t *stream, packet_t *packet_pointer);
