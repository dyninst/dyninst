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
