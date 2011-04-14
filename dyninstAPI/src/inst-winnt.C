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

// $Id: inst-winnt.C,v 1.31 2008/06/19 22:13:42 jaw Exp $

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/instPoint.h"

#include "dyninstAPI/src/function.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
using namespace Dyninst::InstructionAPI;

#ifndef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001
//defined in inst-mips.C

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
#endif

 

// hasBeenBound: returns false
// dynamic linking not implemented on this platform
bool process::hasBeenBound(const SymtabAPI::relocationEntry &,func_instance *&, Address ) {
    return false;
}

// findCallee: returns false unless callee is already set in instPoint
// dynamic linking not implemented on this platform
func_instance *instPoint::findCallee() 
{
   // Already been bound
   if (callee_) {
      return callee_;
   }  
   if (ipType_ != callSite) {
       return NULL;
   }
  
   if (!isDynamic()) {
      // We have a direct call but don't yet know the callee.
      // This may be because we didn't see a symbol for a
      // thunk in the ILT.
      // (I.e., the call is 'call @ILT+<smallconstant>'
      // and at @ILT+<smallconstant> there is a 'jmp <realfuncaddr>'
      // instruction.
      //
      // We consider the callee to be the real function that
      // is eventually called.
      
      // get the target address of the call
        Expression::Ptr cft = insn()->getControlFlowTarget();
		static Expression* theIP = new RegisterAST(x86::eip);
        cft->bind(theIP, Result(s32, addr()));
        Result r = cft->eval();
		assert(r.defined);
		Address callTarget = r.convert<Address>();
		parsing_printf(" **** instPoint::findCallee() callTarget = 0x%lx, insn = %s\n", callTarget, insn()->format().c_str());

        func_instance *func = proc()->findOneFuncByAddr(callTarget);
        if (func == NULL) return NULL;
      
	  /*
       * Handle the idiom discussed above, of calls to an entry in the ILT
	   * that then branch direct to the real function. If the instruction
	   * at `callTarget' is a branch to the start of a known function, 
	   * return that function as `callee_'; otherwise if there is a 
	   * function at `callTarget', return it as `callee_'.
	   */
      // get a "local" pointer to the call target instruction
        static const unsigned max_insn_size = 16; // covers AMD64
      const unsigned char *insnLocalAddr =
        (unsigned char *)(proc()->getPtrToInstruction(callTarget));
	  InstructionDecoder d(insnLocalAddr, max_insn_size, proc()->getArch());
      Instruction::Ptr insn = d.decode();
      if(insn && (insn->getCategory() == c_BranchInsn))
      {
          Expression::Ptr cft = insn->getControlFlowTarget();
		  static Expression* theIP = new RegisterAST(x86::eip);
          cft->bind(theIP, Result(u64, callTarget));
          Result r = cft->eval();
          if(r.defined)
          {
              Address targAddr = r.convert<Address>();
				parsing_printf(" **** instPoint::findCallee() targAddr = 0x%lx\n", targAddr);
              func_instance *target = proc()->findFuncByEntry(targAddr);
              if(target)
              {
                  callee_ = target;
                  return target;
              }
          }
		  else
		  {
			  parsing_printf("WARNING: couldn't evaluate IAT jump at 0x%lx: %s\n", callTarget, insn->format().c_str());
		  }
      }
	  if(NULL == callee_ && NULL != func) {
		  callee_ = func;
		  return callee_;
	  }
   }
   else
   {
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
      if(insn() && (insn()->getCategory() == c_CallInsn))
      {
          Expression::Ptr cft = insn()->getControlFlowTarget();
		  static Expression* theIP = new RegisterAST(x86::eip);
          cft->bind(theIP, Result(u64, addr()));
          Result r = cft->eval();
          if(r.defined)
          {
              Address funcPtrAddress = r.convert<Address>();
              assert( funcPtrAddress != ADDR_NULL );
                  
                // obtain the target address from memory if it is available
              Address targetAddr = ADDR_NULL;
              proc()->readDataSpace( (const void*)funcPtrAddress, sizeof(Address),
              &targetAddr, true );
              if( targetAddr != ADDR_NULL )
              {
            
            // see whether we already know anything about the target
            // this may be the case with implicitly-loaded and delay-loaded
            // DLLs, and it is possible with other types of indirect calls
            func_instance *target = proc()->findFuncByEntry( targetAddr );
                          
            // we need to handle the delay-loaded function case specially,
            // since we want the actual target function, not the temporary
            // code sequence that handles delay loading
                  if( (target != NULL) &&
                       (!strncmp( target->prettyName().c_str(), "_imp_load_", 10 )) )
                  {
               // The target is named like a linker-generated
               // code sequence for a call into a delay-loaded DLL.
                      //
               // Try to look up the function based on its name
               // without the 
                                  
#if READY
               // check if the target is in the same module as the function
               // containing the instPoint
               // check whether the function pointer is in the IAT
               if((funcPtrAddress >= something.iatBase) &&
                  (funcPtrAddress <= (something.iatBase+something.iatLength)))
{
                  // it is a call through the IAT, to a function in our
                  // own module, so it is a call into a delay-loaded DLL
                  // ??? how to handle this case
}
#else
               // until we can detect the actual callee for delay-loaded
               // DLLs, make sure that we don't report the linker-
               // generated code sequence as the target
               target = NULL;
#endif // READY
                  }
                  else {
                      callee_ = target;
                      return target;
                  }
              }
		  }
      }
   }
   return NULL;
}
