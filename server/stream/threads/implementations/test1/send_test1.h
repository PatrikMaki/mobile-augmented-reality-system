#pragma once
#include "../../../packet.h"
#include "../../send.h"

int init_send_test1(stream_send_t *stream);

void send_packet_test1(stream_send_t *stream, packet_t *packet_pointer);