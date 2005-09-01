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
#include "InstrucIter.h"
#include "multiTramp.h"

#include "mapped_object.h"
#include "mapped_module.h"

#if defined(cap_relocation)
#include "reloc-func.h"
#endif

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
    ifunc_(f),
    mod_(mod),
    flowGraph(NULL),
#if defined(cap_relocation)
    canBeRelocated_(f->canBeRelocated()),
    generatedVersion_(0),
    installedVersion_(0),
    linkedVersion_(0),
#endif
    version_(0)
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
    ifunc_(parFunc->ifunc_),
    mod_(childMod),
    flowGraph(NULL),
#if defined(cap_relocation)
    canBeRelocated_(parFunc->canBeRelocated_),
    generatedVersion_(parFunc->generatedVersion_),
    installedVersion_(parFunc->installedVersion_),
    linkedVersion_(parFunc->linkedVersion_),
#endif
    version_(parFunc->version_)
 {
     // Construct the raw blocklist;
     for (unsigned i = 0; i < parFunc->blockList.size(); i++) {
         int_basicBlock *block = new int_basicBlock(parFunc->blockList[i], this);
         blockList.push_back(block);
     }
     
     // And hook up sources/targets
     for (unsigned i = 0; i < blockList.size(); i++) {
         int_basicBlock *child = blockList[i];
         int_basicBlock *parent = parFunc->blockList[i];
         for (unsigned t = 0; t < parent->targets_.size(); t++) {
             child->targets_.push_back(blockList[parent->targets_[t]->id()]);
         }
         for (unsigned s = 0; s < parent->sources_.size(); s++) {
             child->sources_.push_back(blockList[parent->sources_[s]->id()]);
         }
     }         


     for (unsigned i = 0; i < parFunc->entryPoints_.size(); i++) {
         instPoint *parP = parFunc->entryPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
         entryPoints_.push_back(childP);
     }

     for (unsigned ii = 0; ii < parFunc->exitPoints_.size(); ii++) {
         instPoint *parP = parFunc->exitPoints_[ii];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
         exitPoints_.push_back(childP);
     }

     for (unsigned iii = 0; iii < parFunc->callPoints_.size(); iii++) {
         instPoint *parP = parFunc->callPoints_[iii];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
         callPoints_.push_back(childP);
     }

     for (unsigned iiii = 0; iiii < parFunc->arbitraryPoints_.size(); iiii++) {
         instPoint *parP = parFunc->arbitraryPoints_[iiii];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
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
    // ifunc_ doesn't keep tabs on us, so don't need to let it know.
    // mod_ is cleared by the mapped_object
    // blockList isn't allocated
    // flowGraph is taken care of by BPatch...
    // instPoints are process level (should be deleted here and refcounted)

#if defined(cap_relocation)
    for (unsigned i = 0; i < enlargeMods_.size(); i++)
        delete enlargeMods_[i];
    enlargeMods_.clear();
#endif
}

// This needs to go away: how is "size" defined? Used bytes? End-start?

unsigned int_function::getSize_NP()  {
    blocks();
    if (blockList.size() == 0) return 0;
    return (blockList.back()->origInstance()->endAddr() - 
            blockList.front()->origInstance()->firstInsnAddr());
}

void int_function::addArbitraryPoint(instPoint *insp) {
    arbitraryPoints_.push_back(insp);
}

const pdvector<instPoint *> &int_function::funcEntries() {
    if (entryPoints_.size() == 0) {
        const pdvector<image_instPoint *> &img_entries = ifunc_->funcEntries();        
        for (unsigned i = 0; i < img_entries.size(); i++) {

            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_entries[i]->offset() - img_entries[i]->func()->getOffset();
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for entry point at 0x%lx (0x%lx) (func 0x%lx to 0x%lx\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP());
                
                continue;
            }

            instPoint *point = instPoint::createParsePoint(this,
                                                           img_entries[i]);
            assert(point);
            entryPoints_.push_back(point);
        }
    }
    return entryPoints_;
}

const pdvector<instPoint*> &int_function::funcExits() {
    if (exitPoints_.size() == 0) {
        const pdvector<image_instPoint *> &img_exits = ifunc_->funcExits();
        
        for (unsigned i = 0; i < img_exits.size(); i++) {
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_exits[i]->offset() - img_exits[i]->func()->getOffset();
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for exit point at 0x%lx (0x%lx) (func 0x%lx to 0x%lx\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP());
                
                continue;
            }

            instPoint *point = instPoint::createParsePoint(this,
                                                           img_exits[i]);
            assert(point);
            exitPoints_.push_back(point);
        }
    }
    return exitPoints_;
}

const pdvector<instPoint*> &int_function::funcCalls() {
    if (callPoints_.size() == 0) {
        const pdvector<image_instPoint *> &img_calls = ifunc_->funcCalls();
        
        for (unsigned i = 0; i < img_calls.size(); i++) {
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_calls[i]->offset() - img_calls[i]->func()->getOffset();
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for call point at 0x%lx (0x%lx) (func 0x%lx to 0x%lx\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP());
                
                continue;
            }
            instPoint *point = instPoint::createParsePoint(this,
                                                           img_calls[i]);
            assert(point);
            callPoints_.push_back(point);
        }
    }
    return callPoints_;
}

const pdvector<instPoint*> &int_function::funcArbitraryPoints() {
  // We add these per-process, so there's no chance to have
  // a parse-level list
    return arbitraryPoints_;
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

bblInstance *int_function::findBlockInstanceByAddr(Address addr) {
    codeRange *range;
    if (blockList.size() == 0) {
        // Will make the block list...
        blocks();
    }
    
    if (blocksByAddr_.find(addr, range)) {
        assert(range->is_basicBlockInstance());
        return range->is_basicBlockInstance();
    }
    return NULL;
}

int_basicBlock *int_function::findBlockByAddr(Address addr) {
    bblInstance *inst = findBlockInstanceByAddr(addr);
    if (inst)
        return inst->block();
    else
        return NULL;
}


const pdvector<int_basicBlock *> &int_function::blocks() {
    inst_printf("blocks() for %s\n", symTabName().c_str());
    if (blockList.size() == 0) {
        Address base = getAddress() - ifunc_->getOffset();
        // TODO: create flowgraph pointer...
        const pdvector<image_basicBlock *> &img_blocks = ifunc_->blocks();
        
        for (unsigned i = 0; i < img_blocks.size(); i++) {
            blockList.push_back(new int_basicBlock(img_blocks[i],
                                                   base,
                                                   this));
            // And add to the mapped_object code range
            obj()->codeRangesByAddr_.insert(blockList.back()->origInstance());
        }
        // Now that we have the correct list, patch up sources/targets
        for (unsigned j = 0; j < img_blocks.size(); j++) {
            pdvector<image_basicBlock *> targets;
            img_blocks[j]->getTargets(targets);
            for (unsigned t = 0; t < targets.size(); t++) {
                if ((targets[t]->id() >= blockList.size()) ||
                    (targets[t]->id() < 0)) {
                    fprintf(stderr, "WARNING: omitting illegal target %d block %d for block %d, function %s\n",
                            t, targets[t]->id(), blockList[j]->id(), symTabName().c_str());
                    continue;
                }
                inst_printf("Adding target to block %d, block %d (of %d)\n",
                            j, targets[t]->id(),
                            blockList.size());
                blockList[j]->addTarget(blockList[targets[t]->id()]);
            }

            pdvector<image_basicBlock *> sources;
            img_blocks[j]->getSources(sources);
            for (unsigned s = 0; s < sources.size(); s++) {
                if ((sources[s]->id() >= blockList.size()) ||
                    (sources[s]->id() < 0)) {
                    fprintf(stderr, "WARNING: omitting illegal target %d block %d for block %d, function %s\n",
                            s, sources[s]->id(), blockList[j]->id(), symTabName().c_str());
                    continue;
                }
                inst_printf("Adding source to block %d, block %d (of %d)\n",
                            j, sources[s]->id(),
                            blockList.size());
                blockList[j]->addSource(blockList[sources[s]->id()]);
            }
        }
    }
    if (blockList.size() == 0) {
        fprintf(stderr, "WARNING: no blocks in function %s!\n", symTabName().c_str());
    }
    // And a quick consistency check...
    return blockList;
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
        if (targets_[i]->origInstance()->firstInsnAddr() == origInstance()->endAddr())
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

void int_function::addBBLInstance(bblInstance *instance) {
    assert(instance);
    blocksByAddr_.insert(instance);
}

void int_function::deleteBBLInstance(bblInstance *instance) {
    assert(instance);
    blocksByAddr_.remove(instance->firstInsnAddr());
}

const image_func *int_function::ifunc() const {
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
    isEntryBlock_(ib->isEntryBlock()),
    isExitBlock_(ib->isExitBlock()),
    blockNumber_(ib->id()),
    dataFlowIn(NULL),
    dataFlowOut(NULL),
    dataFlowGen(NULL),
    dataFlowKill(NULL),
    func_(func),
    ib_(ib)
{
    bblInstance *inst = new bblInstance(ib->firstInsnOffset() + baseAddr,
                                        ib->lastInsnOffset() + baseAddr,
                                        ib->endOffset() + baseAddr,
                                        this, 
                                        0);
    instances_.push_back(inst);
    assert(func_);
    func_->addBBLInstance(inst);
}

int_basicBlock::int_basicBlock(const int_basicBlock *parent, int_function *func) :
    isEntryBlock_(parent->isEntryBlock_),
    isExitBlock_(parent->isExitBlock_),
    blockNumber_(parent->blockNumber_),
    dataFlowGen(NULL),
    dataFlowKill(NULL),
    func_(func),
    ib_(parent->ib_) {
    for (unsigned i = 0; i < parent->instances_.size(); i++) {
        bblInstance *bbl = new bblInstance(parent->instances_[i], this);
        instances_.push_back(bbl);
        func_->addBBLInstance(bbl);
    }
}

int_basicBlock::~int_basicBlock() {
    if (dataFlowIn) delete dataFlowIn;
    if (dataFlowOut) delete dataFlowOut;
    // Do not delete dataFlowGen and dataFlowKill; they're legal pointers
    // don't kill func_;
    // don't kill ib_;
    for (unsigned i = 0; i < instances_.size(); i++) {
        delete instances_[i];
    }
    instances_.clear();
}

bblInstance *int_basicBlock::origInstance() const {
    assert(instances_.size());
    return instances_[0];
}

bblInstance *int_basicBlock::instVer(unsigned id) const {
    if (id >= instances_.size())
        fprintf(stderr, "ERROR: requesting bblInstance %d, only %d known\n",
                id, instances_.size());
    return instances_[id];
}

void int_basicBlock::removeVersion(unsigned id) {
    if (id >= instances_.size()) {
        fprintf(stderr, "ERROR: deleting bblInstance %d, only %d known\n",
                id, instances_.size());
        return;
    }
    if (id < (instances_.size() - 1)) {
        fprintf(stderr, "ERROR: deleting bblInstance %d, not last\n",
                id, instances_.size());
        assert(0);
        return;
    }
    bblInstance *inst = instances_[id];
    delete inst;
    instances_.pop_back();
}


const pdvector<bblInstance *> &int_basicBlock::instances() const {
    return instances_;
}

bblInstance::bblInstance(Address start, Address last, Address end, int_basicBlock *parent, int version) : 
#if defined(cap_relocation)
    changedAddrs_(addrHash4),
    maxSize_(0),
    origInstance_(NULL),
    jumpToBlock(NULL),
#endif
    firstInsnAddr_(start),
    lastInsnAddr_(last),
    blockEndAddr_(end),
    block_(parent),
    version_(version)
{};

bblInstance::bblInstance(int_basicBlock *parent, int version) : 
#if defined(cap_relocation)
    changedAddrs_(addrHash4),
    maxSize_(0),
    origInstance_(NULL),
    jumpToBlock(NULL),
#endif
    firstInsnAddr_(0),
    lastInsnAddr_(0),
    blockEndAddr_(0),
    block_(parent),
    version_(version)
{};

bblInstance::bblInstance(const bblInstance *parent, int_basicBlock *block) :
#if defined(cap_relocation)
    changedAddrs_(parent->changedAddrs_),
    insns_(parent->insns_),
    maxSize_(parent->maxSize_),
    appliedMods_(parent->appliedMods_),
    generatedBlock_(parent->generatedBlock_),    
#endif
    firstInsnAddr_(parent->firstInsnAddr_),
    lastInsnAddr_(parent->lastInsnAddr_),
    blockEndAddr_(parent->blockEndAddr_),
    block_(block),
    version_(parent->version_) {
#if defined(cap_relocation)
    if (parent->origInstance_) {
        origInstance_ = block->instVer(parent->origInstance_->version_);
        jumpToBlock = new functionReplacement(*(parent->jumpToBlock));
    }
    else {
        origInstance_ = NULL;
        jumpToBlock = NULL;
    }
#endif
    // TODO relocation
}

bblInstance::~bblInstance() {
#if defined(cap_relocation)
    for (unsigned i = 0; i < insns_.size(); i++)
        delete insns_[i];
    insns_.clear();
    // appliedMods is deleted by the function....
    // jumpToBlock is deleted by the process....
#endif
}

int_basicBlock *bblInstance::block() const {
    assert(block_);
    return block_;
}

int_function *bblInstance::func() const {
    assert(block_);
    return block_->func();
}

process *bblInstance::proc() const {
    assert(block_);
    return block_->func()->proc();
}


Address bblInstance::equivAddr(bblInstance *origBBL, Address origAddr) const {
    // Divide and conquer
    if (origBBL == this)
         return origAddr;
#if defined(cap_relocation)
    if (origBBL == origInstance_) {
        assert(changedAddrs_.find(origAddr));
        return changedAddrs_[origAddr];
    }
#endif
    assert(0 && "Unhandled case in equivAddr");
    return 0;
}


void *bblInstance::getPtrToInstruction(Address addr) const {
    if (addr < firstInsnAddr_) return NULL;
    if (addr >= blockEndAddr_) return NULL;

#if defined(cap_relocation)
    // We might be relocated...
    if (generatedBlock_ != NULL) {
        addr -= firstInsnAddr_;
        return generatedBlock_.get_ptr(addr);
    }
#endif
    return func()->obj()->getPtrToInstruction(addr);
}

