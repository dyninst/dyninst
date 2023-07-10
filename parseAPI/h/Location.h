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
#ifndef LOCATION_H
#define LOCATION_H


#include "CFG.h"
#include "CodeObject.h"


#include "InstructionDecoder.h"
#include "Instruction.h"

#include <string>
#include <assert.h>
#include <utility>
#include <vector>

namespace Dyninst{
namespace ParseAPI{

struct EntrySite {
   Function *func;
   Block *block;
EntrySite(Function *f, Block *b) : func(f), block(b) {}
};
struct CallSite {
   Function *func;
   Block *block;
CallSite(Function *f, Block *b) : func(f), block(b) {}
};
struct ExitSite {
   Function *func;
   Block *block;
ExitSite(Function *f, Block *b) : func(f), block(b) {}
};

struct EdgeLoc{
   Function *func;
   Edge *edge;
   EdgeLoc(Function *f, Edge *e): func(f), edge(e) {}
};

struct BlockSite{
   Function *func;
   Block *block;
   BlockSite(Function *f, Block *b): func(f), block(b) {}
};

typedef std::vector<std::pair<InstructionAPI::Instruction, Offset> > InsnVec;

static void getInsnInstances(Block *block,
			     InsnVec &insns) {
  Offset off = block->start();
  const unsigned char *ptr = (const unsigned char *)block->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionAPI::InstructionDecoder d(ptr, block->size(), block->obj()->cs()->getArch());
  while (off < block->end()) {
    insns.push_back(std::make_pair(d.decode(), off));
    off += insns.back().first.size();
  }
}


struct InsnLoc {
   Block *const block;
   const Offset offset;
   InstructionAPI::Instruction insn;
InsnLoc(Block *const b,  Offset o, const InstructionAPI::Instruction& i) :
   block(b), offset(o), insn(i) {}
};
      
     

struct Location {

Location() : func(NULL), block(NULL), offset(0), edge(NULL), untrusted(false), type(illegal_) {}
   // Function
Location(Function *f) : func(f), block(NULL), offset(0), edge(NULL), untrusted(false), type(function_) {}
Location(EntrySite e) : func(e.func), block(e.block), offset(0), edge(NULL), untrusted(false), type(entry_) {}
Location(CallSite c) : func(c.func), block(c.block), offset(0), edge(NULL), untrusted(false), type(call_) {}
Location(ExitSite e) : func(e.func), block(e.block), offset(0), edge(NULL), untrusted(false), type(exit_) {}
   // A block in a particular function
Location(Function *f, Block *b) : func(f), block(b), offset(0), edge(NULL), untrusted(true), type(blockInstance_) {}
   // A block of a function
Location(BlockSite b): func(b.func), block(b.block), offset(0), edge(NULL), untrusted(false), type(blockInstance_) {}
   // A trusted instruction (in a particular function)
Location(Function *f, InsnLoc l) : func(f), block(l.block), offset(l.offset), insn(l.insn), edge(NULL), untrusted(false), type(instructionInstance_) {}
   // An untrusted (raw) instruction (in a particular function)
Location(Function *f, Block *b, Offset o, InstructionAPI::Instruction i) : func(f), block(b), offset(o), insn(i), edge(NULL), untrusted(true), type(instructionInstance_) {}
   // An edge (in a particular function)
Location(Function *f, Edge *e) : func(f), block(NULL), offset(0), edge(e), untrusted(true), type(edge_) {}
Location(EdgeLoc e): func(e.func), block(NULL), offset(0), edge(e.edge), untrusted(false), type(edge_){}
   // A block in general
Location(Block *b) : func(NULL), block(b), offset(0), edge(NULL), untrusted(false), type(block_) {}
   // A trusted instruction in general
Location(InsnLoc l) : func(NULL), block(l.block), offset(l.offset), insn(l.insn), edge(NULL), untrusted(false), type(instruction_) {}
   // An untrusted (raw) instruction
Location(Block *b, Offset o) : func(NULL), block(b), offset(o), edge(NULL), untrusted(true), type(instruction_) {}
   // An edge
Location(Edge *e) : func(NULL), block(NULL), offset(0), edge(e), untrusted(false), type(edge_) {}

   typedef enum {
      function_,
      block_,
      blockInstance_,
      instruction_,
      instructionInstance_,
      edge_,
      entry_,
      call_,
      exit_,
      illegal_ } type_t;

   bool legal(type_t t) { return t == type; }

   InsnLoc insnLoc() { return InsnLoc(block, offset, insn); }

   bool isValid(){
   	if (untrusted){
		InsnVec insns;
		switch (type){
			case blockInstance_:
				return func->contains(block);
			case instructionInstance_:
				if (!func->contains(block)) return false;
				getInsnInstances(block, insns);
				for (InsnVec::iterator iter = insns.begin(); iter != insns.end(); ++iter)
					if (iter->second == offset) return true;
				return false;
			case edge_:{
				Function::blocklist blk = func->blocks();
				for (Function::blocklist::iterator blockIter = blk.begin(); blockIter != blk.end(); ++blockIter){
					Intraproc epred;
					const Block::edgelist& target_edges = (*blockIter) -> targets();
					if(std::find(boost::make_filter_iterator(epred, target_edges.begin(), target_edges.end()),
						     boost::make_filter_iterator(epred, target_edges.end(), target_edges.end()),
						     edge)
					   != boost::make_filter_iterator(epred, target_edges.end(), target_edges.end()))
					{
					  return true;
					}
				}
				return false;
			}	
			case instruction_:
				getInsnInstances(block, insns);
				for (InsnVec::iterator iter = insns.begin(); iter != insns.end(); ++iter)
					if (iter->second == offset) return true;
				return false;
			case function_: 
			case block_:  
			case entry_: 
			case call_:  
			case exit_: 
			case illegal_: assert(0);
	
		}
	}
	return true;
   }

Function *const func;
Block *const block;
const Offset offset;
InstructionAPI::Instruction insn;
Edge *const edge;
const bool untrusted;
const type_t type;


};

} //ParseAPI
} //Dyninst

#endif
