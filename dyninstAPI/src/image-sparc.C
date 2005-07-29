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


// $Id: image-sparc.C,v 1.1 2005/07/29 19:22:57 bernat Exp $

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "InstrucIter.h"
#include "showerror.h"
#include "arch.h"
#include "inst-sparc.h" // REG_? should be in arch-sparc, but isn't


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


static inline bool CallRestoreTC(instruction instr, instruction nexti) {
    return (instr.isCall() && nexti.isRestore());
}

/****************************************************************************/
/****************************************************************************/

static inline bool MovCallMovTC(instruction instr, instruction nexti) {
    return (instr.isCall() && nexti.isMovToO7());
}

/****************************************************************************/
/****************************************************************************/


/*
    Return bool value indicating whether instruction sequence
     found signals tail-call jmp; nop; sequence.  Note that this should 
     NOT include jmpl; nop;, ret; nop;, retl; nop;....

    Current heuristic to detect such sequences :
     look for jmp %reg, nop in function w/ no stack frame, if jmp, nop
     are last 2 instructions, return true (definate TC), at any other point,
     return false (not TC).  Otherwise, return false (no TC).
     w/ no stack frame....
    instr is instruction being examioned.
    nexti is instruction after
    addr is address of <instr>
    func is pointer to function class object describing function
     instructions come from....
 */
static inline bool JmpNopTC(instruction instr, instruction nexti,
			    Address addr, image_func *func) {

    if (!instr.isInsnType(JMPLmask, JMPLmatch)) {
        return 0;
    }
    
    assert((*instr).resti.op3 == 0x38);
    
    // only looking for jump instructions which don't overwrite a register
    //  with the PC which the jump comes from (g0 is hardwired to 0, so a write
    //  there has no effect?)....  
    //  instr should have gdb disass syntax : 
    //      jmp  %reg, 
    //  NOT jmpl %reg1, %reg2
    if ((*instr).resti.rd != REG_G(0)) {
        return 0;
    }

    // only looking for jump instructions in which the destination is
    //  NOT %i7 + 8/12/16 or %o7 + 8/12/16 (ret and retl synthetic 
    //  instructions, respectively)
    if ((*instr).resti.i == 1) {
        if ((*instr).resti.rs1 == REG_I(7) || (*instr).resti.rs1 == REG_O(7)) {
	    // NOTE : some return and retl instructions jump to {io}7 + 12,
	    //  or (io)7 + 16, not + 8, to have some extra space to store the size of a 
	    //  return structure....
            if ((*instr).resti.simm13 == 0x8 || (*instr).resti.simm13 == 12 ||
		    (*instr).resti.simm13 == 16) {
	        return 0;
	    }
        }
    }  

    // jmp, foloowed by NOP....
    if (!nexti.isNop()) {
        return 0;
    }

    // in function w/o stack frame....
    if (!func->hasNoStackFrame()) {
        return 0;
    }
    
    // if sequence is detected, but not at end of fn 
    //  (last 2 instructions....), return value indicating possible TC.
    //  This should (eventually) mark the fn as uninstrumenatble....
    if (addr != (func->getOffset() + func->getSize() - 8)) {
        return 0;
    }
    
    return 1;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
  Is the specified call instruction one whose goal is to set the 07 register
  (the sequence of execution is as if the call instruction did not change the
  control flow, and the O7 register is set)?

  here, we define a call whose goal is to set the 07 regsiter
    as one where the target is the call address + 8, AND where that
    target is INSIDE the same function (need to make sure to check for that
    last case also, c.f. function DOW, which ends with):
     0xef601374 <DOW+56>:    call  0xef60137c <adddays>
     0xef601378 <DOW+60>:    restore 

  instr - raw instruction....
  functionSize - size of function (in bytes, NOT # instructions)....
  instructionOffset - BYTE offset in function at which instr occurs....
 */        
static inline bool is_set_O7_call(instruction instr, unsigned functionSize, 
			      unsigned instructionOffset) {
    // if the instruction is call %register, assume that it is NOT a 
    //  call designed purely to set %O7....
    if((*instr).call.op != CALLop) {
        return false;
    }
    if ((((*instr).call.disp30 << 2) == 8) && 
             (instructionOffset < (functionSize - 2 * instruction::size()))) {
        return true; 
    }
    return false; 
}  

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
    Does the specified call instruction call to target inside function
    or outside - may be indeterminate if insn is call %reg instead of 
    call <address> (really call PC + offset)
    Note: (recursive) calls back to the beginning of the function are OK
    since we really want to consider these as instrumentable call sites!
 */
enum fuzzyBoolean {eFalse = 0, eTrue = 1, eDontKnow = 2}; 

static enum fuzzyBoolean is_call_outside_function(const instruction instr,
                const Address functionStarts, const Address instructionAddress, 
		const unsigned int functionSize) 
{
    // call %register - don't know if target inside function....
    if((*instr).call.op != CALLop) {
        return eDontKnow;
    }
    const Address call_target = instructionAddress + ((*instr).call.disp30 << 2);
    if ((call_target > functionStarts) && 
        (call_target < (functionStarts + functionSize))) {
        return eFalse;
    }
    return eTrue;
}

bool image_func::findInstPoints(pdvector<Address> &callTargets)
{
    if( parsed_ ) 
    {
        parsing_printf("Error: multiple call of findInstPoints\n");
        return false;
    } 
    parsed_ = true;

    parsing_printf("findInstPoints for func %s, at 0x%x, size %d\n",
            symTabName().c_str(), getOffset(), getSize());

    makesNoCalls_ = true;
    noStackFrame = true;
    int insnSize = instruction::size();

    if (getSize() <= 3*insnSize)
        return false;

   BPatch_Set< Address > leaders;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );
       
    Address funcBegin = getOffset();
    Address funcEnd = funcBegin;
    
    InstrucIter ah (funcBegin, this);

    if (!ah.getInstruction().valid()) {
        isInstrumentable_ = false;
        return false;
    }

    // And here we have hackage. Our jumptable code is b0rken, but I don't know
    // how to fix it. So we define anything that doesn't work as... argh.
    // Better: a size limit on a jump table.
    if (symTabName().c_str() == "__rtboot") {
        isInstrumentable_ = false;
        return false;
    }
        

    image_instPoint *p;

    //define entry instpoint 
    p = new image_instPoint( funcBegin,
                             ah.getInstruction(),
                             this,
                             functionEntry);
    funcEntries_.push_back(p);
    
    Address currAddr = funcBegin;
    
    // Can move; so we reset
    int framesize;
    if (ah.isStackFramePreamble(framesize))
        noStackFrame = false;
    ah.setCurrentAddress(funcBegin);

    // Will peek a few instructions ahead.
    // Not sure if this is needed on sparc; we have that o7_live instead
    //if (ah.isReturnValueSave())
    ///        makesNoCalls_ = false;
    ah.setCurrentAddress(funcBegin);
 
    //find all the basic blocks and define the instpoints for this function
    BPatch_Set< Address > visited;
    pdvector< Address > jmpTargets;

    //entry basic block
    leaders += funcBegin;
    leadersToBlock[ funcBegin ] = new image_basicBlock(this, funcBegin);
    leadersToBlock[ funcBegin ]->isEntryBlock_ = true;
    blockList.push_back( leadersToBlock[ funcBegin ] );
    jmpTargets.push_back( funcBegin );
    
    for( unsigned i = 0; i < jmpTargets.size(); i++ )
    {
        //parsing_printf("Making block iter for target 0x%x\n", jmpTargets[i]);
        InstrucIter ah( jmpTargets[ i ], this); 
        image_basicBlock* currBlk = leadersToBlock[ jmpTargets[ i ] ];
        assert(currBlk->firstInsnOffset() == jmpTargets[i]);

        while( true ) {
            currAddr = *ah;

            if (currAddr >= getOffset() + getSize())
                break;

            if( visited.contains( currAddr ) )
                break;
            else
                visited += currAddr;

            parsing_printf("checking insn at 0x%x, %s + %d\n", *ah,
                    symTabName().c_str(), (*ah) - funcBegin);

            // Check for tail calls; we auto-relocate the function (why?)
            // but flag it here.
            if (CallRestoreTC(ah.getInstruction(), ah.getNextInstruction()) ||
                JmpNopTC(ah.getInstruction(), ah.getNextInstruction(), *ah, this) ||
                MovCallMovTC(ah.getInstruction(), ah.getNextInstruction())) {
                parsing_printf("ERROR: tail call (?) not handled in func %s at 0x%x\n",
                        symTabName().c_str(), *ah);
            }

            // Check whether "07" is live, AKA we can't call safely.
            // Could we just always assume this?
            if (!o7_live) {
                InsnRegister rd, rs1, rs2;
                ah.getInstruction().get_register_operands(&rd, &rs1, &rs2);
                if (rs1.is_o7() || rs2.is_o7()) {
                    o7_live = true;
                    parsing_printf("Setting o7 to live at 0x%x, func %s\n",
                            *ah, symTabName().c_str());
                }
            }

            
            if( ah.isACondBranchInstruction() )
                {
                    parsing_printf("cond branch\n");

                    currBlk->lastInsnOffset_ = currAddr; 
                    currBlk->blockEndOffset_ = currAddr + insnSize;
                    if( currAddr >= funcEnd )
                        funcEnd = currAddr + insnSize;
                    
                    Address target = ah.getBranchTargetAddress();
                    // img()->addJumpTarget(target);
		    if( (target < funcBegin) ||
			(target >= funcBegin + size_)) {

                        currBlk->isExitBlock_ = true;
                        // And make an inst point
                        p = new image_instPoint(currAddr, 
                                                ah.getInstruction(),
                                                this,
                                                functionExit);
                        parsing_printf("Function exit (1) at 0x%x\n", *ah);
                        funcReturns.push_back(p);
                    }
                    else {
                        jmpTargets.push_back( target );
                        
                        //check if a basicblock object has been 
                        //created for the target
                        if( !leaders.contains( target ) )
                            {
                                //if not, then create one
                                leadersToBlock[ target ] = new image_basicBlock (this, target);
                                leaders += target;
                                blockList.push_back( leadersToBlock[ target ] );
                            }
                        leadersToBlock[ target ]->addSource( currBlk );
                        currBlk->addTarget( leadersToBlock[ target ] );
                    }            
                    
                    Address t2 = currAddr + insnSize;
                    jmpTargets.push_back( t2 );
                    if( !leaders.contains( t2 ) )
                        {
                            leadersToBlock[ t2 ] = new image_basicBlock(this, t2);
                            leaders += t2;
                            blockList.push_back( leadersToBlock[ t2 ] );
                        }                 
                    leadersToBlock[ t2 ]->addSource( currBlk );
                    currBlk->addTarget( leadersToBlock[ t2 ] );
                    break;
                }
             else if( ah.isAIndirectJumpInstruction() )
             {
                 parsing_printf("ind branch\n");
                 //parsing_printf("Copying iter for indirect jump\n");
                 InstrucIter ah2( ah );
                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = currAddr + insnSize;
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                 
                 BPatch_Set< Address > res;
                 ah2.getMultipleJumpTargets( res );
                                  
                 BPatch_Set< Address >::iterator iter;
                 iter = res.begin();

                 bool leavesFunc = false;

                 while( iter != res.end() )
                 {
                     if( *iter < funcBegin ||
                         *iter >= getOffset() + getSize()) {
                         leavesFunc = true;
                     }
                     else
                         {
                             if( !leaders.contains( *iter ) )
                                 {
                                     leadersToBlock[ *iter ] = new image_basicBlock(this, *iter);
                                     leaders += *iter;
                                     jmpTargets.push_back( *iter );
                                     blockList.push_back( leadersToBlock[ *iter] );
                                 }                        
                             currBlk->addTarget( leadersToBlock[ *iter ] );
                             leadersToBlock[ *iter ]->addSource( currBlk );
                         }
                     iter++;
                 }

                 if (leavesFunc) {
                     currBlk->isExitBlock_ = true;
                     // And make an inst point
                     p = new image_instPoint(currAddr, 
                                             ah.getInstruction(),
                                             this,
                                             functionExit);
                     parsing_printf("Function exit (2) at 0x%x\n", *ah);
                     funcReturns.push_back(p);
                 }
                 
                 break;
             }             
             else if( ah.isAReturnInstruction() ||
                      !ah.getInstruction().valid())
             {
                 parsing_printf("ret or invalid\n");

                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = currAddr + insnSize;
                 currBlk->isExitBlock_ = true;
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                 
                 // And make an inst point
                 if (ah.isAReturnInstruction()) {
                     p = new image_instPoint(currAddr, 
                                             ah.getInstruction(),
                                             this,
                                             functionExit);
                     //parsing_printf("Function exit at 0x%x\n", *ah);
                     funcReturns.push_back(p);
                 }
                 else {
                     // Uhh....
                 }

                 break;

             }
             else if( ah.isAJumpInstruction() )
             {
                 parsing_printf("branch\n");

                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = currAddr + insnSize;
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                
                 Address target = ah.getBranchTargetAddress();

		 if( (target < funcBegin) ||
		     (target >= funcBegin + size_)) {
                     currBlk->isExitBlock_ = true;
                     // And make an inst point
                     p = new image_instPoint(currAddr, 
                                             ah.getInstruction(),
                                             this,
                                             functionExit);
                     funcReturns.push_back(p);
                 }
                 else
                 {
                     jmpTargets.push_back( target );
                     //check if a basicblock object has been 
                     //created for the target
                     if( !leaders.contains( target ) )
                     {
                         //if not, then create one
                         leadersToBlock[ target ] = new image_basicBlock(this, target);
                         leaders += target;
                         blockList.push_back( leadersToBlock[ target ] );
                     }                     
                     leadersToBlock[ target ]->addSource( currBlk );
                     currBlk->addTarget( leadersToBlock[ target ] );
                 }                 
                 break;                 
             }
             else if( ah.isACallInstruction() || 
                      ah.isADynamicCallInstruction())
             {
                 parsing_printf("call branch\n");
                 if (ah.isADynamicCallInstruction()) {
                     p = new image_instPoint(currAddr,
                                             ah.getInstruction(),
                                             this,
                                             0, 
                                             true);
                     calls.push_back(p);
                 }
                 else {

                     Address callTarget = ah.getBranchTargetAddress();
                     if (callTarget == 0) {
                         // Call to self; skip
                     }
                     else if (img()->isValidAddress(callTarget)) {
                         
                         // We have the annoying "call to return" combo like
                         // on x86. Those aren't calls, and we handle them
                         // in instrumentation
                         // Grab the insn at target
                         codeBuf_t *target =
                             (codeBuf_t *)img()->getPtrToOrigInstruction(callTarget);
                         instruction callTargetInsn;
                         callTargetInsn.setInstruction(target);
                         if (((*callTargetInsn).raw & 0xfffff000) == 0x81c3e000) {
                             parsing_printf("Skipping call to retl at 0x%x, func %s\n",
                                     *ah, symTabName().c_str());
                         }
                         else {
                             // Should check if all target is inside function...
                             parsing_printf("Call target 0x%x, func 0x%x to 0x%x\n",
                                     callTarget, getOffset(), getOffset()+getSize());
                             if (callTarget < getOffset() ||
                                 callTarget >= getOffset() + getSize()) {
                                 p = new image_instPoint(currAddr,
                                                         ah.getInstruction(),
                                                         this,
                                                         ah.getBranchTargetAddress(),
                                                         false);
                                 calls.push_back(p);
                             }
                             else {
                                 parsing_printf("Skipping call inside function at 0x%x, %s\n",
                                         *ah, symTabName().c_str());
                             }
                         }
                     }
                 }
             }
            ah++;
        }                   
    }
    
    cleanBlockList();

    
    size_ = funcEnd - funcBegin;
    
    isInstrumentable_ = true;
    return true;
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#if 0
// Old linear method; moving to basic-block-based.


/*
 * Find the instPoints of this function.
 */
bool int_function::findInstPoints(const image *owner) {
  parsed_ = true;

   Address firstAddress = get_address();
   Address lastAddress = get_address() + get_size();
   Address adr;
   Address target;
   Address entry;
   Address disp;

   instruction instr; 
   instruction nexti;

   instPoint *point = 0;

   needs_relocation_ = false;
   mayNeedRelocation_ = false;
   canBeRelocated_ = true;

   // Initially assume function has no stack frame 
   noStackFrame = true;

   // variables for function parameters
   const instPoint *blah = 0;
  
   bool dummyParam;

   // Ids for instPoints
   unsigned retId = 0;
   unsigned callsId = 0; 


   bool checkPoints; 

   if (get_size() == 0) {
      goto set_uninstrumentable;
   }

   instr.raw = owner->get_instruction(firstAddress);
   if (!IS_VALID_INSN(instr)) {
      goto set_uninstrumentable;
   }
   // Determine if function needs to be relocated when instrumented
   for ( adr = firstAddress; adr < lastAddress; adr += 4) { 
      instr.raw = owner->get_instruction(adr);
      nexti.raw = owner->get_instruction(adr+4);

      // If there's an TRAP instruction in the function, we assume
      // that it is an system call and will relocate it to the heap
      if (isInsnType(instr, TRAPmask, TRAPmatch)) {
         needs_relocation_ = true;
      } 

      // TODO: This is a hacking for the solaris(solaris2.5 actually)
      // We will relocate that function if the function has been 
      // tail-call optimazed.
      // (Actully, the reason of this is that the system calls like 
      //  read, write, etc have the tail-call optimazation to call
      //  the _read, _write etc. which contain the TRAP instruction 
      //  This is only done if libc is statically linked...if the
      //  libTag is set, otherwise we instrument read and _read
      //  both for the dynamically linked case
      // New for Solaris 2.6 support - new form of tail-call opt-
      //  imization found:
      //   jmp %register
      //   nop
      //  as last 2 instructions in function which does not have
      //  own register frame.
      if (CallRestoreTC(instr, nexti) || 
          JmpNopTC(instr, nexti, adr, this) ||
          MovCallMovTC(instr, nexti)) {
         needs_relocation_ = true;
      }

      // if call is directly to a retl, this is not a real call, but
      // is instead used to set the o7 register. Set the function to be
      // relocated when instrumented.
      if (isCallInsn(instr)) {

         // find target address of call
         disp = instr.call.disp30 << 2;
         target = adr + disp;

         // get target instruction of the call   
         instruction tmpInsn;
         tmpInsn.raw = owner->get_instruction( target );

         if((tmpInsn.raw & 0xfffff000) == 0x81c3e000) {
            mayNeedRelocation_ = true;
         }

         // if this is a call instr to a location within the function, 
         // and if the offest is 8 then this is used to set the o7 register
         // with the pc and we may need to relocate the function 
         if ( is_set_O7_call(instr, get_size(), adr - firstAddress)) {
            mayNeedRelocation_ = true;
         }
      }
   }


   /* FIND FUNCTION ENTRY */


   entry = firstAddress;
   for (adr = firstAddress; adr < lastAddress; adr += 4) { 

      // The function Entry is defined as the first SAVE instruction plus
      // the instructions after this.
      // ( The first instruction for the nonleaf function is not 
      //   necessarily a SAVE instruction. ) 
      instr.raw = owner->get_instruction(adr);

      if (isInsnType(instr, SAVEmask, SAVEmatch)) {
         entry = adr;
         noStackFrame = false;
         continue;
      }
   }


   // If there's no SAVE instruction found, this is a leaf function
   // and function Entry will be defined from the first instruction
   if (noStackFrame) {

      // noStackFrame, apparently leaf function
      adr = firstAddress;
      entry = adr;
   }


   /* CHECK IF FUNCTION SHOULD NOT BE RELOCATED WHEN INSTRUMENTED */

   // FUNCTION TOO SMALL
   if (get_size() <= 3*instruction::size()) {
      canBeRelocated_ = false;
   }


   // Can't handle function
   if (canBeRelocated_ == false && needs_relocation_ == true) {
      goto set_uninstrumentable;
   }    
#ifdef BPATCH_LIBRARY
   if (BPatch::bpatch->hasForcedRelocation_NPInt()) {
      if (canBeRelocated_ == true) {
         needs_relocation_ = true;
      }
   }
#endif

   o7_live = false;

   /* CREATE ENTRY INSTPOINT */
   instr.raw = owner->get_instruction(entry);

   funcEntry_ = new instPoint(this, entry, true, functionEntry);
   assert(funcEntry_);

   // ITERATE OVER INSTRUCTIONS, locating instPoints
   adr = firstAddress;

   instructions = new instruction[ get_size() / instruction::size() ];
 
   for (int i=0; adr < lastAddress; adr += instruction::size(), i++) {

      instr.raw = owner->get_instruction(adr);
      instructions[i] = instr;
      nexti.raw = owner->get_instruction(adr+4);

      if(!o7_live){
         InsnRegister rd,rs1,rs2;
         get_register_operands(instr,&rd,&rs1,&rs2);

         if(rs1.is_o7() || rs2.is_o7())// ||
            // (rd.is_o7() && 
            //  ((instr.raw & 0xc1f80000) != 0x81c00000))) /*indirect call*/
            o7_live = true;
      }

      // check for return insn and as a side, decide if we are at the
      // end of the function.
      if (isReturnInsn(owner, adr, dummyParam, prettyName())) {
         // define the return point

         instPoint *point;
         point = new instPoint(this, adr, false, functionExit);
         funcReturns.push_back(point);
         funcReturns[retId] -> instId = retId; retId++;
      } 
    
      else if (instr.branch.op == 0      
               &&  (instr.branch.op2 == 2 || instr.branch.op2 == 6) 
               && (instr.branch.cond == 0 || instr.branch.cond == 8)) {

         // find if this branch is going out of the function
         disp = instr.branch.disp22;
         Address target = adr +  (disp << 2);
         
         if (target < firstAddress || target >= lastAddress) {

            instPoint *point;
            point = new instPoint(this, adr, false, functionExit);

            if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  

               point->isBranchOut = true;
               point->branchTarget = target;
            }

            funcReturns.push_back(point);
            funcReturns[retId] -> instId = retId; retId++;

         }
      } 
    
      else if (isCallInsn(instr)) {

         // if the call target is the address of the call instruction
         // then this is not something that we can instrument...
         // this occurs in functions with code that is modifined when 
         // they are loaded by the run-time linker, or when the .init
         // section is executed.  In this case the instructions in the
         // parsed image file are different from the ones in the executable
         // process.
         Address call_target = adr + (instr.call.disp30 << 2);
         if(instr.call.op == CALLop) { 
            if(call_target == adr){ 
               cerr << "WARN : function " << prettyName().c_str()
                    << " has call to same location as call, NOT instrumenting"
                    << endl;
               goto set_uninstrumentable;
            }
         }

         // first, check for tail-call optimization: a call where the 
         // instruction in the delay slot write to register %o7(15), usually 
         // just moving the caller's return address, or doing a restore
         // Tail calls are instrumented as return points, not call points.

         if (CallRestoreTC(instr, nexti) || MovCallMovTC(instr, nexti)) {

            // generate a call instPoint for the call instruction
            point = new instPoint(this, adr, false, callSite);

            // record the call instPoint in pdFunction's calls vector 
            addCallPoint(instr, callsId, 0, point, blah);
  
            // generate a functionExit instPoint for the tail-call sequence  
            // (mark the instruction in the delay slot of the call as the exit)
            point = new instPoint(this, adr, false, functionExit);
            funcReturns.push_back(point);
            funcReturns[retId] -> instId = retId; retId++;

         } else {

            // check if the call is to inside the function - if definately
            // inside function (meaning that thew destination can be determined
            // statically because its a call to an address, not to a register 
            // or register + offset) then don't instrument as call site, 
            // otherwise (meaning that the call destination is known statically 
            // to be outside the function, or is not known statically), then 
            // instrument as a call site....
            enum fuzzyBoolean is_inst_point;
            is_inst_point = is_call_outside_function(instr, firstAddress, 
                                                     adr, get_size());
            if (is_inst_point == eFalse) {

               // if this is a call instr to a location within the function, 
               // and if the offest is not 8 then do not define this function 
               if (!is_set_O7_call(instr, get_size(), adr - firstAddress)) {
                  goto set_uninstrumentable;
               }

               // generate a call instPoint for the call instruction
               point = new instPoint(this, adr, false, callSite);

               // record the call instPoint in pdFunction's calls vector 
               addCallPoint(instr, callsId, 0, point, blah);

            } else {
 
               // get call target instruction   
               Address call_target = adr + (instr.call.disp30 << 2);
               instruction tmpInsn;
               tmpInsn.raw = owner->get_instruction( call_target );

               // check that call is not directly to a retl instruction,
               // and thus a real call
               if((tmpInsn.raw & 0xfffff000) != 0x81c3e000) {

                  // generate a call instPoint for the call instruction
                  point = new instPoint(this, adr, false, callSite);

                  // record the call instPoint in pdFunction's calls vector 
                  addCallPoint(instr, callsId, 0, point, blah);

               } 
            }         
         }
      }

      else if (JmpNopTC(instr, nexti, adr, this)) {

         // generate a call instPoint for the jump instruction
         point = new instPoint(this, adr, false, callSite);

         // record the call instPoint in pdFunction's calls vector 
         addCallPoint(instr, callsId, 0, point, blah);

         // generate a functionExit instPoint for the tail-call sequence  
         // (mark the instruction in the delay slot of the jump as the exit)
         point = new instPoint(this, adr, false, functionExit);
         funcReturns.push_back(point);
         funcReturns[retId] -> instId = retId; retId++;
      }

      else if (isInsnType(instr, JMPLmask, JMPLmatch)) {
         /* A register indirect jump. Some jumps may exit the function 
            (e.g. read/write on SunOS). In general, the only way to 
            know if a jump is exiting the function is to instrument
            the jump to test if the target is outside the current 
            function. Instead of doing this, we just check the 
            previous two instructions, to see if they are loading
            an address that is out of the current function.
            This should catch the most common cases (e.g. read/write).
            For other cases, we would miss a return point.

            This is the case considered:

            sethi addr_hi, r
            or addr_lo, r, r
            jump r
         */

         Register jumpreg = instr.rest.rs1;
         instruction prev1;
         instruction prev2;

         prev1.raw = owner->get_instruction(adr-4);
         prev2.raw = owner->get_instruction(adr-8);

         Address targetAddr;

         if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
             && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
             && prev2.sethi.rd == (unsigned)jumpreg
             && prev1.rest.op == RESTop 
             && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
             && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg) {

            targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
            targetAddr |= prev1.resti.simm13;

            if ( (targetAddr < firstAddress) || (targetAddr >= lastAddress) ){

               instPoint *point;
               point = new instPoint(this, adr, false, functionExit);

               funcReturns.push_back(point);
               funcReturns[retId] -> instId = retId; retId++;
            }
         }
      }
   }


   // For leaf functions, beware of exit points with a DCTI just prior.
   // If we can't use a branch instruction (i.e. ba,a) at the exit, we 
   // will need to relocate the function.  
   if ( hasNoStackFrame() ) {

      // Check each return point
      for (unsigned j=0; j < funcReturns.size(); j++) {

         // Leaf exits always require prior instructions to instrument with
         // a save; call; restore; sequence
         assert(funcReturns[j]->usesPriorInstructions);

         // Check if the first prior instruction is in the delay slot of a
         // DCTI, or is an aggregate for a DCTI
         if ( (funcReturns[j]->secondPriorIsDCTI) ||
              (funcReturns[j]->thirdPriorIsDCTI && 
               funcReturns[j]->firstPriorIsAggregate) ) {

            mayNeedRelocation_ = true;
            continue;
         }
      }


      // Check for branches to the exit point. If there is such a branch,
      // we will need to relocate the function and add nops after exit point
      // instead of claiming prior instructions
      Address insnAddr    = firstAddress;
      int num_instructions = get_size() / instruction::size();
 
      // Iterate over all instructions
      for (int j=0; j < num_instructions; j++) {

         // Grab the instruction
         instr = instructions[j];

         // Check if the instruction is a branch
         if ( instr.branch.op == 0 ) {

            // find if this branch is going to an exit point
            int displacement = instr.branch.disp22;
            Address target   = insnAddr + (displacement << 2);

            // Check each return point address
            for (unsigned k=0; k < funcReturns.size(); k++) {

               // Check if the branch target matches the exit point address
               if (target == funcReturns[k]->pointAddr()) {
                  mayNeedRelocation_ = true;
                  continue;
               }
            }
         }

         insnAddr += instruction::size();
      }
   }

   checkPoints = checkInstPoints(owner);

   if ( (checkPoints == false) || (!canBeRelocated_ && needs_relocation_) ){
      goto set_uninstrumentable;
   }

   isInstrumentable_ = 1;
   return true;

 set_uninstrumentable:
      isInstrumentable_ = 0;
   return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Check all the instPoints within this function to see if there's 
 * any conficts happen.
 */
bool int_function::checkInstPoints(const image *owner) {
   // Our own library function, skip the test.
   if (prettyName().prefixed_by("DYNINST")) 
      return true;

#ifndef BPATCH_LIBRARY /* XXX Users of libdyninstAPI might not agree. */
   // The function is too small to be worth instrumenting.
   if (get_size() <= 12){
      return false;
   }
#endif

   // No function return! return false;
   if (sizeof(funcReturns) == 0) {
      return false;
   }

   instruction instr;
   Address adr = getAddress(0);

   bool retl_inst = false;
   bool restore_inst = false;
   // Check if there's any branch instruction jump to the middle
   // of the instruction sequence in the function entry point
   // and function exit point.
   for ( ; adr < getAddress(0) + get_size(); adr += instruction::size()) {

      instr.raw = owner->get_instruction(adr);
      if(isInsnType(instr, RETLmask, RETLmatch)) retl_inst = true;
      if(isInsnType(instr, RESTOREmask, RESTOREmatch)) restore_inst = true;
      if (isInsnType(instr, BRNCHmask, BRNCHmatch)||
          isInsnType(instr, FBRNCHmask, FBRNCHmatch)) {
        
         int disp = instr.branch.disp22;
         Address target = adr + (disp << 2);
        
         if ((target > funcEntry_->pointAddr())&&
             (target < (funcEntry_->pointAddr() + funcEntry_->size))) {
            if (adr > (funcEntry_->pointAddr()+funcEntry_->size)){
                
               // function can be instrumented if we relocate it
                
               needs_relocation_ = true;
            }
         }
        
         for (u_int i = 0; i < funcReturns.size(); i++) {
            if ((target > funcReturns[i]->pointAddr())&&
                (target < (funcReturns[i]->pointAddr() +funcReturns[i]->size)))
            {
               if ((adr < funcReturns[i]->pointAddr())||
                   (adr > (funcReturns[i]->pointAddr() +funcReturns[i]->size)))
               {
                    
                  // function can be instrumented if we relocate it
                    
                  needs_relocation_ = true; 
               }
            }
         }
      }
   }

   // if there is a retl instruction and we don't think this is a leaf
   // function then this is a way messed up function...well, at least we
   // we can't deal with this. Lots of examples in libthread.so, which seem to
   // be assembly wrappers for some libc functions (e.g., creat)
   if(retl_inst && !noStackFrame && !restore_inst){ 
      //cerr << "WARN : function " << prettyName().c_str()
      //     << " retl instruction in non-leaf function, can't instrument"
      //      << endl;
      return false;
   }

   // check that no instrumentation points could overlap
   Address func_entry = funcEntry_->pointAddr() + funcEntry_->size; 
   for (u_int i = 0; i < funcReturns.size(); i++) {
      if(func_entry >= funcReturns[i]->pointAddr()){

         // function can be instrumented if we relocate it 
         needs_relocation_ = true; 
      }
      if(i >= 1) { // check if return points overlap
         Address prev_exit = funcReturns[i-1]->pointAddr() +
                             funcReturns[i-1]->size;  
         if(funcReturns[i]->pointAddr() < prev_exit) {
            // function can be instrumented if we relocate it 
            needs_relocation_ = true;
         } 
      }
   }

   return true;	
}

#endif


