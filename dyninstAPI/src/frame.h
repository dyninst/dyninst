/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <iosfwd>
#include <vector>

#include "instPoint.h"
#include "baseTramp.h"
#include "function.h"

#include "stackwalk/h/frame.h"

class PCThread;
class PCProcess;

class Frame {
 public:

  // default ctor (zero frame)
  Frame();

  // Real constructor -- fill-in.
  Frame(const Dyninst::Stackwalker::Frame &swf,
	PCProcess *proc, 
	PCThread *thread,
	bool uppermost);

 Frame(const Frame &f) :
      sw_frame_(f.sw_frame_),
      proc_(f.proc_),
      thread_(f.thread_),
      uppermost_(f.uppermost_) {}

  const Frame &operator=(const Frame &f) {
      sw_frame_ = f.sw_frame_;
      proc_ = f.proc_;
      thread_ = f.thread_;
      uppermost_ = f.uppermost_;
      return *this;
  }
  
  bool operator==(const Frame &F) const {
    return ((uppermost_ == F.uppermost_) &&
	    (sw_frame_ == F.sw_frame_) &&
	    (proc_    == F.proc_) &&
	    (thread_  == F.thread_));
  }

  Address  getPC() const { return (Address) sw_frame_.getRA(); }
  // New method: unwind instrumentation
  Address  getUninstAddr() const;
  Address  getFP() const { return (Address) sw_frame_.getFP(); }
  Address  getSP() const { return (Address) sw_frame_.getSP(); }
  PCProcess *getProc() const { return proc_; }
  PCThread *getThread() const { return thread_; }
  void setThread(PCThread *thrd) { thread_ = thrd; }
  bool     isUppermost() const { return uppermost_; }


  instPoint *getPoint();
  baseTramp *getBaseTramp();
  func_instance *getFunc();

  bool	   isSignalFrame();
  bool 	   isInstrumentation();
  Address  getPClocation();

  friend std::ostream & operator << ( std::ostream & s, Frame & m );
  bool setPC(Address newpc);

#if defined(arch_power)
  // We store the actual return addr in a word on the stack
  bool setRealReturnAddr(Address retaddr);
#endif

 private:
  Dyninst::Stackwalker::Frame sw_frame_;        // StackwalkerAPI frame
  PCProcess *		proc_;				// We're only valid for a single process anyway
  PCThread *            thread_;                // User-level thread
  bool			uppermost_;
};

class int_stackwalk {
   bool isValid_;
   std::vector<Frame> stackwalk_;
 public:
   int_stackwalk();
   bool isValid();
   bool setStackwalk(std::vector<Frame> &new_stack);
   bool clear();
   std::vector<Frame>& getStackwalk();
};

#endif
