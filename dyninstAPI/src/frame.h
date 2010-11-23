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

// $Id: frame.h,v 1.31 2006/11/28 23:34:04 legendre Exp $

#ifndef FRAME_H
#define FRAME_H

#include "common/h/std_namesp.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"

class PCThread;
class PCProcess;
class codeRange;
class instPoint;
class miniTramp;
class int_function;

typedef enum { FRAME_unset, FRAME_instrumentation, FRAME_signalhandler, FRAME_normal, FRAME_syscall, FRAME_iRPC, FRAME_unknown } frameType_t;

class Frame {
 public:
  
  // default ctor (zero frame)
  Frame();

  // Real constructor -- fill-in.
  // Option 2 would be to have the constructor look up this info,
  // but getCallerFrame works as is.
  Frame(Address pc, Address fp, Address sp,
	unsigned pid, PCProcess *proc, 
	PCThread *thread,
	bool uppermost,
	Address pcAddr = 0 );

  // getCallerFrame constructor. Choose what we want from the
  // callee frame, set pc/fp/sp/pcAddr manually
  Frame(Address pc, Address fp, 
	Address sp, Address pcAddr,
	Frame *calleeFrame);

 Frame(const Frame &f) :
  frameType_(f.frameType_),
  uppermost_(f.uppermost_),
      pc_(f.pc_),
      fp_(f.fp_),
      sp_(f.sp_),
      pid_(f.pid_),
      proc_(f.proc_),
      thread_(f.thread_),
      range_(f.range_),
      pcAddr_(f.pcAddr_) {};

  const Frame &operator=(const Frame &f) {
      frameType_ = f.frameType_;
      uppermost_ = f.uppermost_;
      pc_ = f.pc_;
      fp_ = f.fp_;
      sp_ = f.sp_;
      pid_ = f.pid_;
      proc_ = f.proc_;
      thread_ = f.thread_;
      range_ = f.range_;
      pcAddr_ = f.pcAddr_;
      return *this;
  }
  
  bool operator==(const Frame &F) {
    return ((uppermost_ == F.uppermost_) &&
	    (pc_      == F.pc_) &&
	    (fp_      == F.fp_) &&
	    (sp_      == F.sp_) &&	    
	    (pid_     == F.pid_) &&
	    (proc_    == F.proc_) &&
	    (thread_  == F.thread_));
  }

  Address  getPC() const { return pc_; }
  // New method: unwind instrumentation
  Address  getUninstAddr(); // calls getRange so can't be const
  Address  getFP() const { return fp_; }
  Address  getSP() const { return sp_; }
  unsigned getPID() const { return pid_; }
  PCProcess *getProc() const { return proc_; }
  PCThread *getThread() const { return thread_; }
  void setThread(PCThread *thrd) { thread_ = thrd; }
  bool     isUppermost() const { return uppermost_; }
  bool	   isSignalFrame();
  bool 	   isInstrumentation();
  bool     isSyscall();
  Address  getPClocation() { return pcAddr_; }

  instPoint *getPoint(); // If we're in instrumentation returns the appropriate point
  int_function *getFunc(); // As above

  codeRange *getRange();
  void setRange (codeRange *range); 
  friend std::ostream & operator << ( std::ostream & s, Frame & m );

  bool setPC(Address newpc);

#if defined(arch_power)
  // We store the actual return addr in a word on the stack
  bool setRealReturnAddr(Address retaddr);
#endif

  // check for zero frame
  bool isLastFrame() const;
  
  // get stack frame of caller
  // May need the process image for various reasons
  Frame getCallerFrame();
  frameType_t frameType_;

  // Set the frameType_ member
  void calcFrameType();
  
 private:
  bool			uppermost_;
  Address		pc_;
  Address		fp_;
  Address		sp_;				// NOTE: this is not always populated
  int			pid_;				// Process id 
  PCProcess *		proc_;				// We're only valid for a single process anyway
  PCThread *            thread_;                // User-level thread
  codeRange *	range_;				// If we've done a by-address lookup, keep it here

  Address		pcAddr_;
  
};

class int_stackwalk {
   bool isValid_;
   pdvector<Frame> stackwalk_;
 public:
   int_stackwalk();
   bool isValid();
   bool setStackwalk(pdvector<Frame> &new_stack);
   bool clear();
   pdvector<Frame>& getStackwalk();
};

#endif
