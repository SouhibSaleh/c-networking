
#include <stdio.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

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




    freeaddrinfo(res);
    return 0;
}
