/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/************************************************************************
 * $Id: RTwinnt.c,v 1.22 2006/06/09 03:50:49 jodom Exp $
 * RTwinnt.c: runtime instrumentation functions for Windows NT
 ************************************************************************/
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTcommon.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <errno.h>
#include <limits.h>
#include <process.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
//#include <winsock2.h>

/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void DYNINSTbreakPoint(void) {
  /* TODO: how do we stop all threads? */
    DYNINST_break_point_event = 1;
    DebugBreak();
    DYNINST_break_point_event = 0;
}

void DYNINSTsafeBreakPoint() {
    DYNINSTbreakPoint();
}

static dyntid_t initial_thread_tid;
void DYNINSTos_init(int calledByFork, int calledByAttach)
{
  RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
  initial_thread_tid = (dyntid_t) dyn_lwp_self();
}

/* this function is automatically called when windows loads this dll
 if we are launching a mutatee to instrument, dyninst will place
 the correct values in libdyninstAPI_RT_DLL_localPid and
 libdyninstAPI_RT_DLL_localCause and they will be passed to
 DYNINSTinit to correctly initialize the dll.  this keeps us
 from having to instrument two steps from the mutator (load and then 
 the execution of DYNINSTinit()
*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   static int DllMainCalledOnce = 0;

   if(DllMainCalledOnce)
      return 1;
   DllMainCalledOnce++;

   if(libdyninstAPI_RT_init_localPid != -1 || libdyninstAPI_RT_init_localCause != -1)
      DYNINSTinit(libdyninstAPI_RT_init_localCause, libdyninstAPI_RT_init_localPid,
                  libdyninstAPI_RT_init_maxthreads, libdyninstAPI_RT_init_debug_flag);


   return 1; 
}
 


char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
    HMODULE res;
    gLoadLibraryErrorString[0] = '\0';
    res = LoadLibrary(libname);
    if (res == NULL) {
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  gLoadLibraryErrorString, ERROR_STRING_LENGTH, NULL);
        return 0;
    }
    return 1;
}

/************************************************************************
 * void DYNINSTasyncConnect()
 *
 * Connect to mutator's async handler thread. <pid> is pid of mutator
 ************************************************************************/
//CRITICAL_SECTION comms_mutex;

int async_socket = -1;
int connect_port = 0;

int DYNINSTasyncConnect(int mutatorpid)
{
  int sock_fd;
  struct sockaddr_in sadr;
  struct in_addr *inadr;
  struct hostent *hostptr;
  
  WORD wsversion = MAKEWORD(2,0);
  WSADATA wsadata;
  rtBPatch_asyncEventRecord ev;

  if (async_socket != -1) 
  {
	  /*fprintf(stderr, "%s[%d]:  already connected\n", __FILE__, __LINE__);*/
      return 0;
  }

  RTprintf("%s[%d]:  inside DYNINSTasyncConnect\n", __FILE__, __LINE__);

  if (0 == connect_port) 
  {
    fprintf(stderr, "%s[%d]:  DYNINSTasyncConnect, no port\n",
            __FILE__, __LINE__);    
  }

  WSAStartup(wsversion, &wsadata);
   
  RTprintf("%s[%d]:  DYNINSTasyncConnect before gethostbyname\n", __FILE__, __LINE__);

  hostptr = gethostbyname("localhost");
  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);

  RTprintf("%s[%d]:  inside DYNINSTasyncConnect before memset\n", __FILE__, __LINE__);

  memset((void*) &sadr, 0, sizeof(sadr));
  sadr.sin_family = PF_INET;
  sadr.sin_port = htons((u_short)connect_port);
  sadr.sin_addr = *inadr;

  RTprintf("%s[%d]:   DYNINSTasyncConnect before socket\n", __FILE__, __LINE__);

  sock_fd = socket(PF_INET, SOCK_STREAM, 0);

  if (sock_fd == INVALID_SOCKET) 
  {
    fprintf(stderr, "DYNINST: socket failed: %d\n", WSAGetLastError());
  }

  RTprintf("%s[%d]:   DYNINSTasyncConnect before connect\n", __FILE__, __LINE__);

  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) == SOCKET_ERROR) 
  {
    fprintf(stderr, "DYNINSTasyncConnect: connect failed: %d\n", WSAGetLastError());
  }

  /* maybe need to do fcntl to set nonblocking writes on this fd */

  async_socket = sock_fd;

  RTprintf("%s[%d]:   DYNINSTasyncConnect before write\n", __FILE__, __LINE__);

  /* after connecting, we need to send along our pid */
  ev.type = rtBPatch_newConnectionEvent;
  ev.pid = _getpid();

  if (!DYNINSTwriteEvent((void *) &ev, sizeof(rtBPatch_asyncEventRecord))) 
  {
    fprintf(stderr, "%s[%d]:  DYNINSTwriteEventFailed\n", __FILE__, __LINE__);
  }

  /* initialize comms mutex */

  //InitializeCriticalSection(&comms_mutex);
  //fprintf(stderr, "%s[%d]: DYNINSTasyncConnect appears to have succeeded\n", __FILE__, __LINE__);

  RTprintf("%s[%d]:  leaving DYNINSTasyncConnect\n", __FILE__, __LINE__);

  return 1; /*true*/
}

int DYNINSTasyncDisconnect()
{
  WSACleanup();
  return _close (async_socket);
}

void printSysError(unsigned errNo) {
    char buf[1000];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);

    fprintf(stderr, "*** System error [%d]: %s\n", errNo, buf);
    fflush(stderr);
}

int DYNINSTwriteEvent(void *ev, size_t sz)
{
  DYNINSTasyncConnect(DYNINST_mutatorPid);

  if (send((SOCKET)async_socket, ev, sz, 0) != sz) 
  {
    printSysError(WSAGetLastError());
    printf("DYNINSTwriteTrace: send error %d, %d %d\n",
           WSAGetLastError(), sz, async_socket);

    if (async_socket == -1)
      return 1;
    return 0;
  }
  return 1;
}

int dyn_pid_self()
{
   return _getpid();
}

int dyn_lwp_self()
{
	/* getCurrentThreadId() is conflicting with SD-Dyninst instrumentation. 
	So I'm doing the massively unportable thing here and hard-coding the assembly
	FOR GREAT JUSTICE! */
/*    return GetCurrentThreadId(); */
	/* This will do stack frame setup, but that seems harmless in this context... */
	__asm
    {
        mov     EAX,FS:[0x18]
		mov     EAX,DS:[EAX+0x24]
	}
}

dyntid_t dyn_pthread_self()
{
   return (dyntid_t) dyn_lwp_self();
}

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev)
{
    return 1;
}

/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread(dyntid_t tid) {
    return (tid == initial_thread_tid);
}
