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

// $Id: instPoint-power.h,v 1.10 2003/10/21 17:22:12 bernat Exp $

#ifndef _INST_POINT_POWER_H_
#define _INST_POINT_POWER_H_

#include "inst-power.h"

class process;

#ifdef BPATCH_LIBRARY
class BPatch_point;
#endif

class instPoint {
public:
  instPoint(pd_Function *f, const instruction &instr, const image *owner,
	    const Address adr, const bool delayOK, const ipFuncLoc);

  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified
  // until all functions have been seen -- this might be cleaned up
  void set_callee(pd_Function *to) { callee = to; }

  pd_Function *func() const { return func_; }
  const function_base *iPgetFunction() const { return func();   }
  const function_base *iPgetCallee()   const { return callee; }
  const image         *iPgetOwner()    const { 
      return (func()) ? ( (func()->file()) ? func()->file()->exec() : NULL ) : NULL; }
  Address        iPgetAddress(process *p = 0)  const;
  

  bool match(instPoint *p);
  
  instruction originalInstruction;    /* original instruction */

  bool callIndirect;		/* is it a call whose target is rt computed ? */

  pd_Function *callee;		/* what function is called */
  pd_Function *func_;		/* what function we are inst */

  
  
  ipFuncLoc ipLoc;

  // VG(11/06/01): there is some common stuff amongst instPoint
  // classes on all platforms (like addr and the back pointer to
  // BPatch_point). 
  // TODO: Merge these classes and put ifdefs for platform-specific
  // fields.
 private:
  // We need this here because BPatch_point gets dropped before
  // we get to generate code from the AST, and we glue info needed
  // to generate code for the effective address snippet/node to the
  // BPatch_point rather than here.
  friend class BPatch_point;
  BPatch_point *bppoint; // unfortunately the correspondig BPatch_point
  			 // is created afterwards, so it needs to set this
  // CALL iPgetAddress() instead! This is not an absolute address!
  Address addr;                   /* address of inst point */

  public:
  const BPatch_point* getBPatch_point() const { return bppoint; }
};

#endif
