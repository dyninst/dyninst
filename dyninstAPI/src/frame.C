/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: frame.C,v 1.24 2008/06/19 19:53:15 legendre Exp $

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
  frameType_(unset), 
  uppermost_(false), 
  pc_(0), 
  fp_(0), 
  sp_(0),
  pid_(0), 
  proc_(NULL), 
  thread_(NULL), 
  lwp_(NULL), 
  pcAddr_(0) {
    stackwalk_cerr << "*** Null frame ***" << endl;
}


Frame::Frame(Address pc, Address fp, Address sp,
	     unsigned pid, process *proc, 
	     dyn_thread *thread, dyn_lwp *lwp, 
	     bool uppermost,
	     Address pcAddr ) :
  frameType_(unset),
  uppermost_(uppermost),
  pc_(pc), fp_(fp), sp_(sp),
  pid_(pid), proc_(proc), thread_(thread), lwp_(lwp), 
  pcAddr_(pcAddr) {
  stackwalk_cerr << "Base frame:   " << (*this) << endl;
};

Frame::Frame(Address pc, Address fp, Address sp,
	     Address pcAddr, Frame *f) : 
  frameType_(unset),
  uppermost_(false),
  pc_(pc), fp_(fp), 
  sp_(sp),
  pid_(f->pid_), proc_(f->proc_),
  thread_(f->thread_), lwp_(f->lwp_),
  pcAddr_(pcAddr) {
  stackwalk_cerr << "Called frame: " << (*this) << endl;
}

bool Frame::isLastFrame() const
{
#if !defined(arch_x86) && !defined(arch_x86_64)
   if (fp_ == 0) return true;
#endif
   if (pc_ == 0) return true;
   return false;
}

#if defined(os_linux) && defined(arch_x86)
extern void calcVSyscallFrame(process *p);
#endif

void Frame::calcFrameType()
{
  // Can be called multiple times...
  if (frameType_ != unset) {
    return;
  }
  
  // Without a process pointer, we're not going to get far.
  if (!getProc()) {
    cerr << "\t no process" << endl;
    return;
  }
  
  // Checking for a signal handler must go before the vsyscall check
  // since (on Linux) the signal handler is _inside_ the vsyscall page.
  if (getProc()->isInSignalHandler(pc_)) {
    frameType_ = signalhandler;
    return;
  }
  
  // Better to have one function that has platform-specific IFDEFs
  // than a stack of 90% equivalent functions
#if defined(os_linux) && defined(arch_x86)
  calcVSyscallFrame(getProc());
  if ((pc_ >= getProc()->getVsyscallStart() && pc_ < getProc()->getVsyscallEnd()) || /* Hack for RH9 */ (pc_ >= 0xffffe000 && pc_ < 0xfffff000)) {
    frameType_ = syscall;
    return;
  }
#endif   
  
  if (getProc()->findObject(pc_)) {
    // We're in original code
    frameType_ = normal;
    return;
  }
 
  // Check for instrumentation

  baseTramp *bti;
  Address origPC = 42;
  int_block *tmp;
  if (getProc()->getRelocInfo(pc_,
			                  origPC,
                              tmp,
			                  bti)) {
    if (bti) {
      frameType_ = instrumentation;
    }
    else {
      frameType_ = normal;
    }
    return;
  }

  frameType_ = unset;
  return;
}

// Get the instPoint corresponding with this frame
instPoint *Frame::getPoint() {
  baseTramp *bt = getBaseTramp();
  if (!bt) return NULL;
  return bt->instP();
}

baseTramp *Frame::getBaseTramp() {
  baseTramp *bti = NULL;
  Address origPC;
  int_block *tmp;
  if (getProc()->getRelocInfo(pc_,
                              origPC,
                              tmp,
                              bti)) {
     if (bti) {
        return bti;
     }
  }
  return NULL;
}  

int_function *Frame::getFunc() {
  return getProc()->findOneFuncByAddr(getUninstAddr());
}

Address Frame::getUninstAddr() {
  
  baseTramp *bti;
  Address origPC;
  int_block *tmp;
  if (getProc()->getRelocInfo(pc_,
                              origPC,
                              tmp,
                              bti)) {
    return origPC;
  }
  return pc_;
}


ostream & operator << ( ostream & s, Frame & f ) {
  f.calcFrameType();
  
  s << "PC: 0x" << std::hex << f.getPC() << " ";
  switch (f.frameType_) {
  case Frame::unset:
    s << "[UNSET FRAME TYPE]";
    break;
  case Frame::instrumentation:
    s << "[Instrumentation:";

    // And the address
    s << std::hex << "/0x" << f.getUninstAddr();
    s << "]" << std::dec;
    
    break;
  case Frame::signalhandler:
    s << "[SIGNAL HANDLER]";
    break;
  case Frame::normal:
    break;
  case Frame::syscall:
    s << "[SYSCALL]";
    break;
  case Frame::iRPC:
    s << "[iRPC]";
    break;
  case Frame::unknown:
    s << "[UNKNOWN]";
    break;
  default:
    s << "[ERROR!]";
    break;
  }
  s << " FP: 0x" << std::hex << f.getFP() << " SP: 0x" << f.getSP() << " PID: " << std::dec << f.getPID() << " "; 
  if( f.getThread() ) {
    s << "TID: " << f.getThread()->get_tid() << " ";
  }
  if( f.getLWP() ) {
    s << "LWP: " << f.getLWP()->get_lwp_id() << " ";
  }
  
  return s;
}

bool Frame::isSignalFrame()
{ 
    calcFrameType();
    return frameType_ == signalhandler;
}

bool Frame::isInstrumentation()
{ 
    calcFrameType();
    return frameType_ == instrumentation;
}

bool Frame::isSyscall()
{ 
    calcFrameType();
    return frameType_ == syscall;
}

int_stackwalk::int_stackwalk() { 
   isValid_ = false; 
}

bool int_stackwalk::isValid() { 
   return isValid_; 
}

bool int_stackwalk::setStackwalk(pdvector<Frame> &new_stack) {
   stackwalk_ = new_stack;
   isValid_ = true;
   return true;
}

bool int_stackwalk::clear() { 
   isValid_ = false; 
   return true;
}

pdvector<Frame>& int_stackwalk::getStackwalk() {
   assert(isValid_);
   return stackwalk_;
}
