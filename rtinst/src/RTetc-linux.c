/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: RTetc-linux.c,v 1.34 2006/05/22 14:59:34 tlmiller Exp $ */

/************************************************************************
 * RTetc-linux.c: clock access functions, etc.
************************************************************************/

#include <signal.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <assert.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include <sys/procfs.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "rtinst/h/RThwtimer-linux.h"
#include "thread-compat.h"

#if defined( cap_mmtimer )
#include "common/src/mmtimer.c"
#endif /* defined( cap_mmtimer ) */

/************************************************************************
 * symbolic constants.
************************************************************************/

rawTime64 cpuPrevious_hw  = 0;
rawTime64 cpuPrevious_sw  = 0;
rawTime64 wallPrevious_hw = 0;
rawTime64 wallPrevious_sw = 0;

void PARADYNos_init( int calledByFork, int calledByAttach ) {
	/* We default to SOFTWARE, and if we find something better, use that. */
	hintBestCpuTimerLevel = SOFTWARE_TIMER_LEVEL;
	hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;

	if( isTSCAvail() ) {
		hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;
		}

#if defined( cap_mmtimer )
	if( isMMTimerAvail() ) {   
		// /* DEBUG */ fprintf( stderr, "%s[%d]: runtime library using MMTIMER.\n", __FILE__, __LINE__ );
		hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;
		}
#endif

	/* needs to be reinitialized when fork occurs */
	cpuPrevious_hw = 0;
	cpuPrevious_sw = 0;
	wallPrevious_hw = 0;
	wallPrevious_sw = 0;
	} /* end PARADYNos_init() */

void PARADYN_forkEarlyInit() {
	} /* end PARADYN_forkEarlyInit() */

static unsigned long long mulMillion(unsigned long long in) {
   unsigned long long result = in;

   /* multiply by 125 by multiplying by 128 and subtracting 3x */
   result = (result << 7) - result - result - result;

   /* multiply by 125 again, for a total of 15625x */
   result = (result << 7) - result - result - result;

   /* multiply by 64, for a total of 1,000,000x */
   result <<= 6;

   /* cost was: 3 shifts and 6 subtracts
    * cost of calling mul1000(mul1000()) would be: 6 shifts and 4 subtracts
    *
    * Another algorithm is to multiply by 2^6 and then 5^6.
    * The former is super-cheap (one shift); the latter is more expensive.
    * 5^6 = 15625 = 16384 - 512 - 256 + 8 + 1
    * so multiplying by 5^6 means 4 shift operations and 4 add/sub ops
    * so multiplying by 1000000 means 5 shift operations and 4 add/sub ops.
    * That may or may not be cheaper than what we're doing (3 shifts; 6 subtracts);
    * I'm not sure.  --ari
    */

   return result;
}

/*static int MaxRollbackReport = 0; /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; /* only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


/* --- CPU time retrieval functions --- */
/* Hardware Level ---
   method:      libhrtime get_hrvtime()
   return unit: ticks
*/
rawTime64 DYNINSTgetCPUtime_hw(void) 
{
  static int cpuRollbackOccurred = 0;
  rawTime64 now=0, tmp_cpuPrevious = cpuPrevious_hw;

  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld ticks, using previous value %lld ticks.",
	      tmp_cpuPrevious - now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      PARADYNgenerateTraceRecord(TR_ERROR, 
                                 sizeof(traceData), &traceData, 
                                 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious_hw;
  }
  else  cpuPrevious_hw = now;

  return now;
}

/* Software Level --- 
   method:      times()
   return unit: jiffies  
*/
rawTime64 DYNINSTgetCPUtime_sw(void) 
{
  static int cpuRollbackOccurred = 0;
  rawTime64 now=0, tmp_cpuPrevious = cpuPrevious_sw;
  struct tms tm;
  
  times( &tm );
  now = (rawTime64)tm.tms_utime + (rawTime64)tm.tms_stime;
  
  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld jiffies, using previous value %lld jiffies.",
	      tmp_cpuPrevious - now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      PARADYNgenerateTraceRecord(TR_ERROR, 
                                 sizeof(traceData), &traceData, 
                                 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious_sw;
  }
  else  cpuPrevious_sw = now;
  return now;
}

/* --- Wall time retrieval functions --- */
/* Hardware Level ---
   method:      direct read of TSC (ie. time stamp counter) register
   return unit: ticks
*/
rawTime64 DYNINSTgetWalltime_hw(void) 
{
  static int wallRollbackOccurred = 0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_hw;
  struct timeval tv;

#if defined( cap_mmtimer )
	if( mmdev_clicks_per_tick != 0 ) {
		now = mmdev_clicks_per_tick * (*mmdev_timer_addr);
		}
	else {
		getTSC( now );
		}
#else
	getTSC( now );
#endif

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld ticks, using previous value %lld ticks.",
                tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      PARADYNgenerateTraceRecord(TR_ERROR, 
                                 sizeof(traceData), &traceData, 
                                 1, 1);
    }
    wallRollbackOccurred++;
    wallPrevious_hw = now;
  }
  else  wallPrevious_hw = now;


  return now;
}

/* Software Level --- 
   method:      gettimeofday()
   return unit: microseconds
*/
rawTime64 DYNINSTgetWalltime_sw(void) 
{
  static int wallRollbackOccurred = 0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_sw;
  struct timeval tv;

  if (gettimeofday(&tv,NULL) == -1) {
    perror("gettimeofday");
    assert(0);
    abort();
  }
  
  now = mulMillion( (rawTime64)tv.tv_sec );
  now += (rawTime64)tv.tv_usec;

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld usecs, using previous value %lld usecs.",
                tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      PARADYNgenerateTraceRecord(TR_ERROR, 
                                 sizeof(traceData), &traceData, 
                                 1, 1);
    }
    wallRollbackOccurred++;
    wallPrevious_sw = now;
  }
  else  wallPrevious_sw = now;

  return(now);
}


/* Private refactoring function, should be inline. */
rawTime64 use_times() {
	struct tms tm;
	clock_t status = times( & tm );
	assert( status != -1 );
	// /* DEBUG */ fprintf( stderr, "%s[%d]: times(): utime %lu + stime %lu = cpu time %lu\n", __FILE__, __LINE__, tm.tms_utime, tm.tms_stime, tm.tms_utime + tm.tms_stime );
	return (rawTime64)tm.tms_utime + (rawTime64)tm.tms_stime;
	} /* end use_times() */

/* Private refactoring function, should be inline. */
rawTime64 use_proc_task_stat( int fd ) {
	char procStat[1024], * strPtr;
	int j, numSpaces, status;
	unsigned long utime = (unsigned long)-1, stime = (unsigned long)-1;

	status = lseek( fd, 0, SEEK_SET );
	assert( status != -1 );

	/* FIXME: should have retries in here (atomic reads), for either method. */
	status = read( fd, procStat, 1023 );
	assert( status >= 0 );

#if defined( SLOW_BUT_STEADY )
	status = sscanf( procStat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu ", & utime, & stime );
	assert( status == 2 );
#else
	for( j = 0, numSpaces = 0; j < (status - 1); ++j ) {
		if( procStat[j] == ' ' ) { ++numSpaces; }

		/* Magic constant.  See 'man proc'. */
		if( numSpaces == 13 ) {
			/* We could repeat utime (and get the stime value)
			   if we don't do stime and break the loop right away. */
			utime = strtol( & procStat[j + 1], & strPtr, 10 );
			assert( errno == 0 );
			stime = strtol( strPtr, NULL, 10 );
			assert( errno == 0 );
			break;
			}
		}
	assert( j != numSpaces );
#endif /* defined( SLOW_BUT_STEADY ) */

	// /* DEBUG */ fprintf( stderr, "%s[%d]: proc//task//stat: utime %lu + stime %lu = cpu time %lu\n", __FILE__, __LINE__, utime, stime, utime + stime );
	return (rawTime64)utime + (rawTime64)stime;
	} /* end use_proc_task_stat() */

/* We're trying to minimize the amount of time we spend figuring out
   how much CPU lwp_id has been scheduled for.  On cap_threaded_times
   systems and on Linux 2.4, we can just use times().  Otherwise, we
   can only use /proc//task//stat to get per-thread CPU information.
   However, accessing this is quite slow, so we use times() until
   we have more than one thread. */
rawTime64 DYNINSTgetCPUtime_LWP( unsigned lwp_id, unsigned fd ) {
	/* Threads stay locked to one LWP on Linux. */
	assert( P_lwp_self() == lwp_id );

#if defined( cap_threaded_times )
	return use_times();
#else
	if( fd == -1 ) {
		/* Then we failed to open() /proc//task//stat, and are on Linux 2.4. */
		return use_times();
		}
	
	/* If we're only Linux 2.6 without cap_threaded_times, we switch
	   between methods depending on if we're actually multi-threaded or not. */
	if( DYNINSTthreadCount() > 1 ) {
		return use_proc_task_stat( fd );
		}
	else {
		return use_times();
		}
#endif /* defined( cap_threaded_times ) */
	} /* end DYNINSTgetCPUtime_LWP() */

/* See above.  Note that while '-1' is the normal error value,
   the fd of '0' is also considered special ('unitialized';
   see code in RTthread-timer.c). */
unsigned PARADYNgetFD( unsigned lwp ) {
	char procTaskStat[512];
	int status;
	
	// /* DEBUG */ fprintf( stderr, "%s[%d]: P_lwp_self() = %d\n", __FILE__, __LINE__, P_lwp_self() );
	status = snprintf( procTaskStat, 511, "/proc/%d/task/%d/stat", P_lwp_self(), P_lwp_self() );
	assert(! (status < 0 || status > 511) );

	return open( procTaskStat, O_RDONLY );
	} /* end PARADYNgetFD() */
