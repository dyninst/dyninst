/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#ifndef __timer_h
#define __timer_h 1

#include <sys/time.h>
#include "utils.h"

using namespace MRN;

class Timer{
 public:
  struct timeval _start;
  struct timeval _stop;

  void start(){
    while(gettimeofday(&_start, NULL) == -1);
  }

  void stop(){
    while(gettimeofday(&_stop, NULL) == -1);
  }

  unsigned int get_latency_usecs( ) {
      float s = (float(_start.tv_sec)) + ((float)_start.tv_usec) / 1000000.0;
      float e = (float(_stop.tv_sec)) + ((float)_stop.tv_usec) / 1000000.0;
      return ((e-s)*1000000.0);
  }
};

#endif /* __timer_h */
