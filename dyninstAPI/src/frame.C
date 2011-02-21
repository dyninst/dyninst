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
#include "pcProcess.h"
#include "pcThread.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "miniTramp.h"
#include "debug.h"

#include "stackwalk/h/framestepper.h"

Frame::Frame() : 
  sw_frame_(Dyninst::Stackwalker::Frame()),
  proc_(NULL),
  thread_(NULL),
  range_(0),
  uppermost_(false) {}

Frame::Frame(const Dyninst::Stackwalker::Frame &swf,
	     PCProcess *proc,
	     PCThread *thread,
	     bool uppermost) :

  sw_frame_(swf),
  proc_(proc),
  thread_(thread),
  range_(0),
  uppermost_(uppermost) {}

codeRange *Frame::getRange() {
  if (!range_) {
    // First time... so get it and cache
    if (!getProc())
      return NULL;
    range_ = getProc()->findOrigByAddr(getPC());
  }
  return range_;
}

void Frame::setRange(codeRange *range) {
  assert(!range_ ||
	 (range == range_));
  range_ = range;
}

// Get the instPoint corresponding with this frame
instPoint *Frame::getPoint() {
    // not detecting instrumentation properly
    // TODO Should be fixed with Kevin/Drew merge
    //if (getPC() == getUninstAddr()) {
    //    return NULL;
    //}

    codeRange *range = getRange();
    
    multiTramp *m_ptr = range->is_multitramp();
    miniTrampInstance *mt_ptr = range->is_minitramp();

    if (mt_ptr) {
        return mt_ptr->mini->instP();
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
            
int_function *Frame::getFunc() {
    codeRange *range = getRange();
    if (range->is_function())
        return range->is_function();
    else if (range->is_multitramp())
        return range->is_multitramp()->func();
    else if (range->is_minitramp())
        return range->is_minitramp()->baseTI->multiT->func();
    else if (BPatch_defensiveMode == getProc()->getHybridMode() && 
             range->is_mapped_object()) {
        // in defensive mode, return the function at getPC-1, since
        // the PC could be at the fallthrough address of a call
        // instruction that was assumed to be non-returning
        range = getProc()->findModByAddr(getPC()-1);
        if (range == NULL) 
            return NULL;
        if (range->is_function())
            return range->is_function();
        else if (range->is_multitramp())
            return range->is_multitramp()->func();
        else if (range->is_minitramp())
            return range->is_minitramp()->baseTI->multiT->func();
    }

    return NULL;
}

Address Frame::getUninstAddr() {
    codeRange *range = getRange();
    multiTramp *m_ptr = range->is_multitramp();
    miniTrampInstance *mt_ptr = range->is_minitramp();
    baseTrampInstance *bt_ptr = range->is_basetramp_multi();
    bblInstance *bbl_ptr = range->is_basicBlockInstance();
    Address uninst =0;

    if (m_ptr) {
        // Figure out where in the multiTramp we are
        uninst = m_ptr->instToUninstAddr(getPC());
    }
    else if (mt_ptr) {
        // Don't need the actual PC for minitramps
        uninst = mt_ptr->uninstrumentedAddr();
    }
    else if (bt_ptr) {
        // Don't need actual PC here either
        uninst = bt_ptr->uninstrumentedAddr();
    }

    if (0 != uninst) {
        range = proc_->findOrigByAddr(uninst);
        if (!range || mt_ptr || bt_ptr) {
            return uninst;
        }
        bbl_ptr = range->is_basicBlockInstance();
        if (!bbl_ptr) {
            return uninst;
        }
    }

    if (bbl_ptr) {
        // Relocated function... back-track
        assert(range->is_basicBlock());
        return bbl_ptr->equivAddr(0, getPC());
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

  if (f.isInstrumentation())
  {
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
  }
  else if (f.isSignalFrame())
  {
    s << "[SIGNAL HANDLER]";
  }
  else 
  {
    if( func_ptr ) {
      s << "[" << func_ptr->prettyName() << "]";
    }
  }

  s << " FP: 0x" << std::hex << f.getFP() << " SP: 0x" << f.getSP() << " PID: " << std::dec << f.getProc()->getPid() << " "; 
  if( f.getThread() ) {
    s << "TID: " << f.getThread()->getTid() << " ";
    s << "LWP: " << f.getThread()->getLWP() << " ";
  }

  return s;
}

bool Frame::isSignalFrame()
{ 
  if (sw_frame_.getNextStepper() &&
      dynamic_cast<Dyninst::Stackwalker::SigHandlerStepper*>(sw_frame_.getNextStepper()))
    return true;
  else
    return false;
}

bool Frame::isInstrumentation()
{ 
  if (sw_frame_.getNextStepper() &&
      dynamic_cast<Dyninst::Stackwalker::DyninstDynamicStepper*>(sw_frame_.getNextStepper()))
    return true;
  else
    return false;
}

Address Frame::getPClocation()
{
  Dyninst::Stackwalker::location_t pcLoc = sw_frame_.getRALocation();
    if (pcLoc.location != Dyninst::Stackwalker::loc_address)
    {
      return 0;
    }
    else
    {
      return pcLoc.val.addr;
    }
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

