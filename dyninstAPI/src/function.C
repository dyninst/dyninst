/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
 
// $Id: function.C,v 1.10 2005/03/02 19:44:45 bernat Exp 

#include "function.h"
#include "process.h"
#include "instPoint.h"
#include "multiTramp.h"

#include "mapped_object.h"
#include "mapped_module.h"

#if defined(cap_relocation)
#include "reloc-func.h"
#endif

//std::string int_function::emptyString("");

#include "Parsing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;


int int_function_count = 0;

// 
int_function::int_function(image_func *f,
			   Address baseAddr,
			   mapped_module *mod) :
    ifunc_(f),
    mod_(mod),
    blockIDmap(intHash),
    instPsByAddr_(addrHash4),
    isBeingInstrumented_(false),
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
    ptrAddr_ = (f->getPtrOffset() ? f->getPtrOffset() + baseAddr : 0);

    parsing_printf("%s: creating new proc-specific function at 0x%lx\n",
                   symTabName().c_str(), addr_);

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
    ptrAddr_(parFunc->ptrAddr_),
    ifunc_(parFunc->ifunc_),
    mod_(childMod),
    blockIDmap(intHash),
    instPsByAddr_(addrHash4),
    isBeingInstrumented_(parFunc->isBeingInstrumented_),
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

    // int_basicBlocks
    for(unsigned i = 0; i < blockList.size(); i++) {
        delete blockList[i];
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
        pdvector<image_instPoint *> img_entries;
        ifunc_->funcEntries(img_entries);
#if defined (cap_use_pdvector)
        entryPoints_.reserve_exact(img_entries.size());
#endif
        for (unsigned i = 0; i < img_entries.size(); i++) {

            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_entries[i]->offset()-ifunc_->getOffset();
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
        pdvector<image_instPoint *> img_exits;
        ifunc_->funcExits(img_exits);
#if defined (cap_use_pdvector)
        exitPoints_.reserve_exact(img_exits.size());
#endif
        
        for (unsigned i = 0; i < img_exits.size(); i++) {
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_exits[i]->offset()-ifunc_->getOffset();
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
        pdvector<image_instPoint *> img_calls;
        ifunc_->funcCalls(img_calls);
#if defined (cap_use_pdvector)
        callPoints_.reserve_exact(img_calls.size());
#endif
        
        for (unsigned i = 0; i < img_calls.size(); i++) {
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_calls[i]->offset()-ifunc_->getOffset();
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for call point at 0x%lx (0x%lx) (func 0x%lx to 0x%lx, %s/%s)\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP(),
                        symTabName().c_str(),
                        obj()->fileName().c_str());
                debugPrint();
                
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
    int i = 0;

    if (blockList.size() == 0) {
        Address base = getAddress() - ifunc_->getOffset();

        Function::blocklist & img_blocks = ifunc_->blocks();
        Function::blocklist::iterator sit = img_blocks.begin();

        for( ; sit != img_blocks.end(); ++sit) {
            image_basicBlock *b = (image_basicBlock*)*sit;
            blockList.push_back(new int_basicBlock(b,
                                                   base,
                                                   this,i));
            blockIDmap[b->id()] = i;
            ++i;
        }
    }
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

    /* Only allow edges that are within this current function; hide sharing */
    /* Also avoid CALL and RET edges */
    SingleContext epred(func()->ifunc(),true,true);
    Intraproc epred2(&epred);

    Block::edgelist & ib_ins = ib_->sources();
    Block::edgelist::iterator eit = ib_ins.begin(&epred2);

    for( ; eit != ib_ins.end(); ++eit) {
        // FIXME debugging assert
        assert((*eit)->type() != CALL && (*eit)->type() != RET);

        //if((*eit)->type() != CALL)
        //{
                image_basicBlock * sb = (image_basicBlock*)(*eit)->src();
                // Note the mapping between int_basicBlock::id() and
                // image_basicBlock::id()
                unsigned img_id = sb->id();
                unsigned int_id = func()->blockIDmap[img_id];
                ins.push_back(func()->blockList[int_id]);
        //}
    }
}

void int_basicBlock::getTargets(pdvector<int_basicBlock *> &outs) const {
    SingleContext epred(func()->ifunc(),true,true);
    Intraproc epred2(&epred);
    NoSinkPredicate epred3(&epred2);

    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred3);

    for( ; eit != ib_outs.end(); ++eit) {
        // FIXME debugging assert
        assert((*eit)->type() != CALL && (*eit)->type() != RET);
        //if((*eit)->type() != CALL)
        //{
                image_basicBlock * tb = (image_basicBlock*)(*eit)->trg();
                // Note the mapping between int_basicBlock::id() and
                // image_basicBlock::id()
                unsigned img_id = tb->id();
                unsigned int_id = func()->blockIDmap[img_id];
                outs.push_back(func()->blockList[int_id]);
        //}
    }
}

EdgeTypeEnum int_basicBlock::getTargetEdgeType(int_basicBlock * target) const {
    SingleContext epred(func()->ifunc(),true,true);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred);
    for( ; eit != ib_outs.end(); ++eit)
        if((*eit)->trg() == target->ib_)
            return (*eit)->type();
    return NOEDGE;
}

EdgeTypeEnum int_basicBlock::getSourceEdgeType(int_basicBlock *source) const {
    SingleContext epred(func()->ifunc(),true,true);
    Block::edgelist & ib_ins = ib_->sources();
    Block::edgelist::iterator eit = ib_ins.begin(&epred);
    for( ; eit != ib_ins.end(); ++eit)
        if((*eit)->trg() == source->ib_)
            return (*eit)->type();
    return NOEDGE;
}

int_basicBlock *int_basicBlock::getFallthrough() const {
    SingleContext epred(func()->ifunc(),true,true);
    NoSinkPredicate epred2(&epred);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred2);
    for( ; eit != ib_outs.end(); ++eit) {
        Edge * e = *eit;
        if(e->type() == FALLTHROUGH ||
           e->type() == CALL_FT ||
           e->type() == COND_NOT_TAKEN)
        {
            unsigned img_id = ((image_basicBlock*)e->trg())->id();
            unsigned int_id = func()->blockIDmap[img_id];
            return func()->blockList[int_id];
        }
    }
    return NULL;
}

bool int_basicBlock::needsRelocation() const {
   if(ib_->isShared() || ib_->needsRelocation()) {
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
    for(std::vector<int_basicBlock*>::const_iterator cb = blockList.begin();
        cb != blockList.end();
        ++cb)
    {
        bblInstance* orig = (*cb)->origInstance();
        fprintf(stderr, "  Block start 0x%lx, end 0x%lx\n", orig->firstInsnAddr(),
                orig->endAddr());
    }
}

// Add to internal
// Add to mapped_object if a "new" name (true return from internal)
void int_function::addSymTabName(const std::string name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
        obj()->addFunctionName(this, name, mapped_object::mangledName);
}

void int_function::addPrettyName(const std::string name, bool isPrimary) {
    if (ifunc()->addPrettyName(name, isPrimary))
        obj()->addFunctionName(this, name, mapped_object::prettyName);
}

void int_function::getStaticCallers(pdvector< int_function * > &callers)
{
    pdvector<image_edge *> ib_ins;

    if(!ifunc_ || !ifunc_->entryBlock())
        return;

    Block::edgelist & ins = ifunc_->entryBlock()->sources();
    Block::edgelist::iterator eit = ins.begin();
    for( ; eit != ins.end(); ++eit) {
        if((*eit)->type() == CALL)
        {   
            vector<Function *> ifuncs;
            (*eit)->src()->getFuncs(ifuncs);
            vector<Function *>::iterator ifit = ifuncs.begin();
            for( ; ifit != ifuncs.end(); ++ifit)
            {   
                int_function * f;
                f = obj()->findFunction((image_func*)*ifit);
                
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

int int_basicBlock_count = 0;

int_basicBlock::int_basicBlock(image_basicBlock *ib, Address baseAddr, int_function *func, int id) :
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
        fprintf(stderr, "ERROR: requesting bblInstance %u, only %ld known\n",
                id, (long) instances_.size());
    return instances_[id];
}

void int_basicBlock::removeVersion(unsigned id) {
    if (id >= instances_.size()) {
        fprintf(stderr, "ERROR: deleting bblInstance %u, only %ld known\n",
                id, (long) instances_.size());
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

pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &bblInstance::get_relocs() const {
  assert(reloc_info);
  return reloc_info->relocs_;
}

pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &bblInstance::relocs() {
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
  // XXX this wasn't safe, as copies of bblInstances
  //     reference the same relocInsns.
  //     relocs_ now holds shared_ptrs
  //for (unsigned i = 0; i < relocs_.size(); i++) {
  //  delete relocs_[i];
  //}

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

    vector<Function *> lfuncs;
    b->llb()->getFuncs(lfuncs);
    vector<Function *>::iterator fit = lfuncs.begin();
    for( ; fit != lfuncs.end(); ++fit) {
        image_func *ll_func = static_cast<image_func*>(*fit);
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
    Block::edgelist & out_edges = block_->llb()->targets();
    
    // May be greater; we add "extra" edges for things like function calls, etc.
    assert (out_edges.size() >= targets.size());
   
    Block::edgelist::iterator eit = out_edges.begin();
    for( ; eit != out_edges.end(); ++eit) {
        EdgeTypeEnum edgeType = (*eit)->type();
        if ((edgeType == COND_TAKEN) ||
            (edgeType == DIRECT)) {
            // Got the right edge... now find the matching high-level
            // basic block
            image_basicBlock *llTarget = (image_basicBlock*)(*eit)->trg();
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
    Block::edgelist & out_edges = block_->llb()->targets();
    
    // May be greater; we add "extra" edges for things like function calls, etc.
    assert (out_edges.size() >= targets.size());

    NoSinkPredicate nsp;
    
    Block::edgelist::iterator eit = out_edges.begin(&nsp);
    for( ; eit != out_edges.end(); ++eit) {
        EdgeTypeEnum edgeType = (*eit)->type();
        if ((edgeType == COND_NOT_TAKEN) ||
            (edgeType == FALLTHROUGH) ||
            (edgeType == CALL_FT)) {
            // Got the right edge... now find the matching high-level
            // basic block
            image_basicBlock *llTarget = (image_basicBlock*)(*eit)->trg();
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


bool int_function::performInstrumentation(bool stopOnFailure,
                                          pdvector<instPoint *> &failedInstPoints) {

    // We have the following possible side-effects:
    // 
    // 1) Generating an instPoint (e.g., creating the multiTramp and its code)
    //    may determine the function is too small to fit the instrumentation,
    //    requiring relocation.
    // 2) Instrumenting a shared block may also trigger relocation as a 
    //    mechanism to unwind the sharing. 
    //
    // 3) Relocation will add additional instPoint instances.
    //

    // Thus, we have the following order of events:
    //
    // 1) Generate all instPoints that actually have instrumentation. 
    //    This will identify whether the function requires relocation.
    // 2) If relocation is necessary:
    // 2a) Generate relocation; this will create the function copy and update 
    //     function-local data structures.
    // 2b) Install relocation; this will update process-level data structures and
    //     copy the relocated function into the address space.
    // 2c) Generate instPoints again to handle any new instPointInstances that
    //     have showed up. This should _not_ result in required relocation.
    // 3) Install instPoints
    // 4) Link (relocated copy of the function) and instPoints.

    // Assumptions: 
    // 1) calling generate/install/link on "empty" instPoints has no effect.
    // 2) Generate/install/link operations are idempotent.

    // Let's avoid a lot of work and collect up all instPoints that have
    // something interesting going on; that is, that have instrumentation
    // added since the last time something came up. 

#if defined(arch_x86_64)
  if(proc()->getAddressWidth() == 8)
  {
    ia32_set_mode_64(true);
  }
  else
  {
    ia32_set_mode_64(false);
  }
#endif  

  if (isBeingInstrumented_) return false;
  isBeingInstrumented_ = true;

    pdvector<instPoint *> newInstrumentation;
    pdvector<instPoint *> anyInstrumentation;

    getNewInstrumentation(newInstrumentation);
    getAnyInstrumentation(anyInstrumentation);

    // Quickie correctness assert: newInstrumentation \subseteq anyInstrumentation
    assert(newInstrumentation.size() <= anyInstrumentation.size()); 

    bool relocationRequired = false;

    // Step 1: Generate all new instrumentation
    generateInstrumentation(newInstrumentation, failedInstPoints, relocationRequired); 
    
    if (failedInstPoints.size() && stopOnFailure) {
      isBeingInstrumented_ = false;
      return false;
    }

#if defined(cap_relocation)
    // Step 2: is relocation necessary?
    if (relocationRequired) {
        // Yar.
        // This will calculate the sizes required for our basic blocks.
        expandForInstrumentation();
        
        // And keep a list of other functions that need relocation due to
        // sharing.
        pdvector<int_function *> need_reloc;

        // Generate the relocated copy of the function.
        relocationGenerate(enlargeMods(), 0, need_reloc);
        
        // Install the relocated copy of the function.
        relocationInstall();

        // Aaaand link it. 
        pdvector<codeRange *> overwritten_objs;
        relocationLink(overwritten_objs);

        // We've added a new version of the function; therefore, we need
        // to update _everything_ that's been instrumented. 
        // We do this in two ways. First, we call generate on all
        // instPoints to get them in the right place.
        // Second, we replace newInstrumentation with anyInstrumentation,
        // then call install/link as normal.

        // Clear the failedInstPoints vector first; we'll re-generate
        // it in any case.
        failedInstPoints.clear();
        relocationRequired = false;

        // Update instPoint instances to include the new function
        for (unsigned i = 0; i < anyInstrumentation.size(); i++) {
            anyInstrumentation[i]->updateInstancesBatch();
        }
        // We _explicitly_ don't call the corresponding updateInstancesFinalize,
        // as the only purpose of that function is to regenerate instrumentation;
        // we do that explicitly below.

        generateInstrumentation(anyInstrumentation,
                                failedInstPoints,
                                relocationRequired);
        // I'm commenting this out; I originally thought it would be the case,
        // but on further thought the original instPoint will _still_ be asking
        // for relocation. 
        //assert(relocationRequired == false);

        newInstrumentation = anyInstrumentation;

	// If there are any other functions that we need to relocate
	// due to this relocation, handle it now. We don't care if they
	// fail to install instrumentation though.
	pdvector<instPoint *> dontcare;
	for (unsigned i = 0; i < need_reloc.size(); i++) {
	  need_reloc[i]->performInstrumentation(false, dontcare);
	}
    }
#endif

    // Okay, back to what we were doing...
    
    installInstrumentation(newInstrumentation,
                           failedInstPoints);
    linkInstrumentation(newInstrumentation,
                        failedInstPoints);

    if (obj()->isSharedLib()) {
        //printf("===> Instrumenting function in shared library: %s [%s]\n",
                //prettyName().c_str(), obj()->fileName().c_str());
        obj()->setDirty();
    }

    isBeingInstrumented_ = false;
    return (failedInstPoints.size() == 0);
}

void int_function::getNewInstrumentation(pdvector<instPoint *> &ret) {
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        if (entryPoints_[i]->hasNewInstrumentation()) {
            ret.push_back(entryPoints_[i]);
        }
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        if (exitPoints_[i]->hasNewInstrumentation()) {
            ret.push_back(exitPoints_[i]);
        }
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        if (callPoints_[i]->hasNewInstrumentation()) {
            ret.push_back(callPoints_[i]);
        }
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        if (arbitraryPoints_[i]->hasNewInstrumentation()) {
            ret.push_back(arbitraryPoints_[i]);
        }
    }
}

void int_function::getAnyInstrumentation(pdvector<instPoint *> &ret) {
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        if (entryPoints_[i]->hasAnyInstrumentation()) {
            ret.push_back(entryPoints_[i]);
        }
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        if (exitPoints_[i]->hasAnyInstrumentation()) {
            ret.push_back(exitPoints_[i]);
        }
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        if (callPoints_[i]->hasAnyInstrumentation()) {
            ret.push_back(callPoints_[i]);
        }
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        if (arbitraryPoints_[i]->hasAnyInstrumentation()) {
            ret.push_back(arbitraryPoints_[i]);
        }
    }
}

void int_function::generateInstrumentation(pdvector<instPoint *> &input,
                                           pdvector<instPoint *> &failed,
                                           bool &relocationRequired) {
    for (unsigned i = 0; i < input.size(); i++) {
        switch (input[i]->generateInst()) {
        case instPoint::tryRelocation:
            relocationRequired = true;
            break;
        case instPoint::generateSucceeded:
            break;
        case instPoint::generateFailed:
            failed.push_back(input[i]);
            break;
        default:
            assert(0);
            break;
        }
    }
}

void int_function::installInstrumentation(pdvector<instPoint *> &input,
                                          pdvector<instPoint *> &failed) {
    for (unsigned i = 0; i < input.size(); i++) {
        switch (input[i]->installInst()) {
        case instPoint::wasntGenerated:
            break;
        case instPoint::installSucceeded:
            break;
        case instPoint::installFailed:
            failed.push_back(input[i]);
            break;
        default:
            assert(0);
            break;
        }
    }
}


void int_function::linkInstrumentation(pdvector<instPoint *> &input,
                                          pdvector<instPoint *> &failed) {
    for (unsigned i = 0; i < input.size(); i++) {
        switch (input[i]->linkInst()) {
        case instPoint::wasntInstalled:
            break;
        case instPoint::linkSucceeded:
            break;
        case instPoint::linkFailed:
            failed.push_back(input[i]);
            break;
        default:
            assert(0);
            break;
        }
    }
}


Offset int_function::addrToOffset(const Address addr) const { 
    return addr - getAddress() + ifunc_->getOffset(); 
}

const pdvector< int_parRegion* > &int_function::parRegions()
{
  if (parallelRegions_.size() > 0)
    return parallelRegions_;

  for (unsigned int i = 0; i < ifunc_->parRegions().size(); i++)
    {
      image_parRegion * imPR = ifunc_->parRegions()[i];
      //int_parRegion * iPR = new int_parRegion(imPR, baseAddr, this); 
      int_parRegion * iPR = new int_parRegion(imPR, addr_, this); 
      parallelRegions_.push_back(iPR);
    }
  return parallelRegions_;
}
