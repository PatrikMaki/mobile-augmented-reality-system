#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphproblem.h"
#include "inference_test1.h"

float* create_table(int n) {
    float *d = (float*)calloc(n*n, sizeof(float));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            d[i*n + j] = random() % 10;
        }
    }
    return d;
}

int init_inference_test1(stream_inference_t *stream) {
    return 1;
}

packet_t* inference_test1(stream_inference_t *stream, packet_t *packet_pointer) {
    packet_t* packet = packet_copy(packet_pointer);
    int a = 0;
    int n = 0;
    n = atoi((const char*)packet->frame.data);
    /*
    const float d[] = {
        0, 8, 2, 1, 0, 9,
        1, 0, 9, 0, 8, 2,
        4, 5, 0, 4, 5, 0,
        0, 8, 2, 1, 0, 9,
        1, 0, 9, 0, 8, 2,
        4, 5, 0, 4, 5, 0,
    };
    */
    float *d = create_table(n);
    float r[n*n];
    #ifdef ENABLE_CUDA
    step(r, d, n);
    #endif
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            //printf("%.1f ",r[i*n + j]);
            if (r[i*n + j] != 0.0) {
                a += 1;
            }
        }
        //printf("\n");
    }
    free(packet->frame.data);
    packet->frame.data = (uint8_t*)calloc(16, 1);
    sprintf((char*)packet->frame.data, "%d %d", n, a);
    packet->frame.size_of_data = strlen((const char*)packet->frame.data);
    printf("inference test1 result: %s", packet->frame.data);
    free(d);
    return packet;
}
