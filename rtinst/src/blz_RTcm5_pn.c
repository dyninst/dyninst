#include <stdlib.h>
/* #include <signal.h> */
#include <assert.h>

/* our include files */
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#define extern
#include "traceio.h"
#undef extern

#include <cm/timers.h>
#include <cm/cmmd/amx.h>
#include <cm/cmmd/mp.h>
#include <cm/cmmd/cn.h>
#include <cm/cmmd/io.h>
#include <cm/cmmd/util.h>
#include <cm/cmmd/cmmd_constants.h>
#include <cm/cmmd.h>
#include <cmsys/cm_signal.h>
#define pe_obj
#include <cm/cmna.h>
#include <cmsys/ni_interface.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stdtypes.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <cm/cm_file.h>
#include <cm/cm_errno.h>
#include <errno.h>

/* The following two wall time is defined to pass gettimeofday value from the CP daemon*/
/* TS_wall is when the TS daemon is started                                            */
/* -zxu 10/21/95                                                                       */
CM_TIME TS_wall ;          /* TS wall time, ptraced by the node daemon */
char    CP_wall[8] ;       /* CP wall time, ptraced by the node daemon */

/*
CMOS_get_time returns the amount of wall-clock time that has
elapsed since the ts-daemon started to run. The time is
returned in CM_TIME format, and must be converted to seconds
by calling CM_ni_time_in_sec(time_value).
typedef unsigned int     CM_TIME[2];
CM_TIME[1] is the number of times the CM_TIME[0] wrapped.
*/

void CPgettimeofday(struct timeval *tv) 
{/* copy the CP wall time */
	memcpy(tv, CP_wall, sizeof(struct timeval)) ;
}
void  DaemonCMOS_get_time(time64 *tv)
{/* copy the daemon TS time when CPgettimeofday is obtained */
	memcpy(tv, TS_wall, sizeof(time64)) ;
}

#define gettimeofday(x,y) CPgettimeofday(x)
/* Stuff from RTcm5_pn.c  */

/* stuff that was static class */
extern volatile unsigned int *ni;
extern int DYNINSTinitDone;
extern time64 startWall;       /*time64 is long long int */


/* stuff that was copied */
extern tTimer DYNINSTelapsedCPUTime;
extern volatile struct timer_buf timerBuffer;
extern time64 DYNINSTlastCPUTime;
extern time64 DYNINSTlastWallTime;
extern float DYNINSTsamplingRate;

extern float DYNINSTcyclesToUsec;
extern time64 DYNINSTelapsedTime;

#define MILLION 1000000
#define NI_CLK_USEC 33


time64 DYNINSTgetWallTime();
time64 DYNINSTgetCPUtime();
void DYNINSTstartProcessTimer(tTimer *);
void RecurringBlizzardAlarm(int, void (*)());
void PD_set_timer_buf(volatile struct timer_buf *param);

/*
 * should be called before main in each node  process.
 *
 */
void blzDYNINSTinit()
{
    extern int DYNINSTnprocs;
    char *interval;
    struct itimerval timeInterval;
    struct timeval tv;
    int sampleInterval ;
    time64 startNItime;
    extern void DYNINSTalarmExpire();
    extern int DYNINSTsampleMultiple;

    /* temporary correction until we can make the CM-5 aggregation code
       perform a max operation in addition to sum - jk 10/19/94 */
    DYNINSTnprocs = 32;
    /*
    {
	struct itimerval xinterval;
	CMOS_getitimer (1, &xinterval);
	fprintf(stderr, "interval.it_interval = %li\n", xinterval.it_interval.tv_usec) ;
	fprintf(stderr, "interval.it_value    = %li\n", xinterval.it_value.tv_usec) ;
    }
    */

    /*printf("blzDYNINSTinit has been called\n");*/

    DaemonCMOS_get_time(&startNItime);
    gettimeofday(&tv, NULL);

    startWall = tv.tv_sec;
    startWall *= MILLION;
    startWall += tv.tv_usec;

    /* change time base to ni time */
    startWall *= NI_CLK_USEC;

    startWall -= startNItime;

/*     printf ("startWall is %x%x\n", *(int *) &startWall,  */
/* 	    *((int *) &startWall) + 1); */

    ni = (unsigned int *) NI_TIME_A;
    PD_set_timer_buf(&timerBuffer);

    if (getenv("DYNINSTsampleMultiple")) {
	DYNINSTsampleMultiple = atoi(getenv("DYNINSTsampleMultiple"));
    }

   /*printf("DYNINSTsampleMultiple=%d\n", DYNINSTsampleMultiple) ;*/

    /*
     * Allocate a trace buffer for this node, and set up the buffer
     * pointers.
     */
    TRACELIBcurrPtr = TRACELIBfreePtr = TRACELIBtraceBuffer = (char *) malloc (TRACE_BUF_SIZE);
    TRACELIBendPtr = TRACELIBtraceBuffer + TRACE_BUF_SIZE - 1;

    /*printf("in blzDYNINSTinit, TRACELIBtraceBuffer = %d\n", TRACELIBtraceBuffer) ; */


    /* init these before the first alarm can expire */
    DYNINSTlastCPUTime = DYNINSTgetCPUtime();
    DYNINSTlastWallTime = DYNINSTgetWallTime();

    /*
     * Set up the SIGALRM handler stuff so counters/timers get sampled and
     * traces get puit into the traceBuffer every once in a while.
     */

    sampleInterval = 500000;     /* default is 500msec */ 
    interval = (char *) getenv("DYNINSTsampleInterval");
    if (interval) {
	sampleInterval = atoi(interval);
    }
    /*printf("DYNINSTinitBlizzard past sampleInterval 3 sample interval %d\n", sampleInterval);*/
    DYNINSTsamplingRate = ((float) sampleInterval)/ 1000000.0;
    RecurringBlizzardAlarm(sampleInterval, DYNINSTalarmExpire);
    DYNINSTcyclesToUsec = 1.0/((float) NI_CLK_USEC);

    CMOS_get_time(&DYNINSTelapsedTime);
    DYNINSTstartProcessTimer(&DYNINSTelapsedCPUTime);

    DYNINSTinitDone = 1;

}
