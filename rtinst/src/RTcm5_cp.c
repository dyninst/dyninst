/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a normal Sparc with SUNOS.
 *
 * $Log: RTcm5_cp.c,v $
 * Revision 1.10  1996/03/08 18:48:12  newhall
 * added wall and process time args to DYNINSTgenerateTraceRecord.  This fixes
 * a bug that occured when the appl. is paused between reading a timer to compute
 * a metric value and reading a timer again to compute a header value.
 *
 * Revision 1.9  1996/02/01  17:47:52  naim
 * Fixing some problems related to timers and race conditions. I also tried to
 * make a more standard definition of certain procedures (e.g. reportTimer)
 * across all platforms - naim
 *
 * Revision 1.8  1995/12/10  16:34:57  zhichen
 * Minor cleanup
 *
 * Revision 1.7  1995/10/27  00:59:50  zhichen
 * fixed prototype for DYNINSTnodeCreate
 *
 * Revision 1.6  1995/02/16  09:07:04  markc
 * Made Boolean type RT_Boolean to prevent picking up a different boolean
 * definition.
 *
 * Revision 1.5  1994/07/14  23:36:06  hollings
 * added extra arg to generateTrace.
 *
 * Revision 1.4  1994/07/11  22:47:43  jcargill
 * Major CM5 commit: include syntax changes, some timer changes, removal
 * of old aggregation code, old pause code, added signal-driven sampling
 * within node processes
 *
 * Revision 1.3  1993/09/02  22:10:10  hollings
 * new include syntax.
 *
 * Revision 1.2  1993/07/02  21:53:33  hollings
 * removed unnecessary include files
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>

#include <cm/cmmd/amx.h>
#include <cm/cmmd/mp.h>
#include <cm/cmmd/cn.h>
#include <cm/cmmd/io.h>
#include <cm/cmmd/util.h>
#include <cm/cmmd/cmmd_constants.h>
#include <cm/cmmd.h>
#include <cm/cmna.h>

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
#include <sys/un.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/filio.h>
#include <math.h>

/* now our include files */
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

extern void DYNINSTgenerateTraceRecord(traceStream sid, short type, 
                                      short length, void *eventData, int flush,
				      time64 wall_time,time64 process_time) ;

extern time64 startWall;
extern time64 DYNINSTgetCPUtime();
extern time64 DYNINSTgetWallTime();

extern time64 DYNINSTtotalSampleTime;
float DYNINSTcyclesToUsec;
int DYNINSTnprocs;
int DYNINSTtotalSamples;
time64 DYNINSTlastCPUTime;
time64 DYNINSTlastWallTime;


/*
 * Generate a fork record for each node in the partition.
 *
 */
void DYNINSTnodeCreate()
{
    int sid = 0;
    traceFork forkRec;
    extern int CMNA_partition_size;
    time64 process_time;
    time64 wall_time;

    process_time = DYNINSTgetCPUtime();
    wall_time = DYNINSTgetWallTime();
    wall_time -= startWall;


    forkRec.ppid = getpid();
    if (CMNA_partition_size <= 0) {
	perror("Error in CMMD_partition_size");
    }
    forkRec.pid = forkRec.ppid + MAXPID;
    forkRec.npids = CMNA_partition_size;
    forkRec.stride = MAXPID;
    DYNINSTgenerateTraceRecord(sid, TR_MULTI_FORK, sizeof(forkRec), 
	&forkRec, RT_TRUE,wall_time,process_time);
}

void DYNINSTparallelInit()
{
    /* printf("DYNINSTparallelInit called on the CP\n"); */
/* 	initTraceLibCP(CONTROLLER_FD); */
}

int callFunc(int x)
{
	
	return x ;
}


void DYNINSTreportCounter(intCounter *counter)
{
    traceSample sample;
    time64 process_time;
    time64 wall_time;

    process_time = DYNINSTgetCPUtime();
    wall_time = DYNINSTgetWallTime();
    wall_time -= startWall;

    sample.value = counter->value;
    sample.id = counter->id;
    DYNINSTtotalSamples++;
    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0,
				wall_time, process_time);
}


void DYNINSTreportBaseTramps()
{
    costUpdate sample;
    time64 currentCPU;
    time64 currentWall;
    time64 elapsedWallTime;
    static time64 elapsedPauseTime = 0;

    sample.slotsExecuted = 0;

    /*
    // Adding the cost corresponding to the alarm when it goes off.
    // This value includes the time spent inside the routine (DYNINSTtotal-
    // sampleTime) plus the time spent during the context switch (106 usecs
    // for monona, CM-5)
    */

    sample.obsCostIdeal  = ((((double) DYNINSTgetObservedCycles(1) *
                              (double)DYNINSTcyclesToUsec) + 
                             DYNINSTtotalSampleTime + 106) / 1000000.0);

    currentCPU = DYNINSTgetCPUtime();
    currentWall = DYNINSTgetWallTime();
    elapsedWallTime = currentWall - DYNINSTlastWallTime;
    elapsedPauseTime += elapsedWallTime - (currentCPU - DYNINSTlastCPUTime);
    sample.pauseTime = ((double) elapsedPauseTime);
    sample.pauseTime /= 1000000.0;
    DYNINSTlastWallTime = currentWall;
    DYNINSTlastCPUTime = currentCPU;

    currentWall -= startWall;
    DYNINSTgenerateTraceRecord(0, TR_COST_UPDATE, sizeof(sample), 
	&sample, 0,currentWall,currentCPU);
}

void DYNINSTreportCost(intCounter *counter)
{
    /*
     *     This should eventually be replaced by the normal code to report
     *     a mapped counter???
     */

    double cost;
    int64 value; 
    static double prevCost;
    traceSample sample;
    time64 process_time;
    time64 wall_time;

    process_time = DYNINSTgetCPUtime();
    wall_time = DYNINSTgetWallTime();
    wall_time -= startWall;


    value = DYNINSTgetObservedCycles(1);

    cost = ((double) value) * (DYNINSTcyclesToUsec / 1000000.0);

    if (cost < prevCost) {
	fprintf(stderr, "Fatal Error Cost counter went backwards\n");
	fflush(stderr);
	abort();
    }

    prevCost = cost;

    /* temporary until CM-5 aggregation code can handle avg operator */
    if (DYNINSTnprocs) cost /= DYNINSTnprocs;

    sample.value = cost;
    sample.id = counter->id;
    DYNINSTtotalSamples++;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0,
				wall_time,process_time);

}
