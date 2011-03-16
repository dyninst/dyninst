/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

/*
 * dyn_lwp.h -- header file for LWP interaction
 * $Id: dyn_lwp.h,v 1.68 2008/04/15 16:43:14 roundy Exp $
 */

#if !defined(DYN_LWP_H)
#define DYN_LWP_H

#include "dyninstAPI/src/os.h"
#include "frame.h"
#include "syscalltrap.h"
#include "signalhandler.h"

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5)
#include <procfs.h>
#endif

#if defined(os_aix)
#include <sys/procfs.h>
#endif

// note: handleT is normally unsigned on unix platforms, void * for 
// NT (as needed) defined in os.h

enum { NoSignal = -1 };

/*
 * The dyn_lwp class wraps a kernel thread (lightweight process, or LWP)
 * for use by dyninst/paradyn. It serves mainly as a location for
 * thread-specific data and functions.
 */

class DebuggerInterface;

class dyn_lwp
{
  friend class DebuggerInterface;
  friend class process;
  friend class dyn_thread;

  bool getRegisters_(struct dyn_saved_regs *regs, bool includeFP);
  bool restoreRegisters_(const struct dyn_saved_regs &regs, bool includeFP);

 public:
  // default constructor
  dyn_lwp();
  dyn_lwp(unsigned lwp, process *proc);
  dyn_lwp(const dyn_lwp &l);

  ~dyn_lwp();       // we only want process::deleteLWP to do this

  // Returns a struct used by changePC/restoreRegisters
  bool getRegisters(struct dyn_saved_regs *regs, bool includeFP = true);
  // Sets register file to values retrieved by getRegisters
  bool restoreRegisters(const struct dyn_saved_regs &regs, bool includeFP = true);
  // Changes PC to the given address. If regs is non-NULL,
  // sets register values as above (restoreRegisters), then changes PC
  bool changePC(Address addr, struct dyn_saved_regs *regs);
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
  bool clearOPC();
#endif
  // Partially implemented: will return default iRPC result value
  // on many platforms, ignoring register argument
  Address readRegister(Register reg);
  
  // Unimplemented
  //bool setRegister(Register reg, Address value);


#if defined(cap_syscall_trap)
  // True iff lwp is executing in the kernel
  bool executingSystemCall();
  // What syscall are we in?
  Address getCurrentSyscall();
  // Set a breakpoint at the system call exit
  // Actually sets up some state and calls the process version,
  // but hey...
  bool setSyscallExitTrap(syscallTrapCallbackLWP_t callback,
                          void *data);

  bool decodeSyscallTrap(EventRecord &ev);
  bool handleSyscallTrap(EventRecord &ev, bool &continueHint);

  // Remove the trap. Either called by signal handling code,
  // or by whoever set the trap in the first place (if we don't
  // need it anymore).
  bool clearSyscallExitTrap();

  // Query functions for syscall exits
  bool isWaitingForSyscall() const;
#endif // cap_syscall_trap

  
  int getLastSignal() { return lastSig_; }
  void setSignal(int sig) { lastSig_ = sig; }

  // On Alpha we need to change the PC as part of restarting the
  // process, so changePC just sets this value. continueProc then handles
  // the dirty work. 
  Address changedPCvalue;

  // Returns the active frame of the LWP
  Frame getActiveFrame();

  // Walk the stack of the given LWP
  bool walkStack(pdvector<Frame> &stackWalk, bool ignoreRPC = false);
  bool markRunningIRPC();
  void markDoneRunningIRPC();
  bool waitUntilStopped();

  processState status() const { return status_;}
  std::string getStatusAsString() const; // useful for debug printing etc.
  // to set dyn_lwp status, use process::set_lwp_status()
  void internal_lwp_set_status___(processState st);
  
  bool pauseLWP(bool shouldWaitUntilStopped = true);
  bool stop_(); // formerly OS::osStop
  bool continueLWP(int signalToContinueWith = NoSignal, 
                   bool clear_stackwalk = true);

#if defined( os_linux )
  bool continueLWP_(int signalToContinueWith, bool ignore_suppress = false);
#else
  bool continueLWP_(int signalToContinueWith);
#endif

  bool writeDataSpace(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readDataSpace(const void *inTracedProcess, u_int amount, void *inSelf);
  bool writeTextWord(caddr_t inTracedProcess, int data);
  bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readTextSpace(const void *inTracedProcess, u_int amount, void *inSelf);

  Address step_next_insn();

#if defined( os_linux )
  bool removeSigStop();  
  bool isRunning() const;
  bool isWaitingForStop() const;
#endif
  void clearStackwalk();

#if defined(cap_proc) && defined(os_aix)
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
  bool get_status(procProcStatus_t *status) const;
  //should only be called from process::set_status() or process::set_lwp_status

  bool isRunning() const;
#endif  
#if defined (os_osf)
  bool get_status(procProcStatus_t *status) const;
#endif
  
  // Access methods
  unsigned get_lwp_id() const { return lwp_id_; };

  int getPid() const;
  handleT get_fd() const { return fd_;  };

  bool is_attached() const    { return is_attached_; }
  void setIsAttached(bool newst) { is_attached_ = newst; }

  handleT ctl_fd() const { 
     assert(is_attached());
     return ctl_fd_;
  };
  handleT status_fd() const {
     assert(is_attached());
     return status_fd_;
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
  void set_lwp_id(int newid) {
     lwp_id_ = newid;
  }
#if defined (os_windows)
  Address getThreadInfoBlockAddr();
#endif

  int changeMemoryProtections(Address addr, Offset size, unsigned rights);

  bool is_dead() const { return is_dead_; }
  void set_dead() { is_dead_ = true; }

  // Open and close (if necessary) the file descriptor/handle. Used
  // by /proc-based platforms. Moved outside the constructor for
  // error reporting reasons. 
  // Platform-specific method
  bool attach();
  void detach();
  process *proc() { return proc_; }

  bool isSingleStepping() { return singleStepping; }
  void setSingleStepping(bool nval) { singleStepping = nval; }

  void setDebuggerLWP(bool newval) { is_debugger_lwp = newval; }
  bool isDebuggerLWP() { return is_debugger_lwp; }

  // Solaris uses a dedicated LWP to handle signal dispatch. Read all about it:
  // http://developers.sun.com/solaris/articles/signalprimer.html
  // For ease of use, returns false on non-Solaris platforms.
  bool is_asLWP();


  //  dumpRegisters:  dump a select set of registers, useful for when 
  //  the mutatee crashes, or for debug output.
  void dumpRegisters();
 private:
  // Internal system call trap functions

  // What if the wrong lwp hits the trap?
  bool stepPastSyscallTrap();
  volatile processState status_;

  bool representativeLWP_attach_();  // os specific
  bool realLWP_attach_();   // os specific
  void representativeLWP_detach_();   // os specific
  void realLWP_detach_();   // os specific
  void closeFD_();  // os specific

  process *proc_;
  unsigned lwp_id_;
  handleT fd_;

  // "new" /proc model: multiple files instead of ioctls.
  handleT ctl_fd_;
  handleT status_fd_;
  handleT as_fd_; // Process memory image (/proc)
  handleT auxv_fd_;
  handleT map_fd_;
  handleT ps_fd_; // ps (/proc)

  handleT procHandle_; // Process-specific, as opposed to thread-specific,
                       // handle. Currently used by NT

  bool singleStepping;
  // System call interruption, currently for Solaris, only.  If the
  // process is sleeping in a system call during an inferior RPC
  // attempt, we interrupt the system call, perform the RPC, and
  // restart the system call.  (This var is defined on all platforms
  // to avoid platform-dependent initialization in process ctor.)
  bool stoppedInSyscall_;  
  Address postsyscallpc_;  // PC after the syscall is interrupted
  bool waiting_for_stop;
#if defined(cap_proc)
  // These variables are meaningful only when `stoppedInSyscall' is true.
  int stoppedSyscall_;     // The number of the interrupted syscall
  dyn_saved_regs *syscallreg_; // Registers during sleeping syscall
                          // (note we do not save FP registers)
  sigset_t sighold_;       // Blocked signals during sleeping syscall
#endif

  // Windows maintains a Thread Information Block (TIB) per-process,
  // we currently use it for identifying exception handlers and for
  // unsetting the "beingDebugged" bit for the process.  It's a bit
  // tricky to calculate the TIB pointer, so we cache it here
  Address threadInfoBlockAddr_;

  int lastSig_;
  // Pointer to the syscall trap data structure
  syscallTrap *trappedSyscall_;
  // Callback to be made when the syscall exits
  syscallTrapCallbackLWP_t trappedSyscallCallback_;
  void *trappedSyscallData_;

  // When we run an inferior RPC we cache the stackwalk of the
  // process and return that if anyone asks for a stack walk
  int_stackwalk cached_stackwalk;

  bool isRunningIRPC;
  bool isDoingAttach_;

  bool is_attached_;

  bool is_as_lwp_;

  bool is_dead_;
  bool is_debugger_lwp;
};

#endif
