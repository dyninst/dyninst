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

//TODO: isRunning(), isStopped()...
//TODO: Bug if trying to stop an already stopped process

#ifndef PROCSTATE_H_
#define PROCSTATE_H_

#include "basetypes.h"
#include "dyn_regs.h"

#include <vector>
#include <map>
#include <queue>
#include <string>

namespace Dyninst {
namespace Stackwalker {

class LibraryState;
class ThreadState;
class Walker;

class ProcessState {
   friend class Walker;
protected:
   Dyninst::PID pid;
   std::string exec_path;
   LibraryState *library_tracker;
   Walker *walker;
   static std::map<Dyninst::PID, ProcessState *> proc_map;
   std::string executable_path;

   ProcessState(Dyninst::PID pid_ = 0, std::string executable_path_ = std::string(""));
   void setPid(Dyninst::PID pid_);
public:

  //look-up Process-State by pid
  static ProcessState* getProcessStateByPid(Dyninst::PID pid);

  //Read register in thread
  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val) = 0;
  
  //Read memory in process
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size) = 0;

  //Return list of available threads
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0;
  
  //Return the default thread
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0;

  //Return PID
  virtual Dyninst::PID getProcessId();

  //Return the size of an address in process in bytes
  virtual unsigned getAddressWidth() = 0;

  //Get Architecture, see dyn_regs.h
  virtual Dyninst::Architecture getArchitecture() = 0;

  virtual ~ProcessState();

  Walker *getWalker() const;

  void setLibraryTracker(LibraryState *);
  void setDefaultLibraryTracker();
  LibraryState *getLibraryTracker();

  //Allow initialization/uninitialization
  virtual bool preStackwalk(Dyninst::THR_ID tid);
  virtual bool postStackwalk(Dyninst::THR_ID tid);

  virtual bool isFirstParty() = 0;

  std::string getExecutablePath();
};

class ProcSelf : public ProcessState {
 public:
  ProcSelf(std::string exe_path = std::string(""));
  void initialize();

  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val);
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads);
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
  virtual unsigned getAddressWidth();
  virtual bool isFirstParty();
  virtual Dyninst::Architecture getArchitecture();
  virtual ~ProcSelf();
};

typedef enum {
  ps_neonatal,                // 0
  ps_attached_intermediate,
  ps_attached,
  ps_running,
  ps_exited,
  ps_errorstate               // 5
} proc_state;

typedef enum {
  dbg_err,           // 0
  dbg_exited,
  dbg_crashed,
  dbg_stopped,
  dbg_other,
  dbg_noevent,      // 5
  dbg_libraryload,
  // BG events are below.
  dbg_continued,
  dbg_mem_ack,
  dbg_setmem_ack,
  dbg_reg_ack,      // 10
  dbg_allregs_ack,
  dbg_setreg_ack,
  dbg_attached,
  dbg_thread_info,
  dbg_detached      // 15
} dbg_t;

class ProcDebug;

struct DebugEvent {
   dbg_t dbg;
   union {
      int idata;
      void *pdata;
   } data;
   unsigned size;
   ProcDebug *proc;
   ThreadState *thr;

   DebugEvent() : dbg(dbg_noevent), size(0), proc(NULL), thr(NULL) {}
};

struct procdebug_ltint
{
   bool operator()(int a, int b) const;
};

class ProcDebug : public ProcessState {
  friend class Walker;
  friend class ThreadState;
 protected:
  ProcDebug(Dyninst::PID pid, std::string executable="");
  ProcDebug(std::string executable, 
            const std::vector<std::string> &argv);
  
  /**
   * attach() is the top-level command, and is implemented by debug_attach and
   * debug_waitfor_attach. 
   **/
  virtual bool attach();
  static bool multi_attach(std::vector<ProcDebug *> &pids);


  virtual bool debug_attach(ThreadState *ts) = 0;
  virtual bool debug_waitfor_attach(ThreadState *ts);
  virtual bool debug_post_attach(ThreadState *ts);
  virtual bool debug_post_create();

  virtual bool create(std::string executable, 
                      const std::vector<std::string> &argv);
  virtual bool debug_create(std::string executable, 
                            const std::vector<std::string> &argv) = 0;
  virtual bool debug_waitfor_create();
  
  /**
   * pause() is the top-level command (under the public section), and is 
   * implemented by debug_pause() and debug_waitfor_pause()
   **/
  virtual bool pause_thread(ThreadState *thr);
  virtual bool debug_pause(ThreadState *thr) = 0;
  virtual bool debug_waitfor_pause(ThreadState *thr);

  virtual bool resume_thread(ThreadState *thr);
  virtual bool debug_continue(ThreadState *thr) = 0;
  virtual bool debug_continue_with(ThreadState *thr, long sig) = 0;
  virtual bool debug_waitfor_continue(ThreadState *thr);

  virtual bool debug_handle_signal(DebugEvent *ev) = 0;
  virtual bool debug_handle_event(DebugEvent ev) = 0;

  static DebugEvent debug_get_event(bool block);
  static bool debug_wait_and_handle(bool block, bool flush, bool &handled, dbg_t *event_type = NULL);

  static bool debug_waitfor(dbg_t event_type);

  proc_state state();
  void setState(proc_state p);

 public:
  
  static ProcDebug *newProcDebug(Dyninst::PID pid, std::string executable="");
  static bool newProcDebugSet(const std::vector<Dyninst::PID> &pids,
                              std::vector<ProcDebug *> &out_set);
  static ProcDebug *newProcDebug(std::string executable, 
                                 const std::vector<std::string> &argv);
  virtual ~ProcDebug();

  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val) = 0;
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size) = 0;
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &thrds) = 0;
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0;
  virtual unsigned getAddressWidth() = 0;

  virtual bool preStackwalk(Dyninst::THR_ID tid);
  virtual bool postStackwalk(Dyninst::THR_ID tid);

  
  virtual bool pause(Dyninst::THR_ID tid = NULL_THR_ID);
  virtual bool resume(Dyninst::THR_ID tid = NULL_THR_ID);
  virtual bool isTerminated();

  virtual bool detach(bool leave_stopped = false) = 0;

  static int getNotificationFD();

  static bool handleDebugEvent(bool block = false);
  virtual bool isFirstParty();

  typedef void (*sig_callback_func)(int &signum, ThreadState *thr);
  void registerSignalCallback(sig_callback_func f);

  virtual Dyninst::Architecture getArchitecture();

 protected:
  /**
   * Helper for polling for new threads.  Sees if the thread exists, 
   * creates a ThreadState if not, and handles errors.  Mechanism for 
   * finding thread ids is left to derived class.
   **/
  virtual bool add_new_thread(Dyninst::THR_ID tid);

  /**
   * Syntactic sugar for add_new_thread.  Use for adding lots of 
   * threads from your favorite iterable data structure.
   **/
  template <class Iterator>
  bool add_new_threads(Iterator start, Iterator end) 
  {
     bool good = true;
     for (Iterator i=start; i != end; i++) {
        if (!add_new_thread(*i)) good = false;
     }
     return good;
  }
   
  ThreadState *initial_thread;
  ThreadState *active_thread;
  typedef std::map<Dyninst::THR_ID, ThreadState*> thread_map_t;
  thread_map_t threads;
  sig_callback_func sigfunc;
 public:
  static int pipe_in;
  static int pipe_out;
};

//LibAddrPair.first = path to library, LibAddrPair.second = load address
typedef std::pair<std::string, Address> LibAddrPair;
typedef enum { library_load, library_unload} lib_change_t;
class LibraryState {
 protected:
   ProcessState *procstate;
 public:
   LibraryState(ProcessState *parent);
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0;
   virtual bool getLibraries(std::vector<LibAddrPair> &libs) = 0;
   virtual void notifyOfUpdate() = 0;
   virtual Address getLibTrapAddress() = 0;
   virtual bool getLibc(LibAddrPair &lc) = 0;
   virtual bool getLibthread(LibAddrPair &lt) = 0;
   virtual bool getAOut(LibAddrPair &ao) = 0;
   virtual ~LibraryState();
};

class ThreadState {
 public:
   static ThreadState* createThreadState(ProcDebug *parent, 
                                         Dyninst::THR_ID id = NULL_THR_ID,
                                         bool alrady_attached = false);
   virtual ~ThreadState();

   bool isStopped();
   void setStopped(bool s);
   
   bool userIsStopped();
   void setUserStopped(bool u);

   bool shouldResume();
   void setShouldResume(bool r);

   Dyninst::THR_ID getTid();

   proc_state state();
   void setState(proc_state s);
   
   ProcDebug *proc();

   //For internal use only
   void markPendingStop();
   void clearPendingStop();
   bool hasPendingStop();
protected:
   ThreadState(ProcDebug *p, Dyninst::THR_ID id);
   bool is_stopped;
   bool user_stopped;
   bool should_resume;
   Dyninst::THR_ID tid;
   proc_state thr_state;
   ProcDebug *parent;
   unsigned pending_sigstops;
};

// ----- Useful functors for dealing with ThreadStates. -----
// invokes setStopped on the thread.
struct set_stopped {
  bool value;
  set_stopped(bool v) : value(v) { }
  void operator()(ThreadState *ts) { ts->setStopped(value); }
};

// Sets thread state.
struct set_state {
  proc_state value;
  set_state(proc_state v) : value(v) { }
  void operator()(ThreadState *ts) { ts->setState(value); }
};

// Sets thread state.
struct set_should_resume {
  bool value;
  set_should_resume(bool v) : value(v) { }
  void operator()(ThreadState *ts) { ts->setShouldResume(value); }
};

}
}

#endif
