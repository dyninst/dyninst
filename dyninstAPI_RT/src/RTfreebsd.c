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

//static struct trap_mapping_header *getStaticTrapMap(unsigned long addr);

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

/*
static int get_dlopen_error() {
    assert(!NOT_ON_FREEBSD);
    return 1;
}
*/

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

/*
 * This code extracts the lwp, the address of the thread entry function,
 * and the top of the thread's stack. It uses predefined offset to 
 * extract this information from pthread_t, which is usually opaque. 
 * 
 * Hopefully, only one set of offsets should be needed per architecture
 * because there should exist only one version of FreeBSD libc per 
 * FreeBSD version.
 *
 * If different versions are encountered, see the Linux version of this
 * for ideas on how to handle them.
 *
 * Finally, there is a problem that can be used to determine these offsets
 * at the end of this file.
 */
#define READ_OPAQUE(buffer, pos, type) *((type *)(buffer + pos))

typedef struct pthread_offset_t {
    unsigned long lwp_pos;
    unsigned long start_func_pos;
    unsigned long stack_start_pos;
    unsigned long stack_size_pos;
} pthread_offset_t;

#if defined(amd64_unknown_freebsd7_0)
static pthread_offset_t offsets = { 0, 112, 152, 160 };
#else
#error pthread_t offsets undefined for this architecture
#endif

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev) {
    static int err_printed = 0;
    unsigned char *buffer = (unsigned char *)ev->tid;

    unsigned long lwp = READ_OPAQUE(buffer, offsets.lwp_pos, unsigned long);
    ev->stack_addr = (void *)(READ_OPAQUE(buffer, offsets.stack_start_pos, unsigned long) + 
        READ_OPAQUE(buffer, offsets.stack_size_pos, unsigned long));
    ev->start_pc = (void *)(READ_OPAQUE(buffer, offsets.start_func_pos, unsigned long));

    if( lwp != ev->lwp && !err_printed ) {
        RTprintf("%s[%d]: Failed to parse pthread_t information. Making a best effort guess.\n",
                __FILE__, __LINE__);
        err_printed = 1;
    }

    return 1;
}


/*
 * A program to determine the offsets of certain thread structures on FreeBSD
 *
 * This program should be compiled with the headers from the libthr library from
 * /usr/src. This can be installed using sysinstall. The following arguments 
 * should be added to the compile once these headers are installed.
 *
 * -I/usr/src/lib/libthr/arch/amd64/include -I/usr/src/lib/libthr/thread
 *
 * Change amd64 to what ever is appropriate.

#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "thr_private.h"

pthread_attr_t attr;

void *foo(void *f) {
    unsigned long stack_addr;
    void *(*start_func)(void *);
    unsigned long tid;

    // Get all the values
    syscall(SYS_thr_self, &tid);

    start_func = foo;

    asm("mov %%rbp,%0" : "=r" (stack_addr));

    pthread_t threadSelf = pthread_self();

    printf("TID: %u == %u\n", tid, threadSelf->tid);
    printf("STACK: 0x%lx == 0x%lx\n", stack_addr, threadSelf->attr.stackaddr_attr + threadSelf->attr.stacksize_attr);
    printf("START: 0x%lx == 0x%lx\n", (unsigned long)start_func, (unsigned long)threadSelf->start_routine);

    unsigned char *ptr = (unsigned char *)threadSelf;
    unsigned long tidVal = *((unsigned long *)(ptr + offsetof(struct pthread, tid)));
    unsigned long stackAddrVal = *((unsigned long *)(ptr + offsetof(struct pthread, attr) + offsetof(struct pthread_attr, stackaddr_attr)));
    unsigned long stackSizeVal = *((unsigned long *)(ptr + offsetof(struct pthread, attr) + offsetof(struct pthread_attr, stacksize_attr)));
    unsigned long startFuncVal = *((unsigned long *)(ptr + offsetof(struct pthread, start_routine)));

    printf("TID = %u, offset = %u\n", tidVal, offsetof(struct pthread, tid));
    printf("STACK = 0x%lx, offset = %u\n", stackAddrVal, offsetof(struct pthread, attr) + offsetof(struct pthread_attr, stackaddr_attr));
    printf("SIZE = 0x%lx, offset = %u\n", stackSizeVal, offsetof(struct pthread, attr) + offsetof(struct pthread_attr, stacksize_attr));
    printf("START = 0x%lx, offset = %u\n", startFuncVal, offsetof(struct pthread, start_routine));

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t t;
    void *result;
    pthread_attr_init(&attr);
    pthread_create(&t, &attr, foo, NULL);
    pthread_join(t, &result);

    return 0;
}
*/
