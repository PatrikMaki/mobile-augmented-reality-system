#pragma once

#include <threads.h>
#include "../../../packet.h"
#include "../../inference.h"

int init_inference_test1(stream_inference_t *stream);

packet_t* inference_test1(stream_inference_t *stream, packet_t *packet_pointer);
