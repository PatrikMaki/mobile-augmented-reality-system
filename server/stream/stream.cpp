/*
- UDP/TCP socket
- HW threads for each process
- in use
*/
#include "stream.h"

/**
 * Initialize the stream components
 */
stream_t* init_stream(int port, const char* config_string) {
    // This gets called from control protocol or main
    // for now it will called from same place as start_stream_with_x
    printf("#DEBUG: init_stream\n%s\n", config_string);
    stream_t *stream = (stream_t*)calloc(1, sizeof(stream_t));
    if (0==strcmp("", config_string)) {
        stream->receiver = init_receive_stream(port, "");
        stream->decoder = init_decode(stream->receiver, "");
        stream->inferencer = init_inference(stream->decoder, "");
        stream->encoder = init_encode(stream->inferencer, "");
        stream->sender = init_stream_send(stream->encoder, stream->receiver->socket, "");
        stream->running = 1;
    } else if (0==strcmp("test1", config_string)) {
        stream->receiver = init_receive_stream(port, "test1");
        stream->decoder = init_decode(stream->receiver, "test1");
        stream->inferencer = init_inference(stream->decoder, "test1");
        stream->encoder = init_encode(stream->inferencer, "test1");
        stream->sender = init_stream_send(stream->encoder, stream->receiver->socket, "test1");
        stream->running = 1;
    } else if (0==strcmp("tcpjpg", config_string)) {
        stream->receiver = init_receive_stream(port, "tcpjpg");
        stream->decoder = init_decode(stream->receiver, "tcpjpg");
        stream->inferencer = init_inference(stream->decoder, "tcpjpg");
        stream->encoder = init_encode(stream->inferencer, "tcpjpg");
        stream->sender = init_stream_send(stream->encoder, stream->receiver->socket, "tcpjpg");
        stream->running = 1;
    } else if (0==strcmp("tcpjpg c", config_string)) {
        // Initialize inference with (py)torch mobilenet model
        stream->receiver = init_receive_stream(port, "tcpjpg");
        stream->decoder = init_decode(stream->receiver, "tcpjpg");
        stream->inferencer = init_inference(stream->decoder, "tcpjpg c");
        stream->encoder = init_encode(stream->inferencer, "tcpjpg");
        stream->sender = init_stream_send(stream->encoder, stream->receiver->socket, "tcpjpg");
        // This should wait that the stream->receiver is ready...
        stream->running = 1;
    } else if (0==strcmp("receive rtp h264 decode h264 inference faceblur encode h264 send rtp", config_string)) {
        // TODO: Implement as an idea what the stream config could look like to be as flexible as possible
        return NULL;
    }

    return stream;
}

void close_stream(stream_t **stream_pointer_pointer) {
    if (stream_pointer_pointer==NULL || *stream_pointer_pointer==NULL) return;
    (*stream_pointer_pointer)->running = 0;
}

//*************************************************************************/
// OLD CODE TO BE REMOVED....
#if 0

packet_t* get_stream_packet(stream_t *stream_pointer);

//old:
void *receive_stream_with_x(void *vargp) { 
    printf("*receive_stream_with_x\n");
    int sockfd;
    int opt = 1;
    printf("1\n"); 
    char buffer[MAXLINE]; 
    const char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 
    printf("2\n");
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("3\n");
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    //?
    // Forcefully attaching socket to the port PORT, removed: "| SO_REUSEPORT"
    if (setsockopt(sockfd, SOL_SOCKET,
                   SO_REUSEADDR , &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //?
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(UDPPORT); 
    printf("4\n");
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("5\n");
    unsigned int len;
    int n; 
    
    len = sizeof(cliaddr);  //len is value/result 
    printf("5a\n"); 
    //############################  I should split the following loop to three threads
    /*
    receive packet
    decode video packet/image
    modify image
    encode video packet/image
    send packet
    control protocol
    CLI
    */
    //receive untill "\n", then send to decoder thread(GPU later)
    /*
    while(1) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len);
        
    } 
    return NULL;
    */
    //old code: 
    while(1) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len);
        //printf("6\n"); 
        int nn = sizeof(buffer) / sizeof(buffer[0]);
        //modify packet here:
        //ReverseArray(buffer, nn);
        buffer[n] = '\0'; 
        printf("Client : %s\n", buffer); 
        
        sendto(sockfd, (const char *)buffer, strlen(hello),  
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                len); 
        printf("Hello message sent.\n");  
    }
    return NULL;
}

int start_stream_with_x(char* request, char* response) {
    //should I start the processes/threads here? 
    //receive_stream_with_x();
    //send_stream_with_x();
    
    pthread_t thread_id;
    printf("Before Thread\n");
    pthread_create(&thread_id, NULL, receive_stream_with_x, NULL);
    //pthread_join(thread_id, NULL);
    printf("After Thread\n");
    return 1;
    
}
#endif
