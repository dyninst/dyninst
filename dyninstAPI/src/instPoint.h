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

// $Id: instPoint.h,v 1.15 2005/02/09 03:27:47 jaw Exp $
// Defines class instPoint

#ifndef _INST_POINT_H_
#define _INST_POINT_H_

#include <stdlib.h>
#include "common/h/Types.h"

typedef enum {
    noneType,
    functionEntry,
    functionExit,
    callSite,
    otherPoint
} instPointType;

class int_function;
class instPoint;
class process;
class image;
#ifdef __XLC__
class BPatch_point;
#endif

class instPointBase {
   static unsigned int id_ctr;
   unsigned int id;       // for tracking matching instPoints in original
                          // and relocated functions

   bool relocated_;       // true if the function where this instPoint belongs
                          // has been relocated, and this instPoint has been 
                          // updated to reflect that relocation. 

   instPointType ipType;
   Address      addr_;    //The address of this instPoint: this is the address
                          // of the actual point (i.e. a function entry point,
                          // a call or a return instruction)
   int_function *func_;	 //The function where this instPoint belongs to
   int_function *callee_;	//If this point is a call, the function being called

   instPoint *getMatchingInstPoint(process *p) const;

   // We need this here because BPatch_point gets dropped before
   // we get to generate code from the AST, and we glue info needed
   // to generate code for the effective address snippet/node to the
   // BPatch_point rather than here.
   friend class BPatch_point;
   BPatch_point *bppoint; // unfortunately the correspondig BPatch_point
                          // is created afterwards, so it needs to set this

 public:
    //ELI
    // if this instPoint is created in an edge tramp then this 
    // address is the address of the jcc instruction
    Address addrInFunc;

   instPointBase(instPointType iptype, Address addr, int_function *func) :
      id(id_ctr++), relocated_(false), ipType(iptype), addr_(addr),
      func_(func), callee_(NULL), bppoint(NULL),addrInFunc(0) { }

   instPointBase(unsigned int id_to_use, instPointType iptype, Address addr,
                 int_function *func) :
      id(id_to_use), relocated_(true), ipType(iptype), addr_(addr),
      func_(func), callee_(NULL), bppoint(NULL),addrInFunc(0) { }

   virtual ~instPointBase() { }

   unsigned int getID() const { return id; }   
   bool isRelocatedPointType() const { return relocated_; }   
   instPointType getPointType() const { return ipType; }

   // returns NULL if can't find a matching relocated inst-point
   // should be called from an original inst point
   instPoint *getMatchingRelocInstPoint(process *p) const {
      if(isRelocatedPointType())
         return NULL;
      
      return getMatchingInstPoint(p);
   }

   // returns NULL if can't find a matching original inst-point
   // should be called from a relocated inst point
   instPoint *getMatchingOrigInstPoint() const {
      if(! isRelocatedPointType())
         return NULL;
      
      return getMatchingInstPoint(NULL);
   }

   Address pointAddr() const { return addr_; }
   int_function *pointFunc() const { return func_; }
   virtual int_function *getCallee() const { return callee_; }

   image *getOwner() const;
   Address absPointAddr(process *proc) const;
   
   // can't set this in the constructor because call points can't be
   // classified until all functions have been seen -- this might be cleaned
   // up
   void setCallee(int_function * to) { callee_ = to;  }


   BPatch_point* getBPatch_point() const { return bppoint; }
   void setBPatch_point(const BPatch_point *p) { 
      bppoint = const_cast<BPatch_point *>(p);
   }
};


// architecture-specific implementation of class instPoint
#if defined(sparc_sun_solaris2_4) || defined(sparc_sun_sunos4_1_3)
#include "instPoint-sparc.h"
#elif defined(rs6000_ibm_aix4_1)
#include "instPoint-power.h"
#elif defined(i386_unknown_nt4_0)
#include "instPoint-x86.h"
#elif defined(i386_unknown_solaris2_5)
#include "instPoint-x86.h"
#elif defined(alpha_dec_osf4_0)
#include "instPoint-alpha.h"
#elif defined(i386_unknown_linux2_0)
#include "instPoint-x86.h"
#elif defined(ia64_unknown_linux2_4)
#include "instPoint-ia64.h"
#elif defined(mips_sgi_irix6_4)  || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
#include "instPoint-mips.h"
#else
#error unknown architecture
#endif

#endif /* _INST_POINT_H_ */
