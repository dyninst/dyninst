
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
#include "boost/tuple/tuple.hpp"
#include "dyninstAPI/src/mapped_object.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;
using namespace ParseAPI;

bool CFAtomCreator::processTrace(TraceList::iterator &iter) {
  int_block *bbl = (*iter)->bbl();

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

  //relocation_cerr << "Creating block ender for block @ "
//		  << std::hex << (*iter)->origAddr() << std::dec << endl;

  SuccVec successors;

  getInterproceduralSuccessors(bbl, successors);
  
  // FIXME TODO
  getRawSuccessors(bbl, successors);

  // SD-Dyninst: if we haven't parsed past a call (or indirect branch?)
  // we need to drop in a patch area for a future control flow fixup
  unsigned gap = modifiedCallFallthrough(bbl);
  if (gap) {
      cerr << "We have a believed gap between source and fallthrough block of " << gap << " bytes, adding pad" << endl;
      ender->setPostCallPadding(gap);
  }

  for (unsigned i = 0; i < successors.size(); ++i) {
    TargetInt *targ = successors[i].targ;
    EdgeTypeEnum type = successors[i].type;
    Address addr = successors[i].addr;
    switch(type) {
    case INDIRECT: {
      //relocation_cerr << "Adding indirect destination: "
//		      << std::hex << addr << std::dec << endl;
      ender->addDestination(addr, targ);
      break;
    }
    case CALL:
    case DIRECT:
    case COND_TAKEN:
      //relocation_cerr << "Adding taken destination: "
//		      << std::hex << addr << std::dec << endl;
      ender->addDestination(CFAtom::Taken, targ);
      break;
    case COND_NOT_TAKEN:
    case FALLTHROUGH:
    case CALL_FT:
      //relocation_cerr << "Adding fallthrough destination: "
//		      << std::hex << addr << std::dec << endl;
      ender->addDestination(CFAtom::Fallthrough, targ);
      break;
    case NOEDGE:
    case CATCH:
    case RET: // I think...?
    default:
      //relocation_cerr << "Ignoring destination type " << type << endl;
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


void CFAtomCreator::getInterproceduralSuccessors(const int_block *block,
                                                 SuccVec &succ) {
  int_function *func = block->func();

  // This function is only annoying because our parsing layer is so incredibly
  // unintuitive and backwards. Hence... yeah. Annoyance. 

  // The int_ layer (that int_blocks work on) has two restrictions:
  // Single-function; we don't see interprocedural edges
  // No efficient way to get a list of all edges with edge types;
  //   instead we see target blocks and can query the type of each
  //   edge to each block (nice O(n^2)...)

  // Instead we do it ourselves. I've got the code here for one simple
  // reason: handling calls between modules in the static case. Calls
  // go to PLT entries and we don't parse those, so normally we'd see
  // an image_basicBlock with no image_function, int_function,
  // int_block, or int_block... useless. Instead I'm using the
  // Target concept to create a destination out of whole cloth.

  // This requires an... interesting... dodge through to the internals

  const ParseAPI::Block::edgelist &targets = block->llb()->targets();
  ParseAPI::Block::edgelist::iterator iter = targets.begin();
  for (; iter != targets.end(); ++iter) {
     Succ out(NULL, (*iter)->type(), 0);

     if ((*iter)->sinkEdge()) {
        // Special case time
        // A sink edge represents an unknown (or illegal/unparseable)
        // successor. Unfortunately, Kevin sees these all the time.
        // ParseAPI doesn't represent the destination, but should still
        // have an edge type. So if we see a direct sink edge, we 
        // need to do some extra work.
        switch ((*iter)->type()) {
           case ParseAPI::CALL:
           case ParseAPI::COND_TAKEN:
           case ParseAPI::DIRECT: {
              // Okay, this must have had an unparseable target. We want
              // it anyway, so figure out manually what the destination
              // is by binding the PC to the address. Ugh. Ugh ugh ugh.
              std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> > insns;
              block->getInsnInstances(insns);
              InstructionAPI::Instruction::Ptr insn = insns.back().first;
              Expression::Ptr exp = insn->getControlFlowTarget();
              if (!exp) {
                  //relocation_cerr << "WARNING: Null expr for CFT of sink edge for insn at " 
//                      << hex << bbl->lastInsnAddr() << endl;
                  break;
              }
              static Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(Arch_x86)));
              static Expression::Ptr thePC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
              exp->bind(thePC.get(), Result(u32, insns.back().second));
              exp->bind(thePC64.get(), Result(u64, insns.back().second));
              Result res = exp->eval();
              if (res.defined) {
                 out.targ = new Target<Address>(res.convert<Address>());
                 out.addr = res.convert<Address>();
              }
              break;
           }
              
           case ParseAPI::COND_NOT_TAKEN:
           case ParseAPI::FALLTHROUGH:
           case ParseAPI::CALL_FT: {
              out.targ = new Target<Address>(block->end());
              out.addr = block->end();
              break;
           }

           default:
              break;
        }
     }
     else {
        // We have an image_basicBlock... now we need to map up
        // to both an int_block and an int_block.
        image_basicBlock *ib = static_cast<image_basicBlock *>((*iter)->trg());
        
        if (out.type == RET || out.type == NOEDGE || out.type == INDIRECT) {
           continue;
        }
        else if (out.type != CALL) {
           int_block *targ = func->findBlock(ib);
           assert(targ);
           out.targ = new Target<int_block *>(targ);
           out.addr = targ->start();
        }
        else {
           // Trace must be an entry point since we reach it with
           // a call...
           image_func *iCallee = ib->getEntryFunc();
           int_function *callee = NULL;
           if (iCallee) callee = block->proc()->findFuncByInternalFunc(iCallee);
           if (callee) {
              // Make sure it's parsed
              callee->blocks();
              // Same as above
              int_block *targ = callee->findBlock(ib);
              out.targ = new Target<int_block *>(targ);
              out.addr = targ->start();
           }
           else {
              // Okay. This is obviously (really) a call to a PLT
              // entry. Now, calls to PLT entries are tricky, since
              // we currently don't parse them. OTOH, that means that
              // I don't have anything to find a int_block with.
              // Instead we use a special-form Target
              
              // First, assert that our offset is 0. If it isn't we're in
              // real trouble since we can't upcast.
              assert(block->start() == block->llb()->firstInsnOffset());
              out.targ = new Target<Address>(ib->firstInsnOffset());
              out.addr = ib->firstInsnOffset();
           }
        }
     }
     if (out.targ)
        succ.push_back(out);
  }
}

unsigned CFAtomCreator::modifiedCallFallthrough(const int_block *inst) {
   // Find if the program does anything funky with a call fallthrough
   // 1) A call edge with no fallthrough
   // 2) A gap between the call block and the fallthrough block.
   
   ParseAPI::Edge *callEdge = NULL;
   ParseAPI::Edge *ftEdge = NULL;
   
   const ParseAPI::Block::edgelist &targets = inst->llb()->targets();
   ParseAPI::Block::edgelist::iterator iter = targets.begin();
   for (; iter != targets.end(); ++iter) {
      if ((*iter)->type() == ParseAPI::CALL) {
         callEdge = *iter;
      }
      if ((*iter)->type() == ParseAPI::CALL_FT) {
         ftEdge = *iter;
      }
   }

   if (ftEdge) {
      Address callEnd = ftEdge->src()->end();
      Address ftStart = ftEdge->trg()->start();
        return ftStart - callEnd;
   }
    return 0;
}

void CFAtomCreator::getRawSuccessors(const int_block *block, 
    SuccVec &succ) 
{
    // There is a specific bug in ParseAPI. If we see a function that
    // looks like garbage code, we do not include it in the CFG and remove
    // all edges into that function - including edges from (e.g.) 
    // direct jumps and calls. However, these functions are typically overwritten
    // at runtime before said direct jump or call executes. Since the edges
    // are not in the CFG the code in getInterproceduralSuccessors misses
    // them, and we end up skipping the control transfer instruction entirely.
    // That is... suboptimal. As a temporary workaround, I'm regenerating
    // the transfer from the raw instruction and setting it as an Address-typed
    // target.
    if (!succ.empty()) {
        using namespace ParseAPI;
        Block::edgelist edges= block->llb()->targets();
        int pairEdgeCnt = 0;
        for (Block::edgelist::iterator eit= edges.begin();
             eit != edges.end(); eit++) 
        {
            switch((*eit)->type()) {
                case CALL:
                case CALL_FT:
                case COND_TAKEN:
                case COND_NOT_TAKEN:
                    pairEdgeCnt++;
                    break;
                default:
                    break;
            }
        }
        if ( 0 == (pairEdgeCnt % 2)) {
            return;
        }
    }

    using namespace InstructionAPI;

    int_block::InsnInstances insns;
    block->getInsnInstances(insns);

    // If we have a resolveable control flow target, make sure there's
    // a matching target in the successor vector

    Expression::Ptr cft = insns.back().first->getControlFlowTarget();
    if (!cft) return;

    cerr << "Checking for missed successor @ " << hex << insns.back().second << " : format " << insns.back().first->format(insns.back().second) << dec << endl;

    Expression::Ptr thePC = Expression::Ptr(new RegisterAST(MachRegister::getPC(insns.back().first->getArch())));
    cft->bind(thePC.get(), Result(u32, insns.back().second));
    Result res = cft->eval();
    if (!res.defined) return;

    Address target = res.convert<Address>();
    cerr << "\t Determined target " << hex << target << dec << endl;

    for (SuccVec::iterator iter = succ.begin(); iter != succ.end(); ++iter) {
        if (iter->addr == target) return;
    }
    cerr << "\t Failed to find in CFG target list, creating" << endl;
    // Oops...
    Succ out;
    out.targ = new Target<Address>(target);
    out.addr = target;
    switch(insns.back().first->getCategory())
    {
    case c_CallInsn:
        out.type = CALL;
        break;
    case c_BranchInsn:
        if (!insns.back().first->allowsFallThrough())
        {
            out.type = DIRECT;
        }
        else 
        {
            out.type = COND_TAKEN;
        }
        break;
    default:
        assert(0);
        break;
    }
    succ.push_back(out);
}