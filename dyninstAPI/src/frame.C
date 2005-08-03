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

// $Id: frame.C,v 1.12 2005/08/03 23:00:53 bernat Exp $

#include <stdio.h>
#include <iostream>
#include "frame.h"
#include "process.h"
#include "dyn_thread.h"
#include "dyn_lwp.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "miniTramp.h"


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
  pcAddr_(0) {
    stackwalk_cerr << "*** Null frame ***" << endl;
}


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
  stackwalk_cerr << "Base frame:   " << (*this) << endl;
};

Frame::Frame(Address pc, Address fp, Address sp,
	     Address pcAddr, Frame *f) : 
    frameType_(FRAME_unset),
  uppermost_(false),
  pc_(pc), fp_(fp), 
  sp_(sp),
  pid_(f->pid_), proc_(f->proc_),
  thread_(f->thread_), lwp_(f->lwp_),
  range_(0), hasValidCursor(false), pcAddr_(pcAddr) {
  calcFrameType();
  stackwalk_cerr << "Called frame: " << (*this) << endl;
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
    // Can be called multiple times...
    if (frameType_ != FRAME_unset) {
        return;
    }

   int_function *func;
   miniTrampInstance *mini;
   multiTramp *multi;

   // Without a process pointer, we're not going to get far.
   if (!getProc())
       return;

   // Checking for a signal handler must go before the vsyscall check
   // since (on Linux) the signal handler is _inside_ the vsyscall page.
   if (getProc()->isInSignalHandler(pc_)) {
     frameType_ = FRAME_signalhandler;
     return;
   }

   // Better to have one function that has platform-specific IFDEFs
   // than a stack of 90% equivalent functions
#if defined(os_linux) && defined(arch_x86)
   calcVSyscallFrame(getProc());
   if (pc_ >= getProc()->getVsyscallStart() && pc_ < getProc()->getVsyscallEnd()) {
     frameType_ = FRAME_syscall;
     return;
   }
#endif   
   
   codeRange *range = getRange();

   func = range->is_function();
   multi = range->is_multitramp();
   mini = range->is_minitramp();

   if (mini != NULL) {
       frameType_ = FRAME_instrumentation;
       return;
   }
   else if (multi != NULL) {
       frameType_ = FRAME_instrumentation;
       return;
   }
   else if (func != NULL) {
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

// Get the instPoint corresponding with this frame
instPoint *Frame::getPoint() {
    // Easy check:
    if (getPC() == getUninstAddr())
        return NULL;

    codeRange *range = getRange();
    
    multiTramp *m_ptr = range->is_multitramp();
    miniTrampInstance *mt_ptr = range->is_minitramp();
    baseTrampInstance *bt_ptr = range->is_basetramp_multi();

    if (mt_ptr) {
        return mt_ptr->mini->instP;
    }
    else if (m_ptr) {
        // We're in a multiTramp, so between instPoints. 
        // However, we're in a multiTramp and not the original code, so 
        // we do need to discover an instpoint. We're not in a baseTramp,
        // so that's not a problem; other options are relocated instruction
        // or trampEnd.
        return m_ptr->findInstPointByAddr(getPC());
    }
    return NULL;
}
            
        
        
    

Address Frame::getUninstAddr() {
    codeRange *range = getRange();
    int_function *f_ptr = range->is_function();
    multiTramp *m_ptr = range->is_multitramp();
    miniTrampInstance *mt_ptr = range->is_minitramp();
    baseTrampInstance *bt_ptr = range->is_basetramp_multi();

    if (f_ptr)
        return getPC();
    else if (m_ptr) {
        // Figure out where in the multiTramp we are
        return m_ptr->instToUninstAddr(getPC());
    }
    else if (mt_ptr) {
        // Don't need the actual PC for minitramps
        return mt_ptr->uninstrumentedAddr();
    }
    else if (bt_ptr) {
        // Don't need actual PC here either
        return bt_ptr->uninstrumentedAddr();
    }
    else {
        // Where are we?
        return getPC();
    }
}


ostream & operator << ( ostream & s, Frame & f ) {
	codeRange * range = f.getRange();
	int_function * func_ptr = range->is_function();
        multiTramp *multi_ptr = range->is_multitramp();
	miniTrampInstance * minitramp_ptr = range->is_minitramp();

	s << "PC: 0x" << std::hex << f.getPC() << " ";
	switch (f.frameType_) {
	case FRAME_unset:
	  s << "[UNSET FRAME TYPE]";
	  break;
	case FRAME_instrumentation:
	  s << "[Instrumentation:";
	  if (minitramp_ptr) {
	    s << "mt from "
	      << minitramp_ptr->baseTI->multiT->func()->prettyName();
	  }
	  else if (multi_ptr) {
              baseTrampInstance *bti = multi_ptr->getBaseTrampInstanceByAddr(f.getPC());
              if (bti) {
                  s << "bt from ";
              }
              else {
                  s << "multitramp from ";
              }
              
              s << multi_ptr->func()->prettyName();
	  }

          // And the address
          s << std::hex << "/0x" << f.getUninstAddr();
          s << "]" << std::dec;
          
	  break;
	case FRAME_signalhandler:
	  s << "[SIGNAL HANDLER]";
	  break;
	case FRAME_normal:
	  if( func_ptr ) {
	    s << "[" << func_ptr->prettyName() << "]";
	  }
	  break;
	case FRAME_syscall:
	  s << "[SYSCALL]";
	  break;
	case FRAME_unknown:
	  s << "[UNKNOWN]";
	  break;
        default:
            s << "[ERROR!]";
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

