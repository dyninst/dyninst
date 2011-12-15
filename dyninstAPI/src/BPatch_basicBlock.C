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

#define BPATCH_FILE

#include <stdio.h>
#include <iostream>
#include "util.h"
#include "common/h/Types.h"

#include "BPatch_flowGraph.h"
#include "BPatch_collections.h"
#include "BPatch_basicBlock.h"
#include "process.h"
#include "function.h"
#include "BPatch_instruction.h"
#include "BPatch_point.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "BPatch_libInfo.h"
#include "BPatch_edge.h"
#include "instPoint.h"
#include "mapped_object.h"

int bpatch_basicBlock_count = 0;

BPatch_basicBlock::BPatch_basicBlock(block_instance *ib, BPatch_flowGraph *fg):
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
  pdvector<block_instance *> in_blocks;
  const PatchBlock::edgelist &isrcs = iblock->getSources();
  for (PatchBlock::edgelist::const_iterator iter = isrcs.begin(); iter != isrcs.end(); ++iter) {
    edge_instance* iedge = SCAST_EI(*iter);
    // We don't include interprocedural predecessors in the BPatch layer
    if (iedge->interproc()) continue;

    b = flowGraph->findBlock(iedge->src());
    if (b) srcs.push_back(b);
  }
}

//returns the successors of the basic block in a set
void BPatch_basicBlock::getTargetsInt(BPatch_Vector<BPatch_basicBlock*>& tgrts){
  BPatch_basicBlock *b;
  pdvector<block_instance *> out_blocks;
  const PatchBlock::edgelist &itrgs = iblock->getTargets();
  for (PatchBlock::edgelist::const_iterator iter = itrgs.begin(); iter != itrgs.end(); ++iter) {
    edge_instance* iedge = SCAST_EI(*iter);
    // We don't include interprocedural predecessors in the BPatch layer
    if (iedge->interproc() || iedge->sinkEdge()) continue;

    b = flowGraph->findBlock(iedge->trg());
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
    //  fprintf(stderr, "Instruction %s failed filter\n", i->format().c_str());
    return false;
  }
  bool findLoads;
  bool findStores;
  bool findPrefetch;
};

BPatch_point* BPatch_basicBlock::findEntryPointInt()
{
  return flowGraph->getAddSpace()->findOrCreateBPPoint(flowGraph->getFunction(),
                                                       instPoint::blockEntry(ifunc(), block()),
                                                       BPatch_locBasicBlockEntry);
}

// This should be edge instrumentation...
BPatch_point* BPatch_basicBlock::findExitPointInt()
{
  return flowGraph->getAddSpace()->findOrCreateBPPoint(flowGraph->getFunction(),
                                                       instPoint::blockExit(ifunc(), block()),
                                                       BPatch_locBasicBlockExit);
}

BPatch_Vector<BPatch_point*>*
BPatch_basicBlock::findPointByPredicate(insnPredicate& f)
{
  BPatch_Vector<BPatch_point*>* ret = new BPatch_Vector<BPatch_point*>;
  block_instance::Insns insns;
  block()->getInsns(insns);
  for (block_instance::Insns::iterator iter = insns.begin();
       iter != insns.end(); ++iter) {
    if(f(iter->second)) {
      instPoint *p = instPoint::preInsn(ifunc(), block(), iter->first, iter->second, true);
      BPatch_point *tmp = flowGraph->getAddSpace()->findOrCreateBPPoint(flowGraph->getFunction(),
                                                                        p,
                                                                        BPatch_locInstruction);
      if(!tmp) {
        fprintf(stderr, "WARNING: failed to create instpoint for load/store/prefetch %s at 0x%lx\n",
                iter->second->format().c_str(), iter->first);
      }
      else {
        ret->push_back(tmp);
      }
    }
  }
  return ret;
}

BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPointInt(const BPatch_Set<BPatch_opCode>& ops)
{

  // function is generally uninstrumentable (with current technology)
  if (!flowGraph->getFunction()->func->isInstrumentable())
    return NULL;

  findInsns filter(ops);
  return findPointByPredicate(filter);
}

BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPointInt(bool(*filter)(Instruction::Ptr))
{

  funcPtrPredicate filterPtr(filter);
  return findPointByPredicate(filterPtr);
}

// returns BPatch_point for an instPoint, unless the point isn't in this block
BPatch_point *BPatch_basicBlock::convertPoint(instPoint *pt)
{
  BPatch_point *bpPt = NULL;
  if (iblock == pt->block()) {
    bpPt = flowGraph->getFunction()->getAddSpace()->findOrCreateBPPoint
      ( flowGraph->getFunction(),
        pt,
        BPatch_point::convertInstPointType_t(pt->type()) );
  }
  return bpPt;
}

// does not return duplicates even if some points belong to multiple categories
//
void BPatch_basicBlock::getAllPoints(std::vector<BPatch_point*>& bpPoints)
{
  instPoint *entry = instPoint::blockEntry(ifunc(), iblock);
  instPoint *preCall = instPoint::preCall(ifunc(), iblock);
  // Exit 'point'?
  // Side-effect is creation, so we don't need to keep it around. 
  instPoint::postCall(ifunc(), iblock);
  instPoint *exit = instPoint::blockExit(ifunc(), iblock);

  if (entry) bpPoints.push_back(convertPoint(entry));
  // TODO bind pre- and post-call together
  if (preCall) bpPoints.push_back(convertPoint(preCall));
  if (exit) bpPoints.push_back(convertPoint(exit));
}


BPatch_function * BPatch_basicBlock::getCallTarget()
{
  func_instance *callee = lowlevel_block()->callee();
  if (!callee) return NULL;
  return flowGraph->addSpace->findOrCreateBPFunc(callee, NULL);
}


/*
 * BPatch_basicBlock::getInstructions
 *
 * Returns a vector of the instructions contained within this block
 *
 */

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

unsigned long BPatch_basicBlock::getStartAddressInt() CONST_EXPORT
{
  return iblock->start();
}

unsigned long BPatch_basicBlock::getLastInsnAddressInt() CONST_EXPORT
{
  return iblock->last();
}

unsigned long BPatch_basicBlock::getEndAddressInt() CONST_EXPORT
{
  return iblock->end();
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
  return (iblock->entryOfFunc() == ifunc());
}

bool BPatch_basicBlock::isExitBlockInt() CONST_EXPORT {
  return iblock->isFuncExit();
}

BPatch_flowGraph *BPatch_basicBlock::getFlowGraphInt() CONST_EXPORT {
  return flowGraph;
}

func_instance *BPatch_basicBlock::ifunc() CONST_EXPORT {
  return flowGraph->ll_func();
}

Dyninst::ParseAPI::Block *Dyninst::ParseAPI::convert(const BPatch_basicBlock *b) {
   return b->iblock->block();
}

Dyninst::PatchAPI::PatchBlock *Dyninst::PatchAPI::convert(const BPatch_basicBlock *b) {
   return b->iblock;
}
