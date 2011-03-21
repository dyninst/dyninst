
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

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include "parseAPI/src/InstrucIter.h"
#include "MemoryEmulator/memEmulator.h"
#include "Relocation/Transformers/Movement-analysis.h"

//std::string int_function::emptyString("");

#include "Parsing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::Relocation;

int int_function_count = 0;

// 
int_function::int_function(image_func *f,
			   Address baseAddr,
			   mapped_module *mod) :
   addr_(baseAddr + f->getOffset()),
   ptrAddr_(f->getPtrOffset() ? f->getPtrOffset() + baseAddr : 0),
   ifunc_(f),
   mod_(mod),
   entry_(NULL),
   entryPoint_(NULL),
   handlerFaultAddr_(0),
   handlerFaultAddrAddr_(0), 
   isBeingInstrumented_(false)
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

    parsing_printf("%s: creating new proc-specific function at 0x%lx\n",
                   symTabName().c_str(), addr_);

    
}

int_function::int_function(const int_function *parFunc,
                           mapped_module *childMod,
                           process *) :
    addr_(parFunc->addr_),
    ptrAddr_(parFunc->ptrAddr_),
    ifunc_(parFunc->ifunc_),
    mod_(childMod),
    handlerFaultAddr_(0),
    handlerFaultAddrAddr_(0), 
    isBeingInstrumented_(parFunc->isBeingInstrumented_)
 {
     assert(0 && "Unimplemented!");

     // Construct the raw blocks_;
     for (BlockSet::const_iterator bIter = parFunc->blocks_.begin();
          bIter != parFunc->blocks_.end(); bIter++) 
     {
         createBlockFork((*bIter));
     }
}

int_function::~int_function() { 
    // int_blocks
    BlockSet::iterator bIter = blocks_.begin();
    for (; bIter != blocks_.end(); bIter++) {
        delete *bIter;
    }

    for (unsigned i = 0; i < parallelRegions_.size(); i++)
      delete parallelRegions_[i];
      
}

int_block *int_function::createBlock(image_basicBlock *ib) {
    int_block *block = new int_block(ib, this);
    blocks_.insert(block);
    blockMap_[ib] = block;

       
    return block;
}

int_block *int_function::createBlockFork(const int_block *parent)
{
    int_block *block = new int_block(parent, this);
    blocks_.insert(block);
    blockMap_[parent->llb()] = block;
    return block;
}

Address int_function::baseAddr() const {
    return obj()->codeBase();
}


// This needs to go away: how is "size" defined? Used bytes? End-start?

unsigned int_function::getSize_NP()  {
    blocks();
    if (blocks_.size() == 0) return 0;
            
    return ((*blocks_.rbegin())->end() - 
            (*blocks_.begin())->start());
}

// finds new entry point, sets the argument to the new 
int_block * int_function::setNewEntryPoint()
{
    int_block *newEntry = NULL;

    // find block with no intraprocedural entry edges
    assert(blocks_.size());
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++)  {
       SingleContext epred(ifunc(),true,true);
       Block::edgelist & ib_ins = (*bIter)->llb()->sources();
       Block::edgelist::iterator eit = ib_ins.begin(&epred);
       if (eit == ib_ins.end()) {
          if (NULL != newEntry) {
             fprintf(stderr,"ERROR: multiple blocks in function %lx "
                     "have no incoming edges: [%lx %lx) and [%lx %lx)\n",
                     getAddress(), newEntry->llb()->start(),
                     newEntry->llb()->start() + newEntry->llb()->end(),
                     (*bIter)->llb()->start(),
                     (*bIter)->llb()->start() + (*bIter)->llb()->end());
          } else {
             newEntry = *bIter;
          }
       }
    }
    
    if (newEntry) {
       entry_ = newEntry;
       ifunc()->setEntryBlock(newEntry->llb());
       addr_ = entry_->start();
       return newEntry;
    }
    else {
       return entry_;
    }
}

void int_function::setHandlerFaultAddr(Address fa) 
{ 
    handlerFaultAddr_ = fa;
}

// Sets the address in the structure at which the fault instruction's
// address is stored if "set" is true.  Accesses the fault address and 
// translates it back to an original address if it corresponds to 
// relocated code in the Dyninst heap 
void int_function::setHandlerFaultAddrAddr(Address faa, bool set) 
{ 
    if (set) {
        // save the faultAddrAddr
        handlerFaultAddrAddr_ = faa; 
    }

    // get the faultAddr 
    assert(proc()->proc());
    assert(sizeof(Address) == proc()->getAddressWidth());
    Address faultAddr=0;
    if (!proc()->readDataSpace
        ((void*)faa, proc()->getAddressWidth(), (void*)&faultAddr, true)) 
    {
        assert(0);
    }

    // translate the faultAddr back to an original address, and if
    // that translation was necessary, save it to the faultAddrAddr in the 
    // CONTEXT struct
    if (proc()->proc()->isRuntimeHeapAddr(faultAddr)) {

        Address origAddr = faultAddr;
        vector<int_function *> tmps;
        baseTramp *bti = NULL;
        bool success = proc()->getAddrInfo(faultAddr, origAddr, tmps, bti);
        assert(success);
        assert( proc()->writeDataSpace((void*)faa, 
                                       sizeof(Address), 
                                       (void*)&origAddr) );
    }
}

// Set the handler return addr to the most recent instrumented or
// relocated address, similar to instPoint::instrSideEffect.
// Also, make sure that we update our mapped view of memory, 
// we may have overwritten memory that was previously not code
void int_function::fixHandlerReturnAddr(Address /*faultAddr*/)
{
    if ( !proc()->proc() || ! handlerFaultAddrAddr_ ) {
        assert(0);
        return;
    }
#if 0 //KEVINTODO: this function doesn't work, I tried setting newPC to 0xdeadbeef and it had no impact on the program's behavior.  If the springboards work properly this code is unneeded
    // Do a straightfoward forward map of faultAddr
    // First, get the original address
    int_function *func;
    int_block *block; baseTrampInstance *ignored;
    Address origAddr;
    if (!proc()->getRelocInfo(faultAddr, origAddr, block, ignored)) {
       func = dynamic_cast<process *>(proc())->findActiveFuncByAddr(faultAddr);
       origAddr = faultAddr;
    }
    else {
       func = block->func();
    }
    std::list<Address> relocAddrs;
    proc()->getRelocAddrs(origAddr, func, relocAddrs, true);
    Address newPC = (!relocAddrs.empty() ? relocAddrs.back() : origAddr);
    
    if (newPC != faultAddr) {
       if(!proc()->writeDataSpace((void*)handlerFaultAddrAddr_, 
                                  sizeof(Address), 
                                  (void*)&newPC)) {
          assert(0);
       }
    }
#endif
}

void int_function::findPoints(int_block *block,
                              std::set<instPoint *> &foundPoints) const {
   // What's better - iterate over all our point contaners 
   // looking for options, or run through instPsByAddr_...

   for (Address a = block->start(); a < block->end(); ++a) {
      std::map<Address, instPoint *>::const_iterator p_iter = instPsByAddr_.find(a);
      if ((p_iter != instPsByAddr_.end()) &&
          (p_iter->second->block() == block)) {
         foundPoints.insert(p_iter->second);
      }
   }
}

// doesn't delete the ParseAPI::Block's, those are removed in a batch
// call to the parseAPI
void int_function::deleteBlock(int_block* block) 
{
   assert(0 && "TODO");
   
   // init stuff
   assert(block && this == block->func());
   

    blocks_.erase(block);
    blockMap_.erase(block->llb());

    // It appears that we delete the int_block before the image_basicBlock...
    //assert(consistency());
    triggerModified();
}

void int_function::splitBlock(image_basicBlock *img_orig, 
                              image_basicBlock *img_new) {
    blocks();
   int_block *origBlock = blockMap_[img_orig];
   assert(origBlock);
   
   int_block *newBlock = blockMap_[img_new];
   if (!newBlock) {
       newBlock = createBlock(img_new);
   }

   // Shift block-level points over...
   InstPointMap::iterator iter = exitPoints_.find(origBlock);
   if (iter != exitPoints_.end()) {
      // Move it to the new block, by definition
      exitPoints_.erase(iter);
      exitPoints_[newBlock] = iter->second;
      iter->second->block_ = newBlock;
   }

   iter = preCallPoints_.find(origBlock);
   if (iter != preCallPoints_.end()) {
      // Move it to the new block, by definition
      preCallPoints_.erase(iter);
      preCallPoints_[newBlock] = iter->second;
      iter->second->block_ = newBlock;
   }
   
   iter = postCallPoints_.find(origBlock);
   if (iter != postCallPoints_.end()) {
      // Move it to the new block, by definition
      postCallPoints_.erase(iter);
      postCallPoints_[newBlock] = iter->second;
      iter->second->block_ = newBlock;
   }

   iter = blockEntryPoints_.find(origBlock);
   if (iter != blockEntryPoints_.end()) {
      // Move it to the new block, by definition
      blockEntryPoints_.erase(iter);
      blockEntryPoints_[newBlock] = iter->second;
      iter->second->block_ = newBlock;
   }
   
   ArbitraryMap::iterator iter2 = preInsnPoints_.find(origBlock);
   if (iter2 != preInsnPoints_.end()) {
      for (ArbitraryMapInt::iterator iter3 = iter2->second.lower_bound(newBlock->start());
           iter3 != iter2->second.end(); ++iter3) {
         preInsnPoints_[newBlock][iter3->first] = iter3->second;
         iter3->second->block_ = newBlock;
      }
      iter2->second.erase(iter2->second.lower_bound(newBlock->start()),
                          iter2->second.end());
   }

   iter2 = postInsnPoints_.find(origBlock);
   if (iter2 != postInsnPoints_.end()) {
      for (ArbitraryMapInt::iterator iter3 = iter2->second.lower_bound(newBlock->start());
           iter3 != iter2->second.end(); ++iter3) {
         postInsnPoints_[newBlock][iter3->first] = iter3->second;
         iter3->second->block_ = newBlock;
      }
      iter2->second.erase(iter2->second.lower_bound(newBlock->start()),
                          iter2->second.end());
   }


   triggerModified();
}

// Remove funcs from:
//   mapped_object & mapped_module datastructures
//   addressSpace::textRanges codeRangeTree<int_function*> 
//   image-level & SymtabAPI datastructures
//   BPatch_addressSpace::BPatch_funcMap <int_function -> BPatch_function>
void int_function::removeFromAll() 
{
    mal_printf("purging blocks_ of size = %d from func at %lx\n",
               blocks_.size(), getAddress());

    BlockSet::const_iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) 
    {
        int_block *bbi = (*bIter);
        mal_printf("purged block [%lx %lx]\n",bbi->start(), bbi->end());
    }
    // delete blocks 
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end();
         bIter = blocks_.begin()) 
    {
        deleteBlock(*bIter);// removes block from blocks_ too
    }
    // remove from mapped_object & mapped_module datastructures
    obj()->removeFunction(this);
    mod()->removeFunction(this);

    // remove func & blocks from image, ParseAPI, & SymtabAPI datastructures
    ifunc()->img()->deleteFunc(ifunc());

    // invalidates analyses related to this function
	triggerModified();

    delete(this);
}

void int_function::addMissingBlock(image_basicBlock *missingB)
{
   int_block *bbi = findBlock(missingB);
   if (bbi) {
      assert(bbi->llb() == missingB);
      return;
    }
   
    cerr << "Function " << hex << this << " @ " << getAddress() << " adding block " << missingB->start() << " (" << missingB << ")" << dec << endl;
   createBlock(missingB);
}


/* Find image_basicBlocks that are missing from these datastructures and add
 * them.  The int_block constructor does pretty much all of the work in
 * a chain of side-effects extending all the way into the mapped_object class
 * 
 * We have to take into account that additional parsing may cause basic block splitting,
 * in which case it is necessary not only to add new int-level blocks, but to update 
 * int_block and BPatch_basicBlock objects. 
 */
void int_function::addMissingBlocks()
{
    // A bit of a hack, but be sure that we've re-checked the blocks in the
    // image_func as well.
    ifunc_->invalidateCache();

   blocks();
   cerr << "addMissingBlocks for function " << hex << this << dec << endl;
   // Add new blocks

   const vector<image_basicBlock*> & nblocks = obj()->parse_img()->getNewBlocks();
   // add blocks by looking up new blocks, if it promises to be more 
   // efficient than looking through all of the llfunc's blocks
   vector<image_basicBlock*>::const_iterator nit = nblocks.begin();
   for( ; nit != nblocks.end(); ++nit) {
       if (ifunc()->contains(*nit)) {
           addMissingBlock(*nit);
       }
   }

   if (ifunc()->blocks().size() > blocks_.size()) { //not just the else case!
       // we may have parsed into an existing function and added its blocks 
       // to ours, or this may just be a more efficient lookup method
       Function::blocklist & iblks = ifunc()->blocks();
       for (Function::blocklist::iterator bit = iblks.begin(); 
            bit != iblks.end(); 
            bit++) 
       {
           if (!findBlock(*bit)) {
               addMissingBlock(static_cast<image_basicBlock*>(*bit));
           }
       }
   }
}

void int_function::getReachableBlocks(const set<int_block*> &exceptBlocks,
                                      const list<int_block*> &seedBlocks,
                                      set<int_block*> &reachBlocks)//output
{
    list<image_basicBlock*> imgSeeds;
    for (list<int_block*>::const_iterator sit = seedBlocks.begin();
         sit != seedBlocks.end(); 
         sit++) 
    {
        imgSeeds.push_back((*sit)->llb());
    }
    set<image_basicBlock*> imgExcept;
    for (set<int_block*>::const_iterator eit = exceptBlocks.begin();
         eit != exceptBlocks.end(); 
         eit++) 
    {
        imgExcept.insert((*eit)->llb());
    }

    // image-level function does the work
    set<image_basicBlock*> imgReach;
    ifunc()->getReachableBlocks(imgExcept,imgSeeds,imgReach);

    for (set<image_basicBlock*>::iterator rit = imgReach.begin();
         rit != imgReach.end(); 
         rit++) 
    {
        reachBlocks.insert( findBlock(*rit) );
    }
}

bool int_function::findBlocksByAddr(Address addr, 
                                    std::set<int_block *> &blocks) 
{
   this->blocks();
   bool ret = false;    
   Address offset = addr-baseAddr();
   std::set<ParseAPI::Block *> iblocks;
   if (!ifunc()->img()->findBlocksByAddr(offset, iblocks)) {
       return false;
   }
   for (std::set<ParseAPI::Block *>::iterator iter = iblocks.begin(); 
        iter != iblocks.end(); ++iter) {
      int_block *bbl = findBlock(*iter);
      if (bbl) {
         ret = true; 
         blocks.insert(bbl);
      }
      // It's okay for this to fail if we're in the middle of a CFG
      // modification. 

   }
   return ret;
}

int_block *int_function::findOneBlockByAddr(Address addr) {
    std::set<int_block *> blocks;
    if (!findBlocksByAddr(addr, blocks)) {
        return NULL;
    }
    for (std::set<int_block *>::iterator iter = blocks.begin();
        iter != blocks.end(); ++iter) {
        // Walk this puppy and see if it matches our address.
        int_block::InsnInstances insns;
        (*iter)->getInsns(insns);
        for (int_block::InsnInstances::iterator i_iter = insns.begin(); i_iter != insns.end(); ++i_iter) {
            if (i_iter->second == addr) return *iter;
        }
    }
    return NULL;
}

int_block *int_function::findBlockByEntry(Address addr) {
    std::set<int_block *> blocks;
    if (!findBlocksByAddr(addr, blocks)) {
        return NULL;
    }
    for (std::set<int_block *>::iterator iter = blocks.begin();
        iter != blocks.end(); ++iter) {
        int_block *cand = *iter;
        if (cand->start() == addr) return cand;
    }
    return NULL;
}

int_block *int_function::findBlock(ParseAPI::Block *block) {
   blocks();
    image_basicBlock *iblock = static_cast<image_basicBlock *>(block);
    BlockMap::iterator iter = blockMap_.find(iblock);
    if (iter != blockMap_.end()) return iter->second;
    return NULL;
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

const int_function::BlockSet &int_function::blocks()
{
    if (blocks_.empty()) {
        // defensiveMode triggers premature block list creation when it
        // checks that the targets of control transfers have not been
        // tampered with.  
        Function::blocklist & img_blocks = ifunc_->blocks();
        Function::blocklist::iterator sit = img_blocks.begin();

        for( ; sit != img_blocks.end(); ++sit) {
            image_basicBlock *b = (image_basicBlock*)(*sit);
            createBlock(b);
        }
    }
    return blocks_;
}

const int_function::BlockSet &int_function::callBlocks() {
   // Check the list...
   if (callBlocks_.empty()) {
      const ParseAPI::Function::edgelist &callEdges = ifunc_->callEdges();
      for (ParseAPI::Function::edgelist::iterator iter = callEdges.begin();
           iter != callEdges.end(); ++iter) {
         ParseAPI::Block *src = (*iter)->src();
         int_block *block = findBlock(src);
         assert(block);
         callBlocks_.insert(block);
      }
   }
   return callBlocks_;
}

const int_function::BlockSet &int_function::exitBlocks() {
   // Check the list...
   if (exitBlocks_.empty()) {
      const ParseAPI::Function::blocklist &exitBlocks = ifunc_->returnBlocks();
      if (exitBlocks.empty()) {
         cerr << "Odd: function " << symTabName() << " reports no exit blocks @ ParseAPI level" << endl;
      }
      for (ParseAPI::Function::blocklist::iterator iter = exitBlocks.begin();
           iter != exitBlocks.end(); ++iter) {
         ParseAPI::Block *iblock = (*iter);
         int_block *block = findBlock(iblock);
         assert(block);
         exitBlocks_.insert(block);
      }
   }
   if (exitBlocks_.empty()) {
      cerr << "Odd: exitBlocks empty for " << symTabName() << endl;
      assert (ifunc_->returnBlocks().empty());
   }

   return exitBlocks_;
}

const int_function::BlockSet &int_function::unresolvedCF() {
   if (unresolvedCF_.empty()) {
      // A block has unresolved control flow if it has an indirect
      // out-edge. 
      blocks();
      for (BlockSet::iterator iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
         if ((*iter)->llb()->unresolvedCF()) {
            unresolvedCF_.insert(*iter);
         }
      }
   }
   return unresolvedCF_;
}

const int_function::BlockSet &int_function::abruptEnds() {
   if (abruptEnds_.empty()) {
      // A block has unresolved control flow if it has an indirect
      // out-edge. 
      blocks();
      for (BlockSet::iterator iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
         if ((*iter)->llb()->abruptEnd()) {
            abruptEnds_.insert(*iter);
         }
      }
   }
   return abruptEnds_;
}

int_block *int_function::entryBlock() {
   if (!entry_) {
      if (!ifunc_->parsed()) {
         ifunc_->blocks();
      }
      ParseAPI::Block *iEntry = ifunc_->entry();
      if (!iEntry) return NULL;
      entry_ = findBlock(iEntry);
   }
   return entry_;
}

unsigned int_function::getNumDynamicCalls()
{
   unsigned count=0;
   for (BlockSet::iterator iter = callBlocks().begin(); iter != callBlocks().end(); ++iter) {
      if ((*iter)->containsDynamicCall()) {
         count++;
      }
   }

   return count;
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
    for (BlockSet::const_iterator 
             cb = blocks_.begin();
         cb != blocks_.end(); 
         cb++) 
    {
        int_block* orig = (*cb);
        fprintf(stderr, "  Block start 0x%lx, end 0x%lx\n", orig->start(),
                orig->end());
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

image_func *int_function::ifunc() {
    return ifunc_;
}

// Dig down to the low-level block of b, find the low-level functions
// that share it, and map up to int-level functions and add them
// to the funcs list.
bool int_function::getSharingFuncs(int_block *b,
                                   std::set<int_function *> & funcs)
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

        if (funcs.find(hl_func) == funcs.end()) ret = true;
        funcs.insert(hl_func);
    }

    return ret;
}

// Find sharing functions via checking all basic blocks. We might be
// able to check only exit points; but we definitely need to check _all_
// exits so for now we're checking everything.

bool int_function::getSharingFuncs(std::set<int_function *> &funcs) {
    bool ret = false;

    // Create the block list.
    blocks();
    
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) {
       if (getSharingFuncs(*bIter,funcs))
          ret = true;
    }

    return ret;
}

// Find functions that have overlapping, but disjoint, blocks in 
// addition to shared blocks. Very slow!

bool int_function::getOverlappingFuncs(int_block *block,
                                       std::set<int_function *> &funcs) 
{
	ParseAPI::Block *llB = block->llb();
	std::set<ParseAPI::Block *> overlappingBlocks;
	for (Address i = llB->start(); i < llB->end(); ++i) {
		llB->obj()->findBlocks(llB->region(), i, overlappingBlocks);
	}
	// We now have all of the overlapping ParseAPI blocks. Get the set of 
	// ParseAPI::Functions containing each and up-map to int_functions
	for (std::set<ParseAPI::Block *>::iterator iter = overlappingBlocks.begin();
		iter != overlappingBlocks.end(); ++iter) {
		std::vector<ParseAPI::Function *> llFuncs;
		(*iter)->getFuncs(llFuncs);
		for (std::vector<ParseAPI::Function *>::iterator iter2 = llFuncs.begin();
			iter2 != llFuncs.end(); ++iter2) 
        {
            funcs.insert(obj()->findFunction(*iter2));
		}
	}
	return (funcs.size() > 1);
}

bool int_function::getOverlappingFuncs(std::set<int_function *> &funcs) 
{
    bool ret = false;

    // Create the block list.
    blocks();
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) {
       if (getOverlappingFuncs(*bIter,funcs))
          ret = true;
    }

    return ret;
}

Address int_function::get_address() const 
{
   return getAddress();
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

Offset int_function::addrToOffset(const Address addr) const { 
  return addr - (getAddress() - ifunc_->getOffset());
}

Address int_function::offsetToAddr(const Offset off ) const { 
  return off + (getAddress() - ifunc_->getOffset());
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

bool int_function::validPoint(instPoint *p) const {
   // TODO
   return true;
}

bool int_function::consistency() const {
   // 1) Check for 1:1 block relationship in
   //    the block list and block map
   // 2) Check that all instPoints are in the
   //    correct block.

   const ParseAPI::Function::blocklist &img_blocks = ifunc_->blocks();
   assert(img_blocks.size() == blocks_.size());
   assert(blockMap_.size() == blocks_.size());
   for (ParseAPI::Function::blocklist::iterator iter = img_blocks.begin();
        iter != img_blocks.end(); ++iter) {
      image_basicBlock *img_block = static_cast<image_basicBlock *>(*iter);
      BlockMap::const_iterator m_iter = blockMap_.find(img_block);
      assert(m_iter != blockMap_.end());
      assert(blocks_.find(m_iter->second) != blocks_.end());
   }

   return true;
}

void int_function::triggerModified() {
	// A single location to drop anything that needs to be
	// informed when an int_function was updated.

    // invalidate liveness calculations
    ifunc()->invalidateLiveness();
}

int_edge *int_function::getEdge(ParseAPI::Edge *iedge, 
                                int_block *src, 
                                int_block *trg) {
   EdgeMap::iterator iter = edgeMap_.find(iedge);
   if (iter != edgeMap_.end()) {
      // Consistency check
      if (src) assert(iter->second->src() == src);
      if (trg) assert(iter->second->trg() == trg);
      return iter->second;
   }
   
   int_edge *newEdge = new int_edge(iedge, src, trg);
   edgeMap_.insert(std::make_pair(iedge, newEdge));
   return newEdge;
}

void int_function::removeEdge(int_edge *e) {
   edgeMap_.erase(e->edge());
}

void int_function::getCallers(std::vector<int_block *> &callers) {
   // Follow the edges...
   for (int_block::edgelist::iterator iter = entryBlock()->sources().begin();
        iter != entryBlock()->sources().end(); ++iter) {
      assert((*iter)->src());
      callers.push_back((*iter)->src());
   }
}


// Instpoints!
instPoint *int_function::entryPoint() {
   if (!entryPoint_) {
      entryPoint_ = instPoint::funcEntry(this);
   }
   return entryPoint_;
}

instPoint *int_function::exitPoint(int_block *block) {
   std::map<int_block *, instPoint *>::iterator iter = exitPoints_.find(block);
   if (iter != exitPoints_.end()) return iter->second;
   if (block->isExit()) {
      instPoint *iP = instPoint::funcExit(block);
      exitPoints_[block] = iP;
      return iP;
   }
   return NULL;
}

instPoint *int_function::findPoint(instPoint::Type type) {
   if (type == instPoint::FunctionEntry) {
      return entryPoint_;
   }
   return NULL;
}

instPoint *int_function::findPoint(instPoint::Type type, int_block *block) {
   switch (type) {
      case instPoint::FunctionEntry: {
         if (block != entry_) return NULL;
         return entryPoint_;
      }
      case instPoint::FunctionExit: {
         InstPointMap::iterator iter = exitPoints_.find(block);
         if (iter != exitPoints_.end()) return iter->second;
         return NULL;
      }
      case instPoint::PreCall: {
         InstPointMap::iterator iter = preCallPoints_.find(block);
         if (iter != preCallPoints_.end()) return iter->second;
         return NULL;
      }
      case instPoint::PostCall: {
         InstPointMap::iterator iter = postCallPoints_.find(block);
         if (iter != postCallPoints_.end()) return iter->second;
         return NULL;
      }
      case instPoint::BlockEntry: {
         InstPointMap::iterator iter = blockEntryPoints_.find(block);
         if (iter != blockEntryPoints_.end()) return iter->second;
         return NULL;
      }
      default:
         return NULL;
   }
}

const std::map<Address, instPoint *> &int_function::findPoints(instPoint::Type type, int_block *block) {
   switch (type) {
      case instPoint::PreInsn:
         return preInsnPoints_[block];
      case instPoint::PostInsn:
         return postInsnPoints_[block];
      default:
         assert(0);
         return preInsnPoints_[NULL];
   }
}
