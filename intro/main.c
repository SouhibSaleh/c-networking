
#include <stdio.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main(void) {
    short s_port = 8080;
    short l_port = 8080;

    int host_to_network_short = htons(s_port);
    int host_to_network_long = htonl(s_port);
    int network_to_host_short = ntohs(host_to_network_short);
    int network_to_host_long = ntohs(host_to_network_long);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    struct addrinfo *res;

    int status = 0;
    if ((status = getaddrinfo("www.google.com", "80", &hints, &res)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }

    for (struct addrinfo *a = res; a != NULL; a = a->ai_next) {
        char address[INET6_ADDRSTRLEN];

        if (a->ai_family == AF_INET) {
            const struct sockaddr_in* ip4 = (struct sockaddr_in*)a->ai_addr;
            if (inet_ntop(AF_INET, &ip4->sin_addr, address, sizeof(address)) == NULL) {
                perror("inet_ntop");
            } else {
                printf("address: %s , canonical name: %s, port: %d\n", address, a->ai_canonname, ntohs(ip4->sin_port));
            }
        }else if (a->ai_family == AF_INET6) {
            const struct sockaddr_in6* ip6= (struct sockaddr_in6*)a->ai_addr;
            if (inet_ntop(AF_INET6, &ip6->sin6_addr, address, sizeof(address)) == NULL) {
                perror("inet_ntop");
            } else {
                printf("address: %s , canonical name: %s, port: %d\n", address, a->ai_canonname, ntohs(ip6->sin6_port));
            }
        }

    }

    freeaddrinfo(res);


    return 0;
}
