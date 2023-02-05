/*
 * This the MARS project server main code.
 * The server starts the control protocol and processes the control commands in the event handler.
 */

#include <stdio.h>
#include "mars.h"
#include "control/controlProtocol/controlProtocol.h"
#include "stream/stream.h"

static stream_t *stream;

/**
 * The message_handler is local function to implement the control protocol message handler
 */
static int message_handler(const char* request, char* response) {
    printf("#DEBUG message_handler: request:%s\n",request);
    if (0==strcmp(request,"Hello")) {
        strcpy(response, "Hi");

    } else if (0==strcmp(request,"QUIT")) {
        exit(0);

    } else if (0==strcmp(request,"test connection")) {
        strcpy(response, "start test");

    } else if (0==strcmp(request,"start stream, with protocol x")) { //First version that mirrors strings
        //int portnumber = start_stream_with_x("not","used"); //should also return the thread is so the thread can be killed
        printf("message_handler starting protocol \n");
        stream = init_stream(4321, "");
        sprintf(response, "start stream with x ,%d", 1); //added this
    } else if (0==strcmp(request,"start stream with test1")) {
        stream = init_stream(4321, "test1");
        sprintf(response, "start stream with test1 ,%d", 1); //added this
    } else if (0==strcmp(request,"start stream with receive rtp h264 decode h264 inference faceblur encode h264 send rtp")) {
        stream = init_stream(4321, "receive rtp h264 decode h264 inference faceblur encode h264 send rtp");
        sprintf(response, "start stream with x ,%d", 1); //added this
    } else if (0==strcmp(request,"tcpjpg")) {
        stream = init_stream(4321, "tcpjpg");
        sprintf(response, "tcpjpg"); //added this
    } else if (0==strcmp(request,"tcpjpg c")) {
        stream = init_stream(4321, "tcpjpg c");
        sprintf(response, "tcpjpg"); //same type of stream
        //<-STARTING STREAM

    } else if (0==strcmp(request,"stop stream")) {
        strcpy(response, "stopping stream");
        close_stream(&stream);

    } else if (0==strcmp(request,"close connection")) {
        strcpy(response, "close");
        close_stream(&stream);
        // should have sessions TODO: add them
        // close control procol  connection
        return 1;
    } else {
        printf("ERROR: Unsupported Control message: %s\n", request);
    }
    return 0;
}

int main() {
    printf("STARTING MARS\n");
    while (1) {
        ControlProtocol(message_handler);
    }
    return 0;
}
