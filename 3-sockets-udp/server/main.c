
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define ERROR(operation, context) \
    fprintf(stderr, "[ERROR] Operation: %s | Context: %s | errno: %d\n", operation, context, errno)

#define GAI_ERROR(operation, context, status) \
    fprintf(stderr, "[ERROR] Operation: %s | Context: %s | Details: %s\n", operation, context, gai_strerror(status))

volatile sig_atomic_t running = 1;

void signal_handler(int sig)
{
    running = 0;
}

struct addrinfo* resolve_address() {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int status = 0;
    if ((status = getaddrinfo(NULL, "4433", &hints, &res)) != 0) {
        GAI_ERROR("getaddrinfo", "Setting up server address info on port 4433", status);
        return NULL;
    }
    printf("[INFO] Address info configured: domain=%d, type=%d, protocol=%d\n",
           res->ai_family, res->ai_socktype, res->ai_protocol);

    return res;
}


int main(void) {

    pthread_t handle_thread;

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    struct addrinfo* res = resolve_address();
    if (res == NULL) {
        return 0;
    }

    const int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd == -1) {
        ERROR("socket", "Creating server socket");
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Server socket created successfully (fd=%d)\n", sd);

    const int bind_err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (bind_err == -1) {
        ERROR("bind", "Binding socket to port 4433");
        close(sd);
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Socket bound to port 4433\n");


    printf("[INFO] Waiting for incoming messages...\n");
    while (running) {
        struct sockaddr_storage req_addr;
        socklen_t req_addr_size = sizeof req_addr;

        ssize_t n;
        char buffer[4];
        n = recvfrom(sd, buffer, sizeof(buffer)-1, 0,(struct sockaddr*)&req_addr ,&req_addr_size);
        if (n>0){
            buffer[n] = '\0';
            printf("Received: %s\n", buffer);
        }


    }

    printf("[INFO] Cleaning allocated memory\n");

    close(sd);
    freeaddrinfo(res);

    return 0;
}
