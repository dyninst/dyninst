
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
#include "ControlFlow.h"
#include "dyninstAPI/src/debug.h"
#include "../Atoms/CFAtom.h"
#include "../Atoms/Target.h"
#include "../Atoms/CopyInsn.h"
#include "dyninstAPI/src/addressSpace.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;
using namespace ParseAPI;

bool CFAtomCreator::processTrace(TraceList::iterator &iter) {
  bblInstance *bbl = (*iter)->bbl();

  // Can be true if we see an instrumentation block...
  if (!bbl) return true;

  CFAtom::Ptr ender = CFAtom::create(bbl);
  // Okay, now we need to construct a CFAtom matching this block's successors.
  // The CFAtom contains a certain amount of modelling *when* an edge is taken,
  // so we need to reconstruct that. We can do that via edge types. Here
  // [Taken] and [Fallthrough] are constants defined by CFAtom
  
  // ET_CALL(callee) : [Taken] -> Target(callee)
  // ET_COND_TAKEN(targ) : [Taken]  -> Target(targ)
  // ET_COND_NOT_TAKEN(ft) : [Fallthrough] -> Target(ft)
  // ET_INDIR(b) : b.addr -> Target(b)
  // ET_DIRECT(targ) : [Fallthrough] -> Target(targ)
  // ET_CATCH(...) : (ignored)
  // ET_FUNLINK(succ) : [Fallthrough] -> Target(succ)
  // ET_NOEDGE : wtf?

  relocation_cerr << "Creating block ender for block @ "
		  << std::hex << (*iter)->origAddr() << std::dec << endl;

  SuccVec successors;

  getInterproceduralSuccessors(bbl, successors);
  
  // SD-Dyninst: if we haven't parsed past a call (or indirect branch?)
  // we need to drop in a patch area for a future control flow fixup
  if (unparsedFallthrough(bbl)) {
    ender->setNeedsFTPadding();
  }

  for (unsigned i = 0; i < successors.size(); ++i) {
    TargetInt *targ = successors[i].targ;
    EdgeTypeEnum type = successors[i].type;
    Address addr = successors[i].addr;
    switch(type) {
    case INDIRECT: {
      relocation_cerr << "Adding indirect destination: "
		      << std::hex << addr << std::dec << endl;
      ender->addDestination(addr, targ);
      break;
    }
    case CALL:
    case DIRECT:
    case COND_TAKEN:
      relocation_cerr << "Adding taken destination: "
		      << std::hex << addr << std::dec << endl;
      ender->addDestination(CFAtom::Taken, targ);
      break;
    case COND_NOT_TAKEN:
    case FALLTHROUGH:
    case CALL_FT:
      relocation_cerr << "Adding fallthrough destination: "
		      << std::hex << addr << std::dec << endl;
      ender->addDestination(CFAtom::Fallthrough, targ);
      break;
    case NOEDGE:
    case CATCH:
    case RET: // I think...?
    default:
      relocation_cerr << "Ignoring destination type " << type << endl;
      // Ignore...
      break;
    }
  }

  // Now check the last Atom. If it's an explicit CF instruction
  // pull it and replace it with the CFAtom; otherwise append.

  CopyInsn::Ptr reloc = dyn_detail::boost::dynamic_pointer_cast<CopyInsn>((*iter)->elements().back());
  assert(reloc);

  Instruction::Ptr insn = reloc->insn();
  Address addr = reloc->addr();
  assert(addr);
  ender->updateAddr(addr);

  if ((insn->getCategory() != c_CompareInsn) &&
      (insn->getCategory() != c_NoCategory)) {
    // Remove it so that it will be replaced by end
    (*iter)->elements().pop_back();
    
    // We want to shove this into the CFAtom so it'll know how to regenerate the
    // branch/call/whatever
    ender->updateInsn(insn);
  } 

  (*iter)->elements().push_back(ender);

  return true;
}


void CFAtomCreator::getInterproceduralSuccessors(const bblInstance *bbl,
                                                 SuccVec &succ) {
  int_basicBlock *block = bbl->block();
  int_function *func = block->func();

  // This function is only annoying because our parsing layer is so incredibly
  // unintuitive and backwards. Hence... yeah. Annoyance. 

  // The int_ layer (that bblInstances work on) has two restrictions:
  // Single-function; we don't see interprocedural edges
  // No efficient way to get a list of all edges with edge types;
  //   instead we see target blocks and can query the type of each
  //   edge to each block (nice O(n^2)...)

  // Instead we do it ourselves. I've got the code here for one simple
  // reason: handling calls between modules in the static case. Calls
  // go to PLT entries and we don't parse those, so normally we'd see
  // an image_basicBlock with no image_function, int_function,
  // int_basicBlock, or bblInstance... useless. Instead I'm using the
  // Target concept to create a destination out of whole cloth.

  // This requires an... interesting... dodge through to the internals

  const ParseAPI::Block::edgelist &targets = block->llb()->targets();
  ParseAPI::Block::edgelist::iterator iter = targets.begin();
  for (; iter != targets.end(); ++iter) {
    if ((*iter)->sinkEdge()) continue;

    Succ out(NULL, (*iter)->type(), 0);
    
    // We have an image_basicBlock... now we need to map up
    // to both an int_basicBlock and an bblInstance.
    image_basicBlock *ib = static_cast<image_basicBlock *>((*iter)->trg());
    
    if (out.type == RET) {
        continue;
    }
    else if (out.type != CALL) {
      int_basicBlock *targ = func->findBlockByImage(ib);
      assert(targ);
      out.targ = new Target<bblInstance *>(targ->origInstance());
      out.addr = targ->origInstance()->firstInsnAddr();
    }
    else {
      // Trace must be an entry point since we reach it with
      // a call...
      image_func *iCallee = ib->getEntryFunc();
      int_function *callee = NULL;
      if (iCallee) callee = bbl->proc()->findFuncByInternalFunc(iCallee);
      if (callee) {
	// Make sure it's parsed
	callee->blocks();
	// Same as above
	int_basicBlock *targ = callee->findBlockByImage(ib);
	out.targ = new Target<bblInstance *>(targ->origInstance());
        out.addr = targ->origInstance()->firstInsnAddr();
      }
      else {
	// Okay. This is obviously (really) a call to a PLT
	// entry. Now, calls to PLT entries are tricky, since
	// we currently don't parse them. OTOH, that means that
	// I don't have anything to find a bblInstance with.
	// Instead we use a special-form Target

	// First, assert that our offset is 0. If it isn't we're in
	// real trouble since we can't upcast.
	assert(bbl->firstInsnAddr() == block->llb()->firstInsnOffset());
	out.targ = new Target<Address>(ib->firstInsnOffset());
        out.addr = ib->firstInsnOffset();
      }
    }
    assert(out.targ);
    succ.push_back(out);
  }
}

bool CFAtomCreator::unparsedFallthrough(const bblInstance *inst) {
  // I'm not sure if Kevin marks these in the parseAPI. I'm guessing not,
  // so if we see a call edge without a call_ft edge return yes.

  bool seen_call = false;
  bool seen_ft = false;

  const ParseAPI::Block::edgelist &targets = inst->block()->llb()->targets();
  ParseAPI::Block::edgelist::iterator iter = targets.begin();
  for (; iter != targets.end(); ++iter) {
    if ((*iter)->type() == ParseAPI::CALL) {
      seen_call = true;
    }
    if ((*iter)->type() == ParseAPI::CALL_FT) {
      seen_ft = true;
    }
  }
  
  if (seen_call && !seen_ft) {
    return true;
  }
  else {
    return false;
  }
}
