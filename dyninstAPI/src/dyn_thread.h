/*
 * Copyright (c) 1996-2001 Barton P. Miller
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

// $Id: dyn_thread.h,v 1.2 2002/10/18 22:41:12 bernat Exp $

#ifndef _DYNTHREAD_H_
#define _DYNTHREAD_H_

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inferiorRPC.h"

class Frame;
class dyn_lwp;

class dyn_thread {
 public:
  //
  dyn_thread(process *pproc) : 
    pos(0),
    stack_addr(0),
    start_pc(0),
    start_func(NULL),
    rid(NULL),
    pending_tramp_addr( ADDR_NULL ),
    in_IRPC(false), in_syscall(false)
    { 
      proc = pproc; 
      ppid = pproc->getPid();
      lwp  = pproc->getDefaultLWP();
    }
  dyn_thread(process *proc_, unsigned tid_, unsigned pos_, dyn_lwp *lwp_) :
    tid(tid_),
    pos(pos_),
    lwp(lwp_),
    stack_addr(0),
    start_pc(0),
    start_func(NULL),
    rid(NULL),
    pending_tramp_addr( ADDR_NULL ),
    in_IRPC(false), in_syscall(false)
    {
      proc = proc_;
      ppid = proc_->getPid();
    }
  dyn_thread(process *parent, dyn_thread *src) {
    assert(src && parent);
    lwp = src->lwp;
    ppid = parent->getPid();
    pos = src->pos;
    stack_addr = src->stack_addr;
    start_pc = src->start_pc;
    start_func = src->start_func;
    rid = src->rid;
    proc = parent;
    pending_tramp_addr = ADDR_NULL;
    in_IRPC = false;
    in_syscall = false;
  }
  ~dyn_thread() {
  }
  
  // Get the active frame (PC, SP, FP) of the thread
  // calls dyn_lwp::getActiveFrame if necessary
  // Note: OS specific, defined in <OS>MT.C files
  Frame getActiveFrame();

  // Walk the stack of the thread
  bool walkStack(vector<Frame> &stackWalk);

  bool updateLWP();
  
  unsigned       get_tid()           const { return(tid); }
  unsigned       get_pos()           const { return(pos); }
  dyn_lwp *      get_lwp();
  unsigned       get_stack_addr()    const { return(stack_addr); }
  int            get_ppid()          const { return(ppid); }
  resource*      get_rid()                 { return(rid); }
  process*       get_proc()                { return(proc); }
  function_base* get_start_func()          { return(start_func); }
  unsigned       get_start_pc()      const { return(start_pc); }
  void*          get_resumestate_p()       { return resumestate_p; }
#if defined(MT_THREAD)
  rawTime64  getInferiorVtime(virtualTimer*, bool&);
#endif
  void update_tid          (unsigned tid_)        { tid = tid_; }
  void update_pos          (unsigned pos_)        { pos = pos_; }
  void update_lwp          (dyn_lwp *lwp_)        { lwp = lwp_; }
  void update_rid          (resource *rid_)       { rid = rid_; } 
  void update_stack_addr   (unsigned stack_addr_) { stack_addr=stack_addr_; }
  void update_start_pc     (unsigned start_pc_)   { start_pc=start_pc_; }
  void update_start_func   (function_base *pdf)   { start_func=pdf; }
  void update_resumestate_p(void* resumestate_p_) { resumestate_p=resumestate_p_; }
  
  Address get_pending_tramp_addr( void ) const	{ return pending_tramp_addr; }
  void set_pending_tramp_addr( Address a )	{ pending_tramp_addr = a; }

  void scheduleIRPC(inferiorRPCtoDo todo);
  bool readyIRPC();
  inferiorRPCtoDo peekIRPC();
  inferiorRPCtoDo popIRPC();
  void runIRPC(inferiorRPCinProgress running);
  void setRunningIRPC();
  inferiorRPCinProgress getIRPC();
  void clearRunningIRPC();
  bool isRunningIRPC();
  void setInSyscall();
  void clearInSyscall();
  bool isInSyscall();
  
  ///
 private:
  int ppid;

  unsigned tid;
  unsigned pos;
  dyn_lwp *lwp;
  unsigned stack_addr;
  unsigned start_pc ;
  void*    resumestate_p; //platform specific
  function_base *start_func ;
  resource *rid;
  process *proc;
  Address pending_tramp_addr;	// address of pending instrumentation
  // currently used on NT only

  // For multithread
  vectorSet<inferiorRPCtoDo> thrRPCsWaitingToStart;
  inferiorRPCinProgress thrCurrRunningRPC;
  bool in_IRPC;
  bool in_syscall;

};

#endif
