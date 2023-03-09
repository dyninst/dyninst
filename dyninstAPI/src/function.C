/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <random>
#include "function.h"
#include "instPoint.h"
#include "debug.h"
#include "dynProcess.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include "Relocation/Transformers/Movement-analysis.h"

#include "PatchMgr.h" // Scope

#include "Parsing.h"

#include "binaryEdit.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::PatchAPI;
using namespace Dyninst::Relocation;
using namespace Dyninst::PatchAPI;

int func_instance_count = 0;

func_instance::func_instance(parse_func *f,
                             Address baseAddr,
                             mapped_module *mod) :
  PatchFunction(f, mod->obj()),
  ptrAddr_(f->getPtrOffset() ? f->getPtrOffset() + baseAddr : 0),
  mod_(mod),
  prevBlocksAbruptEnds_(0),
  handlerFaultAddr_(0),
  handlerFaultAddrAddr_(0)
#if defined(os_windows)
  , callingConv(unknown_call)
  , paramSize(0)
#endif
   , wrapperSym_(NULL),
     _noPowerPreambleFunc(NULL),
     _powerPreambleFunc(NULL)
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
#if defined(cap_stack_mods)
    _hasStackMod = false ;
    _processedOffsetVector = false;
    _hasDebugSymbols = false;
    _seeded = false;
    _randomizeStackFrame = false;
    _hasCanary = false;
    _modifications = new std::set<StackMod*>();
    _offVec = new OffsetVector();
    _tmpObjects = new set<tmpObject, less_tmpObject>();
    _tMap = new TMap();
    _accessMap = new std::map<Address, Accesses*>();
    _definitionMap = new std::map<Address, StackAccess *>();
    assert(_modifications && _offVec && _tMap && _accessMap && _definitionMap);
#endif
}

unsigned func_instance::footprint() {
    unsigned totalSize = 0;
    vector<FuncExtent*> exts = ifunc()->extents();
    for (unsigned i=0; i < exts.size(); i++) {
        totalSize += (exts[i]->end() - exts[i]->start());
    }
    return totalSize;
}

func_instance::func_instance(const func_instance *parFunc,
                             mapped_module *childMod) :
  PatchFunction(parFunc->ifunc(), childMod->obj()),
  ptrAddr_(parFunc->ptrAddr_),
  mod_(childMod),
  prevBlocksAbruptEnds_(0),
  handlerFaultAddr_(0),
  handlerFaultAddrAddr_(0)
#if defined(os_windows)
  , callingConv(parFunc->callingConv)
  , paramSize(parFunc->paramSize)
#endif
   , wrapperSym_(NULL),
     _noPowerPreambleFunc(parFunc->_noPowerPreambleFunc),
     _powerPreambleFunc(parFunc->_powerPreambleFunc)

{
   assert(ifunc());
   // According to contract /w/ the mapped_object
   // all blocks have already been constructed.
   // Do we duplicate the parent or wait? I'm
   // tempted to wait, just because of the common
   // fork/exec case.
#if defined(cap_stack_mods)
   _hasStackMod = false ;
   _processedOffsetVector = false;
   _hasDebugSymbols = false;
   _seeded = false;
   _randomizeStackFrame = false;
   _hasCanary = false;
   _modifications = new std::set<StackMod*>();
   _offVec = new OffsetVector();
   _tmpObjects = new set<tmpObject, less_tmpObject>();
   _tMap = new TMap();
   _accessMap = new std::map<Address, Accesses*>();
   _definitionMap = new std::map<Address, StackAccess *>();
   assert(_modifications && _offVec && _tMap && _accessMap && _definitionMap);
#endif
}

func_instance::~func_instance() { 
   // We don't delete blocks, since they're shared between functions
   // We _do_ delete context instPoints, though
   // Except that should get taken care of normally since the
   // structures are static. 
   for (unsigned i = 0; i < parallelRegions_.size(); i++)
      delete parallelRegions_[i];
   if (wrapperSym_ != NULL) {
      delete wrapperSym_;
   }
}

// the original entry block is gone, we choose a new entry block from the
// function, whichever non-dead block we can find that has no intraprocedural 
// incoming edges.  If there's no obvious block to choose, we stick with the
// default block
block_instance * func_instance::setNewEntry(block_instance *def,
                                            std::set<block_instance*> &deadBlocks)
{
    block_instance *newEntry = NULL;
    assert(!all_blocks_.empty());

    // choose block with no intraprocedural incoming edges
    PatchFunction::Blockset::iterator bIter;
    for (bIter = all_blocks_.begin(); 
         bIter != all_blocks_.end(); 
         bIter++) 
    {
        block_instance *block = static_cast<block_instance*>(*bIter);
        if (deadBlocks.find(block) == deadBlocks.end()) {
            ParseAPI::Intraproc epred;
            const Block::edgelist & ib_ins = block->llb()->sources();
	    
	    if(std::distance(boost::make_filter_iterator(epred, ib_ins.begin(), ib_ins.end()), 
			     boost::make_filter_iterator(epred, ib_ins.end(), ib_ins.end())) == 0)
            {
                if (NULL != newEntry) {
                    fprintf(stderr,"WARNING: multiple blocks in function %lx "
                        "with overwritten entry point have no incoming edges: "
                        "[%lx %lx) and [%lx %lx) %s[%d]\n",
                        addr_, newEntry->llb()->start(),
                        newEntry->llb()->start() + newEntry->llb()->end(),
                        block->llb()->start(),
                        block->llb()->start() + block->llb()->end(),
                        FILE__,__LINE__);
                } else {
                    newEntry = block;
                }
            }
        }
    }
    // if all blocks have incoming edges, choose the default block
    if( ! newEntry ) {
        newEntry = def;
        mal_printf("Setting new entry block for func at 0x%lx to "
                "actively executing block [%lx %lx), as none of the function "
                "blocks lacks intraprocedural edges %s[%d]\n", addr_, 
                def->start(), def->end(), FILE__,__LINE__);
    }

    // set the new entry block
    ifunc()->setEntryBlock(newEntry->llb());
    addr_ = newEntry->start();

    // debug output
    if (newEntry->isShared()) {
        mal_printf("New entry block chosen for func 0x%lx is shared\n",addr_);
    }
    mal_printf("Func has new entry block [%lx %lx)\n",
               newEntry->start(), newEntry->end());
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
                                      std::vector<func_instance *>*funcs) {
    unsigned int i;
    func_instance *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

mapped_object *func_instance::obj() const { return mod()->obj(); }
AddressSpace *func_instance::proc() const { return obj()->proc(); }

const func_instance::BlockSet &func_instance::unresolvedCF() {
   if (ifunc()->getPrevBlocksUnresolvedCF() != ifunc()->num_blocks()) {
       ifunc()->setPrevBlocksUnresolvedCF(ifunc()->num_blocks());
       // A block has unresolved control flow if it has an indirect
       // out-edge.
       blocks(); // force initialization of all_blocks_
       for (PatchFunction::Blockset::const_iterator iter = all_blocks_.begin(); 
            iter != all_blocks_.end(); ++iter) 
       {
          block_instance* iblk = SCAST_BI(*iter);
          if (iblk->llb()->unresolvedCF()) {
             unresolvedCF_.insert(iblk);
         }
      }
   }
   return unresolvedCF_;
}

const func_instance::BlockSet &func_instance::abruptEnds() {
    if (prevBlocksAbruptEnds_ != ifunc()->num_blocks()) {
       prevBlocksAbruptEnds_ = blocks().size();
        for (PatchFunction::Blockset::const_iterator iter = all_blocks_.begin(); 
             iter != all_blocks_.end(); ++iter) 
        {
            block_instance* iblk = SCAST_BI(*iter);
            if (iblk->llb()->abruptEnd()) {
                abruptEnds_.insert(iblk);
            }
        }
    }
    return abruptEnds_;
}

void func_instance::removeAbruptEnd(const block_instance *block)
{
    if (abruptEnds_.empty()) {
        return;
    }

    BlockSet::iterator bit = abruptEnds_.find(const_cast<block_instance*>(block));
    if (abruptEnds_.end() != bit) {
        abruptEnds_.erase(bit);
    }
}


block_instance *func_instance::entryBlock() {
  return SCAST_BI(entry());
}

unsigned func_instance::getNumDynamicCalls()
{
   unsigned count=0;
   for (PatchFunction::Blockset::const_iterator iter = callBlocks().begin(); 
        iter != callBlocks().end(); ++iter) 
   {
      block_instance* iblk = SCAST_BI(*iter);
      if (iblk->containsDynamicCall()) {
         count++;
      }
   }
   return count;
}


// warning: doesn't (and can't) force initialization of lazily-built 
// data structures because this function is declared to be constant
void func_instance::debugPrint() const {
    fprintf(stderr, "Function debug dump (%p):\n", (const void*)this);
    fprintf(stderr, "  Symbol table names:\n");
    for (auto i = symtab_names_begin(); 
	 i != symtab_names_end(); ++i) {
      fprintf(stderr, "    %s\n", i->c_str());
    }
    fprintf(stderr, "  Demangled names:\n");
    for (auto j = pretty_names_begin(); 
	 j != pretty_names_end(); ++j) {
      fprintf(stderr, "    %s\n", j->c_str());
    }
    fprintf(stderr, "  Typed names:\n");
    for (auto k = typed_names_begin(); 
	 k != typed_names_end(); ++k) {
      fprintf(stderr, "    %s\n", k->c_str());
    }
    fprintf(stderr, "  Address: 0x%lx\n", addr());
    fprintf(stderr, "  Internal pointer: %p\n", (void*)ifunc());
    fprintf(stderr, "  Object: %s (%p), module: %s (%p)\n",
            obj()->fileName().c_str(),
            (void*)obj(),
            mod()->fileName().c_str(),
            (void*)mod());
    for (Blockset::const_iterator
         cb = all_blocks_.begin();
         cb != all_blocks_.end();
         cb++)
    {
        block_instance* orig = SCAST_BI(*cb);
        fprintf(stderr, "  Block start 0x%lx, end 0x%lx\n", orig->start(),
                orig->end());
    }
}

// Add to internal
// Add to mapped_object if a "new" name (true return from internal)
void func_instance::addSymTabName(const std::string name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
       obj()->addFunctionName(this, name, obj()->allFunctionsByMangledName);
}

void func_instance::addPrettyName(const std::string name, bool isPrimary) {
    if (ifunc()->addPrettyName(name, isPrimary))
       obj()->addFunctionName(this, name, obj()->allFunctionsByPrettyName);
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
    PatchFunction::Blockset::const_iterator bIter;
    for (bIter = blocks().begin();
         bIter != blocks().end();
         bIter++) {
      block_instance* iblk = SCAST_BI(*bIter);
       if (getSharingFuncs(iblk,funcs))
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
    PatchFunction::Blockset::const_iterator bIter;
    for (bIter = blocks().begin();
         bIter != blocks().end();
         bIter++) {
      block_instance* iblk = SCAST_BI(*bIter);
       if (getOverlappingFuncs(iblk,funcs))
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

const std::vector< int_parRegion* > &func_instance::parRegions()
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

   auto img_blocks = ifunc()->blocks();
   assert(ifunc()->num_blocks() == all_blocks_.size());
   for (auto iter = img_blocks.begin();
        iter != img_blocks.end(); ++iter) {
      parse_block *img_block = SCAST_PB(*iter);
      block_instance *b_inst = obj()->findBlock(img_block);
      assert(b_inst->llb() == img_block);
   }

   return true;
}

void func_instance::triggerModified() {
    // KEVINTODO: is there anything to do here? 
}

Address func_instance::get_address() const { assert(0); return 0; }
unsigned func_instance::get_size() const { assert(0); return 0; }


bool func_instance::isInstrumentable() {
#if defined(os_freebsd)
  // FreeBSD system call wrappers are using an indirect jump to an error
  // handling function; this confuses our parsing and we conclude they
  // are uninstrumentable. They're not. It's fine. 
  
  std::string wrapper_prefix = "__sys_";
  for(auto i = symtab_names_begin(); i != symtab_names_end(); ++i) 
  {
    if (i->compare(0, 6, wrapper_prefix) == 0) {
      return true;
    }
  }
#endif

  Dyninst::PatchAPI::Instrumenter* inst = proc()->mgr()->instrumenter();
  if (inst) return inst->isInstrumentable(this);
  cerr << "Error: no instrumenter for " << symTabName() << endl;
  return false;
}

block_instance *func_instance::getBlockByEntry(const Address addr) {
   block_instance *block = obj()->findBlockByEntry(addr);
   if (block) {
      blocks(); // force initialization of all_blocks_
      if (all_blocks_.find(block) != all_blocks_.end()) {
         return block;
      }
   }
   return NULL;
}


// get all blocks that have an instruction starting at addr, or if 
// there are none, return all blocks containing addr
bool func_instance::getBlocks(const Address addr, set<block_instance*> &blks) {
   set<block_instance*> objblks;
   obj()->findBlocksByAddr(addr, objblks);
   blocks(); // ensure that all_blocks_ is filled in 
   std::vector<std::set<block_instance *>::iterator> to_erase; 
   for (set<block_instance*>::iterator bit = objblks.begin(); bit != objblks.end(); bit++) {
      // Make sure it's one of ours
      if (all_blocks_.find(*bit) == all_blocks_.end()) {
         to_erase.push_back(bit);
      }
   }
   for (unsigned i = 0; i < to_erase.size(); ++i) {
      objblks.erase(to_erase[i]);
   }

   if (objblks.size() > 1) { 
      // only add blocks that have an instruction at "addr"
      for (set<block_instance*>::iterator bit = objblks.begin(); bit != objblks.end(); bit++) {
         if ((*bit)->getInsn(addr).isValid()) {
            blks.insert(*bit);
         }
      }
   }
   // if there are no blocks containing an instruction that starts at addr, 
   // but there are blocks that contain addr, add those to blks
   if (blks.empty() && !objblks.empty()) {
      std::copy(objblks.begin(), objblks.end(), std::inserter(blks, blks.end()));
   }
   return ! blks.empty();
}

block_instance *func_instance::getBlock(const Address addr) {
   std::set<block_instance *> blks;
   getBlocks(addr, blks);
   for (std::set<block_instance *>::iterator iter = blks.begin(); iter != blks.end(); ++iter) {
      if ((*iter)->getInsn(addr).isValid()) return *iter;
   }
   return NULL;
}

using namespace SymtabAPI;

bool func_instance::addSymbolsForCopy() {
   // Not implemented for dynamic mode
   if (proc()->proc()) return false;
   // And not implemented for same-module

   // Get the old symbol
   Symbol *oldsym = getRelocSymbol();

   // Get the new symbol
   Symbol *wrapperSym = getWrapperSymbol();
   if (!wrapperSym) {
      return false;
   }
   // Now we split. If this is a static binary, we want to point all the relocations
   // in this function at the new symbol. If it's a dynamic binary, we can just relocate
   // the daylights out of it.
   if (obj()->isStaticExec()) {
      proc()->edit()->addDyninstSymbol(wrapperSym_);
      if (!updateRelocationsToSym(oldsym, wrapperSym)) return false;
   }
   else {
      // I think we just add this to the dynamic symbol table...
      wrapperSym->setDynamic(true);
      proc()->edit()->addDyninstSymbol(wrapperSym_);
   }

   return true;
}

bool func_instance::updateRelocationsToSym(Symbol *oldsym, Symbol *newsym) {
   for (Blockset::const_iterator iter = blocks().begin();
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

void func_instance::createWrapperSymbol(Address entry, std::string name) {
   if (wrapperSym_) {
      // Just update the address
      wrapperSym_->setOffset(entry);
      return;
   }
   // Otherwise we need to create a new symbol
   wrapperSym_ = new Symbol(name,
                            Symbol::ST_FUNCTION,
                            Symbol::SL_GLOBAL,
                            Symbol::SV_DEFAULT,
                            entry,
                            NULL, // This is module - I probably want this?
                            NULL, // Region - again, find it?
                            0, // size - zero okay?
                            false, // Definitely not dynamic ("Static binaries don't have dynamic symbols - Dan")
                            false); // Definitely not absolute

}

/* PatchAPI stuffs */

instPoint *func_instance::funcEntryPoint(bool create) {
   // Lookup the cached points
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::Function(this), Point::FuncEntry, create));
   return p;
}

instPoint *func_instance::funcExitPoint(block_instance* b, bool create) {
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::ExitSite(this, b), Point::FuncExit, create));
   return p;
}

void func_instance::funcExitPoints(Points* pts) {
  std::vector<Point*> points;
  proc()->mgr()->findPoints(Scope(this), Point::FuncExit, back_inserter(points));
  for (std::vector<Point*>::iterator pi = points.begin(); pi != points.end(); pi++) {
    instPoint* p = IPCONV(*pi);
    pts->push_back(p);
    assert(p->block());
  }
}

instPoint *func_instance::preCallPoint(block_instance* b, bool create) {
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::CallSite(this, b), Point::PreCall, create));
  return p;
}

instPoint *func_instance::postCallPoint(block_instance* b, bool create) {
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::CallSite(this, b), Point::PostCall, create));
   return p;
}

void func_instance::callPoints(Points* pts) {
  std::vector<Point*> points;
  proc()->mgr()->findPoints(Scope(this), Point::PreCall|Point::PostCall, back_inserter(points));
  for (std::vector<Point*>::iterator pi = points.begin(); pi != points.end(); pi++) {
     instPoint* p = static_cast<instPoint*>(*pi);
     pts->push_back(p);
  }
}

instPoint *func_instance::blockEntryPoint(block_instance* b, bool create) {
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::BlockInstance(this, b), Point::BlockEntry, create));
   return p;
}

instPoint *func_instance::blockExitPoint(block_instance* b, bool create) {
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::BlockInstance(this, b), Point::BlockExit, create));
   return p;
}

instPoint *func_instance::preInsnPoint(block_instance *b, Address a,
                                       InstructionAPI::Instruction insn,
                                       bool trusted, bool create) {
   Location loc = Location::InstructionInstance(this, b, a, insn, trusted);

   instPoint *p = IPCONV(proc()->mgr()->findPoint(loc, Point::PreInsn, create));
   return p;
}

instPoint *func_instance::postInsnPoint(block_instance *b, Address a,
                                        InstructionAPI::Instruction insn,
                                        bool trusted, bool create) {
   Location loc = Location::InstructionInstance(this, b, a, insn, trusted);

   instPoint *p = IPCONV(proc()->mgr()->findPoint(loc, Point::PostInsn, create));
   return p;
}

void func_instance::blockInsnPoints(block_instance* b, Points* pts) {
  std::vector<Point*> points;
  proc()->mgr()->findPoints(Scope(this, b), Point::PreInsn|Point::PostInsn, back_inserter(points));
  assert(points.size() > 0);
  for (std::vector<Point*>::iterator i = points.begin(); i != points.end(); i++) {
    instPoint* pt = static_cast<instPoint*>(*i);
    pts->push_back(pt);
  }
}

instPoint* func_instance::edgePoint(edge_instance* e, bool create) {
   instPoint *p = IPCONV(proc()->mgr()->findPoint(Location::EdgeInstance(this, e), Point::EdgeDuring, create));
   return p;
}

void func_instance::edgePoints(Points* pts) {
   std::vector<Point *> points;
   proc()->mgr()->findPoints(Scope(this), Point::EdgeDuring, back_inserter(points));
   for (std::vector<Point*>::iterator i = points.begin(); i != points.end(); i++) {
      instPoint* pt = static_cast<instPoint*>(*i);
      pts->push_back(pt);
   }
}


void func_instance::removeBlock(block_instance *block) {
    // Put things here that go away from the perspective of this function
    BlockSet::iterator bit = unresolvedCF_.find(block);
    if (bit != unresolvedCF_.end()) {
        unresolvedCF_.erase(bit);
        size_t prev = ifunc()->getPrevBlocksUnresolvedCF();
        if (prev > 0) {
           ifunc()->setPrevBlocksUnresolvedCF(prev - 1);
        }
    }
    bit = abruptEnds_.find(block);
    if (bit != abruptEnds_.end()) {
        abruptEnds_.erase(block);
        if (prevBlocksAbruptEnds_ > 0) {
           prevBlocksAbruptEnds_ --;
        }
    }
}

void func_instance::destroy(func_instance *f) {
   // Put things here that go away when we destroy this function
   delete f;
}

void func_instance::split_block_cb(block_instance *b1, block_instance *b2)
{

    BlockSet::iterator bit = unresolvedCF_.find(b1);
    if (bit != unresolvedCF_.end()) {
        unresolvedCF_.erase(*bit);
        unresolvedCF_.insert(b2);
    }
    bit = abruptEnds_.find(b1);
    if (bit != abruptEnds_.end()) {
        abruptEnds_.erase(*bit);
        abruptEnds_.insert(b2);
    }
}

void func_instance::add_block_cb(block_instance * /*block*/)
{
#if 0 // KEVINTODO: eliminate this?  as presently constituted, 
      // these if cases will never execute anyway, at least not 
      // when we intend them to
    if (block->llb()->unresolvedCF()) {
       size_t prev = ifunc()->getPrevBlocksUnresolvedCF();
       if (ifunc()->blocks().size() == prev) {
          unresolvedCF_.insert(block);
          ifunc()->setPrevBlocksUnresolvedCF(prev+1);
       }
    }
    if (block->llb()->abruptEnd() && 
        prevBlocksAbruptEnds_ == ifunc()->blocks().size())
    {
        abruptEnds_.insert(block);
        prevBlocksAbruptEnds_ ++;
    }
#endif
}

void func_instance::markModified() {
   proc()->addModifiedFunction(this);
}

// get caller blocks that aren't in deadBlocks
bool func_instance::getLiveCallerBlocks
(const std::set<block_instance*> &deadBlocks, 
 const std::list<func_instance*> &deadFuncs, 
 std::map<Address,vector<block_instance*> > & stubs)  // output: block + target addr
{
   using namespace ParseAPI;

   const PatchBlock::edgelist &callEdges = entryBlock()->sources();
   PatchBlock::edgelist::const_iterator eit = callEdges.begin();
   for( ; eit != callEdges.end(); ++eit) {
      if (CALL == (*eit)->type()) {// includes tail calls
          block_instance *cbbi = static_cast<block_instance*>((*eit)->src());
          if (deadBlocks.end() != deadBlocks.find(cbbi)) {
             continue; 
          }

          // don't use stub if it only appears in dead functions 
          std::set<func_instance*> bfuncs;
          cbbi->getFuncs(std::inserter(bfuncs,bfuncs.end()));
          bool allSrcFuncsDead = true;
          for (std::set<func_instance*>::iterator bfit = bfuncs.begin();
               bfit != bfuncs.end(); 
               bfit++) 
          {
             bool isSrcFuncDead = false;
             for (std::list<func_instance*>::const_iterator dfit = deadFuncs.begin();
                  dfit != deadFuncs.end();
                  dfit++)
             {
                if (*bfit == *dfit) {
                   isSrcFuncDead = true;
                   break;
                }
             }
             if (!isSrcFuncDead) {
                allSrcFuncsDead = false;
                break;
             }
          }
          if (allSrcFuncsDead) {
             continue; 
          }
          // add stub
          stubs[addr()].push_back(cbbi);
      }
   }
   return stubs.end() != stubs.find(addr()) && !stubs[addr()].empty();
}

#if defined(cap_stack_mods)
// Stack modifications
void func_instance::addMod(StackMod* mod, TMap* tMap)
{
    _modifications->insert(mod);

    // Update the transformation mapping
    createTMap_internal(mod, tMap);
}

void func_instance::removeMod(StackMod* mod)
{
    _modifications->erase(mod);
}

void func_instance::printMods() const
{
    stackmods_printf("Modifications for %s\n", prettyName().c_str());
    for (auto modIter = _modifications->begin(); modIter != _modifications->end(); ++modIter) {
        StackMod* mod = *modIter;
        stackmods_printf("\t %s\n", mod->format().c_str());
    }
}

Accesses* func_instance::getAccesses(Address addr)
{
    if (!_processedOffsetVector) {
        createOffsetVector();
    }
    auto found = _accessMap->find(addr);
    if (found != _accessMap->end()) {
        return found->second;
    } else {
        return NULL;
    }
}

bool func_instance::createOffsetVector()
{
    if (_processedOffsetVector) {
        return _validOffsetVector;
    }

    bool ret = true;

    stackmods_printf("createOffsetVector for %s\n", prettyName().c_str());

    // 2-tiered approach:
    //      1. If symbols (DWARF, PDB), use this
    //          Need to get BPatch_localVar info from the BPatch_function
    //      2. Derive from instructions (even with symbols, check for consistency)

    // Use symbols to update the offset vector
    if (!createOffsetVector_Symbols()) {
        ret = false;
    }

    // Use binary analysis to update offset vector
    stackmods_printf("\t using analysis:\n");
    ParseAPI::Function* func = ifunc();
    const ParseAPI::Function::blocklist& blocks = func->blocks();
    for (auto blockIter = blocks.begin(); blockIter != blocks.end(); ++blockIter) {
        if (!ret) break;
        ParseAPI::Block* block = *blockIter;
        ParseAPI::Block::Insns insns;
        block->getInsns(insns);

        for (auto insnIter = insns.begin(); insnIter != insns.end(); ++insnIter) {
            Address addr = (*insnIter).first;
            InstructionAPI::Instruction insn = (*insnIter).second;
            if (!createOffsetVector_Analysis(func, block, insn, addr)) {
                ret = false;
                break;
            }
        }
    }

    // Now that createOffsetVector_Analysis has created entries in
    // _definitionMap for all the definitions we need to modify, we can now fill
    // out the entries with all the information we need.
    for (auto defIter = _definitionMap->begin();
        defIter != _definitionMap->end(); defIter++) {
        const Address defAddr = defIter->first;
        StackAccess *&defAccess = defIter->second;

        // Get the appropriate block and instruction for the definition
        ParseAPI::Block *defBlock = NULL;
        InstructionAPI::Instruction defInsn;
        for (auto blockIter = blocks.begin(); blockIter != blocks.end();
            blockIter++) {
            ParseAPI::Block *block = *blockIter;
            Address tmp;
            if (block->start() <= defAddr &&
                defAddr < block->end() &&
                block->consistent(defAddr, tmp)) {
                defBlock = block;
                defInsn = block->getInsn(defAddr);
                break;
            }
        }
        assert(defBlock != NULL);

        // Populate definition information in defAccess
        Accesses tmpDefAccesses;
        std::set<Address> tmp;
        if (::getAccesses(func, defBlock, defAddr, defInsn, &tmpDefAccesses,
            tmp, true)) {
            assert(tmpDefAccesses.size() == 1);
            std::set<StackAccess *> &defAccessSet =
                tmpDefAccesses.begin()->second;
            assert(defAccessSet.size() == 1);
            defAccess = *defAccessSet.begin();

            _tmpObjects->insert(tmpObject(defAccess->readHeight().height(),
                getAccessSize(defInsn), defAccess->type()));
        } else {
            // If any definition can't be understood sufficiently, we can't do
            // stack modifications safely.
            stackmods_printf("\t\t\t INVALID: getAccesses failed\n");
            ret = false;
            break;
        }
    }

    // Populate offset vector
    for (auto iter = _tmpObjects->begin(); iter != _tmpObjects->end(); ++iter) {
        if (!addToOffsetVector((*iter).offset(), (*iter).size(), (*iter).type(), false, (*iter).valid())) {
            stackmods_printf("INVALID: addToOffsetVector failed (5)\n");
            ret = false;
        }
    }

    _processedOffsetVector = true;
    if (ret) {
        _validOffsetVector = true;
        stackmods_printf("Function %s has FINE OFFSET VECTOR\n", prettyName().c_str());
    } else {
        _validOffsetVector = false;
        stackmods_printf("Function %s has INCONSISTENT OFFSET VECTOR (FUNCTION WILL BE SKIPPED)\n",  prettyName().c_str());
    }

    delete _tmpObjects;
    _tmpObjects = NULL;

    return ret;
}

static bool matchRanges(ValidPCRange* a, ValidPCRange* b)
{
    auto aIter = a->begin();
    auto bIter = b->begin();

    while ( (aIter != a->end()) && (bIter != b->end())) {
        if ( ((*aIter).first == (*bIter).first) && ((*aIter).second.first == (*aIter).second.first)) {
            aIter++;
            bIter++;
        } else {
            return false;
        }
    }
    return true;
}

bool func_instance::randomize(TMap* tMap, bool seeded, int seed)
{
    if (seeded) {
        _seeded = true;
        _seed = seed;
    }

    if (_seeded) {
        stackmods_printf("randomize: using seed %d\n", _seed);
        srand(_seed);
    } else {
        stackmods_printf("randomize: using NULL seed\n");
        srand(unsigned(time(NULL)));
    }

    // Grab the locals
    // Find the lowest in offVec that's not a StackAccess::REGHEIGHT
    bool foundBottom = false;

    IntervalTree<OffsetVector::K,OffsetVector::V> stack = _offVec->stack();

    if (stack.empty()) {
        stackmods_printf("\t no stack variables to randomize\n");
        return false;
    }

    StackAnalysis::Height curLB = stack.lowest();
    IntervalTree<StackAnalysis::Height, int> localsRanges;
    map<int, vector<StackLocation*> > locals;

    Architecture arch = ifunc()->isrc()->getArch();
    int raLoc;
    if (arch == Arch_x86_64) {
        raLoc = -8;
    } else if (arch == Arch_x86) {
        raLoc = -4;
    } else {
        assert(0);
    }
    StackLocation* curLoc;
    int counter = -1;
    for (auto iter = stack.begin(); iter != stack.end(); ) {
        curLoc = (*iter).second.second;
        while ( /* Stop at the end */
                (iter != stack.end()) &&
                /* Stop at/above the RA (i.e., in the caller) */
                (curLoc->off() < raLoc) &&
                (curLoc->off()+curLoc->size() < raLoc) &&
                /* Skip non-debug */
                ((curLoc->type() != StackAccess::StackAccessType::DEBUGINFO_LOCAL) && (curLoc->type() != StackAccess::StackAccessType::DEBUGINFO_PARAM))) {
            ++iter;
            if (iter == stack.end()) {
                break;
            }
            curLoc = (*iter).second.second;

            // Start a new range
            curLB = curLoc->off();
        }

        // Want to always create *contiguous* ranges
        curLB = curLoc->off()-1;

        // Bail if we've reached the end of the stack objects, or gone above the RA
        if ( (iter == stack.end()) || (curLoc->off() >= raLoc) ) {
            break;
        }

        if ((curLoc->type() != StackAccess::StackAccessType::DEBUGINFO_LOCAL) && (curLoc->type() != StackAccess::StackAccessType::DEBUGINFO_PARAM)) {
            break;
        }

        if (!foundBottom) {
            foundBottom = true;
        }

        StackAnalysis::Height lb, ub;
        int tmp;
        if (localsRanges.find(curLB, lb, ub, tmp) && matchRanges(locals.at(counter).back()->valid(), curLoc->valid())) {
            // If there already exists a range for the current LB, update
            localsRanges.update(lb, max(ub, curLoc->off() + curLoc->size()));
            stackmods_printf("%s in range %d\n", curLoc->format().c_str(), counter);
            locals.at(counter).push_back(curLoc);
        } else {
            // Otherwise, start a new range
            counter++;
            curLB = curLoc->off();
            localsRanges.insert(curLoc->off(), curLoc->off() + curLoc->size(), counter);
            stackmods_printf("%s in range %d\n", curLoc->format().c_str(), counter);
            vector<StackLocation*> nextvec;
            locals.insert(make_pair(counter, nextvec));
            locals.at(counter).push_back(curLoc);
        }
        ++iter;
    }

    if (locals.size() == 0) {
        stackmods_printf("\t no locals to randomize\n");
        return false;
    }

    bool randomizedRange = false;

    stackmods_printf("Found locals ranges:\n");
    for (auto iter = localsRanges.begin(); iter != localsRanges.end(); ++iter) {
        stackmods_printf("\t %d: [%ld, %ld)\n", (*iter).second.second, (*iter).first.height(), (*iter).second.first.height());
        vector<StackLocation*> vec = locals.at((*iter).second.second);
        for (auto viter = vec.begin(); viter != vec.end(); ++viter) {
            stackmods_printf("\t\t %s\n", (*viter)->format().c_str());
        }
        if (vec.size() == 1) {
            stackmods_printf("\t skipping range %d: [%ld, %ld), only one local\n", (*iter).second.second, (*iter).first.height(), (*iter).second.first.height());
            continue;
        }

        randomizedRange = true;
        std::random_device rd;
        std::mt19937 urbg{rd()};
        std::shuffle(vec.begin(), vec.end(), urbg);
        StackAnalysis::Height nextLoc = (*iter).first.height();

        for (auto viter = vec.begin(); viter != vec.end(); ++viter) {
            StackLocation* cur = *viter;
            Move* tmp = new Move(cur->off().height(), cur->off().height()+cur->size(), nextLoc.height());
            addMod(tmp, tMap);
            nextLoc += cur->size();
        }
    }

    if (randomizedRange) {
        stackmods_printf("\t After randomize:\n");
        printMods();
    }

    _randomizeStackFrame = true;

    return randomizedRange;
}

bool func_instance::createOffsetVector_Symbols()
{
    bool ret = true;

    if (_vars.size() == 0 && _params.size() == 0) {
        // No DWARF or no locals
        // Is there another way to check for DWARF?
        stackmods_printf("\t No symbols\n");
        return ret;
    }

    stackmods_printf("\t using symbols:\n");
    _hasDebugSymbols = true;

    // Calculate base pointer height for locals; the frame offset is relative to this
    Architecture arch = ifunc()->isrc()->getArch();

    for (auto vIter = _vars.begin(); vIter != _vars.end(); ++vIter) {
        SymtabAPI::localVar* var = *vIter;
        vector<VariableLocation> locs = var->getLocationLists();
        if (locs.empty()) {
            continue;
        }

        ValidPCRange* valid = new ValidPCRange();

        bool found = false;
        long offset = 0;
        for (auto locIter = locs.begin(); locIter != locs.end(); ++locIter) {
            VariableLocation curLoc = *locIter;
            if (curLoc.mr_reg == MachRegister::getFramePointer(arch)) {
                found = true;
                valid->insert(curLoc.lowPC, curLoc.hiPC+1, 0);
                offset = curLoc.frameOffsetAbs;
            }
        }

        if (!found || !offset) {
            continue;
        }

        stackmods_printf("\t\t\t Found %s (type %s) @ %ld, size = %u\n", var->getName().c_str(), var->getType(Type::share)->getName().c_str(), offset, var->getType(Type::share)->getSize());
        for (auto locIter = locs.begin(); locIter != locs.end(); ++locIter) {
            VariableLocation curLoc = *locIter;
            if (curLoc.mr_reg == MachRegister::getFramePointer(arch)) {
                stackmods_printf("\t\t\t\t Range = [0x%lx, 0x%lx)\n", curLoc.lowPC, curLoc.hiPC);
            } else {
                stackmods_printf("\t\t\t\t SKIPPED Range = [0x%lx, 0x%lx)\n", curLoc.lowPC, curLoc.hiPC);
            }
        }

        StackAccess::StackAccessType sat = StackAccess::StackAccessType::DEBUGINFO_LOCAL;
        if (var->getType(Type::share)->getSize() == 0) {
            _tmpObjects->insert(tmpObject(offset, 4, sat, valid));
        } else {
            _tmpObjects->insert(tmpObject(offset, var->getType(Type::share)->getSize(), sat, valid));
        }
    }

    for (auto vIter = _params.begin(); vIter != _params.end(); ++vIter) {
        SymtabAPI::localVar* var = *vIter;
        vector<VariableLocation> locs = var->getLocationLists();
        if (locs.empty()) {
            continue;
        }

        ValidPCRange* valid = new ValidPCRange();

        bool found = false;
        long offset = 0;
        for (auto locIter = locs.begin(); locIter != locs.end(); ++locIter) {
            VariableLocation curLoc = *locIter;
            if (curLoc.mr_reg == MachRegister::getFramePointer(arch)) {
                found = true;
                valid->insert(curLoc.lowPC, curLoc.hiPC+1, 0);
                offset = curLoc.frameOffsetAbs;
            }
        }

        if (!found || !offset) {
            continue;
        }

        stackmods_printf("\t\t\t Found %s (type %s) @ %ld, size = %u\n", var->getName().c_str(), var->getType(Type::share)->getName().c_str(), offset, var->getType(Type::share)->getSize());

        for (auto locIter = locs.begin(); locIter != locs.end(); ++locIter) {
            VariableLocation curLoc = *locIter;
            if (curLoc.mr_reg == MachRegister::getFramePointer(arch)) {
                stackmods_printf("\t\t\t\t Range = [0x%lx, 0x%lx)\n", curLoc.lowPC, curLoc.hiPC);
            } else {
                stackmods_printf("\t\t\t\t SKIPPED Range = [0x%lx, 0x%lx)\n", curLoc.lowPC, curLoc.hiPC);
            }
        }

        StackAccess::StackAccessType sat = StackAccess::StackAccessType::DEBUGINFO_PARAM;
        if (var->getType(Type::share)->getSize() == 0) {
            _tmpObjects->insert(tmpObject(offset, 4, sat, valid));
        } else {
            _tmpObjects->insert(tmpObject(offset, var->getType(Type::share)->getSize(), sat, valid));
        }
    }

    return ret;
}


bool func_instance::createOffsetVector_Analysis(ParseAPI::Function *func,
                                                ParseAPI::Block *block,
                                                InstructionAPI::Instruction insn,
                                                Address addr)
{
    bool ret = true;

    stackmods_printf("\t Processing %lx: %s\n", addr, insn.format().c_str());

    int accessSize = getAccessSize(insn);
    Accesses* accesses = new Accesses();
    assert(accesses);
    std::set<Address> defAddrsToMod;
    if (::getAccesses(func, block, addr, insn, accesses, defAddrsToMod)) {
        _accessMap->insert(make_pair(addr, accesses));

        // Create entries in our definition map for any definitions that need
        // to be modified.  We will fill out the entries after we've collected
        // all the definitions.
        for (auto addrIter = defAddrsToMod.begin();
            addrIter != defAddrsToMod.end(); addrIter++) {
            const Address defAddr = *addrIter;
            if (_definitionMap->find(defAddr) == _definitionMap->end()) {
                (*_definitionMap)[defAddr] = NULL;
            }
        }

        for (auto accessIter = accesses->begin(); accessIter != accesses->end();
            ++accessIter) {
            const std::set<StackAccess *> &saSet = accessIter->second;
            for (auto saIter = saSet.begin(); saIter != saSet.end(); saIter++) {
                StackAccess *access = *saIter;

                stackmods_printf("\t\t Processing %s, size %d\n",
                    access->format().c_str(), accessSize);

                assert(!access->skipReg());
                if (!addToOffsetVector(access->regHeight(), 1,
                    StackAccess::StackAccessType::REGHEIGHT, true, NULL, access->reg())) {
                    stackmods_printf("\t\t\t INVALID: addToOffsetVector "
                        "failed\n");
                    return false;
                }
                _tmpObjects->insert(tmpObject(access->readHeight().height(),
                    accessSize, access->type()));
            }
        }
    } else {
        stackmods_printf("\t\t\t INVALID: getAccesses failed\n");
        delete accesses;
        // Once we've found a bad access, stop looking!
        return false;
    }

    return ret;
}

bool func_instance::addToOffsetVector(StackAnalysis::Height off, int size, StackAccess::StackAccessType type, bool isRegisterHeight, ValidPCRange* valid, MachRegister reg)
{
    bool ret = true;

    bool skip_misunderstood = false;

    if (isRegisterHeight) {
        StackLocation* existingLoc = NULL;
        if (_offVec->find(off, reg, existingLoc)) {
            if (existingLoc->isRegisterHeight() && existingLoc->reg() == reg) {
                return ret;
            } else {
                stackmods_printf("\t\t\t loc %s is not a match\n", existingLoc->format().c_str());
            }
        }

        stackmods_printf("\t\t\t addToOffsetVector: added %s %ld\n", StackAccess::printStackAccessType(type).c_str(), off.height());
        StackLocation* tmp = new StackLocation(off, type, reg);
        _offVec->insert(off, off+size, tmp, isRegisterHeight);
        return ret;
    }

    if (size == 0) {
        stackmods_printf("\t\t\t addToOffsetVector: trying to add non-StackAccess::REGHEIGHT of size 0. Skipping.\n");
        return ret;
    }

    bool found = false;
    stackmods_printf("\t\t\t Adding %ld, size %d, type %s\n", off.height(), size, StackAccess::printStackAccessType(type).c_str());

    // If the access is inside the expected space for the return address, declare INCONSISTENT
    Architecture arch = ifunc()->isrc()->getArch();
    int raLoc;
    if (arch == Arch_x86_64) {
        raLoc = -8;
    } else if (arch == Arch_x86) {
        raLoc = -4;
    } else {
        assert(0);
    }

    if ( ( (raLoc <= off.height()) && (off.height() < 0) ) ||
         ( (raLoc <  off.height() + size) && (off.height() + size <= 0) ) ||
         ( (off.height() < raLoc) && (off.height() + size > 0)) ) {
        stackmods_printf("\t\t\t\t This stack access interferes with the RA. We may be confused. Skipping.\n");
        if (isDebugType(type)) {
            // Silently skip; doesn't hurt--if there's an actual access, the others will catch it.
            // Okay to skip here because getAccesses won't return this one because it came from the DWARF.
            type = StackAccess::StackAccessType::MISUNDERSTOOD;
        } else {
            return false;
        }
    }

    StackAnalysis::Height lb, ub;
    StackLocation* existing;
    if (_offVec->find(off, lb, ub, existing)) {
        found = true;
        stackmods_printf("\t\t\t\t Found existing offset range %s\n", existing->format().c_str());
        if (lb == off) {
            int existingSize = existing->size();
            stackmods_printf("\t\t\t\t\t Existing has same lb.\n");
            if (size == existingSize) {
                stackmods_printf("\t\t\t\t\t Existing has same size. Skip adding.\n");
                // Merge intervals
                if (isDebugType(type)) {
                    for (auto vIter = valid->begin(); vIter != valid->end(); ++vIter) {
                        existing->valid()->insert((*vIter).first, (*vIter).second.first, (*vIter).second.second);
                    }
                }
            } else if (size < existingSize) {
                stackmods_printf("\t\t\t\t\t Existing has larger size. Skip adding.\n");
            } else {
                stackmods_printf("\t\t\t\t\t Existing has smaller size. Checking whether to update.\n");
                bool skip = false;

                // Iterate through update offsets to make sure no conflict that overlaps or is contained in the region
                for (int i = 0; i < size; i++) {

                    StackAnalysis::Height lb2, ub2;
                    StackLocation* existing2;
                    // Is there an existing range at the potential update size?
                    if (_offVec->find(off+i, lb2, ub2, existing2)) {

                        // If we find a different range at the potential update size, skip!
                        if (existing2 != existing) {
                            stackmods_printf("\t\t\t\t\t\t\t\t WARNING: updating size would conflict with different existing range.\n");
                            skip = true;

                            if (skip_misunderstood) {
                                ret = false;
                                break;
                            } else {
                                // We know the new one overlaps with existing
                                existing->setType(StackAccess::StackAccessType::MISUNDERSTOOD);

                                // But, it also overlaps with existing2
                                existing2->setType(StackAccess::StackAccessType::MISUNDERSTOOD);

                                StackLocation* tmp = new StackLocation(off, size, StackAccess::StackAccessType::MISUNDERSTOOD, isRegisterHeight, valid);
                                assert(tmp);
                                _offVec->insert(off, off+size, tmp, isRegisterHeight);
                            }
                        }
                    }
                }

                // Safe to update existing to a new larger size!
                if (!skip) {
                    stackmods_printf("\t\t\t\t\t\t Updating size to %d. Added\n", size);
                    existing->setSize(size);
                    _offVec->update(existing->off(), existing->off() + existing->size());
                }
            }
        } else if (lb < off && off < ub) {
            stackmods_printf("\t\t\t\t Found overlapping offset range (lb < off < ub): %s\n", existing->format().c_str());

            int existingUpperBound = existing->off().height() + existing->size();
            int newUpperBound = off.height() + size;

            if (existingUpperBound == newUpperBound) {
                stackmods_printf("\t\t\t\t\t Existing and new upper bounds are the same. Skip adding.\n");
            } else if (existingUpperBound > newUpperBound) {
                stackmods_printf("\t\t\t\t\t Existing upper bound is greater than what we're adding. Skip adding.\n");
            } else {
                stackmods_printf("\t\t\t\t\t WARNING: conflict because existing upper bound is less than what we're adding.\n");

                // Goal: figure out what other range(s) we're in conflict with
                if (skip_misunderstood) {
                    ret = false;
                } else {
                    for (int i = 0; i < size; i++) {
                        StackAnalysis::Height lb2, ub2;
                        StackLocation* existing2;
                        if (_offVec->find(off+i, lb2, ub2, existing2)) {
                            if (existing2 != existing) {
                                stackmods_printf("\t\t\t\t\t\t Range overlaps with another existing range %s\n", existing2->format().c_str());
                                existing2->setType(StackAccess::StackAccessType::MISUNDERSTOOD);
                            }
                        }
                    }

                    // We know we're in conflict with the existing range, since lb != off
                    existing->setType(StackAccess::StackAccessType::MISUNDERSTOOD);

                    // Add new range as misunderstood
                    StackLocation* tmp = new StackLocation(off, size, StackAccess::StackAccessType::MISUNDERSTOOD, isRegisterHeight, valid);
                    assert(tmp);
                    _offVec->insert(off, off+size, tmp, isRegisterHeight);
                }
            }
        } else if (off == ub) {
            // Not a match! Adjacent ranges. Continue searching.
            found = false;
        } else {
            assert(0);
        }
    }

    if (!found) {
            stackmods_printf("\t\t\t\t ... created new\n");
            StackLocation* tmp = new StackLocation(off, size, type, isRegisterHeight, valid);
            assert(tmp);
            _offVec->insert(off, off+size, tmp, isRegisterHeight);
    }

    return ret;
}

void func_instance::createTMap_internal(StackMod* mod, StackLocation* loc, TMap* tMap)
{
    StackAnalysis::Height off = loc->off();
    switch(mod->type()) {
        case(StackMod::INSERT): {
                          /* Model:
                           * o < d: o' = o + (c-d)
                           */

                          Insert* insertMod = dynamic_cast<Insert*>(mod);
                          StackAnalysis::Height c(insertMod->low());
                          StackAnalysis::Height d(insertMod->high());

                          if (off < d || off == d) {
                              stackmods_printf("\t\t Processing interaction with %s\n", loc->format().c_str());
                              int delta = c.height() - d.height();
                              tMap->update(loc, delta);
                          }
                          break;
                      }
        case(StackMod::REMOVE): {
                          /* Model:
                           * O' = O - [c,d)
                           * o < d: o' = o - (c-d)
                           */

                          Remove* removeMod = dynamic_cast<Remove*>(mod);
                          StackAnalysis::Height c(removeMod->low());
                          StackAnalysis::Height d(removeMod->high());

                          // Remove existing element
                          if ((c == off || c < off) &&
                                  (off < c + (d-c))) {
                              stackmods_printf("\t\t Processing interaction with %s\n", loc->format().c_str());
                              StackLocation* tmp = new StackLocation();
                              tMap->insert(make_pair(loc, tmp));
                              stackmods_printf("\t\t Adding to tMap %s -> %s\n", loc->format().c_str(), tmp->format().c_str());
                          }

                          // Shift other elements
                          if (off < d) {
                              stackmods_printf("\t\t Processing interaction with %s\n", loc->format().c_str());
                              int delta = d.height() - c.height();
                              tMap->update(loc, delta);
                          }


                          break;
                      }
        case(StackMod::MOVE): {
                        /* Model:
                         * o in [c,d): o' = o + (m-c)
                         */

                        Move* moveMod = dynamic_cast<Move*>(mod);
                        StackAnalysis::Height c(moveMod->srcLow());
                        int size = moveMod->size();

                        // Future: could check the DWARF and use the vailD PC range if we have one
                        ValidPCRange* valid = NULL;
                        StackLocation* found = NULL;
                        _offVec->find(c, found);
                        if (found) {
                            valid = found->valid();
                        }

                        StackAnalysis::Height m(moveMod->destLow());

                        if (    (c == off || c < off) &&
                                (off < c+size) ) {
                            stackmods_printf("\t\t Processing interaction with %s\n", loc->format().c_str());
                            // This does not go through the updateTMap because it's a fixed change
                            StackLocation* src;
                            StackLocation* tmp;
                            if (loc->isRegisterHeight()) {
                                src = new StackLocation(loc->off(), loc->type(), loc->reg(), valid);
                                tmp = new StackLocation(off + (m-c), loc->type(), loc->reg(), valid);
                            } else {
                                src = loc;
                                tmp = new StackLocation(off + (m-c), size-(off.height()-c.height()), loc->type(), false, loc->valid());
                            }
                            auto res = tMap->insert(make_pair(src, tmp));
                            stackmods_printf("\t\t\t Adding to tMap: %s -> %s\n", loc->format().c_str(), tmp->format().c_str());
                            if (res.second) {
                                stackmods_printf("\t\t\t\t Added.\n");
                            } else {
                                stackmods_printf("\t\t\t\t Not added. Found  %s -> %s already existed\n", res.first->first->format().c_str(), res.first->second->format().c_str());
                            }
                            // Update size
                            loc->setSize(size - (off.height() - c.height()));
                        }
                        break;
                    }
        default:
                    assert(0 && "unknown modification type");
    }
}

void func_instance::createTMap_internal(StackMod* mod, TMap* tMap)
{

    stackmods_printf("\t Processing %s\n", mod->format().c_str());

    // Handle insert/add side effects
    if (mod->type() == StackMod::INSERT) {
        Insert* insertMod = dynamic_cast<Insert*>(mod);
        StackAnalysis::Height c(insertMod->low());
        StackAnalysis::Height d(insertMod->high());
        StackLocation* tmpSrc = new StackLocation();
        StackLocation* tmpDest = new StackLocation(c, (d-c).height(), StackAccess::StackAccessType::UNKNOWN, false);
        tMap->insert(make_pair(tmpSrc, tmpDest));
        stackmods_printf("\t\t\t Adding to tMap: %s -> %s\n", tmpSrc->format().c_str(), tmpDest->format().c_str());
    }

    if (!_offVec->stack().empty()) {
        IntervalTree<OffsetVector::K,OffsetVector::V> stack = _offVec->stack();
        for (auto offIter = stack.begin(); offIter != stack.end(); ++offIter) {
            StackLocation* loc = (*offIter).second.second;
            createTMap_internal(mod, loc, tMap);
        }
    }
    if (!_offVec->definedRegs().empty()) {
        std::map<MachRegister,IntervalTree<OffsetVector::K,OffsetVector::V> > definedRegs = _offVec->definedRegs();
        for (auto offIter = definedRegs.begin(); offIter != definedRegs.end(); ++offIter) {
            for (auto iter2 = (*offIter).second.begin(); iter2 != (*offIter).second.end(); ++iter2) {
                createTMap_internal(mod, (*iter2).second.second, tMap);
            }
        }
    }
}

namespace
{
AnnotationClass<StackAnalysis::Intervals>
        Stack_Anno_Intervals(std::string("Stack_Anno_Intervals"), NULL);
AnnotationClass<StackAnalysis::BlockEffects>
        Stack_Anno_Block_Effects(std::string("Stack_Anno_Block_Effects"), NULL);
AnnotationClass<StackAnalysis::InstructionEffects>
        Stack_Anno_Insn_Effects(std::string("Stack_Anno_Insn_Effects"), NULL);
AnnotationClass<StackAnalysis::CallEffects>
        Stack_Anno_Call_Effects(std::string("Stack_Anno_Call_Effects"), NULL);
}
void func_instance::freeStackMod() {
    // Free stack analysis intervals
    StackAnalysis::Intervals *i = NULL;
    ifunc()->getAnnotation(i, Stack_Anno_Intervals);
    ifunc()->removeAnnotation(Stack_Anno_Intervals);
    if (i != NULL) delete i;

    // Free stack analysis block effects
    StackAnalysis::BlockEffects *be = NULL;
    ifunc()->getAnnotation(be, Stack_Anno_Block_Effects);
    ifunc()->removeAnnotation(Stack_Anno_Block_Effects);
    if (be != NULL) delete be;

    // Free stack analysis instruction effects
    StackAnalysis::InstructionEffects *ie = NULL;
    ifunc()->getAnnotation(ie, Stack_Anno_Insn_Effects);
    ifunc()->removeAnnotation(Stack_Anno_Insn_Effects);
    if (ie != NULL) delete ie;

    // Free stack analysis call effects
    StackAnalysis::CallEffects *ce = NULL;
    ifunc()->getAnnotation(ce, Stack_Anno_Call_Effects);
    ifunc()->removeAnnotation(Stack_Anno_Call_Effects);
    if (ce != NULL) delete ce;
}
#endif
