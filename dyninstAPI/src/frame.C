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

// $Id: frame.C,v 1.7 2005/03/04 21:12:15 bernat Exp $

#include <stdio.h>
#include <iostream>
#include "process.h"
#include "dyn_thread.h"
#include "dyn_lwp.h"
#include "function.h"
#include "func-reloc.h"
#include "instPoint.h"
#include "trampTemplate.h"
#include "miniTrampHandle.h"
#include "edgeTrampTemplate.h"
#include "frame.h"


Frame::Frame() : 
  frameType_(FRAME_unset), 
  uppermost_(false), 
  pc_(0), 
  fp_(0), 
  sp_(0),
  pid_(0), 
  proc_(NULL), 
  thread_(NULL), 
  lwp_(NULL), 
  range_(0), 
  hasValidCursor(false),
  pcAddr_(0)
  {}


Frame::Frame(Address pc, Address fp, Address sp,
	     unsigned pid, process *proc, 
	     dyn_thread *thread, dyn_lwp *lwp, 
	     bool uppermost,
	     Address pcAddr ) :
  frameType_(FRAME_unset),
  uppermost_(uppermost),
  pc_(pc), fp_(fp), sp_(sp),
  pid_(pid), proc_(proc), thread_(thread), lwp_(lwp), 
  range_(0), hasValidCursor(false), pcAddr_(pcAddr) {
  calcFrameType();
};

Frame::Frame(Address pc, Address fp, Address sp,
	     Address pcAddr, Frame *f) : 
  uppermost_(false),
  pc_(pc), fp_(fp), 
  sp_(sp),
  pid_(f->pid_), proc_(f->proc_),
  thread_(f->thread_), lwp_(f->lwp_),
  range_(0), hasValidCursor(false), pcAddr_(pcAddr) {
  calcFrameType();
}

codeRange *Frame::getRange() {
  if (!range_) {
    // First time... so get it and cache
    if (!getProc())
      return NULL;
    range_ = getProc()->findCodeRangeByAddress(getPC());
  }
  return range_;
}

void Frame::setRange(codeRange *range) {
  assert(!range_ ||
	 (range == range_));
  range_ = range;
}

bool Frame::isLastFrame() const
{
   if (fp_ == 0) return true;
   if (pc_ == 0) return true;
   return false;
}

#if defined(os_linux) && defined(arch_x86)
extern void calcVSyscallFrame(process *p);
#endif

void Frame::calcFrameType()
{
   int_function *func;
   relocatedFuncInfo *reloc;
   miniTrampHandle *mini;
   trampTemplate *base;
   edgeTrampTemplate *edge;

   // Without a process pointer, we're not going to get far.
   assert(getProc());

   // Better to have one function that has platform-specific IFDEFs
   // than a stack of 90% equivalent functions
#if defined(os_linux) && defined(arch_x86)
   calcVSyscallFrame(getProc());
   if (pc_ >= getProc()->getVsyscallStart() && pc_ < getProc()->getVsyscallEnd()) {
     frameType_ = FRAME_syscall;
     return;
   }
#endif
   
   if (getProc()->isInSignalHandler(pc_)) {
     frameType_ = FRAME_signalhandler;
     return;
   }
   
   codeRange *range = getRange();

   func = range->is_function();
   base = range->is_basetramp();
   mini = range->is_minitramp();
   reloc = range->is_relocated_func();
   edge = range->is_edge_tramp(); 

   if (base != NULL || mini != NULL) {
     frameType_ = FRAME_instrumentation;
     return;
   }
   else if (reloc != NULL || func != NULL || edge != NULL) {
     frameType_ = FRAME_normal;
     return;
   }
   else {
     frameType_ = FRAME_unknown;
     return;
   }
   assert(0 && "Unreachable");
   frameType_ = FRAME_unset;
   return;
}

ostream & operator << ( ostream & s, Frame & f ) {
	codeRange * range = f.getRange();
	int_function * func_ptr = range->is_function();
	trampTemplate * basetramp_ptr = range->is_basetramp();
	miniTrampHandle * minitramp_ptr = range->is_minitramp();
	relocatedFuncInfo * reloc_ptr = range->is_relocated_func();
	multitrampTemplate * multitramp_ptr = range->is_multitramp();
	edgeTrampTemplate *edgetramp_ptr = range->is_edge_tramp();
	s << "PC: 0x" << std::hex << f.getPC() << " ";
	switch (f.frameType_) {
	case FRAME_unset:
	  s << "[UNSET FRAME TYPE]";
	  break;
	case FRAME_instrumentation:
	  s << "[Instrumentation:";
	  if (minitramp_ptr) {
	    s << "mt from "
	      << minitramp_ptr->baseTramp->location->pointFunc()->prettyName() 
	      << "]";
	  }
	  else if (basetramp_ptr) {
	    s << "bt from "
	      << basetramp_ptr->location->pointFunc()->prettyName() 
	      << "]";
	  }
	  if( multitramp_ptr ) {
	    s << "[mt from " 
	      << multitramp_ptr->location->pointFunc()->prettyName() << "]";
	  }
	  break;
	case FRAME_signalhandler:
	  s << "[SIGNAL HANDLER]";
	  break;
	case FRAME_normal:
	  if( func_ptr ) {
	    s << "[" << func_ptr->prettyName() << "]";
	  }
	  if( reloc_ptr ) {
	    s << "[" << reloc_ptr->func()->prettyName() << " *RELOCATED*]";
	  }
	  break;
	case FRAME_syscall:
	  s << "[SYSCALL]";
	  break;
	case FRAME_unknown:
	  s << "[UNKNOWN]";
	  break;
	}
	s << " FP: 0x" << std::hex << f.getFP() << " PID: " << std::dec << f.getPID() << " "; 
	if( f.getThread() ) {
   		s << "TID: " << f.getThread()->get_tid() << " ";
   		}
   	if( f.getLWP() ) {
   		s << "LWP: " << f.getLWP()->get_lwp_id() << " ";
   		}
	
	return s;
	}

