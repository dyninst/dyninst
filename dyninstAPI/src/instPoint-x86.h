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

// $Id: instPoint-x86.h,v 1.26 2005/01/11 22:47:11 legendre Exp $

#ifndef _INST_POINT_X86_H_
#define _INST_POINT_X86_H_

#include "dyninstAPI/src/symtab.h"

class process;

#ifdef BPATCH_LIBRARY
class BPatch_point;
#endif

/************************************************************************
 *
 *  class instPoint: representation of instrumentation points
 *
 ***********************************************************************/

class instPoint : public instPointBase {   
 public:    
   
  instPoint(pd_Function *f, const image *, Address adr, instPointType ipt,
            instruction inst, bool conservative = false) :
      instPointBase(ipt, adr, f), 
      insnAtPoint_(inst), hasInsnAtPoint_(true), conservative_(conservative)
  {
     init();
  };

  instPoint(pd_Function *f, const image *, Address adr, instPointType ipt,
            bool conservative = false) :
     instPointBase(ipt, adr, f), 
      hasInsnAtPoint_(false), conservative_(conservative)
  {
     init();
     // adr is fine for the jump address since we know adr is the start
     // of a sequence of 5 nops
     setJumpAddr(adr);
  };

  instPoint(unsigned int id_of_parent, pd_Function *f, const image *i,
            Address adr, instPointType ipt, instruction inst,
            bool conservative = false) :
      instPointBase(id_of_parent, ipt, adr, f),
      insnAtPoint_(inst), hasInsnAtPoint_(true), conservative_(conservative)
  {
     init();
  };


  ~instPoint() {
    if (insnBeforePt_) delete insnBeforePt_;
    if (insnAfterPt_) delete insnAfterPt_;
  };

  // return true if there is an instruction at this point, the point
  // is a sequence of 5 nops that we created to instrument
  bool hasInsnAtPoint() const { return hasInsnAtPoint_; }

  const instruction &insnAtPoint() const { return insnAtPoint_; }

  int insnAddress() { return pointAddr(); }

  // add an instruction before the point. Instructions should be added in reverse
  // order, the instruction closest to the point first.
  void addInstrBeforePt(instruction inst) {
    if (!insnBeforePt_) insnBeforePt_ = new pdvector<instruction>;
    (*insnBeforePt_).push_back(inst);
  };

  // add an instruction after the point.
  void addInstrAfterPt(instruction inst) {
    if (!insnAfterPt_) insnAfterPt_ = new pdvector<instruction>;
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
      if (!hasInsnAtPoint()) {
          return pointAddr() + 5;
      }
      else {
          Address ret = pointAddr() + insnAtPoint_.size();
          for (unsigned u = 0; u < insnsAfter(); u++)
              ret += (*insnAfterPt_)[u].size();
          return ret;
      }
  }

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
  unsigned size() const;

    // return the number of bytes that need to be added so that the
  // instruction will be instrumentable after the function has been relocated
  int extraBytes() const {
      if (!hasInsnAtPoint()) return 0;

    assert (jumpAddr_ <= pointAddr());

    int tSize = (pointAddr() - jumpAddr_) + insnAtPoint_.size();

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

  // check for jumps to instructions before and/or after this point, and
  // discard instructions when there is a jump.  Can only be done after an
  // image has been parsed (that is, all inst. points in the image have been
  // found.
  void checkInstructions ();

  pd_Function *getCallee() const { 
      if (!hasInsnAtPoint()) return NULL;
      if (insnAtPoint().isCall()) {
          if (insnAtPoint().isCallIndir())
              return NULL;
          else {
             return instPointBase::getCallee();
          }
      }
      return NULL;
  }

  Address getTargetAddress() {
      if (!hasInsnAtPoint()) return 0;
     if (insnAtPoint().isCall()) {
        if (insnAtPoint().isCallIndir()) {
           return 0;
        }
        else {
           return insnAtPoint().getTarget(pointAddr());
        }
     }
     return 0;
  }

  Address firstAddress() {return pointAddr();}
  Address followingAddress() {
      if (hasInsnAtPoint()) 
          return pointAddr() + insnAtPoint_.size();
      else 
          return pointAddr() + 5;
  }
  unsigned sizeOfInsnAtPoint() {
      if (hasInsnAtPoint()) 
          return insnAtPoint_.size();
      else 
          return 0;
  }
  int sizeOfInstrumentation() {return 5;}

  bool usesTrap(process *proc) const;
  bool canUseExtraSlot(process *proc) const;

  bool isConservative() const { return conservative_; }

 private:
  Address              jumpAddr_;     // the address where we insert the jump.
                                      // may be an instruction before the point

  instruction          insnAtPoint_;  // the instruction at this point

  // instPoint might not have an instruction at the point
  bool hasInsnAtPoint_;

  //Additional instructions before the point
  pdvector<instruction> *insnBeforePt_;

  //Additional instructions after the point
  pdvector<instruction> *insnAfterPt_;

  //Additional bytes after function for points at end of function.
  unsigned            bonusBytes_;    

  bool conservative_;    // true for arbitrary inst points

  // VG(11/06/01): there is some common stuff amongst instPoint
  // classes on all platforms (like addr and the back pointer to
  // BPatch_point). 
  // TODO: Merge these classes and put ifdefs for platform-specific
  // fields.

  void init() {
     jumpAddr_     = 0;
     insnBeforePt_ = 0;
     insnAfterPt_  = 0;
     bonusBytes_   = 0;
     addrInFunc = 0;
  }
};

#endif
