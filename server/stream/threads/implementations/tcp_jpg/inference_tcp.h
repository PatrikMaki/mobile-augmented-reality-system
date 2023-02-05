#pragma once
/*
 * Definitions of the dummy Image frame inference
 */

#include <threads.h>
#include "../../../packet.h"
#include "../../inference.h"

int init_inference_tcpjpg(stream_inference_t *stream);

packet_t* inference_tcpjpg(stream_inference_t *stream, packet_t *packet_pointer);
