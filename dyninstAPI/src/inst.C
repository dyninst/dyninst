/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

// $Id: inst.C,v 1.166 2008/06/26 20:40:15 bill Exp $
// Code to install and remove instrumentation from a running process.
// Misc constructs.

#include <assert.h>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/codegen.h"


/*
 * return the time required to execute the passed primitive.
 *
 */
unsigned getPrimitiveCost(const std::string &name)
{

    static bool init=false;

    if (!init) { init = 1; initPrimitiveCost(); }

    if (!primitiveCosts.defines(name)) {
      return 1;
    } else
      return (primitiveCosts[name]);
}


// find any tags to associate semantic meaning to function
unsigned findTags(const std::string ) {
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
    insnCodeGen::generateInterFunctionBranch(gen, fromAddr, newAddr);
#else
    insnCodeGen::generateBranch(gen, fromAddr, newAddr);
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
    return (instruction::maxJumpSize(multi_->proc()->getAddressWidth()) + illegalSize);
}


trampEnd::trampEnd(multiTramp *multi, Address target) :
    multi_(multi), target_(target) 
{}

Address relocatedInstruction::originalTarget() const {
  if (targetAddr_) return targetAddr_;
  return insn->getTarget(origAddr_);
}

void relocatedInstruction::overrideTarget(patchTarget *newTarget) {
  targetOverride_ = newTarget;
}

bool trampEnd::generateCode(codeGen &gen,
                            Address baseInMutatee,
                            UNW_INFO_TYPE ** /*unwindRegion*/ )
{
    generateSetup(gen, baseInMutatee);

    if (target_) {
        insnCodeGen::generateBranch(gen,
                                    gen.currAddr(baseInMutatee),
                                    target_);
    }

    // And a sigill insn
    insnCodeGen::generateIllegal(gen);
    
    size_ = gen.currAddr(baseInMutatee) - addrInMutatee_;
    generated_ = true;
    hasChanged_ = false;
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

bool AddressSpace::replaceFunctionCall(instPoint *point,
                                       const int_function *func) {
   // Must be a call site
   if (point->getPointType() != callSite)
      return false;

   if (!func) {
      // Remove the function call entirely
      AstNodePtr nullNode = AstNode::nullNode();
      return point->replaceCode(nullNode);
   }

   // To do this we must be able to make a non-state-affecting
   // function call. Currently it's only implemented on POWER, although
   // it would be easy to do for x86 as well...
#if defined(cap_instruction_replacement) 
   pdvector<AstNodePtr> emptyArgs;
   AstNodePtr call = AstNode::funcCallNode(const_cast<int_function *>(func), emptyArgs);
#else
   AstNodePtr call = AstNode::funcCallNode(const_cast<int_function *>(func));
#endif
   return point->replaceCode(call);
}
