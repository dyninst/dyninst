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

/************************************************************************
 * RTwinnt.c: runtime instrumentation functions for Windows NT
 *
 * $Id: RTetc-winnt.c,v 1.16 2004/05/11 19:02:10 bernat Exp $
 *
 ************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <errno.h>
#include <limits.h>


#include "rtinst/h/trace.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/RThwtimer-winnt.h"


static HANDLE DYNINSTprocHandle;  /*  ccw 4 jun 2002*/
void
DYNINSTbreakPoint(void) {
  /* TODO: how do we stop all threads? */
  DebugBreak();
}


void DYNINSTos_init(int calledByFork, int calledByAttach)
{
#ifndef mips_unknown_ce2_11 //ccw 23 july 2001
  RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
#endif
}
/* end  ccw 4 jun 2002*/
void PARADYNos_init(int calledByFork, int calledByAttach) {
  RTprintf("PARADYNos_init(%d,%d)\n", calledByFork, calledByAttach);
  DYNINSTprocHandle = GetCurrentProcess();
  hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
    hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;
    if(isTSCAvail()) { 
      hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;   
    }
    else {
      hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;
    }
}

void PARADYN_forkEarlyInit() {
}

void
PARADYNbreakPoint(void) {
  DYNINSTbreakPoint();
  /* On other (posix) platforms, we follow with DYNINSTgenerateTraceRecord
     but curiously not here. */
}

unsigned DYNINSTgetCurrentThreadId() {
  return GetCurrentThreadId();
}

#ifndef SHM_SAMPLING
void CALLBACK DYNINSTalarmExpireCallback(UINT id, UINT msg, 
				 DWORD usr, DWORD w1, DWORD w2) {
  void DYNINSTalarmExpire();
  DYNINSTalarmExpire();
}

/************************************************************************
 * void DYNINST_install_ualarm(unsigned interval)
 *
 * install an alarm with period 'interval' microseconds
************************************************************************/

void DYNINST_install_ualarm(unsigned interval) {
  if (timeSetEvent(interval/1000, 100, DYNINSTalarmExpireCallback, 
		   0, TIME_PERIODIC) == (int)NULL) {
    fprintf( stderr, "DYNINST: unable to create periodic timer\n");
    abort();
  }

}
#endif /* SHM_SAMPLING */


#define FILETIME2rawTime64(FT) \
   (((rawTime64)(FT).dwHighDateTime<<32) | ((rawTime64)(FT).dwLowDateTime))

/*static int MaxRollbackReport = 0; /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; /* only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
rawTime64 
DYNINSTgetCPUtime_hw(void) {
  return 0;
}

/* Software Level ---
   method:      GetProcessTimes()
   return unit: 100 ns resolution
*/
rawTime64
DYNINSTgetCPUtime_sw(void) {
  FILETIME kernelT, userT, creatT, exitT;
  static rawTime64 cpuPrevious=0;
  static int cpuRollbackOccurred=0;
  rawTime64 now, tmp_cpuPrevious=cpuPrevious;
  static HANDLE procHandle;
  HANDLE thrHandle = GetCurrentThread();
  DWORD timerTries = 0;
  DWORD maxTimerTries = 5;
  DWORD gttRet = 0;

  // Originally, we only called GetThreadTimes once here.
  // However, we observed that the GetThreadTimes call sometimes failed,
  // but didn't give us any good indication why.  (GetLastError() returned
  // strange values like 0xbaadf00d, 0xfeeefeee, and 0xffffffff that are
  // not documented values for errors.)  Retrying the call has succeeded
  // in every case we've tried.  To be safe, we allow a small number of
  // retries in case of failure.
  // 
  do
  {
      ZeroMemory( &kernelT, sizeof(kernelT) );
      ZeroMemory( &userT, sizeof(userT) );
      ZeroMemory( &creatT, sizeof(creatT) );
      ZeroMemory( &exitT, sizeof(exitT) );
      
      gttRet = GetThreadTimes(thrHandle, &creatT, &exitT, &kernelT,&userT);
      timerTries++;
    } while( (gttRet == 0) && (timerTries < maxTimerTries) );
    if( (gttRet == 0) && (timerTries == maxTimerTries) )
    {
        fprintf( stderr, "RT: GetThreadTimes failed too many times: %08x\n",
            GetLastError() );
        fflush( stderr );
        abort();
    }

  now = (FILETIME2rawTime64(userT)+FILETIME2rawTime64(kernelT));

  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"CPU time rollback %I64d with current time: "
	      "%I64d raw units, using previous value %I64d raw units.",
	      tmp_cpuPrevious-now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious;
  }
  else  cpuPrevious = now;

  return now;
}

/* --- Wall time retrieval functions --- */
/* Hardware Level ---
   method:      rdtsc
   return unit: cycles
*/

rawTime64
DYNINSTgetWalltime_hw(void) {
  static rawTime64 wallPrevious=0;
  static int wallRollbackOccurred=0;
  rawTime64 now, tmp_wallPrevious=wallPrevious;
  
  getTSC(now);
  
  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %I64d with current time:"
	      " %lld ticks, using previous value %I64d ticks.",
	      tmp_wallPrevious-now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 
				 1, 1, 1);
    }
    wallRollbackOccurred++;
    wallPrevious = now;
  }
  else  wallPrevious = now;

  return now;
}

/* Software Level ---
   method:      QueryPerformanceCounter()
   return unit: resolution discovered at runtime, typically at cycle rate res
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  LARGE_INTEGER time;

  static rawTime64 wallPrevious=0;
  static int wallRollbackOccurred=0;
  rawTime64 now, tmp_wallPrevious=wallPrevious;

  if (QueryPerformanceCounter(&time)) {
    /* dividing time by freq gives a value is seconds.
       we need to multiply by 1000000.0 to get microseconds
       */
    now = (rawTime64)time.QuadPart;
  } else {
    fprintf( stderr, "RT: DYNINSTgetWalltime_sw calling abort\n" );
    abort();
  }

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %I64d with current time:"
	      " %I64d raw units, using previous value %I64d raw units.",
	      tmp_wallPrevious-now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    wallRollbackOccurred++;
    now = wallPrevious;
  }
  else  wallPrevious = now;

  return now;
}


static int DYNINSTtraceFp = -1;


/************************************************************************
 * void DYNINSTflushTrace(void)
 *
 * flush any accumulated traces.
************************************************************************/

void
DYNINSTflushTrace(void) {
  /*    if (DYNINSTtraceFp) fflush(DYNINSTtraceFp); */
}


/************************************************************************
 * void DYNINSwriteTrace(void)
 *
 * write data on the trace stream
************************************************************************/

int
DYNINSTwriteTrace(void *buffer, unsigned count) {
  if (send((SOCKET)DYNINSTtraceFp, buffer, count, 0) != count) {
    printf("DYNINSTwriteTrace: send error %d, %d %d\n",
	   WSAGetLastError(), count, DYNINSTtraceFp);
    if (DYNINSTtraceFp == -1)
      return 1;
    return 0;
  }
  return 1;
}


/************************************************************************
 * static int connectToDaemon(int port)
 *
 * get a connection to a paradyn daemon for the trace stream
 *
 * does: socket(), connect()
************************************************************************/

static int connectToDaemon(int daemonPort) {
  struct sockaddr_in sadr;
  struct in_addr *inadr;
  struct hostent *hostptr;
  int sock_fd;

  hostptr = gethostbyname("localhost");
  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  memset((void*) &sadr, 0, sizeof(sadr));
  sadr.sin_family = PF_INET;
  sadr.sin_port = htons((u_short)daemonPort);
  sadr.sin_addr = *inadr;

  sock_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (sock_fd == INVALID_SOCKET) {
    fprintf(stderr, "DYNINST: socket failed: %d\n", WSAGetLastError());
    abort();
  }

  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) == SOCKET_ERROR) {
    fprintf(stderr, "DYNINST: connect failed: %d\n", WSAGetLastError());
    abort();
  }
  return sock_fd;
}

/************************************************************************
 * DYNINSTinitTrace(int daemon_addr)
 *
 * initializes the trace stream. daemon_addr is a platform dependent value
 * that is used to create a connection. If daemon_addr is -1, we assume
 * that the trace stream is already open (by the daemon, when it started
 * this process). For posix systems, daemon_addr is the pid of the paradynd 
 * daemon. For WINNT it is the port address of a socket.
************************************************************************/

void DYNINSTinitTrace(int daemon_addr) {

  WORD wsversion = MAKEWORD(2,0);
  WSADATA wsadata;
  WSAStartup(wsversion, &wsadata);

  DYNINSTtraceFp = connectToDaemon(daemon_addr);
  if (DYNINSTtraceFp == -1) {
    fprintf(stderr, "connectToDaemon failed\n");
    abort();
  }
}


void DYNINSTcloseTrace() {
  closesocket(DYNINSTtraceFp);
}


static void printSysError(unsigned errNo) {
  char buf[1000];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, 1000, NULL);
  printf("*** System error [%d]: %s\n", errNo, buf);
}
