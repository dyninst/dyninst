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


#if !defined(_R_CODE_MOVER_H_)
#define _R_CODE_MOVER_H_

#include "dyn_detail/boost/shared_ptr.hpp"
#include "common/h/Types.h"
#include <list>
#include <map>
#include "codegen.h" // codeGen structure

#include "r_t_Base.h"

#include "r_Springboard.h"

class int_function;
class int_basicBlock;
class codeGen;

namespace Dyninst {
  class AddressMapper;

namespace Relocation {

class Block;
class Transformer;
class CodeMover;


class CodeMover {
 public:
  typedef dyn_detail::boost::shared_ptr<CodeMover> Ptr;
  typedef dyn_detail::boost::shared_ptr<Block> BlockPtr;
  typedef std::list<BlockPtr> BlockList;
  typedef std::map<bblInstance *, BlockPtr> BlockMap;
  typedef std::set<int_function *> FuncSet;

  // A generic mover of code; an instruction, a basic block, or
  // a function. This is the algorithm (fixpoint) counterpart
  // of the classes described in relocation.h
  
  // Input: 
  //  A structured description of code in terms of an instruction, a
  //    block, a function, or a set of functions;
  //  A starting address for the moved code 
  //
  // Output: a buffer containing the moved code 

  static Ptr create();

  template<typename BlockIter>
    static Ptr create(BlockIter begin, BlockIter end);
 
  // Needs a different name to get the arguments
  // right.
  static Ptr createFunc(FuncSet::const_iterator begin, FuncSet::const_iterator end);

  static void causeTemplateInstantiations();

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

  // Aaand debugging functionality
  void disassemble() const;

  void extractAddressMap(AddressMapper &addrMap, Address baseAddr);

  void extractPostCallPads(AddressSpace *);

  // Get a map from original addresses to new addresses
  // for all blocks
  typedef std::map<Address, Address> EntryMap;
  const EntryMap &entryMap() { return entryMap_; }

  // Some things (LocalCFTransformer) require the
  // map of (original) addresses to BlockPtrs
  // so they can refer to blocks other than those
  // they are transforming.
  const BlockMap &blockMap() const { return blockMap_; }
  const SpringboardMap &sBoardMap();
  // Not const so that Transformers can modify it...
  PriorityMap &priorityMap();

  // Get either an estimate (pre-relocation) or actual
  // size
  unsigned size() const;

  // (void *) to start of code
  void *ptr() const { return gen_.start_ptr(); }

  std::string format() const;

 private:
    

 CodeMover() : addr_(0), size_(0) {};

  
  void setAddr(Address &addr) { addr_ = addr; }
  template <typename BlockIter>
    bool addBlocks(BlockIter begin, BlockIter end);

  bool addBlock(bblInstance *block);

  BlockList blocks_;
  // We also want to have a map from a bblInstance
  // to a Block so we can wire together jumps within
  // moved code
  BlockMap blockMap_;

  Address addr_;

  codeGen gen_;

  EntryMap entryMap_;

  PriorityMap priorityMap_;
  
  SpringboardMap sboardMap_;

  unsigned size_;
};


};

};

#endif
