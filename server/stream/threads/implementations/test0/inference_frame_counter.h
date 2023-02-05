#pragma once

#include <threads.h>
#include "../../../packet.h"
#include "../../inference.h"

int init_inference_frame_counter(stream_inference_t *stream);

packet_t* inference(stream_inference_t *stream, packet_t *packet_pointer);
