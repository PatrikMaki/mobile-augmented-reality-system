#include "../../mars.h"
#include "controlProtocol.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 4231

int ControlProtocol(int (*callback)(const char*, char*))
{
    int server_fd, new_socket, hexread, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int length;
    char hexbuffer[5] = { 0 };
    char buffer[1024] = { 0 };
    char response[1024] = { 0 };
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the port PORT
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR , &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  
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
    printf("ControlProtocol: Calling accept\n");
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("ControlProtocol: new connection\n");
    do {
        hexread = read(new_socket, hexbuffer, 4);
        if(hexread != 4) break;
        length = strtol(hexbuffer, NULL, 16);
	if (length > 1024) {
            printf("ERROR: Too large control message, closing connection\n");
	    break;
	}
        int t_length = length;
        char *p = buffer;
        do {
            valread = read(new_socket, p, length);
            printf("ControlProtocol read valread %d\n",valread);
            if(valread < 0) break;
	    length -= valread;
	    p += valread;
        } while (length > 0);
        p[length] = 0;
        printf("ControlProtocol received message length: %d, message: %s\n", t_length, buffer);
        if(0==(*callback)(buffer, &response[4])){
            snprintf(hexbuffer, sizeof(hexbuffer), "%04lx",strlen(&response[4]));
            memcpy(response, hexbuffer, 4);
            write(new_socket, response, strlen(response));
            printf("ControlProtocol response sent:%s\n",response);
        }else break;
    } while(1);
    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
