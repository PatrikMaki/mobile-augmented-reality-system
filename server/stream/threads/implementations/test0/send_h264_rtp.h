#pragma once
#include "../../../packet.h"
#include "../../send.h"

int init_send(stream_send_t *stream);

void send_packet(stream_send_t *stream, packet_t *packet_pointer);