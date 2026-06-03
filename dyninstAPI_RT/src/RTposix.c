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

#if !(defined(DYNINST_HOST_ARCH_POWER) && defined(os_linux))
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

// Important note: addr will be zero in two cases here
// One is the case where we're doing a constrained low mmap, in which case MAP_32BIT
// is precisely correct. The other is the case where our
// constrained map attempts have failed, and we're doing a scan for first available
// mappable page. In that case, MAP_32BIT does no harm.
void *map_region(void *addr, int len, int fd) {
     void *result;
    int flags = DYNINSTheap_mmapFlags;
#if defined(DYNINST_HOST_ARCH_X86_64)
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
