/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
#include "MemoryEmulator/memEmulator.h"
#include "Relocation/Transformers/Movement-analysis.h"


#include "Parsing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::Relocation;

int func_instance_count = 0;

func_instance::func_instance(parse_func *f,
                             Address baseAddr,
                             mapped_module *mod) :
  PatchFunction(f, mod->obj()),
  ptrAddr_(f->getPtrOffset() ? f->getPtrOffset() + baseAddr : 0),
  mod_(mod),
  entry_(NULL),
  handlerFaultAddr_(0),
  handlerFaultAddrAddr_(0)
#if defined(os_windows)
  , callingConv(unknown_call)
  , paramSize(0)
#endif
   , wrapperSym_(NULL)
{
  assert(f);
#if defined(ROUGH_MEMORY_PROFILE)
  func_instance_count++;
  if ((func_instance_count % 1000) == 0)
    fprintf(stderr, "func_instance_count: %d (%d)\n",
            func_instance_count, func_instance_count*sizeof(func_instance));
#endif

    parsing_printf("%s: creating new proc-specific function at 0x%lx\n",
                   symTabName().c_str(), addr_);
}

unsigned func_instance::footprint() {
    unsigned totalSize = 0;
    vector<FuncExtent*> exts = ifunc()->extents();
    for (unsigned i=0; i < exts.size(); i++) {
        totalSize += (exts[i]->end() - exts[i]->start());
    }
    return totalSize;
}

//KEVINTODO: add parse-level to int-level callbacks to update instpoint lists 
// of callBlock, exitBlock, entryBlock, etc.  

func_instance::func_instance(const func_instance *parFunc,
                             mapped_module *childMod) :
  PatchFunction(parFunc->ifunc(), childMod->obj()),
  ptrAddr_(parFunc->ptrAddr_),
  mod_(childMod),
  entry_(NULL),
  handlerFaultAddr_(0),
  handlerFaultAddrAddr_(0)
#if defined(os_windows)
  , callingConv(parFunc->callingConv)
  , paramSize(parFunc->paramSize)
#endif
   , wrapperSym_(NULL)
{
   assert(ifunc());
   // According to contract /w/ the mapped_object
   // all blocks have already been constructed.
   // Do we duplicate the parent or wait? I'm
   // tempted to wait, just because of the common
   // fork/exec case.
}

func_instance::~func_instance() { 
   // We don't delete blocks, since they're shared between functions
   // We _do_ delete context instPoints, though
   // Except that should get taken care of normally since the
   // structures are static. 
    for (unsigned i = 0; i < parallelRegions_.size(); i++)
      delete parallelRegions_[i];
}

// finds new entry point, sets entry block to the new entry
block_instance * func_instance::setNewEntryPoint()
{
    block_instance *newEntry = NULL;

    // find block with no intraprocedural entry edges
    assert(blocks_.size());
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) 
    {
        ParseAPI::Intraproc epred;
        Block::edgelist & ib_ins = (*bIter)->llb()->sources();
        Block::edgelist::iterator eit = ib_ins.begin(&epred);
        if (eit == ib_ins.end()) {
            if (NULL != newEntry) {
                fprintf(stderr,"ERROR: multiple blocks in function %lx "
                    "have no incoming edges: [%lx %lx) and [%lx %lx)\n",
                    addr_, newEntry->llb()->start(),
                    newEntry->llb()->start() + newEntry->llb()->end(),
                    (*bIter)->llb()->start(),
                    (*bIter)->llb()->start() + (*bIter)->llb()->end());
            } else {
                newEntry = *bIter;
            }
        }
    }
    if( ! newEntry ) {
        newEntry = *blocks_.begin();
    }
    ifunc()->setEntryBlock(newEntry->llb());
    this->addr_ = newEntry->start();

    assert(!newEntry->llb()->isShared()); //KEVINTODO: unimplemented case

    return newEntry;
}

void func_instance::setHandlerFaultAddr(Address fa) {
    handlerFaultAddr_ = fa;
}

// Sets the address in the structure at which the fault instruction's
// address is stored if "set" is true.  Accesses the fault address and
// translates it back to an original address if it corresponds to
// relocated code in the Dyninst heap
void func_instance::setHandlerFaultAddrAddr(Address faa, bool set) {
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
    vector<func_instance *> tmps;
    baseTramp *bti = NULL;
    bool success = proc()->getAddrInfo(faultAddr, origAddr, tmps, bti);
    assert(success);
    assert( proc()->writeDataSpace((void*)faa,
                                   sizeof(Address),
                                   (void*)&origAddr) );
  }
}

// Remove funcs from:
//   mapped_object & mapped_module datastructures
//   addressSpace::textRanges codeRangeTree<func_instance*>
//   image-level & SymtabAPI datastructures
//   BPatch_addressSpace::BPatch_funcMap <func_instance -> BPatch_function>
void func_instance::removeFromAll()
{
    mal_printf("purging blocks_ of size = %d from func at %lx\n",
               blocks_.size(), addr());

    // remove from mapped_object & mapped_module datastructures
    obj()->removeFunction(this);
    mod()->removeFunction(this);

    delete(this);
}

/* Find parse_blocks that are missing from these datastructures and add
 * them.  The block_instance constructor does pretty much all of the work in
 * a chain of side-effects extending all the way into the mapped_object class
 *
 * We have to take into account that additional parsing may cause basic block splitting,
 * in which case it is necessary not only to add new int-level blocks, but to update
 * block_instance and BPatch_basicBlock objects.
 */
void func_instance::addMissingBlocks()
{
   cerr << "addMissingBlocks for function " << hex << this << dec << endl;

   assert(0 && "KEVINTODO: need to clear block set, or add missing blocks, but getNewBlocks would have to be a func->block map"); 

   // A bit of a hack, but be sure that we've re-checked the blocks in the
   // parse_func as well.
   ifunc_->invalidateCache();

   // Add new blocks
   //const vector<parse_block*> & nblocks = obj()->parse_img()->getNewBlocks();
}

void func_instance::getReachableBlocks(const set<block_instance*> &exceptBlocks,
                                      const list<block_instance*> &seedBlocks,
                                      set<block_instance*> &reachBlocks)//output
{
    list<parse_block*> imgSeeds;
    for (list<block_instance*>::const_iterator sit = seedBlocks.begin();
         sit != seedBlocks.end();
         sit++)
    {
        imgSeeds.push_back((*sit)->llb());
    }
    set<parse_block*> imgExcept;
    for (set<block_instance*>::const_iterator eit = exceptBlocks.begin();
         eit != exceptBlocks.end();
         eit++)
    {
        imgExcept.insert((*eit)->llb());
    }

    // image-level function does the work
    set<parse_block*> imgReach;
    ifunc()->getReachableBlocks(imgExcept,imgSeeds,imgReach);

    for (set<parse_block*>::iterator rit = imgReach.begin();
         rit != imgReach.end();
         rit++)
    {
        reachBlocks.insert( obj()->findBlock(*rit) );
    }
}

void print_func_vector_by_pretty_name(std::string prefix,
                                      pdvector<func_instance *>*funcs) {
    unsigned int i;
    func_instance *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

mapped_object *func_instance::obj() const { return mod()->obj(); }
AddressSpace *func_instance::proc() const { return obj()->proc(); }

const func_instance::BlockSet &func_instance::blocks() {
  if (blocks_.empty()) {
    for (PatchFunction::blocklist::const_iterator i = getAllBlocks().begin();
         i != getAllBlocks().end(); i++) {
      blocks_.insert(SCAST_BI(*i));
    }
  }
  return blocks_;
}

const func_instance::BlockSet &func_instance::callBlocks() {
  // Check the list...
  if (callBlocks_.empty()) {
    for (PatchFunction::blocklist::const_iterator i = getCallBlocks().begin();
         i != getCallBlocks().end(); i++) {
      callBlocks_.insert(SCAST_BI(*i));
    }
  }
  return callBlocks_;
}

const func_instance::BlockSet &func_instance::exitBlocks() {
  // Check the list...
  if (exitBlocks_.empty()) {
    for (PatchFunction::blocklist::const_iterator i = getExitBlocks().begin();
         i != getExitBlocks().end(); i++) {
      exitBlocks_.insert(SCAST_BI(*i));
    }
  }
  return exitBlocks_;
}

const func_instance::BlockSet &func_instance::unresolvedCF() {
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

const func_instance::BlockSet &func_instance::abruptEnds() {
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

block_instance *func_instance::entryBlock() {
  if (!entry_) {
    entry_ = SCAST_BI(getEntryBlock());
    if (!entry_) {
      cerr << "ERROR: Couldn't find entry block for " << name() << endl;
    }
  }
  return entry_;
}

unsigned func_instance::getNumDynamicCalls()
{
   unsigned count=0;
   for (BlockSet::const_iterator iter = callBlocks().begin(); iter != callBlocks().end(); ++iter) {
      if ((*iter)->containsDynamicCall()) {
         count++;
      }
   }

   return count;
}


void func_instance::debugPrint() const {
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
    fprintf(stderr, "  Address: 0x%lx\n", addr());
    fprintf(stderr, "  Internal pointer: %p\n", ifunc());
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
        block_instance* orig = (*cb);
        fprintf(stderr, "  Block start 0x%lx, end 0x%lx\n", orig->start(),
                orig->end());
    }
}

// Add to internal
// Add to mapped_object if a "new" name (true return from internal)
void func_instance::addSymTabName(const std::string name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
        obj()->addFunctionName(this, name, mapped_object::mangledName);
}

void func_instance::addPrettyName(const std::string name, bool isPrimary) {
    if (ifunc()->addPrettyName(name, isPrimary))
        obj()->addFunctionName(this, name, mapped_object::prettyName);
}

// Dig down to the low-level block of b, find the low-level functions
// that share it, and map up to int-level functions and add them
// to the funcs list.
bool func_instance::getSharingFuncs(block_instance *b,
                                   std::set<func_instance *> & funcs)
{
    bool ret = false;

    vector<Function *> lfuncs;
    b->llb()->getFuncs(lfuncs);
    vector<Function *>::iterator fit = lfuncs.begin();
    for( ; fit != lfuncs.end(); ++fit) {
      parse_func *ll_func = SCAST_PF(*fit);
        func_instance *hl_func = obj()->findFunction(ll_func);
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

bool func_instance::getSharingFuncs(std::set<func_instance *> &funcs) {
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

bool func_instance::getOverlappingFuncs(block_instance *block,
                                       std::set<func_instance *> &funcs)
{
        ParseAPI::Block *llB = block->llb();
        std::set<ParseAPI::Block *> overlappingBlocks;
        for (Address i = llB->start(); i < llB->end(); ++i) {
                llB->obj()->findBlocks(llB->region(), i, overlappingBlocks);
        }
        // We now have all of the overlapping ParseAPI blocks. Get the set of
        // ParseAPI::Functions containing each and up-map to func_instances
        for (std::set<ParseAPI::Block *>::iterator iter = overlappingBlocks.begin();
                iter != overlappingBlocks.end(); ++iter) {
                std::vector<ParseAPI::Function *> llFuncs;
                (*iter)->getFuncs(llFuncs);
                for (std::vector<ParseAPI::Function *>::iterator iter2 = llFuncs.begin();
                     iter2 != llFuncs.end(); ++iter2)  {
                   funcs.insert(obj()->findFunction(*iter2));
                }
        }
        return (funcs.size() > 1);
}

bool func_instance::getOverlappingFuncs(std::set<func_instance *> &funcs)
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

std::string func_instance::get_name() const
{
   return symTabName();
}

Offset func_instance::addrToOffset(const Address a) const {
   return a - (addr() - ifunc()->getOffset());
}

const pdvector< int_parRegion* > &func_instance::parRegions()
{
  if (parallelRegions_.size() > 0)
    return parallelRegions_;

  for (unsigned int i = 0; i < ifunc()->parRegions().size(); i++)
    {
      image_parRegion * imPR = ifunc()->parRegions()[i];
      //int_parRegion * iPR = new int_parRegion(imPR, baseAddr, this);
      int_parRegion * iPR = new int_parRegion(imPR, addr_, this);
      parallelRegions_.push_back(iPR);
    }
  return parallelRegions_;
}

bool func_instance::consistency() const {
   // 1) Check for 1:1 block relationship in
   //    the block list and block map
   // 2) Check that all instPoints are in the
   //    correct block.

   const ParseAPI::Function::blocklist &img_blocks = ifunc()->blocks();
   assert(img_blocks.size() == blocks_.size());
   for (ParseAPI::Function::blocklist::iterator iter = img_blocks.begin();
        iter != img_blocks.end(); ++iter) {
      parse_block *img_block = SCAST_PB(*iter);
      block_instance *b_inst = obj()->findBlock(img_block);
      assert(blocks_.find(b_inst) != blocks_.end());
   }

   return true;
}

Address func_instance::get_address() const { assert(0); return 0; }
unsigned func_instance::get_size() const { assert(0); return 0; }

bool func_instance::findInsnPoints(instPoint::Type type,
                                   block_instance *b,
                                   InsnInstpoints::const_iterator &begin,
                                   InsnInstpoints::const_iterator &end) {
   std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
   if (iter == blockPoints_.end()) return false;

   switch(type) {
      case instPoint::PreInsn:
         begin = iter->second.preInsn.begin();
         end = iter->second.preInsn.end();
         return true;
      case instPoint::PostInsn:
         begin = iter->second.postInsn.begin();
         end = iter->second.postInsn.end();
         return true;
      default:
         return false;
   }
   return false;
}

bool func_instance::isInstrumentable() {
   return ifunc()->isInstrumentable();

   if (!ifunc()->isInstrumentable()) return false;

   // Hack: avoid things that throw exceptions
   // Make sure we parsed calls
   callBlocks();
   for (BlockSet::iterator iter = callBlocks_.begin(); iter != callBlocks_.end(); ++iter) {
      if ((*iter)->calleeName().find("cxa_throw") != std::string::npos) {
         cerr << "Func " << symTabName() << " found exception ret false" << endl;
         return false;
      }
      func_instance *callee = (*iter)->callee();

      cerr << "Func " << symTabName() << " @ " << hex
           << (*iter)->start() << ", callee " << (*iter)->calleeName() << dec << endl;

      if (!callee) {
         cerr << "Warning: null callee" << endl;
         continue;
      }
      cerr << "Checking callee named " << callee->symTabName() << endl;

      if (callee->symTabName().find("cxa_throw") != std::string::npos) {
         cerr << "Func " << symTabName() << " found exception ret false" << endl;
         return false;
      }

      // TEMPORARY HACKAGE because we're not picking up names for
      // PLT functions?
      if (callee->symTabName().find("402700") != std::string::npos) {
         cerr << "Func " << symTabName() << " found exception ret false" << endl;
         return false;
      }

   }
   return true;

}

block_instance *func_instance::getBlock(const Address addr) {
        block_instance *block = obj()->findOneBlockByAddr(addr);
        // Make sure it's one of ours
        std::set<func_instance *> funcs;
        block->getFuncs(std::inserter(funcs, funcs.end()));
        if (funcs.find(this) != funcs.end()) {
          //addBlock(block); // Update parent class's bookkeeping stuffs
          return block;
        }
        return NULL;
}

using namespace SymtabAPI;

bool func_instance::callWrappedFunction(func_instance *target) {
   // Not implemented for dynamic mode
   if (proc()->proc()) return false;
   // And not implemented for same-module
   if (obj() == target->obj()) return false;
   // Get the old symbol
   Symbol *oldsym = target->getRelocSymbol();

   // Get the new symbol
   Symbol *wrapperSym = target->getWrapperSymbol();
   if (!wrapperSym) {
      return false;
   }

   // Now we split. If this is a static binary, we want to point all the relocations
   // in this function at the new symbol. If it's a dynamic binary, we can just relocate
   // the daylights out of it.
   if (obj()->isStaticExec() || target->obj()->isStaticExec() ) {
      if (!updateRelocationsToSym(oldsym, wrapperSym)) return false;
   }
   else {
      // Not implemented yet
      return false;
   }

   return true;
}

bool func_instance::updateRelocationsToSym(Symbol *oldsym, Symbol *newsym) {
   for (BlockSet::const_iterator iter = blocks().begin();
        iter != blocks().end(); ++iter) {
      obj()->parse_img()->getObject()->updateRelocations((*iter)->start(), (*iter)->last(), oldsym, newsym);
   }
   return true;
}

Symbol *func_instance::getWrapperSymbol() {
   // Is created during relocation, which should have
   // already happened.
   return wrapperSym_;
}

Symbol *func_instance::getRelocSymbol() {
   // there should be only one...
   // HIGHLANDER!

   // find the Symbol corresponding to the func_instance
   std::vector<Symbol *> syms;
   ifunc()->func()->getSymbols(syms);

   if (syms.size() == 0) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s"
              , __FILE__, __LINE__, name().c_str());
      showErrorCallback(80, msg);
      assert(0);
   }

   // try to find a dynamic symbol
   // (take first static symbol if none are found)
   Symbol *referring = syms[0];
   for (unsigned k=0; k<syms.size(); k++) {
      if (syms[k]->isInDynSymtab()) {
         referring = syms[k];
         break;
      }
   }
   return referring;
}

void func_instance::createWrapperSymbol(Address entry) {
   if (wrapperSym_) {
      // Just update the address
      wrapperSym_->setOffset(entry);
      return;
   }
   // Otherwise we need to create a new symbol
   std::string wrapperName = name();
   wrapperName += "_dyninst_wrapper";

   wrapperSym_ = new Symbol(wrapperName,
                            Symbol::ST_FUNCTION,
                            Symbol::SL_GLOBAL,
                            Symbol::SV_DEFAULT,
                            entry,
                            NULL, // This is module - I probably want this?
                            NULL, // Region - again, find it?
                            0, // size - zero okay?
                            false, // Definitely not dynamic ("Static binaries don't have dynamic symbols - Dan")
                            false); // Definitely not absolute
   obj()->parse_img()->getObject()->addSymbol(wrapperSym_);

}

/* PatchAPI stuffs */

instPoint *func_instance::funcEntryPoint(bool create) {
   assert(proc()->mgr());
   // Lookup the cached points
   if (points_.entry) return points_.entry;
   if (!create) return NULL;
   // Cache miss, resort to PatchAPI's findPoint
   std::vector<Point*> pts;
   proc()->mgr()->findPoints(this, Point::FuncEntry, back_inserter(pts));
   assert(pts.size() == 1);
   points_.entry = static_cast<instPoint*>(pts[0]);
   return points_.entry;
}

instPoint *func_instance::funcExitPoint(block_instance* b, bool create) {
  assert(proc()->mgr());

  // Lookup the cached points
  std::map<block_instance *, instPoint *>::iterator iter = points_.exits.find(b);
  if (iter != points_.exits.end()) return iter->second;
  if (!create) return NULL;
  // Cache miss, resort to PatchAPI's findPoint
  Points pts;
  funcExitPoints(&pts);
  assert(pts.size() > 0);
  iter = points_.exits.find(b);
  if (iter != points_.exits.end()) return iter->second;
  assert(0);
  return NULL;
}

void func_instance::funcExitPoints(Points* pts) {
  assert(proc()->mgr());
  // Lookup the cached points
  if (points_.exits.size() > 0) {
    for (std::map<block_instance*, instPoint*>::iterator pi = points_.exits.begin();
         pi != points_.exits.end(); pi++) {
      pts->push_back(pi->second);
    }
    return;
  }
  // Cache miss, resort to PatchAPI's findPoint
  std::vector<Point*> points;
  proc()->mgr()->findPoints(this, Point::FuncExit, back_inserter(points));
  for (std::vector<Point*>::iterator pi = points.begin(); pi != points.end(); pi++) {
    instPoint* p = static_cast<instPoint*>(*pi);
    pts->push_back(p);
    points_.exits[p->block()] = p;
  }
}

instPoint *func_instance::preCallPoint(block_instance* b, bool create) {
  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.preCall) return iter->second.preCall;
  }
  if (!create) return NULL;
  Points pts;
  callPoints(&pts);
  iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.preCall) return iter->second.preCall;
  }
  return NULL;
}

instPoint *func_instance::postCallPoint(block_instance* b, bool create) {
  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.postCall) return iter->second.postCall;
  }
  if (!create) return NULL;
  Points pts;
  callPoints(&pts);
  iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.postCall) return iter->second.postCall;
  }
  return NULL;
}

void func_instance::callPoints(Points* pts) {
  assert(proc()->mgr());
  // Lookup the cached points
  if (blockPoints_.size() > 0) {
    bool done = true;
    for (std::map<block_instance*, BlockInstpoints>::iterator pi = blockPoints_.begin();
         pi != blockPoints_.end(); pi++) {
      if (!pi->second.preCall) {
        done = false;
        break;
      }
      pts->push_back(pi->second.preCall);
      pts->push_back(pi->second.postCall);
    }
    if (done) return;
  }
  // Cache miss, resort to PatchAPI's findPoint
  std::vector<Point*> points;
  proc()->mgr()->findPoints(this, Point::PreCall|Point::PostCall, back_inserter(points));
  for (std::vector<Point*>::iterator pi = points.begin(); pi != points.end(); pi++) {
    instPoint* p = static_cast<instPoint*>(*pi);
    pts->push_back(p);
    if (p->type() == Point::PreCall) blockPoints_[p->block()].preCall = p;
    else blockPoints_[p->block()].postCall = p;
  }
}

instPoint *func_instance::blockEntryPoint(block_instance* b, bool create) {
  assert(proc()->mgr());
  // Lookup the cached points
  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.entry) return iter->second.entry;
  }
  if (!create) return NULL;
  // Cache miss, resort to PatchAPI's findPoint
  std::vector<Point*> pts;
  proc()->mgr()->findPoints(b, Point::BlockEntry, back_inserter(pts));
  assert(pts.size() == 1);
  blockPoints_[b].entry = static_cast<instPoint*>(pts[0]);
  return blockPoints_[b].entry;
}

instPoint *func_instance::blockExitPoint(block_instance* b, bool create) {
  assert(proc()->mgr());
  // Lookup the cached points
  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.exit) return iter->second.exit;
  }
  if (!create) return NULL;
  // Cache miss, resort to PatchAPI's findPoint
  std::vector<Point*> pts;
  proc()->mgr()->findPoints(b, Point::BlockExit, back_inserter(pts));
  assert(pts.size() == 1);
  blockPoints_[b].exit = static_cast<instPoint*>(pts[0]);
  return blockPoints_[b].exit;
}

instPoint *func_instance::preInsnPoint(block_instance* b, Address a,
                                       InstructionAPI::Instruction::Ptr ptr,
                                       bool trusted, bool create) {
  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    std::map<Address, instPoint *>::iterator iter2 = iter->second.preInsn.find(a);
    if (iter2 != iter->second.preInsn.end()) {
      return iter2->second;
    }
  }
  if (!create) return NULL;
  if (!trusted || !ptr) {
    ptr = b->getInsn(a);
    if (!ptr) return NULL;
  }
  Points pts;
  blockInsnPoints(b, &pts);
  assert(pts.size() > 0);
  iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    std::map<Address, instPoint *>::iterator iter2 = iter->second.preInsn.find(a);
    if (iter2 != iter->second.preInsn.end()) {
      return iter2->second;
    }
  }
  assert(0);
  return NULL;
}

instPoint *func_instance::postInsnPoint(block_instance* b, Address a,
                                        InstructionAPI::Instruction::Ptr ptr,
                                        bool trusted, bool create) {

  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    std::map<Address, instPoint *>::iterator iter2 = iter->second.postInsn.find(a);
    if (iter2 != iter->second.postInsn.end()) {
      return iter2->second;
    }
  }
  if (!create) return NULL;
  if (!trusted || !ptr) {
    ptr = b->getInsn(a);
    if (!ptr) return NULL;
  }
  Points pts;
  blockInsnPoints(b, &pts);
  assert(pts.size() > 0);
  iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    std::map<Address, instPoint *>::iterator iter2 = iter->second.postInsn.find(a);
    if (iter2 != iter->second.postInsn.end()) {
      return iter2->second;
    }
  }
  assert(0);
  return NULL;
}

void func_instance::blockInsnPoints(block_instance* b, Points* pts) {
  assert(proc()->mgr());
  // Lookup the cache
  std::map<block_instance *, BlockInstpoints>::iterator iter = blockPoints_.find(b);
  if (iter != blockPoints_.end()) {
    if (iter->second.preInsn.size() > 0 && iter->second.postInsn.size() > 0) {
      for (std::map<Address, instPoint*>::iterator iter2 = iter->second.preInsn.begin();
           iter2 != iter->second.preInsn.end(); iter2++) {
        pts->push_back(iter2->second);
      }
      for (std::map<Address, instPoint*>::iterator iter2 = iter->second.postInsn.begin();
           iter2 != iter->second.postInsn.end(); iter2++) {
        pts->push_back(iter2->second);
      }
      return;
    }
  }
  // Cache miss!
  std::vector<Point*> points;
  proc()->mgr()->findPoints(b, Point::PreInsn|Point::PostInsn, back_inserter(points));
  assert(points.size() > 0);
  for (std::vector<Point*>::iterator i = points.begin(); i != points.end(); i++) {
    instPoint* pt = static_cast<instPoint*>(*i);
    Address a = pt->address();
    pts->push_back(pt);
    if (pt->type() == Point::PreInsn) blockPoints_[b].preInsn[a] = pt;
    else blockPoints_[b].postInsn[a] = pt;
  }
}

instPoint* func_instance::edgePoint(edge_instance* e, bool create) {
  std::map<edge_instance *, EdgeInstpoints>::iterator iter = edgePoints_.find(e);
  if (iter != edgePoints_.end()) {
    if (iter->second.point) return iter->second.point;
  }
  if (!create) return NULL;
  Points pts;
  edgePoints(&pts);
  iter = edgePoints_.find(e);
  if (iter != edgePoints_.end()) {
    if (iter->second.point) return iter->second.point;
  }
  return NULL;
}

void func_instance::edgePoints(Points* pts) {
  assert(proc()->mgr());
  if (edgePoints_.size() > 0) {
    for (std::map<edge_instance*, EdgeInstpoints>::iterator iter = edgePoints_.begin();
         iter != edgePoints_.end(); iter++) {
      pts->push_back(iter->second.point);
    }
    return;
  }
  std::vector<Point*> points;
  proc()->mgr()->findPoints(this, Point::EdgeDuring, back_inserter(points));
  for (std::vector<Point*>::iterator i = points.begin(); i != points.end(); i++) {
    instPoint* pt = static_cast<instPoint*>(*i);
    edge_instance* e = static_cast<edge_instance*>(pt->edge());
    pts->push_back(pt);
    edgePoints_[e].point = pt;
  }
}

void func_instance::destroyBlock(block_instance *block) {
   // Put things here that go away from the perspective of this function
}

void func_instance::destroy(func_instance *f) {
   // Put things here that go away when we destroy this function
   delete f;
}

using namespace SymtabAPI;

bool func_instance::callWrappedFunction(func_instance *target) {
   // Not implemented for dynamic mode
   if (proc()->proc()) return false;
   // And not implemented for same-module
   if (obj() == target->obj()) return false;
   // Get the old symbol
   Symbol *oldsym = target->getRelocSymbol();

   // Get the new symbol
   Symbol *wrapperSym = target->getWrapperSymbol();
   if (!wrapperSym) {
      return false;
   }
   
   // Now we split. If this is a static binary, we want to point all the relocations
   // in this function at the new symbol. If it's a dynamic binary, we can just relocate
   // the daylights out of it. 
   if (obj()->isStaticExec() || target->obj()->isStaticExec() ) {
      if (!updateRelocationsToSym(oldsym, wrapperSym)) return false;
   }
   else {
      // Not implemented yet
      return false;
   }

   return true;
}

bool func_instance::updateRelocationsToSym(Symbol *oldsym, Symbol *newsym) {
   for (BlockSet::const_iterator iter = blocks().begin(); 
        iter != blocks().end(); ++iter) {
      obj()->parse_img()->getObject()->updateRelocations((*iter)->start(), (*iter)->last(), oldsym, newsym);
   }
   return true;
}

Symbol *func_instance::getWrapperSymbol() {
   // Is created during relocation, which should have 
   // already happened.
   return wrapperSym_;
}

Symbol *func_instance::getRelocSymbol() {
   // there should be only one...
   // HIGHLANDER!
   
   // find the Symbol corresponding to the func_instance
   std::vector<Symbol *> syms;
   ifunc()->func()->getSymbols(syms);
   
   if (syms.size() == 0) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s"
              , __FILE__, __LINE__, name().c_str());
      showErrorCallback(80, msg);
      assert(0);
   }
   
   // try to find a dynamic symbol
   // (take first static symbol if none are found)
   Symbol *referring = syms[0];
   for (unsigned k=0; k<syms.size(); k++) {
      if (syms[k]->isInDynSymtab()) {
         referring = syms[k];
         break;
      }
   }
   return referring;
}

void func_instance::createWrapperSymbol(Address entry) {
   if (wrapperSym_) {
      // Just update the address
      wrapperSym_->setOffset(entry);
      return;
   }
   // Otherwise we need to create a new symbol
   std::string wrapperName = name();
   wrapperName += "_dyninst_wrapper";

   wrapperSym_ = new Symbol(wrapperName,
                            Symbol::ST_FUNCTION,
                            Symbol::SL_GLOBAL,
                            Symbol::SV_DEFAULT,
                            entry,
                            NULL, // This is module - I probably want this?
                            NULL, // Region - again, find it?
                            0, // size - zero okay?
                            false, // Definitely not dynamic ("Static binaries don't have dynamic symbols - Dan")
                            false); // Definitely not absolute
   obj()->parse_img()->getObject()->addSymbol(wrapperSym_);

}

void func_instance::destroyBlock(block_instance *block) {
   // Put things here that go away from the perspective of this function
}

void func_instance::destroy(func_instance *f) {
   // Put things here that go away when we destroy this function
   delete f;
}
