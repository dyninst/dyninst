#if !defined(sparc_sun_solaris2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef SOLARIS_PD_HDR
#define SOLARIS_PD_HDR

#include <sys/param.h>
#define EXIT_NAME "exit"

#define START_WALL_TIMER "DYNINSTstartWallTimer"
#define STOP_WALL_TIMER  "DYNINSTstopWallTimer"
#define START_PROC_TIMER "DYNINSTstartProcessTimer"
#define STOP_PROC_TIMER  "DYNINSTstopProcessTimer" 

#endif
