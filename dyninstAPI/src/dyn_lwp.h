/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/*
 * dyn_lwp.h -- header file for LWP interaction
 * $Id: dyn_lwp.h,v 1.7 2002/12/20 07:49:56 jaw Exp $
 */

#if !defined(DYN_LWP_H)
#define DYN_LWP_H

#include "os.h"
#include "frame.h"
#include "common/h/vectorSet.h"
#include "rtinst/h/rtinst.h"

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif

// note: handleT is normally unsigned on unix platforms, void * for 
// NT (as needed) defined in os.h

class process;

/*
 * The dyn_lwp class wraps a kernel thread (lightweight process, or LWP)
 * for use by dyninst/paradyn. It serves mainly as a location for
 * thread-specific data and functions.
 */

class dyn_lwp
{
 public:
  // default constructor
  dyn_lwp();
  // UNIX constructor: lwp ID and process pointer
  dyn_lwp(unsigned lwp, process *proc);
  // NT constructor: pre-specified (or pre-open) file descriptor/handle
  dyn_lwp(unsigned lwp, handleT fd, process *proc);

  dyn_lwp(const dyn_lwp &l);
  ~dyn_lwp();

  // Returns a struct used by changePC/restoreRegisters
  struct dyn_saved_regs *getRegisters();
  // Sets register file to values retrieved by getRegisters
  bool restoreRegisters(struct dyn_saved_regs *regs);			
  // Changes PC to the given address. If regs is non-NULL,
  // sets register values as above (restoreRegisters), then changes PC
  bool changePC(Address addr, struct dyn_saved_regs *regs);

  // Partially implemented: will return default iRPC result value
  // on many platforms, ignoring register argument
  Address readRegister(Register reg);
  
  // Unimplemented
  //bool setRegister(Register reg, Address value);

  // True iff lwp is executing in the kernel
  bool executingSystemCall();

#if !defined(BPATCH_LIBRARY)
  // Timing functions
  rawTime64 getRawCpuTime_hw();
  rawTime64 getRawCpuTime_sw();
#endif

  // On Alpha we need to change the PC as part of restarting the
  // process, so changePC just sets this value. continueProc then handles
  // the dirty work. 
  Address changedPCvalue;

  // Returns the active frame of the LWP
  Frame getActiveFrame();

  // Walk the stack of the given LWP
  bool walkStack(pdvector<Frame> &stackWalk);

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
  // Implemented where aborting system calls is possible
  int abortSyscall();
  // Solaris: keep data in the LWP instead of in class process
  bool pauseLWP();
  bool continueLWP();
#endif  
  
  // Access methods
  unsigned get_lwp() const { return lwp_; };
  handleT get_fd() const { return fd_;  };

  // Open and close (if necessary) the file descriptor/handle. Used
  // by /proc-based platforms. Moved outside the constructor for
  // error reporting reasons. 
  // Platform-specific method
  bool openFD();
  void closeFD();
  process *proc() { return proc_; }

 private:

  process *proc_;
  const unsigned lwp_;
  handleT fd_;

  rawTime64 hw_previous_;
  rawTime64 sw_previous_;

  
  // System call interruption, currently for Solaris, only.  If the
  // process is sleeping in a system call during an inferior RPC
  // attempt, we interrupt the system call, perform the RPC, and
  // restart the system call.  (This var is defined on all platforms
  // to avoid platform-dependent initialization in process ctor.)
  bool stoppedInSyscall_;  
  Address postsyscallpc_;  // PC after the syscall is interrupted
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
  // These variables are meaningful only when `stoppedInSyscall' is true.
  int stoppedSyscall_;     // The number of the interrupted syscall
  prgregset_t syscallreg_; // Registers during sleeping syscall
                          // (note we do not save FP registers)
  sigset_t sighold_;       // Blocked signals during sleeping syscall
#endif
  
};

#endif
