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
 
// $Id: image-func.C,v 1.54 2008/04/18 17:07:24 jaw Exp $

#include "function.h"
#include "instPoint.h"
#include "InstrucIter.h"
#include "symtab.h"
#include "debug.h"

std::string image_func::emptyString("");

int image_func_count = 0;

// Creates a typed edge between two basic blocks.
// Updates the source/target vectors of the basic blocks.
void addEdge(image_basicBlock *source, image_basicBlock *target,
            EdgeTypeEnum type)
{
    image_edge *e = new image_edge(source,target,type);

    // since source and target blocks share edge objects, it is
    // guaranteed that either both of these operations will
    // succeed or both will fail. (failure is when an existing
    // edge of the same type is found between the blocks; this
    // shows up a lot in multi-way branches)
    if(!(source->addTarget(e) && target->addSource(e)))
    {
        delete e;
    }
}

void image_edge::breakEdge()
{
    source_->removeTarget(this);
    target_->removeSource(this);
}

char * image_edge::getTypeString()
{
    switch(type_) {
        case ET_CALL:
            return "CALL";
            break;
        case ET_COND_TAKEN:
            return "COND BRANCH - TAKEN";
            break;
        case ET_COND_NOT_TAKEN:
            return "COND BRANCH - NOT TAKEN";
            break;
        case ET_INDIR:
            return "INDIRECT BRANCH";
            break;
        case ET_DIRECT:
            return "UNCOND BRANCH";
            break;
        case ET_FALLTHROUGH:
            return "FALLTHROUGH";
            break;
        case ET_CATCH:
            return "CATCH";
            break;
        case ET_FUNLINK:
            return "POST-CALL FALLTHROUGH";
            break;
        default:
            return "ERROR UNKNOWN";
            break;
    }
}

// Verify that this is code
// Find the call points
// Find the return address
// TODO -- use an instruction object to remove
// Sets err to false on error, true on success
//
// Note - this must define funcEntry and funcReturn
// 
image_func::image_func(const std::string &symbol,
		       Address offset, 
		       const unsigned symTabSize,
		       pdmodule *m,
		       image *i) :
//  startOffset_(offset),
//  symTabSize_(symTabSize),
  endOffset_(0),
  mod_(m),
  image_(i),
  parsed_(false),
  OMPparsed_(false),
  cleansOwnStack_(false),
  usedRegisters(NULL),
  containsFPRWrites_(unknown),
  containsSPRWrites_(unknown),
  noStackFrame(false),
  makesNoCalls_(false),
  savesFP_(false),
  call_points_have_been_checked(false),
  containsSharedBlocks_(false),
  retStatus_(RS_UNSET),
  isTrap(false),
#if defined(arch_ia64)
  usedFPregs(NULL),
#endif
  instLevel_(NORMAL),
  canBeRelocated_(true),
  needsRelocation_(false),
  originalCode(NULL),
  o7_live(false),
  bl_is_sorted(false)
#if defined(cap_liveness)
  , livenessCalculated_(false)
#endif
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_func_count++;
    if ((image_func_count % 100) == 0)
        fprintf(stderr, "image_func_count: %d (%d)\n",
                image_func_count, image_func_count*sizeof(image_func));
#endif
    endOffset_ = offset + symTabSize;
    Region * sec = NULL;
    Symtab * st = i->getObject();
    if(st)
        st->findRegion(sec, ".text");
     sym_ = new Symbol(symbol.c_str(), m->fileName(), Symbol::ST_FUNCTION , Symbol:: SL_GLOBAL, 
	  		     								offset, sec, symTabSize);
     //i->getObject()->addSymbol(sym_);								
     image_func *th = this;
     annotate<Symbol, image_func *>(sym_, th, std::string("image_func_ptr"));
     //sym_->setUpPtr(this);
     //symTabNames_.push_back(symbol);
}

image_func::image_func(Symbol *symbol, pdmodule *m, image *i):
  sym_(symbol),
  mod_(m),
  image_(i),
  parsed_(false),
  cleansOwnStack_(false),
  usedRegisters(NULL),
  containsFPRWrites_(unknown),
  containsSPRWrites_(unknown),
  noStackFrame(false),
  makesNoCalls_(false),
  savesFP_(false),
  call_points_have_been_checked(false),
  containsSharedBlocks_(false),
  retStatus_(RS_UNSET),
  isTrap(false),
#if defined(arch_ia64)
  usedFPregs(NULL),
#endif
  instLevel_(NORMAL),
  canBeRelocated_(true),
  needsRelocation_(false),
  originalCode(NULL),
  o7_live(false),
  bl_is_sorted(false)
#if defined(cap_liveness)
  , livenessCalculated_(false)
#endif
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_func_count++;
    if ((image_func_count % 100) == 0)
        fprintf(stderr, "image_func_count: %d (%d)\n",
                image_func_count, image_func_count*sizeof(image_func));
#endif
    endOffset_ = symbol->getAddr() + symbol->getSize();
 }	


image_func::~image_func() { 
  /* TODO */ 
  delete usedRegisters;
}

#if defined(arch_ia64)
int image_func::getFramePointerCalculator(){
    return sym_->getFramePtrRegnum();
}
#endif

// Two-copy version... can't really do better.
bool image_func::addSymTabName(std::string name, bool isPrimary) {
    if(sym_->addMangledName(name.c_str(), isPrimary)){
        // Add to image class...
//        image_->addFunctionName(this, name, true);
	return true;
    }

    // Bool: true if the name is new; AKA !found
    return false;

#if 0
    pdvector<std::string> newSymTabName;

    // isPrimary defaults to false
    if (isPrimary)
        newSymTabName.push_back(name);

    bool found = false;
    for (unsigned i = 0; i < symTabNames_.size(); i++) {
        if (symTabNames_[i] == name) {
            found = true;
        }
        else {
            newSymTabName.push_back(symTabNames_[i]);
        }
    }
    if (!isPrimary)
        newSymTabName.push_back(name);

    symTabNames_ = newSymTabName;

    if (!found) {
        // Add to image class...
        image_->addFunctionName(this, name, true);
    }

    // Bool: true if the name is new; AKA !found
    return (!found);
#endif

}

// Two-copy version... can't really do better.
bool image_func::addPrettyName(std::string name, bool isPrimary) {
    if (sym_->addPrettyName(name.c_str(), isPrimary)) {
        // Add to image class...
//        image_->addFunctionName(this, name, false);
	return true;
    }

    // Bool: true if the name is new; AKA !found
    return false;
 
#if 0 
    pdvector<std::string> newPrettyName;

    // isPrimary defaults to false
    if (isPrimary)
        newPrettyName.push_back(name);

    bool found = false;
    for (unsigned i = 0; i < prettyNames_.size(); i++) {
        if (prettyNames_[i] == name) {
            found = true;
        }
        else {
            newPrettyName.push_back(prettyNames_[i]);
        }
    }
    if (!isPrimary)
        newPrettyName.push_back(name);

    prettyNames_ = newPrettyName;

    if (!found) {
        // Add to image class...
        image_->addFunctionName(this, name, false);
    }

    // Bool: true if the name is new; AKA !found
    return (!found);
#endif

}

// Two-copy version... can't really do better.
bool image_func::addTypedName(std::string name, bool isPrimary) {
    // Count this as a pretty name in function lookup...
    if (sym_->addTypedName(name.c_str(), isPrimary)) {
        // Add to image class...
//        image_->addFunctionName(this, name, false);
	return true;
    }

    // Bool: true if the name is new; AKA !found
    return false;

#if 0
    pdvector<std::string> newTypedName;

    // isPrimary defaults to false
    if (isPrimary)
        newTypedName.push_back(name);

    bool found = false;
    for (unsigned i = 0; i < typedNames_.size(); i++) {
        if (typedNames_[i] == name) {
            found = true;
        }
        else {
            newTypedName.push_back(typedNames_[i]);
        }
    }
    if (!isPrimary)
        newTypedName.push_back(name);

    typedNames_ = newTypedName;

    // Count this as a pretty name in function lookup...
    if (!found) {
        // Add to image class...
        image_->addFunctionName(this, name, false);
    }

    // Bool: true if the name is new; AKA !found
    return (!found);
#endif    

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
    if (!parsed_) image_->analyzeIfNeeded();
    return endOffset_;
}


const pdvector<image_instPoint *> &image_func::funcEntries() {
  if (!parsed_) image_->analyzeIfNeeded();
  return funcEntries_;
}

const pdvector<image_instPoint*> &image_func::funcExits() {
  if (!parsed_) image_->analyzeIfNeeded();

  return funcReturns;
}

const pdvector<image_instPoint*> &image_func::funcCalls() {
  if (!parsed_) image_->analyzeIfNeeded();

  return calls;
}

const pdvector<image_basicBlock *> &image_func::blocks() {
  if (!parsed_) image_->analyzeIfNeeded();
  return blockList;
}

const pdvector<image_parRegion *> &image_func::parRegions() {
  if (!parsed_) image_->analyzeIfNeeded();
  return parRegionsList;
}


bool image_func::hasNoStackFrame() { 
    if (!parsed_) image_->analyzeIfNeeded();
    return noStackFrame;
}

bool image_func::makesNoCalls() { 
    if (!parsed_) image_->analyzeIfNeeded();
    return makesNoCalls_;
}

bool image_func::savesFramePointer() { 
    if (!parsed_) image_->analyzeIfNeeded();
    return savesFP_;
}

bool image_func::cleansOwnStack() { 
    if (!parsed_) image_->analyzeIfNeeded();
    return cleansOwnStack_;
}

int image_basicBlock_count = 0;

image_basicBlock::image_basicBlock(image_func *func, Address firstOffset) :
    firstInsnOffset_(firstOffset),
    lastInsnOffset_(0),
    // this fake value is to ensure the object is always a valid code range.
    blockEndOffset_(firstOffset+1),
    isEntryBlock_(false),
    isExitBlock_(false),
    isShared_(false),
    isStub_(false),
    containsRet_(false),
    containsCall_(false),
    callIsOpaque_(false),
    isSpeculative_(false),
    canBeRelocated_(true)
{ 
    funcs_.push_back(func);
    // basic block IDs are unique within images.
    blockNumber_ = func->img()->getNextBlockID();
#if defined(ROUGH_MEMORY_PROFILE)
    image_basicBlock_count++;
    if ((image_basicBlock_count % 100) == 0)
        fprintf(stderr, "image_basicBlock_count: %d (%d)\n",
                image_basicBlock_count, image_basicBlock_count*sizeof(image_basicBlock));
#endif
}

bool image_basicBlock::addSource(image_edge *edge) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i]->source_ == edge->source_ &&
            sources_[i]->target_ == edge->target_ &&
            sources_[i]->type_ == edge->type_)
            return false;

    sources_.push_back(edge);
    return true;
}

bool image_basicBlock::addTarget(image_edge *edge) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i]->source_ == edge->source_ &&
            targets_[i]->target_ == edge->target_ &&
            targets_[i]->type_ == edge->type_)
            return false;

    targets_.push_back(edge);
    return true;
}

void image_basicBlock::removeSource(image_edge *edge) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i] == edge) {
            sources_[i] = sources_.back();
            sources_.resize(sources_.size()-1);
            return;
        }
}

void image_basicBlock::removeTarget(image_edge *edge) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i] == edge)
        {
            targets_[i] = targets_.back();
            targets_.resize(targets_.size()-1);
            return;
        }
}

void image_basicBlock::getSources(pdvector<image_edge *> &ins) const {
    for (unsigned i = 0; i < sources_.size(); i++)
        ins.push_back(sources_[i]);
}
// Need to be able to get a copy
void image_basicBlock::getTargets(pdvector<image_edge *> &outs) const {
    for (unsigned i = 0; i < targets_.size(); i++)
        outs.push_back(targets_[i]);
}

// Split a basic block at loc by control transfer (call, branch) and return
// a pointer to the new (succeeding) block. The new block should--in the
// case that succ_func is a new function--be shared by both the old
// block's original function(s) and succ_func.
image_basicBlock * image_basicBlock::split(Address loc, image_func *succ_func)
{
    image_basicBlock * newBlk;

    newBlk = new image_basicBlock(succ_func,loc);

    split(newBlk);

    return newBlk;
}

// Split this basic block at the start address of newBlk.
// This method must never be called with a basic block
// argument which contains more than one function object
// in its funcs_ vector.
void image_basicBlock::split(image_basicBlock * &newBlk)
{
    Address loc;
    image_func * existing;

    loc = newBlk->firstInsnOffset_;

    // update new block's properties
    newBlk->lastInsnOffset_ = lastInsnOffset_;
    newBlk->blockEndOffset_ = blockEndOffset_;

    for(unsigned int i=0;i<targets_.size();i++)
    {
        targets_[i]->source_ = newBlk;  // update source of edge
        newBlk->addTarget(targets_[i]);
    }
    targets_.clear();
    
    addEdge(this,newBlk,ET_FALLTHROUGH);

    // If the block that was split was owned by other functions, those
    // functions need to have newBlk added to their blocklists.
    //
    // Because of the way that pre-parsed code is handled, only functions
    // that have already been parsed need this information.
    existing = newBlk->getFirstFunc();
    parsing_printf("... newBlk->getFirstFunc() location: 0x%lx\n",
        (existing ? existing->getOffset() : 0));
    for(unsigned int i=0;i<funcs_.size();i++)
    {
        if(funcs_[i] != existing && funcs_[i]->parsed())
        {
            parsing_printf("... adding func at 0x%lx to newBlk\n",
                funcs_[i]->getOffset());

            // tell the functions they own newBlk
            funcs_[i]->addToBlocklist(newBlk);
            // tell newBlk it's owned by the functions
            newBlk->addFunc(funcs_[i]);
        }
    }

    // update this block
   
    InstrucIter ah( this );
    while( *ah + ah.getInstruction().size() < newBlk->firstInsnOffset() )
                ah++;
    
    lastInsnOffset_ = *ah;
    blockEndOffset_ = loc;      // ah.getInstruction().size()

    // Copy properties and update:
    newBlk->containsCall_ = containsCall_;
    newBlk->containsRet_ = containsRet_;
    newBlk->isExitBlock_ = isExitBlock_;
    containsCall_ = false;
    containsRet_ = false;
    isExitBlock_ = false;
}

int image_instPoint_count = 0;

image_instPoint::image_instPoint(Address offset,
                                 instruction insn,
                                 image_func *func,
                                 instPointType_t type) :
    instPointBase(insn, type),
    offset_(offset),
    func_(func),
    callee_(NULL),
    callTarget_(0),
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
                                 instruction insn,
                                 image_func *func,
                                 Address callTarget,
                                 bool isDynamic,
                                 bool isAbsolute,
                                 instPointType_t ptType) :
    instPointBase(insn, ptType),
    offset_(offset),
    func_(func),
    callee_(NULL),
    callTarget_(callTarget),
    targetIsAbsolute_(isAbsolute),
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

// Leaving some sensible const modifiers in place elsewhere, but
// we actually *do* need to append to this vector from outside the
// function object
void image_func::addCallInstPoint(image_instPoint *p)
{
    calls.push_back(p);
}

void image_func::addExitInstPoint(image_instPoint *p) 
{
    funcReturns.push_back(p);
}

/* modified 20.oct.05 to test whether the new block will split an existing
   block and, if so, to behave correctly. Also tests for attempts to
   create a basic block at a location where one has already been created.
   If one is found, that existing block is used instead. 

   Blocks created by this method have their isStub_ property set to true.
   --nater */
bool image_func::addBasicBlock(Address newAddr,
                   image_basicBlock *oldBlock,
                   BPatch_Set<Address> &leaders,
                   dictionary_hash<Address, image_basicBlock *> &leadersToBlock,
                   EdgeTypeEnum edgeType,
                   pdvector<Address> &worklist,
                   BPatch_Set< image_basicBlock * > &parserVisited)
{
    image_basicBlock *newBlk;
    codeRange *tmpRange;
    bool speculative = false;

    // Doublecheck 
    if (!image_->isCode(newAddr))
    {
        parsing_printf("! prospective block at 0x%lx rejected by isCode()\n",
            newAddr);
        return false;
    }

    // test for split of existing block
    image_basicBlock *splitBlk = NULL;
    if(image_->basicBlocksByRange.find(newAddr, tmpRange))
    {
        splitBlk = dynamic_cast<image_basicBlock *>(tmpRange);

        if(splitBlk->firstInsnOffset_ == newAddr)
        {
            // not a split, but a re-use of an existing block.
            newBlk = splitBlk;
            // The block can be in one of two states: it can be parsed
            // block of another function, or it can be a stub block of
            // this current function or the stub block representing the
            // first block of another function (i.e., a block
            // created by parsing a call instruction).
            // 
            // If the block is fully parsed (isStub_ == false), then
            // we'll end up calling parseSharedCode on it in buildCFG().
            // If isStub_ == true, then we'll do the parsing ourselves.
            // In this latter case, we need to add ourselves to the block's
            // function list here.
            //
            // If we've already parsed this block, naturally it doesn't
            // go back on the worklist
            if(!leaders.contains(newAddr))
            {
                if(newBlk->isStub_) {
                    newBlk->addFunc(this); // see above comment
                }
                worklist.push_back(newAddr);
                parsing_printf("[%s:%u] adding block %d (0x%lx) to worklist\n",
                    FILE__,__LINE__,newBlk->id(),newBlk->firstInsnOffset_);
            }
        }
        else
        {
            parsing_printf("[%s:%u] block at 0x%lx split at 0x%lx\n", FILE__,
                __LINE__,splitBlk->firstInsnOffset_,newAddr);
            // split
            newBlk = splitBlk->split(newAddr,this);
            // newBlk only goes on the worklist if the block that was split
            // has not already been parsed in /this/ function
            if(!leaders.contains(splitBlk->firstInsnOffset_))
            {
                worklist.push_back(newAddr);
                parsing_printf("[%s:%u] adding block %d (0x%lx) to worklist\n",
                    FILE__,__LINE__,newBlk->id(),newBlk->firstInsnOffset_);
            }
            else if(!leaders.contains(newBlk->firstInsnOffset_))
            {
                // This is a new (to this function) block that will not be
                // parsed, and so must be added to the blocklist here.
                parsing_printf("[%s:%u] adding block %d (0x%lx) to blocklist\n",
                    FILE__,__LINE__,newBlk->id(),newBlk->firstInsnOffset_);
                blockList.push_back(newBlk);
                parserVisited.insert(newBlk);
            }
            else
                assert(0);  //FIXME debug--remove

            image_->basicBlocksByRange.insert(newBlk);

        }
    }
    else
    {
        newBlk = new image_basicBlock(this,newAddr);
        newBlk->isStub_ = true;
        image_->basicBlocksByRange.insert(newBlk);
        parsing_printf("[%s:%u] adding block %d (0x%lx) to worklist\n",
            FILE__,__LINE__,newBlk->id(),newBlk->firstInsnOffset_);
        worklist.push_back(newAddr);
    }

    // Determine whether we are confident about this new block. A
    // call-following basic block should be marked speculative under
    // the following conditions:
    //
    // A) It is not reachable by any other non-spec control flow AND
    // ( The preceeding block's call target has not been parsed OR
    //   The preceeding block's call target has unknown return status )
    //
    // B) A non-call-following block is speculative if all of its
    //    predecessors (including oldBlock) are speculative
    if(edgeType == ET_FUNLINK)
    {
        // oldBlock may have zero or one targets, depending on whether
        // the callTarget was indirect or direct.
        if(oldBlock->targets_.size() > 0)
        {
            image_func *callTarget = 
                            oldBlock->targets_[0]->getTarget()->funcs_[0];

            if(callTarget->returnStatus() == RS_UNSET ||
            callTarget->returnStatus() == RS_UNKNOWN)
            {
                speculative = true;
            }
        }
        else
        {
            speculative = true;
        }
    }
    else
    {
        speculative = oldBlock->isSpeculative_;
    }
    
    // (What the above boils down to is that if this is a new block
    //  without existing incomming edges, we want to base our speculative
    //  decision on oldBlock, and otherwise we want it based on the
    //  existing status of the block.)    
    if(newBlk->sources_.size() == 0)
        newBlk->isSpeculative_ = speculative;
    else
    {
        if(newBlk->isSpeculative_ && !speculative)
        {
            // TODO initiate despeculation for this block
            newBlk->isSpeculative_ = false;
            //ConfirmBlocks(newBlk);
        }
        else
            newBlk->isSpeculative_ = newBlk->isSpeculative_ && speculative;
    }

    // special case: a conditional branch in block A splits A
    if(splitBlk == oldBlock)
        addEdge(newBlk,newBlk,edgeType);
    else
        addEdge(oldBlock,newBlk,edgeType);

    leadersToBlock[newAddr] = newBlk;
    leaders += newAddr;

    assert(leadersToBlock[newAddr]);

    return true;
}

void image_func::addToBlocklist(image_basicBlock * newBlk)
{
    blockList.push_back(newBlk);
    bl_is_sorted = false;
}

#if 0 // TODO
/* This method follows the speculative control flow path from a basic 
 * block and marks all reachable blocks as non-speculatively-parsed. 
 * This update pass will not cross boundaries presented by opaque
 * call sites.
 */
void ConfirmBlocks(image_basicBlock *b)
{
    parsing_printf("Confirming blocks reachable from %d (0x%lx)\n",
                   b->getBlockID(), b->getFirstInsnOffset());

    // for all blocks reachable from this that are speculative:
        // push on workset
    
    // while workset not empty
        // pop block from workset
        // mark not speculative
        // for each reachable block not across an opaque call:
            // push on workset
}
#endif

void image_basicBlock::debugPrint() {
    // no looping if we're not printing anything
    if(!dyn_debug_parsing)
        return;

    // 64-bit
    /* Function offsets are not meaningful if blocks can be
     * shared between functions.
    parsing_printf("Block %d: starts 0x%lx (%d), last 0x%lx (%d), end 0x%lx (%d)\n",
                   blockNumber_,
                   firstInsnOffset_,
                   firstInsnOffset_ - func_->getOffset(),
                   lastInsnOffset_,
                   lastInsnOffset_ - func_->getOffset(),
                   blockEndOffset_,
                   blockEndOffset_ - func_->getOffset());
    */

    parsing_printf("Block %d: starts 0x%lx, last 0x%lx, end 0x%lx\n",
                   blockNumber_,
                   firstInsnOffset_,
                   lastInsnOffset_,
                   blockEndOffset_);

    parsing_printf("  Flags: entry %d, exit %d\n",
                   isEntryBlock_, isExitBlock_);
    if (isEntryBlock_ && sources_.size()) {
        fprintf(stderr, "========== multiple entry block!\n");
    }
    
    parsing_printf("  Sources:\n");
    for (unsigned s = 0; s < sources_.size(); s++) {
        parsing_printf("    %d: block %d (%s)\n",
                       s, sources_[s]->getSource()->blockNumber_,
                       sources_[s]->getTypeString());
    }
    parsing_printf("  Targets:\n");
    for (unsigned t = 0; t < targets_.size(); t++) {
        parsing_printf("    %d: block %d (%s)\n",
                       t, targets_[t]->getTarget()->blockNumber_,
                       targets_[t]->getTypeString());
    }
}

// Make sure no blocks overlap, sort stuff by address... you know,
// basic stuff.

bool image_func::cleanBlockList() {
    //unsigned i;
#if 0
    // For all entry, exit, call points...
    //points_[u].point->checkInstructions();
    // Should also make multipoint decisions

#if !defined(arch_x86) && !defined(arch_power) && !defined(arch_x86_64)
    // We need to make sure all the blocks are inside the function
    pdvector<image_basicBlock *>cleanedList;
    for (unsigned foo = 0; foo < blockList.size(); foo++) {
        if ((blockList[foo]->firstInsnOffset() < getOffset()) ||
            (blockList[foo]->firstInsnOffset() >= getEndOffset())) {
            inst_printf("Block %d at 0x%lx is outside of function (0x%lx to 0x%lx)\n",
                        foo,
                        blockList[foo]->firstInsnOffset(),
                        getOffset(),
                        getEndOffset());
                        
            delete blockList[foo];
        }
        else
            cleanedList.push_back(blockList[foo]);
    }
    blockList.clear();
    for (unsigned bar = 0; bar < cleanedList.size(); bar++)
        blockList.push_back(cleanedList[bar]);


#endif

   //sorted_ips_vector expects funcReturns and calls to be sorted

    //check if basic blocks need to be split   
    VECTOR_SORT( blockList, image_basicBlock::compare );
    //parsing_printf("INITIAL BLOCK LIST\n");
    //maybe image_flowGraph.C would be a better home for this bit of code?
    for( unsigned int iii = 0; iii < blockList.size(); iii++ )
    {
        blockList[iii]->blockNumber_ = iii;
        blockList[iii]->debugPrint();        
    }
  
    for( unsigned int r = 0; r + 1 < blockList.size(); r++ )
    {
        image_basicBlock* b1 = blockList[ r ];
        image_basicBlock* b2 = blockList[ r + 1 ];
        
        if( b2->firstInsnOffset() < b1->endOffset() )
        {
            //parsing_printf("Blocks %d and %d overlap...\n",
            //b1->blockNumber_,
            //b2->blockNumber_);
            pdvector< image_basicBlock* > out;
            b1->getTargets( out );
            
            for( unsigned j = 0; j < out.size(); j++ )
            {
                out[j]->removeSource( b1 );
                out[j]->addSource( b2 );
            }        
          
            //set end address of higher block
            b2->lastInsnOffset_ =  b1->lastInsnOffset();
            b2->blockEndOffset_ =  b1->endOffset();
            b2->targets_ = b1->targets_;	    
            b2->addSource( b1 );
            
            b1->targets_.clear();
            b1->targets_.push_back(b2);
            
            //find the end of the split block	       
            InstrucIter ah( b1 );
            while( *ah + ah.getInstruction().size() < b2->firstInsnOffset() )
                ah++;
            
            b1->lastInsnOffset_ = *ah;
            b1->blockEndOffset_ = *ah + ah.getInstruction().size();

            if( b1->isExitBlock_ )
            {
                b1->isExitBlock_ = false;
                b2->isExitBlock_ = true;
            }
        }
    }
    for( unsigned q = 0; q + 1 < blockList.size(); q++ )
    {
        image_basicBlock* b1 = blockList[ q ];
        image_basicBlock* b2 = blockList[ q + 1 ];
        
        if( b1->endOffset() == 0 )
        {
            ///parsing_printf("Block %d has zero size; expanding to block %d\n",
            //b1->blockNumber_,
            //b2->blockNumber_);

            //find the end of this block.

            // Make the iterator happy; we can set the end offset to
            // the start of b2. It will be that or smaller.
            b1->blockEndOffset_ = b2->firstInsnOffset();

            InstrucIter ah( b1 );
            while( *ah + ah.getInstruction().size() < b2->firstInsnOffset() )
                ah++;
            
            b1->lastInsnOffset_ = *ah;
            b1->blockEndOffset_ = *ah + ah.getInstruction().size();
            b1->addTarget( b2 );	  
            b2->addSource( b1 );	            
        }        
    }    
    for (i = 0; i < blockList.size(); i++) {
        // Check sources and targets for legality
        image_basicBlock *b1 = blockList[i];
        for (unsigned s = 0; s < b1->sources_.size(); s++) {
            if (((unsigned)b1->sources_[s]->id() >= blockList.size()) ||
                (b1->sources_[s]->id() < 0)) {
                fprintf(stderr, "WARNING: block %d in function %s has illegal source block %d\n",
                        b1->id(), symTabName().c_str(), b1->sources_[s]->id());
                b1->removeSource(b1->sources_[s]);
            }
        }
#if defined(cap_relocation)
        // Don't do multiple-exit-edge blocks; 1 or 2 is cool, > is bad
        if (b1->targets_.size() > 2) {
            // Disabled as a test... bernat, 18MAY06
            //canBeRelocated_ = false;
        }
#endif
        for (unsigned t = 0; t < b1->targets_.size(); t++) {
            if (((unsigned)b1->targets_[t]->id() >= blockList.size()) ||
                (b1->targets_[t]->id() < 0)) {
                fprintf(stderr, "WARNING: block %d in function %s has illegal target block %d\n",
                        b1->id(), symTabName().c_str(), b1->targets_[t]->id());
                b1->removeTarget(b1->targets_[t]);
            }
        }

    }
#endif
   
    // Safety checks assume the block list is sorted 
    sortBlocklist();

    parsing_printf("CLEANED BLOCK LIST\n");
    for (unsigned foo = 0; foo < blockList.size(); foo++) {
        // Safety check; we need the blocks to be sorted by addr
        blockList[foo]->debugPrint();

        // Safety checks.

        // I've disabled this one but left it in so that we don't add
        // it in the future.. Since we're doing offsets, zero is
        // _fine_.
        //assert(blockList[foo]->firstInsnOffset() != 0);
        // if the first instruction is at 0 and it is a control
        // transfer instruction, lastInsnOffset can be zero too.
        // -nater 1/20/06
        //assert(blockList[foo]->lastInsnOffset() != 0);
        assert(blockList[foo]->endOffset() != 0);

        /* Serious safety checks. These can tag things that are
           legal. Enable if you're trying to track down a parsing
           problem. */

        // x86 instruction prefixes necessitate "overlapping" blocks.
        // Annoying but true.
#if !defined(arch_x86) && !defined(arch_x86_64)
        if (foo > 0) {
            assert(blockList[foo]->firstInsnOffset() >= blockList[foo-1]->endOffset());
        }
#endif

        assert(blockList[foo]->endOffset() >= blockList[foo]->firstInsnOffset());
        blockList[foo]->finalize();
    }

    VECTOR_SORT( funcEntries_, image_instPoint::compare);
    VECTOR_SORT( funcReturns, image_instPoint::compare);
    VECTOR_SORT( calls, image_instPoint::compare);
    
    funcEntries_.reserve_exact(funcEntries_.size());
    funcReturns.reserve_exact(funcReturns.size());
    calls.reserve_exact(calls.size());
    blockList.reserve_exact(blockList.size());
    return true;
}

// Bind static call points

void image_func::checkCallPoints() {
    parsing_printf("%s: checking call points\n", symTabName().c_str());
    // Check if there are any dangling calls
    for (unsigned c = 0; c < calls.size(); c++) {
        image_instPoint *p = calls[c];
        assert(p);

        parsing_printf("... 0x%lx...", p->offset());
        if (p->getCallee() != NULL) {
            parsing_printf(" already bound\n");
            continue;
        }

        Address destOffset = p->callTarget();
        if (!destOffset) {
            // Couldn't determine contact; skip
            parsing_printf(" no destination\n");
            continue;
        }
        
        image_func *pdf = image_->findFuncByOffset(destOffset);
        
        if (pdf) {
            p->setCallee(pdf);
            parsing_printf(" set to %s\n", pdf->symTabName().c_str());
        }
        else {
            parsing_printf(" failed lookup for 0x%lx\n",
                           destOffset);
        }
    }
}

void image_func::sortBlocklist()
{
    VECTOR_SORT( blockList, image_basicBlock::compare );
    bl_is_sorted = true;
}

// No longer needed but kept around for reference
#if 0

//correct parsing errors that overestimate the function's size by
// 1. updating all the vectors of instPoints
// 2. updating the vector of basicBlocks
// 3. updating the function size
// 4. update the address of the last basic block if necessary
void image_func::updateFunctionEnd(Address newEnd)
{

    //update the size
    endOffset_ = newEnd;
    //remove out of bounds call Points
    //assumes that calls was sorted by address in findInstPoints
    for( int i = (int)calls.size() - 1; i >= 0; i-- )
        {
            if( calls[ i ]->offset() >= newEnd )
                {
                    delete calls[ i ];
                    calls.pop_back();
                }
            else 
                break;
        }
    //remove out of bounds return points
    //assumes that funcReturns was sorted by address in findInstPoints
    for( int j = (int)funcReturns.size() - 1; j >= 0; j-- )
        {
            if( funcReturns[ j ]->offset() >= newEnd )
                {
                    delete funcReturns[ j ];
                    funcReturns.pop_back();
                }
            else
                break;
        }

    //remove out of bounds basicBlocks
    //assumes blockList was sorted by start address in findInstPoints
    for( int k = (int) blockList.size() - 1; k >= 0; k-- )
        {
            image_basicBlock* curr = blockList[ k ];
            if( curr->firstInsnOffset() >= newEnd )
                {
                    //remove all references to this block from the flowgraph
                    //my source blocks should no longer have me as a target
                    pdvector< image_basicBlock* > ins;
                    curr->getSources( ins );
                    for( unsigned o = 0; o < ins.size(); o++ )
                        {
                            ins[ o ]->removeTarget(curr);
                            ins[ o ]->isExitBlock_ = true;
                        } 
                    
                    //my target blocks should no longer have me as a source 
                    pdvector< image_basicBlock* > outs;
                    curr->getTargets( outs );
                    for( unsigned p = 0; p < outs.size(); p++ )
                        {
                            outs[ p ]->removeSource(curr);
                        }                     
                    delete curr;
                    blockList.pop_back();          
                }
            else
                break;
        } 
    
    //we might need to correct the end address of the last basic block
    int n = blockList.size() - 1;
    if( n >= 0 && blockList[n]->endOffset() >= newEnd )
        {
            parsing_printf("Updating block end: new end of function 0x%lx\n",
                           newEnd);
            blockList[n]->debugPrint();
            
            // TODO: Ask Laune; this doesn't look like it's doing what we want.
            image_basicBlock* blk = blockList[n];
            
            InstrucIter ah(blk);	
            while( ah.peekNext() < newEnd )
                ah++;
            
            blk->lastInsnOffset_ = *ah ;
            blk->blockEndOffset_ = ah.peekNext();
            
            if (!blk->isExitBlock_) {
                blk->isExitBlock_ = true;
                // Make a new exit point
                image_instPoint *p = new image_instPoint(*ah,
                                                         ah.getInstruction(),
                                                         this,
                                                         functionExit);
                funcReturns.push_back(p);
            }
            
            parsing_printf("After fixup:\n");
            blk->debugPrint();
        }

    funcReturns.reserve_exact(funcReturns.size());
    calls.reserve_exact(calls.size());
    
    // And rerun memory trimming
    for (unsigned foo = 0; foo < blockList.size(); foo++)
        blockList[foo]->finalize();
    
}    

#endif

void image_basicBlock::addFunc(image_func * func)
{
    /* enforced elsewhere; uncomment to debug
    for(unsigned i=0;i<funcs_.size(); ++i) {
        //assert(funcs_[i] != func);
        if(funcs_[i] == func) {
            fprintf(stderr,"duplicate function in addFunc\n");
            assert(0);
        }
    }
    */

    funcs_.push_back(func);

    if(funcs_.size() > 0)
        isShared_ = true;
}

bool image_basicBlock::containedIn(image_func * f)
{
    for(unsigned i=0;i<funcs_.size();i++)
    {
        if(funcs_[i] == f)
            return true;
    }
    return false;
}

void *image_basicBlock::getPtrToInstruction(Address addr) const {
    if (addr < firstInsnOffset_) return NULL;
    if (addr >= blockEndOffset_) return NULL;
    // XXX all potential parent functions have the same image
    return getFirstFunc()->img()->getPtrToInstruction(addr);
}

void *image_func::getPtrToInstruction(Address addr) const {
    if (addr < getOffset()) return NULL;
    if (!parsed_) image_->analyzeIfNeeded();
    if (addr >= endOffset_) return NULL;
    for (unsigned i = 0; i < blockList.size(); i++) {
        void *ptr = blockList[i]->getPtrToInstruction(addr);
        if (ptr) return ptr;
    }
    return NULL;
}

image_instPoint * image_basicBlock::getCallInstPoint()
{
    pdvector< image_instPoint * > calls;

    if(!containsCall_ || funcs_.size() == 0)
        return NULL;

    // every function that this block belongs to should have exactly
    // one call instPoint within this block's range. Select an arbitrary
    // function.

    for(unsigned int j=0;j<funcs_.size();j++)
    {
        calls = funcs_[j]->funcCalls();
        for(unsigned int i=0;i<calls.size();i++)
        {
            if(calls[i]->offset_ >= firstInsnOffset_ &&
               calls[i]->offset_ <= lastInsnOffset_)
                return calls[i]; 
        } 
    }
       
    return NULL;
}

image_instPoint * image_basicBlock::getRetInstPoint()
{
    pdvector< image_instPoint * > rets;

    if(!containsRet_ || funcs_.size() == 0)
        return NULL;

    for(unsigned int j=0;j<funcs_.size();j++)
    {
        rets = funcs_[j]->funcExits();
        for(unsigned int i=0;i<rets.size();i++)
        {
            if(rets[i]->offset_ >= firstInsnOffset_ &&
               rets[i]->offset_ <= lastInsnOffset_)
                return rets[i];
        } 
    }

    return NULL;
}

void image_basicBlock::finalize() {
    targets_.reserve_exact(targets_.size());
    sources_.reserve_exact(sources_.size());
}


bool image_basicBlock::isEntryBlock(image_func * f) const
{
    if(!isEntryBlock_)
        return false;

    for(unsigned i=0;i<funcs_.size();i++)
    {
        if(funcs_[i] == f && f->entryBlock() == this)
            return true;
    }

    return false;
} 

image_func *image_basicBlock::getEntryFunc() const {
  for (unsigned i = 0; i < funcs_.size(); i++) {
    if (funcs_[i]->entryBlock() == this)
      return funcs_[i];
  }
  return NULL;
}

void image_basicBlock::getFuncs(pdvector<image_func *> &funcs) const
{
    for(unsigned i=0; i < funcs_.size(); i++)
    {
        funcs.push_back(funcs_[i]);
    }
}

image_basicBlock * image_func::entryBlock() { 
    if (!parsed_) image_->analyzeIfNeeded();
    return entryBlock_;
}

bool image_func::isLeafFunc() {
    if (!parsed_)
        image_->analyzeIfNeeded();

    return calls.size() > 0;
}
