#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define fail(msg)    \
    do {             \
        perror(msg); \
        exit(1);     \
    } while (0);

#define MY_SOCK_ADDR "my_socket"
#define BACKLOG 1
#define BUF_SIZE 16

int main(int argc, char* argv[]) {
    int sock_fd;
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) fail("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MY_SOCK_ADDR, sizeof(addr.sun_path) - 1);

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) fail("bind");

    if (listen(sock_fd, BACKLOG) < 0) fail("listen");

    int new_fd;
    size_t addrlen = sizeof(addr);
    if ((new_fd = accept(sock_fd, (struct sockaddr*)&addr,
                         (socklen_t*)&addrlen)) < 0) {
        fail("accept");
    }

    char buf[BUF_SIZE] = {0};
    ssize_t valread;
    if ((valread = read(new_fd, buf, sizeof(buf))) < 0) fail("read");

    if (printf("Read %ld bytes: \"%s\"\n", valread, buf) < 0) fail("printf");

    if (close(new_fd) < 0) fail("close");

    // if (unlink(MY_SOCK_ADDR) < 0) fail("unlink");

    if (shutdown(sock_fd, SHUT_RDWR) < 0) fail("shutdown");

    return 0;
}
