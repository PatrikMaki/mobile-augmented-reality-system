#pragma once
#include <threads.h>
#include "../../../packet.h"
#include "../../decode.h"

int init_decode_h264(stream_decode_t *stream);

packet_t* decode(stream_decode_t *stream, packet_t *packet_pointer);
