#include <stdio.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#define ERROR(operation, context) \
fprintf(stderr, "[ERROR] Operation: %s | Context: %s | errno: %d\n", operation, context, errno)

#define GAI_ERROR(operation, context, status) \
fprintf(stderr, "[ERROR] Operation: %s | Context: %s | Details: %s\n", operation, context, gai_strerror(status))
volatile sig_atomic_t running = 1;

void signal_handler(int sig)
{
    running = 0;
}

struct connect_response {
    int client_sock;
    int err;
};

struct addrinfo* resolve_address() {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    const int status = getaddrinfo("0.0.0.0", "4433", &hints, &res);
    if (status != 0) {
        GAI_ERROR("getaddrinfo", "Resolving server address (127.0.0.1:4433)", status);
    }
    return res;
}


struct connect_response* connect_to_server(struct addrinfo* addr_info) {

    struct connect_response *resp = malloc(sizeof(struct connect_response));
    resp->err = 1;

    const int sockfd = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (sockfd == -1) {
        ERROR("socket", "Creating client socket");
        freeaddrinfo(addr_info);
        return resp;
    }

    if (connect(sockfd, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        ERROR("connect", "Connecting to server (127.0.0.1:4433)");
        close(sockfd);
        freeaddrinfo(addr_info);
        return resp;
    }

    printf("[INFO] Successfully connected to server!\n");

    resp->err = 0;
    resp->client_sock = sockfd;
    return resp;
}

void send_message(int client_fd,char *message,size_t message_len) {

    ssize_t sent_size = send(client_fd, message, message_len, 0);
    if (sent_size == -1) {
        ERROR("send", "Sending message to server");
    } else {
        printf("[INFO] Message sent: %ld bytes (expected: %ld bytes)\n", sent_size, message_len);
    }
}

int main(void) {

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    struct addrinfo* res_add = resolve_address();
    struct connect_response *resp =  connect_to_server(res_add);

    if (!resp->err) {
        while (true) {
            if (!running)break;
            char message[128];
            scanf("%99s", message);

            send_message(resp->client_sock,message,strlen(message));
        }
    }

    printf("[INFO] Cleaning allocated memory\n");

    free(resp);
    freeaddrinfo(res_add);


    return 0;
}
