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

#include "BPatch_dependenceGraphNode.h"
#include "BPatch_dependenceGraphEdge.h"

void BPatch_dependenceGraphNode::BPatch_dependenceGraphNodeInt(BPatch_instruction* inst) {
  BPatch_dependenceGraphNodeInt(inst,new BPatch_Vector<BPatch_dependenceGraphEdge*>(),new BPatch_Vector<BPatch_dependenceGraphEdge*>());
  /*
    bpinst=inst;
  incoming=
  outgoing=new BPatch_Vector<BPatch_dependenceGraphEdge*>();
  */
}

void BPatch_dependenceGraphNode::BPatch_dependenceGraphNodeInt(BPatch_instruction* inst,BPatch_Vector<BPatch_dependenceGraphEdge*>* inc,BPatch_Vector<BPatch_dependenceGraphEdge*>* out) {
  bpinst=inst;
  incoming=inc;
  outgoing = out;
  
  inter = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
}

BPatch_instruction* BPatch_dependenceGraphNode::getBPInstructionInt() {
  return bpinst;
}

void BPatch_dependenceGraphNode::getOutgoingEdgesInt(BPatch_Vector <BPatch_dependenceGraphEdge *>& out) {
  unsigned i;
  for(i=0; i<outgoing->size(); i++)
    out.push_back((*outgoing)[i]);
}

void BPatch_dependenceGraphNode::getIncomingEdgesInt(BPatch_Vector <BPatch_dependenceGraphEdge *>& inc) {
  unsigned i;  
  for(i=0; i<incoming->size(); i++)
    inc.push_back((*incoming)[i]);
}

bool BPatch_dependenceGraphNode::isImmediateSuccessorInt(BPatch_instruction* other_instruction) {
  unsigned int i;
  unsigned int size=outgoing->size();
  void* address = other_instruction->getAddress();
  for(i=0; i<size; i++) {
    if(bpinst != NULL && address == (*outgoing)[i]->getTarget()->bpinst->getAddress())
      return true;
  }
  return false;
}

bool BPatch_dependenceGraphNode::isImmediatePredecessorInt(BPatch_instruction* other_instruction) {
  unsigned int i;
  unsigned int size=incoming->size();
  void* address = other_instruction->getAddress();
  for(i=0; i<size; i++) {
    if(bpinst != NULL && address == (*incoming)[i]->getSource()->bpinst->getAddress())
      return true;
  }
  return false;
}

bool BPatch_dependenceGraphNode::addToOutgoing(BPatch_dependenceGraphNode* other_node) {
  BPatch_instruction* bp = other_node->getBPInstruction();
  if(bp!=NULL && isImmediateSuccessor(bp))
    return false;
  else {
    outgoing->push_back(new BPatch_dependenceGraphEdge(this,other_node));
    return true;
  }
}

bool BPatch_dependenceGraphNode::addToIncoming(BPatch_dependenceGraphNode* other_node) {
  BPatch_instruction* bp = other_node->getBPInstruction();
  if(bp != NULL && isImmediatePredecessor(bp))
    return false;
  else {
    incoming->push_back(new BPatch_dependenceGraphEdge(other_node,this));
    return true;
  }
}
