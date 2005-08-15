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
 * $Id: image-x86.C,v 1.5 2005/08/15 22:20:12 bernat Exp $
 */

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "InstrucIter.h"
#include "showerror.h"


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

bool isRealCall(instruction insn, Address adr, image *img,
                bool &validTarget)
{
   // Initialize return value
   validTarget = true;
   //parsing_printf("*** Analyzing call, offset 0x%x target 0x%x\n",
   //adr, insn.getTarget(adr));
   // calls to adr+5 are not really calls, they are used in 
   // dynamically linked libraries to get the address of the code.
   if (insn.getTarget(adr) == adr + 5) {
       //parsing_printf("... getting PC\n");
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

   int targetOffset = insn.getTarget(adr);
 
   if ( !img->isValidAddress(targetOffset) ) {
       //parsing_printf("... Call to 0x%x is invalid (outside code or data)\n",
       //targetOffset);
       validTarget = false;
       return false;
   }    

   // Get a pointer to the call target
   const unsigned char *target =
      (const unsigned char *)img->getPtrToOrigInstruction(targetOffset);

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


bool checkEntry( instruction insn, Address offset, image_func *func )
{
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

//correct parsing errors that overestimate the function's size by
// 1. updating all the vectors of instPoints
// 2. updating the vector of basicBlocks
// 3. updating the function size
// 4. update the address of the last basic block if necessary
void image_func::updateFunctionEnd(Address newEnd)
{

  //update the size
  size_ = newEnd - getOffset();
  //remove out of bounds call Points
  //assumes that calls was sorted by address in findInstPoints
  for( int i = (int)calls.size() - 1; i >= 0; i-- )
    {
      if( calls[ i ]->offset() >= newEnd )
        {
	  delete calls[ i ];
	  calls.pop_back();
        }
      else 
	break;
    }
  //remove out of bounds return points
  //assumes that funcReturns was sorted by address in findInstPoints
  for( int j = (int)funcReturns.size() - 1; j >= 0; j-- )
    {
      if( funcReturns[ j ]->offset() >= newEnd )
        {
	  delete funcReturns[ j ];
	  funcReturns.pop_back();
        }
      else
	break;
    }
    //remove out of bounds basicBlocks
    //assumes blockList was sorted by start address in findInstPoints
  for( int k = (int) blockList.size() - 1; k >= 0; k-- )
      {
          image_basicBlock* curr = blockList[ k ];
          if( curr->firstInsnOffset() >= newEnd )
              {
                  //remove all references to this block from the flowgraph
                  //my source blocks should no longer have me as a target
                  pdvector< image_basicBlock* > ins;
                  curr->getSources( ins );
                  for( unsigned o = 0; o < ins.size(); o++ )
                      {
                          ins[ o ]->removeTarget(curr);
                          ins[ o ]->isExitBlock_ = true;
                      } 
                  
                  //my target blocks should no longer have me as a source 
                  pdvector< image_basicBlock* > outs;
                  curr->getTargets( outs );
                  for( unsigned p = 0; p < outs.size(); p++ )
                      {
                          outs[ p ]->removeSource(curr);
                      }                     
                  delete curr;
                  blockList.pop_back();          
              }
          else
              break;
      } 

    //we might need to correct the end address of the last basic block
    int n = blockList.size() - 1;
    if( n >= 0 && blockList[n]->endOffset() >= newEnd )
    {
        parsing_printf("Updating block end: new end of function 0x%x\n",
                       newEnd);
        blockList[n]->debugPrint();

        // TODO: Ask Laune; this doesn't look like it's doing what we want.
        image_basicBlock* blk = blockList[n];
        
        InstrucIter ah(blk);	
        while( *ah + ah.getInstruction().size() < newEnd )
            ah++;
        
        blk->lastInsnOffset_ = *ah ;
        blk->blockEndOffset_ = *ah + ah.getInstruction().size() ;
        blk->isExitBlock_ = true;
        parsing_printf("After fixup:\n");
        blk->debugPrint();
    }

    
}    

 
/*****************************************************************************
findInstpoints: uses recursive disassembly to parse a function. instPoints and
                basicBlock information is collected here. findInstpoints
                does not rely on function size information. This helps us
                to parse stripped x86 binaries.  
******************************************************************************/
bool image_func::findInstPoints( pdvector< Address >& callTargets)
{
    parsing_printf("%s: parsing for instPoints\n", symTabName().c_str());

    if (parsed_) 
    {
        fprintf(stderr, "Error: multiple call of findInstPoints\n");
        return false;
    }
    parsed_ = true;
 
    //temporary convenience hack.. we don't want to parse the PLT as a function
    //but we need pltMain to show up as a function
    //so we set size to zero and make sure it has no instPoints.    
    if( prettyName() == "DYNINST_pltMain" )//|| 
        //prettyName() == "winStart" ||
        //prettyName() == "winFini" )
    {
        size_ = 0; 
        return true;
    }
         
    pdvector< instruction > allInstructions;   
    pdvector< instPoint* > foo;
    bool canBeRelocated = true;
    image_instPoint *p;
    unsigned numInsns = 0;
    noStackFrame = true; // Initial assumption
    BPatch_Set< Address > leaders;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );
   
    int insnSize;
    Address funcBegin = getOffset();  
    
    //funcEnd set to the beginning of the function
    //will be updated if necessary at the end of each basic block  
    Address funcEnd = funcBegin; 
    Address currAddr = funcBegin;
    
    if( !isInstrumentableByFunctionName() || getSize() == 0 )
    {
        parsing_printf("... uninstrumentable by func name or size equals zero\n");
        isInstrumentable_ = false;
        return false;
    }  
    
    size_ = 0; //shouldn't need this, but better safe than sorry

    InstrucIter ah( funcBegin, this ); 
    if( !checkEntry( ah.getInstruction(), funcBegin, this ) )
    {
        if( ah.isAJumpInstruction() || ah.isACallInstruction() )
        {
            Address target = ah.getBranchTargetAddress();
            callTargets.push_back( target );
        }

        size_ = ah.getInstruction().size();
        isInstrumentable_ = false;
        //parsing_printf("Jump or call at entry of function\n");
        return false;
    }
    
    //define the entry point
    // Someday we'll do multiple entry points...
    //parsing_printf("... Creating entry instPoint at 0x%x\n", funcBegin);
    p = new image_instPoint(funcBegin, 
                            ah.getInstruction(),
                            this, 
                            functionEntry);

    funcEntries_.push_back(p);

    int frameSizeNotSet;
    if( ah.isStackFramePreamble(frameSizeNotSet) )
    {
        noStackFrame = false;
    }
    savesFP_ = ah.isFramePush();
    
    //jump table inside this function. 
    if (prettyName() == "__libc_start_main") 
    {
        canBeRelocated = false;
    }

    canBeRelocated_ = canBeRelocated;
    
    // get all the instructions for this function, and define the
    // instrumentation points.  
    BPatch_Set< Address > visited;
    pdvector< Address > jmpTargets;

    jmpTargets.push_back( funcBegin ); 
    
    leaders += funcBegin;
    leadersToBlock[ funcBegin ] = new image_basicBlock(this, funcBegin);
    leadersToBlock[ funcBegin ]->firstInsnOffset_ = funcBegin;
    leadersToBlock[ funcBegin ]->isEntryBlock_ = true;
    blockList.push_back( leadersToBlock[ funcBegin ] );

    //parsing_printf("... Adding initial basic block: 0x%x\n", funcBegin);
    for( unsigned i = 0; i < jmpTargets.size(); i++ )
    {      
      InstrucIter ah( jmpTargets[ i ], this );
        
      image_basicBlock* currBlk = leadersToBlock[ jmpTargets[ i ] ];

      //parsing_printf("... parsing block at 0x%x, first insn offset 0x%x\n", 
      //jmpTargets[i],
      //currBlk->firstInsnOffset());

      // Good idea...
      assert(currBlk->firstInsnOffset() == jmpTargets[i]);
      
      while( true  )
        {    
            currAddr = *ah;
            insnSize = ah.getInstruction().size();
#if 0
            fprintf(stderr, "... checking 0x%x (%d)\n", currAddr, currAddr - getOffset());
            for (unsigned foo = 0; foo < insnSize; foo++) {
                fprintf(stderr, "%02hhx ", *(((char *)ah.getInstruction().ptr())+foo));
            }
            fprintf(stderr, "\n");
#endif                       
            if( visited.contains( currAddr ) ) {
                parsing_printf("... already touched addr 0x%x\n", currAddr);
                break;
            }
            else 
                visited += currAddr;

            allInstructions.push_back( ah.getInstruction() );
            
            if (ah.isFrameSetup() && savesFP_)
            {
               noStackFrame = false;
            }
            
            if( ah.isACondBranchInstruction() )
            {
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = currAddr + insnSize;

                if( currAddr >= funcEnd )
                    funcEnd = currAddr + insnSize;
                
                Address target = ah.getBranchTargetAddress();
                //parsing_printf("... conditional branch at 0x%x, going to 0x%x\n", currAddr, target);
                
                img()->addJumpTarget( target );
                             
                if( target < funcBegin )
                {
                    currBlk->isExitBlock_ = true;
                }
                else {
                    addBasicBlock(target,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  jmpTargets);
                }
                
                Address t2 = currAddr + insnSize;

                addBasicBlock(t2,
                              currBlk,
                              leaders,
                              leadersToBlock,
                              jmpTargets);
                break;
            }
            else if( ah.isAIndirectJumpInstruction() ) 
            { 
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = currAddr + insnSize;
                parsing_printf("... indirect jump at 0x%x\n", currAddr);
                //if this instructions goes to a jumpTable, 
                //we retrieve the addresses from the table
                checkIfRelocatable( ah.getInstruction(), canBeRelocated );
                
                if( currAddr >= funcEnd )
                    funcEnd = currAddr + insnSize;
                
                //if register indirect then table address will be taken from
                //previous insn

                numInsns = allInstructions.size() - 2;
                if( numInsns == 0 )
                {
                   // this "function" is a single instruction long
                   // (a jmp to the "real" function)
                    parsing_printf("... uninstrumentable due to 0 size\n");
                    isInstrumentable_ = false;
                    return false;
                }

                if (ah.peekPrev() &&
                    (*ah.getPrevInstruction().op_ptr()) == POP_EBX) {

                    //this looks like a tail call
                    currBlk->isExitBlock_ = true;
                    break;
                }
                
                //we are going to get the instructions to parse the 
                //jump table from my source block(s)
                pdvector< image_basicBlock* > in;
                currBlk->getSources( in );
                
                //we can't find the targets of this indirect jump

                if( in.size() < 1 )
                {
                    parsing_printf("... uninstrumentable, unable to find targets of indirect jump\n");
                    isInstrumentable_ = false;
                    return false;
                }

                instruction tableInsn = ah.getInstruction();
                instruction maxSwitch;
                bool isAddInJmp = true;
                
                int j = allInstructions.size() - 2;
                assert(j > 0);

                const unsigned char* ptr = ah.getInstruction().op_ptr();
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
                        parsing_printf("Checking 0x%x for register...\n", *findReg);
                        if ((*findReg.getInstruction().op_ptr()) == MOVREGMEM_REG) {
                            tableInsn = findReg.getInstruction();
                            foundTableInsn = true;
                            parsing_printf("Found register at 0x%x\n", *findReg);
                            break;    
                        }
                    }
                    if( !foundTableInsn )
                    {
                        //can't determine register contents
                        //give up on this function
                        parsing_printf("... uninstrumentable, unable to determine reg contents\n");
                        isInstrumentable_ = false;
                        return false;
                    }
                }    
                //now examine the instructions in my source block to 
                //get the maximum switch value
                //we are looking for the cmp instruction before the 
                //conditional jump
                Address saddr = in[0]->firstInsnOffset();
                InstrucIter iter( in[0] );
                instruction ins = iter.getInstruction();
                iter++;
                bool foundMaxSwitch = false;
                while( *iter < in[0]->endOffset() )
                {
                    if( iter.getInstruction().type() & IS_JCC )
                    {
                        maxSwitch = ins;
                        foundMaxSwitch = true;
                    }
                    ins = iter.getInstruction();
                    iter++;
                }
                 
                if( !foundMaxSwitch )
                {
                    parsing_printf("... uninstrumentable, unable to fix max switch size\n");
                    isInstrumentable_ = false;
                    return false;
                }
                //found the max switch assume jump table
                else
                {
                    pdvector< Address > result;
                    if( !ah.getMultipleJumpTargets( result, tableInsn, 
                                                    maxSwitch, isAddInJmp ) )
                    {
                        //getMultipleJumpTargets failed.
                        //give up on this function

                        //XXX
                        parsing_printf("... getMultipleJumpTargets failed, uninstrumentable\n");
                        isInstrumentable_ = false;
                        break;

                        return false;
                    }
                    for( unsigned l = 0; l < result.size(); l++ )
                    {
                        Address res = result[ l ];
                        if( !img()->isValidAddress( res ) )
                            continue;
                        //parsing_printf("... %d target is 0x%x\n",
                        //l,
                        //res);
                        if( res < funcBegin )
                        {
                            currBlk->isExitBlock_ = true;
                        }
                        else {
                            addBasicBlock(res,
                                          currBlk,
                                          leaders,
                                          leadersToBlock,
                                          jmpTargets);
                        }
                    }                   
                } 
                break; 
            }
            else if ( ah.isAJumpInstruction() ) 
            {
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = currAddr + insnSize;
                
                if( currAddr >= funcEnd )
                    funcEnd = currAddr + insnSize;
                
                Address target = ah.getBranchTargetAddress();

                //parsing_printf("... 0x%x is a jump to 0x%x\n",
                //currAddr, target);
                Address val = *ah + insnSize;

                ExceptionBlock b;
                if (img()->getObject().getCatchBlock(b, val))
                {
                    //Create a basic block for the catch block
                    Address cstart = b.catchStart();
                    addBasicBlock(cstart,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  jmpTargets);
                }                

                if( !img()->isValidAddress( target ) )
                    break;
                 
                img()->addJumpTarget( target );
                		
                //check if tailcall
		// -1 includes the jump -- skip it.
                numInsns = allInstructions.size() - 2;
		
                if( img()->findFuncByEntry( target ) || 
                    ( *allInstructions[ numInsns ].ptr() == POP_EBP ||
                      allInstructions[ numInsns ].isLeave() ))
		  {
                      currBlk->isExitBlock_ = true;
                      //parsing_printf("... making new exit point at 0x%x\n", currAddr);
                      p = new image_instPoint(currAddr, 
                                              ah.getInstruction(),
                                              this, 
                                              functionExit);
                      funcReturns.push_back( p );
		  }
                else if(  target < funcBegin )  
		  {
                      //parsing_printf("... making new exit point at 0x%x\n", currAddr);
                      p = new image_instPoint(currAddr, 
                                              ah.getInstruction(),
                                              this, 
                                              functionExit); 

                    funcReturns.push_back( p );
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
            else if( ah.isAReturnInstruction() ) 
            {
                parsing_printf("... 0x%x (%d) is a return\n", currAddr, currAddr - getOffset());
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = currAddr + insnSize;
                currBlk->isExitBlock_ = true;

                if( currAddr >= funcEnd )
                    funcEnd = currAddr + insnSize;
                
                //parsing_printf("... making new exit point at 0x%x\n", currAddr);
                p = new image_instPoint( currAddr, 
                                         ah.getInstruction(),
                                         this, 
                                         functionExit);
                funcReturns.push_back( p );
                
                break;
            } 
            else if( ah.isACallInstruction() ) 
            {
                //parsing_printf("... 0x%x is a call\n", currAddr);
                //Uncomment this and run paradyn on eon to reproduce bug# 440
                //window = 0;
                //validTarget is set to false if the call target is not a 
                //valid address in the applications process space 
                bool validTarget = true;
                              
                if ( isRealCall( ah.getInstruction(), currAddr, img(), 
                                 validTarget) ) 
                {
                    Address target = ah.getBranchTargetAddress();
                    //parsing_printf("... making new call point at 0x%x to 0x%x\n", currAddr, target);
                    if (ah.isIndir()) {
                        p = new image_instPoint( currAddr, 
                                                 ah.getInstruction(),
                                                 this, 
                                                 0,
                                                 true);
                    }
                    else {
                        p = new image_instPoint( currAddr, 
                                                 ah.getInstruction(),
                                                 this, 
                                                 target,
                                                 false);
                    }
                    calls.push_back( p );		    
                    
                    if( !ah.isIndir() && 
                        ( target < currAddr || target > currAddr + insnSize ))
                    {
                        
                        if( !img()->funcsByEntryAddr.defines( target ) &&
                            img()->isCode( target ) )
                            {
                                callTargets.push_back( target );
                            };
                        
                        
                    }
                } 
                else 
                {	

                    // Call was to an invalid address, do not instrument func 
                    if( validTarget == false ) 
                    {
                        parsing_printf("... invalid call target\n");
                        isInstrumentable_ = false;
                        return false;
                    }
                    else {
                        //parsing_printf("... not real call, skipping inst point\n");
                    }
                    // Force relocation when instrumenting function
                    //needs_relocation_ = true;
                }
            }
            else if ( ah.isALeaveInstruction() )
            {
                //parsing_printf("... 0x%x is a leave\n", currAddr);
                noStackFrame = false;
            }
            
            ah++;
        }
    }    

    //if the function contains a jump to a jump table, we can't relocate
    if ( !canBeRelocated ) 
    {
        // Function would have needed relocation 
#ifdef DEBUG_FUNC_RELOC      
        if ( needs_relocation_ == true ) 
	  cerr << "Jump Table: Can't relocate function"
	       << prettyName().c_str() << endl;
#endif
	//needs_relocation_ = false;
    }


    cleanBlockList();

    
    size_ = funcEnd - funcBegin;

    isInstrumentable_ = true;
    return true;    
}

