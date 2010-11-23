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

// $Id: linux.h,v 1.37 2007/12/04 18:05:24 legendre Exp $

#if !defined(os_linux)
#error "invalid architecture-os inclusion"
#endif

class PCProcess;

#ifndef LINUX_PD_HDR
#define LINUX_PD_HDR

#include <sys/param.h>
#include <pthread.h>
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"

#include "common/h/parseauxv.h"

#define BYTES_TO_SAVE   256

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
#if 0
#define DW_RSP 7
#define DW_RBP 6
#define DW_PC  16
#endif

#define MAX_DW_VALUE 17

Address getRegValueAtFrame(void *ehf, Address pc, int reg, 
                           Address *reg_map,
                           PCProcess *p, bool *error);
#endif

typedef int handleT; // a /proc file descriptor

#if defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4)
#include "linux-x86.h"
#elif defined(os_linux) && defined(arch_power)
#include "linux-power.h"
#else
#error Invalid or unknown architecture-os inclusion
#endif

#include "unix.h"

/* For linux.C */
void printRegs( void * save );

#ifndef WNOWAIT
#define WNOWAIT WNOHANG
#endif
bool get_linux_version(int &major, int &minor, int &subvers);
bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers);

bool attachToChild(int pid);

void calcVSyscallFrame(PCProcess *p);
struct maps_entries *getLinuxMaps(int pid, unsigned &maps_size);

//  no /proc, dummy function
typedef int procProcStatus_t;

#define PREMS_PRIVATE (1 << 4)
#define PREMS_SHARED  (1 << 3)
#define PREMS_READ    (1 << 2)
#define PREMS_WRITE   (1 << 1)
#define PREMS_EXEC    (1 << 0)

#define INDEPENDENT_LWP_CONTROL true

class SignalGenerator;
class eventLock;

typedef struct waitpid_ret_pair {
   int pid;
   int status;
} waitpid_ret_pair_t;

typedef struct pid_generator_pair {
   int pid;
   SignalGenerator *sg;
} pid_generator_pair_t;

class WaitpidMux {

  public:
   int waitpid(SignalGenerator *me, int *status);
   
   bool registerProcess(SignalGenerator *me); 
   bool registerLWP(unsigned lwp_id, SignalGenerator *me);
   bool unregisterLWP(unsigned lwp_id, SignalGenerator *me);
   bool unregisterProcess(SignalGenerator *me);
   void forceWaitpidReturn();
   bool suppressWaitpidActivity();
   bool resumeWaitpidActivity();
   int enqueueWaitpidValue(waitpid_ret_pair ev, SignalGenerator *event_owner);

   pthread_mutex_t waiter_mutex;
   pthread_cond_t waiter_condvar;

   WaitpidMux() :
     isInWaitpid(false),
     isInWaitLock(false),
     waitpid_thread_id(0L),
     forcedExit(false),
     pause_flag(false),
     waiter_exists(0)
   {}

  private:
   bool isInWaitpid;
   bool isInWaitLock;
   unsigned long waitpid_thread_id;
   bool forcedExit;
   bool pause_flag;
   pdvector<waitpid_ret_pair> unassigned_events;
   pdvector<SignalGenerator *> first_timers;

   volatile int waiter_exists;
   pdvector<pid_generator_pair_t> pidgens;
   void addPidGen(int pid, SignalGenerator *sg);
   void removePidGen(int pid, SignalGenerator *sg);
   void removePidGen(SignalGenerator *sg);
   bool hasFirstTimer(SignalGenerator *me);
};

#endif
