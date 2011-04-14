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

// $Id: dyn_thread.h,v 1.31 2007/06/13 18:50:39 bernat Exp $

#ifndef _DYNTHREAD_H_
#define _DYNTHREAD_H_

#include "dyninstAPI/src/dyn_lwp.h"

typedef long dynthread_t;

class Frame;
class dyn_lwp;
class process;

class dyn_thread {
 private:
   Frame getActiveFrameMT();  // called by getActiveFrame

 public:
  //
  dyn_thread(process *pproc);
  dyn_thread(process *proc_, unsigned index_, dyn_lwp *lwp_);
  dyn_thread(dyn_thread *src, process *child, dyn_lwp *lwp_ = NULL);
  ~dyn_thread();
  
  // Get the active frame (PC, SP, FP) of the thread
  // calls dyn_lwp::getActiveFrame if necessary
  // Note: OS specific, defined in <OS>MT.C files
  Frame getActiveFrame();

  // Walk the stack of the thread
  bool walkStack(pdvector<Frame> &stackWalk);

  bool updateLWP();
  
  dynthread_t    get_tid()           const { return(tid); }
  int            get_index()           const { return(index); }
  dyn_lwp *      get_lwp();
  Address        get_stack_addr() const;
  int            get_ppid()          const { return(ppid); }
  process*       get_proc()                { return(proc); }
  func_instance*  get_start_func()          { return(start_func); }
  Address        get_start_pc()      const { return(start_pc); }
  void*          get_resumestate_p()       { return resumestate_p; }
  Address        get_indirect_start_addr() const { return indirect_start_func; }

  void update_tid          (dynthread_t tid_)        { tid = tid_; }
  void update_index        (unsigned index_);
  void update_lwp          (dyn_lwp *lwp_)        { lwp = lwp_; }
  void update_stack_addr   (Address stack_addr_);
  void update_start_pc     (Address start_pc_)   { start_pc=start_pc_; }
  void update_start_func   (func_instance *pdf)   { start_func=pdf; }
  void update_sfunc_indir  (Address addr)        {indirect_start_func = addr; }
  void update_resumestate_p(void* resumestate_p_) { resumestate_p=resumestate_p_; }
  
  Address get_pending_tramp_addr( void ) const	{ return pending_tramp_addr; }
  void set_pending_tramp_addr( Address a )	{ pending_tramp_addr = a; }
  bool is_exited()                         { return lwp->status() == exited; }
  func_instance *map_initial_func(func_instance *ifunc);

  ///
 private:
  int ppid;

  dynthread_t tid;
  int index;
  dyn_lwp *lwp;
  Address stack_addr;
  Address start_pc ;
  void*    resumestate_p; //platform specific
  func_instance *start_func ;
  process *proc;
  Address pending_tramp_addr;	// address of pending instrumentation
  Address indirect_start_func;
  // currently used on NT only  
};

#endif
