/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a normal Sparc with SUNOS.
 *
 * $Log: RTcm5_cp.c,v $
 * Revision 1.13  1996/08/16 21:27:28  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.12  1996/04/09 22:20:50  newhall
 * changed DYNINSTgetWallTime to DYNINSTgetWalltime to fix undefined symbol
 * errors when applications are linked with libdyninstRT_cp.a
 *
 * Revision 1.11  1996/04/09  15:52:36  naim
 * Fixing prototype for procedure DYNINSTgenerateTraceRecord and adding
 * additional parameters to a call to this function in RTtags.c that has these
 * parameters missing - naim
 *
 * Revision 1.10  1996/03/08  18:48:12  newhall
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

extern time64 startWall;
extern time64 DYNINSTgetCPUtime();
extern time64 DYNINSTgetWalltime();

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
    wall_time = DYNINSTgetWalltime();
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
    wall_time = DYNINSTgetWalltime();
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
    currentWall = DYNINSTgetWalltime();
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
    wall_time = DYNINSTgetWalltime();
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
