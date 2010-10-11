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
 
// $Id: image-func.C,v 1.60 2008/11/03 15:19:24 jaw Exp $

#include "function.h"
#include "instPoint.h"

#if defined(cap_instruction_api)
#include "instructionAPI/h/InstructionDecoder.h"
#endif //defined(cap_instruction_api)

#include "symtab.h"
#include "debug.h"
#include "common/h/singleton_object_pool.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

int image_func_count = 0;

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

image_func::image_func(
    SymtabAPI::Function *func, 
    pdmodule *m, 
    image *i, 
    CodeObject * obj,
    CodeRegion * reg,
    InstructionSource * isrc,
    FuncSource src):
  Function(func->getOffset(),func->getFirstSymbol()->getName(),obj,reg,isrc),
  func_(func),
  mod_(m),
  image_(i),
  usedRegisters(NULL),
  containsFPRWrites_(unknown),
  containsSPRWrites_(unknown),
  containsSharedBlocks_(false),
  instLevel_(NORMAL),
  canBeRelocated_(true),
  init_retstatus_(UNSET),
  o7_live(false),
  ppc_saves_return_addr_(false)
#if defined(cap_liveness)
  , livenessCalculated_(false)
#endif
  ,isPLTFunction_(false)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_func_count++;
    if ((image_func_count % 100) == 0)
        fprintf(stderr, "image_func_count: %d (%d)\n",
                image_func_count, image_func_count*sizeof(image_func));
#endif
    _src = src;
    extern AnnotationClass<image_func> ImageFuncUpPtrAnno;
    if (!func_->addAnnotation(this, ImageFuncUpPtrAnno))
    {
       fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
    }
}	


image_func::~image_func() 
{
    /* FIXME */ 
  fprintf(stderr,"SCREAMING FIT IN UNIMPL ~image_func()\n");
  delete usedRegisters;
}

bool image_func::addSymTabName(std::string name, bool isPrimary) 
{
    if(func_->addMangledName(name.c_str(), isPrimary)){
	return true;
    }

    return false;
}

bool image_func::addPrettyName(std::string name, bool isPrimary) {
   if (func_->addPrettyName(name.c_str(), isPrimary)) {
      return true;
   }
   
   return false;
}

bool image_func::addTypedName(std::string name, bool isPrimary) {
    // Count this as a pretty name in function lookup...
    if (func_->addTypedName(name.c_str(), isPrimary)) {
	return true;
    }

    return false;
}

void image_func::changeModule(pdmodule *mod) {
  // Called from buildFunctionLists, so we aren't entered in any 
  // module-level data structures. If this changes, UPDATE THE
  // FUNCTION.
  mod_ = mod;
}

bool image_func::isInstrumentableByFunctionName()
{
#if defined(i386_unknown_solaris2_5)
    /* On Solaris, this function is called when a signal handler
       returns.  If it requires trap-based instrumentation, it can foul
       the handler return mechanism.  So, better exclude it.  */
    if (prettyName() == "_setcontext" || prettyName() == "setcontext")
        return false;
#endif /* i386_unknown_solaris2_5 */
    
    // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
    // they can't be instrumented or we would have an infinite loop
    if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
        || prettyName() == "GetProcessTimes")
        return false;
    return true;
}

Address image_func::getEndOffset() {
    if (!parsed()) image_->analyzeIfNeeded();
    if(blocks().empty()) {
        fprintf(stderr,"error: end offset requested for empty function\n");
        return addr();
    } else {
        return extents().back()->end();
    }
}


const pdvector<image_parRegion *> &image_func::parRegions() {
  if (!parsed()) image_->analyzeIfNeeded();
  return parRegionsList;
}

bool image_func::isPLTFunction() {
    return obj()->cs()->linkage().find(addr()) !=
           obj()->cs()->linkage().end();
}

int image_basicBlock_count = 0;

/*
 * For CFGFactory::mksink only 
 */
image_basicBlock::image_basicBlock(
        CodeObject * obj, 
        CodeRegion * reg,
        Address addr) :
    Block(obj,reg,addr),
    needsRelocation_(false),
    canBeRelocated_(false)
{
     
}

image_basicBlock::image_basicBlock(
        image_func * func, 
        CodeRegion * reg,
        Address firstOffset) :
    Block(func->obj(),reg,firstOffset),
    needsRelocation_(false),
    canBeRelocated_(true)
{ 
    // basic block IDs are unique within images.
    blockNumber_ = func->img()->getNextBlockID();
#if defined(ROUGH_MEMORY_PROFILE)
    image_basicBlock_count++;
    if ((image_basicBlock_count % 100) == 0)
        fprintf(stderr, "image_basicBlock_count: %d (%d)\n",
                image_basicBlock_count, image_basicBlock_count*sizeof(image_basicBlock));
#endif
}

int image_instPoint_count = 0;

image_instPoint::image_instPoint(Address offset,
                                 unsigned char * insn_buf,
                                 size_t insn_len,
                                 image * img,
                                 instPointType_t type) :
    instPointBase(insn_buf,
                  insn_len,
                  type),
    offset_(offset),
    image_(img),
    callee_(NULL),
    callTarget_(0),
    isUnres_(false),
    isDynamic_(0)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_instPoint_count++;
    if ((image_instPoint_count % 100) == 0)
        fprintf(stderr, "image_instPoint_count: %d (%d)\n",
                image_instPoint_count,
                image_instPoint_count*sizeof(image_instPoint));
#endif
}

image_instPoint::image_instPoint(Address offset,
                                 unsigned char * insn_buf,
                                 size_t insn_len,
                                 image * img,
                                 Address callTarget,
                                 bool isDynamic,
                                 bool isAbsolute,
                                 instPointType_t ptType, 
                                 bool isUnresolved) :
    instPointBase(insn_buf,
                  insn_len,
                  ptType),
    offset_(offset),
    image_(img),
    callee_(reinterpret_cast<image_func*>(-1)),
    callTarget_(callTarget),
    targetIsAbsolute_(isAbsolute),
    isUnres_(isUnresolved),
    isDynamic_(isDynamic)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_instPoint_count++;
    if ((image_instPoint_count % 100) == 0)
        fprintf(stderr, "image_instPoint_count: %d (%d)\n",
                image_instPoint_count, image_instPoint_count * sizeof(image_instPoint));
#endif
    if (isDynamic_)
        assert(callTarget_ == 0);
}

void image_basicBlock::debugPrint() {
    // no looping if we're not printing anything
    if(!dyn_debug_parsing)
        return;

    parsing_printf("Block %d: starts 0x%lx, last 0x%lx, end 0x%lx\n",
                   blockNumber_,
                   start(),
                   lastInsnAddr(),
                   end());

    parsing_printf("  Sources:\n");
    Block::edgelist & srcs = sources();
    Block::edgelist::iterator sit = srcs.begin();
    unsigned s = 0;
    for ( ; sit != srcs.end(); ++sit) {
        image_basicBlock * src = static_cast<image_basicBlock*>((*sit)->src());
        parsing_printf("    %d: block %d (%s)\n",
                       s, src->blockNumber_,
                       static_cast<image_edge*>(*sit)->getTypeString());
        ++s;
    }
    parsing_printf("  Targets:\n");
    Block::edgelist & trgs = sources();
    Block::edgelist::iterator tit = trgs.begin();
    unsigned t = 0;
    for( ; tit != trgs.end(); ++tit) {
        image_basicBlock * trg = static_cast<image_basicBlock*>((*tit)->trg());
        parsing_printf("    %d: block %d (%s)\n",
                       t, trg->blockNumber_,
                       static_cast<image_edge*>(*tit)->getTypeString());
        ++t;
    }
}

image_func*
image_instPoint::getCallee() const
{
    if(getPointType() != callSite)
        return NULL;
    if(callee_ != reinterpret_cast<image_func*>(-1))
        return callee_;

    callee_ = NULL;

    if(callTarget_) {
        callee_ = image_->findFuncByEntry(callTarget_);
        if(callee_) {
            parsing_printf("[%s:%d] bound call instpoint %lx to %lx\n",
                FILE__,__LINE__,offset_,callee_->addr());
        }
    } 
    return callee_;
}

// merge any otherP information that was unset in this point, we need 
// to do this since there can only be one point at a given address and
// we create entryPoints and exit points without initializing CF info
void image_instPoint::mergePoint(image_instPoint *otherP)
{
    isUnres_ = isUnres_ || otherP->isUnresolved();
    isDynamic_ = isDynamic_ || otherP->isDynamic();
    targetIsAbsolute_ = targetIsAbsolute_ || otherP->targetIsAbsolute();

    if (!getCallee() && otherP->getCallee()) {
        setCallee(otherP->getCallee());
        setCalleeName(otherP->getCalleeName());
    }
    if (!callTarget() && otherP->callTarget()) {
        callTarget_ = otherP->callTarget();
    }
}

void *image_basicBlock::getPtrToInstruction(Address addr) const {
    if (addr < start()) return NULL;
    if (addr >= end()) return NULL;
    // XXX all potential parent functions have the same image
    return region()->getPtrToInstruction(addr);
}

/* Returns NULL if the address is not within a block belonging to this function.
   Why do we even bother returning NULL if the address is outside of this
   function? FIXME check whether we can do away with that.
*/
void *image_func::getPtrToInstruction(Address addr) const {
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

bool image_basicBlock::isEntryBlock(image_func * f) const
{
    return f->entryBlock() == this;
} 

/*
 * All edges of an exit block are the same: returns
 * or interprocedural branches
 */
bool image_basicBlock::isExitBlock()
{
    Block::edgelist & trgs = targets();
    if(!trgs.empty())
    {
        Edge * e = *trgs.begin();
        return e->interproc() || e->type() == RET;
    }
    return false;
}

image *image_basicBlock::img()
{
    vector<Function*> funcs;
    getFuncs(funcs);
    return static_cast<image_func*>(funcs[0])->img();
}

image_func *image_basicBlock::getEntryFunc() const {
    image_func *ret =
        static_cast<image_func*>(obj()->findFuncByEntry(region(),start()));

    // sanity check
    if(ret && ret->entryBlock() != this) {
        parsing_printf("[%s:%d] anomaly: block [%lx,%lx) is not entry for "
                       "func at %lx\n",
            FILE__,__LINE__,start(),end(),ret->addr());
    }
    return ret;
}

image_basicBlock * image_func::entryBlock() { 
    if (!parsed()) image_->analyzeIfNeeded();
    return static_cast<image_basicBlock*>(entry());
}

bool image_func::isLeafFunc() {
    if (!parsed())
        image_->analyzeIfNeeded();

    return !callEdges().empty();
}

void image_func::addParRegion(Address begin, Address end, parRegType t)
{
    image_parRegion * iPar = new image_parRegion(begin, this);
    iPar->setRegionType(t);
    iPar->setParentFunc(this); // when not outlined, parent func will be same as regular
    iPar->setLastInsn(end);
    parRegionsList.push_back(iPar);
}

/*
 * Instpoint queries -- instpoints are stored at the image level
 */
void 
image_func::funcEntries(pdvector<image_instPoint*> &points)
{
    // there can be only one
    image_instPoint * p = img()->getInstPoint(addr());
    if(p)
        points.push_back(p);
}
void 
image_func::funcExits(pdvector<image_instPoint*> &points)
{
    vector<FuncExtent *>::const_iterator eit = extents().begin();
    for( ; eit != extents().end(); ++eit) {
        FuncExtent * fe = *eit;
        pdvector<image_instPoint *> pts;
        img()->getInstPoints(fe->start(),fe->end(),pts);
        for(unsigned i=0;i<pts.size();++i) {
            image_instPoint *p = pts[i];
            if(p->getPointType() == functionExit)
                points.push_back(p);
        }
    }
}
void 
image_func::funcCalls(pdvector<image_instPoint*> &points)
{
    vector<FuncExtent *>::const_iterator eit = extents().begin();
    for( ; eit != extents().end(); ++eit) {
        FuncExtent * fe = *eit;
        pdvector<image_instPoint *> pts;
        img()->getInstPoints(fe->start(),fe->end(),pts);
        for(unsigned i=0;i<pts.size();++i) {
            image_instPoint *p = pts[i];
            if(p->getPointType() == callSite)
                points.push_back(p);
        }
    }
}

void image_func::funcUnresolvedControlFlow(pdvector<image_instPoint*> & points)
{
    vector<FuncExtent *>::const_iterator eit = extents().begin();
    for( ; eit != extents().end(); ++eit) {
        FuncExtent * fe = *eit;
        pdvector<image_instPoint *> pts;
        img()->getInstPoints(fe->start(),fe->end(),pts);
        for(unsigned i=0;i<pts.size();++i) {
            image_instPoint *p = pts[i];
            if( p->isUnresolved() )
                points.push_back(p);
        }
    }
}

void image_func::funcAbruptEnds(pdvector<image_instPoint*> & points)
{
    vector<FuncExtent *>::const_iterator eit = extents().begin();
    for( ; eit != extents().end(); ++eit) {
        FuncExtent * fe = *eit;
        pdvector<image_instPoint *> pts;
        img()->getInstPoints(fe->start(),fe->end(),pts);
        for(unsigned i=0;i<pts.size();++i) {
            image_instPoint *p = pts[i];
            if(p->getPointType() == abruptEnd)
                points.push_back(p);
        }
    }
}


#if defined(cap_instruction_api)
void image_basicBlock::getInsnInstances(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Offset> >&instances) {
    using namespace InstructionAPI;
    Offset off = firstInsnOffset();
    const unsigned char *ptr = (const unsigned char *)getPtrToInstruction(off);
    if (ptr == NULL) return;
    InstructionDecoder d(ptr, getSize(),obj()->cs()->getArch());
    while (off < endOffset()) {
      instances.push_back(std::make_pair(d.decode(), off));
      off += instances.back().first->size();
    }
}
#endif


/* This function is static.
 *
 * Find the blocks that would become unreachable if we were to delete
 * the dead blocks.
 */
void image_func::getUnreachableBlocks
( std::set<image_basicBlock*> &deadBlocks,  // input
  std::set<image_basicBlock*> &unreachable )// output
{
    using namespace ParseAPI;
    mal_printf("GetUnreachableBlocks for %d dead blocks\n",deadBlocks.size());

    // find all funcs containing dead blocks
    std::set<image_func*> deadFuncs; 
    vector<Function*> curfuncs;
    for (set<image_basicBlock*>::iterator dIter = deadBlocks.begin();
         dIter != deadBlocks.end(); 
         dIter++) 
    {
        (*dIter)->getFuncs(curfuncs);
        for (vector<Function*>::iterator fit = curfuncs.begin();
             fit != curfuncs.end();
             fit++) 
        {
            deadFuncs.insert(dynamic_cast<image_func*>(*fit));
        }
        curfuncs.clear();
    }

    // add function entry blocks to the worklist and the visited set
    std::set<image_basicBlock*> visited;
    std::list<image_basicBlock*> worklist;
    for(std::set<image_func*>::iterator fIter=deadFuncs.begin(); 
        fIter != deadFuncs.end();
        fIter++) 
    {
        image_basicBlock *entryBlock = (*fIter)->entryBlock();
        if (deadBlocks.end() == deadBlocks.find(entryBlock)) {
            visited.insert(entryBlock);
            worklist.push_back(entryBlock);
            mal_printf("func [%lx %lx] entryBlock [%lx %lx]\n",
                    (*fIter)->getOffset(),(*fIter)->getEndOffset(),
                    entryBlock->firstInsnOffset(),
                    entryBlock->endOffset());
        }
    }

    // iterate through worklist, adding all blocks (except for
    // deadBlocks) that are reachable through target edges to the
    // visited set
    while(worklist.size()) {
        image_basicBlock *curBlock = worklist.front();
        Block::edgelist & outEdges = curBlock->targets();
        Block::edgelist::iterator tIter = outEdges.begin();
        for (; tIter != outEdges.end(); tIter++) {
            image_basicBlock *targB = (image_basicBlock*) (*tIter)->trg();
            if ( CALL != (*tIter)->type() &&
                 deadBlocks.end() == deadBlocks.find(targB) && 
                 visited.end() == visited.find(targB) )
            {   
                worklist.push_back(targB);
                visited.insert(targB);
                mal_printf("block [%lx %lx] is reachable\n",
                           targB->firstInsnOffset(),
                           targB->endOffset());
            }
        }
        worklist.pop_front();
    } 

    // add all blocks in deadFuncs but not in "visited" to the unreachable set
    for(std::set<image_func*>::iterator fIter=deadFuncs.begin(); 
        fIter != deadFuncs.end();
        fIter++) 
    {
        Function::blocklist & blks = (*fIter)->blocks();
        Function::blocklist::iterator bIter = blks.begin();
        for( ; bIter != blks.end(); ++bIter) {
            if (visited.end() == visited.find( (image_basicBlock*)(*bIter) )) {
                unreachable.insert( (image_basicBlock*)(*bIter) );
                mal_printf("block [%lx %lx] is unreachable\n",
                           (*bIter)->start(), (*bIter)->end());
            }
        }
    }
}

void image_func::setinit_retstatus(ParseAPI::FuncReturnStatus rs)
{
    init_retstatus_ = rs;
    if (rs > retstatus()) {
        set_retstatus(rs);
    }
}
ParseAPI::FuncReturnStatus image_func::init_retstatus() const
{
    if (init_retstatus_ > retstatus()) {
        return retstatus();
    }
    return init_retstatus_;
}