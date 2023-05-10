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

/* FreeBSD libc has stubs so a static version shouldn't need libpthreads */
#include <pthread.h>

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

static
int tkill(pid_t pid, long lwp, int sig) {
    static int has_tkill = 1;
    int result = 0;

    if( has_tkill ) {
        result = syscall(SYS_thr_kill2, pid, lwp, sig);
        if( 0 != result && ENOSYS == errno ) {
            has_tkill = 0;
        }
    }

    if( !has_tkill ) {
        result = kill(pid, sig);
    }

    return (result == 0);
}

void DYNINSTbreakPoint()
{
    if(DYNINSTstaticMode) return;

    DYNINST_break_point_event = 1;
    while( DYNINST_break_point_event ) {
        tkill(getpid(), dyn_lwp_self(), DYNINST_BREAKPOINT_SIGNUM);
    }
    /* Mutator resets to 0 */
}

static int failed_breakpoint = 0;
void uncaught_breakpoint(int sig)
{
   failed_breakpoint = 1;
}

void DYNINSTsafeBreakPoint()
{
    if(DYNINSTstaticMode) return;

    DYNINST_break_point_event = 1;
    while( DYNINST_break_point_event ) {
        tkill(getpid(), dyn_lwp_self(), SIGSTOP);
    }
    /* Mutator resets to 0 */

#if 0
    if( DYNINSTstaticMode ) return;
    DYNINST_break_point_event = 2;
    sigset_t emptyset;
    sigemptyset(&emptyset);

    // There is a bug with attaching to a stopped process on FreeBSD This
    // achieves the same result as long as Dyninst attaches to the process when
    // it is in sigsuspend
    while( DYNINST_break_point_event ) {
        sigsuspend(&emptyset);
    }
#endif
}

#if !defined(DYNINST_RT_STATIC_LIB)
static int get_dlopen_error() {
    const char *err_str;
    err_str = dlerror();
    if( err_str ) {
        strncpy(gLoadLibraryErrorString, err_str, (size_t) ERROR_STRING_LENGTH);
        return 1;
    }

    sprintf(gLoadLibraryErrorString, "unknown error withe dlopen");
    return 0;
}

int DYNINSTloadLibrary(char *libname)
{
    void *res;
    gLoadLibraryErrorString[0] = '\0';
    res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
    if( res ) return 1;

    get_dlopen_error();
    return 0;
}
#endif

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

int DYNINST_am_initial_thread( dyntid_t tid ) {
    /*
     * LWPs and PIDs are in different namespaces on FreeBSD.
     *
     * I don't really know a good way to determine this without
     * doing an expensive sysctl.
     *
     * Luckily, this function isn't used anymore
     */
    assert(!"This function is unimplemented on FreeBSD");
    return 0;
}

/** trap based instrumentation **/

#if defined(cap_mutatee_traps)

#include <ucontext.h>

#if defined(arch_x86) || defined(MUTATEE_32)
#define UC_PC(x) x->uc_mcontext.mc_eip
#elif defined(arch_x86_64)
#define UC_PC(x) x->uc_mcontext.mc_rip
#endif // UC_PC

extern unsigned long dyninstTrapTableUsed;
extern unsigned long dyninstTrapTableVersion;
extern trapMapping_t *dyninstTrapTable;
extern unsigned long dyninstTrapTableIsSorted;

/**
 * This comment is now obsolete, left for historic purposes
 *
 * Called by the SIGTRAP handler, dyninstTrapHandler.  This function is 
 * closly intwined with dyninstTrapHandler, don't modify one without 
 * understanding the other.
 *
 * This function sets up the calling context that was passed to the
 * SIGTRAP handler so that control will be redirected to our instrumentation
 * when we do the setcontext call.
 * 
 * There are a couple things that make this more difficult than it should be:
 *   1. The OS provided calling context is similar to the GLIBC calling context,
 *      but not compatible.  We'll create a new GLIBC compatible context and
 *      copy the possibly stomped registers from the OS context into it.  The
 *      incompatiblities seem to deal with FP and other special purpose registers.
 *   2. setcontext doesn't restore the flags register.  Thus dyninstTrapHandler
 *      will save the flags register first thing and pass us its value in the
 *      flags parameter.  We'll then push the instrumentation entry and flags
 *      onto the context's stack.  Instead of transfering control straight to the
 *      instrumentation, we'll actually go back to dyninstTrapHandler, which will
 *      do a popf/ret to restore flags and go to instrumentation.  The 'retPoint'
 *      parameter is the address in dyninstTrapHandler the popf/ret can be found.
 **/
void dyninstTrapHandler(int sig, siginfo_t *sg, ucontext_t *context)
{
   void *orig_ip;
   void *trap_to;

   orig_ip = UC_PC(context);
   assert(orig_ip);

   // Find the new IP we're going to and substitute. Leave everything else untouched
   if (DYNINSTstaticMode) {
      unsigned long zero = 0;
      unsigned long one = 1;
      struct trap_mapping_header *hdr = getStaticTrapMap((unsigned long) orig_ip);
      if (!hdr) return;

      assert(hdr);
      trapMapping_t *mapping = &(hdr->traps[0]);
      trap_to = dyninstTrapTranslate(orig_ip, 
                                     (unsigned long *) &hdr->num_entries, 
                                     &zero, 
                                     (volatile trapMapping_t **) &mapping,
                                     &one);
   }
   else {
      trap_to = dyninstTrapTranslate(orig_ip, 
                                     &dyninstTrapTableUsed,
                                     &dyninstTrapTableVersion,
                                     (volatile trapMapping_t **) &dyninstTrapTable,
                                     &dyninstTrapTableIsSorted);
                                     
   }
   UC_PC(context) = (long) trap_to;
}

#if defined(cap_binary_rewriter)

#define NUM_LIBRARIES 512 //Important, max number of rewritten libraries

#define WORD_SIZE (8 * sizeof(unsigned))
#define NUM_LIBRARIES_BITMASK_SIZE (1 + NUM_LIBRARIES / WORD_SIZE)
struct trap_mapping_header *all_headers[NUM_LIBRARIES];

static unsigned all_headers_current[NUM_LIBRARIES_BITMASK_SIZE];
static unsigned all_headers_last[NUM_LIBRARIES_BITMASK_SIZE];

#if !defined(arch_x86_64) || defined(MUTATEE_32)
typedef Elf32_Dyn ElfX_Dyn;
typedef Elf32_Ehdr ElfX_Ehdr;
#else
typedef Elf64_Dyn ElfX_Dyn;
typedef Elf64_Ehdr ElfX_Ehdr;
#endif

static int parse_libs();
static int parse_link_map(struct link_map *l);
static void clear_unloaded_libs();

static void set_bit(unsigned *bit_mask, int bit, char value);
static void clear_bitmask(unsigned *bit_mask);
static unsigned get_next_free_bitmask(unsigned *bit_mask, int last_pos);
static unsigned get_next_set_bitmask(unsigned *bit_mask, int last_pos);

static tc_lock_t trap_mapping_lock;

static struct trap_mapping_header *getStaticTrapMap(unsigned long addr)
{
   struct trap_mapping_header *header;
   int i;
   
   tc_lock_lock(&trap_mapping_lock);
   parse_libs();

   i = -1;
   for (;;) {
      i = get_next_set_bitmask(all_headers_current, i);
      assert(i >= 0 && i <= NUM_LIBRARIES);
      if (i == NUM_LIBRARIES) {
         header = NULL;
         goto done;
      }
      header = all_headers[i];
      if (addr >= header->low_entry && addr <= header->high_entry) {
         goto done;
      }
   }  
 done:
   tc_lock_unlock(&trap_mapping_lock);
   return header;
}

static struct link_map *getLinkMap() {
    struct link_map *map = NULL;
#if !defined(DYNINST_RT_STATIC_LIB)
    if( dlinfo(RTLD_SELF, RTLD_DI_LINKMAP, &map) ) {
        return NULL;
    }

    // Rewind the current link map pointer to find the
    // start of the list
    struct link_map *last_map = NULL;
    while( map != NULL ) {
        last_map = map;
        map = map->l_prev;
    }

    map = last_map;
#endif
    return map;
}

static int parse_libs()
{
   struct link_map *l_current;

   l_current = getLinkMap();
   if (!l_current)
      return -1;

   clear_bitmask(all_headers_current);
   while (l_current) {
      parse_link_map(l_current);
      l_current = l_current->l_next;
   }
   clear_unloaded_libs();

   return 0;
}

//parse_link_map return values
#define PARSED 0
#define NOT_REWRITTEN 1
#define ALREADY_PARSED 2
#define ERROR_INTERNAL -1
#define ERROR_FULL -2
static int parse_link_map(struct link_map *l) 
{
   ElfX_Dyn *dynamic_ptr;
   struct trap_mapping_header *header;
   unsigned int i, new_pos;

   dynamic_ptr = (ElfX_Dyn *) l->l_ld;
   if (!dynamic_ptr)
      return -1;

   assert(sizeof(dynamic_ptr->d_un.d_ptr) == sizeof(void *));
   for (; dynamic_ptr->d_tag != DT_NULL && dynamic_ptr->d_tag != DT_DYNINST; dynamic_ptr++);
   if (dynamic_ptr->d_tag == DT_NULL) {
      return NOT_REWRITTEN;
   }

   header = (struct trap_mapping_header *) (dynamic_ptr->d_un.d_val + l->l_addr);

   caddr_t libAddr = l->l_addr;

   // Executables have an implicit zero load address but the library load address
   // may be non-zero
   if( ((ElfX_Ehdr *)libAddr)->e_type == ET_EXEC ) {
       libAddr = 0;
   }else if( ((ElfX_Ehdr *)libAddr)->e_type == ET_DYN ) {
       // Account for library_adjust mechanism which is used for shared libraries
       // on FreeBSD
       libAddr += getpagesize();
   }

   header = (struct trap_mapping_header *) (dynamic_ptr->d_un.d_val + libAddr);
   
   if (header->signature != TRAP_HEADER_SIG)
      return ERROR_INTERNAL;
   if (header->pos != -1) {
      set_bit(all_headers_current, header->pos, 1);
      assert(all_headers[header->pos] == header);
      return ALREADY_PARSED;
   }
 
   for (i = 0; i < header->num_entries; i++)
   {
      header->traps[i].source = (void *) (((unsigned long) header->traps[i].source) + libAddr);
      header->traps[i].target = (void *) (((unsigned long) header->traps[i].target) + libAddr);
      if (!header->low_entry || header->low_entry > (unsigned long) header->traps[i].source)
         header->low_entry = (unsigned long) header->traps[i].source;
      if (!header->high_entry || header->high_entry < (unsigned long) header->traps[i].source)
         header->high_entry = (unsigned long) header->traps[i].source;
   }

   new_pos = get_next_free_bitmask(all_headers_last, -1);
   assert(new_pos >= 0 && new_pos < NUM_LIBRARIES);
   if (new_pos == NUM_LIBRARIES)
      return ERROR_FULL;

   header->pos = new_pos;
   all_headers[new_pos] = header;
   set_bit(all_headers_current, new_pos, 1);
   set_bit(all_headers_last, new_pos, 1);

   return PARSED;
}

static void clear_unloaded_libs()
{
   unsigned i;
   for (i = 0; i<NUM_LIBRARIES_BITMASK_SIZE; i++)
   {
      all_headers_last[i] = all_headers_current[i];
   }
}

static void set_bit(unsigned *bit_mask, int bit, char value) {
   assert(bit < NUM_LIBRARIES);
   unsigned *word = bit_mask + bit / WORD_SIZE;
   unsigned shift = bit % WORD_SIZE;
   if (value) {
      *word |= (1 << shift);
   }
   else {
      *word &= ~(1 << shift);
   }
}

static void clear_bitmask(unsigned *bit_mask) {
   unsigned i;
   for (i = 0; i < NUM_LIBRARIES_BITMASK_SIZE; i++) {
      bit_mask[i] = 0;
   }
}

static unsigned get_next_free_bitmask(unsigned *bit_mask, int last_pos) {
   unsigned i, j;
   j = last_pos+1;
   i = j / WORD_SIZE;
   for (; j < NUM_LIBRARIES; i++) {
      if (bit_mask[i] == (unsigned) -1) {
         j += WORD_SIZE;
         continue;
      }
      for (;;) {
         if (!((1 << (j % WORD_SIZE) & bit_mask[i]))) {
            return j;
         }
         j++;
         if (j % WORD_SIZE == 0) {
            break;
         }
      }
   }
   return NUM_LIBRARIES;
}

static unsigned get_next_set_bitmask(unsigned *bit_mask, int last_pos) {
   unsigned i, j;
   j = last_pos+1;
   i = j / WORD_SIZE;
   for (; j < NUM_LIBRARIES; i++) {
      if (bit_mask[i] == (unsigned) 0) {
         j += WORD_SIZE;
         continue;
      }
      for (;;) {
         if ((1 << (j % WORD_SIZE) & bit_mask[i])) {
            return j;
         }
         j++;
         if (j % WORD_SIZE == 0) {
            break;
         }
      }
   }
   return NUM_LIBRARIES;
}

#endif

#endif /* cap_mutatee_traps */

/*
 * Note: this program is for historical purposes only, we use libthread_db
 * now to get thread information.
 *
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
