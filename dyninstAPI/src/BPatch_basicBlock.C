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

/* Bit array functions for liveness calculation */

void dispose(BITARRAY** bitarray)
{
  if(*bitarray){
    if((*bitarray)->data){
      free((*bitarray)->data);
      (*bitarray)->data = NULL;
    }
    free(*bitarray);
    *bitarray = NULL;
  }
}

void bitarray_init(int size,BITARRAY* bitarray)
{
	int bytesize;

	bitarray->size = size;
	if(!(size % 8))
		bytesize = size / 8;
	else
		bytesize = (size / 8) + 1;

	bitarray->data = (char*) malloc (sizeof(char)*bytesize);

	memset(bitarray->data,'\0',bytesize);
}
void bitarray_set(int location,BITARRAY* bitarray)
{
	int pass,pos;
	char* ptr;
	unsigned char mask = 0x80;
	
	if(location > bitarray->size){
		fprintf(stderr,"trying to set a bit more than the size\n");
		return;
	}
	pass = location / 8;
	pos = location % 8;

	ptr = bitarray->data + pass;
	mask = mask >> pos;

	*ptr = (*ptr) | mask;
}
void bitarray_unset(int location,BITARRAY* bitarray)
{
        int pass,pos;
        char* ptr;
        unsigned char mask = 0x80;
        
        if(location > bitarray->size){
                fprintf(stderr,"trying to unset a bit more than the size\n");
                return;
        }
        pass = location / 8;
        pos = location % 8;
 
        ptr = bitarray->data + pass;
        mask = mask >> pos;
 
        *ptr = (*ptr) & (~mask);
}
int bitarray_check(int location,BITARRAY* bitarray)
{
        int pass,pos;
        char* ptr;
        char unsigned mask = 0x80;
        
	if(location < 0)
		return FALSE;

        if(location > bitarray->size){
                fprintf(stderr,"trying to unset a bit more than the size\n");
                return FALSE;
        }
        pass = location / 8;
        pos = location % 8;

	ptr = bitarray->data + pass;
	mask = mask >> pos;

	if((*ptr) & mask)
		return TRUE;
	return FALSE;
}

void bitarray_and(BITARRAY* s1,BITARRAY* s2,BITARRAY* r)
{
	int bytesize,i;

	if(s1->size != s2->size){
		fprintf(stderr,"size do not match to and \n");
		return ;
	}
	if(s1->size % 8)
		bytesize = (s1->size / 8)+1;
	else
		bytesize = s1->size / 8;
	r->size = s1->size;
	for(i=0;i<bytesize;i++)
		*((r->data)+i) = *((s1->data)+i) & *((s2->data)+i);
}
void bitarray_or(BITARRAY* s1,BITARRAY* s2,BITARRAY* r)
{
	int bytesize,i;

	if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;
        r->size = s1->size;
        for(i=0;i<bytesize;i++)
                *((r->data)+i) = *((s1->data)+i) | *((s2->data)+i);
}
void bitarray_copy(BITARRAY* s1,BITARRAY* s2)
{
	int bytesize,i;

        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;

	for(i=0;i<bytesize;i++)
		*((s1->data)+i) = *((s2->data)+i);
}
void bitarray_comp(BITARRAY* s1,BITARRAY* s2)
{
        int bytesize,i;

        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;

        for(i=0;i<bytesize;i++)
                *((s1->data)+i) = ~(*((s2->data)+i));
}
int bitarray_same(BITARRAY* s1,BITARRAY* s2)
{
	int bytesize,i;

        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return FALSE;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;
	for(i=0;i<bytesize;i++)
		if(*((s1->data)+i) ^ *((s2->data)+i))
			return FALSE;
	return TRUE;
}
	
void bitarray_diff(BITARRAY* s1,BITARRAY* s2,BITARRAY* r)
{
        int bytesize,i;
 
        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;
        r->size = s1->size;
        for(i=0;i<bytesize;i++)
		*((r->data)+i) = *((s1->data)+i) & (~(*((s2->data)+i)));
}


#if defined (arch_ia64)
BPatch_Set< BPatch_basicBlock * > *BPatch_basicBlock::getDataFlowInInt() CONST_EXPORT
{
  return dataFlowIn;
}
BPatch_Set< BPatch_basicBlock * > * BPatch_basicBlock::getDataFlowOutInt() CONST_EXPORT
{
  return dataFlowOut;
}
void BPatch_basicBlock::setDataFlowInInt(BPatch_Set< BPatch_basicBlock * > *dfi)
{
  dataFlowIn = dfi;
}
void BPatch_basicBlock::setDataFlowOutInt(BPatch_Set< BPatch_basicBlock * > *dfo)
{
  dataFlowOut = dfo;
}
BPatch_basicBlock * BPatch_basicBlock::getDataFlowGenInt() CONST_EXPORT
{
   return dataFlowGen;
}
void BPatch_basicBlock::setDataFlowGenInt(BPatch_basicBlock *dfg)
{
  dataFlowGen = dfg;
}
BPatch_basicBlock *BPatch_basicBlock::getDataFlowKillInt() CONST_EXPORT
{
  return dataFlowKill;
}
void BPatch_basicBlock::setDataFlowKillInt(BPatch_basicBlock *dfk)
{
  dataFlowKill = dfk;
}
#endif

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


/* All of these functions are only defined for power right now
   because they involve iterating over individual instructions
   that don't have corresponding functions in all the
   InstrucIter-*  yest. 
*/
#if defined(arch_power)

/* Iterates over instructions in the basic block to 
   create the initial gen kill sets for that block */
bool BPatch_basicBlock::initRegisterGenKillInt() 
{  
   in=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,in);  

   out=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,out);  

   gen=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,gen);  
  
   kill=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,kill);  

   inFP=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,inFP);  

   outFP=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,outFP);  

   genFP=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,genFP);  
  
   killFP=(BITARRAY*)malloc(sizeof(BITARRAY));
   bitarray_init(BITARRAY_SIZE,killFP);  

  
   InstrucIter ii(this);
  
  
   while(ii.hasMore()) {
      /* GPR Gens */
      if (ii.isA_RT_ReadInstruction()){
         if (!bitarray_check(ii.getRTValue(),kill))
            bitarray_set(ii.getRTValue(),gen);
      }
      if (ii.isA_RA_ReadInstruction()){
         if (!bitarray_check(ii.getRAValue(),kill))
            bitarray_set(ii.getRAValue(),gen);
      }
      if (ii.isA_RB_ReadInstruction()){
         if (!bitarray_check(ii.getRBValue(),kill))
            bitarray_set(ii.getRBValue(),gen);
      }

      /* FPR Gens */
      if (ii.isA_FRT_ReadInstruction()){
         if (!bitarray_check(ii.getFRTValue(),killFP))
            bitarray_set(ii.getFRTValue(),genFP);
      }
      if (ii.isA_FRA_ReadInstruction()){
         if (!bitarray_check(ii.getFRAValue(),killFP))
            bitarray_set(ii.getFRAValue(),genFP);
      }
      if (ii.isA_FRB_ReadInstruction()){
         if (!bitarray_check(ii.getFRBValue(),killFP))
            bitarray_set(ii.getFRBValue(),genFP);
      }
      if (ii.isA_FRC_ReadInstruction()){
         if (!bitarray_check(ii.getFRCValue(),killFP))
            bitarray_set(ii.getFRCValue(),genFP);
      }

      /* GPR Kills */
      if (ii.isA_RT_WriteInstruction()){
         bitarray_set(ii.getRTValue(),kill);
      }    
      if (ii.isA_RA_WriteInstruction()){
         bitarray_set(ii.getRAValue(),kill);
      }

      /* FPR Kills */
      if (ii.isA_FRT_WriteInstruction()){
         bitarray_set(ii.getFRTValue(),killFP);
      }    
      if (ii.isA_FRA_WriteInstruction()){
         bitarray_set(ii.getFRAValue(),killFP);
      }

      if (ii.isAReturnInstruction()){
         /* Need to gen the possible regurn arguments */
         bitarray_set(3,gen);
         bitarray_set(4,gen);
         bitarray_set(1,genFP);
         bitarray_set(2,genFP);
      }

    
      /* If it is a call instruction we look at which registers are used
         at the beginning of the called function, If we can't do that, we just
         gen registers 3-10 (the parameter holding volative 
         registers for power) & (1-13 for FPR)*/
      if (ii.isACallInstruction())
      {
         Address callAddress = ii.getBranchTargetAddress();
         //printf("Call Address is 0x%x \n",callAddress);

         process *proc = flowGraph->getBProcess()->lowlevel_process();
         int_function * funcc;
          
         codeRange * range = proc->findCodeRangeByAddress(callAddress);
          
         if (range)
         {
            funcc = range->is_function();
            if (funcc)
            {
               InstrucIter ah(funcc);
               while (ah.hasMore())
               {
                  // GPR
                  if (ah.isA_RT_ReadInstruction()){
                     bitarray_set(ah.getRTValue(),gen);
                  }
                  if (ah.isA_RA_ReadInstruction()){
                     bitarray_set(ah.getRAValue(),gen);
                  }
                  if (ah.isA_RB_ReadInstruction()){
                     bitarray_set(ah.getRBValue(),gen);
                  }

                  // FPR
                  if (ah.isA_FRT_ReadInstruction()){
                     bitarray_set(ah.getFRTValue(),genFP);
                  }
                  if (ah.isA_FRA_ReadInstruction()){
                     bitarray_set(ah.getFRAValue(),genFP);
                  }
                  if (ah.isA_FRB_ReadInstruction()){
                     bitarray_set(ah.getFRBValue(),genFP);
                  }
                  if (ah.isA_FRC_ReadInstruction()){
                     bitarray_set(ah.getFRCValue(),genFP);
                  }
                  if (ah.isACallInstruction())
                     break;
                                  
                  ah++;
               }
            }
            else
            {
               for (int a = 3; a <= 10; a++)
                  bitarray_set(a,gen);
               for (int a = 1; a <= 13; a++)
                  bitarray_set(a,genFP);
            }
         }
         else
         {
            for (int a = 3; a <= 10; a++)
               bitarray_set(a,gen);
            for (int a = 1; a <= 13; a++)
               bitarray_set(a,genFP);
         }
      } 
      ii++;
   } 
   return true;
}

/* This is used to do fixed point iteration until 
   the in and out don't change anymore */
bool BPatch_basicBlock::updateRegisterInOutInt(bool isFP) 
{
   bool change = false;
   // old_IN = IN(X)
  
   BITARRAY oldIn;

   BITARRAY tmp; 
  
   bitarray_init(BITARRAY_SIZE, &oldIn);
   bitarray_init(BITARRAY_SIZE, &tmp);

   if (!isFP)
   {
      bitarray_copy(&oldIn, in);
      bitarray_and(in, &tmp, in);
   }
   else
   {
      bitarray_copy(&oldIn, inFP);
      bitarray_and(inFP, &tmp, inFP);
   }

   // OUT(X) = UNION(IN(Y)) for all successors Y of X
   pdvector<int_basicBlock *> elements;
   iblock->getTargets(elements);
   for(unsigned  i=0;i<elements.size();i++)
   {
      BPatch_basicBlock *bb = (BPatch_basicBlock*) elements[i]->getHighLevelBlock();
      if (!isFP)
         bitarray_or(&tmp,bb->getInSet(),&tmp);
      else
         bitarray_or(&tmp,bb->getInFPSet(),&tmp);
   }
  
   if (!isFP)
      bitarray_copy(out, &tmp);
   else
      bitarray_copy(outFP, &tmp);


   // IN(X) = USE(X) + (OUT(X) - DEF(X))
   if (!isFP)
   {
      bitarray_diff(out, kill, &tmp);
      bitarray_or(&tmp, gen, in);
   }
   else
   {
      bitarray_diff(outFP, killFP, &tmp);
      bitarray_or(&tmp, genFP, inFP);
   }
  
   // if (old_IN != IN(X)) then change = true
   if (!isFP)
   {
      if (bitarray_same(&oldIn, in))
         change = false;
      else
         change = true;
   }
   else
   {
      if (bitarray_same(&oldIn, inFP))
         change = false;
      else
         change = true;
   }
   return change;
}


BITARRAY * BPatch_basicBlock::getInSetInt()
{
  return in;
}

BITARRAY * BPatch_basicBlock::getInFPSetInt()
{
  return inFP;
}

/* Debugging tool for checking basic block/liveness info */
bool BPatch_basicBlock::printAllInt()
{
   cout << this;
	return true;
}

/* For Power it checks the basic block for any usage of the MQ
   SPR, when other platforms are done we'll probably need to move
   this into inst-* files
   Right now it isn't really doing liveness, but just checks
   to see if any instruction uses MQ ... it should be fairly
   rare.  If there are alot of hits we can do full liveness
   analysis on it and other SPR
*/
int BPatch_basicBlock::liveSPRegistersIntoSetInt(int *& liveSPReg, 
					       unsigned long address)
{
  if (liveSPReg == NULL)
    {
      liveSPReg = new int[1]; // only care about MQ for Power for now
      liveSPReg[0] = 0;
      InstrucIter ii(this);
      
      while (ii.hasMore() &&
	     *ii <= address)
	{
	  if (ii.isA_MX_Instruction())
	    {
	      liveSPReg[0] = 1;
	      break;
	    }
	  ii++;
	}
        return liveSPReg[0];
    }
  return -1;
}




/* The liveReg int * is a instance variable in the instPoint classes.
   This puts the liveness information into that variable so 
   we can access it from every instPoint without recalculation */
int BPatch_basicBlock::liveRegistersIntoSetInt(int *& liveReg, 
					       int *& liveFPReg,
					       unsigned long address)
{
  int numLive = 0;
  if (liveReg == NULL)
    {
      liveReg = new int[BITARRAY_SIZE];
      liveFPReg = new int[BITARRAY_SIZE];

      BITARRAY newIn;
      BITARRAY newInFP;
  
      bitarray_init(BITARRAY_SIZE, &newIn);
      bitarray_copy(&newIn,in);

      bitarray_init(BITARRAY_SIZE, &newInFP);
      bitarray_copy(&newInFP,inFP);
      
      InstrucIter ii(this);

      /* The liveness information from the bitarrays are for the
	 basic block, we need to do some more gen/kills until
	 we get to the individual instruction within the 
	 basic block that we want the liveness info for. */
      
      while(ii.hasMore() &&
            *ii <= address)
	{
	  if (ii.isA_RT_WriteInstruction())
	    bitarray_set(ii.getRTValue(),&newIn);
	  if (ii.isA_RA_WriteInstruction())
	    bitarray_set(ii.getRAValue(),&newIn);

	  if (ii.isA_FRT_WriteInstruction())
	    bitarray_set(ii.getFRTValue(),&newInFP);
	  if (ii.isA_FRA_WriteInstruction())
	    bitarray_set(ii.getFRAValue(),&newInFP);
	  ii++;
	}    
      numLive = 0;
      for (int a = 0; a < BITARRAY_SIZE; a++)
	{
	  if (bitarray_check(a,&newIn))
	    {
	      //printf("1 ");
	      liveReg[a] = 1;
	      numLive++;
	    }
	  else
	    {
	      //printf("0 ");
	      liveReg[a] = 0;
	    }
	  if (bitarray_check(a,&newInFP))
	    {
	      //printf("1 ");
	      liveFPReg[a] = 1;
	    }
	  else
	    {
	      //printf("0 ");
	      liveFPReg[a] = 0;
	    }
	} 
      //printf("\n");
    }

  return numLive;
}

#endif 



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
