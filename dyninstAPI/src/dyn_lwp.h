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

/*
 * dyn_lwp.h -- header file for LWP interaction
 * $Id: dyn_lwp.h,v 1.31 2004/07/23 20:38:59 tlmiller Exp $
 */

#if !defined(DYN_LWP_H)
#define DYN_LWP_H

#include "os.h"
#include "frame.h"
#include "common/h/vectorSet.h"
#include "syscalltrap.h"

#if !defined(BPATCH_LIBRARY)
//rawtime64
#include "rtinst/h/rtinst.h"
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include <procfs.h>
#endif

#if defined(os_aix)
#include <sys/procfs.h>
#endif

// note: handleT is normally unsigned on unix platforms, void * for 
// NT (as needed) defined in os.h

#if !defined(BPATCH_LIBRARY)
#ifdef PAPI
class papiMgr;
#endif
#endif

#if !defined(i386_unknown_nt4_0)  // so Windows code will compile on unix
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE 0
#endif
#endif


/*
 * The dyn_lwp class wraps a kernel thread (lightweight process, or LWP)
 * for use by dyninst/paradyn. It serves mainly as a location for
 * thread-specific data and functions.
 */

class dyn_lwp
{
  bool getRegisters_(struct dyn_saved_regs *regs);
  bool restoreRegisters_(const struct dyn_saved_regs &regs);

 public:
  // default constructor
  dyn_lwp();
  dyn_lwp(unsigned lwp, process *proc);
  dyn_lwp(const dyn_lwp &l);

  ~dyn_lwp();       // we only want process::deleteLWP to do this

  void clearCachedRegister() {
     if(cached_regs) {
        delete cached_regs;
        cached_regs = NULL;
     }
  }

  // Returns a struct used by changePC/restoreRegisters
  bool getRegisters(struct dyn_saved_regs *regs);
  // Sets register file to values retrieved by getRegisters
  bool restoreRegisters(const struct dyn_saved_regs &regs);
  // Changes PC to the given address. If regs is non-NULL,
  // sets register values as above (restoreRegisters), then changes PC
  bool changePC(Address addr, struct dyn_saved_regs *regs);
#if defined(i386_unknown_linux2_0)
  bool clearOPC();
#endif
  // Partially implemented: will return default iRPC result value
  // on many platforms, ignoring register argument
  Address readRegister(Register reg);
  
  // Unimplemented
  //bool setRegister(Register reg, Address value);

  // True iff lwp is executing in the kernel
  bool executingSystemCall();
  // And what syscall are we in (or return address)
  Address getCurrentSyscall();
  // Set a breakpoint at the system call exit
  // Actually sets up some state and calls the process version,
  // but hey...
  bool setSyscallExitTrap(syscallTrapCallbackLWP_t callback,
                          void *data);

  void clearSyscallExitTrapCallback() {
     trappedSyscallCallback_ = NULL;
  }

  // Clear the above, and perform any necessary emulation work
  bool clearSyscallExitTrap();
  // What if the wrong lwp hits the trap?
  bool stepPastSyscallTrap();
  // Query functions for syscall exits
  bool isWaitingForSyscall() const;
  int hasReachedSyscallTrap();
  
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
  bool markRunningIRPC();
  void markDoneRunningIRPC();
  bool waitUntilStopped();

  processState status() const { return status_;}
  // to set dyn_lwp status, use process::set_lwp_status()
  void internal_lwp_set_status___(processState st) {
     status_ = st;
  }
  
  enum { NoSignal = -1 };  // matches declaration in process.h

  bool pauseLWP(bool shouldWaitUntilStopped = true);
  bool stop_(); // formerly OS::osStop
  bool continueLWP(int signalToContinueWith = NoSignal);
  bool continueLWP_(int signalToContinueWith);

  bool writeDataSpace(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readDataSpace(const void *inTracedProcess, u_int amount, void *inSelf);
  bool writeTextWord(caddr_t inTracedProcess, int data);
  bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);

#if defined( os_linux )
  bool deliverPtrace(int req, Address addr, Address data);
  int deliverPtraceReturn(int req, Address addr, Address data);
#elif defined(rs6000_ibm_aix4_1) && !defined(AIX_PROC)
  bool deliverPtrace(int request, void *addr, int data, void *addr2);
#endif

#if defined( os_linux )
  bool isRunning() const;
  bool isStopPending() const;

  int numStopsPending() { return sigStopsQueued; }
  void deqStop() { if (sigStopsQueued > 0) sigStopsQueued--; }
  void enqStop() { sigStopsQueued++; }
#endif

#if defined(AIX_PROC)
  void reopen_fds(); // Re-open whatever FDs might need to be
#endif

  // This should be ifdef SOL_PROC or similar
#if defined(cap_proc_fd)
  // Implemented where aborting system calls is possible
  bool abortSyscall();
  // And restart a system call that was previously aborted
  // Technically: restore the system to the pre-syscall state
  bool restartSyscall();

  // Solaris: keep data in the LWP instead of in class process
  // Continue, clearing signals
  // Clear signals, leaved paused
  bool clearSignal();
  // Continue, forwarding signals
  bool get_status(lwpstatus_t *status) const;
  //should only be called from process::set_status() or process::set_lwp_status

  bool isRunning() const;
#endif  
  
  // Access methods
  unsigned get_lwp_id() const { return lwp_id_; };

  int getPid() const;
  handleT get_fd() const { return fd_;  };

  bool is_attached() const    { return is_attached_; }

  handleT ctl_fd() const { 
     assert(is_attached());
     return ctl_fd_;
  };
  handleT status_fd() const {
     assert(is_attached());
     return status_fd_;
  };
  handleT usage_fd() const {
     assert(is_attached());
     return usage_fd_;
  };  
  handleT as_fd() const {
     assert(is_attached());
     return as_fd_;
  }
  handleT auxv_fd() const {
     assert(is_attached());
     return auxv_fd_;
  }
  handleT map_fd() const {
     assert(is_attached());
     return map_fd_;
  }
  handleT ps_fd() const {
     assert(is_attached());
     return ps_fd_;
  }

  // ===  WINDOWS  ========================================
  bool isProcessHandleSet() const {
     return (procHandle_ != INVALID_HANDLE_VALUE);
  }
  handleT getProcessHandle() const {
     assert(isProcessHandleSet());
     return procHandle_;
  }
  void setProcessHandle( handleT h ) {
     procHandle_ = h;
  }

  bool isFileHandleSet() const {
     return (fd_ != INVALID_HANDLE_VALUE);
  }
  handleT getFileHandle() const {
     assert(isFileHandleSet());
     return fd_;
  }
  void setFileHandle( handleT h ) {
     fd_ = h;
  }

  
  // Open and close (if necessary) the file descriptor/handle. Used
  // by /proc-based platforms. Moved outside the constructor for
  // error reporting reasons. 
  // Platform-specific method
  bool attach();
  void detach();
  process *proc() { return proc_; }

#if !defined(BPATCH_LIBRARY)
#ifdef PAPI
  papiMgr* papi();
#endif
#endif
  
 private:
  processState status_;

  bool representativeLWP_attach_();  // os specific
  bool realLWP_attach_();   // os specific
  void representativeLWP_detach_();   // os specific
  void realLWP_detach_();   // os specific
  void closeFD_();  // os specific

  process *proc_;
  const unsigned lwp_id_;
  handleT fd_;

  // "new" /proc model: multiple files instead of ioctls.
  handleT ctl_fd_;
  handleT status_fd_;
  handleT usage_fd_;
  handleT as_fd_; // Process memory image (/proc)
  handleT auxv_fd_;
  handleT map_fd_;
  handleT ps_fd_; // ps (/proc)

  handleT procHandle_; // Process-specific, as opposed to thread-specific,
                       // handle. Currently used by NT

#if !defined(BPATCH_LIBRARY)
  rawTime64 hw_previous_;
  rawTime64 sw_previous_;
#endif
  
  // System call interruption, currently for Solaris, only.  If the
  // process is sleeping in a system call during an inferior RPC
  // attempt, we interrupt the system call, perform the RPC, and
  // restart the system call.  (This var is defined on all platforms
  // to avoid platform-dependent initialization in process ctor.)
  bool stoppedInSyscall_;  
  Address postsyscallpc_;  // PC after the syscall is interrupted
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5) || defined(AIX_PROC)
  // These variables are meaningful only when `stoppedInSyscall' is true.
  int stoppedSyscall_;     // The number of the interrupted syscall
  dyn_saved_regs *syscallreg_; // Registers during sleeping syscall
                          // (note we do not save FP registers)
  sigset_t sighold_;       // Blocked signals during sleeping syscall
#endif
#if defined( os_linux )
  int sigStopsQueued;
#endif

  // Pointer to the syscall trap data structure
  syscallTrap *trappedSyscall_;
  // Callback to be made when the syscall exits
  syscallTrapCallbackLWP_t trappedSyscallCallback_;
  void *trappedSyscallData_;

  struct dyn_saved_regs *cached_regs;  
  
  // When we run an inferior RPC we cache the stackwalk of the
  // process and return that if anyone asks for a stack walk
  pdvector<Frame> cachedStackWalk;
  bool isRunningIRPC;

  bool is_attached_;
};

#endif
