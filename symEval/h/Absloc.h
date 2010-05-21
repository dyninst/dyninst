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

#include <values.h>

#include "Instruction.h"
#include "AST.h"

class image_func;

namespace Dyninst {

class Absloc {
 public:
  typedef enum {
    Register,
    Stack,
    Heap,
    Unknown } Type;

  static Absloc makePC(Dyninst::Architecture arch);
  static Absloc makeSP(Dyninst::Architecture arch);
  static Absloc makeFP(Dyninst::Architecture arch);
  
  // Some static functions for "well-known" Abslocs
  bool isPC() const;
  bool isSPR() const;
  
  bool isSP() const;
  bool isFP() const;

 Absloc() :
  type_(Unknown),
    reg_(),
    off_(-1),
    region_(-1),
    addr_(-1) {};
 Absloc(MachRegister reg) :
  type_(Register),
    reg_(reg) {};
    
 Absloc(Address addr) :
  type_(Heap),
    addr_(addr) {};
 Absloc(int o,
	int r,
	const std::string &f) :
    type_(Stack),
      off_(o),
      region_(r),
      func_(f) {};
    
  std::string format() const;

  const Type &type() const { return type_; };

  const MachRegister &reg() const { assert(type_ == Register); return reg_; };

  int off() const { assert(type_ == Stack); return off_; };
  int region() const { assert(type_ == Stack); return region_; };
  const std::string &func() const { assert(type_ == Stack); return func_; };
  
  bool operator<(const Absloc &rhs) const {
    if (type_ != rhs.type_) 
      return type_ < rhs.type_;
    switch(type_) {
    case Register:
      return reg_ < rhs.reg_;
    case Stack:
      if (off_ != rhs.off_)
	return off_ < rhs.off_;
      // Now we get arbitrary
      if (region_ != rhs.region_)
	return region_ < rhs.region_;
      return func_ < rhs.func_;
    case Heap:
      return addr_ < rhs.addr_;
    default:
      return true;
    }
  }

  bool operator==(const Absloc &rhs) const {
    if (type_ != rhs.type_) return false;
    switch(type_) {
    case Register:
      return reg_ == rhs.reg_;
    case Stack:
      return ((off_ == rhs.off_) &&
	      (region_ == rhs.region_) &&
	      (func_ == rhs.func_));
    case Heap:
      return addr_ == rhs.addr_;
    default:
      return true;
    }
  }

  bool operator!=(const Absloc &rhs) const {
    return !(*this == rhs);
  }

  static char typeToChar(const Type t) {
    switch(t) {
    case Register:
      return 'r';
    case Stack:
      return 's';
    case Heap:
      return 'h';
    default:
      return 'u';
    }
  };

 private:
  Type type_;

  MachRegister reg_;

  int off_;
  int region_;  
  std::string func_;

  Address addr_;
};

class AbsRegion {
 public:
  // Set operations get included here? Or third-party
  // functions?
  
  bool contains(const Absloc::Type t) const;
  bool contains(const Absloc &abs) const;
  bool contains(const AbsRegion &rhs) const;
  // Logically, "intersect(rhs) != 0"
  //bool overlaps(const AbsRegion &rhs) const;

  bool containsOfType(Absloc::Type t) const;

  //iterator &begin();
  //iterator &end();

  bool operator==(const AbsRegion &rhs) const;
  bool operator!=(const AbsRegion &rhs) const;
  bool operator<(const AbsRegion &rhs) const;

  const std::string format() const;

  void insert(const Absloc &abs);
  void insert(const AbsRegion &reg);

  void erase(const Absloc &abs);
  void erase(const AbsRegion &reg);

  AbsRegion() :
    type_(Absloc::Unknown) {};

  AbsRegion(Absloc::Type t) :
    type_(t) {};

  AbsRegion(Absloc a) :
    type_(Absloc::Unknown),
      absloc_(a) {};


  void setGenerator(AST::Ptr generator) {
      generator_ = generator;
  }

  static bool equivalent(const AbsRegion &lhs,
			 const AbsRegion &rhs,
			 Address addr,
			 image_func *caller,
			 image_func *callee);

  const Absloc absloc() const { return absloc_; }
  const Absloc::Type type() const { return type_; }

 private:
  // Type is for "we're on the stack but we don't know where".
  // Effectively, it's a wildcard.
  Absloc::Type type_;

  // For specific knowledge.
  Absloc absloc_;

  // And the AST that gave rise to this Absloc. We use this
  // as a generating function (if present and not overridden)
  AST::Ptr generator_;
};


class Assignment {
 public:
  typedef dyn_detail::boost::shared_ptr<Assignment> Ptr;
  typedef std::set<AbsRegion> Aliases;

  const std::vector<AbsRegion> &inputs() const { return inputs_; }
  std::vector<AbsRegion> &inputs() { return inputs_; }

  const InstructionAPI::Instruction::Ptr insn() const { return insn_; }
  const Address addr() const { return addr_; }

  const AbsRegion &out() const { return out_; }
  AbsRegion &out() { return out_; }

  const std::string format() const;

  // FIXME
  Aliases aliases;

  // Factory functions. 
  static std::set<Assignment::Ptr> create(InstructionAPI::Instruction::Ptr insn,
					  Address addr);

  Assignment(const InstructionAPI::Instruction::Ptr i,
	     const Address a,
	     image_func *f,
	     const std::vector<AbsRegion> &ins,
	     const AbsRegion &o) : 
    insn_(i),
    addr_(a),
      func_(f),
    inputs_(ins),
    out_(o) {};

  Assignment(const InstructionAPI::Instruction::Ptr i,
	     const Address a,
	     image_func *f,
	     const AbsRegion &o) : 
    insn_(i),
    addr_(a),
      func_(f),
    out_(o) {};

  // Internally used method; add a dependence on 
  // a new abstract region. If this is a new region
  // we'll add it to the dependence list. Otherwise 
  // we'll join the provided input set to the known
  // inputs.
  void addInput(const AbsRegion &reg);
  void addInputs(const std::vector<AbsRegion> &regions);

  image_func *func() const { return func_; }

 private:
  InstructionAPI::Instruction::Ptr insn_;
  Address addr_;

  image_func *func_;

  std::vector<AbsRegion> inputs_;
  AbsRegion out_;
};

// Dyninst namespace
};


std::ostream &operator<<(std::ostream &os, const Dyninst::Absloc &a);
std::ostream &operator<<(std::ostream &os, const Dyninst::AbsRegion &a);
std::ostream &operator<<(std::ostream &os, const Dyninst::Assignment::Ptr &a);

#endif

