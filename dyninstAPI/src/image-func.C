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
 
// $Id: image-func.C,v 1.60 2008/11/03 15:19:24 jaw Exp $

#include "function.h"
#include "instPoint.h"

#if defined(cap_instruction_api)
#include "instructionAPI/h/InstructionDecoder.h"
#else
#include "InstrucIter.h"
#endif //defined(cap_instruction_api)
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

const char * image_edge::getTypeString()
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
		       image *i,
               FuncSource src) :
#if defined(arch_ia64)
  usedFPregs(NULL),
#endif
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
  containsSharedBlocks_(false),
  retStatus_(RS_UNSET),
  isTrap(false),
  instLevel_(NORMAL),
  canBeRelocated_(true),
  needsRelocation_(false),
  originalCode(NULL),
  o7_live(false)
#if defined(cap_liveness)
  , livenessCalculated_(false)
#endif
  , howDiscovered_(src)
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
    if (st)
       st->findRegion(sec, ".text");

    Symbol *sym_;
    sym_ = new Symbol(symbol.c_str(), m->fileName(), Symbol::ST_FUNCTION , Symbol:: SL_GLOBAL, 
                      Symbol::SV_DEFAULT, offset, sec, symTabSize);
    std::vector<Module *> mods;
    st->getAllModules(mods);
    if (mods.size())
        sym_->setModule(mods[0]);

    i->getObject()->addSymbol(sym_);
    func_ = sym_->getFunction();
    assert(func_);

    //i->getObject()->addSymbol(sym_);								
    //sym_->setUpPtr(this);
    //symTabNames_.push_back(symbol);
    image_func *th = this;
    extern AnnotationClass<image_func> ImageFuncUpPtrAnno;
    if (!func_->addAnnotation(th, ImageFuncUpPtrAnno))
    {
       fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
    }

}

image_func::image_func(Function *func, pdmodule *m, image *i, FuncSource src):
#if defined(arch_ia64)
  usedFPregs(NULL),
#endif
  func_(func),
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
  containsSharedBlocks_(false),
  retStatus_(RS_UNSET),
  isTrap(false),
  instLevel_(NORMAL),
  canBeRelocated_(true),
  needsRelocation_(false),
  originalCode(NULL),
  o7_live(false)
#if defined(cap_liveness)
  , livenessCalculated_(false)
#endif
 , howDiscovered_(src)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_func_count++;
    if ((image_func_count % 100) == 0)
        fprintf(stderr, "image_func_count: %d (%d)\n",
                image_func_count, image_func_count*sizeof(image_func));
#endif
    endOffset_ = func->getAddress() + func->getFirstSymbol()->getSize();
 }	


image_func::~image_func() 
{
  /* TODO */ 
  delete usedRegisters;
}

#if defined(arch_ia64)
int image_func::getFramePointerCalculator()
{
    return func_->getFramePtrRegnum();
}
#endif

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

const set<image_basicBlock*, image_basicBlock::compare> & image_func::blocks()
{
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
#if defined(cap_instruction_api)   
    using namespace Dyninst::InstructionAPI;
    const unsigned char* buffer = 
    reinterpret_cast<const unsigned char*>(getPtrToInstruction(firstInsnOffset_));
    InstructionDecoder decoder(buffer, newBlk->firstInsnOffset() -
			       firstInsnOffset_);
    Instruction tmp = decoder.decode();
    lastInsnOffset_ = firstInsnOffset_;
    
    while(lastInsnOffset_ + tmp.size() < newBlk->firstInsnOffset())
    {
      lastInsnOffset_ += tmp.size();
      tmp = decoder.decode();
    }
#else
    InstrucIter ah( this );
    while( *ah + ah.getInstruction().size() < newBlk->firstInsnOffset() )
                ah++;
    
    lastInsnOffset_ = *ah;
#endif // defined(cap_instruction_api)
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
                   pdvector<Address> &worklist)
{
    image_basicBlock *newBlk;
    codeRange *tmpRange;

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
                addToBlocklist(newBlk);
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
    pair< set<image_basicBlock *, image_basicBlock::compare>::iterator,
          bool > res;

    res = blockList.insert( newBlk );
    if(!res.second) {
        parsing_printf("[%s:%u] failed to insert block at 0x%lx into func\n",
            FILE__,__LINE__,newBlk->firstInsnOffset());
    }
}

bool image_func::containsBlock(image_basicBlock *b)
{
    return blockList.find(b) != blockList.end();
}

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

/* Sorts the basic block list and instrumentation point lists by address,
 * prints debug information, and includes sanity checks against overlapping
 * basic blocks when applicable (by platform).
 */
bool image_func::finalize()
{
    parsing_printf("[%s:%u] entering finalize for %p\n",
        FILE__,__LINE__,this);

    parsing_printf("BASIC BLOCK LIST [%s]\n",symTabName().c_str());

    set<image_basicBlock*,image_basicBlock::compare>::iterator bit =
        blockList.begin();
    for( ; bit != blockList.end(); bit++) {
        image_basicBlock *cur = *bit;
        cur->debugPrint(); 

        // Safety checks.
        // I've disabled this one but left it in so that we don't add
        // it in the future.. Since we're doing offsets, zero is
        // _fine_.
        //assert(blockList[foo]->firstInsnOffset() != 0);
        // if the first instruction is at 0 and it is a control
        // transfer instruction, lastInsnOffset can be zero too.
        // -nater 1/20/06
        //assert(blockList[foo]->lastInsnOffset() != 0);
        assert(cur->endOffset() != 0);

        assert(cur->endOffset() >= cur->firstInsnOffset());
        cur->finalize();
    }

    VECTOR_SORT( funcEntries_, image_instPoint::compare);
    VECTOR_SORT( funcReturns, image_instPoint::compare);
    VECTOR_SORT( calls, image_instPoint::compare);
    
#if defined (cap_use_pdvector)
    funcEntries_.reserve_exact(funcEntries_.size());
    funcReturns.reserve_exact(funcReturns.size());
    calls.reserve_exact(calls.size());
#endif
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
        
        image_func *pdf = image_->findFuncByEntry(destOffset);
        
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

/* XXX This would be much faster if we could make stabbing queries
   instead of iterating the list */
void *image_func::getPtrToInstruction(Address addr) const {
    if (addr < getOffset()) return NULL;
    if (!parsed_) image_->analyzeIfNeeded();
    if (addr >= endOffset_) return NULL;
    set<image_basicBlock*, image_basicBlock::compare>::const_iterator sit;
    for(sit = blockList.begin(); sit != blockList.end(); sit++) {
        void *ptr = (*sit)->getPtrToInstruction(addr);
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

void image_basicBlock::finalize() 
{
#if defined (cap_use_pdvector)
    targets_.reserve_exact(targets_.size());
    sources_.reserve_exact(sources_.size());
#endif
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
