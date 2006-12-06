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
 * $Id: image-flowGraph.C,v 1.28 2006/12/06 21:17:22 bernat Exp $
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
#include "dyninstAPI/src/debug.h"

#include "dyninstAPI/src/InstrucIter.h"

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
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  stats_parse.startTimer(PARSE_ANALYZE_TIMER);

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
    stats_parse.stopTimer(PARSE_ANALYZE_TIMER);
    return true;
  }
  
  //pdvector<image_func *> new_functions;  
    pdvector<Address> new_targets;      // call targets detected in
                                        // parseStaticCallTargets

  // Parse the functions reported in the symbol table
  for (unsigned i = 0; i < everyUniqueFunction.size(); i++) {
    pdf = everyUniqueFunction[i];
    mod = pdf->pdmod();
    assert(pdf); assert(mod);
    pdstring name = pdf->symTabName();
    parseFunction( pdf, callTargets, preParseStubs);
        
    // these functions have not been added to the code range tree
    funcsByRange.insert(pdf);
  }      
 
  // callTargets now holds a big list of target addresses; some are already
  // in functions that we know about, some point to new functions. 
    while(callTargets.size() > 0)
    {
        // parse any new functions entered at addresses in callTargets
        parseStaticCallTargets( callTargets, new_targets, preParseStubs );
        callTargets.clear();
      
        VECTOR_APPEND(callTargets,new_targets); 
        new_targets.clear();
    }

#if defined(cap_stripped_binaries)
  
  int numIndir = 0;
  unsigned p = 0;
  
  image_func *func1, *func2;
  func1 = NULL;
  func2 = NULL;
  // We start over until things converge; hence the goto target
 top:

    while(callTargets.size() > 0)
    {
        // FIXME because of the assumption that functions in callTargets
        // will be successfully parsed, we may skip possible gaps because we
        // have advanced our p index too far.
        for( unsigned r = 0; r < callTargets.size(); r++ )
        {   
            if( func1 && callTargets[r] < func1->getOffset() )
                p++;
        }

        parseStaticCallTargets( callTargets, new_targets, preParseStubs );
        callTargets.clear(); 

        VECTOR_APPEND(callTargets,new_targets);
        new_targets.clear();
    }
   
    // nothing to do, exit
    if( everyUniqueFunction.size() <= 0 )
    {
        stats_parse.stopTimer(PARSE_ANALYZE_TIMER);
        return true;
    }

    VECTOR_SORT(everyUniqueFunction, addrfunccmp);
  
    Address lastPos;
    lastPos = everyUniqueFunction[0]->getOffset() + 
              everyUniqueFunction[0]->getSymTabSize();
  
    unsigned int rawFuncSize = everyUniqueFunction.size();
  
    for( ; p + 1 < rawFuncSize; p++ )
    {
        func1 = everyUniqueFunction[p];
        func2 = everyUniqueFunction[p + 1];
      
        Address gapStart = func1->getEndOffset();
        Address gapEnd = func2->getOffset();
        Address gap = gapEnd - gapStart;

        parsing_printf("searching for function prologue in gap (0x%lx - 0x%lx)\n", gapStart, gapEnd);
      
        //gap should be big enough to accomodate a function prologue
        if( gap >= 5 )
        {
            Address pos = gapStart;
            while( pos < gapEnd && isCode( pos ) )
            {
                const unsigned char* instPtr;
                instPtr = (const unsigned char *)getPtrToInstruction( pos );
              
                instruction insn;
                insn.setInstruction( instPtr, pos );

                if( isFunctionPrologue(insn) && !funcsByEntryAddr.defines(pos))
                {
                    char name[32];
                    numIndir++;
                    snprintf( name, 32, "gap_f%lx", pos );
                    pdf = new image_func( name, pos, UINT_MAX, mod, this);
                    if(parseFunction( pdf, callTargets, preParseStubs)) {
                        addFunctionName(pdf, name, true);
                        pdf->addPrettyName( name );
                        // No typed name
                  
                        enterFunctionInTables(pdf, false);

                        // If any calls were discovered, adjust our
                        // position in the function vector accordingly
                        if( callTargets.size() > 0 )
                        {   
                            goto top; //goto is the devil's construct. repent!! 
                        }
                        
                        pos = ( gapEnd < pdf->getEndOffset() ?
                                gapEnd : pdf->getEndOffset() );
                    }
                }
                pos++;
            }   
        }
    }
#endif

  // Sort block list and bind all intra-module call points
  for (unsigned b_iter = 0; b_iter < everyUniqueFunction.size(); b_iter++) {
      image_func *f = everyUniqueFunction[b_iter];
      if(!f->isBLSorted())
        f->sortBlocklist();
      everyUniqueFunction[b_iter]->checkCallPoints();
  }

  // TODO remove this; it is just for getting some cheap statistics

  //DumpAllStats();

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": analyzeImage of " << name_ << " took "<<dursecs <<" msecs" << endl;
#endif 

  
  parseState_ = analyzed;
  stats_parse.stopTimer(PARSE_ANALYZE_TIMER);
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
                     dictionary_hash< Address, image_func * > &preParseStubs)
{
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

                enterFunctionInTables(pdf,false);
                
                // mangled name
                addFunctionName(pdf, pdf->symTabName().c_str(), true);
                // Auto-adds to our list
                pdf->addPrettyName( pdf->symTabName().c_str() );
            }
        }
        else
        {
            parsing_printf("Call target 0x%lx does not have associated func\n",
                            callTargets[j]);
        } 
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
    instLevel_ = NORMAL;
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
        instLevel_ = UNINSTRUMENTABLE;
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
    BPatch_Set< Address > allInstAddrs;
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
    // have size > 1 is if we're on alpha.
    for(int j=funcEntries.size()-1; j >= 0; j--)
    {
        Address a = funcEntries[j]->firstInsnOffset();
        leaders += a;
        leadersToBlock[a] = funcEntries[j];
        worklist.push_back(a);
    }

    for(unsigned i=0; i < worklist.size(); i++)
    {
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
            parseSharedBlocks(currBlk, leaders, leadersToBlock, 
                              visited, funcEnd);
            continue;
        }

        // Remember where the next block is, so we don't blindly run over
        // the top of it when scanning through instructions.
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

            // The following three cases ensure that we properly handle
            // situations where our parsing comes across previously
            // pased code:
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

                    // add this function to block ownership list if we're
                    // rolling over an unparsed stub of another function
                    // (i.e., the next block is the unparsed entry block
                    // of some function)
                    if(nextExistingBlock->isStub_ && 
                       !nextExistingBlock->containedIn(this))
                    {
                        nextExistingBlock->addFunc(this);
                    }
                }
                break;
            }
#if defined(arch_x86) || defined(arch_x86_64)
            // These portions correspond only to situations that arise
            // because of overlapping but offset instruction streams --
            // the kind that only happen on x86.

            else if(allInstAddrs.contains( currAddr ))
            {
                // This address has been seen but is not the start of
                // a basic block. This has the following interpretation:
                // The current instruction stream has become aligned with
                // an existing (previously parsed) instruction stream.
                // This should only be possible on x86, and in normal code
                // it should only be seen when dealing with cases like
                // instruction prefixes (where the bytes of two different
                // versions of an instruction effectively overlap).

                // XXX Because our codeRangeTree has no support for
                // overlapping ranges, we are only capable of supporting the
                // case where the current instruction stream starts at a
                // lower value in the address space than the stream it
                // overlaps with. This is sufficient to handle the
                // prefixed instruction case (because of the order we
                // place blocks on the work list), but any other case
                // will make the function uninstrumentable.

                if(currAddr > currBlk->firstInsnOffset_) {
                    currBlk->lastInsnOffset_ = ah.peekPrev();
                    currBlk->blockEndOffset_ = currAddr;

                    // The newly created basic block will split
                    // the existing basic block that encompasses this
                    // address.
                    addBasicBlock(currAddr,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  ET_FALLTHROUGH,
                                  worklist,
                                  visited);
                } else {
                    parsing_printf(" ... uninstrumentable due to instruction stream overlap\n");
                    currBlk->lastInsnOffset_ = currAddr;
                    currBlk->blockEndOffset_ = ah.peekNext();
                    instLevel_ = UNINSTRUMENTABLE;
                }
                break;
            }
            else if(currAddr > nextExistingBlockAddr)
            {
                // Overlapping instructions on x86 can mean that we
                // can step over the start of an existing block because
                // our instruction stream does not exactly coincide with its
                // instruction stream. In that case, find the next existing
                // block address and reprocess this instruction.

                nextExistingBlockAddr = ULONG_MAX;
                if(image_->basicBlocksByRange.successor(currAddr,tmpRange))
                {
                    nextExistingBlock = 
                            dynamic_cast<image_basicBlock*>(tmpRange);

                    nextExistingBlockAddr = nextExistingBlock->firstInsnOffset_;
                }
                continue; // reprocess current instruction
            }
#endif

            allInstructions.push_back( ah.getInstruction() );
            allInstAddrs += currAddr;

            // only ever true on x86
            if(ah.isFrameSetup() && savesFP_)
            {
                noStackFrame = false;
            }

            // Special architecture-specific instruction processing
            archInstructionProc( ah );

            if( ah.isACondBranchInstruction() )
            {
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();

                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
                    //funcEnd = currAddr + insnSize;

                Address target = ah.getBranchTargetAddress();
                img()->addJumpTarget( target );

                if(ah.isDelaySlot())
                {
                    // Skip delay slot (sparc)
                    ah++;
                } 

                // Process target first, to prevent overlap between
                // the fallthrough block and the target block.
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
                                  ET_COND_TAKEN,
                                  worklist,
                                  visited);
                }

                Address t2 = ah.peekNext(); 
                //Address t2 = currAddr + insnSize;
               
                // If this branch instruction split the current block,
                // the fallthrough should be added to the newly created
                // block instead of this current block. 
                image_basicBlock * real_src;
                if(target > currBlk->firstInsnOffset_ && target <= currAddr) {
                    real_src = leadersToBlock[target];
                    assert(real_src);
                } else {
                    real_src = currBlk;
                }

                addBasicBlock(t2,
                              real_src,
                              leaders,
                              leadersToBlock,
                              ET_COND_NOT_TAKEN,
                              worklist,
                              visited);

                break;                              
            }
            else if( ah.isAIndirectJumpInstruction() )
            {
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                parsing_printf("... indirect jump at 0x%lx\n", currAddr);

                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
    
                numInsns = allInstructions.size() - 2;
                if( numInsns == 0 ) {
                    // this "function" is a single instruction long
                    // (a jmp to the "real" function)
                    parsing_printf("... uninstrumentable due to 0 size\n");
                    instLevel_ = UNINSTRUMENTABLE;
                    endOffset_ = startOffset_;
                    return false; 
                }                 

                if( archIsIndirectTailCall(ah) )
                {
                    // looks like a tail call
                    currBlk->isExitBlock_ = true;
                    p = new image_instPoint(currAddr,
                                            ah.getInstruction(),
                                            this,
                                            functionExit);
                    break;
                }

#if 0
                // Whether or not we are able to determine the targets
                // of this indirect branch instruction, it is not safe
                // to relocate this function.
                
                // We're now relocating by redirecting targets. We now only
                // label unrelocateable if the jump table can't be parsed.

                canBeRelocated_ = false;
#endif

                BPatch_Set< Address > targets;
                BPatch_Set< Address >::iterator iter;
               
                // get the list of targets
                if(!archGetMultipleJumpTargets(targets,currBlk,ah,
                                               allInstructions))
                {
                    instLevel_ = HAS_BR_INDIR;
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
                                      ET_INDIR,
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

                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();

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

                if(archIsATailCall( ah, allInstructions ))
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
                currBlk->isExitBlock_ = true;
                    
                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
                
                parsing_printf("... making new exit point at 0x%lx\n", currAddr);                
                p = new image_instPoint( currAddr, 
                                         ah.getInstruction(),
                                         this,
                                         functionExit);
                funcReturns.push_back( p );
                currBlk->containsRet_ = true;
                retStatus_ = RS_RETURN;

                if (ah.getInstruction().isCleaningRet()) {
                    cleansOwnStack_ = true;
                }

                break;
            }
            else if(ah.isACondReturnInstruction() )
            {
                parsing_printf("cond ret branch at 0x%lx\n", currAddr);
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();
                currBlk->isExitBlock_ = true;
                  
                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
                  
                // And make an inst point
                p = new image_instPoint(currAddr, 
                                        ah.getInstruction(),
                                        this,
                                        functionExit);
                parsing_printf("Function exit at 0x%x\n", *ah);
                funcReturns.push_back(p);
                currBlk->containsRet_ = true;
                retStatus_ = RS_RETURN;
                
                Address t2 = ah.peekNext();
                addBasicBlock(t2,
                              currBlk,
                              leaders,
                              leadersToBlock,
                              ET_FALLTHROUGH,
                              worklist,
                              visited);
                break;
            }
            else if( ah.isACallInstruction() ||
                     ah.isADynamicCallInstruction() )
            {
                currBlk->lastInsnOffset_ = currAddr;
                currBlk->blockEndOffset_ = ah.peekNext();

                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();

                parsing_printf("... 0x%lx is a call\n", currAddr);
               
                //validTarget is set to false if the call target is not a 
                //valid address in the applications process space 
                bool validTarget = true;
                //simulateJump is set to true if the call should be treated
                //as an unconditional branch for the purposes of parsing
                //(a special case)
                bool simulateJump = false;

                image_func *targetFunc = NULL;
                
                // XXX move this out of archIsRealCall protection... safe?
                bool isAbsolute = false;
                Address target = ah.getBranchTargetAddress(&isAbsolute);
                if ( archIsRealCall(ah, validTarget, simulateJump) )
                {
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
                                                 false,
                                                 isAbsolute);

                        targetFunc = bindCallTarget(target,currBlk,callTargets,
                                       preParseStubs);
                    }
                    calls.push_back( p );
                    currBlk->containsCall_ = true;

                } // real call
                else
                {
                    parsing_printf(" ! call at 0x%lx rejected by isRealCall()\n",
                                    currAddr);
                    if( validTarget == false )
                    {
                        parsing_printf("... invalid call target\n");
                        currBlk->canBeRelocated_ = false;
                        canBeRelocated_ = false;
                    }
                    else if( simulateJump )
                    {
                        addBasicBlock(target,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  ET_DIRECT,
                                  worklist,
                                  visited);
                    }
                }

		        if (ah.isDelaySlot()) {
		            // Delay slots get skipped; effectively pinned to 
                    // the prev. insn.
		            ah++;
		        }

                if(targetFunc && (targetFunc->symTabName() == "exit" ||
                                  targetFunc->symTabName() == "abort" ||
                                  targetFunc->symTabName() == "__f90_stop"))
                { 
                    parsing_printf("Call to %s (%lx) detected at 0x%lx\n",
                                    targetFunc->symTabName().c_str(),
                                    target, currAddr);
                }
                else if(pltFuncs[target] == "exit" || 
                        pltFuncs[target] == "abort" ||
                        pltFuncs[target] == "__f90_stop")
                {
                    parsing_printf("Call to %s (%lx) detected at 0x%lx\n",
                                    pltFuncs[target].c_str(),
                                    target, currAddr);
                }
                else if(!simulateJump)
                {
                    // we don't wire up a fallthrough edge if we're treating
                    // the call insruction as an unconditional branch
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

                if( currAddr >= funcEnd )
                    funcEnd = ah.peekNext();
            
                parsing_printf("... making new exit point at 0x%lx\n", currAddr);                
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

                    // remove some cumulative information
                    allInstAddrs.remove(currAddr);
                    allInstructions.pop_back();

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
            if (!img()->isValidAddress(ah.peekNext())) {
               //The next instruction is not in the.text segment.  We'll 
               // abort this basic block as if it were terminating with 
               // an illegal instruction.
               parsing_printf("Next instruction is invalid, ending basic block\n");
               
               currBlk->lastInsnOffset_ = currAddr;
               currBlk->blockEndOffset_ = ah.peekNext();

               if( currAddr >= funcEnd )
                  funcEnd = ah.peekNext();
                parsing_printf("... making new exit point at 0x%lx\n", currAddr);                
                p = new image_instPoint( currAddr,
                                         ah.getInstruction(),
                                         this,
                                         functionExit);
                funcReturns.push_back( p );
                retStatus_ = RS_NORETURN;
                break;
            }
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
        char name[32];
#if defined (os_windows)
        _snprintf(name,32,"targ%lx",target);
#else
        snprintf(name,32,"targ%lx",target);
#endif

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
                BPatch_Set< image_basicBlock* > &parserVisited,
                Address & funcEnd)
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

    parsing_printf("Parsing shared code at 0x%lx, blockList size: %ld startoffset: 0x%lx endoffset: 0x%lx\n",firstBlock->firstInsnOffset_, blockList.size(), startOffset_, endOffset_);

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
            parsing_printf("... copying exit point at 0x%lx\n", tmpInstPt->offset());                
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
        // update "function end"
        if(funcEnd < curBlk->blockEndOffset_)
            funcEnd = curBlk->blockEndOffset_;
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

    endOffset_ = startOffset_;
    parseSharedBlocks(firstBlock,leaders,leadersToBlock,pv,endOffset_);  
}
