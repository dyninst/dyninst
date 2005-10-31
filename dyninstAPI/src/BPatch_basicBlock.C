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
#include "InstrucIter.h"
#include "BPatch_instruction.h"
#include "BPatch_libInfo.h"


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
BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPointInt(const BPatch_Set<BPatch_opCode>& ops) {

    // function is generally uninstrumentable (with current technology)
    if (!flowGraph->getBFunction()->func->isInstrumentable())
        return NULL;
    
    // Use an instruction iterator
    InstrucIter ii(this);
    BPatch_function *func = flowGraph->getBFunction();
    
    return BPatch_point::getPoints(ops, ii, func);
}

/*
 * BPatch_basicBlock::getInstructions
 *
 * Returns a vector of the instructions contained within this block
 *
 */

BPatch_Vector<BPatch_instruction*> *BPatch_basicBlock::getInstructionsInt(void) {

  if (!instructions) {

    instructions = new BPatch_Vector<BPatch_instruction*>;
    InstrucIter ii(this);
    
    while(ii.hasMore()) {
      BPatch_instruction *instr = ii.getBPInstruction();
      instructions->push_back(instr);
      ii++;
    }
  }

  return instructions;
}

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
    BPatch_edge **elements = new BPatch_edge*[incomingEdges.size()];
    incomingEdges.elements(elements);
    for(unsigned i = 0; i < incomingEdges.size(); i++)
        inc.push_back(elements[i]);
    delete[] elements;
}
        
void BPatch_basicBlock::getOutgoingEdgesInt(BPatch_Vector<BPatch_edge*>& out)
{
    BPatch_edge **elements = new BPatch_edge*[outgoingEdges.size()];
    outgoingEdges.elements(elements);
    for(unsigned i = 0; i < outgoingEdges.size(); i++)
        out.push_back(elements[i]);
    delete[] elements;
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
