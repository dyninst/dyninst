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

#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/process.h"


dyn_thread::dyn_thread(process *pproc) : 
   tid(0), 
   index(0), 
   stack_addr(0),
   start_pc(0), 
   start_func(NULL),
   pending_tramp_addr(ADDR_NULL),
   indirect_start_func(0)
{ 
   assert(pproc);
   proc = pproc; 
   lwp  = pproc->getRepresentativeLWP();
   proc->addThread(this);
}

dyn_thread::dyn_thread(process *proc_, unsigned index_, dyn_lwp *lwp_) :
    tid(0),
    index(index_),
    lwp(lwp_),
    stack_addr(0),
    start_pc(0),
    start_func(NULL),
    pending_tramp_addr( ADDR_NULL ),
    indirect_start_func(0)
{
   assert(proc_);
   proc = proc_;
   proc->addThread(this);
}

dyn_thread::dyn_thread(dyn_thread *src, process *child, dyn_lwp *lwp_)
{
   assert(src && child);
   tid = src->tid;
   index = src->index;
   if(lwp_ == NULL)
      lwp = child->getRepresentativeLWP();
   else
      lwp = lwp_;
   stack_addr = src->stack_addr;
   start_pc = src->start_pc;
   resumestate_p = src->resumestate_p;
   start_func = src->start_func;
   indirect_start_func = src->indirect_start_func;
   proc = child;
   pending_tramp_addr = ADDR_NULL;
   proc->addThread(this);
}

// This will need to be filled in if we (ever) handle migrating threads
bool dyn_thread::updateLWP()
{
  return true;
}

  
// MT version lives in the <os>MT.C files, and can do things like
// get info for threads not currently scheduled on an LWP
Frame dyn_thread::getActiveFrame()
{
   updateLWP();

   if(! get_proc()->multithread_capable(true)) {
      Frame lwpFrame = lwp->getActiveFrame();  
      lwpFrame.thread_ = this;
      return lwpFrame;
   } else
      return getActiveFrameMT();
}

// stackWalk: return parameter.
bool dyn_thread::walkStack(pdvector<Frame> &stackWalk)
{
   bool continueWhenDone = false;
   if (get_lwp()->status() == running) {
       continueWhenDone = true;
       get_lwp()->pauseLWP(true);
   }

   if (!process::IndependentLwpControl()) {
     if (proc->status() == running) {
       continueWhenDone = true;
       proc->pause();
     }
   }

   if (get_lwp()->cached_stackwalk.isValid()) {
      stackWalk = get_lwp()->cached_stackwalk.getStackwalk();
      for (unsigned i=0; i<stackWalk.size(); i++) {
         stackWalk[i].thread_ = this;
      }
      return true;
   }

   stackwalk_printf("%s[%d]: beginning stack walk on thread %ld\n",
		    FILE__, __LINE__, get_tid());
   

   Frame active = getActiveFrame();
   active.thread_ = this;
   bool retval = proc->walkStackFromFrame(active, stackWalk);

   // if the stackwalk was successful, cache it
   if (retval) {
      get_lwp()->cached_stackwalk.setStackwalk(stackWalk);
   }

   stackwalk_printf("%s[%d]: ending stack walk on thread %ld\n",
		    FILE__, __LINE__, get_tid());
 

   if (continueWhenDone) {
      get_lwp()->continueLWP();
   }
   
   return retval;
}

dyn_lwp *dyn_thread::get_lwp()
{
  if (proc->multithread_ready(true))
    updateLWP();
   return lwp;
}

dyn_thread::~dyn_thread() 
{
}
  
void dyn_thread::update_index(unsigned index_)
{
    index = index_;
}

void dyn_thread::update_stack_addr(Address stack_addr_) 
{
   stack_addr=stack_addr_; 
}

Address dyn_thread::get_stack_addr() const 
{ 
   return(stack_addr); 
}
