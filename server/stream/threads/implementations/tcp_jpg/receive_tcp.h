#pragma once
/*
 * Definitions of the custom TCP Image stream receiver for MARS Project.
 */
#include <threads.h>
#include "../../../packet.h"
#include "../../receive.h"

/**
 * Initialize the MARS TCP Receive stream
 */
int init_tcp(stream_receive_t *stream, int port);

/**
 * Close the MARS TCP Receive socket stream
 */
void close_receive_tcp_stream(stream_receive_t *stream);

/**
 * Receive a packet from the MARS TCP Stream
 */
packet_t* receive_tcp(stream_receive_t *stream);
