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

// $Id: image-power.C,v 1.4 2005/09/01 22:18:19 bernat Exp $

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

bool image_func::findInstPoints(pdvector<Address> &callTargets)
{
    if( parsed_ ) 
    {
        parsing_printf("Error: multiple call of findInstPoints\n");
        return false;
    } 
    parsed_ = true;

    parsing_printf("findInstPoints for func %s, at 0x%x\n",
            symTabName().c_str(), getOffset());

    makesNoCalls_ = true;
    noStackFrame = true;
    int insnSize = instruction::size();

   BPatch_Set< Address > leaders;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );
       
    Address funcBegin = getOffset();
    Address funcEnd = funcBegin;
    
    InstrucIter ah (funcBegin, this);

    if (!ah.getInstruction().valid()) {
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
    int framesize; // This isn't actually set yet
    if (ah.isStackFramePreamble(framesize))
        noStackFrame = false;
    ah.setCurrentAddress(funcBegin);

    // Will peek a few instructions ahead.
    if (ah.isReturnValueSave())
        makesNoCalls_ = false;
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
            if( visited.contains( currAddr ) )
                break;
            else
                visited += currAddr;

            parsing_printf("checking insn at 0x%x, %s + %d\n", *ah,
                    symTabName().c_str(), (*ah) - funcBegin);
            
            if( ah.isACondBranchInstruction() )
                {
                    parsing_printf("cond branch\n");
                    currBlk->lastInsnOffset_ = currAddr; 
                    currBlk->blockEndOffset_ = currAddr + insnSize;
                    if( currAddr >= funcEnd )
                        funcEnd = currAddr + insnSize;
                    
                    Address target = ah.getBranchTargetAddress();
                    // img()->addJumpTarget(target);

                    if( target < funcBegin ) {
                        currBlk->isExitBlock_ = true;
                        // And make an inst point
                        p = new image_instPoint(currAddr, 
                                                ah.getInstruction(),
                                                this,
                                                functionExit);
                        parsing_printf("Function exit at 0x%x\n", *ah);
                        funcReturns.push_back(p);
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
            else if(ah.isAIndirectJumpInstruction())  {
                parsing_printf("ind branch\n");
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
                    parsing_printf("Function exit at 0x%x\n", *ah);
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

                 if( target < funcBegin )
                 {
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
                 parsing_printf("call branch\n");
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
                parsing_printf("Non-control-flow insn\n");
            }
            ah++;
        }                   
    }

    cleanBlockList();

    
    endOffset_ = funcEnd;
    
    return true;
}
