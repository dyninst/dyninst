#ifndef __test1_h
#define __test1_h 1

#define PROT_HELLO 101

#include <sys/time.h>
#include "mrnet/src/utils.h"

class timer{
 public:
  const char * id_str;
  struct timeval _start;
  struct timeval _end;
  timer(const char *_s)
    :id_str(_s){}


  void start(){
    while(gettimeofday(&_start, NULL) == -1);
  }
  void end(){
    while(gettimeofday(&_end, NULL) == -1);
  }
  void print_start(){
    _mc_printf((stderr, "TIME: %s started at %d.%d\n", 
	    id_str, (int)_start.tv_sec, (int)_start.tv_usec));
  }
  void print_end(){
    _mc_printf((stderr, "TIME: %s ended at %d.%d\n", 
	    id_str, (int)_end.tv_sec, (int)_end.tv_usec));
  }
};
#endif /* __test1_h */
