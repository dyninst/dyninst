#if !defined(hppa1_1_hp_hpux)
#error "invalid architecture-os inclusion"
#endif

#ifndef HPUX_PD_HDR
#define HPUX_PD_HDR

#include <sys/param.h>
#define EXIT_NAME "_exitcu"
//#define EXIT_NAME "_exit"


#define START_WALL_TIMER "DYNINSTstartWallTimer_hpux"
#define STOP_WALL_TIMER  "DYNINSTstopWallTimer_hpux"
#define START_PROC_TIMER "DYNINSTstartProcessTimer_hpux"
#define STOP_PROC_TIMER  "DYNINSTstopProcessTimer_hpux" 

#endif
