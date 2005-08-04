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
 
// $Id: image-func.C,v 1.3 2005/08/04 22:54:24 bernat Exp $

#include "function.h"
#include "process.h"
#include "instPoint.h"
#include "InstrucIter.h"

pdstring image_func::emptyString("");


// Verify that this is code
// Find the call points
// Find the return address
// TODO -- use an instruction object to remove
// Sets err to false on error, true on success
//
// Note - this must define funcEntry and funcReturn
// 
image_func::image_func(const pdstring &symbol,
		       Address offset, 
		       const unsigned size,
		       pdmodule *m,
		       image *i) :
  line_(0),
  offset_(offset),
  size_(size),
  mod_(m),
  image_(i),
  parsed_(false),
  noStackFrame(false),
  makesNoCalls_(false),
  savesFP_(false),
  call_points_have_been_checked(false),
  isTrap(false),
  isInstrumentable_(false),
#if defined(arch_ia64)
  framePointerCalculator(NULL),
  usedFPregs(NULL),
#endif
  canBeRelocated_(false),
  needsRelocation_(false),
  originalCode(NULL),
  o7_live(false)
{
  symTabName_.push_back(symbol);
}


image_func::~image_func() { 
  /* TODO */ 
}


// coming to dyninstAPI/src/symtab.hC
// needed in metric.C
bool image_func::match(image_func *fb)
{
  if (this == fb)
    return true;
  else
    return ((symTabName_ == fb->symTabName_) &&
	    (prettyName_ == fb->prettyName_) &&
	    (line_       == fb->line_) &&
	    (offset_     == fb->offset_) &&
	    (size_       == fb->size_));
}

void image_func::changeModule(pdmodule *mod) {
  // Called from buildFunctionLists, so we aren't entered in any 
  // module-level data structures. If this changes, UPDATE THE
  // FUNCTION.
  mod_ = mod;
}

#ifdef DEBUG
/*
  Debuggering info for function_prototype....
 */
ostream & function_prototype::operator<<(ostream &s) const {
  
    unsigned i=0;
    s << "Mangled name(s): " << symTabName_[0];
    for(i = 1; i < symTabName_.size(); i++) {
        s << ", " << symTabName_[i];
    }

    s << "\nPretty name(s): " << prettyName_[0];
    for(i = 1; i < prettyName_.size(); i++) {
        s << ", " << prettyName_[i];
    }
      s << "\nline_ = " << line_ << " addr_ = "<< addr_ << " size_ = ";
      s << size_ << endl;
  
    return s;
}

ostream &operator<<(ostream &os, function_prototype &f) {
    return f.operator<<(os);
}

#endif


bool image_func::isInstrumentableByFunctionName()
{
#if defined(i386_unknown_solaris2_5)
    /* On Solaris, this function is called when a signal handler
       returns.  If it requires trap-based instrumentation, it can foul
       the handler return mechanism.  So, better exclude it.  */
    if (prettyName() == "_setcontext" || prettyName() == "setcontext")
        return false;
#endif /* i386_unknown_solaris2_5 */
    
    if( prettyName() == "__libc_free" )
        return false;
    
    // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
    // they can't be instrumented or we would have an infinite loop
    if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
        || prettyName() == "GetProcessTimes")
        return false;
    return true;
}


// passing in a value of 0 for p will return the original size
// otherwise, if the process is relocated it will return the new size
unsigned image_func::getSize() {
  if (!parsed_) image_->analyzeIfNeeded();
  return size_;
}


const pdvector<image_instPoint *> &image_func::funcEntries() {
  if (!parsed_) image_->analyzeIfNeeded();
  return funcEntries_;
}

const pdvector<image_instPoint*> &image_func::funcExits() {
  if (!parsed_) image_->analyzeIfNeeded();

  return funcReturns;
}

const pdvector<image_instPoint*> &image_func::funcCalls() {
  if (!parsed_) image_->analyzeIfNeeded();

  return calls;
}


void image_basicBlock::addSource(image_basicBlock *source) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i] == source)
            return;
    sources_.push_back(source);
}

void image_basicBlock::addTarget(image_basicBlock *target) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i] == target)
            return;
    targets_.push_back(target);
}

void image_basicBlock::removeSource(image_basicBlock *source) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i] == source) {
            sources_[i] = sources_.back();
            sources_.resize(sources_.size()-1);
            return;
        }
}

void image_basicBlock::removeTarget(image_basicBlock *target) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i] == target) {
            targets_[i] = targets_.back();
            targets_.resize(targets_.size()-1);
            return;
        }
}

void image_basicBlock::getSources(pdvector<image_basicBlock *> &ins) const {
    for (unsigned i = 0; i < sources_.size(); i++)
        ins.push_back(sources_[i]);
}
// Need to be able to get a copy
void image_basicBlock::getTargets(pdvector<image_basicBlock *> &outs) const {
    for (unsigned i = 0; i < targets_.size(); i++)
        outs.push_back(targets_[i]);
}

image_instPoint::image_instPoint(Address offset,
                                 instruction insn,
                                 image_func *func,
                                 instPointType_t type) :
    instPointBase(insn, type),
    offset_(offset),
    func_(func),
    callee_(NULL),
    callTarget_(0),
    isDynamicCall_(0)
{}

image_instPoint::image_instPoint(Address offset,
                                 instruction insn,
                                 image_func *func,
                                 Address callTarget,
                                 bool isDynamicCall) :
    instPointBase(insn, callSite),
    offset_(offset),
    func_(func),
    callee_(NULL),
    callTarget_(callTarget),
    isDynamicCall_(isDynamicCall)
{
    if (isDynamicCall_)
        assert(callTarget_ == 0);
    else
        assert(!isDynamicCall_);
}


void image_basicBlock::debugPrint() {
    // 64-bit
    parsing_printf("Block %d: starts 0x%lx (%d), last 0x%lx (%d), end 0x%lx (%d)\n",
                   blockNumber_,
                   firstInsnOffset_,
                   firstInsnOffset_ - func_->getOffset(),
                   lastInsnOffset_,
                   lastInsnOffset_ - func_->getOffset(),
                   blockEndOffset_,
                   blockEndOffset_ - func_->getOffset());
    parsing_printf("  Flags: entry %d, exit %d\n",
                   isEntryBlock_, isExitBlock_);
    parsing_printf("  Sources:\n");
    for (unsigned s = 0; s < sources_.size(); s++) {
        parsing_printf("    %d: block %d\n",
                       s, sources_[s]->blockNumber_);
    }
    parsing_printf("  Targets:\n");
    for (unsigned t = 0; t < targets_.size(); t++) {
        parsing_printf("    %d: block %d\n",
                       t, targets_[t]->blockNumber_);
    }
}

// Make sure no blocks overlap, sort stuff by address... you know,
// basic stuff.

bool image_func::cleanBlockList() {
    
    // For all entry, exit, call points...
    //points_[u].point->checkInstructions();
    // Should also make multipoint decisions

#if !defined(arch_x86) && !defined(arch_power)
    // We need to make sure all the blocks are inside the function
    pdvector<image_basicBlock *>cleanedList;
    for (unsigned foo = 0; foo < blockList.size(); foo++) {
        if ((blockList[foo]->firstInsnOffset() < getOffset()) ||
            (blockList[foo]->firstInsnOffset() >= getOffset() + getSize())) {
            delete blockList[foo];
        }
        else
            cleanedList.push_back(blockList[foo]);
    }
    blockList.clear();
    for (unsigned bar = 0; bar < cleanedList.size(); bar++)
        blockList.push_back(cleanedList[bar]);


#endif

   //sorted_ips_vector expects funcReturns and calls to be sorted
    VECTOR_SORT( funcReturns, image_instPoint::compare);
    VECTOR_SORT( calls, image_instPoint::compare);

    //check if basic blocks need to be split   
    VECTOR_SORT( blockList, image_basicBlock::compare );
    //parsing_printf("INITIAL BLOCK LIST\n");
    //maybe image_flowGraph.C would be a better home for this bit of code?
    for( unsigned int iii = 0; iii < blockList.size(); iii++ )
    {
        blockList[iii]->blockNumber_ = iii;
        //blockList[iii]->debugPrint();
    }
  
    for( unsigned int r = 0; r + 1 < blockList.size(); r++ )
    {
        image_basicBlock* b1 = blockList[ r ];
        image_basicBlock* b2 = blockList[ r + 1 ];
        
        if( b2->firstInsnOffset() < b1->endOffset() )
        {
            //parsing_printf("Blocks %d and %d overlap...\n",
            //b1->blockNumber_,
            //b2->blockNumber_);
            pdvector< image_basicBlock* > out;
            b1->getTargets( out );
            
            for( unsigned j = 0; j < out.size(); j++ )
            {
                out[j]->removeSource( b1 );
                out[j]->addSource( b2 );
            }        
          
            //set end address of higher block
            b2->lastInsnOffset_ =  b1->lastInsnOffset();
            b2->blockEndOffset_ =  b1->endOffset();
            b2->targets_ = b1->targets_;	    
            b2->addSource( b1 );
            
            b1->targets_.clear();
            b1->targets_.push_back(b2);
            
            //find the end of the split block	       
            InstrucIter ah( b1 );
            while( *ah + ah.getInstruction().size() < b2->firstInsnOffset() )
                ah++;
            
            b1->lastInsnOffset_ = *ah;
            b1->blockEndOffset_ = *ah + ah.getInstruction().size();

            if( b1->isExitBlock_ )
            {
                b1->isExitBlock_ = false;
                b2->isExitBlock_ = true;
            }
        }
    }
    for( unsigned q = 0; q + 1 < blockList.size(); q++ )
    {
        image_basicBlock* b1 = blockList[ q ];
        image_basicBlock* b2 = blockList[ q + 1 ];
        
        if( b1->endOffset() == 0 )
        {
            ///parsing_printf("Block %d has zero size; expanding to block %d\n",
            //b1->blockNumber_,
            //b2->blockNumber_);

            //find the end of this block.

            // Make the iterator happy; we can set the end offset to
            // the start of b2. It will be that or smaller.
            b1->blockEndOffset_ = b2->firstInsnOffset();

            InstrucIter ah( b1 );
            while( *ah + ah.getInstruction().size() < b2->firstInsnOffset() )
                ah++;
            
            b1->lastInsnOffset_ = *ah;
            b1->blockEndOffset_ = *ah + ah.getInstruction().size();
            b1->addTarget( b2 );	  
            b2->addSource( b1 );	            
        }        
    }    
    parsing_printf("CLEANED BLOCK LIST\n");
    for (unsigned foo = 0; foo < blockList.size(); foo++) {
        blockList[foo]->debugPrint();
    }
    return true;
}

// Bind static call points

void image_func::checkCallPoints() {
    // Check if there are any dangling calls
    for (unsigned c = 0; c < calls.size(); c++) {
        image_instPoint *p = calls[c];
        assert(p);
        
        if (p->getCallee() != NULL)
            continue;
        
        Address destOffset = p->callTarget();
        if (!destOffset) {
            // Couldn't determine contact; skip
            continue;
        }
        
        image_func *pdf = image_->findFuncByOffset(destOffset);
        
        if (pdf) {
            p->setCallee(pdf);
        }
    }
}

