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

/************************************************************************
 * RTwinnt.c: runtime instrumentation functions for Windows NT
 *
 * RTwinnt.c,v
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

#ifdef CONTROLLER_FD
#undef CONTROLLER_FD
static HANDLE CONTROLLER_FD;
#endif

static HANDLE DYNINSTprocHandle;



/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void
DYNINSTbreakPoint(void) {
  /* TODO: how do we stop all threads? */
  DebugBreak();
}


unsigned DYNINSTgetCurrentThreadId() {
  return GetCurrentThreadId();
}

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
    printf("DYNINST: unable to create periodic timer\n");
    abort();
  }

}


#define FILETIME2time64(FT) \
   (((time64)(FT).dwHighDateTime<<32) | ((time64)(FT).dwLowDateTime))

/*static int MaxRollbackReport = 0; /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; /* only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


time64 DYNINSTgetCPUtime() {

  FILETIME kernelT, userT, creatT, exitT;
  static time64 cpuPrevious = 0;
  static int cpuRollbackOccurred = 0;
  time64 now;
  static HANDLE procHandle;

  if(GetProcessTimes(DYNINSTprocHandle, &creatT, &exitT, &kernelT,&userT)==0) {
    abort();
    return 0;
  }

  now = (FILETIME2time64(userT)+FILETIME2time64(kernelT)) / (time64)10;

  if (now < cpuPrevious) {
    if(cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback with current time: "
	      "%I64d usecs, using previous value %I64d usecs.",now,cpuPrevious);
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


time64 DYNINSTgetWalltime() {
  LARGE_INTEGER time;

  static int firstTime;
  static double freq = 1.0;
  static time64 wallPrevious=0;
  static int wallRollbackOccurred = 0;
  time64 now;

  if (firstTime == 0) {
    firstTime = 1;
    if (QueryPerformanceFrequency(&time)) {
      freq = (time64)time.QuadPart;
    }
  }

  if (QueryPerformanceCounter(&time)) {
    /* dividing time by freq gives a value is seconds.
       we need to multiply by 1000000.0 to get microseconds
       */
    now = (time64)( ((double)time.QuadPart/freq) * 1000000.0);
  } else {
    abort();
  }

  if (now < wallPrevious) {
    if(wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "Wall time rollback with current time: "
	      "%I64d usecs, using previous value %I64d usecs.",now,wallPrevious);
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

extern char *sys_errlist[];

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
  if (sock_fd < 0) {
    fprintf(stderr, "DYNINST: socket failed: %s\n", sys_errlist[errno]);
    abort();
  }

  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) < 0) {
    fprintf(stderr, "DYNINST: connect failed: %s\n", sys_errlist[errno]);
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
    printf("connectToDaemon failed\n");
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

void DYNINSTos_init(int calledFromFork, int calledFromAttach, int paradyndAddr) {
  HANDLE h;
  unsigned ioCookie = 0x33333333;
  unsigned pid = GetCurrentProcessId();
  unsigned ppid = paradyndAddr;
  DYNINSTprocHandle = GetCurrentProcess();

}

