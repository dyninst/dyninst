/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
 * $Id: RTposix.c,v 1.37 2008/04/11 23:30:45 legendre Exp $
 * RTposix.c: runtime instrumentation functions for generic posix.
 ************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <memory.h>
#include <sys/socket.h>
#include <pwd.h>

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTcommon.h"
#include "dyninstAPI_RT/src/RTheap.h"

#define SOCKLEN_T socklen_t

#if !(defined(arch_power) && defined(os_linux))
void RTmutatedBinary_init(void)
{
    return;
}
#endif

#if defined(__GNUC) || defined(__GNUC__)
#if defined(DYNINST_RT_STATIC_LIB)
/*
 * In the static version of the library, constructors cannot be
 * used to run code at initialization. See DYNINSTglobal_ctors_handler.
 */
void libdyninstAPI_RT_init(void);
#else
void libdyninstAPI_RT_init(void) __attribute__ ((constructor));
#endif
#endif

#if defined (cap_async_events)
struct passwd *passwd_info = NULL;
#endif

void libdyninstAPI_RT_init(void)
{
   static int initCalledOnce = 0;

   rtdebug_printf("%s[%d]:  DYNINSTinit:  welcome to libdyninstAPI_RT_init()\n", __FILE__, __LINE__);

   if (initCalledOnce) return;
   initCalledOnce++;

  
   DYNINSTinit();
   rtdebug_printf("%s[%d]:  did DYNINSTinit\n", __FILE__, __LINE__);
}


/************************************************************************
 * void DYNINSTasyncConnect(int pid)
 *
 * Connect to mutator's async handler thread. <pid> is pid of mutator
************************************************************************/

static int async_socket = -1;
static int needToDisconnect = 0;
static char socket_path[255];

int DYNINSTasyncConnect(int pid)
{
   if (DYNINSTstaticMode)
      return 0;
#if defined (cap_async_events)
  int sock_fd;
  struct sockaddr_un sadr;
   int res;
   int mutatee_pid;
   uid_t euid;

   rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  entry\n", __FILE__, __LINE__);
   rtdebug_printf("%s[%d]:  DYNINSTinit:  before geteuid\n", __FILE__, __LINE__);

   euid = geteuid();
   passwd_info = getpwuid(euid);
   assert(passwd_info);

  if (async_socket != -1)
  {
	  fprintf(stderr, "%s[%d]: - DYNINSTasyncConnect already initialized\n",
			  __FILE__, __LINE__);

     rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  already connected\n", 
			 __FILE__, __LINE__);
     return 0;
  }

  rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  before socket 2\n", __FILE__, __LINE__);
  mutatee_pid = getpid();

  snprintf(socket_path, (size_t) 255, "%s/dyninstAsync.%s.%d.%d", 
		  P_tmpdir, passwd_info->pw_name, pid, mutatee_pid);

  rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  before socket: %s\n", __FILE__, __LINE__, socket_path);

  errno = 0;

  sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);

  if (sock_fd < 0) 
  {
    fprintf(stderr, "%s[%d]: DYNINSTasyncConnect() socket(%s): %s\n", 
			__FILE__, __LINE__, socket_path, strerror(errno));
    abort();
  }

  rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  after socket\n", __FILE__, __LINE__);

  sadr.sun_family = PF_UNIX;
  strcpy(sadr.sun_path, socket_path);

  rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  before connect\n", __FILE__, __LINE__);
  res = 0;
  errno = 0;

  res = connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)); 

  if (res < 0)
  {
    perror("DYNINSTasyncConnect() connect()");
  }

  rtdebug_printf("%s[%d]:  DYNINSTasyncConnnect:  after connect to %s, res = %d, -- %s\n", 
		  __FILE__, __LINE__, socket_path, res, strerror(errno));

  /* maybe need to do fcntl to set nonblocking writes on this fd */

  if (async_socket == -1)
  {
	  rtdebug_printf("%s[%d]:  WARN:  async socket has not been reset!!\n", __FILE__, __LINE__);
  }

  async_socket = sock_fd;

  needToDisconnect = 1;

 /* atexit(exit_func); */
  rtdebug_printf("%s[%d]:  leaving DYNINSTasyncConnect\n", __FILE__, __LINE__);
  return 1; 
#else
  fprintf(stderr, "%s[%d]:  called DYNINSTasyncConect when async_events disabled\n",
		  __FILE__, __LINE__);
  return 0;
#endif
}

int DYNINSTasyncDisconnect(void)
{
   if (DYNINSTstaticMode)
      return 0;
   rtdebug_printf("%s[%d]:  welcome to DYNINSTasyncDisconnect\n", __FILE__, __LINE__);
   if (needToDisconnect) {
      close (async_socket);
      needToDisconnect = 0;
   }
   async_socket = -1;
   return 0;
}

int DYNINSTwriteEvent(void *ev, size_t sz)
{
  ssize_t res;

  if (DYNINSTstaticMode)
     return 0;
  
    rtdebug_printf("%s[%d]:  welcome to DYNINSTwriteEvent: %zu bytes\n", __FILE__, __LINE__, sz);
  if (-1 == async_socket)
  {
	  rtdebug_printf("%s[%d]:  failed to DYNINSTwriteEvent, no socket\n", __FILE__, __LINE__);
	  return -1;
  }

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
  if ((size_t)res != sz) {
    /*  maybe we need logic to handle partial writes? */
    fprintf(stderr, "%s[%d]:  partial ? write error, %zd bytes, should be %zu\n",
            __FILE__, __LINE__, res, sz);
    return -1;
  }
  return 0;
}

// Important note: addr will be zero in two cases here
// One is the case where we're doing a constrained low mmap, in which case MAP_32BIT
// is precisely correct. The other is the case where our
// constrained map attempts have failed, and we're doing a scan for first available
// mappable page. In that case, MAP_32BIT does no harm.
void *map_region(void *addr, int len, int fd) {
     void *result;
    int flags = DYNINSTheap_mmapFlags;
#if defined(arch_x86_64)
    if(addr == 0) flags |= MAP_32BIT;
#endif
     result = mmap(addr, len, PROT_READ|PROT_WRITE|PROT_EXEC,
                   flags, fd, 0);
     if (result == MAP_FAILED)
         return NULL;
     return result;
}

int unmap_region(void *addr, int len) {
    int result;
    result = munmap(addr, len);
    if (result == -1)
        return 0;
    return 1;
}

#if defined(cap_mutatee_traps)
extern void dyninstTrapHandler(int sig, siginfo_t *info, void *context);

int DYNINSTinitializeTrapHandler(void)
{
   int result;
   struct sigaction new_handler;
   int signo = SIGTRAP;

   // If environment variable DYNINST_SIGNAL_TRAMPOLINE_SIGILL is set,
   // we use SIGILL as the signal for signal trampoline.
   // The mutatee has to be generated with DYNINST_SIGNAL_TRAMPOLINE_SIGILL set
   // so that the mutator will generates illegal instructions as trampolines.
   if (getenv("DYNINST_SIGNAL_TRAMPOLINE_SIGILL")) {
      signo = SIGILL;
   }

   new_handler.sa_sigaction = dyninstTrapHandler;
   //new_handler.sa_restorer = NULL; obsolete
   sigemptyset(&new_handler.sa_mask);
   new_handler.sa_flags = SA_SIGINFO | SA_NODEFER;
   
   result = sigaction(signo, &new_handler, NULL);
   return (result == 0) ? 1 /*Success*/ : 0 /*Fail*/ ;
}

#endif
