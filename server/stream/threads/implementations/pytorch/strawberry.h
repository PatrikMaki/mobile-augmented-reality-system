#pragma once
/*
 * Definitions of the Pytorch native jpeg image image classifier
 */

#include "../../../packet.h"
#include "../../inference.h"

int init_image_torch(stream_inference_t *stream, const char* model, const char* label);
void deinit_image_torch();
void step(stream_inference_t *stream, packet_t* packet);
