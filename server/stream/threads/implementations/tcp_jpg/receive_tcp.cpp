/*
 * Implementation of the custom TCP Image stream receiver for MARS Project.
 */

#include <netinet/tcp.h>
#include "receive_tcp.h"
#include "../../../stream.h"

void close_receive_tcp_stream(stream_receive_t *stream) {
    close(stream->serverfd);
}

int init_tcp(stream_receive_t *stream, int port) {
    printf("#DEBUG: receive_tcp init_tcp\n");
    
    int server_fd, new_socket, hexread, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int length;
    stream->socket = -1;
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR , &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
  
    // Forcefully attaching socket to the port PORT
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    stream->serverfd = server_fd;
    return 1;
}

/**
 * Read size amount of bytes from the socket possibly in multiple
 * chunks or until the socket is closed.
 */
static size_t readbytes(int new_socket, char* buffer, size_t size) {
    size_t length = size;
    char* p = buffer;
    int valread;
    do {
        valread = read(new_socket, p, length);
        if (valread <= 0) {
	       return valread;
	    }
        length -= valread;
        p += valread;
    } while (length > 0);
    return  size;
}

/**
 * Receive the custom MARS TCP Stream frame with header and jpeg payload.
 */
static packet_t* recv_tcp_packet(int new_socket) {
    int server_fd, hexread, valread;
    int length;
    char hexbuffer[FRAME_H_LEN+1]={0};//calculated from frame
    hexread = readbytes(new_socket, hexbuffer, FRAME_H_LEN);
    if (hexread == 0) {
      printf("DEBUG: Got connection close, returning empty packet as marker to end stream\n");
      packet_t* packet = (packet_t*) calloc(1,sizeof(packet_t));
      return packet;
    }
    if(hexread != FRAME_H_LEN) return NULL;

    //check frame header
    if (memcmp(hexbuffer,"img2",4)) {
        printf("ERROR: only supports img2 format for tcp\n");
        packet_t* packet = (packet_t*) calloc(1,sizeof(packet_t));
        return packet;
    }
    char * pEnd = &hexbuffer[5];
    uint64_t framecount = strtol(pEnd, &pEnd, 16);
    uint64_t timestart = strtol(pEnd, &pEnd, 16);
    uint64_t processtime = strtol(pEnd, &pEnd, 16);
    length = strtol(pEnd, &pEnd, 16);
    if (length > 64*1024*1024) {
        printf("ERROR: Image too large\n"); // TODO: close connection ?
        //break;
    }
    packet_t* packet = (packet_t*) calloc(1,sizeof(packet_t));
    packet->frame.frameCount = framecount;
    packet->frame.timeStart = timestart;
    packet->frame.processTime = processtime;
    packet->frame.data = (uint8_t*) calloc(1, length);
    packet->frame.size_of_data = length;
    int t_length = length;
    char *p = (char*)packet->frame.data;
    do {
        valread = read(new_socket, p, length);
        if (valread <= 0) {
            break;
        }
        length -= valread;
        p += valread;
    } while (length > 0);
    packet->sockfd = new_socket;
    return packet;
}

/**
 * The receive function to wait for a new connection or read messages from the stream.
 */
packet_t* receive_tcp(stream_receive_t *stream) {
    // TODO: Add support for multiple parallel clients
    if (stream->socket == -1) {
        if ((stream->socket = accept(stream->serverfd, NULL, NULL))< 0) {
            perror("accept");
            exit(EXIT_FAILURE); //TODO graceful handling
        }
        // Disable TCP Nagle algorithm as that delays requests by 50 msec
        int one = 1;
        setsockopt(stream->socket, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
    }
    packet_t* packet =  recv_tcp_packet(stream->socket);
    if (packet == NULL || packet->frame.size_of_data == 0) {
        close(stream->socket);
        close(stream->serverfd);
    }

    return packet;
}

