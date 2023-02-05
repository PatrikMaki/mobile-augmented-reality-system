#pragma once
#include <threads.h>
#include "../../../packet.h"
#include "../../encode.h"

int init_encode_h264(stream_encode_t *stream);

packet_t* encode(stream_encode_t *stream, packet_t *packet_pointer);
