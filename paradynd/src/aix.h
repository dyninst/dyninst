#if !defined(rs6000_ibm_aix3_2) && !defined(rs6000_ibm_aix4_1)
#error "invalid architecture-os inclusion"
#endif

#ifndef AIX_PD_HDR
#define AIX_PD_HDR

#include <sys/param.h>
#define EXIT_NAME "exit"

#define START_WALL_TIMER "DYNINSTstartWallTimer"
#define STOP_WALL_TIMER  "DYNINSTstopWallTimer"
#define START_PROC_TIMER "DYNINSTstartProcessTimer"
#define STOP_PROC_TIMER  "DYNINSTstopProcessTimer" 


extern unsigned AIX_TEXT_OFFSET_HACK;
extern unsigned AIX_DATA_OFFSET_HACK;

#endif
