#include "send_h264_rtp.h"
#include "../../../stream.h"
// This is template for future implementation of H.265 RTP Sender
// Note that now this just sends UDP packets...

int init_send(stream_send_t *stream) {
    return 1;
    printf("#DEBUG: send_h264_rtp: init_send\n");
    //int sockfd;
    int opt = 1;
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
    // Creating socket file descriptor // Looks like no, we have to use a
    // previously created socket so the response comes from same port
    // where it was received.
    /*if ( (stream->socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } */
    // TODO: Do we need to set this parameters again to the socket if that was already set in receive?
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    // Forcefully attaching socket to the port PORT, removed: "| SO_REUSEPORT"
    if (setsockopt(stream->socket, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(UDPPORT + 1); 
    // Bind the socket with the server address 
    if ( bind(stream->socket, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("#DEBUG: send_h264_rtp: return\n");
    return 1;
}

void send_packet(stream_send_t *stream, packet_t *packet_pointer) {
    sendto(stream->socket, (const char *)packet_pointer->frame.data, packet_pointer->frame.size_of_data,  
            MSG_CONFIRM, (const struct sockaddr *) &packet_pointer->cliaddr, 
                sizeof(packet_pointer->cliaddr));
    printf("#DEBUG: send_h264_rtp: packet sent\n");
}
