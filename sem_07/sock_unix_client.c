#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUF_SIZE 32
#define MY_SOCK_ADDR "my_socket"

#define fail(msg)           \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int main(int argc, char *argv[]) {
    int sock_fd;
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) fail("socket failed");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MY_SOCK_ADDR, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        fail("connect failed");

    char buf[BUF_SIZE] = {0};
    if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) fail("fgets failed");

    size_t len = strnlen(buf, sizeof(buf));
    int valwrite = write(sock_fd, buf, len);
    if (valwrite < 0) fail("write failed");
    if (valwrite < len)
        fprintf(stderr, "Written %d/%zu bytes\n", valwrite, len);

    if (close(sock_fd) < 0) fail("close failed");

    return EXIT_SUCCESS;
}
