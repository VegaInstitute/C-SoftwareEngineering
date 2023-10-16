#include <unistd.h>

int main() {
    char buf[] = "Hello, world!\n";
    write(1, buf, sizeof(buf));
    return 0;
}
