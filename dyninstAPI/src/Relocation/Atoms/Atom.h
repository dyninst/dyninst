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
class TrackerElement;
class CodeTracker;
class CodeBuffer;

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
  virtual bool generate(const codeGen &templ,
                        const Trace *trace,
                        CodeBuffer &buffer) = 0;

  virtual std::string format() const = 0;

  virtual ~Atom() {};
};

 // A generic code patching mechanism
struct Patch {
   virtual bool apply(codeGen &gen, CodeBuffer *buf) = 0;
   virtual unsigned estimate(codeGen &templ) = 0;
   virtual ~Patch() {};
};

class CFAtom;
typedef dyn_detail::boost::shared_ptr<CFAtom> CFAtomPtr;

class Trace {
 public:
   // Needs to track the definition in CodeBuffer.h
   typedef int Label;

  friend class Transformer;

  static int TraceID;
  
  typedef std::list<Atom::Ptr> AtomList;
  typedef dyn_detail::boost::shared_ptr<Trace> Ptr;

  // Creation via a known bblInstance
  static Ptr create(bblInstance *inst);

  // Creation via a single baseTramp;
  static Ptr create(baseTramp *base);
  
  // Creation via a single Atom
  static Ptr create(Atom::Ptr atom, Address a, int_function *f);

  bool generate(const codeGen &templ,
                CodeBuffer &buffer);

  Address origAddr() const { return origAddr_; }
  //Address curAddr() const { return curAddr_; }
  void setAddr(Address addr) { curAddr_ = addr; }

  int id() const { return id_; }

  // Non-const for use by transformer classes
  AtomList &elements() { return elements_; }

  std::string format() const;

  bblInstance *bbl() const { return bbl_; }
  // Unlike basic blocks, _all_ traces must be
  // created in the context of a function so we can correctly
  // report which function we're from.
  int_function *func() const { return func_; }

  bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift);

  bool extractTrackers(CodeTracker &);

  Label getLabel() { assert(label_ != -1);  return label_; };

 private:

  Trace(bblInstance *bbl)
     : curAddr_(0),
     origAddr_(bbl->firstInsnAddr()),
     bbl_(bbl),
     id_(TraceID++),
     label_(-1),
     func_(bbl->func()) {};
  Trace(Atom::Ptr a, Address origAddr, int_function *f)
     : curAddr_(0),
     origAddr_(origAddr),
     bbl_(NULL),
     id_(TraceID++),
     label_(-1),
     func_(f) { 
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
  Address origAddr_;

  bblInstance *bbl_;

  typedef std::list<Patch *> Patches;

  Patches patches_;

  int id_;

  Label label_;

  int_function *func_;
};

};
};
#endif
