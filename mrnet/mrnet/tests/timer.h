#ifndef __timer_h
#define __timer_h 1

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
    mc_printf(MCFL, stderr, "TIME: %s started at %d.%d\n", 
	    id_str, (int)_start.tv_sec, (int)_start.tv_usec);
  }
  void print_end(){
    mc_printf(MCFL, stderr, "TIME: %s ended at %d.%d\n", 
	    id_str, (int)_end.tv_sec, (int)_end.tv_usec);
  }
};

class mb_time{
 private:
    struct timeval tv;
    double d;
    void set_timeval_from_double() {
        //char tmp[128];
        assert(d != -1);
        tv.tv_sec = (long)d;
        tv.tv_usec = (long)((d - ((double)tv.tv_sec)) * 1000000);
        //sprintf(tmp, "%lf", d);
        //sscanf(tmp, "%ld.%ld", &(tv.tv_sec), &(tv.tv_usec) );
    }
    void set_double_from_timeval(){
        //char tmp[128];
        assert(tv.tv_usec != -1);
        d = (double(tv.tv_sec)) + ((double)tv.tv_usec) / 1000000.0;
        //sprintf(tmp, "%ld.%ld", tv.tv_sec, tv.tv_usec );
        //sscanf(tmp, "%lf", &d);
    }

 public:
    mb_time() :d(-1.0) {
        tv.tv_sec = -1;
        tv.tv_usec = 0;
    }
    void set_time(){
        while( gettimeofday(&tv, NULL) == -1) ;
        d = -1.0;
    }
    void set_time(double _d){
        d = _d;
        tv.tv_sec = -1;  //only set on demand for efficiency
    }
    void set_time(struct timeval _tv){
        tv = _tv;
        d = -1.0;  //only set on demand for efficiency
    }
    void get_time(double *_d){
        if(d != -1.0){
            set_double_from_timeval();
        }
        *_d = d;
    }
    double get_double_time(){
        if(d == -1.0){
            set_double_from_timeval();
        }
        return d;
    }
    void get_time(struct timeval *_tv){
        if(tv.tv_sec == -1.0){
            set_timeval_from_double();
        }
        *_tv = tv;
    }
    mb_time operator-(mb_time& _t){
        mb_time retval;
        double new_d = get_double_time() - _t.get_double_time();
        retval.set_time(new_d);
        return retval; 
    }
    void operator-=(double _d){
        if(d == -1){
            set_double_from_timeval();
        }
        d -= _d;
        tv.tv_sec = -1;
    }
};

#endif /* __timer_h */
