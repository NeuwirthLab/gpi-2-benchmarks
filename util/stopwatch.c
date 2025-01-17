#include "stopwatch.h"

double
Wtime()
{
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return (double)(time.tv_sec * 1e9 + time.tv_nsec) * 1e-3;
}
