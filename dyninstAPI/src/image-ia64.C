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

// $Id: image-ia64.C,v 1.5 2005/10/13 21:12:30 tlmiller Exp $

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

bool image_func::findInstPoints( pdvector<Address> & /* callTargets */ ) {
	if( parsed_ ) {
		parsing_printf("Error: multiple call of findInstPoints\n");
		return false;
		} 
	parsed_ = true;
    
	parsing_printf( "findInstPoints for func %s, at 0x%lx, end 0x%lx, size %d\n", symTabName().c_str(), getOffset(), getEndOffset(), getSymTabSize() );
    
	BPatch_Set< Address > leaders;
	dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );
    
	// FIXME: laune's parser: this is wrong
	Address funcBegin = getOffset();
	Address funcEnd = getEndOffset();
	InstrucIter ah0( funcBegin, this );	

	// define entry instpoint 
	image_instPoint * p = new image_instPoint( funcBegin, ah0.getInstruction(), this, functionEntry );
	funcEntries_.push_back( p );

	/* Iterate forward over instructions from all jump targets in the function,
	   creating basic blocks and inst points as we go. */
	Address currAddr = funcBegin;
	BPatch_Set< Address > visited;
	pdvector< Address > jmpTargets;
    
	/* Create the entry basic block */
	leaders += funcBegin;
	leadersToBlock[ funcBegin ] = new image_basicBlock( this, funcBegin );
	leadersToBlock[ funcBegin ]->isEntryBlock_ = true;
	blockList.push_back( leadersToBlock[ funcBegin ] );
	jmpTargets.push_back( funcBegin );
    
	for( unsigned i = 0; i < jmpTargets.size(); i++ ) {
		InstrucIter ah( jmpTargets[ i ], this ); 
		image_basicBlock * currBlk = leadersToBlock[ jmpTargets[ i ] ];
		assert( currBlk );
		assert( currBlk->firstInsnOffset() == jmpTargets[i] );

		/* Iteratate forward. */
		while( true ) {
			currAddr = * ah;

			/* Have we seen this address before? */
			if( visited.contains( currAddr ) ) {
				break;
				}
			visited += currAddr;
			
			/* Have we exited the function? */
			if( (currAddr < funcBegin) || (currAddr >= funcEnd ) ) {
				// /* DEBUG */ fprintf( stderr, "%s[%d]: exited function boundaries, truncating basic block.\n", __FILE__, __LINE__ );
				currBlk->lastInsnOffset_ = ah.peekPrev();
				currBlk->blockEndOffset_ = currAddr;
				break;
				}
			
			/* Begin considering the different types of instruction. */
			if( ah.isACondBranchInstruction() ) {
				/* Close off the current block. */
				currBlk->lastInsnOffset_ = currAddr; 
				currBlk->blockEndOffset_ = ah.peekNext();
                    
				Address target = ah.getBranchTargetAddress();

				/* Add the branch target, if it isn't headed outside the function. */
				bool exit = false;
				if( (target < funcBegin) || (target >= funcEnd) ) {
					exit = true;
                    }
				else {
					addBasicBlock( target, currBlk, leaders, leadersToBlock, jmpTargets );
                    }                        
                
                /* Consider the fall-through case. */
				Address t2 = ah.peekNext();
				if( t2 < funcEnd ) {
					addBasicBlock( t2, currBlk, leaders, leadersToBlock, jmpTargets );
                    }
				else {
					exit = true;
					}

				if( exit ) {
					currBlk->isExitBlock_ = true;
					
					p = new image_instPoint( currAddr, ah.getInstruction(), this, functionExit );
					parsing_printf("Function exit at 0x%x\n", *ah);
					funcReturns.push_back(p);
                    }
                
				/* We've found the end of this basic block; go look at another jump target. */
				break;
                } /* end if we found a conditional branch */


                
             else if( ah.isAIndirectJumpInstruction() )
             {
                 parsing_printf("ind branch\n");
                 parsing_printf("Copying iter for indirect jump\n");
                 InstrucIter ah2( ah );
                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = ah.peekNext();
                                  
                 BPatch_Set< Address > res;
                 if (!ah2.getMultipleJumpTargets( res ))
                     isInstrumentable_ = false;
                                  
                 BPatch_Set< Address >::iterator iter;
                 iter = res.begin();
                 bool leavesFunc = false;
                 while( iter != res.end() )
                 {
                     if((*iter < funcBegin) ||
                        (*iter >= funcEnd))
                         {
                             leavesFunc = true;
                         }
                     else
                     {
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
            else if( ah.isAReturnInstruction())
             {
                 //parsing_printf("ret or invalid\n");

                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = ah.peekNext();
                 currBlk->isExitBlock_ = true;
                 
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
                 currBlk->blockEndOffset_ = ah.peekNext();
                                 
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

             
            
            /* An alloc instruction doesn't actually start a basic block, but our analysis is simpler if they do. */
			else if( ah.getInstruction().getType() == instruction::ALLOC ) {
				allocs.push_back( currAddr );
				if( currAddr == currBlk->firstInsnOffset() ) {
					/* If an alloc is the first instruction in its block, keep parsing the block. */
					}
				else {
					/* Re-visit this alloc. */
					visited.remove( currAddr );
				
					/* Close off the current block. */
					currBlk->lastInsnOffset_ = ah.peekPrev(); 
					currBlk->blockEndOffset_ = currAddr;
					
					// /* DEBUG */ fprintf( stderr, "%s[%d]: alloc block for function '%s': offset 0x%lx - 0x%lx\n",
					//						__FILE__, __LINE__, symTabName().c_str(), currBlk->firstInsnOffset_ - getOffset(), currBlk->blockEndOffset_ - getOffset() );
					
					/* Start the next one; */
					addBasicBlock( currAddr, currBlk, leaders, leadersToBlock, jmpTargets );
        	        break;
        	        }
                } /* end if we found an alloc */
                
			else {
				parsing_printf( "Non-control-flow insn (0x%lx)\n", currAddr );
				} /* end the default case */
            
            /* Iterate over instructions. */
			ah++;
			} /* end while() loop over instructions */
		} /* end for() loop over jump targets. */

    cleanBlockList();

    isInstrumentable_ = true;
    return true;
	} /* end image_func::findInstPoints() */
