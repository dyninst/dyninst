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

// $Id: inst.C,v 1.160 2008/01/16 22:01:54 legendre Exp $
// Code to install and remove instrumentation from a running process.
// Misc constructs.

#include <assert.h>
//#include <sys/signal.h>
//#include <sys/param.h>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/InstrucIter.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/arch.h"


/*
 * return the time required to execute the passed primitive.
 *
 */
unsigned getPrimitiveCost(const pdstring &name)
{

    static bool init=false;

    if (!init) { init = 1; initPrimitiveCost(); }

    if (!primitiveCosts.defines(name)) {
      return 1;
    } else
      return (primitiveCosts[name]);
}


// find any tags to associate semantic meaning to function
unsigned findTags(const pdstring ) {
  return 0;
#ifdef notdef
  if (tagDict.defines(funcName))
    return (tagDict[funcName]);
  else
    return 0;
#endif
}

// IA64 has its own version; dunno if this would work there, so skipping for now
unsigned generateAndWriteBranch(AddressSpace *proc, 
                                Address fromAddr, 
                                Address newAddr,
                                unsigned fillSize)
{
    assert(fillSize != 0);

    codeGen gen(fillSize);

#if defined(os_aix)
    instruction::generateInterFunctionBranch(gen, fromAddr, newAddr);
#else
    instruction::generateBranch(gen, fromAddr, newAddr);
#endif
    gen.fillRemaining(codeGen::cgNOP);
    
    proc->writeTextSpace((caddr_t)fromAddr, gen.used(), gen.start_ptr());
    return gen.used();
}

unsigned trampEnd::maxSizeRequired() {
#if defined(arch_x86) || defined(arch_x86_64)
    unsigned illegalSize = 2;
#else
    unsigned illegalSize = instruction::size();
#endif
    // Return branch, illegal.
    return (instruction::maxJumpSize() + illegalSize);
}


trampEnd::trampEnd(multiTramp *multi, Address target) :
    multi_(multi), target_(target) 
{}

Address relocatedInstruction::originalTarget() const {
#if defined(cap_relocation)
    if (targetAddr_) return targetAddr_;
#else
    assert(targetAddr_ == 0);
#endif
  return insn->getTarget(origAddr_);
}

void relocatedInstruction::overrideTarget(patchTarget *newTarget) {
    targetOverride_ = newTarget;
}

#if defined (cap_unwind)
bool trampEnd::generateCode(codeGen &gen,
                            Address baseInMutatee,
                            UNW_INFO_TYPE ** unwindRegion ) 
#else
bool trampEnd::generateCode(codeGen &gen,
                            Address baseInMutatee,
                            UNW_INFO_TYPE ** /*unwindRegion*/ )
#endif
{
    generateSetup(gen, baseInMutatee);

    if (target_) {
        instruction::generateBranch(gen,
                                    gen.currAddr(baseInMutatee),
                                    target_);
    }

    // And a sigill insn
    instruction::generateIllegal(gen);
    
    size_ = gen.currAddr(baseInMutatee) - addrInMutatee_;
    generated_ = true;
    hasChanged_ = false;

#if defined( cap_unwind )
    /* The jump back is an zero-length ALIAS to the original location, followed
       by a no-op region covering the jump bundle. */
    dyn_unw_printf( "%s[%d]: aliasing tramp end to 0x%lx\n", __FILE__, __LINE__, multi_->instAddr() );
    unw_dyn_region_info_t * aliasRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
    assert( aliasRegion != NULL );
    aliasRegion->insn_count = 0;
    aliasRegion->op_count = 2;
    
    _U_dyn_op_alias( & aliasRegion->op[0], _U_QP_TRUE, -1, multi_->instAddr() );
    _U_dyn_op_stop( & aliasRegion->op[1] );
    
    unw_dyn_region_info_t * jumpRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 1 ) );
    assert( jumpRegion != NULL );
#if defined( arch_ia64 )    
    jumpRegion->insn_count = (size_ / 16) * 3;
#else
#error How do I know how many instructions are in the jump region?
#endif /* defined( arch_ia64 ) */
    jumpRegion->op_count = 1;
    
    _U_dyn_op_stop( & jumpRegion->op[0] );
    
    /* The care and feeding of pointers. */
    assert( unwindRegion != NULL );
    unw_dyn_region_info_t * prevRegion = * unwindRegion;
    prevRegion->next = aliasRegion;
    aliasRegion->next = jumpRegion;
    jumpRegion->next = NULL;
    * unwindRegion = jumpRegion;
#endif /* defined( cap_unwind ) */
        
    return true;
}

instMapping::instMapping(const instMapping *parIM,
                         process *child) :
    func(parIM->func),
    inst(parIM->inst),
    where(parIM->where),
    when(parIM->when),
    order(parIM->order),
    useTrampGuard(parIM->useTrampGuard),
    mt_only(parIM->mt_only),
    allow_trap(parIM->allow_trap)
{
    for (unsigned i = 0; i < parIM->args.size(); i++) {
        args.push_back(parIM->args[i]);
    }
    for (unsigned j = 0; j < parIM->miniTramps.size(); j++) {
        miniTramp *cMT = NULL;
        cMT = parIM->miniTramps[j]->getInheritedMiniTramp(child);
        assert(cMT);
        miniTramps.push_back(cMT);
    }
}

Address relocatedInstruction::relocAddr() const {
    return addrInMutatee_;
}

void *relocatedInstruction::getPtrToInstruction(Address) const {
    assert(0); // FIXME if we do out-of-line baseTramps
    return NULL;
}

void *trampEnd::getPtrToInstruction(Address) const {
    assert(0); // FIXME if we do out-of-line baseTramps
    return NULL;
}

// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
//
// Note that right now we can only replace a call instruction that is five
// bytes long (like a call to a 32-bit relative address).
// 18APR05: don't see why we can't fix up a base tramp, it's just a little more
// complicated.

// By the way, we want to be able to disable these, which means keeping
// track of what we put in. Since addresses are unique, we can do it with
// a dictionary in the process class.

#if !defined(arch_ia64)
// arch-ia64 has its own version (of course...) since it's a lot
// more complicated
bool AddressSpace::replaceFunctionCall(instPoint *point,
                                       const int_function *func) {
    // Must be a call site
  if (point->getPointType() != callSite)
    return false;

  instPointIter ipIter(point);
  instPointInstance *ipInst;
  while ((ipInst = ipIter++)) {  
      // Multiple replacements. Wheee...
      Address pointAddr = ipInst->addr();

      codeRange *range = findModByAddr(pointAddr);
      if (range) {
            multiTramp *multi = range->is_multitramp();
          if (multi) {
                // We pre-create these guys... so check to see if
              // there's anything there
              if (!multi->generated()) {
                  removeMultiTramp(multi);
              }
              else {
                  // TODO: modify the callsite in the multitramp.
                  assert(0);
              }
          }
          else if (dynamic_cast<functionReplacement *>(range)) {
              // We overwrote this in a function replacement...
              continue; 
          }
      }
      codeGen gen(point->insn().size());
      // Uninstrumented
      // Replace the call
      if (func == NULL) {	// Replace with NOOPs
          gen.fillRemaining(codeGen::cgNOP);
      } else { // Replace with a call to a different function
          // XXX Right only, call has to be 5 bytes -- sometime, we should make
          // it work for other calls as well.
          //assert(point->insn().size() == CALL_REL32_SZ);
          instruction::generateCall(gen, pointAddr, func->getAddress());
      }
      
      // Before we replace, track the code.
      // We could be clever with instpoints keeping instructions around, but
      // it's really not worth it.
      replacedFunctionCall *newRFC = new replacedFunctionCall();
      newRFC->callAddr = pointAddr;
      newRFC->callSize = point->insn().size();
      if (func)
          newRFC->newTargetAddr = func->getAddress();
      else
          newRFC->newTargetAddr = 0;

      codeGen old(point->insn().size());
      old.copy(point->insn().ptr(), point->insn().size());
      
      newRFC->oldCall = old;
      newRFC->newCall = gen;
      
      addReplacedCall(newRFC);
      
      writeTextSpace((void *)pointAddr, gen.used(), gen.start_ptr());
  }
  return true;
}
#endif
