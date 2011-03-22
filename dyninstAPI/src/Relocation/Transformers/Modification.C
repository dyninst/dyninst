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

#include "Transformer.h"
#include "Modification.h"
#include "../patchapi_debug.h"
#include "../Atoms/Target.h"
#include "../Atoms/Atom.h"
#include "../Atoms/CFAtom.h"
#include "../Atoms/ASTAtom.h"
#include "../Atoms/InstAtom.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/function.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;

Modification::Modification(const ext_CallReplaceMap &callRepl,
			   const ext_FuncReplaceMap &funcRepl,
			   const ext_CallRemovalSet &callRem) {
  for (ext_CallReplaceMap::const_iterator iter = callRepl.begin();
       iter != callRepl.end(); ++iter) {
    block_instance *bbl = iter->first->block();
    callRep_[bbl] = std::make_pair<func_instance *, instPoint *>(iter->second, iter->first);
  }

  for (ext_FuncReplaceMap::const_iterator iter = funcRepl.begin();
       iter != funcRepl.end(); ++iter) {
     block_instance *bbl = iter->first->entryBlock();
    funcRep_[bbl] = iter->second;
  }

  for (ext_CallRemovalSet::const_iterator iter = callRem.begin();
       iter != callRem.end(); ++iter) {
    block_instance *bbl = (*iter)->block();
    callRem_.insert(bbl);
  }
}

bool Modification::processTrace(TraceList::iterator &iter) {
  // We define three types of program modification:
  // 1) Function call replacement; change the target of the corresponding
  //    call element
  // 2) Function call removal; modify the CFelement to have only a
  //    fallthrough edge
  // 3) Function replacement; TODO

  Trace::Ptr block = *iter;
  
  CallReplaceMap::iterator c_rep = callRep_.find(block->block());
  if (c_rep != callRep_.end()) {
    replaceCall(block, c_rep->second.first, c_rep->second.second);
  }

  FuncReplaceMap::iterator f_rep = funcRep_.find(block->block());
  if (f_rep != funcRep_.end()) {
    replaceFunction(block, f_rep->second);
  }

  CallRemovalSet::iterator c_rem = callRem_.find(block->block());
  if (c_rem != callRem_.end()) {
    removeCall(block);
  }

  return true;
}

void Modification::replaceCall(TracePtr block, func_instance *target, instPoint *cur) {
  Trace::AtomList &elements = block->elements();

  cerr << "Warning: skipping replacement of call" << endl;

  CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(elements.back());

  if (!cf) return;

  if (!cf->insn()) return;
  
  if (cf->insn()->getCategory() != InstructionAPI::c_CallInsn) return;

  relocation_cerr << "Replacing call @" << hex << cf->addr() << dec
		  << " with a call to " << target->prettyName() << " @"
		  << hex << target->getAddress() << dec << endl;

  if (block->block()->func()->obj() == target->obj()) {
    // In the same mapped object; we can do an efficient replace that just
    // modifies the call distance. 
    CFAtom::DestinationMap::iterator d_iter = cf->destMap_.find(CFAtom::Taken);
    if (d_iter == cf->destMap_.end()) return;

    // Don't leak target objects
    delete d_iter->second;
    
    block_instance *tbbl = target->entryBlock();
    
    Target<block_instance *> *new_target = new Target<block_instance *>(tbbl);
    assert(new_target);
    
    d_iter->second = new_target;
  }
  else {
    // Yuck!
    vector<AstNodePtr> args;
    Atom::Ptr callAST = ASTAtom::create(AstNode::funcCallNode(target, args), cur);
    elements.back().swap(callAST);
  }
}


void Modification::replaceFunction(TracePtr block, func_instance *to) {
    // We replace the original function with a jump to the new function.
    // Did I say "jump"? I meant "CFAtom". 

    // No reason to keep the rest of the stuff, and we often assume CFAtoms are
    // the last thing in the list.
   
   cerr << "Warning: skipping function replacement" << endl;

#if 0
    block->elements().clear();

    CFAtom::Ptr cf = CFAtom::create(block->block());
    cf->updateAddr(block->block()->start());

    block_instance *dest = to->entryBlock();
    assert(dest);

    cf->addDestination(CFAtom::Taken, new Target<block_instance *>(dest));
    block->elements().push_back(cf);
    return;



    // We handle this by "instrumenting" with a AST callReplacementNode.
  // This is primarily due to needing registers to calculate the
  // destination, but means that we need to forge an Inst node
  // and prepend it to the block.

  

  Inst::Ptr inst = Inst::create();
  block->elements().push_front(inst);

  // And now to create a AtomTramp. Let's see if we can find one
  // in the function...
  block_instance *from = block->block()->func();
  const vector<instPoint *> &entries = from->funcEntries();
  assert(!entries.empty());
  for (unsigned i = 0; i < entries.size(); ++i) {
    entries[i]->addInst(AstNode::funcReplacementNode(to, false),
			callPreInsn, 
			orderFirstAtPoint,
			true,
			false);
  }

#endif
}


void Modification::removeCall(TracePtr block) {
  const Trace::AtomList &elements = block->elements();

  cerr << "Warning: skipping removal of call" << endl;
  return;


  CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(elements.back());

  if (!cf) return;

  if (!cf->insn()) return;
  
  if (cf->insn()->getCategory() != InstructionAPI::c_CallInsn) return;

  CFAtom::DestinationMap::iterator d_iter = cf->destMap_.find(CFAtom::Fallthrough);
  TargetInt *ftTarg = d_iter->second;

  for (CFAtom::DestinationMap::iterator e_iter = cf->destMap_.begin();
       e_iter != cf->destMap_.end(); ++e_iter) {
    if (e_iter->second != ftTarg)
      delete e_iter->second;
  }
  cf->destMap_.clear();
  cf->destMap_[CFAtom::Fallthrough] = ftTarg;
}



