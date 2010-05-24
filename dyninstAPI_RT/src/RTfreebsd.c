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
 * $Id: RTlinux.c,v 1.54 2008/04/11 23:30:44 legendre Exp $
 * RTlinux.c: mutatee-side library function specific to Linux
 ************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTthread.h"
#include "dyninstAPI_RT/src/RTcommon.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <link.h>

#define NOT_ON_FREEBSD "This function is unimplemented on FreeBSD"

/* FreeBSD libc has stubs so a static version shouldn't need libpthreads */
#include <pthread.h>

/* TODO threading support, mutatee traps */

extern double DYNINSTstaticHeap_512K_lowmemHeap_1[];
extern double DYNINSTstaticHeap_16M_anyHeap_1[];
extern unsigned long sizeOfLowMemHeap1;
extern unsigned long sizeOfAnyHeap1;

static struct trap_mapping_header *getStaticTrapMap(unsigned long addr);

/** RT lib initialization **/

void mark_heaps_exec() {
    RTprintf( "*** Initializing dyninstAPI runtime.\n" );

    /* Grab the page size, to align the heap pointer. */
    long int pageSize = sysconf( _SC_PAGESIZE );
    if( pageSize == 0 || pageSize == - 1 ) {
        fprintf( stderr, "*** Failed to obtain page size, guessing 16K.\n" );
        perror( "mark_heaps_exec" );
        pageSize = 1024 * 16;
    } /* end pageSize initialization */

    /* Align the heap pointer. */
    unsigned long int alignedHeapPointer = (unsigned long int) DYNINSTstaticHeap_16M_anyHeap_1;
    alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
    unsigned long int adjustedSize = (unsigned long int) DYNINSTstaticHeap_16M_anyHeap_1 - alignedHeapPointer + sizeOfAnyHeap1;

    /* Make the heap's page executable. */
    int result = mprotect( (void *) alignedHeapPointer, (size_t) adjustedSize, PROT_READ | PROT_WRITE | PROT_EXEC );
    if( result != 0 ) {
        fprintf( stderr, "%s[%d]: Couldn't make DYNINSTstaticHeap_16M_anyHeap_1 executable!\n", __FILE__, __LINE__);
        perror( "mark_heaps_exec" );
    }
    RTprintf( "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize );

    /* Mark _both_ heaps executable. */
    alignedHeapPointer = (unsigned long int) DYNINSTstaticHeap_512K_lowmemHeap_1;
    alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
    adjustedSize = (unsigned long int) DYNINSTstaticHeap_512K_lowmemHeap_1 - alignedHeapPointer + sizeOfLowMemHeap1;

    /* Make the heap's page executable. */
    result = mprotect( (void *) alignedHeapPointer, (size_t) adjustedSize, PROT_READ | PROT_WRITE | PROT_EXEC );
    if( result != 0 ) {
        fprintf( stderr, "%s[%d]: Couldn't make DYNINSTstaticHeap_512K_lowmemHeap_1 executable!\n", __FILE__, __LINE__ );
        perror( "mark_heaps_exec" );
    }
    RTprintf( "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize );
} /* end mark_heaps_exec() */

int DYNINST_sysEntry;
void DYNINSTos_init(int calledByFork, int calledByAttach)
{
    assert(!NOT_ON_FREEBSD);
}

#if defined(cap_binary_rewriter) && !defined(DYNINST_RT_STATIC_LIB)
/* For a static binary, all global constructors are combined in an undefined
 * order. Also, DYNINSTBaseInit must be run after all global constructors have
 * been run. Since the order of global constructors is undefined, DYNINSTBaseInit
 * cannot be run as a constructor in static binaries. Instead, it is run from a
 * special constructor handler that processes all the global constructors in
 * the binary. Leaving this code in would create a global constructor for the
 * function runDYNINSTBaseInit(). See DYNINSTglobal_ctors_handler.
 */ 
extern void DYNINSTBaseInit();
void runDYNINSTBaseInit() __attribute__((constructor));
void runDYNINSTBaseInit()
{
   DYNINSTBaseInit();
}
#endif

/** Dynamic instrumentation support **/

void DYNINSTbreakPoint()
{
    assert(!NOT_ON_FREEBSD);
}

static int failed_breakpoint = 0;
void uncaught_breakpoint(int sig)
{
   failed_breakpoint = 1;
}

void DYNINSTsafeBreakPoint()
{
    assert(!NOT_ON_FREEBSD);
}

/* FreeBSD libc includes dl* functions typically in libdl */
typedef struct dlopen_args {
  const char *libname;
  int mode;
  void *result;
  void *caller;
} dlopen_args_t;

void *(*DYNINST_do_dlopen)(dlopen_args_t *) = NULL;

static int get_dlopen_error() {
    assert(!NOT_ON_FREEBSD);
    return 1;
}

int DYNINSTloadLibrary(char *libname)
{
    assert(!NOT_ON_FREEBSD);
    return 1;
}

/** threading support **/

int dyn_lwp_self()
{
    static int gettid_not_valid = 0;
    int result;
    
    if( gettid_not_valid )
        return getpid();

    long lwp_id;
    result = syscall(SYS_thr_self, &lwp_id);
    if( result && errno == ENOSYS ) {
        gettid_not_valid = 1;
        return getpid();
    }

    return lwp_id;
}

int dyn_pid_self()
{
   return getpid();
}

dyntid_t (*DYNINST_pthread_self)(void);

dyntid_t dyn_pthread_self()
{
   dyntid_t me;
   if (DYNINSTstaticMode) {
      return (dyntid_t) pthread_self();
   }
   if (!DYNINST_pthread_self) {
      return (dyntid_t) DYNINST_SINGLETHREADED;
   }
   me = (*DYNINST_pthread_self)();
   return (dyntid_t) me;
}

/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread( dyntid_t tid ) {
    if( dyn_lwp_self() == getpid() ) {
        return 1;
    }
    return 0;
} /* end DYNINST_am_initial_thread() */

// TODO
#define READ_OPAQUE(buffer, pos, type) *((type *)(buffer + pos))

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev) {

}
