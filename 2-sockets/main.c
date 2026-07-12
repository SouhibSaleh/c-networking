
#include <stdio.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define ERROR(operation, context) \
    fprintf(stderr, "[ERROR] Operation: %s | Context: %s | errno: %d\n", operation, context, errno)

#define GAI_ERROR(operation, context, status) \
    fprintf(stderr, "[ERROR] Operation: %s | Context: %s | Details: %s\n", operation, context, gai_strerror(status))

void connect_to_server() {
    struct addrinfo hints, *res;
    int sockfd;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo("127.0.0.1", "4433", &hints, &res);
    if (status != 0) {
        GAI_ERROR("getaddrinfo", "Resolving server address (127.0.0.1:4433)", status);
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        ERROR("socket", "Creating client socket");
        freeaddrinfo(res);
        return;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        ERROR("connect", "Connecting to server (127.0.0.1:4433)");
        close(sockfd);
        freeaddrinfo(res);
        return;
    }

    printf("[INFO] Successfully connected to server!\n");

    const char *msg = "message";

    ssize_t sent_size = send(sockfd, msg, strlen(msg), 0);
    if (sent_size == -1) {
        ERROR("send", "Sending message to server");
    } else {
        printf("[INFO] Message sent: %ld bytes (expected: %ld bytes)\n", sent_size, strlen(msg));
    }

    close(sockfd);
    freeaddrinfo(res);
}


int main(void) {

    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = 0;
    if ((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0) {
        GAI_ERROR("getaddrinfo", "Setting up server address info on port 3490", status);
        exit(1);
    }
    printf("[INFO] Address info configured: domain=%d, type=%d, protocol=%d\n",
           res->ai_family, res->ai_socktype, res->ai_protocol);

    const int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd == -1) {
        ERROR("socket", "Creating server socket");
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Server socket created successfully (fd=%d)\n", sd);

    const int bind_err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (bind_err == -1) {
        ERROR("bind", "Binding socket to port 3490");
        close(sd);
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Socket bound to port 3490\n");

    const int listen_err = listen(sd, 3);
    if (listen_err == -1) {
        ERROR("listen", "Setting up listen queue (backlog=3)");
        close(sd);
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Socket listening with backlog=3\n");

    struct sockaddr_storage req_addr;
    socklen_t req_addr_size = sizeof req_addr;

    printf("[INFO] Waiting for incoming connection...\n");
    int new_fd = accept(sd, (struct sockaddr *)&req_addr, &req_addr_size);
    if (new_fd == -1) {
        ERROR("accept", "Accepting incoming connection");
        close(sd);
        freeaddrinfo(res);
        exit(1);
    }
    printf("[INFO] Accepted connection (new fd=%d)\n", new_fd);

    close(new_fd);
    close(sd);
    freeaddrinfo(res);
    return 0;
}
