#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 10000000

int main(int argc, char *argv[]) {
    // char buf[] = "Hello\n";
    // ssize_t res = write(1, buf, sizeof(buf));
    // if (res < 0) {
    //     return 1;
    // }

    const char str[] = "Hello\n";

    char *buf = calloc(BUF_SIZE, sizeof(char));
    size_t len = strnlen(str, sizeof(str));
    strncpy(buf, str, len);
    ssize_t res = write(1, buf, len);
    free(buf);
    if (res < 0) {
        return 1;
    }
    return 0;
}
