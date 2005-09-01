/* Copyright (c) 1996-2004 Barton P. Miller
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

// $Id: image-alpha.C,v 1.3 2005/09/01 22:18:15 bernat Exp $

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
#include "alpha.h"

bool image_func::findInstPoints(pdvector<Address> & /*callTargets*/)
{
  bool gpKnown = false;
  long gpValue = 0;

    if( parsed_ ) 
    {
        parsing_printf("Error: multiple call of findInstPoints\n");
        return false;
    } 
    parsed_ = true;

    parsing_printf("findInstPoints for func %s, at 0x%lx to 0x%lx\n",
		   symTabName().c_str(), getOffset(), getEndOffset());

    makesNoCalls_ = true;
    noStackFrame = true;
    int insnSize = instruction::size();

    BPatch_Set< Address > leaders;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );
    
    Address funcBegin = getOffset();
    Address funcEnd = getEndOffset();
    
    InstrucIter ah (funcBegin, this);

    if (!ah.getInstruction().valid()) {
        isInstrumentable_ = false;
        return false;
    }

    Address funcEntryAddr = funcBegin;


    // normal linkage on alphas is that the first two instructions load gp.
    //   In this case, many functions jump past these two instructions, so
    //   we put the inst point at the third instruction.
    //   However, system call libs don't always follow this rule, so we
    //   look for a load of gp in the first two instructions.

    // If we don't do this, entry instrumentation gets skipz0red.

    // We have to manually create a basic block to represent these guys; the
    // "common" entry point is added as an in-edge, splitting the block.

    instruction firstInsn = ah.getInstruction();
    instruction secondInsn = ah.getNextInstruction();

    if (((*firstInsn).mem.opcode == OP_LDAH) && ((*firstInsn).mem.ra == REG_GP) &&
	((*secondInsn).mem.opcode == OP_LDA) && ((*secondInsn).mem.ra == REG_GP)) {
      // compute the value of the gp
      gpKnown = true;
      gpValue = ((long) funcEntryAddr) 
	+ (SEXT_16((*firstInsn).mem.disp)<<16) 
	+ (*secondInsn).mem.disp;
      funcEntryAddr += 2*instruction::size();
      ah.setCurrentAddress(funcEntryAddr);
      parsing_printf("Func %s, found entry GP pair, entry at 0x%llx\n",
		     symTabName().c_str(), 
		     funcEntryAddr);
    }
    
    image_instPoint *p;

    //define entry instpoint 
    p = new image_instPoint( funcEntryAddr,
                             ah.getInstruction(),
                             this,
                             functionEntry);
    funcEntries_.push_back(p);
    
    Address currAddr = funcBegin;
    
    // Can move; so we reset
    if (ah.isStackFramePreamble(frame_size))
        noStackFrame = false;
    ah.setCurrentAddress(funcEntryAddr);
 
    //find all the basic blocks and define the instpoints for this function
    BPatch_Set< Address > visited;
    pdvector< Address > jmpTargets;

    //entry basic block
    leaders += funcBegin;
    leadersToBlock[ funcBegin ] = new image_basicBlock(this, funcBegin);
    leadersToBlock[ funcBegin ]->isEntryBlock_ = true;
    blockList.push_back( leadersToBlock[ funcBegin ] );
    jmpTargets.push_back( funcBegin );
    
    // And "actual" entry
    if (funcEntryAddr != funcBegin) {
      leaders += funcEntryAddr;
      leadersToBlock[funcEntryAddr] = new image_basicBlock(this, funcEntryAddr);
      leadersToBlock[funcEntryAddr]->isEntryBlock_ = true;
      blockList.push_back(leadersToBlock[funcEntryAddr]);
      jmpTargets.push_back(funcEntryAddr);
    }
    
    for( unsigned i = 0; i < jmpTargets.size(); i++ )
    {
        //parsing_printf("Making block iter for target 0x%x\n", jmpTargets[i]);
        InstrucIter ah( jmpTargets[ i ], this); 
        image_basicBlock* currBlk = leadersToBlock[ jmpTargets[ i ] ];

	assert(currBlk);
        assert(currBlk->firstInsnOffset() == jmpTargets[i]);

        while( true ) {
            currAddr = *ah;
            if( visited.contains( currAddr ) )
                break;
            else
                visited += currAddr;
	    /*
            parsing_printf("checking insn at 0x%x, %s + %d\n", *ah,
                    symTabName().c_str(), (*ah) - funcBegin);
	    */
            if( ah.isACondBranchInstruction() )
                {
                    parsing_printf("cond branch\n");
                    currBlk->lastInsnOffset_ = currAddr; 
                    currBlk->blockEndOffset_ = ah.peekNext();
                    
                    Address target = ah.getBranchTargetAddress();
                    // img()->addJumpTarget(target);

                    bool exit = false;

                    if( (target < funcBegin) ||
			(target >= funcEnd)) {
                        exit = true;
                    }
                    else {
                        addBasicBlock(target,
                                      currBlk,
                                      leaders,
                                      leadersToBlock,
                                      jmpTargets);
                    }                        
                    
                    Address t2 = ah.peekNext();
                    if (t2 < funcEnd) {
                        addBasicBlock(t2,
                                      currBlk,
                                      leaders,
                                      leadersToBlock,
                                      jmpTargets);
                    }
                    else {
                        exit = true;
                    }
                    if (exit) {
                        currBlk->isExitBlock_ = true;
                        // And make an inst point
                        p = new image_instPoint(currAddr, 
                                                ah.getInstruction(),
                                                this,
                                                functionExit);
                        parsing_printf("Function exit at 0x%x\n", *ah);
                        funcReturns.push_back(p);
                    }
                    break;

                }
             else if( ah.isAIndirectJumpInstruction() )
             {
	       //parsing_printf("ind branch\n");
                 //parsing_printf("Copying iter for indirect jump\n");
                 InstrucIter ah2( ah );
                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = currAddr + insnSize;
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                 
                 BPatch_Set< Address > res;
                 if (!ah2.getMultipleJumpTargets( res ))
                     isInstrumentable_ = false;

                 BPatch_Set< Address >::iterator iter;
                 iter = res.begin();
                 bool leavesFunc = false;
                 while( iter != res.end() ) {
                     if( *iter < funcBegin ) {
                         leavesFunc = true;
                     }
                     else {
			 addBasicBlock(*iter,
                                       currBlk,
                                       leaders,
                                       leadersToBlock,
                                       jmpTargets);
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
                     //parsing_printf("Function exit at 0x%x\n", *ah);
                     funcReturns.push_back(p);
                 }
                 break;
             }             
             else if( ah.isAReturnInstruction() ||
                      !ah.getInstruction().valid())
             {
	       //parsing_printf("ret or invalid\n");

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
		   //cerr << "Warning: detected null instruction, not using as exit point!" << endl;
                 }

                 break;

             }
             else if( ah.isAJumpInstruction() )
             {
	       //parsing_printf("branch\n");

                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = currAddr + insnSize;
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                
                 Address target = ah.getBranchTargetAddress();

		 if( (target < funcBegin) ||
		     (target >= funcEnd)) {
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
                     addBasicBlock(target,
                                   currBlk,
                                   leaders,
                                   leadersToBlock,
                                   jmpTargets);
                 }                 
                 break;                 
             }
             else if( ah.isACallInstruction() || 
                      ah.isADynamicCallInstruction())
             {
	       //parsing_printf("call branch\n");
                 if (ah.isADynamicCallInstruction()) {
                     p = new image_instPoint(currAddr,
                                             ah.getInstruction(),
                                             this,
                                             0, 
                                             true);
                 }
                 else {
                     p = new image_instPoint(currAddr,
                                             ah.getInstruction(),
                                             this,
                                             ah.getBranchTargetAddress(),
                                             false);
                 }
                 calls.push_back(p);
             }
            else {
	      //parsing_printf("Non-control-flow insn\n");
            }
            ah++;
        }                   
    }

    cleanBlockList();

    return true;
}


#if 0
// Obsolete

bool int_function::findInstPoints(const image *owner) 
{  
  parsed_ = true;

   Address adr = get_address();
   instruction instr;
   instruction instr2;
   long gpValue;	// gp needs signed operations
   bool gpKnown = false;
   instPoint *point;
   Address t12Value;
   bool t12Known = false;
   instruction frameRestInsn;

   frame_size = 0;
   // normal linkage on alphas is that the first two instructions load gp.
   //   In this case, many functions jump past these two instructions, so
   //   we put the inst point at the third instruction.
   //   However, system call libs don't always follow this rule, so we
   //   look for a load of gp in the first two instructions.
   instr.raw = owner->get_instruction(adr);
   instr2.raw = owner->get_instruction(adr+4);
   if ((instr.mem.opcode == OP_LDAH) && (instr.mem.ra == REG_GP) &&
       (instr2.mem.opcode == OP_LDA) && (instr2.mem.ra == REG_GP)) {
      // compute the value of the gp
      gpKnown = true;
      gpValue = ((long) adr) + (SEXT_16(instr.mem.disp)<<16) + instr2.mem.disp;
      adr += 8;
   }

   instr.raw = owner->get_instruction(adr);
   if (!IS_VALID_INSN(instr)) 
      goto set_uninstrumentable;

   funcEntry_ = new instPoint(this, instr, adr, true, functionEntry);
   assert(funcEntry_);

   // perform simple data flow tracking on t12 within a basic block.
  
   while (true) {
      instr.raw = owner->get_instruction(adr);

      bool done;

      // check for lda $sp,n($sp) to guess frame size
      if (!frame_size && ((instr.raw & 0xffff0000) == 0x23de0000)) {
         // lda $sp,n($sp)
         frame_size = -((short) (instr.raw & 0xffff));
         if (frame_size < 0) {
            // we missed the setup and found the cleanup
            frame_size = 0;
         }
      }

      // check for return insn and as a side affect decide if we are at the
      //   end of the function.
      if (isReturnInsn(owner, adr, done)) {
         // define the return point
         // check to see if adr-8 is ldq fp, xx(sp) or ldq sp, xx(sp), if so 
         // use it as the
         // address since it will ensure the activation record is still active.
         // Gcc uses a frame pointer, on others only sp is used.

         frameRestInsn.raw = owner->get_instruction(adr-8);
         if (((frameRestInsn.raw & 0xffff0000) == 0xa5fe0000) ||
             ((frameRestInsn.raw & 0xffff0000) == 0x23de0000)) {
            Address tempAddr = adr - 8;
            funcReturns.push_back(new instPoint(this, frameRestInsn,
                                               tempAddr, false, functionExit));
         } else {
            funcReturns.push_back(new instPoint(this,instr,adr,false,functionExit));
         }

         // see if this return is the last one 
         if (done) goto set_instrumentable;
      } else if (isCallInsn(instr)) {
         // define a call point
         point = new instPoint(this, instr, adr, false,callSite);

         if (isJsr(instr)) {
            Address destAddr = 0;
            if ((instr.mem_jmp.rb == REG_T12) && t12Known) {
               destAddr = t12Value;
            }
            point->callIndirect = true;
            // this is the indirect address
            point->setCallee((int_function *) destAddr);
         } else {
            point->callIndirect = false;
            point->setCallee(NULL);
         }
         calls.push_back(point);
         t12Known = false;
      } else if (isJmpType(instr) || isBranchType(instr)) {
         // end basic block, kill t12
         t12Known = false;
      } else if ((instr.mem.opcode == OP_LDQ) && (instr.mem.ra == REG_T12) &&
                 (instr.mem.rb = REG_GP)) {
         // intruction is:  ldq t12, disp(gp)
         if (gpKnown) {
            t12Value = gpValue + instr.mem.disp;
            t12Known = true;
         }
      }

      // now do the next instruction
      adr += 4;

   }

 set_instrumentable:
   isInstrumentable_ = 1;
   return true;
  
 set_uninstrumentable:
   isInstrumentable_ = 0;
   return false;
}

#endif
