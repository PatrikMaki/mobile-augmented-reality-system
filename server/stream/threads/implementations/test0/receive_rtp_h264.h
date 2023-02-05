#pragma once
#include <threads.h>
#include "../../../packet.h"
#include "../../receive.h"

packet_t* get_h264_rtp_packet(stream_receive_t *stream);

void close_receive_h264_rtp_stream(stream_receive_t *stream);

int init_rtp_h264(stream_receive_t *stream);

packet_t* receive(stream_receive_t *stream);
