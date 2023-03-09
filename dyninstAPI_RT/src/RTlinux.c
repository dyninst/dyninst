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

#define _GNU_SOURCE

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTthread.h"
#include "dyninstAPI_RT/src/RTcommon.h"
#include "unaligned_memory_access.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#if !defined(DYNINST_RT_STATIC_LIB)
#include <dlfcn.h>
#endif

#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <link.h>

#if defined(DYNINST_RT_STATIC_LIB)
/*
 * The weak symbol here removes the dependence of the static version of this
 * library on pthread_self. If pthread_self is available, then it will be
 * linked.  Otherwise, the linker will ignore it.
 */
#pragma weak pthread_self
extern pthread_t pthread_self(void);
#else
#include <pthread.h>
#endif

extern double DYNINSTstaticHeap_512K_lowmemHeap_1[];
extern double DYNINSTstaticHeap_16M_anyHeap_1[];
extern unsigned long sizeOfLowMemHeap1;
extern unsigned long sizeOfAnyHeap1;

static struct trap_mapping_header *getStaticTrapMap(unsigned long addr);

#if defined(arch_power) && defined(arch_64bit) && defined(os_linux)
unsigned long DYNINSTlinkSave;
unsigned long DYNINSTtocSave;
#endif

/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

#ifndef SYS_tkill
#define SYS_tkill 238
#endif

int t_kill(int pid, int sig) {
    static int has_tkill = 1;
    long int result = 0;
    if (has_tkill) {
        result = syscall(SYS_tkill, pid, sig);
        if (result == -1 && errno == ENOSYS) {
            has_tkill = 0;
        }
    }
    if (!has_tkill) {
        result = kill(pid, sig);
    }

    return (result == 0);
}

void DYNINSTbreakPoint(void)
{
   if (DYNINSTstaticMode)
      return;
   // Call into a funtion that contains a 
   // trap instruction. 
   DYNINSTtrapFunction();
}

static int failed_breakpoint = 0;
void uncaught_breakpoint(int sig)
{
   (void)sig; /* unused parameter */
   failed_breakpoint = 1;
}

void DYNINSTsafeBreakPoint(void)
{
   if (DYNINSTstaticMode)
      return;

    DYNINST_break_point_event = 2; /* Not the same as above */
    //    while (DYNINST_break_point_event)
    kill(dyn_lwp_self(), SIGSTOP);
}

void mark_heaps_exec(void) {
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

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * OS initialization function
************************************************************************/
int DYNINST_sysEntry;

#if !defined(DYNINST_RT_STATIC_LIB)
/*
 * For now, removing dependence of static version of this library
 * on libdl.
 */
typedef struct dlopen_args {
  const char *libname;
  int mode;
  void *result;
  void *caller;
} dlopen_args_t;

void *(*DYNINST_do_dlopen)(dlopen_args_t *) = NULL;

static int get_dlopen_error(void) {
   char *err_str;
   err_str = dlerror();
   if (err_str) {
      strncpy(gLoadLibraryErrorString, err_str, (size_t) ERROR_STRING_LENGTH);
      return 1;
   }
   else {
      sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
      return 0;
   }
   return 0;
}

int DYNINSTloadLibrary(char *libname)
{
   void *res;
   gLoadLibraryErrorString[0]='\0';
   res = dlopen(libname, RTLD_LAZY | RTLD_GLOBAL);
   if (res)
   {
      return 1;
   }

   get_dlopen_error();
#if defined(arch_x86)
   /* dlopen on recent glibcs has a "security check" so that
      only registered modules can call it. Unfortunately, progs
      that don't include libdl break this check, so that we
      can only call _dl_open (the dlopen worker function) from
      within glibc. We do this by calling do_dlopen
      We fool this check by calling an addr written by the
      mutator */
   if (strstr(gLoadLibraryErrorString, "invalid caller") != NULL &&
       DYNINST_do_dlopen != NULL) {
      dlopen_args_t args;
      args.libname = libname;
      args.mode = RTLD_NOW | RTLD_GLOBAL;
      args.result = 0;
      args.caller = (void *)DYNINST_do_dlopen;
      // There's a do_dlopen function in glibc. However, it's _not_
      // exported; thus, getting the address is a bit of a pain.

      (*DYNINST_do_dlopen)(&args);
      // Duplicate the above
      if (args.result != NULL)
      {
         return 1;
      }
      else
         get_dlopen_error();
   }
#endif
   return 0;
}
#endif

//Define this value so that we can compile on a system that doesn't have
// gettid and still run on one that does.
#if !defined(SYS_gettid)

#if defined(arch_x86)
#define SYS_gettid 224
#elif defined(arch_x86_64)
#define SYS_gettid 186
#endif

#endif

int dyn_lwp_self(void)
{
   static int gettid_not_valid = 0;
   int result;

   if (gettid_not_valid)
      return getpid();

   result = syscall((long int) SYS_gettid);
   if (result == -1 && errno == ENOSYS)
   {
      gettid_not_valid = 1;
      return getpid();
   }
   return result;
}

int dyn_pid_self(void)
{
   return getpid();
}

dyntid_t (*DYNINST_pthread_self)(void);

dyntid_t dyn_pthread_self(void)
{
   dyntid_t me;
   if (DYNINSTstaticMode) {
#if defined(DYNINST_RT_STATIC_LIB)
       /* This special case is necessary because the static
        * version of libc doesn't define a version of pthread_self
        * unlike the shared version of the library.
        */
       if( !pthread_self ) {
           return (dyntid_t) DYNINST_SINGLETHREADED;
       }
#endif
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
	(void)tid; /* unused parameter */
	if( dyn_lwp_self() == getpid() ) {
		return 1;
   }
	return 0;
} /* end DYNINST_am_initial_thread() */

#if defined(cap_mutatee_traps)

#include <ucontext.h>

// Register numbers experimentally verified

#if defined(arch_x86)
  #define UC_PC(x) x->uc_mcontext.gregs[14]
#elif defined(arch_x86_64)
  #if defined(MUTATEE_32)
    #define UC_PC(x) x->uc_mcontext.gregs[14]
  #else // 64-bit
    #define UC_PC(x) x->uc_mcontext.gregs[16]
  #endif // amd-64
#elif defined(arch_power)
  #if defined(arch_64bit)
    #define UC_PC(x) x->uc_mcontext.regs->nip
  #endif // power
#elif defined(arch_aarch64)
	//#warning "UC_PC: in aarch64, pc is not directly accessable."
	//aarch64 pc is not one of 31 GPRs, but an independent reg
	#define UC_PC(x) x->uc_mcontext.pc
#endif // UC_PC

extern volatile unsigned long dyninstTrapTableUsed;
extern volatile unsigned long dyninstTrapTableVersion;
extern volatile trapMapping_t *dyninstTrapTable;
extern volatile unsigned long dyninstTrapTableIsSorted;

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
   (void)sig; /* unused parameter */
   (void)sg; /* unused parameter */

   orig_ip = (void *) UC_PC(context);
   assert(orig_ip);
   // Find the new IP we're going to and substitute. Leave everything else untouched.
   if (DYNINSTstaticMode) {
      unsigned long zero = 0;
      unsigned long one = 1;
      struct trap_mapping_header *hdr = getStaticTrapMap((unsigned long) orig_ip);
      assert(hdr);
      volatile trapMapping_t *mapping = &(hdr->traps[0]);
      trap_to = dyninstTrapTranslate(orig_ip,
                                     CAST_WITHOUT_ALIGNMENT_WARNING(unsigned long *, &hdr->num_entries),
                                     &zero,
                                     &mapping,
                                     &one);
   }
   else {
      trap_to = dyninstTrapTranslate(orig_ip,
                                     &dyninstTrapTableUsed,
                                     &dyninstTrapTableVersion,
                                     &dyninstTrapTable,
                                     &dyninstTrapTableIsSorted);

   }
   UC_PC(context) = (long) trap_to;
}

#if defined(cap_binary_rewriter)

extern struct r_debug _r_debug;

/* Verify that the r_debug variable is visible */
void r_debugCheck(void) { assert(_r_debug.r_map); }

#define NUM_LIBRARIES 512 //Important, max number of rewritten libraries

#define WORD_SIZE (8 * sizeof(unsigned))
#define NUM_LIBRARIES_BITMASK_SIZE (1 + NUM_LIBRARIES / WORD_SIZE)
struct trap_mapping_header *all_headers[NUM_LIBRARIES];

#if !defined(arch_x86_64) || defined(MUTATEE_32)
typedef Elf32_Dyn ElfX_Dyn;
#else
typedef Elf64_Dyn ElfX_Dyn;
#endif

struct trap_mapping_header *getStaticTrapMap(unsigned long addr);

#if !defined (arch_aarch64)
static unsigned all_headers_current[NUM_LIBRARIES_BITMASK_SIZE];
static unsigned all_headers_last[NUM_LIBRARIES_BITMASK_SIZE];

static int parse_libs(void);
static int parse_link_map(struct link_map *l);
static void clear_unloaded_libs(void);

static void set_bit(unsigned *bit_mask, int bit, char value);
//static char get_bit(unsigned *bit_mask, int bit);
static void clear_bitmask(unsigned *bit_mask);
static unsigned get_next_free_bitmask(unsigned *bit_mask, int last_pos);
static unsigned get_next_set_bitmask(unsigned *bit_mask, int last_pos);

static tc_lock_t trap_mapping_lock;
#endif

static struct trap_mapping_header *getStaticTrapMap(unsigned long addr)
{
#if !defined (arch_aarch64)
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
         rtdebug_printf("%s[%d]:  getStaticTrapMap: returning NULL\n", __FILE__, __LINE__);
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
#else
	// Silence compiler warnings
    (void)addr;
	assert(0);
	return NULL;
#endif
}

#if !defined (arch_aarch64)
static int parse_libs(void)
{
   struct link_map *l_current;

   l_current = _r_debug.r_map;
   if (!l_current) {
        rtdebug_printf("%s[%d]:  parse_libs: _r_debug.r_map was not set\n", __FILE__, __LINE__);
       return -1;
   }

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

   if (header->signature != TRAP_HEADER_SIG)
      return ERROR_INTERNAL;
   if (header->pos != -1) {
      set_bit(all_headers_current, header->pos, 1);
      assert(all_headers[header->pos] == header);
      return ALREADY_PARSED;
   }

   for (i = 0; i < header->num_entries; i++)
   {
      header->traps[i].source = (void *) (((unsigned long) header->traps[i].source) + l->l_addr);
      header->traps[i].target = (void *) (((unsigned long) header->traps[i].target) + l->l_addr);
      if (!header->low_entry || header->low_entry > (unsigned long) header->traps[i].source)
         header->low_entry = (unsigned long) header->traps[i].source;
      if (!header->high_entry || header->high_entry < (unsigned long) header->traps[i].source)
         header->high_entry = (unsigned long) header->traps[i].source;
   }

   new_pos = get_next_free_bitmask(all_headers_last, -1);
   assert(new_pos < NUM_LIBRARIES);
   if (new_pos == NUM_LIBRARIES)
      return ERROR_FULL;

   header->pos = new_pos;
   all_headers[new_pos] = header;
   set_bit(all_headers_current, new_pos, 1);
   set_bit(all_headers_last, new_pos, 1);

   return PARSED;
}

static void clear_unloaded_libs(void)
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

//Wasn't actually needed
/*
static char get_bit(unsigned *bit_mask, int bit) {
   assert(bit < NUM_LIBRARIES);
   unsigned *word = bit_mask + bit / WORD_SIZE;
   unsigned shift = bit % WORD_SIZE;
   return (*word & (1 << shift)) ? 1 : 0;
}
*/

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

#endif



#endif /* cap_mutatee_traps */

#if defined(cap_binary_rewriter) && !defined(DYNINST_RT_STATIC_LIB)
/* For a static binary, all global constructors are combined in an undefined
 * order. Also, DYNINSTBaseInit must be run after all global constructors have
 * been run. Since the order of global constructors is undefined, DYNINSTBaseInit
 * cannot be run as a constructor in static binaries. Instead, it is run from a
 * special constructor handler that processes all the global constructors in
 * the binary. Leaving this code in would create a global constructor for the
 * function runDYNINSTBaseInit(). See DYNINSTglobal_ctors_handler.
 */
extern void r_debugCheck(void);
extern void DYNINSTBaseInit(void);
void runDYNINSTBaseInit(void) __attribute__((constructor));
void runDYNINSTBaseInit(void)
{
    r_debugCheck();
   DYNINSTBaseInit();
}
#endif


/*
//Small program for finding the correct values to fill in pos_in_pthreadt
// above
#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define gettid() syscall(SYS_gettid)

pthread_attr_t attr;

void *foo(void *f) {
  pid_t pid, tid;
  unsigned stack_addr;
  unsigned best_stack = 0xffffffff;
  int best_stack_pos = 0;
  void *start_func;
  int *p;
  int i = 0;
  pid = getpid();
  tid = gettid();
  start_func = foo;
  //x86 only.
  asm("movl %%ebp,%0" : "=r" (stack_addr));
  p = (int *) pthread_self();
  while (i < 1000)
  {
    if (*p == (unsigned) pid)
      printf("pid @ %d\n", i);
    if (*p == (unsigned) tid)
      printf("lwp @ %d\n", i);
    if (*p > stack_addr && *p < best_stack)
    {
      best_stack = *p;
      best_stack_pos = i;
    }
    if (*p == (unsigned) start_func)
      printf("func @ %d\n", i);
    i += sizeof(int);
    p++;
  }
  printf("stack @ %d\n", best_stack_pos);
  return NULL;
}

int main(int argc, char *argv[])
{
  pthread_t t;
  void *result;
  pthread_attr_init(&attr);
  pthread_create(&t, &attr, foo, NULL);
  pthread_join(t, &result);
  return 0;
}
*/

