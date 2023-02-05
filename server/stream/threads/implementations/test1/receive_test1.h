#pragma once
#include <threads.h>
#include "../../../packet.h"
#include "../../receive.h"

packet_t* get_test1_packet(stream_receive_t *stream);

void close_receive_test1_stream(stream_receive_t *stream);

int init_test1(stream_receive_t *stream);

packet_t* receive_test1(stream_receive_t *stream);
