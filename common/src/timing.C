

#include "util/h/Timer.h"
#define NOPS_4  asm("nop"); asm("nop"); asm("nop"); asm("nop")
#define NOPS_16 NOPS_4; NOPS_4; NOPS_4; NOPS_4

double timing_loop(const unsigned TRIES, const unsigned LOOP_LIMIT) {
  const double MILLION = 1.0e6;
  int            i, j;
  double         speed;
  timer stopwatch;
  double max_speed=0;

  for (j=0; j<TRIES; j++) {
    stopwatch.start();
    for (i = 0; i < LOOP_LIMIT; i++) {
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    stopwatch.stop();
    speed   = ((256*LOOP_LIMIT)/stopwatch.usecs())/MILLION;
    stopwatch.clear();
    if (speed > max_speed)
      max_speed = speed;
  }

  for (j=0; j<TRIES; j++) {
    stopwatch.start();
    for (i = 0; i < LOOP_LIMIT; i++) {
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    stopwatch.stop();
    speed   = ((512*LOOP_LIMIT)/stopwatch.usecs())/MILLION;
    stopwatch.clear();
    if (speed > max_speed)
      max_speed = speed;
  }

  for (j=0; j<TRIES; j++) {
    stopwatch.start();
    for (i = 0; i < LOOP_LIMIT; i++) {
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    stopwatch.stop();
    speed   = ((1024*LOOP_LIMIT)/stopwatch.usecs())/MILLION;
    stopwatch.clear();
    if (speed > max_speed)
      max_speed = speed;
  }
  return max_speed;
}
