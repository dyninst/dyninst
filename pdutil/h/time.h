#ifndef time_H
#define time_H

#include <sys/time.h>
#include "sys.h"

timeStamp getCurrentTime(void) {
    struct timeval tv;
    assert(gettimeofday(&tv, NULL) == 0); // 0 --> success; -1 --> error

    double seconds_dbl = tv.tv_sec * 1.0;
    assert(tv.tv_usec < 1000000);
    double useconds_dbl = tv.tv_usec * 1.0;
  
    seconds_dbl += useconds_dbl / 1000000.0;

    return seconds_dbl;
}

#endif
