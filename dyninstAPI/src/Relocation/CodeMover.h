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


#if !defined(_R_CODE_MOVER_H_)
#define _R_CODE_MOVER_H_

#include "CFG.h"
#include <set>
#include <string>
#include <utility>
#include <list>
#include <map>
#include "dyninstAPI/src/codegen.h" // codeGen structure

#include "Transformers/Transformer.h"

#include "Springboard.h"
#include "CodeBuffer.h"
#include "Springboard.h"

class codeGen;
class block_instance;
class func_instance;

namespace Dyninst {
  class AddressMapper;

namespace Relocation {

class RelocBlock;
class Transformer;
class CodeMover;
class CodeTracker;
class RelocGraph;

typedef std::map<std::pair<block_instance *, func_instance *>, Priority> PriorityMap;

class CodeMover {
 public:
  typedef boost::shared_ptr<CodeMover> Ptr;
  typedef std::set<func_instance *> FuncSet;
  typedef std::set<block_instance *> BlockSet;

  // A generic mover of code; an instruction, a basic block, or
  // a function. This is the algorithm (fixpoint) counterpart
  // of the classes described in relocation.h
  
  // Input: 
  //  A structured description of code in terms of an instruction, a
  //    block, a function, or a set of functions;
  //  A starting address for the moved code 
  //
  // Output: a buffer containing the moved code 

  // We take a CodeTracker as a reference parameter so that we don't 
  // have to copy it on output; CodeMovers are designed to be discarded
  // while CodeTrackers survive.
  static Ptr create(CodeTracker *);
  ~CodeMover();

  bool addFunctions(FuncSet::const_iterator begin, FuncSet::const_iterator end);

  // Apply the given Transformer to all blocks in the Mover
  bool transform(Transformer &t);
  
  // Does all the once-only work to generate code.
  bool initialize(const codeGen &genTemplate);

  // Allocates an internal buffer and relocates the code provided
  // to the constructor. Returns true for success or false for
  // catastrophic failure.
  // The codeGen parameter allows specification of various
  // codeGen-carried information
  bool relocate(Address addr);

  bool finalize();

  // Aaand debugging functionality
  void disassemble() const;

  void extractDefensivePads(AddressSpace *);

  // Get a map from original addresses to new addresses
  // for all blocks
  typedef std::map<Address, Address> EntryMap;
  const EntryMap &entryMap() { return entryMap_; }

  // Not const so we can add others to it. 
  SpringboardMap &sBoardMap(AddressSpace *as);
  // Not const so that Transformers can modify it...
  PriorityMap &priorityMap();

  // Get either an estimate (pre-relocation) or actual
  // size
  unsigned size() const;

  // (void *) to start of code
  void *ptr() const;

  std::string format() const;

  codeGen &gen();

 private:
    
  CodeMover(CodeTracker *t);
  
  void setAddr(Address &addr) { addr_ = addr; }
  template <typename RelocBlockIter>
     bool addRelocBlocks(RelocBlockIter begin, RelocBlockIter end, func_instance *f);

  bool addRelocBlock(block_instance *block, func_instance *f);

  void finalizeRelocBlocks();

  RelocGraph *cfg_;

  Address addr_;

  EntryMap entryMap_;

  PriorityMap priorityMap_;
  
  SpringboardMap sboardMap_;

  CodeTracker *tracker_;

  CodeBuffer buffer_;

  bool finalized_;
  
};


}

}

#endif
