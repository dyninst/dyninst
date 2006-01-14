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
 * $Id: image-flowGraph.C,v 1.1 2006/01/14 23:47:30 nater Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/Object.h"
#include <fstream>
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/src/InstrucIter.h"

#ifndef BPATCH_LIBRARY
#include "common/h/Dictionary.h"
#endif

#include "LineInformation.h"
#include "dyninstAPI/h/BPatch_flowGraph.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

int addrfunccmp( image_func*& pdf1, image_func*& pdf2 )
{
    if( pdf1->getOffset() > pdf2->getOffset() )
        return 1;
    if( pdf1->getOffset() < pdf2->getOffset() )
        return -1;
    return 0;
}

//analyzeImage() constructs a flow graph of basic blocks in the image and
//assigns them to functions. It follows calls to discover new functions,
//as well as using heuristics to search for functions within gaps between
//basic blocks.
//
//Note: basic blocks may be shared between several functions
bool image::analyzeImage()
{
    // TODO: remove arch_x86 from here - it's just for testing
#if defined(arch_x86_64) || defined(arch_x86)
    ia32_set_mode_64(getObject().getAddressWidth() == 8);
#endif
  
 // Hold unseen call targets
  pdvector< Address > callTargets;
 // Hold function stubs created during parsing
  dictionary_hash< Address, image_func * > preParseStubs( addrHash );
  image_func *pdf;
  pdmodule *mod = NULL;

  assert(parseState_ < analyzed);
  // Prevent recursion: with this set we can call findFoo as often as we want
  parseState_ = analyzing;

  if (parseState_ < symtab) {
    fprintf(stderr, "Error: attempt to analyze before function lists built\n");
    return true;
  }
  
  //pdvector<image_func *> new_functions;  
    pdvector<Address> new_targets;      // call targets detected in
                                        // parseStaticCallTargets

  for (unsigned i = 0; i < everyUniqueFunction.size(); i++) {
    pdf = everyUniqueFunction[i];
    mod = pdf->pdmod();
    assert(pdf); assert(mod);
    pdstring name = pdf->symTabName();
    parseFunction( pdf, callTargets, preParseStubs);
  }      
 
  // callTargets now holds a big list of target addresses; some are already
  // in functions that we know about, some point to new functions. 
    while(callTargets.size() > 0)
    {
        // parse any new functions entered at addresses in callTargets
        parseStaticCallTargets( callTargets, new_targets, preParseStubs, mod );
        callTargets.clear();
      
            // FIXME hi, wasteful person
        VECTOR_APPEND(callTargets,new_targets); 
        new_targets.clear();
    }

#if defined(cap_stripped_binaries)
  
  int numIndir = 0;
  unsigned p = 0;
  // We start over until things converge; hence the goto target
 top:

    parseStaticCallTargets( callTargets, new_targets, preParseStubs, mod );
    callTargets.clear(); 

    VECTOR_APPEND(callTargets,new_targets);
    new_targets.clear();
   
    // nothing to do, exit
    if( everyUniqueFunction.size() <= 0 )
    {
        return true;
    }

    VECTOR_SORT(everyUniqueFunction, addrfunccmp);
  
    Address lastPos;
    lastPos = everyUniqueFunction[0]->getOffset() + 
              everyUniqueFunction[0]->getSymTabSize();
  
    unsigned int rawFuncSize = everyUniqueFunction.size();
  
    for( ; p + 1 < rawFuncSize; p++ )
    {
        image_func* func1 = everyUniqueFunction[p];
        image_func* func2 = everyUniqueFunction[p + 1];
      
        Address gapStart = func1->getOffset() + func1->getSymTabSize();
        Address gapEnd = func2->getOffset();
        Address gap = gapEnd - gapStart;
      
        //gap should be big enough to accomodate a function prologue
        if( gap >= 5 )
        {
            Address pos = gapStart;
            while( pos < gapEnd && isCode( pos ) )
            {
                const unsigned char* instPtr;
                instPtr = (const unsigned char *)getPtrToInstruction( pos );
              
                instruction insn;
                insn.setInstruction( instPtr );

                if( isFunctionPrologue(insn) && !funcsByEntryAddr.defines(pos))
                {
                    char name[20];
                    numIndir++;
                    sprintf( name, "gap_f%lx", pos );
                    pdf = new image_func( name, pos, UINT_MAX, mod, this);
                    if(parseFunction( pdf, callTargets, preParseStubs)) {
                        addFunctionName(pdf, name, true);
                        pdf->addPrettyName( name );
                        // No typed name
                  
                        enterFunctionInTables(pdf, false);
                    }
                  
                    if( callTargets.size() > 0 )
                    {
                        for( unsigned r = 0; r < callTargets.size(); r++ )
                        {
                            if( callTargets[r] < func1->getOffset() )
                                p++;
                        }
                        goto top; //goto is the devil's construct. repent!! 
                    }
                }
                pos++;
            }   
        }
    }
#endif

#if 0  
#if defined(cap_stripped_binaries)

  //phase 2 - error detection and recovery 
  VECTOR_SORT( everyUniqueFunction, addrfunccmp );
  for( unsigned int k = 0; k + 1 < everyUniqueFunction.size(); k++ )
    {
      image_func* func1 = everyUniqueFunction[ k ];
      image_func* func2 = everyUniqueFunction[ k + 1 ];
      
      if( func1->getOffset() == 0 ) 
          continue;

      assert( func1->getOffset() != func2->getOffset() );
      
      //look for overlapping functions
#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
      if ((func2->getOffset() < func1->getEndOffset()) &&
	  (strstr(func2->prettyName().c_str(), "nocancel") ||
	   strstr(func2->prettyName().c_str(), "_L_mutex_lock")))
          
          { 
              func1->markAsNeedingRelocation(true);
          }
      else
#endif
          if( func2->getOffset() < func1->getEndOffset() )
              {
                 /* lies 
                   //use the start address of the second function as the upper bound
                  //on the end address of the first */
                  Address addr = func2->getOffset();
                  func1->updateFunctionEnd(addr);

               // TODO: drop through the basic blocks between
               // func2->getOffset() and func2->getEndOffset and mark
               // as shared.  Also mark func1 and func2 as needing
               // relocation.

                  
              }
    }    
#endif
#endif

  // And bind all intra-module call points
  for (unsigned b_iter = 0; b_iter < everyUniqueFunction.size(); b_iter++) {
      everyUniqueFunction[b_iter]->checkCallPoints();
  }

  // TODO remove this; it is just for getting some cheap statistics

  //DumpAllStats();
  
  parseState_ = analyzed;
  return true;
}

void image::DumpAllStats()
{
    image_func *f;
    //image_basicBlock *b;
    char buf[1024];
    char buf2[1024];
    
    buf[0] = 0;
    buf2[0] = 0;

    //fprintf(stdout,"Module: %s\n",name_.c_str());
    
//    fprintf(stdout,"Func Name:Block Number:Block Start:Block End:Shared:Source Blocks:Target Blocks\n");

    fprintf(stdout,"Module:Func:Contains Shared Blocks\n");

    for (unsigned b_iter = 0; b_iter < everyUniqueFunction.size(); b_iter++) {

        f = everyUniqueFunction[b_iter];
        // pass over all the blocks

        fprintf(stdout,"%s,%s:%d\n",name_.c_str(),f->prettyName().c_str(),f->containsSharedBlocks());

/*
        for(unsigned i=0;i<f->blocks().size();i++)
        {
            // create source and target strings
            pdvector< image_edge * > src;
            pdvector< image_edge * > trg;

            b = f->blocks()[i];

            b->getSources(src);
            b->getTargets(trg);
    
            int total = 0;
            for(unsigned j=0;j<src.size();j++)
            {
                total += snprintf(buf,1024-total,"%d ",
                                    src[j]->getSource()->id()); 
            }

            total = 0;
            for(unsigned j=0;j<trg.size();j++)
            {
                total += snprintf(buf2,1024-total,"%d ",
                                    trg[j]->getTarget()->id()); 
            }

            fprintf(stdout,"%s:%d:0x%x:0x%x:%d:%s:%s\n",
                    f->prettyName().c_str(),
                    b->id(),
                    b->firstInsnOffset(),
                    b->endOffset(),
                    b->isShared(),
                    buf,
                    buf2);
        }
*/
    }


}


//parseStaticCallTargets() iterates its input list of call targets,
//creating new image_function objects for addresses that have not
//previously been visited and parsing each of these new functions.
//It populates newTargets with any call targets detected in the
//parsing of these functions.
void image::parseStaticCallTargets( pdvector< Address >& callTargets,
                     pdvector< Address >& newTargets,
                     dictionary_hash< Address, image_func * > &preParseStubs, 
                     pdmodule* mod )
{

  // TODO: I don't think "mod" is the right thing here; should
  // do a lookup by address
    image_func* pdf;

    for( unsigned j = 0; j < callTargets.size(); j++ )
    {
        // Call targets have already had image_function objects
        // created for them AND have an entry basic block connected
        // by a call edge to the calling block (which is how we
        // found the call target).
        //
        // These stub functions, however, cannot be added to
        // the standard lists before parsing, however, because
        // they may not successfully parse (eg contain only a
        // jump instruction). They are stored temporarily in
        // preParseStubs and are only added to the tables if
        // parsing is successful.

        if( !isCode( callTargets[ j ] ) )
            continue;    
        if( funcsByEntryAddr.defines( callTargets[ j ] ) )
            continue;

        if( preParseStubs.defines( callTargets[ j ] ) )
        {
            pdf = preParseStubs[callTargets[j]];
            
            if(parseFunction(pdf,newTargets,preParseStubs))
            {
                parsing_printf(" ***** Adding %s (0x%lx) to tables\n",
                    pdf->symTabName().c_str(),pdf->getOffset());

                enterFunctionInTables(pdf,true);

                addFunctionName(pdf, pdf->symTabName().c_str(), true); // mangled name
                pdf->addPrettyName( pdf->symTabName().c_str() ); // Auto-adds to our list
            }
        }
        else
        {
            parsing_printf("Call target 0x%lx does not have associated func\n",
                            callTargets[j]);
        } 
 

        // Functions are now created when the call target is detected,
        // in order to have a destination for call edges.
        /*      
        sprintf(name, "f%lx", callTargets[j] );
        //some (rare) compiler generated symbols have 0 for the size.
        //most of these belong to screwy functions and, it seems
        //best to avoid them. to distinguish between these screwy functions
        //and the unparsed ones that we make up we use UINT_MAX for the sizes 
        //of the unparsed functions
        pdf = new image_func( name, callTargets[ j ], UINT_MAX, mod, this);
        pdf->addPrettyName( name );
        
        //we no longer keep separate lists for instrumentable and 
        //uninstrumentable
        if (parseFunction( pdf, newTargets )) {
            everyUniqueFunction.push_back(pdf);
            createdFunctions.push_back(pdf);
            enterFunctionInTables( pdf, mod );
            //raw_funcs.push_back( pdf );
        }
        */
    }
}

//parseFunction() iterates over the instructions of a function, following
//non-call control transfers and constructing a CFG for the function. It
//adds all call targets detected during this process to the callTargets
//parameter. It /also/ creates all the instrumentation points within
//the function, since this is a good time to do that.
//
//parseFunction() doesn't actually do any of this; the architecture-
//dependant methods do the actual work.
bool image::parseFunction(image_func* pdf, pdvector< Address >& callTargets,
                    dictionary_hash< Address, image_func * >& preParseStubs)
{
    // callTargets: targets of this function (to help identify new functions
    // May be returned unmodified

    bool ret;
    ret = pdf->parse( callTargets, preParseStubs );
    return ret;
}

/* parse is responsible for initiating parsing of a function. instPoints
 * and image_basicBlocks are created at this level.
 *
 * Returns: success or failure
 *
 * Side effects:
 *  - callTargets may be appended to
 *  - preParseStubs may be modified
 */
bool image_func::parse(
        pdvector< Address >& callTargets,
        dictionary_hash< Address, image_func *>& preParseStubs)
{
    pdvector< image_basicBlock * > entryBlocks_;

    parsing_printf("%s: initiating parsing\n", symTabName().c_str());

    if(parsed_)
    {
        fprintf(stderr, "Error: multiple call of parse() for %s\n",
                symTabName().c_str());
        return false;
    }
    parsed_ = true;

    // Some architectures have certain functions or classes of functions
    // that should appear to be parsed without actually being parsed.
    if(archAvoidParsing())
    {
        return true;
    }

    // For some architectures, some functions are recognizably unparseable 
    // from the start.
    if(archIsUnparseable())
    {
        return false;
    }

    // Optimistic assumptions
    isInstrumentable_ = true;
    canBeRelocated_ = true;

    noStackFrame = true;
    image_instPoint *p;

    Address funcBegin = getOffset();
    Address funcEntryAddr = funcBegin;

    InstrucIter ah(funcBegin, this);

    // Test for various reasons we might just give up and die
    if( !archCheckEntry( ah, this ) )
    {
        // Check for redirection to another function as first insn.
        if( ah.isAJumpInstruction() || ah.isACallInstruction() )
        {
            Address target = ah.getBranchTargetAddress();

            if(!image_->funcsByEntryAddr.defines(target) &&
               !preParseStubs.defines(target))
            {
                // makes sure a function exists at the point for later parsing
                (void)FindOrCreateFunc(target,callTargets,preParseStubs);
            }
            parsing_printf("Jump or call at entry of function\n");
        }

        endOffset_ = ah.peekNext();
        isInstrumentable_ = false;
        return false;
    }

    // Some architectures (alpha) have an entry point that is
    // not the function start; knowing where it is enables
    // correct instrumentation.
    archGetFuncEntryAddr(funcEntryAddr);
    ah.setCurrentAddress(funcEntryAddr);

    // Define entry point
    p = new image_instPoint(funcEntryAddr,
                            ah.getInstruction(),
                            this,
                            functionEntry);
    funcEntries_.push_back(p);

    int frameSize;
    Address tmp = *ah;
    if(ah.isStackFramePreamble(frameSize))
    {
        archSetFrameSize(frameSize);
        noStackFrame = false;
    }
    ah.setCurrentAddress(tmp);
    savesFP_ = ah.isFramePush();    // only ever true on x86

    // Architecture-specific "do not relocate" list
    if(archNoRelocate())
    {
        canBeRelocated_ = false;
    }

    // Create or find function entry block placeholder
    image_basicBlock *ph_entryBlock;
    codeRange *tmpRange;
    bool preParsed = false;
    if(image_->basicBlocksByRange.find(funcBegin, tmpRange))
    {
        ph_entryBlock = dynamic_cast<image_basicBlock *>(tmpRange);
        if(ph_entryBlock->firstInsnOffset() == funcBegin)
        {
            // perfect match; this is either a 'stub' function
            // or has already been parsed
            if(!ph_entryBlock->isStub_)
            {
                // non-stub existing blocks are handled by
                // parseSharedBlocks
                preParsed = true;
            }
        }
        else
        {
            // existing block was *split* by function entry
            ph_entryBlock = ph_entryBlock->split(funcBegin,this);

            image_->basicBlocksByRange.insert(ph_entryBlock);

            preParsed = true;
        }
    }
    else
    {
        ph_entryBlock = new image_basicBlock(this, funcBegin);
        ph_entryBlock->isStub_ = true;
        image_->basicBlocksByRange.insert(ph_entryBlock);
    }

    // It is always safe to set these
    ph_entryBlock->firstInsnOffset_ = funcBegin;
    ph_entryBlock->isEntryBlock_ = true;

    // Record entry block
    entryBlock_ = ph_entryBlock;
    entryBlocks_.push_back(ph_entryBlock);

    // The "actual" entry block (alpha)
    if(funcEntryAddr != funcBegin)
    {
        image_basicBlock *tmpBlock;
        if(image_->basicBlocksByRange.find(funcEntryAddr,tmpRange))
        {
            tmpBlock = dynamic_cast<image_basicBlock *>(tmpRange);
            if(tmpBlock->firstInsnOffset() == funcEntryAddr)
            {
                tmpBlock->isEntryBlock_ = true;
            }
            else
            {
                assert(tmpBlock == ph_entryBlock);
                tmpBlock = ph_entryBlock->split(funcEntryAddr,this);
                tmpBlock->isEntryBlock_ = true;
            }
        }
        else
        {
            tmpBlock = new image_basicBlock(this, funcEntryAddr);
            tmpBlock->isStub_ = true;
            image_->basicBlocksByRange.insert(tmpBlock);
            tmpBlock->isEntryBlock_ = true;
        }

        entryBlocks_.push_back(tmpBlock);

	// special alpha-case "multiple" entry (really multiple lookup)
	image_->funcsByEntryAddr[funcEntryAddr] = this;
    } 
    // End alpha-specific weirdness

    if(preParsed)
    {

        parseSharedBlocks(ph_entryBlock);
    }
    else
    {
        buildCFG(entryBlocks_, funcBegin, callTargets, preParseStubs);
    }

    cleanBlockList(); // FIXME rename--this sorts and does output right now 

    return true;
}

/* buildCFG does the bulk of the work in parsing a function. It is
 * responsible for following the control-flow through a function, building
 * up the set of basic blocks that compose it, and connecting these blocks
 * via edges.
 *
 * handles:
 *  - noncontiguous blocks
 *  - shared blocks (between other functions)
 *  - previously parsed blocks
 *
 * Side effects:
 *  - Appends discovered call targets to the callTargets parameter
 *  - Appends newly created functions (at call targets) to preParseStubs 
 *
 * Architecture Independance:
 *  All architecture-specific code is abstracted away in the image-arch
 *  modules.
 */
bool image_func::buildCFG(
        pdvector<image_basicBlock *>& funcEntries,
        Address funcBegin,
        pdvector< Address >& callTargets,
        dictionary_hash< Address, image_func * >& preParseStubs)
{
    // Parsing worklist
    pdvector< Address > worklist;

    // Basic block lookup
    BPatch_Set< Address > leaders;
    BPatch_Set< image_basicBlock* > visited;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );

    // Prevent overrunning an existing basic block in linear scan
    Address nextExistingBlockAddr;
    image_basicBlock *nextExistingBlock;

    // Instructions and InstPoints
    pdvector< instruction > allInstructions;
    unsigned numInsns = 0;
    image_instPoint *p;
    int insnSize;

    // misc
    codeRange *tmpRange;

    // ** Use to special-case abort and exit calls
    const Object &obj = img()->getObject();
    const pdvector<relocationEntry> *fbt;
    dictionary_hash<Address, pdstring> pltFuncs(addrHash);
    if(obj.get_func_binding_table_ptr(fbt)) {
        for(unsigned k = 0; k < fbt->size(); k++)
        {
            pltFuncs[(*fbt)[k].target_addr()] = (*fbt)[k].name();
        }
    }

    // ** end abort/exit setup

    Address funcEnd = funcBegin;
    Address currAddr = funcBegin;
    // The reverse ordering here is to make things easier on
    // alpha. Indeed, the only reason funcEntries would ever
    // have size > 1 is if we're on AIX.

    // FIXME the real reason we do this is because our parser will
    // break if there are > 1 pre-existing blocks and they are not
    // processed in descending address order. So, this array should
    // be sorted. (of course, this is a special case for alpha only, so
    // it probably doesn't matter)
    for(int j=funcEntries.size()-1; j >= 0; j--)
    {
        Address a = funcEntries[j]->firstInsnOffset();
        leaders += a;
        leadersToBlock[a] = funcEntries[j];
        worklist.push_back(a);
    }

    /*
    leaders += funcBegin;
    leadersToBlock[funcBegin] = funcEntry;
    worklist.push_back(funcBegin);
    */

    for(unsigned i=0; i < worklist.size(); i++)
    {
        /*
        if(worklist[i] < startOffset_)
        {
            assert(0);
        }
        */

        InstrucIter ah(worklist[i],this);

        image_basicBlock* currBlk = leadersToBlock[worklist[i]];

        parsing_printf("... parsing block at 0x%lx, first insn offset 0x%lx\n",
                       worklist[i], currBlk->firstInsnOffset());

        // debuggin' 
        assert(currBlk->firstInsnOffset() == worklist[i]);

        // If this function has already parsed the block, skip
        if(visited.contains(currBlk))
            continue;

        if(currBlk->isStub_)
        {
            currBlk->isStub_ = false;
            blockList.push_back(currBlk);
            visited.insert(currBlk);

            parsing_printf("- adding block %d (0x%lx) to blocklist\n",
                           currBlk->id(), currBlk->firstInsnOffset());
        }
        else
        {
            // non-stub block must have previously been parsed
            parseSharedBlocks(currBlk, leaders, leadersToBlock, visited);
            continue;
        }

        // Remember where the next block is, so we don't blindly run over
        // the top of it when scanning through instructions.
        //nextExistingBlockAddr = UINT_MAX;
        nextExistingBlockAddr = ULONG_MAX;
        if(image_->basicBlocksByRange.successor(ah.peekNext(),tmpRange))
        {
            nextExistingBlock = dynamic_cast<image_basicBlock*>(tmpRange);
            if(nextExistingBlock->firstInsnOffset_ > worklist[i])
            {
                nextExistingBlockAddr = nextExistingBlock->firstInsnOffset_;
            }
        }

        while(true) // instructions in block
        {
            currAddr = *ah;
            insnSize = ah.getInstruction().size();

           // Check whether we're stomping over an existing basic block
            // (one which we have not yet parsed [addr isn't in visited])
            if(currAddr == nextExistingBlockAddr)
            {
                assert(nextExistingBlock->firstInsnOffset_ == currAddr);

                parsing_printf("rolling over existing block at 0x%lx\n",currAddr);

                // end block with previous addr
                // add edge to to this newly found block
                // push it on the worklist vector            
                Address prevAddr = ah.peekPrev();
                currBlk->lastInsnOffset_ = prevAddr;
                currBlk->blockEndOffset_ = currAddr;

                addEdge(currBlk,nextExistingBlock,ET_FALLTHROUGH);

                if(!visited.contains(nextExistingBlock))
                {
                    leaders += currAddr;
                    leadersToBlock[currAddr] = nextExistingBlock;
                    worklist.push_back(currAddr);
                    parsing_printf("- adding address 0x%lx to worklist\n",
                                    currAddr);
                }
                break;
            }

            allInstructions.push_back( ah.getInstruction() );

            // only ever true on x86
            if(ah.isFrameSetup() && savesFP_)
            {
                noStackFrame = false;
            }

            // Special architecture-specific instruction processing
            archInstructionProc( ah );

            if( ah.isACondBranchInstruction() )
            {
                // delay slots ?
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();

                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
                    //funcEnd = currAddr + insnSize;

                Address target = ah.getBranchTargetAddress();
                img()->addJumpTarget( target );

                // process fallthrough edge first, in case the target
                // is within this block (forcing a split)
                if(ah.isDelaySlot())
                {
                    // Skip delay slot (sparc)
                    ah++;
                } 
                Address t2 = ah.peekNext(); 
                //Address t2 = currAddr + insnSize;
                
                addBasicBlock(t2,
                              currBlk,
                              leaders,
                              leadersToBlock,
                              ET_FALLTHROUGH,
                              worklist,
                              visited);

                if( target < funcBegin )
                {
                    // FIXME This is actually not true in the new model,
                    // but there are numerous sanity checks based on ranges
                    // of functions that need investigating before we can
                    // turn this off.
                    currBlk->isExitBlock_ = true;
                    // FIXME if this stays an exit block, it needs an exit
                    // point
                }
                else {
                    addBasicBlock(target,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  ET_COND,
                                  worklist,
                                  visited);
                }

                break;                              
            }
            else if( ah.isAIndirectJumpInstruction() )
            {
                // delay slots ?
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                //currBlk->blockEndOffset_ = currAddr + insnSize;
                parsing_printf("... indirect jump at 0x%lx\n", currAddr);

                if( currAddr >= funcEnd )
                    funcEnd = currAddr + insnSize;
    
                // TODO: verify this isn't necessary for x86 (or anything)
                // anymore
                // checkIfRelocatable( ah.getInstruction(), canBeRelocated_ );

                // FIXME: these tests were specific to x86 originally, should
                // they remain x86-only?
                numInsns = allInstructions.size() - 2;
                if( numInsns == 0 ) {
                    // this "function" is a single instruction long
                    // (a jmp to the "real" function)
                    parsing_printf("... uninstrumentable due to 0 size\n");
                    isInstrumentable_ = false;
                    endOffset_ = startOffset_;
                    return false; 
                }                 

                if( archIsIndirectTailCall(ah) )
                {
                    // looks like a tail call
                    currBlk->isExitBlock_ = true;
                    break;
                    //FIXME shouldn't there be an instPoint here?
                }
                // *** end x86-only

                BPatch_Set< Address > targets;
                BPatch_Set< Address >::iterator iter;
                
                // get the list of targets
                if(!archGetMultipleJumpTargets(targets,currBlk,ah,allInstructions))
                {
                    // XXX temporarily disabling
                    //isInstrumentable_ = false;
                    canBeRelocated_ = false;
                }
                
                iter = targets.begin();
                for(iter = targets.begin(); iter != targets.end(); iter++)
                {
                    if( !img()->isValidAddress( *iter ) )
                        continue;

                    if(*iter < funcBegin) {
                        // FIXME see cond branch note, above
                        currBlk->isExitBlock_ = true;
                        // FIXME if this stays an exit block, it needs an exit
                        // point
                    }
                    else { 
                        addBasicBlock(*iter,
                                      currBlk,
                                      leaders,
                                      leadersToBlock,
                                      ET_COND,
                                      worklist,
                                      visited);
                    }
                }

                break;
            }
            else if( ah.isAJumpInstruction() )
            {
                // delay slots?
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                //currBlk->blockEndOffset_ = currAddr + insnSize;

                if( currAddr >= funcEnd )
                    funcEnd = currAddr + insnSize;

                Address target = ah.getBranchTargetAddress();

                parsing_printf("... 0x%lx is a jump to 0x%lx\n",
                               currAddr, target);

                Address catchStart;

                // Only used on x86
                if(archProcExceptionBlock(catchStart,ah.peekNext()))
                {
                    addBasicBlock(catchStart,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  ET_CATCH,
                                  worklist,
                                  visited);
                }

                if( !img()->isValidAddress( target ) )
                    break;

                // NOTE we don't do anything with these?
                img()->addJumpTarget( target );

                if(archIsATailCall( target, allInstructions ))
                {
                    // Only on x86 & sparc currently
                    currBlk->isExitBlock_ = true;
                    parsing_printf("... making new exit point at 0x%lx\n", 
                                   currAddr);                

                    p = new image_instPoint(currAddr,
                                            ah.getInstruction(),
                                            this,
                                            functionExit);
                    funcReturns.push_back( p );
                    currBlk->containsRet_ = true;
                    //retStatus_ = RS_RETURN;
                }
                else if( target < funcBegin )
                {
                    //FIXME demonstrably wrong, but see note in cond. branch
                    // code, above 
                    parsing_printf("... making new exit point at 0x%lx\n", currAddr);                
                    p = new image_instPoint(currAddr,
                                            ah.getInstruction(),
                                           this,
                                            functionExit);
                    
                    funcReturns.push_back( p );
                    currBlk->containsRet_ = true;
                //    retStatus_ = RS_RETURN;
                }
                else
                {
                    parsing_printf("... tying up unconditional branch target\n");                   
                    addBasicBlock(target,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  ET_DIRECT,
                                  worklist,
                                  visited);
                }
                break;
            }
            else if( ah.isAReturnInstruction() )
            {
                // delay slots?
                parsing_printf("... 0x%lx (%d) is a return\n", currAddr, 
                                currAddr - getOffset());                       

                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                //currBlk->blockEndOffset_ = currAddr + insnSize;
                currBlk->isExitBlock_ = true;
                    
                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
                    //funcEnd = currAddr + insnSize;
                
                p = new image_instPoint( currAddr, 
                                         ah.getInstruction(),
                                         this,
                                         functionExit);
                funcReturns.push_back( p );
                currBlk->containsRet_ = true;
                retStatus_ = RS_RETURN;

                break;
            }
            else if( ah.isACallInstruction() ||
                     ah.isADynamicCallInstruction() )
            {
                // delay slots?
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                //currBlk->blockEndOffset_ = currAddr + insnSize;

                parsing_printf("... 0x%lx is a call\n", currAddr);
                
                //validTarget is set to false if the call target is not a 
                //valid address in the applications process space 
                bool validTarget = true;

                image_func *targetFunc = NULL;
                
                // XXX move this out of archIsRealCall protection... safe?
                Address target = ah.getBranchTargetAddress();
                if ( archIsRealCall(ah, validTarget) )
                {
                    //parsing_printf("... making new call point at 0x%lx to 0x%lx\n", currAddr, target);
                    if (ah.isADynamicCallInstruction()) {
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

                        targetFunc = bindCallTarget(target,currBlk,callTargets,
                                       preParseStubs);
                    }
                    calls.push_back( p );
                    currBlk->containsCall_ = true;
                }
                else
                {
                    parsing_printf(" ! call at 0x%lx rejected by isRealCall()\n",
                                    currAddr);
                    if( validTarget == false )
                    {
                        parsing_printf("... invalid call target\n");
                    }
                }

		        if (ah.isDelaySlot()) {
		            // Delay slots get skipped; effectively pinned to 
                    // the prev. insn.
		            ah++;
		        }

                if(targetFunc && (targetFunc->symTabName() == "exit" ||
                                  targetFunc->symTabName() == "abort"))
                { 
                    parsing_printf("Call to %s (%lx) detected at 0x%lx\n",
                                        targetFunc->symTabName().c_str(),
                                        target, currAddr);
                }
                else if(pltFuncs[target] == "exit" || 
                        pltFuncs[target] == "abort")
                {
                    parsing_printf("Call to %s (%lx) detected at 0x%lx\n",
                                        targetFunc->symTabName().c_str(),
                                        target, currAddr);
                }
                else
                {
                    Address next = ah.peekNext();
                    addBasicBlock(next,
                              currBlk,
                              leaders,
                              leadersToBlock,
                              ET_FUNLINK,
                              worklist,
                              visited);
                }
                break;
            }
            else if( ah.isALeaveInstruction() )
            {
                noStackFrame = false;
            }
            else if( archIsAbortOrInvalid(ah) )
            {
                // some architectures have invalid instructions, and
                // all have special 'abort-causing' instructions

                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                //currBlk->blockEndOffset_ = currAddr + insnSize;
            
                // FIXME should this really be an exit point if this
                // instruction causes the program to error and terminate?

                p = new image_instPoint( currAddr,
                                         ah.getInstruction(),
                                         this,
                                         functionExit);
                funcReturns.push_back( p );
                retStatus_ = RS_NORETURN;
                break;
            }
            #if defined(arch_ia64)
            else if( ah.isAnAllocInstruction() )
            {
                // IA64 only, sad to say
                if( currAddr == currBlk->firstInsnOffset() )
                {
                    allocs.push_back( currAddr ); 
                } 
                else
                {
                    currBlk->lastInsnOffset_ = ah.peekPrev();
                    currBlk->blockEndOffset_ = currAddr;

                    addBasicBlock(currAddr,
                                    currBlk,
                                    leaders,
                                    leadersToBlock,
                                    ET_FALLTHROUGH,
                                    worklist,
                                    visited);
                    break;
                }
            }
            #endif

            ah++;
        }
    }

    endOffset_ = funcEnd;

    return true;
}

/* bindCallTarget links the target address of a call to the function
 * and entry block to which it refers. If such a function/block pair
 * does not exist (or if the target is the middle of another function),
 * a new function will be created.
 *
 * Returns: pointer to function object that is target of call
 *
 * Side effects:
 *  - May append to callTargets
 *  - May add entry to preParseStubs
 *  - May split existing blocks
 *  - May mark existing blocks as shared
 */
image_func * image_func::bindCallTarget(
        Address target,
        image_basicBlock* currBlk,
        pdvector< Address >& callTargets,
        dictionary_hash< Address, image_func *>& preParseStubs)
{
    codeRange *tmpRange;
    image_basicBlock *ph_callTarget = NULL;
    image_basicBlock *newBlk;
    image_func *targetFunc;

    targetFunc = FindOrCreateFunc(target,callTargets,preParseStubs);

    if(image_->basicBlocksByRange.find(target,tmpRange))
    {
        if(tmpRange->get_size_cr() == 0)
        {
            printf("zero-size basic block found at 0x%lx\n",tmpRange->get_address_cr());
            assert(0);
        }

        ph_callTarget =       
            dynamic_cast<image_basicBlock*>(tmpRange);
        
        if(ph_callTarget->firstInsnOffset_ == target )
        {   
            addEdge(currBlk,ph_callTarget,ET_CALL);
            return targetFunc;
        }   
    }           
                
    // going to need a new basic block
    newBlk = new image_basicBlock(targetFunc,target);
                
    if(ph_callTarget)
    {                                    
        // the target lies within an existing block, which
        // must therefore be split.      
        ph_callTarget->split(newBlk);
    }           
    else
        newBlk->isStub_ = true;
            
    image_->basicBlocksByRange.insert(newBlk);
            
    addEdge(currBlk,newBlk,ET_CALL);
    
    return targetFunc;
}

/* FindOrCreateFunc
 *
 * Look up or create a new function for this image at a given address,
 * depending on whether one exists in the symbol table (or was previously
 * discovered).
 *
 * Returns: pointer to new function
 *
 * Side effects: 
 *  - May update preParseStubs
 *  - May update callTargets
 */
image_func * image_func::FindOrCreateFunc(Address target,
        pdvector< Address >& callTargets,
        dictionary_hash< Address, image_func *>& preParseStubs)
{
    image_func *targetFunc;

    if(image_->funcsByEntryAddr.defines(target))
    {   
        targetFunc = image_->funcsByEntryAddr[target];
    }
    else if(preParseStubs.defines(target))
    {   
        targetFunc = preParseStubs[target];
    }
    else
    {   
        char name[20];
        snprintf(name,20,"targ%lx",target);

        targetFunc = new image_func(name,target,UINT_MAX,mod_,image_);

        preParseStubs[target] = targetFunc;
        callTargets.push_back(target);
    }
    
    return targetFunc;
}

/* parseSharedBlocks handles the case where a reachable path of blocks
 * in a function have already been parsed by another function. All blocks
 * reachable from firstBlock will be added to the current function, and
 * will be updated to reflect their shared status.
 *
 * Call edges are, as in normal parsing, not followed.
 *
 * Side effects: (many)
 *  - Adds to leaders
 *  - Adds to leadersToBlock
 *  - Adds to parserVisited
 *  - May update other functions' "shared" status
 */
void image_func::parseSharedBlocks(image_basicBlock * firstBlock,
                BPatch_Set< Address > &leaders,
                dictionary_hash< Address, image_basicBlock * > &leadersToBlock,
                BPatch_Set< image_basicBlock* > &parserVisited)
{
    pdvector< image_basicBlock * > WL;
    pdvector< image_edge * > targets;

    BPatch_Set< image_basicBlock * > visited;

    image_basicBlock * curBlk;
    image_basicBlock * child;

    image_instPoint * tmpInstPt;
    image_instPoint * cpyInstPt;

    WL.push_back(firstBlock);
    visited.insert(firstBlock);

    parsing_printf("Parsing shared code at 0x%lx\n",firstBlock->firstInsnOffset_);

    // remember that we have shared blocks
    containsSharedBlocks_ = true;

    while(WL.size() > 0)
    {
        curBlk = WL.back();
        WL.pop_back(); 
  
        if(parserVisited.contains(curBlk))
            continue;

        // If the current block's list of owning functions only
        // contains one, that poor naive soul probably doesn't know it
        // contains any shared blocks. We should let it know.
        if(curBlk->funcs_.size() == 1)
        {
            curBlk->funcs_[0]->containsSharedBlocks_ = true;
            curBlk->funcs_[0]->needsRelocation_ = true;
        }

        // note that these have to occur before this function
        // is added to the block's ownership list
        if((tmpInstPt = curBlk->getCallInstPoint()) != NULL)
        {
            cpyInstPt = new image_instPoint(tmpInstPt->offset(),
                                    tmpInstPt->insn(),
                                    this,
                                    tmpInstPt->callTarget(),
                                    tmpInstPt->isDynamicCall());
            calls.push_back(cpyInstPt);    
        }
        if((tmpInstPt = curBlk->getRetInstPoint()) != NULL)
        {
            cpyInstPt = new image_instPoint(tmpInstPt->offset(),
                                    tmpInstPt->insn(),
                                    this,
                                    tmpInstPt->getPointType());
            funcReturns.push_back(cpyInstPt);
        }

        curBlk->addFunc(this);

        // update block
        blockList.push_back(curBlk);
        parsing_printf("XXXX adding pre-parsed block %d (0x%lx) to blocklist\n",
                curBlk->id(),curBlk->firstInsnOffset_);
        parserVisited.insert(curBlk);

        targets.clear();
        curBlk->getTargets(targets);

        for(unsigned int i=0;i<targets.size();i++) 
        {
            child = targets[i]->getTarget();

            if(targets[i]->getType() != ET_CALL && !visited.contains(child))
            {
                leaders += child->firstInsnOffset_;
                leadersToBlock[ child->firstInsnOffset_ ] = child;
                WL.push_back(child);
                visited.insert(child);
            }
        }
    }
    WL.zap();
    targets.zap();
}

/* A specialized version for parsing shared blocks when we don't care
 * to save any information about the blocks that were visited (that is,
 * when the entire function is known to be shared and so no other normal
 * parsing is taking place).
 */
void image_func::parseSharedBlocks(image_basicBlock * firstBlock)
{
    BPatch_Set< Address > leaders;
    BPatch_Set< image_basicBlock* > pv;
    dictionary_hash< Address, image_basicBlock * > leadersToBlock( addrHash );

    parseSharedBlocks(firstBlock,leaders,leadersToBlock,pv);  
}
