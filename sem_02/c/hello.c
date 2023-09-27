#include <unistd.h>

int main() {
  char s[] = "Hello, World!\n";
  ssize_t res = write(1, s, sizeof(s));
  return 0;
}
