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
//#include "InstrucIter.h"
#include "multiTramp.h"

#include "mapped_object.h"
#include "mapped_module.h"

#if defined(cap_relocation)
#include "reloc-func.h"
#endif

//std::string int_function::emptyString("");

using namespace Dyninst;


int int_function_count = 0;

// 
int_function::int_function(image_func *f,
			   Address baseAddr,
			   mapped_module *mod) :
#if defined( arch_ia64 )
	/* We can generate int_functions before parsing, so we need to be able
	   to update the allocs list, which needs the base address. */
	baseAddr_(baseAddr),
#endif			   
    ifunc_(f),
    mod_(mod),
    blockIDmap(intHash),
    instPsByAddr_(addrHash4),
#if defined(cap_relocation)
    generatedVersion_(0),
    installedVersion_(0),
    linkedVersion_(0),
#endif
    version_(0)
#if defined(os_windows) 
   , callingConv(unknown_call)
   , paramSize(0)
#endif
{
#if defined(ROUGH_MEMORY_PROFILE)
    int_function_count++;
    if ((int_function_count % 1000) == 0)
        fprintf(stderr, "int_function_count: %d (%d)\n",
                int_function_count, int_function_count*sizeof(int_function));
#endif
    

    addr_ = f->getOffset() + baseAddr;

    parsing_printf("%s: creating new proc-specific function at 0x%lx\n",
                   symTabName().c_str(), addr_);

    for (unsigned int i = 0; i < f->parRegions().size(); i++)
      {
	image_parRegion * imPR = f->parRegions()[i];
	int_parRegion * iPR = new int_parRegion(imPR, baseAddr, this); 
	parallelRegions_.push_back(iPR);
      }
    
    // We delay the creation of instPoints until they are requested;
    // this saves memory, and really, until something is asked for we
    // don't need it.  TODO: creation of an arbitrary instPoint should
    // trigger instPoint creation; someone may create an arbitrary at
    // a entry/exit/callsite.

    // Same with the flowGraph; we clone it from the image_func when
    // we need it.
    
    /* IA-64: create the cached allocs lazily. */
}


int_function::int_function(const int_function *parFunc,
                           mapped_module *childMod,
                           process *childP) :
    addr_(parFunc->addr_),
    ifunc_(parFunc->ifunc_),
    mod_(childMod),
    blockIDmap(intHash),
    instPsByAddr_(addrHash4),
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
         int_basicBlock *block = new int_basicBlock(parFunc->blockList[i], this,i);
         blockList.push_back(block);
     }
     // got the same blocks in the same order as the parent, so this is safe:
     blockIDmap = parFunc->blockIDmap;
     
     for (i = 0; i < parFunc->entryPoints_.size(); i++) {
         instPoint *parP = parFunc->entryPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         entryPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->exitPoints_.size(); i++) {
         instPoint *parP = parFunc->exitPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         exitPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->callPoints_.size(); i++) {
         instPoint *parP = parFunc->callPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         callPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->arbitraryPoints_.size(); i++) {
         instPoint *parP = parFunc->arbitraryPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         arbitraryPoints_.push_back(childIP);
     }

#if defined(arch_ia64)
	for( unsigned i = 0; i < parFunc->cachedAllocs.size(); i++ ) {
		cachedAllocs.push_back( parFunc->cachedAllocs[i] );
		}
#endif
     // TODO: relocated functions
}

int_function::~int_function() { 
    // ifunc_ doesn't keep tabs on us, so don't need to let it know.
    // mod_ is cleared by the mapped_object
    // blockList isn't allocated

    // instPoints are process level (should be deleted here and refcounted)

    // DEMO: incorrectly delete instPoints here
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        delete entryPoints_[i];
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        delete exitPoints_[i];
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        delete callPoints_[i];
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        delete arbitraryPoints_[i];
    }


#if defined(cap_relocation)
    for (unsigned i = 0; i < enlargeMods_.size(); i++)
        delete enlargeMods_[i];
#if defined (cap_use_pdvector)
    enlargeMods_.zap();
#else
    enlargeMods_.clear();
#endif
#endif
    
    for (unsigned i = 0; i < parallelRegions_.size(); i++)
      delete parallelRegions_[i];
      
}

#if defined( arch_ia64 )

pdvector< Address > & int_function::getAllocs() {
	if( cachedAllocs.size() != ifunc_->allocs.size() ) {
		cachedAllocs.clear();
	    for( unsigned i = 0; i < ifunc_->allocs.size(); i++ ) {
    	    cachedAllocs.push_back( baseAddr_ + ifunc_->allocs[i] );
		    }        
		}

	return cachedAllocs;
	} /* end getAllocs() */

AstNodePtr int_function::getFramePointerCalculator() {
    if(ifunc_->getFramePointerCalculator() == -1)
        return AstNodePtr();

    AstNodePtr constantZero = AstNode::operandNode(AstNode::Constant, (void *)0);
	AstNodePtr framePointer = AstNode::operandNode(AstNode::DataReg, (void *)(long unsigned int)ifunc_->getFramePointerCalculator());
	AstNodePtr moveFPtoDestination = AstNode::operatorNode(plusOp,
														 constantZero,
														 framePointer);
	return moveFPtoDestination;
}
	
bool * int_function::getUsedFPregs() {
	return ifunc_->usedFPregs;
	}

#endif /* defined( arch_ia64 ) */

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
#if defined (cap_use_pdvector)
        entryPoints_.reserve_exact(img_entries.size());
#endif
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
			if (!point) continue; // Can happen if we double-create
			assert(point);
            entryPoints_.push_back(point);
        }
    }
#if defined (cap_use_pdvector)
    entryPoints_.reserve_exact(entryPoints_.size());
#endif
    return entryPoints_;
}

const pdvector<instPoint*> &int_function::funcExits() {
    if (exitPoints_.size() == 0) {
        const pdvector<image_instPoint *> &img_exits = ifunc_->funcExits();
#if defined (cap_use_pdvector)
        exitPoints_.reserve_exact(img_exits.size());
#endif
        
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
						if (!point) continue; // Can happen if we double-create

            assert(point);
            exitPoints_.push_back(point);
        }
    }
#if defined (cap_use_pdvector)
    exitPoints_.reserve_exact(exitPoints_.size());
#endif
    return exitPoints_;
}

const pdvector<instPoint*> &int_function::funcCalls() {
    if (callPoints_.size() == 0) {
        const pdvector<image_instPoint *> &img_calls = ifunc_->funcCalls();
#if defined (cap_use_pdvector)
        callPoints_.reserve_exact(img_calls.size());
#endif
        
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
						if (!point) continue; // Can happen if we double-create

            assert(point);
            callPoints_.push_back(point);
        }
    }
#if defined (cap_use_pdvector)
    callPoints_.reserve_exact(callPoints_.size());
#endif
    return callPoints_;
}

const pdvector<instPoint*> &int_function::funcArbitraryPoints() {
  // We add these per-process, so there's no chance to have
  // a parse-level list
    return arbitraryPoints_;
}

instPoint *int_function::findInstPByAddr(Address addr) {
    // This only finds instPoints that have been previously created...
    // so don't bother parsing. 
    
    if (instPsByAddr_.find(addr))
        return instPsByAddr_[addr];

    // The above should have been sufficient... however, if we forked and have
    // a baseTramp that does not contain instrumentation, then there will never
    // be a instPointInstance created, and so no entry in instPsByAddr_. Argh.
    // So, if the lookup above failed, do the slow search through entries, 
    // exits, and calls - arbitraries should already exist.
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
	if (entryPoints_[i]->addr() == addr) return entryPoints_[i];
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
	if (exitPoints_[i]->addr() == addr) return exitPoints_[i];
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
	if (callPoints_[i]->addr() == addr) return callPoints_[i];
    }
    
    return NULL;
}

void int_function::registerInstPointAddr(Address addr, instPoint *inst) {
    instPoint *oldInstP = findInstPByAddr(addr);
    if (oldInstP) assert(inst == oldInstP);

    instPsByAddr_[addr] = inst;
}

void int_function::unregisterInstPointAddr(Address addr, instPoint* inst) {
    instPoint *oldInstP = findInstPByAddr(addr);
    assert(oldInstP == inst);

    instPsByAddr_.undef(addr);
}

void print_func_vector_by_pretty_name(std::string prefix,
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
AddressSpace *int_function::proc() const { return obj()->proc(); }

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


const std::vector<int_basicBlock *> &int_function::blocks() 
{
    parsing_printf("blocks() for %s, pointer %p\n", symTabName().c_str(), ifunc_);
    if (blockList.size() == 0) {
        Address base = getAddress() - ifunc_->getOffset();
        // TODO: create flowgraph pointer...
        const pdvector<image_basicBlock *> &img_blocks = ifunc_->blocks();
        
        for (unsigned i = 0; i < img_blocks.size(); i++) {
            blockList.push_back(new int_basicBlock(img_blocks[i],
                                                   base,
                                                   this,i));
            blockIDmap[img_blocks[i]->id()] = i;
        }
    }
    // And a quick consistency check...
#if defined (cap_use_pdvector)
    //blockList.reserve_exact(blockList.size());
#endif
    return blockList;
}

AddressSpace *int_basicBlock::proc() const {
    return func()->proc();
}

// Note that code sharing is masked at this level. That is, edges
// to and from a block that do not originate from the low-level function
// that this block's int_function represents will not be included in
// the returned block collection
void int_basicBlock::getSources(pdvector<int_basicBlock *> &ins) const {
    pdvector<image_edge *> ib_ins;
    ib_->getSources(ib_ins);
    for (unsigned i = 0; i < ib_ins.size(); i++) {
        if(ib_ins[i]->getType() != ET_CALL)
        {
            if(ib_ins[i]->getSource()->containedIn(func()->ifunc()))
            {
                // Note the mapping between int_basicBlock::id() and
                // image_basicBlock::id()
                unsigned img_id = ib_ins[i]->getSource()->id();
                unsigned int_id = func()->blockIDmap[img_id];
                ins.push_back(func()->blockList[int_id]);
            }
        }
    }
}

void int_basicBlock::getTargets(pdvector<int_basicBlock *> &outs) const {
    pdvector<image_edge *> ib_outs;
    ib_->getTargets(ib_outs);
    for (unsigned i = 0; i < ib_outs.size(); i++) {
        if(ib_outs[i]->getType() != ET_CALL)
        {
            if(ib_outs[i]->getTarget()->containedIn(func()->ifunc()))
            {
                // Note the mapping between int_basicBlock::id() and
                // image_basicBlock::id()
                unsigned img_id = ib_outs[i]->getTarget()->id();
                unsigned int_id = func()->blockIDmap[img_id];
                outs.push_back(func()->blockList[int_id]);
            }
        }
    }
}

EdgeTypeEnum int_basicBlock::getTargetEdgeType(int_basicBlock * target) const {
    pdvector<image_edge *> ib_outs;

    ib_->getTargets(ib_outs);
    for(unsigned i=0; i< ib_outs.size(); i++) {
        if(ib_outs[i]->getTarget() == target->ib_)
            return ib_outs[i]->getType();
    }

    return ET_NOEDGE;
}

EdgeTypeEnum int_basicBlock::getSourceEdgeType(int_basicBlock *source) const {
    pdvector<image_edge *> ib_ins;

    ib_->getSources(ib_ins);
    for(unsigned i=0; i< ib_ins.size(); i++) {
        if(ib_ins[i]->getSource() == source->ib_)
            return ib_ins[i]->getType();
    }

    return ET_NOEDGE;
}

int_basicBlock *int_basicBlock::getFallthrough() const {
    // We could keep it special...
  pdvector<image_edge *> ib_outs;
  ib_->getTargets(ib_outs);
  for (unsigned i = 0; i < ib_outs.size(); i++) {
    if (ib_outs[i]->getType() == ET_FALLTHROUGH ||
        ib_outs[i]->getType() == ET_FUNLINK ||
        ib_outs[i]->getType() == ET_COND_NOT_TAKEN) 
    {
      if (ib_outs[i]->getTarget()->containedIn(func()->ifunc())) {
        // Get the int_basicBlock equivalent of that image_basicBlock
         unsigned img_id = ib_outs[i]->getTarget()->id();
         unsigned int_id = func()->blockIDmap[img_id];
         return func()->blockList[int_id];
      }
      else {
        // Odd... fallthrough, but not in our function???
        assert(0);
      }
    }
  }
  
  return NULL;
}

bool int_basicBlock::needsRelocation() const {
    if(ib_->isShared()) {
        // If we've _already_ relocated, then we're no longer shared
        // because we have our own copy.

        if (instances_.size() > 1) {
            return false;
        }

        // We have only the one instance, so we're still shared.
        return true;
    }
    //else if(isEntryBlock() && func()->containsSharedBlocks())
    //    return true;
    else
        return false;
}

bool int_basicBlock::isEntryBlock() const { 
    return ib_->isEntryBlock(func_->ifunc());
}

unsigned int_function::getNumDynamicCalls()
{
   unsigned count=0;
   pdvector<instPoint *> callPoints = funcCalls();

   for (unsigned i=0; i<callPoints.size(); i++)
   {
      if (callPoints[i]->isDynamic())
          count++;
   }
   return count;
}

const string &int_function::symTabName() const { 
    return ifunc_->symTabName(); 
}

void int_function::debugPrint() const {
    fprintf(stderr, "Function debug dump (%p):\n", this);
    fprintf(stderr, "  Symbol table names:\n");
    for (unsigned i = 0; i < symTabNameVector().size(); i++) {
        fprintf(stderr, "    %s\n", symTabNameVector()[i].c_str());
    }
    fprintf(stderr, "  Demangled names:\n");
    for (unsigned j = 0; j < prettyNameVector().size(); j++) {
        fprintf(stderr, "    %s\n", prettyNameVector()[j].c_str());
    }
    fprintf(stderr, "  Typed names:\n");
    for (unsigned k = 0; k < typedNameVector().size(); k++) {
        fprintf(stderr, "    %s\n", typedNameVector()[k].c_str());
    }
    fprintf(stderr, "  Address: 0x%lx\n", getAddress());
    fprintf(stderr, "  Internal pointer: %p\n", ifunc_);
    fprintf(stderr, "  Object: %s (%p), module: %s (%p)\n", 
            obj()->fileName().c_str(), 
            obj(),
            mod()->fileName().c_str(),
            mod());
}

// Add to internal
// Add to mapped_object if a "new" name (true return from internal)
void int_function::addSymTabName(const std::string name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
        obj()->addFunctionName(this, name, true);
}

void int_function::addPrettyName(const std::string name, bool isPrimary) {
    if (ifunc()->addPrettyName(name, isPrimary))
        obj()->addFunctionName(this, name, false);
}

void int_function::getStaticCallers(pdvector< int_function * > &callers)
{
    pdvector<image_edge *> ib_ins;

    if(!ifunc_ || !ifunc_->entryBlock())
        return;

    ifunc_->entryBlock()->getSources(ib_ins);

    for (unsigned i = 0; i < ib_ins.size(); i++) {
        if(ib_ins[i]->getType() == ET_CALL)
        {   
            pdvector< image_func * > ifuncs;
            ib_ins[i]->getSource()->getFuncs(ifuncs);
            
            for(unsigned k=0;k<ifuncs.size();k++)
            {   
                int_function * f;
                f = obj()->findFunction(ifuncs[k]);
                
                callers.push_back(f);
            }
        }
    }
}

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

int_basicBlock::int_basicBlock(image_basicBlock *ib, Address baseAddr, int_function *func, int id) :
#if defined(arch_ia64)
    dataFlowIn(NULL),
    dataFlowOut(NULL),
    dataFlowGen(NULL),
    dataFlowKill(NULL),
#endif
    func_(func),
    ib_(ib),
    id_(id)
{
#if defined(ROUGH_MEMORY_PROFILE)
    int_basicBlock_count++;
    if ((int_basicBlock_count % 100) == 0)
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

int_basicBlock::int_basicBlock(const int_basicBlock *parent, int_function *func,int id) :
#if defined(arch_ia64)
    dataFlowGen(NULL),
    dataFlowKill(NULL),
#endif

    func_(func),
    ib_(parent->ib_),
    id_(id)
{
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
    // Do not delete dataFlowGen and dataFlowKill; they're legal pointers
    // don't kill func_;
    // don't kill ib_;
    for (unsigned i = 0; i < instances_.size(); i++) {
        delete instances_[i];
    }
#if defined (cap_use_pdvector)
    instances_.zap();
#else
    instances_.clear();
#endif
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
    if ((bblInstance_count % 100) == 0)
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
        block_->func()->obj()->proc()->addOrigRange(this);
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

AddressSpace *bblInstance::proc() const {
    assert(block_);
    return block_->func()->proc();
}


Address bblInstance::equivAddr(bblInstance *origBBL, Address origAddr) const {
    // Divide and conquer
    if (origBBL == this)
         return origAddr;
#if defined(cap_relocation)
    if (origBBL == getOrigInstance()) {
      for (unsigned i = 0; i < get_relocs().size(); i++) {
	if (get_relocs()[i]->origAddr == origAddr)
	  return get_relocs()[i]->relocAddr;
        if (get_relocs()[i]->relocAddr == origAddr)
            return get_relocs()[i]->origAddr;
      }
      return 0;
    }
#endif
    assert(0 && "Unhandled case in equivAddr");
    return 0;
}


void *bblInstance::getPtrToInstruction(Address addr) const {
    if (addr < firstInsnAddr_) return NULL;
    if (addr >= blockEndAddr_) return NULL;

#if defined(cap_relocation)
    if (version_ > 0) {
      // We might be relocated...
      if (getGeneratedBlock() != NULL) {
        addr -= firstInsnAddr();
        return getGeneratedBlock().get_ptr(addr);
      }
    }
#endif
    
    return func()->obj()->getPtrToInstruction(addr);

}

void *bblInstance::get_local_ptr() const {
#if defined(cap_relocation)
    if (!reloc_info) return NULL; 
    return reloc_info->generatedBlock_.start_ptr();
#else
    return NULL;
#endif
}

int bblInstance::version() const 
{
   return version_;
}

#if defined(cap_relocation)

const void *bblInstance::getPtrToOrigInstruction(Address addr) const {
  if (version_ > 0) {
    for (unsigned i = 0; i < get_relocs().size(); i++) {
      if (get_relocs()[i]->relocAddr == addr) {
         return (const void *) get_relocs()[i]->origPtr;
      }
    }
    assert(0);
    return NULL;
  }

  return getPtrToInstruction(addr);
}

unsigned bblInstance::getRelocInsnSize(Address addr) const {
  if (version_ > 0) {
    for (unsigned i = 0; i < get_relocs().size()-1; i++) {
      if (get_relocs()[i]->relocAddr == addr)
	return get_relocs()[i+1]->relocAddr - get_relocs()[i]->relocAddr;
    }
    if (get_relocs()[get_relocs().size()-1]->relocAddr == addr) {
      return blockEndAddr_ - get_relocs()[get_relocs().size()-1]->relocAddr;
    }
    assert(0);
    return 0;
  }
  // ... uhh...
  // This needs to get handled by the caller

  return 0;
}

void bblInstance::getOrigInstructionInfo(Address addr, const void *&ptr, 
                                         Address &origAddr, 
                                         unsigned &origSize) const 
{
   if (version_ > 0) {
      fprintf(stderr, "getPtrToOrigInstruction 0x%lx, version %d\n",
              addr, version_);
      for (unsigned i = 0; i < get_relocs().size(); i++) {
         if (get_relocs()[i]->relocAddr == addr) {
            fprintf(stderr, "... returning 0x%lx off entry %d\n",
                    get_relocs()[i]->origAddr,i);
            ptr = get_relocs()[i]->origPtr;
            origAddr = get_relocs()[i]->origAddr;
            if (i == (get_relocs().size()-1)) {
               origSize = blockEndAddr_ - get_relocs()[i]->relocAddr;
            }
            else
               origSize = get_relocs()[i+1]->relocAddr - get_relocs()[i]->relocAddr;
            return;
         }
      }
      assert(0);
      return;
   }
   
   // Must be handled by caller
   ptr = NULL;
   origAddr = 0;
   origSize = 0;
   return;
}

unsigned &bblInstance::maxSize() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->maxSize_;
}

unsigned &bblInstance::minSize() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->minSize_;
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

pdvector<bblInstance::reloc_info_t::relocInsn *> &bblInstance::get_relocs() const {
  assert(reloc_info);
  return reloc_info->relocs_;
}

pdvector<bblInstance::reloc_info_t::relocInsn *> &bblInstance::relocs() {
  if (!reloc_info)
    reloc_info = new reloc_info_t();
  return get_relocs();
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


bblInstance::reloc_info_t::reloc_info_t() : 
   maxSize_(0), 
   minSize_(0), 
   origInstance_(NULL), 
   jumpToBlock_(NULL)
{};

bblInstance::reloc_info_t::reloc_info_t(reloc_info_t *parent, 
                                        int_basicBlock *block)  :
   maxSize_(0),
   minSize_(0)
{
   if (parent->origInstance_)
      origInstance_ = block->instVer(parent->origInstance_->version());
   else
      origInstance_ = NULL;

   if (parent->jumpToBlock_)
       jumpToBlock_ = new functionReplacement(*(parent->jumpToBlock_));
   else
       jumpToBlock_ = NULL;

   for (unsigned i = 0; i < parent->relocs_.size(); i++) {
     relocs_.push_back( parent->relocs_[i] );
   }

}

bblInstance::reloc_info_t::~reloc_info_t() {
  for (unsigned i = 0; i < relocs_.size(); i++) {
    delete relocs_[i];
  }

#if defined (cap_use_pdvector)
  relocs_.zap();
#else
  relocs_.clear();
#endif

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


// Dig down to the low-level block of b, find the low-level functions
// that share it, and map up to int-level functions and add them
// to the funcs list.
bool int_function::getSharingFuncs(int_basicBlock *b,
                                   pdvector< int_function *> & funcs)
{
    bool ret = false;
    if(!b->hasSharedBase())
        return ret;

    pdvector<image_func *> lfuncs;

    b->llb()->getFuncs(lfuncs);
    for(unsigned i=0;i<lfuncs.size();i++) {
        image_func *ll_func = lfuncs[i];
        int_function *hl_func = obj()->findFunction(ll_func);
        assert(hl_func);

        if (hl_func == this) continue;

        // Let's see if we've already got it...
        bool found = false;
        for (unsigned j = 0; j < funcs.size(); j++) {
            if (funcs[j] == hl_func) {
                found = true;
                break;
            }
        }
        if (!found) {
            ret = true;
            funcs.push_back(hl_func);
        }
    }

    return ret;
}

// Find overlapping functions via checking all basic blocks. We might be
// able to check only exit points; but we definitely need to check _all_
// exits so for now we're checking everything.

bool int_function::getOverlappingFuncs(pdvector<int_function *> &funcs) {
    bool ret = false;

    funcs.clear();

    // Create the block list.
    blocks();

    for (unsigned i = 0; i < blockList.size(); i++) {
        if (getSharingFuncs(blockList[i],
                            funcs))
            ret = true;
    }

    return ret;
}

Address int_function::get_address() const 
{
#if !defined(cap_relocation)
   return getAddress();
#else
   if (!entryPoints_.size())
      return getAddress();
   
   instPoint *entryPoint = entryPoints_[0];
   int_basicBlock *block = entryPoint->block();
   bblInstance *inst = block->instVer(installedVersion_);
   return inst->firstInsnAddr();
#endif 
}

unsigned int_function::get_size() const 
{
   assert(0);
   return 0x0;
}

std::string int_function::get_name() const
{
   return symTabName();
}

bblInstance * bblInstance::getTargetBBL() {
    // Check to see if we need to fix up the target....
    pdvector<int_basicBlock *> targets;
    block_->getTargets(targets);
    
    // We have edge types on the internal data, so we drop down and get that. 
    // We want to find the "branch taken" edge and override the destination
    // address for that guy.
    pdvector<image_edge *> out_edges;
    block_->llb()->getTargets(out_edges);
    
    // May be greater; we add "extra" edges for things like function calls, etc.
    assert (out_edges.size() >= targets.size());
    
    for (unsigned edge_iter = 0; edge_iter < out_edges.size(); edge_iter++) {
        EdgeTypeEnum edgeType = out_edges[edge_iter]->getType();
        // Update to Nate's commit...
        if ((edgeType == ET_COND_TAKEN) ||
            (edgeType == ET_DIRECT)) {
            // Got the right edge... now find the matching high-level
            // basic block
            image_basicBlock *llTarget = out_edges[edge_iter]->getTarget();
            int_basicBlock *hlTarget = NULL;
            for (unsigned t_iter = 0; t_iter < targets.size(); t_iter++) {
                // Should be the same index, but this is a small set...
                if (targets[t_iter]->llb() == llTarget) {
                    hlTarget = targets[t_iter];
                    break;
                }
            }
            assert(hlTarget != NULL);
            return hlTarget->instVer(version_);
        }
    }
    return NULL;
}

bblInstance * bblInstance::getFallthroughBBL() {
    // Check to see if we need to fix up the target....
    pdvector<int_basicBlock *> targets;
    block_->getTargets(targets);
    
    // We have edge types on the internal data, so we drop down and get that. 
    // We want to find the "branch taken" edge and override the destination
    // address for that guy.
    pdvector<image_edge *> out_edges;
    block_->llb()->getTargets(out_edges);
    
    // May be greater; we add "extra" edges for things like function calls, etc.
    assert (out_edges.size() >= targets.size());
    
    for (unsigned edge_iter = 0; edge_iter < out_edges.size(); edge_iter++) {
        EdgeTypeEnum edgeType = out_edges[edge_iter]->getType();
        // Update to Nate's commit...
        if ((edgeType == ET_COND_NOT_TAKEN) ||
            (edgeType == ET_FALLTHROUGH) ||
            (edgeType == ET_FUNLINK)) {
            // Got the right edge... now find the matching high-level
            // basic block
            image_basicBlock *llTarget = out_edges[edge_iter]->getTarget();
            int_basicBlock *hlTarget = NULL;
            for (unsigned t_iter = 0; t_iter < targets.size(); t_iter++) {
                // Should be the same index, but this is a small set...
                if (targets[t_iter]->llb() == llTarget) {
                    hlTarget = targets[t_iter];
                    break;
                }
            }
            assert(hlTarget != NULL);
            
            return hlTarget->instVer(version_);
        }
    }
    return NULL;
}
