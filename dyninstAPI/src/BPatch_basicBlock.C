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

#define BPATCH_FILE

#include <stdio.h>
#include <iostream>
#include "util.h"
#include "common/h/Types.h"

#include "BPatch_flowGraph.h"
#include "BPatch_collections.h"
#include "BPatch_basicBlock.h"
#include "function.h"
#if !defined(cap_instruction_api)
#include "parseAPI/src/InstrucIter.h"
#endif
#include "BPatch_instruction.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "BPatch_libInfo.h"
#include "BPatch_edge.h"
#include "instPoint.h"
#include "addressSpace.h"
#include "symtab.h"

int bpatch_basicBlock_count = 0;

BPatch_basicBlock::BPatch_basicBlock(int_basicBlock *ib, BPatch_flowGraph *fg):
   iblock(ib),
   flowGraph(fg),
   immediateDominates(NULL),
   immediateDominator(NULL),
   immediatePostDominates(NULL),
   immediatePostDominator(NULL),
   sourceBlocks(NULL),
   instructions(NULL)
{

#if defined(ROUGH_MEMORY_PROFILE)
    bpatch_basicBlock_count++;
    if ((bpatch_basicBlock_count % 10) == 0)
        fprintf(stderr, "bpatch_basicBlock_count: %d (%d)\n",
                bpatch_basicBlock_count, bpatch_basicBlock_count*sizeof(BPatch_basicBlock));
#endif

   ib->setHighLevelBlock(this);
}

//destructor of the class BPatch_basicBlock
void BPatch_basicBlock::BPatch_basicBlock_dtor(){
	if (immediatePostDominates)
		delete immediatePostDominates;
	if (immediateDominates)
		delete immediateDominates;
	if (sourceBlocks)
		delete sourceBlocks;
        if (instructions)
                delete instructions;

        BPatch_Set<BPatch_edge *>::iterator eIter;
        eIter = incomingEdges.begin();
        while (eIter != incomingEdges.end()) {
            delete (*eIter);
            eIter++;
        }
        eIter = outgoingEdges.begin();
        while (eIter != outgoingEdges.end()) {
            delete (*eIter);
            eIter++;
        }
	return;
}

//returns the predecessors of the basic block in aset 
void BPatch_basicBlock::getSourcesInt(BPatch_Vector<BPatch_basicBlock*>& srcs){
   BPatch_basicBlock *b;
   pdvector<int_basicBlock *> in_blocks;
   unsigned i;

   iblock->getSources(in_blocks);
   for (i=0; i<in_blocks.size(); i++)
   {
      b = (BPatch_basicBlock *) in_blocks[i]->getHighLevelBlock();
      if (b) srcs.push_back(b);
   }
}

//returns the successors of the basic block in a set 
void BPatch_basicBlock::getTargetsInt(BPatch_Vector<BPatch_basicBlock*>& tgrts){
   BPatch_basicBlock *b;
   pdvector<int_basicBlock *> out_blocks;
   unsigned i;

   iblock->getTargets(out_blocks);
   for (i=0; i<out_blocks.size(); i++)
   {
      b = (BPatch_basicBlock *) out_blocks[i]->getHighLevelBlock();
      if (b) tgrts.push_back(b);
   }
}

//returns the dominates of the basic block in a set 
void BPatch_basicBlock::getImmediateDominatesInt(BPatch_Vector<BPatch_basicBlock*>& imds){
	flowGraph->fillDominatorInfo();

	if(!immediateDominates)
		return;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediateDominates->size()];
	immediateDominates->elements(elements);
	for(unsigned i=0;i<immediateDominates->size();i++)
		imds.push_back(elements[i]);
	delete[] elements;
	return;
}


void BPatch_basicBlock::getImmediatePostDominatesInt(BPatch_Vector<BPatch_basicBlock*>& imds){
	flowGraph->fillPostDominatorInfo();

	if(!immediatePostDominates)
		return;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediatePostDominates->size()];
	immediatePostDominates->elements(elements);
	for(unsigned i=0;i<immediatePostDominates->size();i++)
		imds.push_back(elements[i]);
	delete[] elements;
	return;
}

//returns the dominates of the basic block in a set 
void
BPatch_basicBlock::getAllDominatesInt(BPatch_Set<BPatch_basicBlock*>& buffer){
	flowGraph->fillDominatorInfo();

	buffer += (BPatch_basicBlock*)this;
	if(immediateDominates){
		BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[immediateDominates->size()];
		immediateDominates->elements(elements);
		for(unsigned i=0;i<immediateDominates->size();i++)
			elements[i]->getAllDominates(buffer);
		delete[] elements;
	}
	return;
}

void
BPatch_basicBlock::getAllPostDominatesInt(BPatch_Set<BPatch_basicBlock*>& buffer){
	flowGraph->fillPostDominatorInfo();

	buffer += (BPatch_basicBlock*)this;
	if(immediatePostDominates){
		BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[immediatePostDominates->size()];
		immediatePostDominates->elements(elements);
		for(unsigned i=0;i<immediatePostDominates->size();i++)
			elements[i]->getAllPostDominates(buffer);
		delete[] elements;
	}
	return;
}

//returns the immediate dominator of the basic block
BPatch_basicBlock* BPatch_basicBlock::getImmediateDominatorInt(){
	flowGraph->fillDominatorInfo();

	return immediateDominator;
}

BPatch_basicBlock* BPatch_basicBlock::getImmediatePostDominatorInt(){
	flowGraph->fillPostDominatorInfo();

	return immediatePostDominator;
}

//returns whether this basic block dominates the argument
bool BPatch_basicBlock::dominatesInt(BPatch_basicBlock* bb){
	if(!bb)
		return false;

	if(bb == this)
		return true;

	flowGraph->fillDominatorInfo();
	
	if(!immediateDominates)
		return false;

	bool done = false;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediateDominates->size()];
	immediateDominates->elements(elements);
	for(unsigned i=0;!done && (i<immediateDominates->size());i++)
		done = done || elements[i]->dominates(bb);
	delete[] elements;
	return done;
}

bool BPatch_basicBlock::postdominatesInt(BPatch_basicBlock* bb){
	if(!bb)
		return false;

	if(bb == this)
		return true;

	flowGraph->fillPostDominatorInfo();

	if(!immediatePostDominates)
		return false;

	bool done = false;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediatePostDominates->size()];
	immediatePostDominates->elements(elements);
	for(unsigned i=0;!done && (i<immediatePostDominates->size());i++)
		done = done || elements[i]->postdominates(bb);
	delete[] elements;
	return done;
}

//returns the source block corresponding to the basic block
//which is created looking at the machine code.
bool
BPatch_basicBlock::getSourceBlocksInt(BPatch_Vector<BPatch_sourceBlock*>& sBlocks)
{
    if(!sourceBlocks)
        flowGraph->createSourceBlocks();
    
    if(!sourceBlocks)
        return false;
    
    for(unsigned int i=0;i<sourceBlocks->size();i++)
        sBlocks.push_back((*sourceBlocks)[i]);

    return true;
}

//returns the block number of the basic block
int BPatch_basicBlock::getBlockNumberInt() {
	return iblock->id();
}

// returns the range of addresses of the code for the basic block
bool BPatch_basicBlock::getAddressRangeInt(void*& _startAddress,
                                           void*& _lastInsnAddress)
{
	_startAddress = (void *) getStartAddress();
	_lastInsnAddress = (void *) getLastInsnAddress();
   return true;
}

#ifdef IBM_BPATCH_COMPAT
bool BPatch_basicBlock::getLineNumbersInt(unsigned int &_startLine,
                                          unsigned int  &_endLine)
{
  BPatch_Vector<BPatch_sourceBlock *> sbvec;
  getSourceBlocks(sbvec);
  if (!sbvec.size()) return false;

  unsigned int temp_start = UINT_MAX, temp_end = 0;
  _startLine = UINT_MAX;
  _endLine = 0;

  //  Loop through all source blocks and accumulate the smallest start line
  //  and the largest end line.  (is there a better way? -- don't we know this a priori?)
  for (unsigned int i = 0; i < sbvec.size(); ++i) {
    sbvec[i]->getLineNumbers(temp_start, temp_end);
    if (temp_start < _startLine) _startLine = temp_start;
    if (temp_end > _endLine) _endLine = temp_end;
  }
  return true;
}
#endif

ostream& operator<<(ostream& os,BPatch_basicBlock& bb)
{
   unsigned i;
	os << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
	os << "Basic Block : " << bb.blockNo() <<" : [ ";
	os << ostream::hex << bb.getStartAddress() << " , ";
	os << ostream::hex << bb.getLastInsnAddress() << " | ";
	os << ostream::dec << bb.getEndAddress() - bb.getStartAddress() << " ]\n";

	if(bb.isEntryBlock())
		os <<"Type : ENTRY TO CFG\n"; 
	else if(bb.isExitBlock())
		os <<"Type : EXIT FROM CFG\n"; 

	cout << "Pred :\n";
   BPatch_Vector<BPatch_basicBlock *> elements;
   bb.getSources(elements);
   for (i=0; i<elements.size(); i++)
		os << "\t<- " << elements[i]->blockNo() << "\n";

	cout << "Succ:\n";
   elements.clear();
   bb.getTargets(elements);
   for (i=0; i<elements.size(); i++)
		os << "\t-> " << elements[i]->blockNo() << "\n";

   BPatch_basicBlock **belements;
	os << "Immediate Dominates: ";
	if(bb.immediateDominates){
		belements = new BPatch_basicBlock*[bb.immediateDominates->size()];
		bb.immediateDominates->elements(belements);
		for(i=0; i<bb.immediateDominates->size(); i++)
			os << belements[i]->blockNo() << " ";
		delete[] belements;
	}
	os << "\n";

	os << "Immediate Dominator: ";
	if(!bb.immediateDominator)
		os << "None\n";
	else
		os << bb.immediateDominator->blockNo() << "\n";

	os << "\n";
	os << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
	return os;
}

/*
 * BPatch_basicBlock::findPoint (based on VG 09/05/01)
 *
 * Returns a vector of the instrumentation points from a basic block that is
 * identified by the parameters, or returns NULL upon failure.
 * (Points are sorted by address in the vector returned.)
 *
 * ops          The points within the basic block to return. A set of op codes
 *              defined in BPatch_opCode (BPatch_point.h)
 */
#if defined(cap_instruction_api)
using namespace Dyninst::InstructionAPI;
bool isLoad(Instruction::Ptr i)
{
    return i->readsMemory();
} 
bool isStore(Instruction::Ptr i)
{
    return i->writesMemory();
}
bool isPrefetch(Instruction::Ptr i)
{
    return i->getCategory() == c_PrefetchInsn;
}

struct funcPtrPredicate : public insnPredicate
{
    result_type(*m_func)(argument_type);
    funcPtrPredicate(result_type(*func)(argument_type)) :
            m_func(func) {}
    result_type operator()(argument_type arg) {
        return m_func(arg);
    }
};

struct findInsns : public insnPredicate
{
    findInsns(const BPatch_Set<BPatch_opCode>& ops)
    : findLoads(false), findStores(false), findPrefetch(false)
    {
        BPatch_opCode* opa = new BPatch_opCode[ops.size()];
        ops.elements(opa);
    
    
        for(unsigned int i=0; i<ops.size(); ++i) {
            switch(opa[i]) {
                case BPatch_opLoad: findLoads = true; break;
                case BPatch_opStore: findStores = true; break;
                case BPatch_opPrefetch: findPrefetch = true; break;
            }
        }
        delete[] opa;
    }
    result_type operator()(argument_type i)
    {
        //static int counter = 0;
        if(findLoads && isLoad(i))
        {
            //counter++;
            //fprintf(stderr, "Instruction #%d %s is a load\n", counter, i->format().c_str());
            return true;
        }
        if(findStores && isStore(i))
        {
            //counter++;
            //fprintf(stderr, "Instruction #%d %s is a store\n", counter, i->format().c_str());
	  return true;
        }
        if(findPrefetch && isPrefetch(i))
        {
            //counter++;
            //fprintf(stderr, "Instruction #%d %s is a prefetch\n", counter, i->format().c_str());
            return true;
        }
	//	fprintf(stderr, "Instruction %s failed filter\n", i->format().c_str());
        return false;
    }
    bool findLoads;
    bool findStores;
    bool findPrefetch;
};
#endif
        
BPatch_point* BPatch_basicBlock::findEntryPointInt()
{
    return BPatch_point::createInstructionInstPoint(flowGraph->getAddSpace(), (void*)this->getStartAddressInt(),
        flowGraph->getBFunction());
}

BPatch_point* BPatch_basicBlock::findExitPointInt()
{
    return BPatch_point::createInstructionInstPoint(flowGraph->getAddSpace(), (void*)this->getEndAddressInt(),
            flowGraph->getBFunction());
}
        
BPatch_Vector<BPatch_point*>*
    BPatch_basicBlock::findPointByPredicate(insnPredicate& f)
{
    BPatch_Vector<BPatch_point*>* ret = new BPatch_Vector<BPatch_point*>;
    std::vector<std::pair<Dyninst::InstructionAPI::Instruction::Ptr, Address> > insns;
    getInstructions(insns);
    for(std::vector<std::pair<Dyninst::InstructionAPI::Instruction::Ptr, Address> >::iterator curInsn = insns.begin();
        curInsn != insns.end();
        ++curInsn)
    {
//        fprintf(stderr, "Checking insn at 0x%lx...", curInsn->second);
        if(f(curInsn->first))
        {
            BPatch_point* tmp = BPatch_point::createInstructionInstPoint(flowGraph->getAddSpace(), (void*) curInsn->second,
                    flowGraph->getBFunction());
            if(!tmp)
            {
#if defined(cap_instruction_api)
                fprintf(stderr, "WARNING: failed to create instpoint for load/store/prefetch %s at 0x%lx\n",
                    curInsn->first->format().c_str(), curInsn->second);
#endif //defined(cap_instruction_api)
            }
            else
            {
                ret->push_back(tmp);
            }
        }
    }
    return ret;
    
}
        
BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPointInt(const BPatch_Set<BPatch_opCode>& ops) 
{

    // function is generally uninstrumentable (with current technology)
    if (!flowGraph->getBFunction()->func->isInstrumentable())
        return NULL;
    
#if defined(cap_instruction_api)
    findInsns filter(ops);
    return findPointByPredicate(filter);
#else
    // Use an instruction iterator
    InstrucIter ii(getStartAddress(),size(),flowGraph->getllAddSpace());
    BPatch_function *func = flowGraph->getBFunction();
    
    return BPatch_point::getPoints(ops, ii, func);
#endif
}

#if defined(cap_instruction_api)
BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPointInt(bool(*filter)(Instruction::Ptr))
{

    funcPtrPredicate filterPtr(filter);
    return findPointByPredicate(filterPtr);
}
#endif

// does not return duplicates even if some points belong to multiple categories
//
void BPatch_basicBlock::getAllPoints(std::vector<BPatch_point*>& bpPoints)
{
    set<BPatch_point*> dupCheck;
    BPatch_addressSpace *addSpace = flowGraph->getBFunction()->getAddSpace();
    pdvector<instPoint*> blockPoints = iblock->func()->funcEntries();
    unsigned pIdx;
    for (pIdx=0; pIdx < blockPoints.size(); pIdx++) {
        if (iblock->origInstance()->firstInsnAddr() <= blockPoints[pIdx]->addr()
            && iblock->origInstance()->endAddr() > blockPoints[pIdx]->addr()) 
        {
            BPatch_point *point = addSpace->findOrCreateBPPoint
                ( flowGraph->getBFunction(), 
                  blockPoints[pIdx], 
                  BPatch_point::convertInstPointType_t
                  (blockPoints[pIdx]->getPointType()) );
            dupCheck.insert(point);
            bpPoints.push_back(point);
        }
    }
    blockPoints = iblock->func()->funcExits();
    for (pIdx=0; pIdx < blockPoints.size(); pIdx++) {
        if (iblock->origInstance()->firstInsnAddr() <= blockPoints[pIdx]->addr()
            && iblock->origInstance()->endAddr() > blockPoints[pIdx]->addr()) 
        {
            BPatch_point *point = addSpace->findOrCreateBPPoint
                ( flowGraph->getBFunction(), 
                  blockPoints[pIdx], 
                  BPatch_point::convertInstPointType_t
                  (blockPoints[pIdx]->getPointType()) );
            if (point && dupCheck.end() != dupCheck.find(point)) {
                dupCheck.insert(point);
                bpPoints.push_back(point);
            }
        }
    }
    blockPoints = iblock->func()->funcCalls();
    for (pIdx=0; pIdx < blockPoints.size(); pIdx++) {
        if (iblock->origInstance()->firstInsnAddr() <= blockPoints[pIdx]->addr()
            && iblock->origInstance()->endAddr() > blockPoints[pIdx]->addr()) 
        {
            BPatch_point *point = addSpace->findOrCreateBPPoint
                ( flowGraph->getBFunction(), 
                  blockPoints[pIdx], 
                  BPatch_point::convertInstPointType_t
                  (blockPoints[pIdx]->getPointType()) );
            if (point && dupCheck.end() != dupCheck.find(point)) {
                dupCheck.insert(point);
                bpPoints.push_back(point);
            }
        }
    }
    blockPoints = iblock->func()->funcArbitraryPoints();
    for (pIdx=0; pIdx < blockPoints.size(); pIdx++) {
        if (iblock->origInstance()->firstInsnAddr() <= blockPoints[pIdx]->addr()
            && iblock->origInstance()->endAddr() > blockPoints[pIdx]->addr()) 
        {
            BPatch_point *point = addSpace->findOrCreateBPPoint
                ( flowGraph->getBFunction(), 
                  blockPoints[pIdx], 
                  BPatch_point::convertInstPointType_t
                  (blockPoints[pIdx]->getPointType()) );
            if (point && dupCheck.end() != dupCheck.find(point)) {
                dupCheck.insert(point);
                bpPoints.push_back(point);
            }
        }
    }
    std::set<instPoint*> pointSet = iblock->func()->funcUnresolvedControlFlow();
    std::set<instPoint*>::iterator pIter = pointSet.begin();
    while (pIter != pointSet.end()) {
        if (iblock->origInstance()->firstInsnAddr() <= (*pIter)->addr()
            && iblock->origInstance()->endAddr() > (*pIter)->addr()) 
        {
            BPatch_point *point = addSpace->findOrCreateBPPoint
                ( flowGraph->getBFunction(), 
                  *pIter, 
                  BPatch_point::convertInstPointType_t
                  (blockPoints[pIdx]->getPointType()) );
            if (point && dupCheck.end() != dupCheck.find(point)) {
                dupCheck.insert(point);
                bpPoints.push_back(point);
            }
        }
        pIter++;
    }
    pointSet = iblock->func()->funcAbruptEnds();
    pIter = pointSet.begin();
    while (pIter != pointSet.end()) {
        if (iblock->origInstance()->firstInsnAddr() <= (*pIter)->addr()
            && iblock->origInstance()->endAddr() > (*pIter)->addr()) 
        {
            BPatch_point *point = addSpace->findOrCreateBPPoint
                ( flowGraph->getBFunction(), 
                  *pIter, 
                  BPatch_point::convertInstPointType_t
                  (blockPoints[pIdx]->getPointType()) );
            if (point && dupCheck.end() != dupCheck.find(point)) {
                dupCheck.insert(point);
                bpPoints.push_back(point);
            }
        }
        pIter++;
    }
}


BPatch_function * BPatch_basicBlock::getCallTarget()
{
    image_instPoint* imgPt = iblock->func()->ifunc()->img()->getInstPoint
        ( iblock->llb()->lastInsnAddr() );
    if ( ! imgPt || callSite != imgPt->getPointType() ) {
        return NULL;
    }
    Address baseAddr = iblock->func()->ifunc()->img()->desc().loadAddr();
    Address targetAddr = imgPt->callTarget() + baseAddr;
    Address pointAddr =  imgPt->offset() + baseAddr;
    int_function *targFunc = 
        flowGraph->getllAddSpace()->findFuncByAddr(targetAddr);

    if (!targFunc && imgPt->isDynamic()) { 
        // if this is an indirect call, use its saved target
        instPoint *intCallPoint = iblock->func()->findInstPByAddr(pointAddr);
        if (!intCallPoint) {
            iblock->func()->funcCalls();
            intCallPoint = iblock->func()->findInstPByAddr(pointAddr);
        }
        assert(intCallPoint);
        targFunc = iblock->func()->proc()->findFuncByAddr
            ( intCallPoint->getSavedTarget() );
    }
    if (!targFunc) {
        return NULL;
    }
    BPatch_function * bpfunc = 
        flowGraph->getAddSpace()->findOrCreateBPFunc(targFunc,NULL);
    return bpfunc;
}


/*
 * BPatch_basicBlock::getInstructions
 *
 * Returns a vector of the instructions contained within this block
 *
 */
#if defined(cap_instruction_api)
BPatch_Vector<BPatch_instruction*> *BPatch_basicBlock::getInstructionsInt(void) {
  return NULL;
  
}

#else
BPatch_Vector<BPatch_instruction*> *BPatch_basicBlock::getInstructionsInt(void) {

  if (!instructions) {

    instructions = new BPatch_Vector<BPatch_instruction*>;
    InstrucIter ii(getStartAddress(),size(),flowGraph->getllAddSpace());
    
    while(ii.hasMore()) {
      BPatch_instruction *instr = ii.getBPInstruction();
      instr->parent = this;
      instructions->push_back(instr);
      ii++;
    }
  }

  return instructions;
}
#endif

/*
 * BPatch_basicBlock::getInstructions
 *
 * Returns a vector of the instructions contained within this block
 *
 */
#if defined(cap_instruction_api)
bool BPatch_basicBlock::getInstructionsInt(std::vector<InstructionAPI::Instruction::Ptr>& insns) {
  using namespace InstructionAPI;

  InstructionDecoder d((const unsigned char*)
        (iblock->proc()->getPtrToInstruction(getStartAddress())),
        size(),
        iblock->llb()->obj()->cs()->getArch());
  do {
      insns.push_back(d.decode());
  } while (insns.back() && insns.back()->isValid());

  // Remove final invalid instruction
  if (!insns.empty()) insns.pop_back();

  return !insns.empty();  
}

bool BPatch_basicBlock::getInstructionsAddrs(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> >& insnInstances) {
  using namespace InstructionAPI;
  Address addr = getStartAddress();
  const unsigned char *ptr = (const unsigned char *)iblock->proc()->getPtrToInstruction(addr);
  if (ptr == NULL) return false;
  InstructionDecoder d(ptr, size(), iblock->llb()->obj()->cs()->getArch());

  while (addr < getEndAddress()) {
      insnInstances.push_back(std::make_pair(d.decode(), addr));
      addr += insnInstances.back().first->size();
  }

  return !insnInstances.empty();  
}
#else
bool BPatch_basicBlock::getInstructionsInt(std::vector<InstructionAPI::Instruction::Ptr>& /* insns */)
{
  return false;
}

bool BPatch_basicBlock::getInstructionsAddrs(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> >& /* insnInstances */)
{
  return false;
}
#endif // defined(cap_instruction_api)

unsigned long BPatch_basicBlock::getStartAddressInt() CONST_EXPORT 
{
   return iblock->origInstance()->firstInsnAddr();
}

unsigned long BPatch_basicBlock::getLastInsnAddressInt() CONST_EXPORT 
{
   return iblock->origInstance()->lastInsnAddr();
}

unsigned long BPatch_basicBlock::getEndAddressInt() CONST_EXPORT
{
   return iblock->origInstance()->endAddr();
}

unsigned BPatch_basicBlock::sizeInt() CONST_EXPORT
{
   return getEndAddress() - getStartAddress();
}

void BPatch_basicBlock::getIncomingEdgesInt(BPatch_Vector<BPatch_edge*>& inc)
{
    BPatch_Set<BPatch_edge*>::iterator incIter = incomingEdges.begin();
    while (incIter != incomingEdges.end()) {
        inc.push_back(*incIter);
        incIter++;
    }
}
        
void BPatch_basicBlock::getOutgoingEdgesInt(BPatch_Vector<BPatch_edge*>& out)
{
    BPatch_Set<BPatch_edge*>::iterator outIter = outgoingEdges.begin();
    while (outIter != outgoingEdges.end()) {
        out.push_back(*outIter);
        outIter++;
    }
}

int BPatch_basicBlock::blockNo() const
{ 
   return iblock->id();
}

bool BPatch_basicBlock::isEntryBlockInt() CONST_EXPORT {
   return iblock->isEntryBlock();
}

bool BPatch_basicBlock::isExitBlockInt() CONST_EXPORT {
   return iblock->isExitBlock();
}
