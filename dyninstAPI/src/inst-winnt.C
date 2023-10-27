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

// $Id: inst-winnt.C,v 1.31 2008/06/19 22:13:42 jaw Exp $

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/instPoint.h"

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include <boost/tuple/tuple.hpp>

using namespace Dyninst::InstructionAPI;


//
// All costs are based on Measurements on a SPARC station 10/40.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    primitiveCosts["DYNINSTprintCost"] = 1;

    //
    // I can't find DYNINSTincrementCounter or DYNINSTdecrementCounter
    // I think they are not being used anywhere - naim
    //
    // isthmus acutal numbers from 7/3/94 -- jkh
    // 240 ns
    primitiveCosts["DYNINSTincrementCounter"] = 16;
    // 240 ns
    primitiveCosts["DYNINSTdecrementCounter"] = 16;

    // Updated calculation of the cost for the following procedures.
    // cost in cycles
    // Values (in cycles) benchmarked on a Pentium II 450MHz
    // Level 1 - Hardware Level
    primitiveCosts["DYNINSTstartWallTimer"] = 151;
    primitiveCosts["DYNINSTstopWallTimer"] = 165;

    // Implementation still needs to be added to handle start/stop
    // timer costs for multiple levels
    /* Level 2 - Software Level
    primitiveCosts["DYNINSTstartWallTimer"] = 797;
    primitiveCosts["DYNINSTstopWallTimer"] = 807;
    */
    primitiveCosts["DYNINSTstartProcessTimer"] = 990;
    primitiveCosts["DYNINSTstopProcessTimer"] = 1017;

    // These happen async of the rest of the system.
    primitiveCosts["DYNINSTalarmExpire"] = 3724;
    primitiveCosts["DYNINSTsampleValues"] = 13;
    primitiveCosts["DYNINSTreportTimer"] = 1380;
    primitiveCosts["DYNINSTreportCounter"] = 1270;
    primitiveCosts["DYNINSTreportCost"] = 1350;
    primitiveCosts["DYNINSTreportNewTags"] = 837;
}


// hasBeenBound: returns false
// dynamic linking not implemented on this platform
bool PCProcess::hasBeenBound(const SymtabAPI::relocationEntry &,func_instance *&, Address ) {
    return false;
}

bool PCProcess::bindPLTEntry(const SymtabAPI::relocationEntry &entry, Address base_addr, 
                           func_instance *, Address target_addr) {
   return false;
}

bool thunkILT(edge_instance *edge, AddressSpace *proc, func_instance *&ret) {
	assert(!edge->sinkEdge());
	// We have a direct call but don't yet know the callee.
	// This may be because we didn't see a symbol for a
	// thunk in the ILT.
	// (I.e., the call is 'call @ILT+<smallconstant>'
    // and at @ILT+<smallconstant> there is a 'jmp <realfuncaddr>'
    // instruction.
    //
    // We consider the callee to be the real function that
    // is eventually called.
	// In CFG terms, this is:
	// We're calling a function that satisfies the following:
	//  1) 1 block long
	//  2) 1 instruction long
	//  3) Has a direct (?) edge to another function entry point.

    // get the target address of the call

    func_instance *cFunc = proc->findFuncByEntry(edge->trg());
	if (cFunc == NULL) return false;

	// 1)
	if (cFunc->blocks().size() > 1) return false;

	// 2)
	block_instance *cBlock = cFunc->entryBlock();
	block_instance::Insns cInsns;
	cBlock->getInsns(cInsns);
	if (cInsns.size() > 1) return false;

	// 3) 
	edge_instance *cEdge = cBlock->getTarget();
	if (!cEdge) return false;
	if (cEdge->sinkEdge()) return false;

	block_instance *block = cEdge->trg();
	if (!block) return false;
	func_instance *func = proc->findFuncByEntry(block);
	if (!func) return false;

	ret = func;
	return true;
}

// findCallee: returns false unless callee is already set in instPoint
// dynamic linking not implemented on this platform
func_instance *block_instance::callee() 
{
	/* Unlike Linux, we do some interpretation here. Windows uses a common idiom of the following.
	 * Source:
	 *   void foo() { bar(); }
	 *   void bar() { ... }
	 * Which turns into
	 * foo:
	 *   call stub
	 * bar:
	 *   ...
	 * stub:
	 *   jmp bar
	 *
	 * Since we're interpreting callees, we want to track this down
	 * and represent the callee as bar, not stub.
	 */
   if (!containsCall()) {
      return NULL;
   }

   edge_instance *tEdge = getTarget();
	if (!tEdge) return NULL;

    // Otherwise use the target function...
	if (!tEdge->sinkEdge()) {
	    func_instance *ret;
		// If we're calling a 1-instruction function that branches to a known entry point,
		// elide that...
		if (thunkILT(tEdge, proc(), ret)) {
			return ret;
		}
		return tEdge->trg()->obj()->findFuncByEntry(tEdge->trg());
    }

	  // An call that uses an indirect call instruction could be one
	  // of three things:
	  //
	  // 1. A call to a function within the same executable or DLL.  In 
	  //    this case, it could be a call through a function pointer in
	  //    memory or a register, or some other type of call.
	  //   
	  // 2. A call to a function in an implicitly-loaded DLL.  These are
	  //    indirect calls through an entry in the object's Import Address 
	  //    Table (IAT). An IAT entry is set to the address of the target 
	  //    when the DLL is loaded at process creation.
	  //
	  // 3. A call to a function in a delay-loaded DLL.  Like calls to 
	  //    implicitly-loaded DLLs, these are calls through an entry in the 
	  //    IAT.  However, for delay-loaded DLLs the IAT entry initially 
	  //    holds the address of a short code sequence that loads the DLL,
	  //    looks up the target function, patches the IAT entry with the 
	  //    address of the target function, and finally executes the target 
	  //    function.  After the first call, the IAT entry has been patched
	  //    and subsequent calls look the same as calls into an
	  //    implicitly-loaded DLL.
	  //
	  // Figure out what type of indirect call instruction this is
	  //
	   InstructionAPI::Instruction::Ptr insn = getInsn(last());
	  if(insn && (insn->getCategory() == c_CallInsn))
	  {
		  Expression::Ptr cft = insn->getControlFlowTarget();
		  static Expression* theIP = new RegisterAST(x86::eip);
		  cft->bind(theIP, Result(u64, last()));
		  Result r = cft->eval();
		  if(r.defined)
		  {
			  Address funcPtrAddress = r.convert<Address>();
			  assert( funcPtrAddress != ADDR_NULL );
	              
				// obtain the target address from memory if it is available
			  Address targetAddr = ADDR_NULL;
              if (insn->readsMemory()) {
                 proc()->readDataSpace( (const void*)funcPtrAddress, 
                                        sizeof(Address), &targetAddr, true );
              } 
              else { // this is not an indirect call at all, but a call to 
                     // an uninitialized or invalid address
                  targetAddr = funcPtrAddress;
              }
              if( targetAddr != ADDR_NULL )
              {
                 // see whether we already know anything about the target
                 // this may be the case with implicitly-loaded and delay-loaded
                 // DLLs, and it is possible with other types of indirect calls
                 func_instance *target = proc()->findFuncByEntry( targetAddr );
                 if (target) {
                     updateCallTarget(target);
                     return target;
                 }
              }
		  }
	  }
  return NULL;
}
