
#include <stdio.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

void connect_to_server() {
    struct addrinfo hints, *res;
    int sockfd;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo("127.0.0.1", "4433", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        freeaddrinfo(res);
        return;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        close(sockfd);
        freeaddrinfo(res);
        return;
    }

    printf("Successfully connected!\n");

    const char *msg = "message";

    ssize_t sent_size = send(sockfd, msg, strlen(msg), 0);

    printf("%ld, %ld",sent_size,strlen(msg));

    close(sockfd);
    freeaddrinfo(res);
}


int main(void) {


    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    int status = 0;
    if ((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }

    printf("domain: %d, type: %d, protocol: %d\n",res->ai_family, res->ai_socktype, res->ai_protocol);
    const int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sd == -1) {
        perror("socket failed");
    }

    const int bind_err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (bind_err == -1) {
        perror("bind error");
    }
    const int listen_err = listen(sd,3);
    if (listen_err == -1) {
        perror("listen error");
    }

    struct sockaddr_storage req_addr;
    socklen_t req_addr_size = sizeof req_addr;

    int new_fd = accept(sd,(struct sockaddr *)&req_addr,
                                                       &req_addr_size);

    close(sd);
    freeaddrinfo(res);
    return 0;
}
