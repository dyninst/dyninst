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

// $Id: linux.h,v 1.37 2007/12/04 18:05:24 legendre Exp $

#if !defined(os_vxworks)
#error "invalid architecture-os inclusion"
#endif


#ifndef VXWORKS_PD_HDR
#define VXWORKS_PD_HDR

class process;

#include <sys/param.h>
#include <pthread.h>
#include "common/h/Types.h"
#include "common/h/Vector.h"

#define BYTES_TO_SAVE   256
#define EXIT_NAME "_exit"

#define SIGNAL_HANDLER	 "__restore"

typedef int handleT; // a /proc file descriptor

#if defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4)
#include "linux-x86.h"
#elif defined(os_linux) && defined(arch_power)
#include "linux-power.h"
#elif defined(os_vxworks)
#else
#error Invalid or unknown architecture-os inclusion
#endif

#include "unix.h"
#include "dyninstAPI/h/BPatch.h"
//--------------------------------------------------------------------------
// Exported functions
//--------------------------------------------------------------------------
class mapped_object;
void launch_task(const std::string &filename, mapped_object *obj);
#include "symtabAPI/h/Symtab.h"
bool fixup_offsets(const std::string &filename, Dyninst::SymtabAPI::Symtab *linkedFile);
void addBreakpoint(Address bp);
bool wtxFindFunction(const char *name, Address &addr);
bool relocationTarget(const Address addr, Address *target);
void cleanupVxWorks();
bool doNotParseList(const std::vector<std::string> &name);
void disasAddress(const Address addr);
//--------------------------------------------------------------------------

#ifndef WNOWAIT
#define WNOWAIT WNOHANG
#endif

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

#if defined(arch_power)
struct dyn_saved_regs
{
    unsigned int iu[36]; // 36 regs in WTX_REG_SET_IU (Integer Unit)
};

enum dyn_wtx_iu_reg_map
{
    WTX_REG_R0 = 0,
    WTX_REG_R1,
    WTX_REG_R2,
    WTX_REG_R3,
    WTX_REG_R4,
    WTX_REG_R5,
    WTX_REG_R6,
    WTX_REG_R7,
    WTX_REG_R8,
    WTX_REG_R9,
    WTX_REG_R10,
    WTX_REG_R11,
    WTX_REG_R12,
    WTX_REG_R13,
    WTX_REG_R14,
    WTX_REG_R15,
    WTX_REG_R16,
    WTX_REG_R17,
    WTX_REG_R18,
    WTX_REG_R19,
    WTX_REG_R20,
    WTX_REG_R21,
    WTX_REG_R22,
    WTX_REG_R23,
    WTX_REG_R24,
    WTX_REG_R25,
    WTX_REG_R26,
    WTX_REG_R27,
    WTX_REG_R28,
    WTX_REG_R29,
    WTX_REG_R30,
    WTX_REG_R31,
    WTX_REG_MSR,
    WTX_REG_LR,
    WTX_REG_CTR,
    WTX_REG_PC
};

#define WTX_REG_IU_PC WTX_REG_PC
#define WTX_REG_IU_SP WTX_REG_R1
#define WTX_REG_IU_FP WTX_REG_R31

#elif defined(arch_x86)
struct dyn_saved_regs
{
    unsigned int iu[12]; // 12 regs in WTX_REG_SET_IU (Integer Unit)
};

enum dyn_wtx_iu_reg_map
{
    WTX_REG_EDI = 0,
    WTX_REG_ESI,
    WTX_REG_EBP,
    WTX_REG_ESP,
    WTX_REG_EBX,
    WTX_REG_EDX,
    WTX_REG_ECX,
    WTX_REG_EAX,
    WTX_REG_EFLAGS,
    WTX_REG_EIP,
    WTX_REG_STATUS,
    WTX_REG_UNKNOWN
};

#define WTX_REG_IU_PC WTX_REG_EIP
#define WTX_REG_IU_SP WTX_REG_ESP
#define WTX_REG_IU_FP WTX_REG_EBP

#endif

#endif
