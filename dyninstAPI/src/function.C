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
#include "process.h"
#include "instPoint.h"
//#include "BPatch_basicBlockLoop.h"
//#include "BPatch_basicBlock.h"
#include "InstrucIter.h"
#include "multiTramp.h"

#include "mapped_object.h"
#include "mapped_module.h"

#if defined(cap_relocation)
#include "reloc-func.h"
#endif

pdstring int_function::emptyString("");


int int_function_count = 0;

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
#if defined(cap_relocation)
    generatedVersion_(0),
    installedVersion_(0),
    linkedVersion_(0),
#endif
    version_(0)
{
#if defined(ROUGH_MEMORY_PROFILE)
    int_function_count++;
    if ((int_function_count % 10) == 0)
        fprintf(stderr, "int_function_count: %d (%d)\n",
                int_function_count, int_function_count*sizeof(int_function));
#endif

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
#if defined(cap_relocation)
    generatedVersion_(parFunc->generatedVersion_),
    installedVersion_(parFunc->installedVersion_),
    linkedVersion_(parFunc->linkedVersion_),
#endif
    version_(parFunc->version_)
 {
     unsigned i; // Windows hates "multiple definitions"

     // Construct the raw blocklist;
     for (i = 0; i < parFunc->blockList.size(); i++) {
         int_basicBlock *block = new int_basicBlock(parFunc->blockList[i], this);
         blockList.push_back(block);
     }
     
     for (i = 0; i < parFunc->entryPoints_.size(); i++) {
         instPoint *parP = parFunc->entryPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
         entryPoints_.push_back(childP);
     }

     for (i = 0; i < parFunc->exitPoints_.size(); i++) {
         instPoint *parP = parFunc->exitPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
         exitPoints_.push_back(childP);
     }

     for (i = 0; i < parFunc->callPoints_.size(); i++) {
         instPoint *parP = parFunc->callPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childP = instPoint::createForkedPoint(parP, block);
         callPoints_.push_back(childP);
     }

     for (i = 0; i < parFunc->arbitraryPoints_.size(); i++) {
         instPoint *parP = parFunc->arbitraryPoints_[i];
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
    // instPoints are process level (should be deleted here and refcounted)

#if defined(cap_relocation)
    for (unsigned i = 0; i < enlargeMods_.size(); i++)
        delete enlargeMods_[i];
    enlargeMods_.zap();
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
        entryPoints_.reserve_exact(img_entries.size());
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
        exitPoints_.reserve_exact(img_exits.size());
        
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
        callPoints_.reserve_exact(img_calls.size());
        
        for (unsigned i = 0; i < img_calls.size(); i++) {
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_calls[i]->offset() - img_calls[i]->func()->getOffset();
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for call point at 0x%lx (0x%lx) (func 0x%lx to 0x%lx, %s/%s)\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP(),
                        symTabName().c_str(),
                        obj()->fileName().c_str());
                
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
    parsing_printf("blocks() for %s\n", symTabName().c_str());
    if (blockList.size() == 0) {
        Address base = getAddress() - ifunc_->getOffset();
        // TODO: create flowgraph pointer...
        const pdvector<image_basicBlock *> &img_blocks = ifunc_->blocks();
        
        for (unsigned i = 0; i < img_blocks.size(); i++) {
            blockList.push_back(new int_basicBlock(img_blocks[i],
                                                   base,
                                                   this));
        }
    }
    // And a quick consistency check...
    return blockList;
}

process *int_basicBlock::proc() const {
    return func()->proc();
}

void int_basicBlock::getSources(pdvector<int_basicBlock *> &ins) const {
    pdvector<image_basicBlock *> ib_ins;
    ib_->getSources(ib_ins);
    for (unsigned i = 0; i < ib_ins.size(); i++) {
        unsigned id = ib_ins[i]->id();
        ins.push_back(func()->blockList[id]);
    }
}

void int_basicBlock::getTargets(pdvector<int_basicBlock *> &outs) const {
    pdvector<image_basicBlock *> ib_outs;
    ib_->getTargets(ib_outs);
    for (unsigned i = 0; i < ib_outs.size(); i++) {
        unsigned id = ib_outs[i]->id();
        outs.push_back(func()->blockList[id]);
    }
}

int_basicBlock *int_basicBlock::getFallthrough() const {
    // We could keep it special...
    pdvector<int_basicBlock *> outs;
    getTargets(outs);
    for (unsigned i = 0; i < outs.size(); i++) {
        if (outs[i]->origInstance()->firstInsnAddr() == origInstance()->endAddr())
            return outs[i];
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

// Add to internal
// Add to mapped_object if a "new" name (true return from internal)
void int_function::addSymTabName(const pdstring name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
        obj()->addFunctionName(this, name, true);
}

void int_function::addPrettyName(const pdstring name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
        obj()->addFunctionName(this, name, false);
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

image_func *int_function::ifunc() {
    return ifunc_;
}

#if defined(arch_ia64)
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
#endif

int int_basicBlock_count = 0;

int_basicBlock::int_basicBlock(const image_basicBlock *ib, Address baseAddr, int_function *func) :
#if defined(arch_ia64)
    dataFlowIn(NULL),
    dataFlowOut(NULL),
    dataFlowGen(NULL),
    dataFlowKill(NULL),
#endif
#if defined (os_aix) || defined(arch_x86_64)
    gen(NULL),
    genFP(NULL),
    kill(NULL),
    killFP(NULL),
    in(NULL),
    inFP(NULL),
    out(NULL),
    outFP(NULL),
#endif
    func_(func),
    ib_(ib)
{
#if defined(ROUGH_MEMORY_PROFILE)
    int_basicBlock_count++;
    if ((int_basicBlock_count % 10) == 0)
        fprintf(stderr, "int_basicBlock_count: %d (%d)\n",
                int_basicBlock_count, int_basicBlock_count*sizeof(int_basicBlock));
#endif

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
#if defined(arch_ia64)
    dataFlowGen(NULL),
    dataFlowKill(NULL),
#endif
#if defined (os_aix) || defined(arch_x86_64)
    gen(NULL),
    genFP(NULL),
    kill(NULL),
    killFP(NULL),
    in(NULL),
    inFP(NULL),
    out(NULL),
    outFP(NULL),
#endif
    func_(func),
    ib_(parent->ib_) {
    for (unsigned i = 0; i < parent->instances_.size(); i++) {
        bblInstance *bbl = new bblInstance(parent->instances_[i], this);
        instances_.push_back(bbl);
        func_->addBBLInstance(bbl);
    }
}

int_basicBlock::~int_basicBlock() {
#if defined(arch_ia64)
    if (dataFlowIn) delete dataFlowIn;
    if (dataFlowOut) delete dataFlowOut;
#endif
#if defined (os_aix) || defined(arch_x86_64)
    if (gen) delete gen;
    if (genFP) delete genFP;
    if (kill) delete kill;
    if (killFP) delete killFP;
    if (in) delete in;
    if (inFP) delete inFP;
    if (out) delete out;
    if (outFP) delete outFP;
#endif
    // Do not delete dataFlowGen and dataFlowKill; they're legal pointers
    // don't kill func_;
    // don't kill ib_;
    for (unsigned i = 0; i < instances_.size(); i++) {
        delete instances_[i];
    }
    instances_.zap();
}

bblInstance *int_basicBlock::origInstance() const {
    assert(instances_.size());
    return instances_[0];
}

bblInstance *int_basicBlock::instVer(unsigned id) const {
    if (id >= instances_.size())
        fprintf(stderr, "ERROR: requesting bblInstance %u, only %d known\n",
                id, instances_.size());
    return instances_[id];
}

void int_basicBlock::removeVersion(unsigned id) {
    if (id >= instances_.size()) {
        fprintf(stderr, "ERROR: deleting bblInstance %u, only %d known\n",
                id, instances_.size());
        return;
    }
    if (id < (instances_.size() - 1)) {
        fprintf(stderr, "ERROR: deleting bblInstance %u, not last\n",
                id);
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

int bblInstance_count = 0;

bblInstance::bblInstance(Address start, Address last, Address end, int_basicBlock *parent, int version) : 
#if defined(cap_relocation)
    reloc_info(NULL),
#endif
    firstInsnAddr_(start),
    lastInsnAddr_(last),
    blockEndAddr_(end),
    block_(parent),
    version_(version)
{
#if defined(ROUGH_MEMORY_PROFILE)
    bblInstance_count++;
    if ((bblInstance_count % 10) == 0)
        fprintf(stderr, "bblInstance_count: %d (%d)\n",
                bblInstance_count, bblInstance_count*sizeof(bblInstance));
#endif


    // And add to the mapped_object code range
    block_->func()->obj()->codeRangesByAddr_.insert(this);
};

bblInstance::bblInstance(int_basicBlock *parent, int version) : 
#if defined(cap_relocation)
    reloc_info(NULL),
#endif
    firstInsnAddr_(0),
    lastInsnAddr_(0),
    blockEndAddr_(0),
    block_(parent),
    version_(version)
{
    // And add to the mapped_object code range
    //block_->func()->obj()->codeRangesByAddr_.insert(this);
};

bblInstance::bblInstance(const bblInstance *parent, int_basicBlock *block) :
#if defined(cap_relocation)
    reloc_info(NULL),
#endif
    firstInsnAddr_(parent->firstInsnAddr_),
    lastInsnAddr_(parent->lastInsnAddr_),
    blockEndAddr_(parent->blockEndAddr_),
    block_(block),
    version_(parent->version_) {
#if defined(cap_relocation)
   if (parent->reloc_info) {
      reloc_info = new reloc_info_t(parent->reloc_info, block);
   }
#endif

    // If the bblInstance is the original version, add to the mapped_object
    // code range; if it is the product of relocation, add it to the
    // process.
    if(version_ == 0)
        block_->func()->obj()->codeRangesByAddr_.insert(this);
    else
        block_->func()->obj()->proc()->addCodeRange(this);
}

bblInstance::~bblInstance() {
#if defined(cap_relocation)
   if (reloc_info)
      delete reloc_info;
#endif
}

int_basicBlock *bblInstance::block() const {
    assert(block_);
    return block_;
}

void int_basicBlock::setHighLevelBlock(void *newb)
{
   highlevel_block = newb;
}

void *int_basicBlock::getHighLevelBlock() const {
   return highlevel_block;
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
    if (origBBL == getOrigInstance()) {
       assert(getChangedAddrs().find(origAddr));
       return getChangedAddrs()[origAddr];
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
    if (getGeneratedBlock() != NULL) {
        addr -= firstInsnAddr();
        return getGeneratedBlock().get_ptr(addr);
    }
#endif
    return func()->obj()->getPtrToInstruction(addr);
}

#if defined(cap_relocation)

dictionary_hash<Address, Address> &bblInstance::changedAddrs() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->changedAddrs_;
}

pdvector<instruction *> &bblInstance::insns() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->insns_;
}

unsigned &bblInstance::maxSize() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->maxSize_;
}

bblInstance *&bblInstance::origInstance() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->origInstance_;
}

pdvector<funcMod *> &bblInstance::appliedMods() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->appliedMods_;
}

codeGen &bblInstance::generatedBlock() {
  if (!reloc_info)
      reloc_info = new reloc_info_t();
  return reloc_info->generatedBlock_;
}

functionReplacement *&bblInstance::jumpToBlock() {
  if (!reloc_info)
      reloc_info = new reloc_info_t();
  return reloc_info->jumpToBlock_;
}

dictionary_hash<Address, Address> &bblInstance::getChangedAddrs() const {
   assert(reloc_info);
   return reloc_info->changedAddrs_;
}

pdvector<instruction *> &bblInstance::getInsns() const {
   assert(reloc_info);
   return reloc_info->insns_;
}

unsigned bblInstance::getMaxSize() const {
   if (!reloc_info)
      return 0;
   return reloc_info->maxSize_;
}

bblInstance *bblInstance::getOrigInstance() const {
   if (!reloc_info)
      return NULL;
   return reloc_info->origInstance_;
}

pdvector<funcMod *> &bblInstance::getAppliedMods() const {
   assert(reloc_info);
   return reloc_info->appliedMods_;
}

codeGen &bblInstance::getGeneratedBlock() const {
   assert(reloc_info);
   return reloc_info->generatedBlock_;
}

functionReplacement *bblInstance::getJumpToBlock() const {
   if (!reloc_info)
      return NULL;
   return reloc_info->jumpToBlock_;
}

int bblInstance::version() const {
   return version_;
}

bblInstance::reloc_info_t::reloc_info_t() : 
   changedAddrs_(addrHash4), 
   maxSize_(0), 
   origInstance_(NULL), 
   jumpToBlock_(NULL) 
{};

bblInstance::reloc_info_t::reloc_info_t(reloc_info_t *parent, 
                                        int_basicBlock *block)  :
   changedAddrs_(addrHash4), 
   maxSize_(0)
{
   changedAddrs_ = parent->changedAddrs_;

   if (parent->origInstance_)
      origInstance_ = block->instVer(parent->origInstance_->version());
   else
      origInstance_ = NULL;

   if (parent->jumpToBlock_)
       jumpToBlock_ = new functionReplacement(*(parent->jumpToBlock_));
   else
       jumpToBlock_ = NULL;
}

bblInstance::reloc_info_t::~reloc_info_t() {
   for (unsigned i = 0; i < insns_.size(); i++)
      delete insns_[i];
   insns_.zap();
   // appliedMods is deleted by the function....
   // jumpToBlock is deleted by the process....
};

#endif

int_basicBlock *functionReplacement::source() { 
   return sourceBlock_; 
}

int_basicBlock *functionReplacement::target() { 
   return targetBlock_; 
}

unsigned functionReplacement::sourceVersion() { 
   return sourceVersion_; 
}

unsigned functionReplacement::targetVersion() { 
   return targetVersion_; 
}
