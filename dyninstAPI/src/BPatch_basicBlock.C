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
#include "common/h/Types.h"

#include "BPatch_flowGraph.h"
#include "BPatch_collections.h"
#include "BPatch_basicBlock.h"
#include "process.h"
#include "InstrucIter.h"
#include "BPatch_instruction.h"

extern BPatch_Vector<BPatch_point*> *findPoint(const BPatch_Set<BPatch_opCode>& ops,
					       InstrucIter &ii, 
					       process *proc,
					       BPatch_function *bpf);


//constructor that creates the empty sets for 
//class fields.

BPatch_basicBlock::BPatch_basicBlock() : 
        isEntryBasicBlock(false),
        isExitBasicBlock(false),
        startAddress(0),
        lastInsnAddress(0),
        endAddr(0), 						       
		immediateDominates(NULL),
		immediateDominator(NULL),
		immediatePostDominates(NULL),
		immediatePostDominator(NULL),
		sourceBlocks(NULL),
		instructions(NULL) {}

//The argument is a block number.
//there is no limitation that two blocks can have 
//the same number but since only way the basic blocks 
//can be created is through flowGraph class there will never exist
//two  basic blocks with the same number

BPatch_basicBlock::BPatch_basicBlock(BPatch_flowGraph* fg, int bno) :
		flowGraph(fg),
		blockNumber(bno),
		isEntryBasicBlock(false),
		isExitBasicBlock(false),
        startAddress(0),
        lastInsnAddress(0),
        endAddr(0),    
		immediateDominates(NULL),
		immediateDominator(NULL),
		immediatePostDominates(NULL),
		immediatePostDominator(NULL),
		sourceBlocks(NULL),
        instructions(NULL) {}


//destructor of the class BPatch_basicBlock
BPatch_basicBlock::~BPatch_basicBlock(){
	if (immediatePostDominates)
		delete immediatePostDominates;
	if (immediateDominates)
		delete immediateDominates;
	if (sourceBlocks)
		delete sourceBlocks;
}

//returns the predecessors of the basic block in aset 
void BPatch_basicBlock::getSources(BPatch_Vector<BPatch_basicBlock*>& srcs){
	BPatch_basicBlock** elements = new BPatch_basicBlock*[sources.size()];
	sources.elements(elements);
	for(unsigned i=0;i<sources.size();i++)
		srcs.push_back(elements[i]);
	delete[] elements;
}

//returns the successors of the basic block in a set 
void BPatch_basicBlock::getTargets(BPatch_Vector<BPatch_basicBlock*>& tgrts){
	BPatch_basicBlock** elements = new BPatch_basicBlock*[targets.size()];
	targets.elements(elements);
	for(unsigned  i=0;i<targets.size();i++)
		tgrts.push_back(elements[i]);
	delete[] elements;
}

//returns the dominates of the basic block in a set 
void BPatch_basicBlock::getImmediateDominates(BPatch_Vector<BPatch_basicBlock*>& imds){
	flowGraph->fillDominatorInfo();

	if(!immediateDominates)
		return;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediateDominates->size()];
	immediateDominates->elements(elements);
	for(unsigned i=0;i<immediateDominates->size();i++)
		imds.push_back(elements[i]);
	delete[] elements;
}


void BPatch_basicBlock::getImmediatePostDominates(BPatch_Vector<BPatch_basicBlock*>& imds){
	flowGraph->fillPostDominatorInfo();

	if(!immediatePostDominates)
		return;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediatePostDominates->size()];
	immediatePostDominates->elements(elements);
	for(unsigned i=0;i<immediatePostDominates->size();i++)
		imds.push_back(elements[i]);
	delete[] elements;
}

//returns the dominates of the basic block in a set 
void
BPatch_basicBlock::getAllDominates(BPatch_Set<BPatch_basicBlock*>& buffer){
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
}

void
BPatch_basicBlock::getAllPostDominates(BPatch_Set<BPatch_basicBlock*>& buffer){
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

	bool done = false;
	BPatch_basicBlock** elements = 
		new BPatch_basicBlock*[immediateDominates->size()];
	immediateDominates->elements(elements);
	for(unsigned i=0;!done && (i<immediateDominates->size());i++)
		done = done || elements[i]->dominates(bb);
	delete[] elements;
	return done;
}

bool BPatch_basicBlock::postdominates(BPatch_basicBlock* bb){
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
void
BPatch_basicBlock::getSourceBlocks(BPatch_Vector<BPatch_sourceBlock*>& sBlocks)
{
	flowGraph->createSourceBlocks();
	if(!sourceBlocks)
		return;
	for(unsigned int i=0;i<sourceBlocks->size();i++)
		sBlocks.push_back((*sourceBlocks)[i]);
}

//returns the block number of the basic block
int BPatch_basicBlock::getBlockNumber(){
	return blockNumber;
}


//sets the block number of the basic block
void BPatch_basicBlock::setBlockNumber(int bno){
	blockNumber = bno;
}

// returns the range of addresses of the code for the basic block
bool BPatch_basicBlock::getAddressRange(void*& _startAddress,
                                        void*& _lastInsnAddress)
{
	_startAddress = (void *)getStartAddress();
	_lastInsnAddress =(void *)(getStartAddress()-startAddress+lastInsnAddress);

	return true;
}

#ifdef IBM_BPATCH_COMPAT
bool BPatch_basicBlock::getLineNumbers(unsigned int &_startLine,
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

#ifdef DEBUG
//print method
ostream& operator<<(ostream& os,BPatch_basicBlock& bb)
{
	os << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
	os << "Basic Block : " << bb.blockNumber <<" : [ ";
	os << ostream::hex << bb.startAddress << " , ";
	os << ostream::hex << bb.lastInsnAddress << " | ";
	os << ostream::dec << bb.lastInsnAddress-bb.startAddress<< " ]\n";

	if(bb.isEntryBasicBlock)
		os <<"Type : ENTRY TO CFG\n"; 
	else if(bb.isExitBasicBlock)
		os <<"Type : EXIT FROM CFG\n"; 

	os << "Pred :\n";

	BPatch_basicBlock** belements = new BPatch_basicBlock*[bb.sources.size()];
	bb.sources.elements(belements);
	for(int i=0; i<bb.sources.size(); i++)
		os << "\t<- " << belements[i]->blockNumber << "\n";
	delete[] belements;

	os << "Succ:\n";
	belements =  new BPatch_basicBlock*[bb.targets.size()];
	bb.targets.elements(belements);
	for(int j=0; j<bb.targets.size(); j++)
		os << "\t-> " << belements[j]->blockNumber << "\n";
	delete[] belements;

	os << "Immediate Dominates: ";
	if(bb.immediateDominates){
		belements = new BPatch_basicBlock*[bb.immediateDominates->size()];
		bb.immediateDominates->elements(belements);
		for(int k=0; k<bb.immediateDominates->size(); k++)
			os << belements[k]->blockNumber << " ";
		delete[] belements;
	}
	os << "\n";

	os << "Immediate Dominator: ";
	if(!bb.immediateDominator)
		os << "None\n";
	else
		os << bb.immediateDominator->blockNumber << "\n";

	os << "Source Block:\n";
	if(bb.sourceBlocks){
		for(unsigned l=0; l<bb.sourceBlocks->size(); l++)
			os << *((*(bb.sourceBlocks))[l]);
	}

	os << "\n";
	os << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
	return os;
}
#endif

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
BPatch_Vector<BPatch_point*> *BPatch_basicBlock::findPoint(const BPatch_Set<BPatch_opCode>& ops) {

  // function is generally uninstrumentable (with current technology)
  if (flowGraph->getFunction()->funcEntry(flowGraph->getProcess()) == NULL)
     return NULL;

  // Use an instruction iterator
  InstrucIter ii(this);

  return ::findPoint(ops, ii, flowGraph->getProcess(), 
                     flowGraph->getProcess()->PDFuncToBPFuncMap.get(const_cast<function_base *>(flowGraph->getFunction())));

}

/*
 * BPatch_basicBlock::getInstructions
 *
 * Returns a vector of the instructions contained within this block
 *
 */

BPatch_Vector<BPatch_instruction*> *BPatch_basicBlock::getInstructions(void) {

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

unsigned long BPatch_basicBlock::getStartAddress() const
{
    Address imgBaseAddr = 0;   
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) 
    
     //all other platforms have absolute addresses for basic blocks
     //at this point
     process *proc = flowGraph->getProcess();
     image *img = flowGraph->getModule()->exec();
                                                                               
     if (!proc->getBaseAddress(img, imgBaseAddr)) {
        bperr( "getBaseAddress error\n");
        abort();
     }
#endif
     return startAddress + imgBaseAddr;
}

unsigned long BPatch_basicBlock::getLastInsnAddress() const
{
    Address imgBaseAddr = 0;   
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) 
    
     //all other platforms have absolute addresses for basic blocks
     //at this point
     process *proc = flowGraph->getProcess();
     image *img = flowGraph->getModule()->exec();
                                                                               
     if (!proc->getBaseAddress(img, imgBaseAddr)) {
        fprintf(stdout /* ? */, "getBaseAddress error\n");
        abort();
     }
#endif
     return lastInsnAddress + imgBaseAddr;
}


unsigned BPatch_basicBlock::size() const
{
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) 
    //variable length instructions on x86
    return endAddr - startAddress;
#elif defined(ia64_unknown_linux2_0 )
    //16 byte bundly size on ia64
    return 16 + lastInsnAddress - startAddress;
#else
    //4 byte instructions on all other platforms
    return 4 + lastInsnAddress - startAddress;
#endif

}

unsigned long BPatch_basicBlock::getRelStart() const
{
    return startAddress;
}

unsigned long BPatch_basicBlock::getRelEnd() const
{
    return endAddr;
}

unsigned long BPatch_basicBlock::getRelLast() const
{
    return lastInsnAddress;
}
