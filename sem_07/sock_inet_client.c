#include <arpa/inet.h>  // inet_ntop
#include <memory.h>     // memset
#include <netdb.h>      // |
#include <stdio.h>      // fprintf
#include <stdlib.h>     // EXIT_*
#include <string.h>
#include <sys/socket.h>  // |
#include <sys/types.h>   // | getaddrinfo, socket, etc
#include <unistd.h>

#define BUF_SZ 16384

int main(int argc, char *argv[]) {
    // check argumens
    if (argc < 2) {
        fprintf(stderr, "hostname must be specified as a positional argument");
        exit(EXIT_FAILURE);
    }

    // resolve hostname
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    struct addrinfo *servinfo;  // will point to the result
    int status = getaddrinfo(argv[1], "http", &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // traverse linked list and try to connect
    int conn_success_flag = 0;
    char ipstr[INET6_ADDRSTRLEN];
    int sock_fd;
    struct addrinfo *p;
    struct addrinfo *res;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        if (p->ai_family == AF_INET) {  // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {  // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));

        // already connected, just list other variants
        if (conn_success_flag) {
            printf("other available %s: %s\n", ipver, ipstr);
            continue;
        }

        // get socket
        printf("trying %s: %s", ipver, ipstr);
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <
            0) {
            printf(" FAIL\n");
            perror("socket");
            continue;
        }

        // connect
        int conn_status;
        if ((conn_status = connect(sock_fd, p->ai_addr, p->ai_addrlen)) < 0) {
            printf(" FAIL\n");
            perror("connect");
            continue;
        }

        printf(" SUCCESS\n");
        conn_success_flag = 1;
        res = p;
    }

    if (!conn_success_flag) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);  // missing C++ dtors that do it automatically...

    char request[BUF_SZ] = {0};
    char *req_type;
    if (argc < 3) {
        req_type = "HEAD";
    } else {
        req_type = argv[2];
    }

    snprintf(request, BUF_SZ - 1, "%s / HTTP/1.1\nHost: %s\n\n", req_type,
             argv[1]);
    printf("\n\nSending the request:\n----------\n%s----------\n", request);
    write(sock_fd, request, strlen(request));

    char resp[BUF_SZ] = {0};
    read(sock_fd, resp, sizeof(resp));

    printf("\n\nRecieved a response:\n----------\n%s----------\n", resp);
    return EXIT_SUCCESS;
}
