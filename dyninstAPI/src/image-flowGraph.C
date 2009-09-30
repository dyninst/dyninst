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
 * $Id: image-flowGraph.C,v 1.54 2008/10/28 18:42:46 bernat Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include "symtabAPI/h/Symtab.h"
#if defined(cap_instruction_api)
#include "IA_IAPI.h"
#else
#include "IA_InstrucIter.h"
#endif
#include "InstructionAdapter.h"
#include "dyninstAPI/src/parRegion.h"

#include <fstream>
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"


#include "dyninstAPI/h/BPatch_flowGraph.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

using namespace Dyninst;

#if defined (cap_use_pdvector)
int addrfunccmp( image_func*& pdf1, image_func*& pdf2 )
{
    if ( pdf1->getOffset() > pdf2->getOffset() )
        return 1;
    if ( pdf1->getOffset() < pdf2->getOffset() )
        return -1;
    return 0;
}
#else
//stl needs strict weak ordering (true if arg 1 is strictly less than 2)
bool addrfunccmp( image_func* pdf1, image_func* pdf2 )
{
   if ( pdf1->getOffset() < pdf2->getOffset() )
      return true;
   return false;
#if 0
           if ( pdf1->getOffset() < pdf2->getOffset() )
                      return false;
               return false;
#endif
}
#endif

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

#if defined(arch_x86_64)
    ia32_set_mode_64(getObject()->getAddressWidth() == 8);
#endif
  
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
  
    // Parse the functions reported in the symbol table
    for(unsigned i=0; i<symtabCandidateFuncs.size(); ++i) {
        pdf = symtabCandidateFuncs[i];
        mod = pdf->pdmod();

        assert(pdf); assert(mod);

        // Check whether recursive traversal has dealt with a function
        // before proceeding to parse it
        if(!pdf->parsed()) {
            parsing_printf("[%s] calling parse() at 0x%lx\n",
                FILE__,pdf->getOffset());

            /** Symtab-defined functions are already so ingrained in our
                data structures at this point that they must be entered
                into to the rest regardless of whether they were
                parsed correctly or not. **/

            enterFunctionInTables(pdf);
    
            if(!pdf->parse()) {
                parsing_printf("[%s] symtab-defined function %s at 0x%lx "
                               "failed to parse\n",FILE__,
                    pdf->symTabName().c_str(),pdf->getOffset());
            }
        }
    }

    /* Some functions that built their block lists through the
       `shared code' mechanism may be incomplete. Finish them. */
    parsing_printf("[%s] Completing `shared code' parsing: %u entries\n",
        FILE__,reparse_shared.size());
    for(unsigned i=0;i<reparse_shared.size();++i) {
        pair<image_basicBlock *,image_func*> & p = reparse_shared[i];
        (p.second)->parseSharedBlocks(p.first);
    }

  /* 
     OPENMP PARSING CODE

     For starters, we have all functions with some
     sort of indentifier in the function name ('@' for Power),
     designanting an outlined OpenMP Function.  This is contained in the
     'parallelRegions' variable.  We then want to find out the parent function
     for each of these regions, i.e. the function that sets all the parameters
     for a given OpenMP function.  We need this parent function to gather
     information about what kind of directive is associated with this outline
     function and the clauses associated with that function
  */

#if defined(os_solaris) || defined(os_aix)
  image_parRegion *parReg;
  int currentSectionNum = 0;

  // Most of the time there will be no parallel regions in an image, so nothing will happen.
  // If there is a parallel region we examine it individually 
  for (unsigned i = 0; i < parallelRegions.size(); i++) 
    {
      parReg = parallelRegions[i];
      if (parReg == NULL)
	continue;
      else
	{
	  // Every parallel region has the image_func that contains the
	  //   region associated with it 
	  image_func * imf = parReg->getAssociatedFunc();
	  
	  // Returns pointers to all potential image_funcs that correspond
	  //   to what could be the parent OpenMP function for the region 
	  const pdvector<image_func *> *prettyNames = 
	    findFuncVectorByPretty(imf->calcParentFunc(imf, parallelRegions));
	  
	  //There may be more than one (or none) functions with that name, we take the first 
	  // This function gives us all the information about the parallel region by getting
	  // information for the parallel region parent function 
	  if (prettyNames->size() > 0)
	    imf->parseOMP(parReg, (*prettyNames)[0], currentSectionNum);
	  else
	    continue;
	}
    }
    
  /**************************/
  /* END OpenMP Parsing Code */
  /**************************/
  
#endif 

#if 0 // stripped binary parsing in its current form is dangerous
#if defined(cap_stripped_binaries)
  if (parseGaps_) {
     mod = getOrCreateModule("DEFAULT_MODULE", linkedFile->imageOffset());
    // Sort functions to find gaps
    VECTOR_SORT(everyUniqueFunction, addrfunccmp);

    Address gapStart = linkedFile->imageOffset();
    Address gapEnd;
    int funcIdx = 0; // index into everyUniqueFunction (which may be empty)
    do {
        /* The everyUniqueFunction vector can be updated in the for
           loop.  If functions are discovered preceeding the one
           pointed to by the current 'funcIdx' index, we need to
           advance funcIdx and gapStart to the correct position */
        /* (It would be better to use a data structure with an iterator
           that doesn't break on inserts, such as an ordered set) */
       while(funcIdx < (int) everyUniqueFunction.size() && 
             everyUniqueFunction[funcIdx]->getEndOffset() < gapStart)
          {
             /*parsing_printf("[%s:%u] advancing gap index (gapstart: 0x%lx, "
               "offset: 0x%lx)\n", FILE__,__LINE__,
               gapStart,everyUniqueFunction[funcIdx]->getOffset());*/
             funcIdx++;
          }
       if (funcIdx > 0 && funcIdx < (int) everyUniqueFunction.size() 
           && gapStart < everyUniqueFunction[funcIdx]->getEndOffset()) {
          gapStart = everyUniqueFunction[funcIdx]->getEndOffset();
       }
       
       // set gapEnd to start of next function or end of code section, 
       // on 1st iter, set gapEnd to start of 1st function or end of code section
       if (funcIdx == 0 && everyUniqueFunction.size() > 0) {
          gapEnd = everyUniqueFunction[0]->getOffset();
       }
       else if(funcIdx+1 < (int)everyUniqueFunction.size()) {
          gapEnd = everyUniqueFunction[funcIdx + 1]->getOffset();
       } 
       // if there is no next function, set gapEnd to end of codeSection
       else if (funcIdx+1 == (int) everyUniqueFunction.size() || funcIdx == 0) {
          gapEnd = linkedFile->imageOffset() + linkedFile->imageLength();
       }
       else {  
          break; // advanced gap past the end of the code section
       }
       
       int gap = gapEnd - gapStart;
       
       parsing_printf("[%s:%u] scanning for prologues in range 0x%lx-0x%lx\n",
                      FILE__,__LINE__,gapStart, gapEnd);
       
       //gap should be big enough to accomodate a function prologue
       if(gap >= 5) {
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
                      snprintf( name, 32, "gap_%lx", pos );
                      pdf = new image_func( name, pos, UINT_MAX, mod, this);
                      
                      if(pdf->parse()) {
                         pdf->addSymTabName(name); // newly added
                         pdf->addPrettyName( name );
                         
                         // Update size
                         // TODO FIXME: should update _all_ symbol sizes....
                         pdf->getSymtabFunction()->getFirstSymbol()->setSize(pdf->get_size());

                         enterFunctionInTables(pdf, false);
                        
                        // FIXME this code has not been updated to the reality
                        // of RT parsing at this point, and will not work
                        // or even compile  

                         // If any calls were discovered, adjust our
                         // position in the function vector accordingly
                         if(callTargets.size() > 0) {            
                            while( callTargets.size() > 0 )
                               {   
                                  /* parse static call targets */
                                  parseStaticCallTargets( callTargets, 
                                                          new_targets, preParseStubs );
                                  callTargets.clear(); 
                                  
                                  VECTOR_APPEND(callTargets,new_targets);
                                  new_targets.clear();
                               }
                            // Maintain sort
                            VECTOR_SORT(everyUniqueFunction, addrfunccmp);
                            break;  // Return to for loop
                         } else {
                            VECTOR_SORT(everyUniqueFunction, addrfunccmp);
                         }
                         
                         pos = ( gapEnd < pdf->getEndOffset() ?
                                 gapEnd : pdf->getEndOffset() );
                      } else {
                         pos++;
                      }
                   } else
                      pos++;
             }// end while loop
       }
       // set gapStart for next iteration & increment funcIdx
       if (funcIdx < (int)everyUniqueFunction.size()) {
          gapStart = everyUniqueFunction[funcIdx]->getEndOffset();
       }
       funcIdx++;
    } while (funcIdx < (int)everyUniqueFunction.size());
  } // end gap parsing
#endif
#endif // if 0

    // Bind all intra-module call points
    for (unsigned b_iter = 0; b_iter < everyUniqueFunction.size(); b_iter++) {
       everyUniqueFunction[b_iter]->checkCallPoints();
    }
    
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

void image::recordFunction(image_func *f)
{
    // update symtab's notion of "size"
    // TODO FIXME: should update _all_ symbol sizes....
    f->getSymtabFunction()->getFirstSymbol()->setSize(f->get_size());

    enterFunctionInTables(f);

    // update names
    // XXX this first call to addSymTabName certainly looks redundant...
    f->addSymTabName(f->symTabName().c_str());
    f->addPrettyName(f->symTabName().c_str() );
}

/* parse is responsible for initiating parsing of a function. instPoints
 * and image_basicBlocks are created at this level.
 *
 * Returns: success or failure
 */
bool image_func::parse()
{
    pdvector< image_basicBlock * > entryBlocks_;

    if(parsed_)
    {
        parsing_printf("[%s] multiple call of parse() for %s\n", FILE__,
                symTabName().c_str());
        return false;
    }
    parsed_ = true;

    parsing_printf("[%s] parsing %s at 0x%lx\n", FILE__,
                symTabName().c_str(), getOffset());

    if(!img()->isValidAddress(getOffset())) {
        parsing_printf("[%s] refusing to parse from invalid address 0x%lx\n",
            FILE__,getOffset());
        return false;
    }

    // Some architectures have certain functions or classes of functions
    // that should appear to be parsed without actually being parsed.
    if(archAvoidParsing())
    {
        retStatus_ = RS_UNKNOWN;
        return true;
    }

    // For some architectures, some functions are recognizably unparseable 
    // from the start.
    if(archIsUnparseable())
    {
        retStatus_ = RS_UNKNOWN;
        parsing_printf("[%s] archIsUnparseable denied parse at 0x%lx\n",
            FILE__,getOffset());
        return false;
    }

    // Optimistic assumptions
    instLevel_ = NORMAL;
    canBeRelocated_ = true;

    noStackFrame = true;
    image_instPoint *p;

    Address funcBegin = getOffset();
    Address funcEntryAddr = funcBegin;

#if defined(cap_instruction_api)
    using namespace Dyninst::InstructionAPI;
    typedef IA_IAPI InstructionAdapter_t;
    const unsigned char* bufferBegin = (const unsigned char*)(img()->getPtrToInstruction(funcBegin));
    InstructionDecoder dec(bufferBegin, -1 - (Address)(bufferBegin));
    dec.setMode(img()->getAddressWidth() == 8);
    IA_IAPI ah(dec, funcBegin, this);
#else
    typedef IA_InstrucIter InstructionAdapter_t;
    InstrucIter ii(funcBegin, this);
    IA_InstrucIter ah(ii, this);
#endif
    

    // Test for various reasons we might just give up and die
    if( !ah.checkEntry() )
    {
        // Check for redirection to another function as first insn.
        // XXX This is a heuristic that we should revisit; is it really
        //     a common idiom for redirection to another function? Why
        //     do we need to treat it that way? -- nate & kevin
#if 0
        if( ah.isAJumpInstruction() )
        {
            Address target = ah.getCFT();

            parsing_printf("[%s:%u] direct jump to 0x%lx at entry point\n",
                FILE__,__LINE__,target);

            if(img()->isCode(target)) {
                // Recursively parse this function
                bindCallTarget(target,NULL);
            } else {
                // Create instrumentation point for on-demand parsing
                p = new image_instPoint(*ah,
                                        ah.getInstruction(),
                                        this,
                                        target,
                                        false,
                                        false,
                                        otherPoint); 
                pdmod()->addUnresolvedControlFlow(p);
            }
        }
#endif
        if(ah.isBranch() && !ah.isConditional())
        {
            Address target = ah.getCFT();
            parsing_printf("[%s] direct jump to 0x%lx at entry point\n",
                                FILE__,target);
    
            if(img()->isCode(target)) {
            // Recursively parse this function
                bindCallTarget(target,NULL);
            } else {
        // Create instrumentation point for on-demand parsing
                p = new image_instPoint(ah.getAddr(),
                                        ah.getInstruction(),
                                        this,
                                        target,
                                        false,
                                        false,
                                        otherPoint);
                pdmod()->addUnresolvedControlFlow(p);
            }
            endOffset_ = ah.getNextAddr();
            instLevel_ = UNINSTRUMENTABLE;

            retStatus_ = RS_UNKNOWN;
            parsing_printf("[%s] archCheckEntry denied parse at 0x%lx\n",
                            FILE__,getOffset());
            return false;
        }
    }

    // Define entry point
    p = new image_instPoint(funcEntryAddr,
                            ah.getInstruction(),
                            this,
                            functionEntry);
    funcEntries_.push_back(p);

    int frameSize;
    if(ah.isStackFramePreamble(frameSize))
    {
        archSetFrameSize(frameSize);
        noStackFrame = false;
    }
    savesFP_ = ah.savesFP();    // only ever true on x86

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
            } else {
                ph_entryBlock->addFunc(this);
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

#if 0
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
#endif
    // Note that the current function is in-progess to avoid
    // spurious duplication for loops in the call graph
    img()->activelyParsing[getOffset()] = this;

    if(preParsed)
    {
        parseSharedBlocks(ph_entryBlock);
    }
    else
    {
        buildCFG(entryBlocks_, funcBegin);
    }

    finalize();

    // done parsing
    img()->activelyParsing.undef(getOffset());

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
 * Architecture Independance:
 *  All architecture-specific code is abstracted away in the image-arch
 *  modules.
 */
bool image_func::buildCFG(
        pdvector<image_basicBlock *>& funcEntries,
        Address funcBegin)
{
    // Parsing worklist
    pdvector< Address > worklist;

    // Basic block lookup
    BPatch_Set< Address > leaders;
    BPatch_Set< Address > allInstAddrs;
    dictionary_hash< Address, image_basicBlock* > leadersToBlock( addrHash );

    // Prevent overrunning an existing basic block in linear scan
    Address nextExistingBlockAddr;
    image_basicBlock *nextExistingBlock = NULL;

    // Instructions and InstPoints
#if defined(cap_instruction_api)
    using namespace Dyninst::InstructionAPI;
    pdvector< instruction > allInstructions;
    typedef IA_IAPI InstructionAdapter_t;
#else
    pdvector< instruction > allInstructions;
    typedef IA_InstrucIter InstructionAdapter_t;
#endif
    typedef std::pair< Address, EdgeTypeEnum > edge_pair_t;
    typedef pdvector< edge_pair_t > Edges_t;
    image_instPoint *p;
    int insnSize;

    // misc
    codeRange *tmpRange;

    // ** Use to special-case abort and exit calls
    dictionary_hash<Address, std::string> *pltFuncs = img()->getPltFuncs();
    
    // ** end abort/exit setup

    Address funcEnd = funcBegin;
    Address currAddr = funcBegin;
    isPLTFunction_ = false;

    // The reverse ordering here is to make things easier on
    // alpha. Indeed, the only reason funcEntries would ever
    // have size > 1 is if we're on alpha.
    for(int j=funcEntries.size()-1; j >= 0; j--)
    {
        Address a = funcEntries[j]->firstInsnOffset();
        leaders += a;
        leadersToBlock[a] = funcEntries[j];
        worklist.push_back(a);

        if (pltFuncs->defines(a)) {
           isPLTFunction_ = true;
        }
    }

    for(unsigned i=0; i < worklist.size(); i++)
    {
#if defined(cap_instruction_api)
        using namespace Dyninst::InstructionAPI;
        const unsigned char* bufferBegin = (const unsigned char*)(img()->getPtrToInstruction(worklist[i]));
        InstructionDecoder dec(bufferBegin, -1 - (Address)(bufferBegin));
        dec.setMode(img()->getAddressWidth() == 8);
        InstructionAdapter_t ah(dec, worklist[i], this);
#else        
        InstrucIter iter(worklist[i],this);
        InstructionAdapter_t ah(iter, this);
#endif        
        image_basicBlock* currBlk = leadersToBlock[worklist[i]];

        parsing_printf("[%s] parsing block at 0x%lx, "
                       "first insn offset 0x%lx\n",
                       FILE__,
                       worklist[i], 
                       currBlk->firstInsnOffset());

        // debuggin' 
        assert(currBlk->firstInsnOffset() == worklist[i]);

        // If this function has already parsed the block, skip
        if(containsBlock(currBlk))
            continue;

        if(currBlk->isStub_)
        {
            currBlk->isStub_ = false;
            addToBlocklist(currBlk);

            parsing_printf("- adding block %d (0x%lx) to blocklist\n",
                           currBlk->id(), currBlk->firstInsnOffset());
        }
        else
        {
            // non-stub block must have previously been parsed
            parseSharedBlocks(currBlk, leaders, leadersToBlock, funcEnd);
            continue;
        }

        // Remember where the next block is, so we don't blindly run over
        // the top of it when scanning through instructions.
        nextExistingBlockAddr = ULONG_MAX;
        if(image_->basicBlocksByRange.successor(ah.getNextAddr(),tmpRange))
        {
            nextExistingBlock = dynamic_cast<image_basicBlock*>(tmpRange);
            if(nextExistingBlock->firstInsnOffset_ > worklist[i])
            {
                nextExistingBlockAddr = nextExistingBlock->firstInsnOffset_;
            }
        }
        bool isNopBlock = ah.isNop();

        while(true) // instructions in block
        {
            currAddr = ah.getAddr();
            insnSize = ah.getSize();

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
                Address prevAddr = ah.getPrevAddr();
                currBlk->lastInsnOffset_ = prevAddr;
                currBlk->blockEndOffset_ = currAddr;

                addEdge(currBlk,nextExistingBlock,ET_FALLTHROUGH);

                if(!containsBlock(nextExistingBlock))
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
                    currBlk->lastInsnOffset_ = ah.getPrevAddr();
                    currBlk->blockEndOffset_ = currAddr;

                    //Three blocks involved here, the original block, the new one that we're
                    //currently parsing and just found to overlap, and the new block we're creating
                    //at the point where these two blocks overlap.
                    codeRange *origBlock_CR;
                    image_->basicBlocksByRange.find(currAddr, origBlock_CR);
                    image_basicBlock *origBlock = dynamic_cast<image_basicBlock *>(origBlock_CR);
                    assert(origBlock);

                    // The newly created basic block will split
                    // the existing basic block that encompasses this
                    // address.
                    addBasicBlock(currAddr,
                                  currBlk,
                                  leaders,
                                  leadersToBlock,
                                  ET_FALLTHROUGH,
                                  worklist);
                    parsing_printf(" ... Found split instruction stream, currAddr = %lx\n", currAddr);
                    markAsNeedingRelocation(true);

                    currBlk->markAsNeedingRelocation();
                    origBlock->markAsNeedingRelocation();
                } else {
                    parsing_printf(" ... uninstrumentable due to instruction stream overlap\n");
                    currBlk->lastInsnOffset_ = currAddr;
                    currBlk->blockEndOffset_ = ah.getNextAddr();
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


            // only ever true on x86
            if(savesFP_ && noStackFrame && ah.isFrameSetupInsn())
            {
                noStackFrame = false;
            }

            // Special architecture-specific instruction processing
            archInstructionProc( ah );

            if (isNopBlock) 
            {
                if(!ah.isNop())
                {
                    currBlk->lastInsnOffset_ = ah.getPrevAddr();
                    currBlk->blockEndOffset_ = currAddr;
                    addBasicBlock(currAddr,
                                 currBlk,
                                 leaders,
                                 leadersToBlock,
                                 ET_FALLTHROUGH,
                                 worklist);
                    // Start over at currAddr
                    break;
                    
                }
            }
            allInstructions.push_back(instruction() );
            allInstAddrs += currAddr;

            if(ah.hasCFT())
            {
                markBlockEnd(currBlk, ah, funcEnd);
                Edges_t edges_out;
                ah.getNewEdges(edges_out, currBlk, allInstructions,
                               pltFuncs);
                InstrumentableLevel insnInstLevel = ah.getInstLevel(allInstructions);
                FuncReturnStatus insnRetStatus = ah.getReturnStatus(currBlk, allInstructions);
                instPointType_t insnPointType = ah.getPointType(allInstructions, pltFuncs);
                bool isDynamicCall = ah.isDynamicCall();
                bool isAbsoluteCall = ah.isAbsoluteCall();
                bool hasUnresolvedCF = ah.hasUnresolvedControlFlow(currBlk,
                        allInstructions);
                if(insnInstLevel >= instLevel_) {
                    switch(insnInstLevel)
                    {
                        case NORMAL: 
                                break;
                        case HAS_BR_INDIR:
                            //parsing_printf("[%s]: Setting %s to HAS_BR_INDIR at 0x%x\n", FILE__,
                            //               prettyName().c_str(), currAddr);
                            break;
                        case UNINSTRUMENTABLE:
//                             parsing_printf("[%s]: Setting %s to UNINSTRUMENTABLE at 0x%x\n", FILE__,
//                                            prettyName().c_str(), currAddr);
                            break;
                        default:
                            assert(!"Bad inst level enum!\n");
                            break;
                    }
                    
                    instLevel_ = insnInstLevel;
                }
                if(!ah.isRelocatable(insnInstLevel))
                {
/*                    parsing_printf("%s[%d]: setting relocatable FALSE at 0x%x\n",
                                   FILE__, __LINE__, currAddr);*/
                    currBlk->canBeRelocated_ = false;
                    canBeRelocated_ = false;
                }
                if(insnRetStatus == RS_RETURN)
                {
                    parsing_printf("... 0x%x (%d) is a return\n", currAddr, currAddr - getOffset());
                    if(retStatus_ != RS_UNKNOWN)
                        retStatus_ = insnRetStatus;
                }
                if(insnRetStatus == RS_UNKNOWN)
                {
/*                    parsing_printf("[%s]: Setting return status to RS_UNKNOWN (URCF) at 0x%x\n", FILE__,
                                   currAddr);*/
                    retStatus_ = insnRetStatus;
                }
                image_func* targetFunc = NULL;
                Address fix_cond_branch = 0;
                for(Edges_t::iterator curEdge = edges_out.begin();
                    curEdge != edges_out.end();
                   ++curEdge)
                {
                    if(!img()->isValidAddress(curEdge->first) ||
                        !img()->isCode(curEdge->first))
                    {
                        hasUnresolvedCF = true;
                        if(insnPointType == noneType)
                        {
                            insnPointType = otherPoint;
                        }
                    }
                    else
                    {
                        if(curEdge->second == ET_NOEDGE)
                        {
                            if(!isDynamicCall)
                            {
                                parsing_printf("[%s] binding call 0x%lx -> 0x%lx\n",
                                               FILE__,currAddr, curEdge->first);
                                targetFunc = bindCallTarget(curEdge->first,currBlk);
                                if(ah.isTailCall(allInstructions))
                                {
                                    parsing_printf("%s: tail call %x -> %x inheriting return status of target\n",
                                            FILE__, currAddr, curEdge->first);
                                    if(targetFunc && retStatus_ == RS_UNSET &&
                                       targetFunc->returnStatus() != RS_NORETURN)
                                    {
                                        retStatus_ = targetFunc->returnStatus();
                                    }
                                }
                            }
                            else
                            {
                                parsing_printf("[%s] dynamic call 0x%lx -> ?\n", FILE__, currAddr);
                            }
                            continue;
                        }
                        if(curEdge->second == ET_FUNLINK)
                        {
                            Address target = 0;
                            Edges_t::iterator e = edges_out.begin();
                            while(e != edges_out.end() && e->second != ET_NOEDGE)
                            {
                                ++e;
                            }
                            if(e != edges_out.end())
                            {
                                target = e->first;
                            }
                            bool isInPLT = false;
                            std::string pltName = "";
                            if((*pltFuncs).defines(target))
                            {
                                isInPLT = true;
                                pltName = (*pltFuncs)[target];
                            }
                            if(isNonReturningCall(targetFunc, isInPLT,
                                                  pltName, currAddr, target))
                            {
                                //parsing_printf("%s: non-returning call detected %x->%x\n",
                                //               FILE__, currAddr, target);
                                continue;
                            }
                        }
                        image_basicBlock * real_src = currBlk;
                        // This breaks on direct edges--why?
                        if(curEdge->second == ET_COND_TAKEN)
                        {
                            if(curEdge->first > currBlk->firstInsnOffset_ &&
                            curEdge->first <= currAddr) {
                                fix_cond_branch = curEdge->first;
                            }
                        }
                        if(fix_cond_branch && (curEdge->second == ET_COND_NOT_TAKEN))
                        {
                            real_src = leadersToBlock[fix_cond_branch];
                            assert(real_src);
                        }
#ifdef VERBOSE_EDGE_LOG                        
                        switch(curEdge->second)
                        {
                            case ET_FUNLINK:
                                parsing_printf("%s[%d]: adding function fallthrough edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            case ET_FALLTHROUGH:
                                parsing_printf("%s[%d]: adding fallthrough edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            case ET_COND_TAKEN:
                                parsing_printf("%s[%d]: adding conditional taken edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            case ET_COND_NOT_TAKEN:
                                parsing_printf("%s[%d]: adding conditional not taken edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            case ET_INDIR:
                                parsing_printf("%s[%d]: adding indirect edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            case ET_DIRECT:
                                parsing_printf("%s[%d]: adding direct edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            case ET_CATCH:
                                parsing_printf("%s[%d]: adding catch block edge %x->%x\n",
                                               FILE__, __LINE__, currAddr, curEdge->first);
                                break;
                            default:
                                parsing_printf("%s[%d]: adding unknown edge type %d edge %x->%x\n",
                                               FILE__, __LINE__, curEdge->second, currAddr, curEdge->first);
                                break;
                        }
#endif // VERBOSE_EDGE_LOG
                        addBasicBlock(curEdge->first,
                                    real_src,
                                    leaders,
                                    leadersToBlock,
                                    curEdge->second,
                                    worklist);
                    }
                }
                if(insnPointType != noneType)
                {
                    Address target = 0;
                    if(edges_out.size() && !isDynamicCall)
                    {
                        target = edges_out.begin()->first;
                    }
                    p = new image_instPoint( currAddr,
                                             ah.getInstruction(),
                                             this,
                                             target,
                                             isDynamicCall,
                                             isAbsoluteCall,
                                             insnPointType);
/*                    parsing_printf("%s[%d]: creating inst point at 0x%lx...", FILE__, __LINE__, currAddr);*/
                    if(hasUnresolvedCF)
                    {
/*                        parsing_printf("unresolved control flow, adding to unresolved list\n");*/
                        pdmod()->addUnresolvedControlFlow(p);
                        if(!isDynamicCall && (insnRetStatus != RS_UNKNOWN))
                        {
/*                            parsing_printf("[%s] setting return status to RS_UNKNOWN (URCF) at 0x%x\n",
                                            FILE__, currAddr);*/
                            retStatus_ = RS_UNKNOWN;
                        }
                    }
                    switch(insnPointType)
                    {
                        case functionExit:
                            currBlk->isExitBlock_ = true;
                            if(insnRetStatus == RS_RETURN)
                            {
                                parsing_printf("... making new exit point at 0x%x\n", currAddr);
                                funcReturns.push_back( p );
                                currBlk->containsRet_ = true;
                            }
                            else
                            {
                                parsing_printf("... making new exit point at 0x%x\n", currAddr);
                                //parsing_printf("return status bad, exit point, adding to returns--POSSIBLY WRONG\n");
                                funcReturns.push_back( p );
                                currBlk->containsRet_ = true;
                            }
                            break;
                        case callSite:
/*                            parsing_printf("call point, adding to calls\n");*/
                            calls.push_back( p );
                            currBlk->containsCall_ = true;
                            break;
                        case otherPoint:
/*                            parsing_printf("other point...");*/
                            if(hasUnresolvedCF) {
/*                                parsing_printf("already in unresolved CF list\n");*/
                            } else {
/*                                parsing_printf("where'd it go?\n");*/
                            }
                            break;
                        default:
                            assert(!"ERROR: unrecognized inst point type, not handling it correctly\n");
                            break;
                                    
                    };
                }
                if(ah.isDelaySlot())
                {
                    ah.advance();
                }
                if(!cleansOwnStack_ && ah.cleansStack())
                {
                    cleansOwnStack_ = true;
                }
                break;
            }
            else if(noStackFrame && ah.isLeave())
            {
                noStackFrame = false;
            }
            else if( ah.isAbortOrInvalidInsn() )
            {
                // some architectures have invalid instructions, and
                // all have special 'abort-causing' instructions
                markBlockEnd(currBlk, ah, funcEnd);
                break;
            }
#if defined(arch_ia64)
            else if( ah.isAllocInsn())
            {
                // IA64 only, sad to say
                if( currAddr == currBlk->firstInsnOffset() )
                {
                    allocs.push_back( currAddr ); 
                } 
                else
                {
                    // We weren't extending the function's end address here.
                    // That was probably incorrect.
                    markBlockEnd(currBlk, ah, funcEnd);
                    //currBlk->lastInsnOffset_ = ah.prevAddr();
                    //currBlk->blockEndOffset_ = currAddr;

                    // remove some cumulative information
                    
                    allInstAddrs.remove(currAddr);
                    allInstructions.pop_back();
                    parsing_printf("%s[%d]: ending block due to ALLOC at %x\n", FILE__, __LINE__, currAddr);
                    addBasicBlock(currAddr,
                                    currBlk,
                                    leaders,
                                    leadersToBlock,
                                    ET_FALLTHROUGH,
                                    worklist);
                    break;
                }
            }
#endif
            if (!img()->isValidAddress(ah.getNextAddr())) {
               //The next instruction is not in the.text segment.  We'll 
               // abort this basic block as if it were terminating with 
               // an illegal instruction.
               parsing_printf("Next instruction is invalid, ending basic block\n");
               
               markBlockEnd(currBlk, ah, funcEnd);
                parsing_printf("...making new exit point at 0x%lx\n", currAddr);
                p = new image_instPoint( currAddr,
                                         ah.getInstruction(),
                                         this,
                                         functionExit);
                funcReturns.push_back( p );
                break;
            }
            ah.advance();
        }
    }

    endOffset_ = funcEnd;

    // If the status hasn't been updated to UNKNOWN or RETURN by now,
    // set it to RS_NORETURN
    if(retStatus_ == RS_UNSET)
        retStatus_ = RS_NORETURN;

    parsing_printf("Setting %s retStatus_ to %d \n", prettyName().c_str(), retStatus_); 
    return true;
}

/* bindCallTarget links the target address of a call to the function
 * and entry block to which it refers. If such a function/block pair
 * does not exist (or if the target is the middle of another function),
 * a new function will be created and parsed.
 *
 * Returns: pointer to the [parsed] function object that is target of call
 *
 * Side effects:
 *  - May split existing blocks
 *  - May mark existing blocks as shared
 *  - May recursively parse one or more functions
 *
 * This function can be called with NULL currBlk as a shorthand for
 * finding or recursively parsing the target
 */
image_func * image_func::bindCallTarget(
        Address target,
        image_basicBlock* currBlk)
    {
    codeRange *tmpRange;
    image_basicBlock *ph_callTarget = NULL;
    image_basicBlock *newBlk;
    image_func *targetFunc;
    bool created = false;

    // targetfunc may be parsed or unparsed, and it may not yet have
    // an entry basic block associated with it. `created' indicates
    // whether a new image_func was created
    targetFunc = FindOrCreateFunc(target,FS_RT,created);

    if(image_->basicBlocksByRange.find(target,tmpRange))
    {
        ph_callTarget =       
            dynamic_cast<image_basicBlock*>(tmpRange);
    }
        
    if(ph_callTarget && ph_callTarget->firstInsnOffset_ == target )
    {   
        if(currBlk)
            addEdge(currBlk,ph_callTarget,ET_CALL);
    }           
    else {
        // going to need a new basic block
        newBlk = new image_basicBlock(targetFunc,target);

        if(ph_callTarget) {
            // The target lies within an existing block, which
            // must therefore be split
            ph_callTarget->split(newBlk);
        }
        else
            newBlk->isStub_ = true;
    
        image_->basicBlocksByRange.insert(newBlk);

        if(currBlk)
            addEdge(currBlk,newBlk,ET_CALL);
    }

    // Now parse the function, if necessary                
    if(!targetFunc->parsed()) {
        assert( targetFunc->img()->isCode(targetFunc->getOffset()) );

        parsing_printf("[%s] recursive parsing of call target at 0x%lx\n",
                FILE__,targetFunc->getOffset());

        if(targetFunc->parse()) {
            parsing_printf("[%s] recursive parsing of 0x%lx complete\n",
                FILE__,targetFunc->getOffset());

            targetFunc->img()->recordFunction(targetFunc);
    	     parsing_printf("[%s] call target at 0x%lx parsed - return value %d \n",
		FILE__,targetFunc->getOffset(), targetFunc->returnStatus());
        } else {
            parsing_printf("[%s] recursive parsing of 0x%lx failed\n",
                FILE__,targetFunc->getOffset());

            /* XXX Symtab-declared functions need to be added to the tables
                   to maintain consistency */
            if(targetFunc->howDiscovered() == FS_SYMTAB) {
                parsing_printf("[%s:%u] symtab-defined function %s at 0x%lx "
                               "failed to parse\n",FILE__,__LINE__,
                     targetFunc->symTabName().c_str(),targetFunc->getOffset());

                targetFunc->img()->recordFunction(targetFunc);
            }

            targetFunc = NULL;
        }

        parsing_printf("[%s] resuming parsing of %s\n",
            FILE__,prettyName().c_str());
    } 
    else { 
    	parsing_printf("[%s] call target at 0x%lx is already parsed - return value %d \n",
		FILE__,targetFunc->getOffset(), targetFunc->returnStatus());
    }
    return targetFunc;
}

/* FindOrCreateFunc
 *
 * Look up or create a new function for this image at a given address,
 * depending on whether one exists in the symbol table (or was previously
 * discovered).
 *
 * Returns: pointer to an existing or new function
 */
image_func * image_func::FindOrCreateFunc(Address target, 
                                          FuncSource src,
                                          bool & created)
{
    image_func *targetFunc;

    if(image_->funcsByEntryAddr.defines(target))
    {   
        targetFunc = image_->funcsByEntryAddr[target];
    }
    else
    {   
        if(img()->activelyParsing.defines(target))
            targetFunc = img()->activelyParsing[target];
        else {
            char name[32];
#if defined (os_windows)
            _snprintf(name,32,"targ%lx",target);
#else
            snprintf(name,32,"targ%lx",target);
#endif
            targetFunc = new image_func(name,target,UINT_MAX,mod_,image_,src);
            created = true;
        }
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
 *  - May update other functions' "shared" status
 *
 */
void image_func::parseSharedBlocks(image_basicBlock * firstBlock,
                BPatch_Set< Address > &leaders,
                dictionary_hash< Address, image_basicBlock * > &leadersToBlock,
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

    parsing_printf("[%s:%u] Parsing shared code at 0x%lx, startoffset: "
                   "0x%lx endoffset: 0x%lx\n",
        FILE__,__LINE__,firstBlock->firstInsnOffset_, getOffset(), endOffset_);

    // remember that we have shared blocks
    containsSharedBlocks_ = true;

    // assume that return status is UNKNOWN because we do not do a detailed
    // parse of the code. This assumption leads to the greatest code 
    // coverage
    if(retStatus_ == RS_UNSET)
        retStatus_ = RS_UNKNOWN;

    // There are several cases that can lead to a pre-parsed block
    // having the current function (this) on its funcs_ list:
    //
    // 1. The shared block is the result of splitting a block in another
    // function (current function added to funcs_ in block creation)
    //
    // 2. The shared block was created by a call operation to an unparsed
    //    function *and then subsequently parsed prior to parsing the
    //    function for which it is the entry block* (whew). In that case
    //    the entire "called" function is pre-parsed, but it is already
    //    on the funcs_ list of the *first* block (bound up in 
    //    bindCallTarget. 
    //
    // In both cases, 'this' will always be the first function on the funcs_
    // list, if it is there at all, so we always check whether funcs_[0] == 
    // this prior to adding this to the funcs_ list. 
    //

    while(WL.size() > 0)
    {
        curBlk = WL.back();
        WL.pop_back(); 
  
        if(containsBlock(curBlk))
            continue;

        // If the block isn't shared, it only has one function that doesn't
        // yet know that it contains shared code.
        if(!curBlk->isShared_)
        {
            image_func * first = *curBlk->funcs_.begin();
            first->containsSharedBlocks_ = true;
            first->needsRelocation_ = true;
        }

        // If this block is a stub, we'll revisit it when parsing is done
        if(curBlk->isStub_) {
            img()->reparse_shared.push_back(
                pair<image_basicBlock *, image_func *>(curBlk,this));
            parsing_printf("XXXX posponing stub block %d (0x%lx) for later\n",
                curBlk->id(),curBlk->firstInsnOffset_);
            continue;
        }

        /* Copy instrumentation points (if any) to this function that
           will incorporate the shared code.

           XXX
           Given parsing errors that are possible given misparsed
           control flow (e.g., badly parsed indirect control flow,
           non-returning functions, and opaque conditional branches),
           it is possible for misaligned blocks to overlap &
           include the same instpoint. This is /not safe/ and should
           be addressed in a new, overlap-friendly parser, but
           in the meanwhile it is imperative that instpoints are not
           duplicated in these [rare] cases.

           XXX
           For the moment we are not implementing a check before
           copy to continue to exercise parsing bugs.
        */

        // note that these have to occur before this function
        // is added to the block's ownership list
        if((tmpInstPt = curBlk->getCallInstPoint()) != NULL)
        {
            parsing_printf("... copying call point at 0x%lx\n", tmpInstPt->offset());                
            cpyInstPt = new image_instPoint(tmpInstPt->offset(),
                                    tmpInstPt->insn(),
                                    this,
                                    tmpInstPt->callTarget(),
                                    tmpInstPt->isDynamic());
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

            retStatus_ = RS_RETURN;
        }

        // As described in the comment above, the
        // only cases in which 'this' could be on the block's funcs_ list
        // are when it is the first entry. 
        if(*curBlk->funcs_.begin() != this) {
        curBlk->addFunc(this);
        }

        // update block
        addToBlocklist(curBlk);
        parsing_printf("XXXX adding pre-parsed block %d (0x%lx) to blocklist\n",
                curBlk->id(),curBlk->firstInsnOffset_);
        // update "function end"
        if(funcEnd < curBlk->blockEndOffset_)
            funcEnd = curBlk->blockEndOffset_;

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
#if defined (cap_use_pdvector)
    WL.zap();
    targets.zap();
#else
    WL.clear();
    targets.clear();
#endif
}

/* A specialized version for parsing shared blocks when we don't care
 * to save any information about the blocks that were visited (that is,
 * when the entire function is known to be shared and so no other normal
 * parsing is taking place).
 */
void image_func::parseSharedBlocks(image_basicBlock * firstBlock)
{
    BPatch_Set< Address > leaders;
    dictionary_hash< Address, image_basicBlock * > leadersToBlock( addrHash );

    endOffset_ = getOffset();
    
    parseSharedBlocks(firstBlock,
                      leaders,          /* unused */
                      leadersToBlock,   /* unused */
                      endOffset_);
}


void image_func::markBlockEnd(image_basicBlock* curBlock, InstructionAdapter& ah,
                             Address& funcEnd)
{
    curBlock->lastInsnOffset_ = ah.getAddr();
    curBlock->blockEndOffset_ = ah.getNextAddr();
    if(funcEnd < curBlock->blockEndOffset_)
    {
        funcEnd = curBlock->blockEndOffset_;
        endOffset_ = curBlock->blockEndOffset_;
    }
}


bool image_func::isNonReturningCall(image_func* targetFunc,
                                    bool isInPLT,
                                    std::string pltEntryForTarget,
                                   Address currAddr,
                                   Address target)
{
    if(targetFunc && (targetFunc->symTabName() == "exit" ||
       targetFunc->symTabName() == "abort" ||
       targetFunc->symTabName() == "__f90_stop" ||
       targetFunc->symTabName() == "fancy_abort" ||
       targetFunc->symTabName() == "__stack_chk_fail" ||
       targetFunc->symTabName() == "__assert_fail" ||
       targetFunc->symTabName() == "ExitProcess"))
    {
        parsing_printf("Call to %s (%lx) detected at 0x%lx\n",
                       targetFunc->symTabName().c_str(),
                       target, currAddr);
        return true;
    }
    else if(isInPLT &&
            (pltEntryForTarget == "exit" ||
            pltEntryForTarget == "abort" ||
            pltEntryForTarget == "__f90_stop" ||
            pltEntryForTarget == "fancy_abort" ||
            pltEntryForTarget == "__stack_chk_fail" ||
            pltEntryForTarget == "__assert_fail" ||
            pltEntryForTarget == "ExitProcess"))
    {
        parsing_printf("Call to %s (%lx) detected at 0x%lx\n",
                       pltEntryForTarget.c_str(),
                       target, currAddr);
        return true;
    }
    // HANDLED ELSEWHERE NOW!             
    // we don't wire up a fallthrough edge if we're treating
    // the call insruction as an unconditional branch
    
                 
    // link up the fallthrough edge unless we know for
    // certain that the target function does not return,
    // or if the target is an entry in the PLT (and not
    // one of the special-case non-returning entries above)
    if(targetFunc && targetFunc->returnStatus() == RS_NORETURN
       && !(isInPLT))
    {
        parsing_printf("[%s] not parsing past non-returning "
                "call at 0x%lx (to %s)\n",
        FILE__,currAddr,
        targetFunc->symTabName().c_str());
        return true;
    }
    else
    {
        return false;
    }

}
