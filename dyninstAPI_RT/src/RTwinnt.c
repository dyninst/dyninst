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
 * $Id: RTwinnt.c,v 1.9 2005/02/09 03:27:50 jaw Exp $
 * RTwinnt.c: runtime instrumentation functions for Windows NT
 ************************************************************************/
#if !defined (EXPORT_SPINLOCKS_AS_HEADER)
/* everything should be under this flag except for the assembly code
   that handles the runtime spinlocks  -- this is imported into the
   test suite for direct testing */


#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001
#include <assert.h>
#endif
#include <stdio.h>
#include <stdlib.h>
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001
#include <mmsystem.h>
#include <errno.h>
#include <limits.h>
#endif
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
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

char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
    HMODULE res;
    gLoadLibraryErrorString[0] = '\0';
    //fprintf(stderr, "Attempting to load %s\n", libname);
    
    res = LoadLibrary(libname);
    if (res == NULL) {
        perror("DYNINSTloadLibrary - load of library failed");
        return 0;
    }
    return 1;
}
int DYNINSTwriteEvent(void *ev, int sz);
/************************************************************************
 * void DYNINSTasyncConnect(int pid)
 *
 * Connect to mutator's async handler thread. <pid> is pid of mutator
************************************************************************/
//CRITICAL_SECTION comms_mutex;
dyninst_spinlock thelock;
int async_socket = -1;
int connect_port = 0;
int DYNINSTasyncConnect()
{

  int sock_fd;
  struct sockaddr_in sadr;
  struct in_addr *inadr;
  struct hostent *hostptr;
  WORD wsversion = MAKEWORD(2,0);
  WSADATA wsadata;
  BPatch_asyncEventRecord ev;

  fprintf(stderr, "%s[%d]:  inside DYNINSTasyncConnect\n", __FILE__, __LINE__);
  if (0 == connect_port) {
    fprintf(stderr, "%s[%d]:  DYNINSTasyncConnect, no port no\n",
            __FILE__, __LINE__);
    abort() ;
  }

  WSAStartup(wsversion, &wsadata);

  fprintf(stderr, "%s[%d]:  inside DYNINSTasyncConnect before gethostbyname\n", __FILE__, __LINE__);
  hostptr = gethostbyname("localhost");
  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  fprintf(stderr, "%s[%d]:  inside DYNINSTasyncConnect before memset\n", __FILE__, __LINE__);
  memset((void*) &sadr, 0, sizeof(sadr));
  sadr.sin_family = PF_INET;
  sadr.sin_port = htons((u_short)connect_port);
  sadr.sin_addr = *inadr;

  fprintf(stderr, "%s[%d]:   DYNINSTasyncConnect before socket\n", __FILE__, __LINE__);
  sock_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (sock_fd == INVALID_SOCKET) {
    fprintf(stderr, "DYNINST: socket failed: %d\n", WSAGetLastError());
    abort();
  }

  fprintf(stderr, "%s[%d]:   DYNINSTasyncConnect before connect\n", __FILE__, __LINE__);
  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) == SOCKET_ERROR) {
    fprintf(stderr, "DYNINSTasyncConnect: connect failed: %d\n", WSAGetLastError());
    abort();
  }

  /* maybe need to do fcntl to set nonblocking writes on this fd */

  async_socket = sock_fd;

  fprintf(stderr, "%s[%d]:   DYNINSTasyncConnect before write\n", __FILE__, __LINE__);
  /* after connecting, we need to send along our pid */
  ev.type = BPatch_newConnectionEvent;
  ev.pid = _getpid();
  if (!DYNINSTwriteEvent((void *) &ev, sizeof(BPatch_asyncEventRecord))) {
    fprintf(stderr, "%s[%d]:  DYNINSTwriteEventFailed\n", __FILE__, __LINE__);
  }
  /* initialize comms mutex */

  //InitializeCriticalSection(&comms_mutex);
  //fprintf(stderr, "%s[%d]: DYNINSTasyncConnect appears to have succeeded\n", __FILE__, __LINE__);
  thelock.lock = 0;
  fprintf(stderr, "%s[%d]:  leaving DYNINSTasyncConnect\n", __FILE__, __LINE__);
  return 1; /*true*/
}

int DYNINSTasyncDisconnect()
{
  return _close (async_socket);
}

int DYNINSTwriteEvent(void *ev, int sz)
{
  if (send((SOCKET)async_socket, ev, sz, 0) != sz) {
    printf("DYNINSTwriteTrace: send error %d, %d %d\n",
           WSAGetLastError(), sz, async_socket);
    if (async_socket == -1)
      return 1;
    return 0;
  }
  return 1;

#ifdef NOTDEF
  int res;

try_again:
  res = write(async_socket, ev, sz);
  if (-1 == res) {
    if (errno == EINTR || errno == EAGAIN)
       goto try_again;
    else {
       perror("write");
       abort();
    }
  }
  if (res != sz) {
    /*  maybe we need logic to handle partial writes? */
    fprintf(stderr, "%s[%d]:  partial ? write error, %d bytes, should be %d\n",
            __FILE__, __LINE__, res, sz);
    abort();
  }
  return res;
#endif
}
void DYNINSTlock_spinlock(dyninst_spinlock *mut);
extern void DYNINSTunlock_spinlock(dyninst_spinlock *mut);
void LockCommsMutex()
{
  //fprintf(stderr, "%s[%d]:  before enter crit\n", __FILE__, __LINE__);
  //EnterCriticalSection(&comms_mutex);
  //fprintf(stderr, "%s[%d]:  after enter crit\n", __FILE__, __LINE__);
  DYNINSTlock_spinlock(&thelock);
}
void UnlockCommsMutex()
{
  //fprintf(stderr, "%s[%d]:  before leave crit\n", __FILE__, __LINE__);
  //LeaveCriticalSection(&comms_mutex);
  //fprintf(stderr, "%s[%d]:  after leave crit\n", __FILE__, __LINE__);
  DYNINSTunlock_spinlock(&thelock);
}
#endif /* EXPORT SPINLOCKS */
void DYNINSTlock_spinlock(dyninst_spinlock *mut)
{
  /*  same assembly as for x86 linux, just different format for asm stmt */
  /*  so if you change one, make the same changes in the other, please */
  /*msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_langref___asm.asp*/

 /* seperate command to get arg into ecx */
 __asm mov ecx,mut 

 /* thus this should work whether or not we compiler with __fastcall */
 /* (which passes args in registers instead of on the stack */

 __asm  {
           a_loop:
                                          ; movl        8(%ebp), %ecx
           mov        eax,0 ;
           mov        edx,1 ;
           lock cmpxchg   [ecx], edx ;

           jnz         a_loop
     }

}

