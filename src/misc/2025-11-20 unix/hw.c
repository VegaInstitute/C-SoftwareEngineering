#include <unistd.h>

char str[] = "Hello, World!\n";

int main(void) {
    ssize_t res = write(1, str, sizeof(str));
    if (res != sizeof(str)) {
        return 1;
    }
    return 0;
}