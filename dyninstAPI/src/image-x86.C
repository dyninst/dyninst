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

/*
 * inst-x86.C - x86 dependent functions and code generator
 * $Id: image-x86.C,v 1.27 2008/02/18 19:14:44 bill Exp $
 */

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "InstrucIter.h"
#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>
#include "arch.h"

/**************************************************************
 *
 *  machine dependent methods of pdFunction
 *
 **************************************************************/

void checkIfRelocatable(instruction insn, bool &canBeRelocated) {
  const unsigned char *instr = insn.ptr();

  // Check if REG bits of ModR/M byte are 100 or 101 (Possible jump 
  // to jump table).
  if (instr[0] == 0xFF && 
     ( ((instr[1] & 0x38)>>3) == 4 || ((instr[1] & 0x38)>>3) == 5 )) {

    // function should not be relocated
    canBeRelocated = false;
  }
}

bool image_func::archIsRealCall(InstrucIter &ah, bool &validTarget,
                                bool & /* simulateJump */)
{
    instruction insn = ah.getInstruction();
    Address adr = *ah;

   // Initialize return value
   validTarget = true;
   //parsing_printf("*** Analyzing call, offset 0x%x target 0x%x\n",
   //adr, insn.getTarget(adr));
   // calls to adr+5 are not really calls, they are used in 
   // dynamically linked libraries to get the address of the code.
   if (insn.getTarget(adr) == adr + 5) {
       parsing_printf("... getting PC\n");
       // XXX we can do this like on sparc, but we don't need to: we do it
       // on sparc because the intervening instructions don't get executed;
       // on x86 for this heuristic there *are* no intervening instructions.
       //simulateJump = true;
       return false;
   }

   // Calls to a mov instruction followed by a ret instruction, where the 
   // source of the mov is the %esp register, are not real calls.
   // These sequences are used to set the destination register of the mov 
   // with the pc of the instruction instruction that follows the call.

   // This sequence accomplishes this because the call instruction has the 
   // side effect of placing the value of the %eip on the stack and setting the
   // %esp register to point to that location on the stack. (The %eip register
   // maintains the address of the next instruction to be executed).
   // Thus, when the value at the location pointed to by the %esp register 
   // is moved, the destination of the mov is set with the pc of the next
   // instruction after the call.   

   //    Here is an example of this sequence:
   //
   //       mov    %esp, %ebx
   //       ret    
   //
   //    These two instructions are specified by the bytes 0xc3241c8b
   //

   Address targetOffset = insn.getTarget(adr);
 
   if ( !img()->isValidAddress(targetOffset) ) {
       parsing_printf("... Call to 0x%x is invalid (outside code or data)\n",
       targetOffset);
       validTarget = false;
       return false;
   }    

   // Get a pointer to the call target
   const unsigned char *target =
      (const unsigned char *)img()->getPtrToInstruction(targetOffset);

   // The target instruction is a  mov
   if (*(target) == 0x8b) {
      // The source register of the mov is specified by a SIB byte 
      if (*(target + 1) == 0x1c || *(target + 1) == 0x0c) {

         // The source register of the mov is the %esp register (0x24) and 
         // the instruction after the mov is a ret instruction (0xc3)
         if ( (*(target + 2) == 0x24) && (*(target + 3) == 0xc3)) {
            return false;
         }
      }
   }

   return true;
}

// Determine if the called function is a "library" function or a "user"
// function This cannot be done until all of the functions have been seen,
// verified, and classified
// 
// DELAYED UNTIL PROCESS SPECIALIZATION
//






/********************************************************/
/* Architecture dependent parsing support methods       */
/********************************************************/

bool image_func::archCheckEntry( InstrucIter &ah, image_func *func )
{
    instruction insn = ah.getInstruction();
    Address offset = *ah;

  // check if the entry point contains another point
  if (insn.isJumpDir()) 
    {
      Address target = insn.getTarget(offset);
      func->img()->addJumpTarget(target);
      
      return false;
    } 
  else if (insn.isReturn()) 
    {
      // this is an empty function
      return false;
    } 
  else if (insn.isCall()) 
    {
        // So?
        return true;
    }
  return true;
}

// Architecture-specific a-priori determination that we can't
// parse this function.
bool image_func::archIsUnparseable()
{
    if( !isInstrumentableByFunctionName() )
    {   
        if (!isInstrumentableByFunctionName())
            parsing_printf("... uninstrumentable by func name\n");

        //endOffset_ = startOffset_;
        endOffset_ = getOffset();
        instLevel_ = UNINSTRUMENTABLE; 
        return true;
    }           
    else
        return false;
}

// Architecture-specific hack to give up happily on parsing a
// function.
bool image_func::archAvoidParsing()
{
    //temporary convenience hack.. we don't want to parse the PLT as a function
    //but we need pltMain to show up as a function
    //so we set size to zero and make sure it has no instPoints.    
    if( prettyName() == "DYNINST_pltMain" )//|| 
        //prettyName() == "winStart" ||
        //prettyName() == "winFini" )
    {   
        //endOffset_ = startOffset_;
        endOffset_ = getOffset();
        return true;
    }
    else
        return false;
}

void image_func::archGetFuncEntryAddr(Address & /* funcEntryAddr */)
{
    return;
}

// Architecture-specific hack to prevent relocation of certain functions.
bool image_func::archNoRelocate()
{   
    return prettyName() == "__libc_start_main";
}

// Nop on x86
void image_func::archSetFrameSize(int /* frameSize */)
{
    return;
}

void image_func::archInstructionProc(InstrucIter & /* ah */)
{
    return;
}

bool findMaxSwitchInsn(image_basicBlock *start, instruction &maxSwitch,
        instruction &branchInsn)
{
    BPatch_Set<image_basicBlock *> visited;
    pdvector<image_basicBlock *> WL;
    pdvector<image_edge *> sources;
    image_basicBlock *curBlk;

    bool foundMaxSwitch = false;
    bool foundCondBranch = false;

    WL.push_back(start);

    for(unsigned j=0;j < WL.size(); j++)
    {
        curBlk = WL[j];
        visited.insert(curBlk);

        foundMaxSwitch = false;
        foundCondBranch = false;
    
        InstrucIter iter( curBlk );
        while( *iter < curBlk->endOffset() ) {
            // check for cmp 
            if( iter.getInstruction().isCmp() )
            {
                parsing_printf("Found jmp table cmp instruction at 0x%lx\n",
                                *iter);
                maxSwitch = iter.getInstruction();
                foundMaxSwitch = true;
            }
            if( iter.getInstruction().type() & IS_JCC ) {
                parsing_printf("Found jmp table cond br instruction at 0x%lx\n",
                    *iter);
                branchInsn = iter.getInstruction();
                foundCondBranch = true;
                break;
            }
            iter++;
        }

        if(foundMaxSwitch && foundCondBranch) {
            // done
            break; 
        } else {
            // look further back
            sources.clear();
            curBlk->getSources( sources );
            for(unsigned i=0;i<sources.size();i++)
            {
                if(sources[i]->getType() == ET_CALL)
                    continue;

                image_basicBlock * src = sources[i]->getSource();
                if( !visited.contains( src ) ) {
                    WL.push_back(src);
                }
            }
        }
    }
    WL.zap();
    return foundMaxSwitch && foundCondBranch;
}

bool isNonCallEdge(image_edge* e)
{
  return e->getType() != ET_CALL;
}
bool leadsToVisitedBlock(image_edge* e, const std::set<image_basicBlock*>& visited)
{
  image_basicBlock* src = e->getSource();
  return visited.find(src) != visited.end();
}

struct copyToWorkList
{
  copyToWorkList(std::deque<image_basicBlock*>& wl, 
		      const std::set<image_basicBlock*>& v) :
  worklist(wl), visited(v)
  {
  }
  std::deque<image_basicBlock*>& worklist;
  const std::set<image_basicBlock*>& visited;
  void operator()(image_edge* e)
  {
    if(isNonCallEdge(e) && !leadsToVisitedBlock(e, visited))
    {
      worklist.push_back(e->getSource());
    }
  }
};


bool findThunkInBlock(image_func* f, image_basicBlock* curBlock, Address& thunkOffset)
{
  InstrucIter ah(curBlock);
  bool isValidTarget, dummy;
  
  while(ah.hasMore())
  {
    if(ah.isACallInstruction() && !f->archIsRealCall(ah, isValidTarget, dummy))
    {
      ah++;
      thunkOffset = *ah;
      ia32_memacc mac[3];
      ia32_locations loc;
      ia32_condition cond;
      ia32_instruction itmp(mac, &cond, &loc);
      ia32_decode(IA32_FULL_DECODER, (unsigned char*)(ah.getInstruction().ptr()), itmp);
      int immediate;
      if(itmp.getEntry()->id == e_add)
      {
	switch(loc.imm_size)
	{
	case 4:
	  immediate = *(int*)(ah.getInstruction().ptr() + loc.imm_position);
	  thunkOffset += immediate;
	  break;
	case 1:
	  immediate = *(char*)(ah.getInstruction().ptr() + loc.imm_position);
	  thunkOffset += immediate;
	  break;
	default:
	  thunkOffset = 0;
	  break;
	}
      }
    }
    else // Look for an AMD64 IP-relative LEA.  If we find one, it should point to the start of a
    {    // relative jump table.

      ia32_memacc mac[3];
      ia32_locations loc;
      ia32_condition cond;
      ia32_instruction itmp(mac, &cond, &loc);
      ia32_decode(IA32_FULL_DECODER, (unsigned char*)(ah.getInstruction().ptr()), itmp);

      if(itmp.getEntry()->id == e_lea && 
	 loc.modrm_rm == 0x05 && loc.modrm_mod == 0x00) // IP-relative
      {
	int immediate;
	switch(loc.disp_size)
	{
	case 4:
	  immediate = *(int*)(ah.getInstruction().ptr() + loc.disp_position);
	  ah++;
	  thunkOffset = *ah + immediate;
	  break;
	case 1:
	  immediate = *(char*)(ah.getInstruction().ptr() + loc.disp_position);
	  ah++;
	  thunkOffset = *ah + immediate;
	  break;
	default:
	  thunkOffset = 0;
	  break;	  
	  }
      }
    }
    ah++;
  }
  return false;
}


bool findThunkAndOffset(image_func* f, image_basicBlock* start, Address& thunkOffset)
{
  std::set<image_basicBlock*> visited;
  std::deque<image_basicBlock*> worklist;
  pdvector<image_edge*> sources;
  image_basicBlock* curBlock;
  worklist.insert(worklist.begin(), start);
  while(!worklist.empty())
  {
    curBlock = worklist.front();
    worklist.pop_front();
    visited.insert(curBlock);
    if(findThunkInBlock(f, curBlock, thunkOffset))
    {
      return true;
    }
    else
    {
      sources.clear();
      curBlock->getSources(sources);
      std::for_each(sources.begin(), sources.end(), copyToWorkList(worklist, visited));
    }
  }
  return false;
}


// Very complicated for x86. Look for a jump table in the blocks preceeding
// this block, and extract targets from it if found or mark this function
// unrelocatable if it was not found or not understood.
bool image_func::archGetMultipleJumpTargets( 
                                BPatch_Set< Address >& targets,
                                image_basicBlock * currBlk,
                                InstrucIter &ah,
                                pdvector< instruction >& allInstructions)
{

    //we are going to get the instructions to parse the 
    //jump table from my source block(s)
    pdvector< image_edge* > in;
    currBlk->getSources( in );

    if( in.size() < 1 )
    {
      return false;
    }
    else {
        instruction tableInsn = ah.getInstruction();
        // Obviously, since the indirect branch brought us here
        instruction branchInsn = ah.getInstruction();
        instruction maxSwitch;

        bool isAddInJmp = true;
        
        int j = allInstructions.size() - 2;
        assert(j > 0);
        
        const unsigned char* ptr = ah.getInstruction().op_ptr();
	const unsigned char LEA = 0x8D;
	
        assert( *ptr == 0xff );
        ptr++;
        if( (*ptr & 0xc7) != 0x04) // if not SIB
            {
                isAddInJmp = false;
                //jump via register so examine the previous instructions 
                //in current block to determine the register value
                bool foundTableInsn = false;
                
                InstrucIter findReg(ah);

                while(findReg.hasPrev()) {
                    findReg--;
                    parsing_printf("Checking 0x%lx for register...\n", *findReg);
                    if ((*findReg.getInstruction().op_ptr()) == MOVREGMEM_REG || (*findReg.getInstruction().op_ptr()) == LEA) {
                        tableInsn = findReg.getInstruction();
                        foundTableInsn = true;
                        parsing_printf("Found register at 0x%lx\n", *findReg);
                        break;    
                    }
                }
	    }
	

        // search backward over the blocks that reach this one. we're looking
        // for a comparison on this register. if we find an assignment to the
        // register but don't find the comparison, we give up on this jump
        // table.
        bool foundMaxSwitch = findMaxSwitchInsn(currBlk, maxSwitch, branchInsn);
	Address thunkOffset = 0;
	bool foundThunk = findThunkAndOffset(this, currBlk, thunkOffset);
	
        if( !foundMaxSwitch ) {
	  parsing_printf("... unable to fix max switch size\n");
	  return false;
        }
        //found the max switch assume jump table
        else {
	  if( !ah.getMultipleJumpTargets( targets, tableInsn, 
					  maxSwitch, branchInsn, isAddInJmp, thunkOffset ))
	  {
	    return false;
	  }
	  else
	  {
	    return targets.size() > 0;
	  }
	}
    }
}

bool image_func::archProcExceptionBlock(Address &catchStart, Address a)
{
    ExceptionBlock b;
//    Symtab obj = img()->getObject();
    if (img()->getObject()->findCatchBlock(b,a)) {
        catchStart = b.catchStart();
        return true;
    } else {
        return false;
    }
}

bool image_func::archIsATailCall(InstrucIter &ah, 
                                 pdvector< instruction >& allInstructions)
{
    unsigned numInsns = allInstructions.size() - 2;
    Address target = ah.getBranchTargetAddress();

    if( img()->findFuncByEntry( target ) ||
        ( *allInstructions[ numInsns ].ptr() == POP_EBP ||
        allInstructions[ numInsns ].isLeave() ))
    {
        return true;
    }
    else
        return false;
}

bool image_func::archIsIndirectTailCall(InstrucIter &ah)
{
  if(!ah.peekPrev()) return false;
  InstrucIter temp(ah);
  temp--;
  return (*temp.getInstruction().op_ptr()) == POP_EBX;
}

bool image_func::archIsAbortOrInvalid(InstrucIter &ah)
{
    return ah.isAnAbortInstruction();
}

bool image_func::writesFPRs(unsigned level) {
    // Oh, we should be parsed by now...
    if (!parsed_) image_->analyzeIfNeeded();

    if (containsFPRWrites_ == unknown) {
        // Iterate down and find out...
        // We know if we have callees because we can
        // check the instPoints; no reason to iterate over.
        // We also cache callee values here for speed.

        if (level >= 3) return true; // Arbitrarily decided level 3 iteration.
        
        for (unsigned i = 0; i < calls.size(); i++) {
            if (calls[i]->func()) {
                if (calls[i]->func()->writesFPRs(level+1)) {
                    // One of our kids does... if we're top-level, cache it; in 
                    // any case, return
                    if (level == 0)
                        containsFPRWrites_ = used;
                    return true;
                }
            }
            else {
                // Indirect call... oh, yeah. 
                if (level == 0)
                    containsFPRWrites_ = used;
                return true;
            }
        }

        // No kids contain writes. See if our code does.
        InstrucIter ah(this);
        while (ah.hasMore()) {
            if (ah.isFPWrite()) {
                containsFPRWrites_ = used;
                return true;
            }
            ah++;
        }

        // No kids do, and we don't. Impressive.
        containsFPRWrites_ = unused;
        return false;
    }
    else if (containsFPRWrites_ == used)
        return true;
    else if (containsFPRWrites_ == unused)
        return false;

    fprintf(stderr, "ERROR: function %s, containsFPRWrites_ is %d (illegal value!)\n", 
	    symTabName().c_str(), containsFPRWrites_);
    
    assert(0);
    return false;
}
