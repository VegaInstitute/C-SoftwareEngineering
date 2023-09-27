#include "tv.h"
#include <stdio.h>

#define fail(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main() {
  double dt, vola;
  scanf("%lf%lf", &vola, &dt);
  printf("%lf\n", total_variance(vola, dt));
  return 0;
}
