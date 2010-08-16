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
#include "dyninstAPI/src/codegen.h" // codeGen
#include "instructionAPI/h/Instruction.h" // Instruction::Ptr

#include <list> // stl::list
#include "dyninstAPI/src/function.h" // bblInstance

#include "../AddressMapper.h"

class baseTramp;
class bblInstance;
class baseTrampInstance;

namespace Dyninst {
namespace Relocation {

class Transformer;
class Atom;
class RelocInsn;
class Inst;
class CFAtom;
class Trace;

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

// Atom code generation class
class Atom {
  friend class Transformer;
 public:
  typedef dyn_detail::boost::shared_ptr<Atom> Ptr;
  typedef dyn_detail::boost::shared_ptr<Trace> TracePtr;

  Atom() {};

  // A default value to make sure things don't go wonky.
  virtual Address addr() const { return 0; }
  virtual unsigned size() const { return 0; }
  virtual InstructionAPI::Instruction::Ptr insn() const {
    return InstructionAPI::Instruction::Ptr();
  }

  // Make binary from the thing
  // Current address (if we need it)
  // is in the codeGen object.
  virtual bool generate(GenStack &) = 0;

  virtual std::string format() const = 0;

  virtual ~Atom() {};
};

 // A generic code patching mechanism
struct Patch {
  virtual bool apply(codeGen &gen, int iteration, int shift) = 0;
  virtual bool preapply(codeGen &gen) = 0;
  virtual ~Patch() {};
};

class CFAtom;
typedef dyn_detail::boost::shared_ptr<CFAtom> CFAtomPtr;

class Trace {
 public:
  friend class Transformer;

  static int TraceID;

  typedef std::list<Atom::Ptr> AtomList;
  typedef dyn_detail::boost::shared_ptr<Trace> Ptr;

  // Creation via a known bblInstance
  static Ptr create(bblInstance *inst);

  // Creation via a single baseTramp;
  static Ptr create(baseTramp *base);
  
  // Creation via a single Atom
  static Ptr create(Atom::Ptr atom);

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
  AtomList &elements() { return elements_; }

  // TODO get a more appropriate estimate...
  unsigned estimateSize() const { return size_; }

  std::string format() const;

  const bblInstance *bbl() const { return bbl_; }

  bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift);

  int iteration() const { return iteration_; }
  void resetIteration() { iteration_ = 0; }

  bool extractAddressMap(AddressMapper::accumulatorList &accumulators);

 private:

  Trace(bblInstance *bbl) :
  curAddr_(0),
    size_(bbl->getSize()),
    origAddr_(bbl->firstInsnAddr()),
    bbl_(bbl),
    iteration_(0),
    id_(TraceID++) {};
 Trace(Atom::Ptr a) :
  curAddr_(0),
    size_(0), // Should estimate here
    origAddr_(0), // No original address...
    bbl_(NULL),
    iteration_(0),
    id_(TraceID++) { 
    elements_.push_back(a);
  };


  typedef std::pair<InstructionAPI::Instruction::Ptr, Address> InsnInstance;
  typedef std::vector<InsnInstance> InsnVec;

  // Raw creation method via list of instructions + manually specified
  // post-block control flow:
  static Ptr create(const InsnVec &insns,
		    CFAtomPtr end);

  // Analyze the block ender and create a logical control flow
  // construct matching it. 
  bool createTraceEnd();

  AtomList elements_;

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
