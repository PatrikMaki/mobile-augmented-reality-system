#include "receive_test1.h"
#include "../../../stream.h"

packet_t* get_test1_packet(stream_receive_t *stream) {
    return NULL;
}

void close_receive_test1_stream(stream_receive_t *stream) {
    
}

int init_test1(stream_receive_t *stream) {
    printf("#DEBUG: receive_test1 init_test1\n");
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
    printf("#DEBUG: receive_test1 init_test1 return\n");
    return 1;
    

}

packet_t* receive_test1(stream_receive_t *stream) {
    packet_t* packet = (packet_t*) calloc(1,sizeof(packet_t));
    packet->frame.data = (uint8_t*) calloc(1, MAXLINE);
    unsigned int len;
    int n; 
    
    len = sizeof(packet->cliaddr);  //len is value/result 
    
    n = recvfrom(stream->socket, (char *)packet->frame.data, MAXLINE,  
                    MSG_WAITALL, ( struct sockaddr *) &packet->cliaddr, 
                    &len);
    packet->frame.size_of_data = n;
    if (n>0) {
        packet->frame.data[n] = 0;
    }
    return packet;
}

