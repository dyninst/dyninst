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

// $Id: frame.C,v 1.24 2008/06/19 19:53:15 legendre Exp $

#include <stdio.h>
#include <iostream>
#include "frame.h"
#include "dynProcess.h"
#include "dynThread.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "debug.h"

#include "stackwalk/h/framestepper.h"

Frame::Frame() : 
  sw_frame_(Dyninst::Stackwalker::Frame()),
  proc_(NULL),
  thread_(NULL),
  uppermost_(false) {}

Frame::Frame(const Dyninst::Stackwalker::Frame &swf,
	     PCProcess *proc,
	     PCThread *thread,
	     bool uppermost) :

  sw_frame_(swf),
  proc_(proc),
  thread_(thread),
  uppermost_(uppermost) {}

// Get the instPoint corresponding with this frame
instPoint *Frame::getPoint() {
  baseTramp *bt = getBaseTramp();
  if (!bt) return NULL;
  return bt->instP();
}

baseTramp *Frame::getBaseTramp() {
   AddressSpace::RelocInfo ri;
  if (getProc()->getRelocInfo(getPC(),
                              ri)) {
     if (ri.bt) return ri.bt;
  }
  return NULL;
}  

func_instance *Frame::getFunc() {
  return getProc()->findOneFuncByAddr(getUninstAddr());
}

Address Frame::getUninstAddr() const {
  AddressSpace::RelocInfo ri;
  Address pc = getPC();
  if (getProc()->getRelocInfo(pc,
                              ri)) {
     return ri.orig;
  }
  return getPC();
}

ostream & operator << ( ostream & s, Frame & f ) {
  func_instance *func = NULL;

  s << "PC: 0x" << std::hex << f.getPC() << " ";

  if (f.isInstrumentation())
  {
    s << "[Instrumentation:";
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
    func = f.getFunc();
    if (func)
       s << func->name();
    else
       s << "[UNKNOWN FUNCTION]";
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

bool int_stackwalk::setStackwalk(std::vector<Frame> &new_stack) {
   stackwalk_ = new_stack;
   isValid_ = true;
   return true;
}

bool int_stackwalk::clear() { 
   isValid_ = false; 
   return true;
}

std::vector<Frame>& int_stackwalk::getStackwalk() {
   assert(isValid_);
   return stackwalk_;
}

