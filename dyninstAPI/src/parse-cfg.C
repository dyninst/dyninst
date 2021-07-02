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
 
// $Id: parse-cfg.C,v 1.60 2008/11/03 15:19:24 jaw Exp $

#include "function.h"
#include "instPoint.h"

#include "instructionAPI/h/InstructionDecoder.h"

#include "image.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

int parse_func_count = 0;


const char * image_edge::getTypeString()
{
    switch(type()) {
        case CALL:
            return "CALL";
            break;
        case COND_TAKEN:
            return "COND BRANCH - TAKEN";
            break;
        case COND_NOT_TAKEN:
            return "COND BRANCH - NOT TAKEN";
            break;
        case INDIRECT:
            return "INDIRECT BRANCH";
            break;
        case DIRECT:
            return "UNCOND BRANCH";
            break;
        case FALLTHROUGH:
            return "FALLTHROUGH";
            break;
        case CATCH:
            return "CATCH";
            break;
        case CALL_FT:
            return "POST-CALL FALLTHROUGH";
            break;
        case RET:
            return "RETURN";
            break;
        default:
            return "ERROR UNKNOWN";
            break;
    }
}

parse_func::parse_func(
    SymtabAPI::Function *func, 
    pdmodule *m, 
    image *i, 
    CodeObject * obj,
    CodeRegion * reg,
    InstructionSource * isrc,
    FuncSource src):
  Function(func->getOffset(),func->getFirstSymbol()->getMangledName(),obj,reg,isrc),
  func_(func),
  mod_(m),
  image_(i),
  OMPparsed_(false),
  usedRegisters(NULL),
  containsFPRWrites_(unknown),
  containsSPRWrites_(unknown),
  containsSharedBlocks_(false),
  hasWeirdInsns_(false),
  prevBlocksUnresolvedCF_(0),
  unresolvedCF_(UNSET_CF),
  init_retstatus_(UNSET),
  o7_live(false),
  saves_return_addr_(false),
  livenessCalculated_(false),
  isPLTFunction_(false),
  containsPowerPreamble_(false),
  noPowerPreambleFunc_(NULL)
{
#if defined(ROUGH_MEMORY_PROFILE)
    parse_func_count++;
    if ((parse_func_count % 100) == 0)
        fprintf(stderr, "parse_func_count: %d (%d)\n",
                parse_func_count, parse_func_count*sizeof(parse_func));
#endif
    _src = src;
    func->setData(this);
}	


parse_func::~parse_func() 
{
    /* FIXME */ 
  mal_printf("~image_func() for func at %lx\n",_start);
  delete usedRegisters;
}

bool parse_func::addSymTabName(std::string name, bool isPrimary) 
{
    if(func_->addMangledName(name.c_str(), isPrimary)){
	return true;
    }

    return false;
}

bool parse_func::addPrettyName(std::string name, bool isPrimary) {
   if (func_->addPrettyName(name.c_str(), isPrimary)) {
      return true;
   }
   
   return false;
}

bool parse_func::addTypedName(std::string name, bool isPrimary) {
    // Count this as a pretty name in function lookup...
    if (func_->addTypedName(name.c_str(), isPrimary)) {
	return true;
    }

    return false;
}

void parse_func::changeModule(pdmodule *mod) {
  // Called from buildFunctionLists, so we aren't entered in any 
  // module-level data structures. If this changes, UPDATE THE
  // FUNCTION.
  mod_ = mod;
}

bool parse_func::isInstrumentableByFunctionName()
{
    // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
    // they can't be instrumented or we would have an infinite loop
    if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
        || prettyName() == "GetProcessTimes")
        return false;
    return true;
}

Address parse_func::getEndOffset() {
    if (!parsed()) image_->analyzeIfNeeded();
    if(blocks().empty()) {
        fprintf(stderr,"error: end offset requested for empty function\n");
        return addr();
    } else {
        return extents().back()->end();
    }
}


const std::vector<image_parRegion *> &parse_func::parRegions() {
  if (!parsed()) image_->analyzeIfNeeded();
  return parRegionsList;
}

bool parse_func::isPLTFunction() {
    return obj()->cs()->linkage().find(addr()) !=
           obj()->cs()->linkage().end();
}

int parse_block_count = 0;

/*
 * For CFGFactory::mksink only 
 */
parse_block::parse_block(
        CodeObject * obj, 
        CodeRegion * reg,
        Address addr) :
    Block(obj,reg,addr),
    needsRelocation_(false),
    blockNumber_(0),
    unresolvedCF_(false),
    abruptEnd_(false)
{
     
}

parse_block::parse_block(
        parse_func * func, 
        CodeRegion * reg,
        Address firstOffset) :
    Block(func->obj(),reg,firstOffset, func),
    needsRelocation_(false),
    blockNumber_(0),
    unresolvedCF_(false),
    abruptEnd_(false)
{ 
    // basic block IDs are unique within images.
    blockNumber_ = func->img()->getNextBlockID();
#if defined(ROUGH_MEMORY_PROFILE)
    parse_block_count++;
    if ((parse_block_count % 100) == 0)
        fprintf(stderr, "parse_block_count: %d (%d)\n",
                parse_block_count, parse_block_count*sizeof(parse_block));
#endif
}

parse_block::~parse_block() {

}


void parse_block::debugPrint() {
   // no looping if we're not printing anything
    if(!dyn_debug_parsing)
        return;

    parsing_printf("Block %d: starts 0x%lx, last 0x%lx, end 0x%lx\n",
                   blockNumber_,
                   start(),
                   lastInsnAddr(),
                   end());

    parsing_printf("  Sources:\n");
    const Block::edgelist & srcs = sources();
    Block::edgelist::const_iterator sit = srcs.begin();
    unsigned s = 0;
    for ( ; sit != srcs.end(); ++sit) {
        parse_block * src = static_cast<parse_block*>((*sit)->src());
        parsing_printf("    %u: block %d (%s)\n",
                       s, src->blockNumber_,
                       static_cast<image_edge*>(*sit)->getTypeString());
        ++s;
    }
    parsing_printf("  Targets:\n");
    const Block::edgelist & trgs = sources();
    Block::edgelist::const_iterator tit = trgs.begin();
    unsigned t = 0;
    for( ; tit != trgs.end(); ++tit) {
        parse_block * trg = static_cast<parse_block*>((*tit)->trg());
        parsing_printf("    %u: block %d (%s)\n",
                       t, trg->blockNumber_,
                       static_cast<image_edge*>(*tit)->getTypeString());
        ++t;
    }
}

void *parse_block::getPtrToInstruction(Address addr) const {
    if (addr < start()) return NULL;
    if (addr >= end()) return NULL;
    // XXX all potential parent functions have the same image
    return region()->getPtrToInstruction(addr);
}

/* Returns NULL if the address is not within a block belonging to this function.
   Why do we even bother returning NULL if the address is outside of this
   function? FIXME check whether we can do away with that.
*/
void *parse_func::getPtrToInstruction(Address addr) const {
    // The commented-out code checks whether the address is within
    // the bytes of this function (one of its basic blocks). Instead,
    // we do a fast path and just pass the request through to the image.
    // The old tests are preserved for posterity.
    /*
    if (addr < getOffset()) return NULL;

    // XXX this call may modify the current function, so const semantics
    //     are not actually perserved
    set<Function *> stab;
    img()->findFuncs(addr,stab);

    if(!stab.empty()) {
        set<Function*>::iterator fit = stab.begin();
        for( ; fit != stab.end(); ++fit) {
            if(*fit == this)
                return obj().getPtrToInstruction(addr);
        }
    }
    return NULL;
    */
    return isrc()->getPtrToInstruction(addr);
}

bool parse_block::isEntryBlock(parse_func * f) const
{
    return f->entryBlock() == this;
} 

/*
 * True if the block has a return edge, or a call that does
 * not return (i.e., a tail call or non-returning call)
 */
bool parse_block::isExitBlock()
{
    const Block::edgelist & trgs = targets();
    if(trgs.empty()) {
        return false;
    }

    Edge * e = *trgs.begin();
    if (e->type() == RET) {
        return true;
    }

    if (!e->interproc()) {
        return false;
    }

    if (e->type() == CALL && trgs.size() > 1) {
        // there's a CALL edge and at least one other edge, 
        // it's an exit block if there is no CALL_FT edge
        for(Block::edgelist::const_iterator eit = ++(trgs.begin());
            eit != trgs.end();
            eit++)
        {
            if ((*eit)->type() == CALL_FT && !(*eit)->sinkEdge()) {
                return false;
            }
        }
    }
    return true;
}

bool parse_block::isCallBlock()
{
    const Block::edgelist & trgs = targets();
    if(!trgs.empty())
    {
        for (Block::edgelist::const_iterator eit = trgs.begin();
             eit != trgs.end();
             eit++) 
        {
            if ((*eit)->type() == CALL) {
                return true;
            }
        }
    }
    return false;
}
bool parse_block::isIndirectTailCallBlock()
{
    const Block::edgelist & trgs = targets();
    if(!trgs.empty())
    {
        for (Block::edgelist::const_iterator eit = trgs.begin();
             eit != trgs.end();
             eit++) 
        {
            if ((*eit)->type() == INDIRECT && (*eit)->interproc()) {
                return true;
            }
        }
    }
    return false;
}
image *parse_block::img()
{
    vector<Function*> funcs;
    getFuncs(funcs);
    return static_cast<parse_func*>(funcs[0])->img();
}

parse_func *parse_block::getEntryFunc() const {
    parse_func *ret =
        static_cast<parse_func*>(obj()->findFuncByEntry(region(),start()));

    // sanity check
    if(ret && ret->entryBlock() != this) {
        parsing_printf("[%s:%d] anomaly: block [%lx,%lx) is not entry for "
                       "func at %lx\n",
            FILE__,__LINE__,start(),end(),ret->addr());
    }
    return ret;
}

parse_block * parse_func::entryBlock() { 
    if (!parsed()) image_->analyzeIfNeeded();
    return static_cast<parse_block*>(entry());
}

bool parse_func::isLeafFunc() {
    if (!parsed())
        image_->analyzeIfNeeded();

    return !callEdges().empty();
}

void parse_func::addParRegion(Address begin, Address end, parRegType t)
{
    image_parRegion * iPar = new image_parRegion(begin, this);
    iPar->setRegionType(t);
    iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
    iPar->setLastInsn(end);
    parRegionsList.push_back(iPar);
}

void parse_block::getInsns(Insns &insns, Address base) {
   using namespace InstructionAPI;
   Offset off = firstInsnOffset();
   const unsigned char *ptr = (const unsigned char *)getPtrToInstruction(off);
   if (ptr == NULL) return;

   InstructionDecoder d(ptr, getSize(),obj()->cs()->getArch());

   while (off < endOffset()) {
      Instruction insn = d.decode();

      insns[off + base] = insn;
      off += insn.size();
   }
}


/* This function is static.
 *
 * Find the blocks that are reachable from the seed blocks 
 * if the except blocks are not part of the CFG
 */
void parse_func::getReachableBlocks
(const std::set<parse_block*> &exceptBlocks,
 const std::list<parse_block*> &seedBlocks,
 std::set<parse_block*> &reachBlocks)
{
    using namespace ParseAPI;
    mal_printf("reachable blocks for func %lx from %lu start blocks\n",
               addr(), seedBlocks.size());

    // init visited set with seed and except blocks
    std::set<parse_block*> visited;
    visited.insert(exceptBlocks.begin(), exceptBlocks.end());
    visited.insert(seedBlocks.begin(), seedBlocks.end());

    // add seed blocks to the worklist (unless the seed is in exceptBlocks)
    std::list<parse_block*> worklist;
    for (list<parse_block*>::const_iterator sit = seedBlocks.begin();
         sit != seedBlocks.end();
         sit++) 
    {
        visited.insert(*sit);
        if (exceptBlocks.end() == exceptBlocks.find(*sit)) {
            worklist.push_back(*sit);
            reachBlocks.insert(*sit);
        }
    }
        
    // iterate through worklist, adding all blocks (except for
    // seedBlocks) that are reachable through target edges to the
    // reachBlocks set
    while(worklist.size()) {
        parse_block *curBlock = worklist.front();
        const Block::edgelist & outEdges = curBlock->targets();
        Block::edgelist::const_iterator tIter = outEdges.begin();
        for (; tIter != outEdges.end(); tIter++) {
            parse_block *targB = (parse_block*) (*tIter)->trg();
            if ( CALL != (*tIter)->type() &&
                 false == (*tIter)->sinkEdge() &&
                 visited.end() == visited.find(targB) )
                 //reachBlocks.end() == reachBlocks.find(targB) &&
                 //exceptBlocks.end() == exceptBlocks.find(targB) &&
                 //seedBlocks.end() == seedBlocks.find(targB) )
            {
                worklist.push_back(targB);
                reachBlocks.insert(targB);
                visited.insert(targB);
                mal_printf("block [%lx %lx] is reachable\n",
                           targB->firstInsnOffset(),
                           targB->endOffset());
            }
        }
        worklist.pop_front();
    } 
}
void parse_func::setinit_retstatus(ParseAPI::FuncReturnStatus rs)
{
    init_retstatus_ = rs;
    if (rs > retstatus()) {
        set_retstatus(rs);
    }
}
ParseAPI::FuncReturnStatus parse_func::init_retstatus() const
{
    if (UNSET == init_retstatus_) {
        assert(!obj()->defensiveMode()); // should have been set for defensive binaries
        return retstatus();
    }
    if (init_retstatus_ > retstatus()) {
        return retstatus();
    }
    return init_retstatus_;
}

void parse_func::setHasWeirdInsns(bool wi)
{
   hasWeirdInsns_ = wi;
}

void parse_block::setUnresolvedCF(bool newVal) 
{ 
   unresolvedCF_ = newVal;
}

parse_func *parse_block::getCallee() {
   for (edgelist::const_iterator iter = targets().begin(); iter != targets().end(); ++iter) {
      if ((*iter)->type() == ParseAPI::CALL) {
         parse_block *t = static_cast<parse_block *>((*iter)->trg());
         return t->getEntryFunc();
      }
   }
   return NULL;
}

std::pair<bool, Address> parse_block::callTarget() {
   using namespace InstructionAPI;
   Offset off = lastInsnOffset();
   const unsigned char *ptr = (const unsigned char *)getPtrToInstruction(off);
   if (ptr == NULL) return std::make_pair(false, 0);
   InstructionDecoder d(ptr, endOffset() - lastInsnOffset(), obj()->cs()->getArch());
   Instruction insn = d.decode();

   // Bind PC to that insn
   // We should build a free function to do this...
   
   Expression::Ptr cft = insn.getControlFlowTarget();
   if (cft) {
      Expression::Ptr pc(new RegisterAST(MachRegister::getPC(obj()->cs()->getArch())));
      cft->bind(pc.get(), Result(u64, lastInsnAddr()));
      Result res = cft->eval();
      if (!res.defined) return std::make_pair(false, 0);
   
      return std::make_pair(true, res.convert<Address>());
   }
   return std::make_pair(false, 0);
}

bool parse_func::hasUnresolvedCF() {
   if (unresolvedCF_ == UNSET_CF) {
      for (blocklist::iterator iter = blocks().begin();
           iter != blocks().end(); ++iter) {
         for (Block::edgelist::const_iterator iter2 = (*iter)->targets().begin();
              iter2 != (*iter)->targets().end(); ++iter2) {
	   if ((*iter2)->sinkEdge())
	   {
	     if ((*iter2)->interproc()) {
	       continue;
	     }
	     if (((*iter2)->type() == ParseAPI::INDIRECT) ||
		 ((*iter2)->type() == ParseAPI::DIRECT))
	     {
	       unresolvedCF_ = HAS_UNRESOLVED_CF;
	       break;
	     }
	   }
         }
         if (unresolvedCF_ == HAS_UNRESOLVED_CF) break;
      }
      if (unresolvedCF_ == UNSET_CF)
         unresolvedCF_ = NO_UNRESOLVED_CF;
   }
   return (unresolvedCF_ == HAS_UNRESOLVED_CF);
}

bool parse_func::isInstrumentable() {
   if(!isInstrumentableByFunctionName() || img()->isRelocatableObj())
      return false;
   else {
      // Create instrumentation points for non-plt functions 
      if(obj()->cs()->linkage().find(getOffset()) != obj()->cs()->linkage().end()) { 
          return false;
      }
    }

   if (hasUnresolvedCF()) {
      return false;
   }
   return true;
}
