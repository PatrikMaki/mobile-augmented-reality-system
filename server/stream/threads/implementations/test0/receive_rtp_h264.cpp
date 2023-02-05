#include "receive_rtp_h264.h"
#include "../../../stream.h"
// This is template for future implementation of H.265 RTP Receiver
// Note that now this receives just UDP packets...

packet_t* get_h264_rtp_packet(stream_receive_t *stream) {
    return NULL;
}

void close_receive_h264_rtp_stream(stream_receive_t *stream) {
    
}

int init_rtp_h264(stream_receive_t *stream) {
    printf("#DEBUG: init_rtp_h264\n");
    //int sockfd;
    int opt = 1;
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
    // Creating socket file descriptor 
    if ( (stream->socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    // Forcefully attaching socket to the port PORT, removed: "| SO_REUSEPORT"
    if (setsockopt(stream->socket, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(UDPPORT); 
    // Bind the socket with the server address 
    if ( bind(stream->socket, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("#DEBUG: receive_rtp_h264 OK\n");
    return 1; // TODO: should this return 0 as Unix style status code indicating ok? Now it's True (1).
    

}

packet_t* receive(stream_receive_t *stream) {
    packet_t* packet = (packet_t*) calloc(1,sizeof(packet_t));
    packet->frame.data = (uint8_t*) calloc(1, MAXLINE);
    unsigned int len;
    int n; 
    
    len = sizeof(packet->cliaddr);  //len is value/result 
    printf("#DEBUG: receive_rtp_h264: Calling recvgfrom UDP Packet\n"); 
    
    n = recvfrom(stream->socket, (char *)packet->frame.data, MAXLINE,  
                    MSG_WAITALL, ( struct sockaddr *) &packet->cliaddr, 
                    &len);
    packet->frame.size_of_data = n;
    return packet;
}

