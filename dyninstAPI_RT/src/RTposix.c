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
 * $Id: RTposix.c,v 1.15 2005/09/09 18:05:20 legendre Exp $
 * RTposix.c: runtime instrumentation functions for generic posix.
 ************************************************************************/

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pwd.h>
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTcommon.h"

/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void DYNINSTbreakPoint(void)
{
   kill(getpid(), SIGSTOP);
}

#if !defined(cap_save_the_world)
void RTmutatedBinary_init() 
{
    return;
}
#endif

#ifdef __GNUC
void libdyninstAPI_RT_init(void) __attribute__ ((constructor));
#endif

void libdyninstAPI_RT_init() 
{
   static int initCalledOnce = 0;
   if (initCalledOnce) return;
   initCalledOnce++;

   RTmutatedBinary_init();
   
   if (libdyninstAPI_RT_init_localCause != -1 && 
       libdyninstAPI_RT_init_localPid != -1 &&
       libdyninstAPI_RT_init_maxthreads != -1)
   {
      DYNINSTinit(libdyninstAPI_RT_init_localCause, libdyninstAPI_RT_init_localPid,
                  libdyninstAPI_RT_init_maxthreads);
   }
}


/************************************************************************
 * void DYNINSTasyncConnect(int pid)
 *
 * Connect to mutator's async handler thread. <pid> is pid of mutator
************************************************************************/

static int async_socket = -1;
static int needToDisconnect = 0;

int DYNINSTwriteEvent(void *ev, int sz);

static void exit_func(void)
{
  if (needToDisconnect) close (async_socket);
}

int DYNINSTasyncConnect(int pid)
{
  
  int sock_fd;
  int err = 0;
  struct sockaddr_un sadr;
  rtBPatch_asyncEventRecord ev;
  uid_t euid;
  struct passwd *passwd_info;
  char path[255];

  if (async_socket != -1)
  {
     fprintf(stderr, "[%s:%u] - DYNINSTasyncConnect already initialized\n",
             __FILE__, __LINE__);
     return 0;
  }
  euid = geteuid();
  passwd_info = getpwuid(euid);
  assert(passwd_info);

  snprintf(path, 255, "%s/dyninstAsync.%s.%d", P_tmpdir, passwd_info->pw_name, pid);
  sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("DYNINSTasyncConnect() socket()");
    abort();
  }

  sadr.sun_family = PF_UNIX;
  strcpy(sadr.sun_path, path);

  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) < 0) {
    perror("DYNINSTasyncConnect() connect()");
  }

  /* maybe need to do fcntl to set nonblocking writes on this fd */

  assert(async_socket == -1);
  async_socket = sock_fd;

  /* after connecting, we need to send along our pid */
  ev.type = rtBPatch_newConnectionEvent;
  ev.pid = getpid();

  err = DYNINSTwriteEvent((void *) &ev, sizeof(rtBPatch_asyncEventRecord));

  if (err) {
    fprintf(stderr, "%s[%d]:  report new connection failed\n", __FILE__, __LINE__);
    return 0;
  }
  /* initialize spinlock */
  
  needToDisconnect = 1;

 /* atexit(exit_func); */
  return 1; 

}

int DYNINSTasyncDisconnect()
{
  if (needToDisconnect)
   close (async_socket);
  return 0;
}

int DYNINSTwriteEvent(void *ev, int sz)
{
  int res;
  int i;
try_again:
  res = write(async_socket, ev, sz); 
  if (-1 == res) {
    if (errno == EINTR || errno == EAGAIN) 
       goto try_again;
    else {
       perror("write");
       return -1;
    }
  }
  if (res != sz) {
    /*  maybe we need logic to handle partial writes? */
    fprintf(stderr, "%s[%d]:  partial ? write error, %d bytes, should be %d\n",
            __FILE__, __LINE__, res, sz);
    return -1;
  }
  return 0;
}


