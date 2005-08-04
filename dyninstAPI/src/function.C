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
 
// $Id: function.C,v 1.10 2005/03/02 19:44:45 bernat Exp 

#include "function.h"
#include "BPatch_flowGraph.h"
#include "process.h"
#include "instPoint.h"
#include "BPatch_basicBlockLoop.h"
#include "BPatch_basicBlock.h"

pdstring int_function::emptyString("");


// 
int_function::int_function(image_func *f,
			   Address baseAddr,
			   mapped_module *mod) :
#if defined(arch_ia64)
	/* image_funcs are reference-counted, so we don't have to clone these. */
    framePointerCalculator( assignAst( f->framePointerCalculator ) ),
    usedFPregs( f->usedFPregs ),
#endif
    size_(f->get_size_cr()),
    ifunc_(f),
    mod_(mod),
    flowGraph(NULL),
    previousFunc_(NULL),
#if defined(cap_relocation)
    needs_relocation_(f->needsRelocation()),
    canBeRelocated_(f->canBeRelocated()),
#endif
    code_(NULL),
    version_(1)
{
    //parsing_printf("Function offset: 0x%x; base: 0x%x\n",
    //f->getOffset(), baseAddr);
    addr_ = f->getOffset() + baseAddr;
#if defined(arch_ia64)
    parsing_printf("%s: creating new proc-specific function at 0x%llx\n",
                   symTabName().c_str(), addr_);
#else
    parsing_printf("%s: creating new proc-specific function at 0x%x\n",
                   symTabName().c_str(), addr_);
#endif
    /*
    fprintf(stderr, "%s: creating new proc-specific function at 0x%llx\n",
	    symTabName().c_str(), addr_);
    */
    // We delay the creation of instPoints until they are requested;
    // this saves memory, and really, until something is asked for we
    // don't need it.  TODO: creation of an arbitrary instPoint should
    // trigger instPoint creation; someone may create an arbitrary at
    // a entry/exit/callsite.

    // Same with the flowGraph; we clone it from the image_func when
    // we need it.

#if defined(arch_ia64)
	for( unsigned i = 0; i < f->allocs.size(); i++ ) {
		allocs.push_back( baseAddr + f->allocs[i] );
		}        
#endif

}


int_function::int_function(const int_function *parFunc,
                           mapped_module *childMod) :
#if defined(arch_ia64)
	/* image_funcs are reference-counted, so we don't have to clone these. */
    framePointerCalculator( assignAst( parFunc->ifunc_->framePointerCalculator ) ),
    usedFPregs( parFunc->ifunc_->usedFPregs ),
#endif
    addr_(parFunc->addr_),
    size_(parFunc->size_),
    ifunc_(parFunc->ifunc_),
    mod_(childMod),
    flowGraph(NULL),
    previousFunc_(NULL),
#if defined(cap_relocation)
    needs_relocation_(parFunc->needs_relocation_),
    canBeRelocated_(parFunc->canBeRelocated_),
#endif
    version_(parFunc->version_)
 {
     // Construct the blocklist;
     if (parFunc->blockList.size() > 0) {
         blocks();
     }

     for (unsigned i = 0; i < parFunc->entryPoints_.size(); i++) {
         instPoint *parP = parFunc->entryPoints_[i];
         instPoint *childP = instPoint::createForkedPoint(parP, this);
         entryPoints_.push_back(childP);
     }

     for (unsigned ii = 0; ii < parFunc->exitPoints_.size(); ii++) {
         instPoint *parP = parFunc->exitPoints_[ii];
         instPoint *childP = instPoint::createForkedPoint(parP, this);
         exitPoints_.push_back(childP);
     }

     for (unsigned iii = 0; iii < parFunc->callPoints_.size(); iii++) {
         instPoint *parP = parFunc->callPoints_[iii];
         instPoint *childP = instPoint::createForkedPoint(parP, this);
         callPoints_.push_back(childP);
     }

     for (unsigned iiii = 0; iiii < parFunc->arbitraryPoints_.size(); iiii++) {
         instPoint *parP = parFunc->arbitraryPoints_[iiii];
         instPoint *childP = instPoint::createForkedPoint(parP, this);
         arbitraryPoints_.push_back(childP);
     }

#if defined(arch_ia64)
	for( unsigned i = 0; i < parFunc->allocs.size(); i++ ) {
		allocs.push_back( parFunc->allocs[i] );
		}
     
	if( usedFPregs == NULL && parFunc->usedFPregs != NULL ) {
		/* Clone the usedFPregs, since int_functions can go away. */
		usedFPregs = (bool *)malloc( 128 * sizeof( bool ) );
		assert( usedFPregs != NULL );
		memcpy( usedFPregs, parFunc->usedFPregs, 128 * sizeof( bool ) );
		}
     
    /* ASTs are reference-counted, so assignAst() will prevent fPC from
       vanishing even if f does. */
	if( framePointerCalculator == NULL && parFunc->framePointerCalculator != NULL ) {
		framePointerCalculator = assignAst( parFunc->framePointerCalculator );
		}
#endif

     // TODO: relocated functions
}

int_function::~int_function() { 
  /* TODO */ 
}

unsigned int_function::getSize() {
  if (!size_) {
      // Can parse
      size_ = ifunc_->getSize();
  }
  return size_;
}

// coming to dyninstAPI/src/symtab.hC
// needed in metric.C
bool int_function::match(int_function *fb) const
{
  if (this == fb)
    return true;
  else
    return ifunc_->match(fb->ifunc_);
}

void int_function::addArbitraryPoint(instPoint *insp) {
    assert(previousFunc_ == NULL); // Don't handle per-func-version IPs
    arbitraryPoints_.push_back(insp);
}

const pdvector<instPoint *> &int_function::funcEntries() {
    if (previousFunc_ == NULL) {
        if (entryPoints_.size() == 0) {
            const pdvector<image_instPoint *> &img_entries = ifunc_->funcEntries();
            
            for (unsigned i = 0; i < img_entries.size(); i++) {
                instPoint *point = instPoint::createParsePoint(this,
                                                               img_entries[i]);
                assert(point);
                entryPoints_.push_back(point);
            }
        }
        return entryPoints_;
    }
    else {
        // Don't really feel like maintaining a list in each copy...
        return getOriginalFunc()->funcEntries();
    }
}

const pdvector<instPoint*> &int_function::funcExits() {
    if (previousFunc_ == NULL) {
        if (exitPoints_.size() == 0) {
            const pdvector<image_instPoint *> &img_exits = ifunc_->funcExits();
            
            for (unsigned i = 0; i < img_exits.size(); i++) {
                instPoint *point = instPoint::createParsePoint(this,
                                                               img_exits[i]);
                assert(point);
                exitPoints_.push_back(point);
            }
        }
        return exitPoints_;
    }
    else
        return getOriginalFunc()->funcExits();
}

const pdvector<instPoint*> &int_function::funcCalls() {
    if (previousFunc_ == NULL) {
        if (callPoints_.size() == 0) {
            const pdvector<image_instPoint *> &img_calls = ifunc_->funcCalls();
            
            for (unsigned i = 0; i < img_calls.size(); i++) {
                instPoint *point = instPoint::createParsePoint(this,
                                                               img_calls[i]);
                assert(point);
                callPoints_.push_back(point);
            }
        }
        return callPoints_;
    }
    else
        return getOriginalFunc()->funcCalls();
}

const pdvector<instPoint*> &int_function::funcArbitraryPoints() {
  // We add these per-process, so there's no chance to have
  // a parse-level list
    if (previousFunc_ == NULL) {
        return arbitraryPoints_;
    }
    else 
        return getOriginalFunc()->funcArbitraryPoints();
}

Address int_function::getOriginalAddress() const {
    /// Mmmm recursion
    if (previousFunc_ == NULL)
        return getAddress();
    return previousFunc_->getAddress();
}

int_function *int_function::getOriginalFunc() const {
    /// Mmm recursive goodness
    if (previousFunc_ == NULL)
        return const_cast<int_function *>(this);
    else
        return previousFunc_->getOriginalFunc();
}

void print_func_vector_by_pretty_name(pdstring prefix,
				      pdvector<int_function *>*funcs) {
    unsigned int i;
    int_function *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

mapped_module *int_function::mod() const { return mod_; }
mapped_object *int_function::obj() const { return mod()->obj(); }
process *int_function::proc() const { return obj()->proc(); }

funcIterator::funcIterator(int_function *startfunc) : index(0) {
  unusedFuncs_.push_back(startfunc);
  for (unsigned i = 0; i < startfunc->relocatedFuncs_.size(); i++) {
    unusedFuncs_.push_back(startfunc->relocatedFuncs_[i]);
  }
}

int_function *funcIterator::operator*() {
    if (index < unusedFuncs_.size())
        return unusedFuncs_[index];
    else
        return NULL;
}

int_function *funcIterator::operator++(int) {
  index++;
  if (index < unusedFuncs_.size()) {
      int_function *func = unusedFuncs_[index];
      for (unsigned i = 0; i < func->relocatedFuncs_.size(); i++) {
          unusedFuncs_.push_back(func->relocatedFuncs_[i]);
      }
      return func;
  }
  return NULL;
}

int_basicBlock *int_function::findBlockByAddr(Address addr) {
    if (blockList.size() == 0) {
        // Will make the block list...
        blocks();
    }

    for (unsigned i = 0; i < blockList.size(); i++) {
        if ((addr >= blockList[i]->firstInsnAddr()) &&
            (addr < blockList[i]->endAddr()))
            return blockList[i];
    }
    return NULL;
}

const pdvector<int_basicBlock *> &int_function::blocks() {
    if (blockList.size() == 0) {
        Address base = getAddress() - ifunc_->getOffset();
        // TODO: create flowgraph pointer...
        const pdvector<image_basicBlock *> &img_blocks = ifunc_->blocks();

        for (unsigned i = 0; i < img_blocks.size(); i++) {
            blockList.push_back(new int_basicBlock(img_blocks[i],
                                               base,
                                               this));
        }
        // Now that we have the correct list, patch up sources/targets
        for (unsigned j = 0; j < img_blocks.size(); j++) {
            pdvector<image_basicBlock *> targets;
            img_blocks[j]->getTargets(targets);
            for (unsigned t = 0; t < targets.size(); t++) {
                blockList[j]->addTarget(blockList[targets[t]->id()]);
            }

            pdvector<image_basicBlock *> sources;
            img_blocks[j]->getSources(sources);
            for (unsigned s = 0; s < sources.size(); s++) {
                blockList[j]->addSource(blockList[sources[s]->id()]);
            }
        }
    }
    // And a quick consistency check...
    
    return blockList;
}

void *int_function::getPtrToOrigInstruction(Address addr) const {
    // Asking for a local pointer before we've even _parsed_?
    assert(size_);

    assert(addr >= getAddress());
    assert(addr < (getAddress() + size_));
    
    if (code_ == NULL) return NULL;

    // Yay bitmath. Don't want to use a char... this needs to be 1
    // byte. So we use an Address type instead.
    Address memoryGames = (Address) code_;
    memoryGames += (addr - getAddress());
    return (void *)memoryGames;
}

Address int_function::equivAddr(int_function *other_func, Address addrInOther) const {
    if (getOriginalFunc() != other_func->getOriginalFunc())
        assert(0);

    if (other_func == this) return addrInOther;

    Address origOffset = other_func->offsetInOriginal(addrInOther - other_func->getAddress());
    return getAddress() + offsetInSelf(origOffset);
}

Address int_function::offsetInOriginal(Address offset) const {
    if (this == getOriginalFunc())
        return offset;
    codeRange *ptr;
    if (!relocOffsetsBkwd_.find(offset, ptr)) {
        assert(0);
    }
    relocShift *rs = dynamic_cast<relocShift *>(ptr);
    assert(rs);
    // rs has cumulative shift for our offset.
    return offset - rs->get_shift();
}

Address int_function::offsetInSelf(Address offset) const {
    if (this == getOriginalFunc())
        return offset;
    codeRange *ptr;
    if (!relocOffsetsFwd_.find(offset, ptr)) 
        assert(0);
    relocShift *rs = dynamic_cast<relocShift *>(ptr);
    assert(rs);
    return offset + rs->get_shift();
}

process *int_basicBlock::proc() const {
    return func()->proc();
}

void int_basicBlock::addSource(int_basicBlock *source) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i] == source)
            return;
    sources_.push_back(source);
}

void int_basicBlock::addTarget(int_basicBlock *target) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i] == target)
            return;
    targets_.push_back(target);
}

void int_basicBlock::getSources(pdvector<int_basicBlock *> &ins) const {
    ins = sources_;
}

void int_basicBlock::getTargets(pdvector<int_basicBlock *> &outs) const {
    outs = targets_;
}

int_basicBlock *int_basicBlock::getFallthrough() const {
    // We could keep it special...
    for (unsigned i = 0; i < targets_.size(); i++) {
        if (targets_[i]->firstInsnAddr() == endAddr())
            return targets_[i];
    }
    return NULL;
}

unsigned int_function::getNumDynamicCalls()
{
   unsigned count=0;
   pdvector<instPoint *> callPoints = funcCalls();

   for (unsigned i=0; i<callPoints.size(); i++)
   {
      if (callPoints[i]->isDynamicCall())
          count++;
   }
   return count;
}

const pdstring &int_function::symTabName() const { 
    return ifunc_->symTabName(); 
}

#ifndef BPATCH_LIBRARY
static unsigned int_function_ptr_hash(int_function *const &f) {
  int_function *ptr = f;
  unsigned l = (unsigned)(Address)ptr;
  return addrHash4(l); 
}

// Fill in <callees> with list of statically determined callees of
//  function.  
// Uses process specific info to try to fill in the unbound call
//  destinations through PLT entries.  Note that when it determines
//  the destination of a call through the PLT, it puts that
//  call destination into <callees>, but DOES NOT fill in the
//  call destination in the function's instPoint.  This is because
//  the (through PLT) binding is process specific.  It is possible, 
//  for example, that one could have 2 processes, both sharing the
//  same a.out image, but which are linked with different versions of
//  the same shared library (or with the same shared libraries in 
//  a different order), in which case the int_function data would be 
//  shared between the processes, but the (through-PLT) call 
//  destinations might NOT be the same.
// Should filter out any duplicates in this callees list....

bool int_function::getStaticCallees(pdvector <int_function *>&callees) {
    unsigned u;
    int_function *f;
    bool found;
    
    dictionary_hash<int_function *, int_function *> 
      filter(int_function_ptr_hash);
    
    callees.resize(0);
    
#ifndef CHECK_ALL_CALL_POINTS
    // JAW -- need to checkCallPoints() here to ensure that the
    // vector "calls" has been fully initialized/filtered/classified.
    //
    //
    checkCallPoints();
#endif

    // possible algorithm : iterate over calls (vector of instPoint *)
    //   for each elem : use getCallee() to get statically determined
    //   callee....
    for(u=0;u<callPoints_.size();u++) {
        //this call to getCallee is platform specific
        f = callPoints_[u]->findCallee();
        
        if (f != NULL && !filter.defines(f)) {
            callees += (int_function *)f;
            filter[f] = f;
        }
    }
    return true;
}
#endif

image_func *int_function::ifunc() const {
    return ifunc_;
}

BPatch_Set<int_basicBlock *> *int_basicBlock::getDataFlowOut() {
    return dataFlowOut;
}

BPatch_Set<int_basicBlock *> *int_basicBlock::getDataFlowIn() {
    return dataFlowIn;
}

int_basicBlock *int_basicBlock::getDataFlowGen() {
    return dataFlowGen;
}

int_basicBlock *int_basicBlock::getDataFlowKill() {
    return dataFlowKill;
}

void int_basicBlock::setDataFlowIn(BPatch_Set<int_basicBlock *> *in) {
    dataFlowIn = in;
}

void int_basicBlock::setDataFlowOut(BPatch_Set<int_basicBlock *> *out) {
    dataFlowOut = out;
}

void int_basicBlock::setDataFlowGen(int_basicBlock *gen) {
    dataFlowGen = gen;
}

void int_basicBlock::setDataFlowKill(int_basicBlock *kill) {
    dataFlowKill = kill;
}


int_basicBlock::int_basicBlock(const image_basicBlock *ib, Address baseAddr, int_function *func) :
    firstInsnAddr_(ib->firstInsnOffset() + baseAddr),
    lastInsnAddr_(ib->lastInsnOffset() + baseAddr),
    blockEndAddr_(ib->endOffset() + baseAddr),
    isEntryBlock_(ib->isEntryBlock()),
    isExitBlock_(ib->isExitBlock()),
    blockNumber_(ib->id()),
    dataFlowIn(NULL),
    dataFlowOut(NULL),
    dataFlowGen(NULL),
    dataFlowKill(NULL),
    func_(func),
    ib_(ib)
{}
