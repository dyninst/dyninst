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

#include "Absloc.h"
#include <assert.h>

#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/InstructionAST.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/Expression.h"

#include "dataflowAPI/h/stackanalysis.h"

#include "parseAPI/h/CFG.h"

#include <sstream>

#include "../../common/src/singleton_object_pool.h"

using namespace Dyninst;
// using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;
using namespace std;
//////////////// Replace with generic version ////////////

////////////////
// FIXME architecture...
///////////////
bool Absloc::isPC() const { 
  if (type_ != Register) return false;
  return (reg_ == MachRegister::getPC(reg_.getArchitecture()));
}

bool Absloc::isSP() const {
  if (type_ != Register) return false;
  return (reg_ == MachRegister::getStackPointer(reg_.getArchitecture()));
}

bool Absloc::isFP() const {
  if (type_ != Register) return false;
  return (reg_ == MachRegister::getFramePointer(reg_.getArchitecture()));
}

Absloc Absloc::makePC(Architecture arch) {
  return Absloc(MachRegister::getPC(arch));
}

Absloc Absloc::makeSP(Architecture arch) {
  return Absloc(MachRegister::getStackPointer(arch));
}

Absloc Absloc::makeFP(Architecture arch) {
  return Absloc(MachRegister::getFramePointer(arch));
}

std::string Absloc::format() const {
  std::stringstream ret;
  
  switch(type_) {
  case Register:
    // TODO: I'd like a "current architecture" global
    // and a int->register converter...
    ret << reg_.name();
    break;
  case Stack: {
    if (func_)
        ret << "S[" << func_->name() << "," << off_ << "," << region_ << "]";
    else 
        ret << "S[NULL_FUNC" << "," << off_ << "," << region_ << "]";
    break;
  }
  case Heap:
    ret << "_" << std::hex << addr_ << std::dec;
    break;
  case PredicatedRegister:
    ret << "PRED_REG[";
    if (!trueCond_) ret << "!";
    ret << preg_.name() << "," << reg_.name() << "]";
    break;
  default:
    ret << "(UNKNOWN)";
    break;
  }

  return ret.str();
}

bool Absloc::operator<(const Absloc & rhs) const {
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
    case PredicatedRegister:
      if (reg_ != rhs.reg_)
          return reg_ < rhs.reg_;
      if (preg_ != rhs.preg_)
          return preg_ < rhs.preg_;
      return trueCond_ < rhs.trueCond_;
    case Unknown:
       return false; // everything is less than an unknown
    }
    assert(0);
    return true;
}

bool Absloc::operator==(const Absloc & rhs) const {
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
    case PredicatedRegister:
      return (reg_ == rhs.reg_) && (preg_ == rhs.preg_) && (trueCond_ == rhs.trueCond_);
    default:
      return true;
    }
  }


bool AbsRegion::contains(const Absloc::Type t) const {
  // Abslocs, if they exist, must be specific.
  // So just check our type
  return (type_ == t);
}

bool AbsRegion::contains(const Absloc &loc) const {
  if (type_ != Absloc::Unknown) {
    // If we're a typed region we contain any absloc
    // with our type
     return (type_ == loc.type());
  }

  //if (loc.type() != Absloc::Unknown) {
  //return (type() == loc.type());
  //}

  // See if any of our abslocs matches
  if (absloc_ == loc) return true;

  if (loc.type() == Absloc::Unknown) {
    cerr << "Weird case: comp " << format() << " /w/ " << loc.format() << endl;
  }

  return false;
}  


bool AbsRegion::contains(const AbsRegion &rhs) const {
  if (type_ != Absloc::Unknown) {
    // We're a typed region, so we contain rhs
    // if either it has the same type as us or if all
    // of its abslocs are the same type
    if (rhs.type_ == type_) return true;
    if (rhs.absloc_.type() == type_) return true;
    return false;
  }

  if (rhs.type() != Absloc::Unknown) {
     if (absloc_.type() == rhs.type()) return true;
     return false;
  }

  if (absloc_ == rhs.absloc_) return true;

  // If rhs is a predicated register and the lhs is a non-predicated register,
  // then the lhs contains the rhs when the base registers are the same.
  if (rhs.absloc_.type() == Absloc::PredicatedRegister && rhs.absloc_.reg() == absloc_.reg()) return true;

  // Stack slots operate kinda... odd...
  if ((absloc_.type() == Absloc::Stack) &&
      (rhs.absloc_.type() == Absloc::Stack)) {
         
    // Testing: assume regions do not overlap
    return false;

    // Return true if we're in the same function but different
    // regions    
    if ((absloc_.func() == rhs.absloc_.func()) &&
	(absloc_.region() != rhs.absloc_.region())) return true;
  }

  return false;
}
/*
bool AbsRegion::overlaps(const AbsRegion &rhs) const {
  if (type_ != Absloc::Unknown) {
    // We're a typed region, so we contain rhs
    // if either it has the same type as us or if all
    // of its abslocs are the same type
    if (rhs.type_ == type_) return true;
    for (std::set<Absloc>::const_iterator iter = rhs.abslocs_.begin();
	 iter != rhs.abslocs_.end(); ++iter) {
      if ((*iter).type() == type_) return true;
    }
    return false;
  }

  // We don't have a type, therefore we are a set. 
  // If they are a type...
  if (rhs.type_ != Absloc::Unknown) {
    return containsOfType(rhs.type_);
  }

  // Neither a type, so see if there is any overlap in our sets.

  for (std::set<Absloc>::const_iterator iter = rhs.abslocs_.begin();
       iter != rhs.abslocs_.end(); ++iter) {
    if (abslocs_.find(*iter) != abslocs_.end()) {
      return true;
    }
  }
  return false;
}
*/

bool AbsRegion::containsOfType(Absloc::Type t) const {
  if (type_ == t) return true;

  if (absloc_.type() == t) return true;
  return false;
}

bool AbsRegion::operator==(const AbsRegion &rhs) const {
  // return contains(rhs) && rhs.contains(*this));
  return ((type_ == rhs.type_) &&
	  (absloc_ == rhs.absloc_));
}

bool AbsRegion::operator!=(const AbsRegion &rhs) const { 
  return ((type_ != rhs.type_) ||
	  (absloc_ != rhs.absloc_));
}


bool AbsRegion::operator<(const AbsRegion &rhs) const {
   // Anything with a valid AbsLoc is less than anything with an 
   // invalid AbsLoc. 
   

   if (absloc_ < rhs.absloc_) {
      return true;
   }
   if (rhs.absloc_ < absloc_) {
      return false;
   }

   return type() < rhs.type();
}

/*
void AbsRegion::insert(const Absloc &abs) {
  assert(a
  if (type_ != Absloc::Unknown) 
    assert(0 && "Unimplemented");
  abslocs_.insert(abs);
}

void AbsRegion::insert(const AbsRegion &rhs) {
  if (type_ != Absloc::Unknown)
    assert(0 && "Unimplemented");
  if (rhs.type_ != Absloc::Unknown)
    assert(0 && "Unimplemented");

  abslocs_.insert(rhs.abslocs_.begin(),
		  rhs.abslocs_.end());
}

void AbsRegion::erase(const Absloc &rhs) {
  if (type_ != Absloc::Unknown)
    assert(0 && "Unimplemented");
  abslocs_.erase(rhs);
}

void AbsRegion::erase(const AbsRegion &rhs) {
  if (type_ != Absloc::Unknown)
    assert(0 && "Unimplemented");
  if (rhs.type_ != Absloc::Unknown)
    assert(0 && "Unimplemented");

  abslocs_.erase(rhs.abslocs_.begin(),
		 rhs.abslocs_.end());
}
*/

Assignment::Ptr Assignment::makeAssignment(const InstructionAPI::Instruction& i,
                             const Address a,
                             ParseAPI::Function *f,
                             ParseAPI::Block *b,
                             const std::vector<AbsRegion> &ins,
                             const AbsRegion &o) {
      return make_shared(singleton_object_pool<Assignment>::construct(i, a, f, b, ins, o));
}

Assignment::Ptr Assignment::makeAssignment(const InstructionAPI::Instruction& i,
                             const Address a,
                             ParseAPI::Function *f,
                             ParseAPI::Block *b,
                             const AbsRegion &o) {
      return  make_shared(singleton_object_pool<Assignment>::construct(i, a, f, b, o));

}			     

void Assignment::addInput(const AbsRegion &reg) {
  inputs_.push_back(reg);
}

void Assignment::addInputs(const std::vector<AbsRegion> &region) {
  for (unsigned i = 0; i < region.size(); ++i) {
    inputs_.push_back(region[i]);
  }
}

const std::string AbsRegion::format() const {
  std::stringstream ret;

  if (absloc_ != Absloc()) {
    ret << "[" << absloc_.format();
    if (size_) ret << ":" << size_;
    ret << "]";
  }
  else {
    switch(type_) {
    case Absloc::Register:
      ret << "R[]";
      break;
    case Absloc::Stack:
      ret << "S[]";
      break;
    case Absloc::Heap:
      ret << "H[]";
      break;
    default:
      ret << "?[];";
      break;
    }
  }
  return ret.str();
}

const std::string Assignment::format() const {
  // Err....
  std::stringstream ret;
  ret << "(@"<< std::hex << addr_ << std::dec
      << "<" << out_.format();
  for (unsigned i = 0; i < inputs_.size(); i++) {
    ret << ">" << inputs_[i].format();
  }
  ret << ")";

  return ret.str();
}



#if 0
bool AbsRegion::equivalent(const AbsRegion &lhs,
			   const AbsRegion &rhs,
			   Address addr,
			   ParseAPI::Function *caller,
			   ParseAPI::Function *callee) {
  // Check equivalence given a particular location (and thus
  // possible stack overlap)
  if (lhs == rhs) return true;
  if (lhs.abslocs().empty() || rhs.abslocs().empty()) return false;

  if (lhs.abslocs().size() > 1) return false;
  if (rhs.abslocs().size() > 1) return false;

  // Only stack slots can overlap (for now)
  const Absloc &lLoc = *(lhs.abslocs().begin());
  const Absloc &rLoc = *(rhs.abslocs().begin());
  if (lLoc.type() != Absloc::Stack) return false;
  if (rLoc.type() != Absloc::Stack) return false;

  int caller_offset = -1;
  int callee_offset = -1;

  if (lLoc.func() == caller->name()) {
    if (rLoc.func() != callee->name()) return false;
    caller_offset = lLoc.off();
    callee_offset = rLoc.off();
  }
  else if (rLoc.func() == caller->name()) {
    if (lLoc.func() != callee->name()) return false;
    caller_offset = rLoc.off();
    callee_offset = lLoc.off();
  }
  else {
    return false;
  }

  StackAnalysis sA(caller);

  StackAnalysis::Height heightSA = sA.findSP(addr);

  // Ensure that analysis has been performed.
  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }

  if ((caller_offset - heightSA.height()) == callee_offset)
    return true;
  else
    return false;
}
#endif
