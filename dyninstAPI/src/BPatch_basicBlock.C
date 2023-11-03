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

#define BPATCH_FILE

#include <stdio.h>
#include <iostream>
#include <boost/bind/bind.hpp>

#include "util.h"

#include "BPatch_flowGraph.h"
#include "BPatch_collections.h"
#include "BPatch_basicBlock.h"
#include "function.h"
#include "BPatch_instruction.h"
#include "BPatch_point.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "BPatch_libInfo.h"
#include "BPatch_edge.h"
#include "instPoint.h"
#include "mapped_object.h"
#include "addressSpace.h"

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
BPatch_basicBlock::~BPatch_basicBlock(){
  if (immediatePostDominates)
    delete immediatePostDominates;
  if (immediateDominates)
    delete immediateDominates;
  if (sourceBlocks)
    delete sourceBlocks;
  if (instructions)
    delete instructions;

  std::set<BPatch_edge *>::iterator eIter;
  eIter = incomingEdges.begin();
  while (eIter != incomingEdges.end()) {
    delete (*eIter);
    eIter++;
  }
/* Don't delete outgoing edges; the target block will get them. */
#if 0
  eIter = outgoingEdges.begin();
  while (eIter != outgoingEdges.end()) {
    delete (*eIter);
    eIter++;
  }
#endif
  return;
}

void source_helper(ParseAPI::Edge* e,
		   BPatch_Vector<BPatch_basicBlock*>& srcs,
		   BPatch_flowGraph* flowGraph,
		   func_instance* func)
{
  BPatch_basicBlock* b = flowGraph->findBlock(func->obj()->findBlock(e->src()));
  assert(b);
  srcs.push_back(b);
}


// returns the predecessors of the basic block, provided they are in the same
// function, since our CFGs at the BPatch level are intraprocedural
void BPatch_basicBlock::getSources(BPatch_Vector<BPatch_basicBlock*>& srcs){
  //  BPatch_basicBlock *b;
  std::vector<block_instance *> in_blocks;
  // can't iterate over the PatchAPI cfg since that doesn't allow you to detect
  // edges from shared blocks into blocks that are not shared and not in the 
  // target block's function
  using namespace ParseAPI;
  const Block::edgelist &isrcs = iblock->llb()->sources();
  func_instance *func = flowGraph->getFunction()->lowlevel_func();
  SingleContext epred_(func->ifunc(),false,true);
  //Intraproc epred(&epred_);
  std::for_each(boost::make_filter_iterator(epred_, isrcs.begin(), isrcs.end()),
		boost::make_filter_iterator(epred_, isrcs.end(), isrcs.end()),
		boost::bind(source_helper,
			    boost::placeholders::_1,
			    boost::ref(srcs),
			    flowGraph,
			    func));
  
  /*
  for (Block::edgelist::const_iterator eit = isrcs.begin(&epred_); 
       eit != isrcs.end(&epred_); 
       ++eit) 
  {
    b = flowGraph->findBlock(func->obj()->findBlock((*eit)->src()));
    assert(b);
    srcs.push_back(b);
  }
  */
}

//returns the successors of the basic block in a set
void BPatch_basicBlock::getTargets(BPatch_Vector<BPatch_basicBlock*>& tgrts){
  BPatch_basicBlock *b;
  std::vector<block_instance *> out_blocks;
  const PatchBlock::edgelist &itrgs = iblock->targets();
  for (PatchBlock::edgelist::const_iterator iter = itrgs.begin(); iter != itrgs.end(); ++iter) {
    edge_instance* iedge = SCAST_EI(*iter);
    // We don't include interprocedural predecessors in the BPatch layer
    if (iedge->interproc() || iedge->sinkEdge()) continue;

    b = flowGraph->findBlock(iedge->trg());
    if (b) tgrts.push_back(b);
  }
}

//returns the dominates of the basic block in a set
void BPatch_basicBlock::getImmediateDominates(BPatch_Vector<BPatch_basicBlock*>& imds){
  flowGraph->fillDominatorInfo();

  if(!immediateDominates)
    return;

  imds.insert(imds.end(), immediateDominates->begin(), immediateDominates->end());

  return;
}


void BPatch_basicBlock::getImmediatePostDominates(BPatch_Vector<BPatch_basicBlock*>& imds){
  flowGraph->fillPostDominatorInfo();

  if(!immediatePostDominates)
    return;

  imds.insert(imds.end(), immediatePostDominates->begin(), immediatePostDominates->end());

  return;
}

//returns the dominates of the basic block in a set
void
BPatch_basicBlock::getAllDominates(std::set<BPatch_basicBlock*>& buffer){
  flowGraph->fillDominatorInfo();

  buffer.insert(this);
  if(immediateDominates){
     for (std::set<BPatch_basicBlock *>::iterator iter = immediateDominates->begin();
          iter != immediateDominates->end(); ++iter) {
        (*iter)->getAllDominates(buffer);
     }
  }

  return;
}

void BPatch_basicBlock::getAllDominates(BPatch_Set<BPatch_basicBlock *> &buffer) {
   std::set<BPatch_basicBlock *> tmp;
   getAllDominates(tmp);

   std::copy(tmp.begin(), tmp.end(), std::inserter(buffer.int_set, buffer.begin()));
}

void
BPatch_basicBlock::getAllPostDominates(std::set<BPatch_basicBlock*>& buffer){
  flowGraph->fillPostDominatorInfo();

  buffer.insert(this);
  if(immediatePostDominates){
     for (std::set<BPatch_basicBlock *>::iterator iter = immediatePostDominates->begin();
          iter != immediatePostDominates->end(); ++iter) {
        (*iter)->getAllPostDominates(buffer);
     }
  }
  return;
}

void
BPatch_basicBlock::getAllPostDominates(BPatch_Set<BPatch_basicBlock*>& buffer){
   std::set<BPatch_basicBlock *> tmp;
   getAllPostDominates(tmp);
   std::copy(tmp.begin(), tmp.end(), std::inserter(buffer.int_set, buffer.begin()));
}


//returns the immediate dominator of the basic block
BPatch_basicBlock* BPatch_basicBlock::getImmediateDominator(){
  flowGraph->fillDominatorInfo();

  return immediateDominator;
}

BPatch_basicBlock* BPatch_basicBlock::getImmediatePostDominator(){
  flowGraph->fillPostDominatorInfo();

  return immediatePostDominator;
}

//returns whether this basic block dominates the argument
bool BPatch_basicBlock::dominates(BPatch_basicBlock* bb){
  if(!bb)
    return false;

  if(bb == this)
    return true;

  flowGraph->fillDominatorInfo();

  if(!immediateDominates)
    return false;

  for (std::set<BPatch_basicBlock *>::iterator iter = immediateDominates->begin();
       iter != immediateDominates->end(); ++iter) {
     if ((*iter)->dominates(bb)) return true;
  }
  return false;
}

bool BPatch_basicBlock::postdominates(BPatch_basicBlock* bb){
  if(!bb)
    return false;

  if(bb == this)
    return true;

  flowGraph->fillPostDominatorInfo();

  if(!immediatePostDominates)
    return false;

  for (std::set<BPatch_basicBlock *>::iterator iter = immediatePostDominates->begin();
       iter != immediatePostDominates->end(); ++iter) {
     if ((*iter)->postdominates(bb)) return true;
  }
  return false;
}

//returns the source block corresponding to the basic block
//which is created looking at the machine code.
bool
BPatch_basicBlock::getSourceBlocks(BPatch_Vector<BPatch_sourceBlock*>& sBlocks)
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
int BPatch_basicBlock::getBlockNumber() {
  return iblock->id();
}

// returns the range of addresses of the code for the basic block
bool BPatch_basicBlock::getAddressRange(void*& _startAddress,
                                           void*& _lastInsnAddress)
{
  _startAddress = (void *) getStartAddress();
  _lastInsnAddress = (void *) getLastInsnAddress();
  return true;
}

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

  os << "Immediate Dominates: ";
  if(bb.immediateDominates){
     for (std::set<BPatch_basicBlock *>::iterator iter = bb.immediateDominates->begin();
          iter != bb.immediateDominates->end(); ++iter) {
        os << (*iter)->blockNo() << " ";
     }
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
bool isLoad(Instruction i)
{
  return i.readsMemory();
}
bool isStore(Instruction i)
{
  return i.writesMemory();
}
bool isPrefetch(Instruction i)
{
  return i.getCategory() == c_PrefetchInsn;
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

   findInsns(const std::set<BPatch_opCode>& ops)
      : findLoads(false), findStores(false), findPrefetch(false) {
      for (std::set<BPatch_opCode>::iterator iter = ops.begin();
           iter != ops.end(); ++iter) {
         switch(*iter) {
            case BPatch_opLoad: findLoads = true; break;
            case BPatch_opStore: findStores = true; break;
            case BPatch_opPrefetch: findPrefetch = true; break;
         }            
      }
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

BPatch_point* BPatch_basicBlock::findEntryPoint()
{
  return flowGraph->getAddSpace()->findOrCreateBPPoint(flowGraph->getFunction(),
                                                       instPoint::blockEntry(ifunc(), block()),
                                                       BPatch_locBasicBlockEntry);
}

// This should be edge instrumentation...
BPatch_point* BPatch_basicBlock::findExitPoint()
{
  return flowGraph->getAddSpace()->findOrCreateBPPoint(flowGraph->getFunction(),
                                                       instPoint::blockExit(ifunc(), block()),
                                                       BPatch_locBasicBlockExit);
}

BPatch_point *BPatch_basicBlock::findPoint(Address addr) 
{
   // We verify internally.
   instPoint *p = instPoint::preInsn(ifunc(), block(), addr);

   if (!p) return NULL;
   return flowGraph->getAddSpace()->findOrCreateBPPoint(flowGraph->getFunction(),
                                                        p,
                                                        BPatch_locInstruction);
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
                iter->second.format().c_str(), iter->first);
      }
      else {
        ret->push_back(tmp);
      }
    }
  }
  return ret;
}

BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPoint(const std::set<BPatch_opCode>& ops)
{

  // function is generally uninstrumentable (with current technology)
  if (!flowGraph->getFunction()->func->isInstrumentable())
    return NULL;

  findInsns filter(ops);
  return findPointByPredicate(filter);
}

BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPoint(const BPatch_Set<BPatch_opCode>& ops)
{

  // function is generally uninstrumentable (with current technology)
  if (!flowGraph->getFunction()->func->isInstrumentable())
    return NULL;

  findInsns filter(ops);
  return findPointByPredicate(filter);
}

BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPoint(bool(*filter)(Instruction))
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

bool BPatch_basicBlock::getInstructions(std::vector<InstructionAPI::Instruction>& insns) {
  using namespace InstructionAPI;

  InstructionDecoder d((const unsigned char*)
                       (iblock->proc()->getPtrToInstruction(getStartAddress())),
                       size(),
                       iblock->llb()->obj()->cs()->getArch());
  do {
    insns.push_back(d.decode());
  } while (insns.back().isValid());

  // Remove final invalid instruction
  if (!insns.empty()) insns.pop_back();

  return !insns.empty();
}

bool BPatch_basicBlock::getInstructions(std::vector<std::pair<InstructionAPI::Instruction, Address> >& insnInstances) {
  using namespace InstructionAPI;
  Address addr = getStartAddress();
  const unsigned char *ptr = (const unsigned char *)iblock->proc()->getPtrToInstruction(addr);
  if (ptr == NULL) return false;
  InstructionDecoder d(ptr, size(), iblock->llb()->obj()->cs()->getArch());

  while (addr < getEndAddress()) {
    insnInstances.push_back(std::make_pair(d.decode(), addr));
    addr += insnInstances.back().first.size();
  }

  return !insnInstances.empty();
}

unsigned long BPatch_basicBlock::getStartAddress() const
{
  return iblock->start();
}

unsigned long BPatch_basicBlock::getLastInsnAddress() const
{
  return iblock->last();
}

unsigned long BPatch_basicBlock::getEndAddress() const
{
  return iblock->end();
}

unsigned BPatch_basicBlock::size() const
{
  return getEndAddress() - getStartAddress();
}

void BPatch_basicBlock::getIncomingEdges(BPatch_Vector<BPatch_edge*>& inc)
{
  std::set<BPatch_edge*>::iterator incIter = incomingEdges.begin();
  while (incIter != incomingEdges.end()) {
    inc.push_back(*incIter);
    incIter++;
  }
}

void BPatch_basicBlock::getOutgoingEdges(BPatch_Vector<BPatch_edge*>& out)
{
  std::set<BPatch_edge*>::iterator outIter = outgoingEdges.begin();
  while (outIter != outgoingEdges.end()) {
    out.push_back(*outIter);
    outIter++;
  }
}

int BPatch_basicBlock::blockNo() const
{
  return iblock->id();
}

bool BPatch_basicBlock::isEntryBlock() const {
  return (iblock->entryOfFunc() == ifunc());
}

bool BPatch_basicBlock::isExitBlock() const {
  return iblock->isFuncExit();
}

BPatch_flowGraph *BPatch_basicBlock::getFlowGraph() const {
  return flowGraph;
}

func_instance *BPatch_basicBlock::ifunc() const {
  return flowGraph->ll_func();
}

Dyninst::ParseAPI::Block *Dyninst::ParseAPI::convert(const BPatch_basicBlock *b) {
   return b->iblock->block();
}

Dyninst::PatchAPI::PatchBlock *Dyninst::PatchAPI::convert(const BPatch_basicBlock *b) {
   return b->iblock;
}

bool comparison<BPatch_basicBlock *>::operator()(const BPatch_basicBlock * const &x, 
                                                 const BPatch_basicBlock * const &y) const {
   return (x->getStartAddress() < y->getStartAddress());
}

bool std::less<BPatch_basicBlock *>::operator()(const BPatch_basicBlock * const &l,
                                                const BPatch_basicBlock * const &r) const {
   return (l->getStartAddress() < r->getStartAddress());
}


