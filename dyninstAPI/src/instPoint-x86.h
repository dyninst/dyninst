/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: instPoint-x86.h,v 1.14 2001/08/20 19:59:09 bernat Exp $

#ifndef _INST_POINT_X86_H_
#define _INST_POINT_X86_H_

#include "dyninstAPI/src/symtab.h"

class process;

/************************************************************************
 *
 *  class instPoint: representation of instrumentation points
 *
 ***********************************************************************/

class instPoint {

 public:

  instPoint(pd_Function *f, const image *, Address adr, instruction inst) {
    addr_   = adr;
    func_   = f;
    callee_ = NULL;

    jumpAddr_     = 0;
    insnAtPoint_  = inst;
    insnBeforePt_ = 0;
    insnAfterPt_  = 0;
    bonusBytes_   = 0;
    relocated_ = false;    
  };

  ~instPoint() {
    if (insnBeforePt_) delete insnBeforePt_;
    if (insnAfterPt_) delete insnAfterPt_;
  };


  const function_base *iPgetFunction() const { return func();    }
  const function_base *iPgetCallee()   const { return callee();  }
  const image         *iPgetOwner()    const { return owner();   }
        Address        iPgetAddress()  const { return address(); }


  Address address() const { return addr_; }
  pd_Function *func() const { return func_; }

  const instruction &insnAtPoint() const { return insnAtPoint_; }

  // add an instruction before the point. Instructions should be added in reverse
  // order, the instruction closest to the point first.
  void addInstrBeforePt(instruction inst) {
    if (!insnBeforePt_) insnBeforePt_ = new vector<instruction>;
    (*insnBeforePt_).push_back(inst);
  };

  // add an instruction after the point.
  void addInstrAfterPt(instruction inst) {
    if (!insnAfterPt_) insnAfterPt_ = new vector<instruction>;
    (*insnAfterPt_).push_back(inst);
  }

  const instruction &insnBeforePt(unsigned index) const 
    { assert(insnBeforePt_); return ((*insnBeforePt_)[index]); }

  const instruction &insnAfterPt(unsigned index) const 
    { assert(insnAfterPt_); return ((*insnAfterPt_)[index]); }

  unsigned insnsBefore() const
    { if (insnBeforePt_) return (*insnBeforePt_).size(); return 0; }

  unsigned insnsAfter() const 
    { if (insnAfterPt_) return (*insnAfterPt_).size(); return 0; }

  Address jumpAddr() const { assert(jumpAddr_); return jumpAddr_; }
  
  void setJumpAddr(Address jumpAddr) { jumpAddr_ = jumpAddr; }

  Address returnAddr() const {
    Address ret = addr_ + insnAtPoint_.size();
    for (unsigned u = 0; u < insnsAfter(); u++)
      ret += (*insnAfterPt_)[u].size();
    return ret;
  }

  bool getRelocated() { return relocated_; }
  void setRelocated() {relocated_ = true;}

  image *owner() const { return func()->file()->exec(); }

  // return the number of instructions in this point
  // that will be overwritten when a returnInstance is installed
  unsigned insns() const {
    unsigned size = 1;

    if (insnBeforePt_)
      size += (*insnBeforePt_).size();
    if (insnAfterPt_)
      size += (*insnAfterPt_).size();

    return size;
  }
  
  // return the size of all instructions in this point
  // size may change after point is checked
  unsigned size() const {
    unsigned tSize = insnAtPoint_.size();
    for (unsigned u1 = 0; u1 < insnsBefore(); u1++)
      tSize += (*insnBeforePt_)[u1].size();
    for (unsigned u2 = 0; u2 < insnsAfter(); u2++)
      tSize += (*insnAfterPt_)[u2].size();
    tSize += bonusbytes();
    return tSize;
  }

    // return the number of bytes that need to be added so that the
  // instruction will be instrumentable after the function has been relocated
  int extraBytes() const {
    assert (jumpAddr_ <= addr_);

    int tSize = (addr_ - jumpAddr_) + insnAtPoint_.size();

    for (unsigned u2 = 0; u2 < insnsAfter(); u2++)
      tSize += (*insnAfterPt_)[u2].size();
    if (tSize < 5) 
      return 5 - tSize;
    else 
      return 0;
  }

  void setBonusBytes(unsigned num) {
       bonusBytes_ = num;
  }

  unsigned bonusbytes() const {
       return bonusBytes_;
  }

  // check for jumps to instructions before and/or after this point, and discard
  // instructions when there is a jump.
  // Can only be done after an image has been parsed (that is, all inst. points
  // in the image have been found.
  void checkInstructions ();

  pd_Function *callee() const { 
    if (insnAtPoint().isCall()) {
      if (insnAtPoint().isCallIndir())
	return NULL;
      else {
	Address addr = insnAtPoint().getTarget(address());
	pd_Function *pdf = owner()->findFuncByAddr(addr);
	return pdf;
      }
    }
    return NULL;
  }

  Address getTargetAddress() {
    if (insnAtPoint().isCall()) {
      if (insnAtPoint().isCallIndir()) {
	return 0;
      }
      else {
	return insnAtPoint().getTarget(address());
      }
    }
    return 0;
  }

  Address firstAddress() {return iPgetAddress();}
  Address followingAddress() {return iPgetAddress() + insnAtPoint_.size();}
  unsigned sizeOfInsnAtPoint() {return insnAtPoint_.size();}
  int sizeOfInstrumentation() {return 5;}

  bool match(instPoint *p);

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pd_Function * to) { callee_ = to;  }

  bool usesTrap(process *proc) const;
  bool canUseExtraSlot(process *proc) const;

 private:
  Address      addr_;    //The address of this instPoint: this is the address
                         // of the actual point (i.e. a function entry point,
			 // a call or a return instruction)
  pd_Function *func_;	 //The function where this instPoint belongs to
  pd_Function *callee_;	 //If this point is a call, the function being called

  Address              jumpAddr_;     //This is the address where we insert the jump.
                                      // It may be an instruction before the point
  instruction          insnAtPoint_;  //The instruction at this point
  vector<instruction> *insnBeforePt_; //Additional instructions before the point
  vector<instruction> *insnAfterPt_;  //Additional instructions after the point
  unsigned            bonusBytes_;    //Additional bytes after function for points
                                      //at end of function.
  bool relocated_;       // true if the function where this instPoint belongs
                         // has been relocated, and this instPoint has been 
                         // updated to reflect that relocation. 

};

#endif
