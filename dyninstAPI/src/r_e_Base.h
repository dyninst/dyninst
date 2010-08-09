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

#if !defined(_R_E_BASE_H_)
#define _R_E_BASE_H_

#include "dyn_detail/boost/shared_ptr.hpp" // shared_ptr
#include "common/h/Types.h" // Address
#include "codegen.h" // codeGen
#include "Instruction.h" // Instruction::Ptr

#include <list> // stl::list
#include "dyninstAPI/src/function.h" // bblInstance

#include "r_AddressMapper.h"

class baseTramp;
class bblInstance;
class baseTrampInstance;

namespace Dyninst {
namespace Relocation {

class Transformer;
class Element;
class RelocInsn;
class Inst;
class CFElement;
class Block;

class Patch;

class GenStack {
 public:
  struct GenObj {
    bool apply(codeGen &current, unsigned &size, int iteration, int shift);
  GenObj() : patch(NULL), index(0) {};
    ~GenObj();
    codeGen gen;
    Patch *patch;
    codeBufIndex_t index;
    
    // For building the address map.
    std::list<std::pair<Address, Offset> > addrMap;
    Offset offset;
  };

  typedef std::list<GenObj> GenList;

  typedef GenList::iterator iterator;

  GenStack() {};
  void initialize(codeGen &gen);
  GenObj &cur() { return gens_.back(); }
  codeGen &operator() () { return gens_.back().gen; }
  void inc();
  unsigned size(); 

  iterator begin() { return gens_.begin(); }
  iterator end() { return gens_.end(); }

  void addPatch(Patch *);

 private:
  GenList gens_;
};

// Base code generation class
class Element {
  friend class Transformer;
 public:
  typedef dyn_detail::boost::shared_ptr<Element> Ptr;
  typedef dyn_detail::boost::shared_ptr<Block> BlockPtr;

  Element() {};

  // A default value to make sure things don't go wonky.
  virtual Address addr() const { return 0; }
  virtual unsigned size() const { return 0; }
  virtual InstructionAPI::Instruction::Ptr insn() const {
    return InstructionAPI::Instruction::Ptr();
  }

  // Make binary from the thing
  // Current address (if we need it)
  // is in the codeGen object.
  virtual bool generate(Block &block,
			GenStack &) = 0;

  virtual std::string format() const = 0;

  virtual ~Element() {};
};

 // A generic code patching mechanism
struct Patch {
  virtual bool apply(codeGen &gen, int iteration, int shift) = 0;
  virtual bool preapply(codeGen &gen) = 0;
  virtual ~Patch() {};
};

class CFElement;
typedef dyn_detail::boost::shared_ptr<CFElement> CFElementPtr;

class Block {
 public:
  friend class Transformer;

  static int BlockID;

  typedef std::list<Element::Ptr> ElementList;
  typedef dyn_detail::boost::shared_ptr<Block> Ptr;

  // Creation via a known bblInstance
  static Ptr create(bblInstance *inst);

  // Creation via a single BaseTramp;
  static Ptr create(baseTramp *base);
  
  // Generate code for this block
  // Return: whether generation was successful
  // gen: a codeGen object (AKA wrapper for all sorts
  //      of state we need)
  // changed: whether something in this block changed
  //          necessitating regeneration of the container
  bool generate(codeGen &templ,
		unsigned &sizeEstimate);

  unsigned size() const { return size_; }
  Address origAddr() const { return origAddr_; }
  Address curAddr() const { return curAddr_; }
  void setAddr(Address addr) { curAddr_ = addr; }

  int id() const { return id_; }

  // Non-const for use by transformer classes
  ElementList &elements() { return elements_; }

  // TODO get a more appropriate estimate...
  unsigned estimateSize() const { return size_; }

  std::string format() const;

  const bblInstance *bbl() const { return bbl_; }

  bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift);

  int iteration() const { return iteration_; }
  void resetIteration() { iteration_ = 0; }

  bool extractAddressMap(AddressMapper::accumulatorList &accumulators);

 private:

  Block(bblInstance *bbl) :
  curAddr_(0),
    size_(bbl->getSize()),
    origAddr_(bbl->firstInsnAddr()),
    bbl_(bbl),
    iteration_(0),
    id_(BlockID++) {};
 Block(baseTramp *) :
  curAddr_(0),
    size_(0), // Should estimate here
    origAddr_(0), // No original address...
    bbl_(NULL),
    iteration_(0),
    id_(BlockID++) {};


  typedef std::pair<InstructionAPI::Instruction::Ptr, Address> InsnInstance;
  typedef std::vector<InsnInstance> InsnVec;

  // Raw creation method via list of instructions + manually specified
  // post-block control flow:
  static Ptr create(const InsnVec &insns,
		    CFElementPtr end);

  // Analyze the block ender and create a logical control flow
  // construct matching it. 
  bool createBlockEnd();

  ElementList elements_;

  Address curAddr_;
  unsigned size_;
  Address origAddr_;

  bblInstance *bbl_;

  typedef std::list<Patch *> Patches;

  Patches patches_;

  GenStack gens_;

  int iteration_;

  int id_;
};

};
};
#endif
