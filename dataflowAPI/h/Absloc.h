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

// Base representations for locations in program memory and registers
// We use a two-layered model consisting of AbsLocs (abstract locations)
// and AbsRegions (abstract regions). 
//
// An Absloc defines a unique location in memory or a register. We 
// consider the stack to be a separate, indexed-from-zero memory location.
//
// An AbsRegion is a set of Abslocs. In essence it is a set, though we
// may be able to use a more efficient internal representation than a
// set. As a TODO, we wish to provide shortcuts for comparing one type
// of AbsRegion to another type more efficiently than the fallback
// point-wise comparison.

#if !defined(ABSLOC_H)
#define ABSLOC_H

#if !defined(_MSC_VER) && !defined(os_freebsd)
#include <values.h>
#endif

#include <assert.h>
#include <ostream>
#include <set>
#include <stddef.h>
#include <string>
#include <vector>
#include "Instruction.h"
#include "DynAST.h"

namespace Dyninst {

  namespace ParseAPI {
    class Function;
    class Block;
  }

class Absloc {
 public:
  typedef enum {
    Register,
    Stack,
    Heap,
    PredicatedRegister,
    Unknown } Type;

   DATAFLOW_EXPORT static Absloc makePC(Dyninst::Architecture arch);
   DATAFLOW_EXPORT static Absloc makeSP(Dyninst::Architecture arch);
   DATAFLOW_EXPORT static Absloc makeFP(Dyninst::Architecture arch);
  
  // Some static functions for "well-known" Abslocs
  DATAFLOW_EXPORT bool isPC() const;
  DATAFLOW_EXPORT bool isSPR() const;
  
  DATAFLOW_EXPORT bool isSP() const;
  DATAFLOW_EXPORT bool isFP() const;

 DATAFLOW_EXPORT Absloc() :
  type_(Unknown),
    reg_(),
    off_(-1),
    region_(-1),
     func_(NULL),
    addr_(-1),
    preg_(),
    trueCond_(false) {}
 DATAFLOW_EXPORT Absloc(MachRegister reg) :
  type_(Register),
     reg_(reg),
     off_(-1),
     region_(-1),
     func_(NULL),
     addr_(-1),
     preg_(),
     trueCond_(false)
     {}
    
 DATAFLOW_EXPORT Absloc(Address addr) :
    type_(Heap),
    reg_(),
    off_(-1),
    region_(-1),
    func_(NULL),
    addr_(addr),
    preg_(),
    trueCond_(false)
    {}
 DATAFLOW_EXPORT Absloc(int o,
			int r,
			ParseAPI::Function *f) :
    type_(Stack),
    reg_(),
    off_(o),
    region_(r),
    func_(f),
    addr_(-1),
    preg_(),
    trueCond_(false)
    {}
 DATAFLOW_EXPORT Absloc(MachRegister r, MachRegister p, bool c):
     type_(PredicatedRegister),
     reg_(r),
     off_(-1),
     region_(-1),
     func_(NULL),
     addr_(-1),
     preg_(p),
     trueCond_(c) {}
    
  DATAFLOW_EXPORT std::string format() const;

  DATAFLOW_EXPORT const Type &type() const { return type_; }

  DATAFLOW_EXPORT bool isValid() const { return type_ != Unknown; }

  DATAFLOW_EXPORT const MachRegister &reg() const { assert(type_ == Register || type_ == PredicatedRegister); return reg_; }

  DATAFLOW_EXPORT int off() const { assert(type_ == Stack); return off_; }
  DATAFLOW_EXPORT int region() const { assert(type_ == Stack); return region_; }
  DATAFLOW_EXPORT ParseAPI::Function *func() const { assert(type_ == Stack); return func_; }

  DATAFLOW_EXPORT Address addr() const { assert(type_ == Heap); return addr_; }
  DATAFLOW_EXPORT const MachRegister &predReg() const { assert(type_ == PredicatedRegister); return preg_;}
  DATAFLOW_EXPORT bool isTrueCondition() const { assert(type_ == PredicatedRegister); return trueCond_;}
  DATAFLOW_EXPORT void flipPredicateCondition() { assert(type_ == PredicatedRegister); trueCond_ = !trueCond_; }
  
  DATAFLOW_EXPORT bool operator<(const Absloc &rhs) const;
  DATAFLOW_EXPORT bool operator==(const Absloc &rhs) const;

  DATAFLOW_EXPORT bool operator!=(const Absloc &rhs) const {
    return !(*this == rhs);
  }

  DATAFLOW_EXPORT static char typeToChar(const Type t) {
    switch(t) {
    case Register:
      return 'r';
    case Stack:
      return 's';
    case Heap:
      return 'h';
    case PredicatedRegister:
      return 'p';
    default:
      return 'u';
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Absloc &a) {
    os << a.format();
    return os;
  }

 private:
  Type type_;

  MachRegister reg_;

  int off_;
  int region_;  
  ParseAPI::Function *func_;

  Address addr_;
  
  MachRegister preg_;
  bool trueCond_;
};

class AbsRegion {
 public:
  // Set operations get included here? Or third-party
  // functions?
  
  DATAFLOW_EXPORT bool contains(const Absloc::Type t) const;
  DATAFLOW_EXPORT bool contains(const Absloc &abs) const;
  DATAFLOW_EXPORT bool contains(const AbsRegion &rhs) const;
  // Logically, "intersect(rhs) != 0"
  //bool overlaps(const AbsRegion &rhs) const;

  DATAFLOW_EXPORT bool containsOfType(Absloc::Type t) const;

  //iterator &begin();
  //iterator &end();

  DATAFLOW_EXPORT bool operator==(const AbsRegion &rhs) const;
  DATAFLOW_EXPORT bool operator!=(const AbsRegion &rhs) const;
  DATAFLOW_EXPORT bool operator<(const AbsRegion &rhs) const;

  DATAFLOW_EXPORT const std::string format() const;

  DATAFLOW_EXPORT void insert(const Absloc &abs);
  DATAFLOW_EXPORT void insert(const AbsRegion &reg);

  DATAFLOW_EXPORT void erase(const Absloc &abs);
  DATAFLOW_EXPORT void erase(const AbsRegion &reg);

  DATAFLOW_EXPORT AbsRegion() :
    type_(Absloc::Unknown),
    size_(0) {}

  DATAFLOW_EXPORT AbsRegion(Absloc::Type t) :
    type_(t),
    size_(0) {}

  DATAFLOW_EXPORT AbsRegion(Absloc a) :
    type_(Absloc::Unknown),
      absloc_(a),
      size_(0) {}


  DATAFLOW_EXPORT void setGenerator(AST::Ptr generator) {
      generator_ = generator;
  }

  DATAFLOW_EXPORT void setSize(size_t size) {
    size_ = size;
  }

  DATAFLOW_EXPORT static bool equivalent(const AbsRegion &lhs,
			 const AbsRegion &rhs,
			 Address addr,
			 ParseAPI::Function *caller,
			 ParseAPI::Function *callee);

  DATAFLOW_EXPORT Absloc absloc() const { return absloc_; }
  DATAFLOW_EXPORT Absloc::Type type() const { return type_; }
  DATAFLOW_EXPORT size_t size() const { return size_; }
  DATAFLOW_EXPORT AST::Ptr generator() const { return generator_; }

  DATAFLOW_EXPORT bool isImprecise() const { return type_ != Absloc::Unknown; }
  DATAFLOW_EXPORT void flipPredicateCondition() { absloc_.flipPredicateCondition(); }
  friend std::ostream &operator<<(std::ostream &os, const AbsRegion &a) {
    os << a.format();
    return os;
  }

 private:
  // Type is for "we're on the stack but we don't know where".
  // Effectively, it's a wildcard.
  Absloc::Type type_;

  // For specific knowledge.
  Absloc absloc_;

  // And the AST that gave rise to this Absloc. We use this
  // as a generating function (if present and not overridden)
  AST::Ptr generator_;

  // Size in bits
  size_t size_;
};


class Assignment {
 public:
  typedef boost::shared_ptr<Assignment> Ptr;
  struct AssignmentPtrHasher {
    size_t operator() (const Ptr& ap) const noexcept {
      return (size_t)ap.get();
    }
  };

  typedef std::set<AbsRegion> Aliases;

  DATAFLOW_EXPORT const std::vector<AbsRegion> &inputs() const { return inputs_; }
  DATAFLOW_EXPORT std::vector<AbsRegion> &inputs() { return inputs_; }

  DATAFLOW_EXPORT const InstructionAPI::Instruction &insn() const { return insn_; }
  DATAFLOW_EXPORT InstructionAPI::Instruction &insn() { return insn_; }
  DATAFLOW_EXPORT Address addr() const { return addr_; }

  DATAFLOW_EXPORT const AbsRegion &out() const { return out_; }
  DATAFLOW_EXPORT AbsRegion &out() { return out_; }

  DATAFLOW_EXPORT const std::string format() const;

  // FIXME
  Aliases aliases;

  // Factory functions. 
  DATAFLOW_EXPORT static std::set<Assignment::Ptr> create(InstructionAPI::Instruction insn,
					  Address addr);

  DATAFLOW_EXPORT Assignment(const InstructionAPI::Instruction& i,
                             const Address a,
                             ParseAPI::Function *f,
                             ParseAPI::Block *b,
                             const std::vector<AbsRegion> &ins,
                             const AbsRegion &o) : 
    insn_(i),
       addr_(a),
       func_(f),
       block_(b),
       inputs_(ins),
       out_(o) {}

  DATAFLOW_EXPORT Assignment(const InstructionAPI::Instruction& i,
                             const Address a,
                             ParseAPI::Function *f,
                             ParseAPI::Block *b,
                             const AbsRegion &o) : 
    insn_(i),
       addr_(a),
       func_(f),
       block_(b),
       out_(o) {}

  DATAFLOW_EXPORT static Assignment::Ptr makeAssignment(const InstructionAPI::Instruction& i,
                             const Address a,
                             ParseAPI::Function *f,
                             ParseAPI::Block *b,
                             const std::vector<AbsRegion> &ins,
                             const AbsRegion &o);

  DATAFLOW_EXPORT static Assignment::Ptr makeAssignment(const InstructionAPI::Instruction& i,
                             const Address a,
                             ParseAPI::Function *f,
                             ParseAPI::Block *b,
                             const AbsRegion &o);


  // Internally used method; add a dependence on 
  // a new abstract region. If this is a new region
  // we'll add it to the dependence list. Otherwise 
  // we'll join the provided input set to the known
  // inputs.
  DATAFLOW_EXPORT void addInput(const AbsRegion &reg);
  DATAFLOW_EXPORT void addInputs(const std::vector<AbsRegion> &regions);

  DATAFLOW_EXPORT ParseAPI::Function *func() const { return func_; }

  DATAFLOW_EXPORT ParseAPI::Block *block() const { return block_; }
  friend std::ostream &operator<<(std::ostream &os, const Assignment::Ptr &a) {
    os << a->format();
    return os;
  }

 private:
  InstructionAPI::Instruction insn_;
  Address addr_;

  ParseAPI::Function *func_;
  ParseAPI::Block *block_;

  std::vector<AbsRegion> inputs_;
  AbsRegion out_;
};

// compare assignments by value.
// note this is a fast comparison--it checks output and address only.
struct AssignmentPtrValueComp {
    bool operator()(const Assignment::Ptr& a, const Assignment::Ptr& b) const {
        if (a->addr() < b->addr()) { return true; }
        if (b->addr() < a->addr()) { return false; }
        if (a->out() < b->out()) { return true; }
        return false;
    }
};

// Dyninst namespace
}



#endif

