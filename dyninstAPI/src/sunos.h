#if !defined(sparc_sun_sunos4_1_3)
#error "invalid architecture-os inclusion"
#endif

#ifndef SUNOS_PD_HDR
#define SUNOS_PD_HDR

#include <sys/param.h>
#define EXIT_NAME "_cleanup"

#define START_WALL_TIMER "DYNINSTstartWallTimer"
#define STOP_WALL_TIMER  "DYNINSTstopWallTimer"
#define START_PROC_TIMER "DYNINSTstartProcessTimer"
#define STOP_PROC_TIMER  "DYNINSTstopProcessTimer" 

#endif
