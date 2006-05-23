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

// $Id: linux.h,v 1.30 2006/05/23 06:39:50 jaw Exp $

#if !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(ia64_unknown_linux2_4)

#error "invalid architecture-os inclusion"
#endif

class process;

#ifndef LINUX_PD_HDR
#define LINUX_PD_HDR

#include <sys/param.h>
#include <pthread.h>
#include "common/h/Types.h"

#if !defined( ia64_unknown_linux2_4 )
#define BYTES_TO_SAVE   256
#else
/* More than the number of bundles necessary for loadDYNINSTlib()'s code. */
#define CODE_BUFFER_SIZE	512
#define BYTES_TO_SAVE		(CODE_BUFFER_SIZE * 16)
#endif

#define EXIT_NAME "_exit"

#if !defined(arch_x86_64)
#define SIGNAL_HANDLER	 "__restore"
#else
#define SIGNAL_HANDLER   "__restore_rt"
#endif

#if defined(arch_x86) || defined(arch_x86_64)
//Constant values used for the registers in the vsyscall page.
#define DW_CFA  0
#define DW_ESP 4
#define DW_EBP 5
#define DW_PC  8
#define MAX_DW_VALUE 8

Address getRegValueAtFrame(void *ehf, Address pc, int reg, int *reg_map,
                           process *p, bool *error);
#endif

typedef int handleT; // a /proc file descriptor

#if defined( ia64_unknown_linux2_4 )
#include "linux-ia64.h"
#elif defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4)
#include "linux-x86.h"
#else
#error Invalid or unknown architecture-os inclusion
#endif

#include "unix.h"

class process;

/* For linux.C */
void printRegs( void * save );
Address findFunctionToHijack( process * p );

#ifndef WNOWAIT
#define WNOWAIT WNOHANG
#endif
bool get_linux_version(int &major, int &minor, int &subvers);
bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers);

bool attachToChild(int pid);

void calcVSyscallFrame(process *p);
struct maps_entries *getLinuxMaps(int pid, unsigned &maps_size);

//  no /proc, dummy function
typedef int procProcStatus_t;

#define PREMS_PRIVATE (1 << 4)
#define PREMS_SHARED  (1 << 3)
#define PREMS_READ    (1 << 2)
#define PREMS_WRITE   (1 << 1)
#define PREMS_EXEC    (1 << 0)

typedef struct maps_entries {
   Address start;
   Address end;
   unsigned prems;
   Address offset;
   int dev_major;
   int dev_minor;
   int inode;
   char path[512];
} map_entries;

#define INDEPENDENT_LWP_CONTROL true

class SignalGenerator;
class eventLock;

class WaitpidMux {

  typedef struct waitpid_ret_pair {
     int pid;
     int status;
  } waitpid_ret_pair_t;

  typedef struct pid_generator_pair {
     int pid;
     SignalGenerator *sg;
  } pid_generator_pair_t;

  public:
   int waitpid(SignalGenerator *me, int *status);
   
   bool registerProcess(SignalGenerator *me); 
   bool registerLWP(unsigned lwp_id, SignalGenerator *me);
   bool unregisterLWP(unsigned lwp_id, SignalGenerator *me);
   bool unregisterProcess(SignalGenerator *me);
   void forceWaitpidReturn();

   WaitpidMux() :
     isInWaitpid(false),
     isInWaitLock(false),
     waitpid_thread_id(0L),
     forcedExit(false),
     waiter_exists(0)
   {}

  private:
   pdvector< pdpair< SignalGenerator *, pdvector<waitpid_ret_pair> > > waitpid_results;
   bool isInWaitpid;
   bool isInWaitLock;
   unsigned long waitpid_thread_id;
   bool forcedExit;
   pdvector<waitpid_ret_pair> unassigned_events;
   pdvector<SignalGenerator *> first_timers;

   volatile int waiter_exists;
   static eventLock waiter_lock;
   pdvector<pid_generator_pair_t> pidgens;
   void addPidGen(int pid, SignalGenerator *sg);
   void removePidGen(int pid, SignalGenerator *sg);
   void removePidGen(SignalGenerator *sg);
   bool hasFirstTimer(SignalGenerator *me);

};

#endif
