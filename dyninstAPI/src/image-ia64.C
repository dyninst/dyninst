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

// $Id: image-ia64.C,v 1.2 2005/08/15 22:20:10 bernat Exp $

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


bool image_func::findInstPoints(pdvector<Address> & /*callTargets*/)
{
    if( parsed_ ) 
        {
            parsing_printf("Error: multiple call of findInstPoints\n");
            return false;
        } 
    parsed_ = true;
    
    parsing_printf("findInstPoints for func %s, at 0x%llx\n",
		   symTabName().c_str(), getOffset());
    
    BPatch_Set< Address > leaders;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );
    
    // FIXME: laune's parser: this is wrong
    Address funcBegin = getOffset();
    Address funcEnd = funcBegin + getSize();
    
    InstrucIter ah (funcBegin, this);

    image_instPoint *p;

    //define entry instpoint 
    p = new image_instPoint(funcBegin,
                            ah.getInstruction(),
                            this,
                            functionEntry);
    funcEntries_.push_back(p);
    
    Address currAddr = funcBegin;
    
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
        InstrucIter ah( jmpTargets[ i ], this ); 
        image_basicBlock* currBlk = leadersToBlock[ jmpTargets[ i ] ];

	assert(currBlk);
        assert(currBlk->firstInsnOffset() == jmpTargets[i]);

        while( true ) {
            currAddr = *ah;

            if ((currAddr < funcBegin) ||
                (currAddr >= funcEnd)) {
                // Must not have seen a return....
                currBlk->lastInsnOffset_ = ah.peekPrev();
                currBlk->blockEndOffset_ = currAddr;
                break;
            }

            if( visited.contains( currAddr ) )
                break;
            else
                visited += currAddr;

            //parsing_printf("checking insn at 0x%llx, %s + %d\n", *ah,
            //symTabName().c_str(), (*ah) - funcBegin);

            if( ah.isACondBranchInstruction() )
                {
                    //parsing_printf("cond branch\n");
                    currBlk->lastInsnOffset_ = currAddr; 
                    currBlk->blockEndOffset_ = ah.peekNext();
                    
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
		      //parsing_printf("Function exit at 0x%x\n", *ah);
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
                    
                    Address t2 = ah.peekNext();
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
                 parsing_printf("Copying iter for indirect jump\n");
                 InstrucIter ah2( ah );
                 currBlk->lastInsnOffset_ = currAddr;
                 currBlk->blockEndOffset_ = ah.peekNext();
                                  
                 BPatch_Set< Address > res;
                 ah2.getMultipleJumpTargets( res );
                                  
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
            else if( ah.getInstruction().getType() == instruction::ALLOC ) {
                allocs.push_back(currAddr);
                parsing_printf("Alloc insn (0x%llx)\n", currAddr);
                // Alloc's don't actually start basic blocks, but our analysis
                // has an easier time if they do. -- todd
                if( ! leaders.contains( currAddr ) ) { 
                    leadersToBlock[currAddr] = new image_basicBlock(this, currAddr);
                    leaders += currAddr;
                    blockList.push_back(leadersToBlock[currAddr]);
                    leadersToBlock[currAddr]->addSource(currBlk);
                    currBlk->addTarget(leadersToBlock[currAddr]);
                    currBlk = leadersToBlock[currAddr];

                    // And end the old one
                    currBlk->lastInsnOffset_ = ah.peekPrev();
                    currBlk->blockEndOffset_ = currAddr;
                }
            } /* end if an alloc instruction */
            else {
                parsing_printf("Non-control-flow insn (0x%lx)\n", currAddr);
            }
            ah++;
        }                   
    }

    cleanBlockList();

    
    size_ = funcEnd - funcBegin;
    
    isInstrumentable_ = true;
    return true;
}




#if 0
/* Required by symtab.C:
   Locate the entry point (funcEntry_, an InstPoint), the call sites
   (calls, vector<instPoint *>), and the return(s) (funcReturns, vector<instPoint *>).
 */
bool int_function::findInstPoints( const image * i_owner ) {

    parsed_ = true;
    
    /* We assume the function boundaries are correct.  [Check jump tables, etc.] */
    Address addr = (Address)i_owner->getPtrToInstruction( getAddress( 0 ) );
    
    IA64_iterator iAddr( addr );
    Address lastI = addr + get_size();
    Address addressDelta = getAddress( 0 ) - addr;
    
    /* Generate an instPoint for the function entry. */
    funcEntry_ = new instPoint( getAddress( 0 ), this, * iAddr, functionEntry );
    
    instruction * currInsn;
    for( ; iAddr < lastI; iAddr++ ) {	
        currInsn = * iAddr;
        switch( currInsn->getType() ) {
        case instruction::DIRECT_BRANCH: {
            /* Some system call -wrapping functions (write(), read()) jump to common code
               (at __libc_start_main+560) to handle system calls which return errors,
               but this common code returns directly, instead of jumping back to its
               callee, which means our exit instrumentation may not be run.
               
               Laune's new parsing code will handle cases like this, so for now,
               use the same wrong assumption as the rest of the code (that symbol
               table sizes are correct) to determine the boundaries of the function,
               and claim that jumps outside these boundaries are exits. */
            Address targetAddress = currInsn->getTargetAddress() + iAddr.getEncodedAddress() + addressDelta;
            targetAddress =  targetAddress - (targetAddress % 16);
            Address lowAddress = addr + addressDelta;
            Address highAddress = addr + get_size() + addressDelta;
            
            if( ! ( lowAddress <= targetAddress && targetAddress < highAddress ) ) { 
                // /* DEBUG */ fprintf( stderr, "%s[%d]: 0x%lx targets 0x%lx, outside of 0x%lx to 0x%lx\n", __FILE__, __LINE__, iAddr.getEncodedAddress() + addressDelta, targetAddress, lowAddress, highAddress );
                funcReturns.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, currInsn, functionExit ) );
            }			
        } break;				
        
        case instruction::RETURN:
            funcReturns.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, currInsn, functionExit ) );
            break;
            
        case instruction::DIRECT_CALL:
        case instruction::INDIRECT_CALL:
            calls.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, currInsn, callSite) );
            break;
            
        case instruction::ALLOC:
            allocs.push_back( iAddr.getEncodedAddress() - addr );
            break;
            
        default:
            break;
        } /* end instruction type switch */
    } /* end instruction iteration */
    
    isInstrumentable_ = 1;
    return true; // if this function can ever return false, make sure the is_instrumentable flag
    // is set properly in this function.
} /* end findInstPoints() */

#endif
