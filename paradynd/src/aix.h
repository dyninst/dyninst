#if !defined(rs6000_ibm_aix3_2)
#error "invalid architecture-os inclusion"
#endif

#ifndef AIX_PD_HDR
#define AIX_PD_HDR

#include <sys/param.h>
#define EXIT_NAME "exit"

extern unsigned AIX_TEXT_OFFSET_HACK;
extern unsigned AIX_DATA_OFFSET_HACK;

#endif
