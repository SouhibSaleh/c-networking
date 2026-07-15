#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
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

    const int status = getaddrinfo("127.0.0.1", "4433", &hints, &res);
    if (status != 0) {
        GAI_ERROR("getaddrinfo", "Resolving server address (127.0.0.1:4433)", status);
        return NULL;
    }
    return res;
}


int get_client_fd(struct addrinfo* addr_info) {


    int sockfd = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (sockfd == -1) {
        ERROR("socket", "Creating client socket");
        freeaddrinfo(addr_info);
        return sockfd;
    }
    printf("[INFO] Ready To use  to server!\n");

    return sockfd;
}

int main(void) {

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    struct addrinfo* res_add = resolve_address();
    if (res_add == NULL) {
        return 0;
    }
    int client_fd =  get_client_fd(res_add);

    if (client_fd != -1) {
        while (running) {

            char message[128];
            scanf("%99s", message);

            ssize_t sent_size = sendto(client_fd, message, strlen(message), 0,res_add->ai_addr,res_add->ai_addrlen);
            if (sent_size == -1) {
                ERROR("send", "Sending message to server");
            } else {
                printf("[INFO] Message sent: %zd bytes (message length: %zu bytes)\n",sent_size,strlen(message));
            }

        }
        printf("[INFO] Cleaning allocated memory\n");

        close(client_fd);
        freeaddrinfo(res_add);
    }




    return 0;
}
