#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_LEN 32

int main() {
  char s1[] = "Hello, World!";
  char *s2 = s1;
  s1[0] = 'h';

  /*
  char *s2 = calloc(STR_LEN, sizeof(char));
  if (s2 == NULL)
    return EXIT_FAILURE;

  char *src = s1;
  char *dst = s2;
  while (*dst++ = *src++)
    ;

  printf("%s\n", s1);
  printf("%s\n", s3);
  */

  free(s2);

  return EXIT_SUCCESS;
}
