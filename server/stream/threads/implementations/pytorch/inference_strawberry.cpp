#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inference_strawberry.h"
#include "strawberry.h"

int init_inference_strawberry(stream_inference_t *stream) {
    //init_image_torch(stream, "../models/resnet-pretrained-model.pt", "../models/resnet-labels.txt");
    init_image_torch(stream, "../models/mobilenet-pretrained-model.pt", "../models/resnet-labels.txt");
    //init_image_torch(stream, "../models/model.pt", "../models/labels.txt");
    return 1;
    //could parse the arguments here
}

packet_t* inference_strawberry(stream_inference_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    if (packet->frame.size_of_data == 0) {
        deinit_image_torch();
        return packet;
    }
    //printf("inference of strawberry\n");
    step(stream, packet); //comment this out to disable inference
    return packet;
}
