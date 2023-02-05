#include "send_test1.h"
#include "../../../stream.h"

int init_send_test1(stream_send_t *stream) {
    return 1; // Reuse the receiver UDP socket
    #if 0
    printf("init_send\n");
    //int sockfd;
    int opt = 1;
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
    // Creating socket file descriptor 
    /*if ( (stream->socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } */
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    //?
    // Forcefully attaching socket to the port PORT, removed: "| SO_REUSEPORT"
    if (setsockopt(stream->socket, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //?
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
    return 1;
    #endif
}

void send_packet_test1(stream_send_t *stream, packet_t *packet_pointer) {
    ssize_t a = sendto(stream->socket, (const char *)packet_pointer->frame.data, packet_pointer->frame.size_of_data,  
            MSG_CONFIRM, (const struct sockaddr *) &packet_pointer->cliaddr, 
                sizeof(packet_pointer->cliaddr));
    printf("packet sent\n%d", (int)a);
}
