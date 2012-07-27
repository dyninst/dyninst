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

#ifndef _BPatch_dependenceGraphNode_h_
#define _BPatch_dependenceGraphNode_h_

#include "BPatch_instruction.h"
#include "BPatch_Vector.h"
#include "BPatch_eventLock.h"
//#include "BPatch_dependenceGraphEdge.h"

class BPatch_dependenceGraphEdge;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_dependenceGraphNode

class BPATCH_DLL_EXPORT BPatch_dependenceGraphNode: public BPatch_eventLock {


  BPatch_instruction* bpinst;
  BPatch_Vector<BPatch_dependenceGraphEdge*>* incoming;
  BPatch_Vector<BPatch_dependenceGraphEdge*>* outgoing;

 public:
  BPatch_Vector<BPatch_dependenceGraphEdge*>* inter;

  // BPatch_dependenceGraphNode::BPatch_dependenceGraphNode
  //
  // constructor

  API_EXPORT_CTOR(Int,(inst),
  BPatch_dependenceGraphNode,(BPatch_instruction* inst));

  API_EXPORT_CTOR(Int,(inst,incList,outList),
  BPatch_dependenceGraphNode,(BPatch_instruction* inst,BPatch_Vector<BPatch_dependenceGraphEdge*>* incList,BPatch_Vector<BPatch_dependenceGraphEdge*>* outList));

  API_EXPORT_V(Int,(out),
  void, getOutgoingEdges,(BPatch_Vector<BPatch_dependenceGraphEdge*>& out))
  
  API_EXPORT_V(Int,(inc),
  void, getIncomingEdges,(BPatch_Vector<BPatch_dependenceGraphEdge*>& inc))

  API_EXPORT(Int,(other_node),
  bool,isImmediateSuccessor,(BPatch_instruction * other_node));

  API_EXPORT(Int,(other_node),
  bool,isImmediatePredecessor,(BPatch_instruction * other_node));

  API_EXPORT(Int,(),
  BPatch_instruction*,getBPInstruction,());

  bool addToIncoming(BPatch_dependenceGraphNode* other_instruction);

  bool addToOutgoing(BPatch_dependenceGraphNode* other_instruction);
};
#endif // _BPatch_dependenceGraphNode_h_

