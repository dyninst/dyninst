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

#ifndef _FUNC_RELOC_H_
#define _FUNC_RELOC_H_

/*

#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/instPoint.h"

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/LocalAlteration-x86.h"
#endif

#if defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/arch-sparc.h"
#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/LocalAlteration-Sparc.h"
#endif

#if defined(ia64_unknown_linux2_4)
#include "dyninstAPI/src/arch-ia64.h"
#include "dyninstAPI/src/inst-ia64.h"
#include "dyninstAPI/src/LocalAlteration-ia64.h"
#endif

*/

#include "common/h/headers.h"
#include "codeRange.h"
#include "instPoint.h"

class LocalAlteration;
class LocalAlterationSet;
class process;
class pd_Function;


#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication. - TLM */

// Check for ExpandInstruction alterations that have already been found.
// Sparc does not use ExpandInstruction alterations
bool alreadyExpanded(int offset, int shift, 
                     LocalAlterationSet *alteration_set);
#endif

bool combineAlterationSets(LocalAlterationSet *alteration_set, 
                           LocalAlterationSet *temp_alteration_set);

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration);

// if a function needs to be relocated when it's instrumented then we need
// to keep track of new instrumentation points for this function on a per
// process basis (there is no guarentee that two processes are going to
// relocated this function to the same location in the heap)
class relocatedFuncInfo : public codeRange {
 public:
   relocatedFuncInfo(process *p, Address na, unsigned s, pd_Function *f):
     proc_(p), addr_(na), size_(s), funcEntry_(0), func_(f) {};
        
   ~relocatedFuncInfo(){proc_ = 0;}

   Address get_address() const { return addr_;}
   unsigned get_size() const { return size_;}
   codeRange *copy() const { return new relocatedFuncInfo(*this);}
   pd_Function *func() { return func_;}
    
   const process *getProcess(){ return proc_;}
   void setProcess(process *proc) { proc_ = proc; }
   instPoint *funcEntry() {
      return funcEntry_;
   }

   const pdvector<instPoint*> &funcReturns() const {
     return funcReturns_;
   }

   const pdvector<instPoint*> &funcCallSites() const {
      return calls_;
   }

   const pdvector<instPoint*> &funcArbitraryPoints() {
      return arbitraryPoints_;
   }

   void addFuncEntry(instPoint *e) {
      if(e) funcEntry_ = e;
   }
   void addFuncReturn(instPoint *r) {
      if(r) funcReturns_.push_back(r);
   }
   void addFuncCall(instPoint *c) {
      if(c) calls_.push_back(c);
   }
   void addArbitraryPoint(instPoint *r) {
      if(r) arbitraryPoints_.push_back(r);
   }

 private:
   const process *proc_;		// process assoc. with the relocation
   Address addr_;			// function's relocated address
   unsigned size_;             // Bulked-up size    
   instPoint *funcEntry_;		// function entry point
   pdvector<instPoint*> funcReturns_;    // return point(s)
   pdvector<instPoint*> calls_;          // pointer to the calls
   pdvector<instPoint*> arbitraryPoints_;          // pointer to the calls
   pd_Function *func_;         // "Parent" function pointer
};



#endif





