#pragma once
/*
 * Definitions of the image classification/labeling inference processor.
 */

#include <threads.h>
#include "../../../packet.h"
#include "../../inference.h"

int init_inference_strawberry(stream_inference_t *stream);

packet_t* inference_strawberry(stream_inference_t *stream, packet_t *packet_pointer);
