#include "tv.h"
#include <stdio.h>

double total_variance(double vola, double dt) {
#ifdef _DEBUG
  fprintf(stderr, "%lf %lf\n", vola, dt);
#endif
  /* total variance in BSM model */
  return vola * vola * dt;
}
