#pragma once
#include <threads.h>
#include "../../../packet.h"
#include "../../encode.h"

int init_encode_test1(stream_encode_t *stream);

packet_t* encode_test1(stream_encode_t *stream, packet_t *packet_pointer);
