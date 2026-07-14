
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


void *handle_server_request(void *arg) {
    pthread_detach(pthread_self());
    char buffer[4];
    int *client_fd = arg;

    ssize_t n;
    while ((n = recv(*client_fd, buffer, sizeof(buffer)-1, 0))>0 && running) {
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);
    }

    close(*client_fd);
    free(client_fd);
    return NULL;
}


struct addrinfo* resolve_address() {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
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

    const int listen_err = listen(sd, 3);
    if (listen_err == -1) {
        ERROR("listen", "Setting up listen queue (backlog=3)");
        close(sd);
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Socket listening with backlog=3\n");



    printf("[INFO] Waiting for incoming connection...\n");
    while (running) {
        int *client_fd = malloc(sizeof(int));
        struct sockaddr_storage req_addr;
        socklen_t req_addr_size = sizeof req_addr;

        *client_fd = accept(sd, (struct sockaddr *)&req_addr, &req_addr_size);

        if (!running) {
            if (*client_fd != -1) close(*client_fd);
            free(client_fd);
            break;
        };

        if (*client_fd == -1) {
            ERROR("accept", "Accepting incoming connection");
            close(sd);
            freeaddrinfo(res);
            free(client_fd);
            exit(1);
        }
        printf("[INFO] Accepted connection (new fd=%d)\n", *client_fd);

        void *p = client_fd;
        int p_err = pthread_create(
            &handle_thread,
            NULL,
            handle_server_request,
            p
            );

        if (p_err != 0) {
            ERROR("thread", "Handling new request");
            close(*client_fd);
            free(client_fd);
        }
    }

    printf("[INFO] Cleaning allocated memory\n");

    close(sd);
    freeaddrinfo(res);

    return 0;
}
