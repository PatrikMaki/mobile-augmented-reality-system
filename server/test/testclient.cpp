// This is a simple test client for MARS control and stream protocol

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROL_PORT 4231
#define STREAM_PORT 4321

static double current_time_millis() {
    static struct timeval start = {0,0};
    if (start.tv_sec == 0) gettimeofday(&start, NULL);
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((now.tv_sec - start.tv_sec + now.tv_usec*1e-6))*1000.0;
}

int tcp_connect(const char* server, int port) {
    struct sockaddr_in address;
    int one = 1;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    // Disable delay of sending requests with TCP Nagle algorithm causing extra 50 msec delay
    setsockopt(s, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server);
    address.sin_port = htons(port);
    if (connect(s, (const sockaddr*)&address, sizeof(address)) < 0) {
        perror("Connect failed");
        close(s);
        return -1;
    }
    return s;
}

int control_send(int sock, char *message) {
    char hexbuffer[5] = {0};
    snprintf(hexbuffer, sizeof(hexbuffer), "%04lx", strlen(message));
    write(sock, hexbuffer, 4);
    return write(sock, message, strlen(message));
}

int control_recv(int sock, char *buf, size_t bufsize) {
    char hexbuffer[5] = {0};
    if (read(sock, hexbuffer, 4) != 4) {
        return 0;
    }
    int len = strtol(hexbuffer, NULL, 16);
    if (len > bufsize) {
        fprintf(stderr, "Error: read buffer too small, allocate bigger one\n");
        return 0;
    }
    int remaining = len;
    while (remaining > 0) {
        int n = read(sock, buf, remaining);
        if (n < 0) return 0;
        remaining -= n;
        buf += n;
    }
    return len;
}

static int frameCounter = 0;

int tcp_stream_send(int sock, unsigned char *buf, size_t bufsize) {
    char hexbuffer[64+1] = { 0};
    uint64_t frameCount = frameCounter++;
    uint64_t timeStart = 0;
    uint64_t processTime = 0;
    snprintf(hexbuffer, sizeof(hexbuffer), "img2 %016lx %016lx %016lx %08lx",
        frameCount, 
        timeStart, 
        processTime, 
        bufsize
        );
    #if 0
    char hexbuffer[9] = {0};
    snprintf(hexbuffer, sizeof(hexbuffer), "%08lx", bufsize);
    #endif
    write(sock, hexbuffer, 64);
    return write(sock, buf, bufsize);
}

size_t readbytes(int new_socket, char* buffer, size_t size) {
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

int tcp_stream_recv(int sock, unsigned char *buf, size_t bufsize) {
    char hexbuffer[64+1] = {0};
    if (readbytes(sock, hexbuffer, 64) != 64) {
        return 0;
    }
    char * pEnd = &hexbuffer[5];
    uint64_t framecount = strtol(pEnd, &pEnd, 16);
    uint64_t timestart = strtol(pEnd, &pEnd, 16);
    uint64_t processtime = strtol(pEnd, &pEnd, 16);
    int len = strtol(pEnd, &pEnd, 16);
    if (len > bufsize) {
        fprintf(stderr, "Error: read buffer too small, allocate bigger one\n");
        return 0;
    }
    printf("RECEIVED Frame %d processTime %d jpegSize %d\n", (int)framecount, (int)processtime, (int)len);
    return readbytes(sock, (char*)buf, len);
}

unsigned char *read_image(const char *filename, size_t* size) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return NULL;
    }
    struct stat statbuf;
    fstat(fileno(f), &statbuf);
    unsigned char *buf = (unsigned char *)malloc(statbuf.st_size*2);
    fread(buf, 1, statbuf.st_size, f);
    fclose(f);
    *size = statbuf.st_size;
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s serverip command images...\n", argv[0]);
        return 1;
    }
    char *server = argv[1];
    char *command = argv[2];
    int control_socket = tcp_connect(server, CONTROL_PORT);
    if (control_socket < 0) {
        fprintf(stderr, "Error: Failed to connect to the server\n");
        return 2;
    }
    control_send(control_socket, command);
    char buf[512];
    int n = control_recv(control_socket, buf, sizeof(buf));
    if (n < 0) {
        fprintf(stderr, "Error: Failed to receive response from server\n");
        return 2;
    }
    int stream_socket = tcp_connect(server, STREAM_PORT);
    if (stream_socket < 0) {
        fprintf(stderr, "Error: Failed to connect to tcp stream\n");
        return 2;
    }
    for (int i = 3; i<argc; i++) {
        size_t imgsize = 0;
        unsigned char *img = read_image(argv[i], &imgsize);
        if (img == NULL) {
            fprintf(stderr, "Error: Failed to open image file\n");
            return 3;
        }
        double t1 = current_time_millis();
        tcp_stream_send(stream_socket, img, imgsize);
        n = tcp_stream_recv(stream_socket, img, imgsize*2); // img has space for 2x
        double t2 = current_time_millis();
        printf("Image processing took: %f msec\n", (t2-t1));
        free(img);
    }
    close(stream_socket);
    close(control_socket);
}
