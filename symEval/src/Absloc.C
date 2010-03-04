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

#include "Absloc.h"
#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/InstructionAST.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/Expression.h"

#include <sstream>

using namespace Dyninst;
// using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;
using namespace dyn_detail::boost;
using namespace std;
//////////////// Replace with generic version ////////////

////////////////
// FIXME architecture...
///////////////
bool Absloc::isPC() const { 
  if (type_ != Register) return false;
  return (reg_ == MachRegister::getPC(reg_.getArchitecture()));
}

bool Absloc::isSPR() const {
  if (type_ != Register) return false;
  return (reg_ == MachRegister::getStackPointer(reg_.getArchitecture()));
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
    ret << "S[" << func_ << "," << off_ << "]";
    break;
  }
  case Heap:
    ret << "_" << std::hex << addr_ << std::dec;
    break;
  default:
    ret << "(UNKNOWN)";
    break;
  }

  return ret.str();
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
  // See if any of our abslocs matches

  if (abslocs_.find(loc) == abslocs_.end()) return false;
  return true;
}  


bool AbsRegion::contains(const AbsRegion &rhs) const {
  if (type_ != Absloc::Unknown) {
    // We're a typed region, so we contain rhs
    // if either it has the same type as us or if all
    // of its abslocs are the same type
    if (rhs.type_ == type_) return true;
    for (std::set<Absloc>::const_iterator iter = rhs.abslocs_.begin();
	 iter != rhs.abslocs_.end(); ++iter) {
      if ((*iter).type() != type_) return false;

    }
    return true;
  }

  // We don't have a type, therefore we are a set. 
  // If the other is a type we cannot contain it, so iteration
  // is safe.
  for (std::set<Absloc>::const_iterator iter = rhs.abslocs_.begin();
       iter != rhs.abslocs_.end(); ++iter) {
    if (abslocs_.find(*iter) == abslocs_.end()) return false;
  }
  return true;
}

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

bool AbsRegion::containsOfType(Absloc::Type t) const {
  if (type_ == t) return true;

  for (std::set<Absloc>::const_iterator iter = abslocs_.begin();
       iter != abslocs_.end(); ++iter) {
    if ((*iter).type() == t) return true;
  }
  return false;
}

bool AbsRegion::operator==(const AbsRegion &rhs) const {
  return ((type_ == rhs.type_) &&
	  (abslocs_ == rhs.abslocs_));
}

void AbsRegion::insert(const Absloc &abs) {
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

  if (!abslocs_.empty()) {
    if (abslocs_.size() == 1) {
      ret << abslocs_.begin()->format();
    }
    else {
      ret << "[";
      for (std::set<Absloc>::const_iterator iter = abslocs_.begin();
	   iter != abslocs_.end(); ++iter) {
	ret << iter->format() << ",";
      }
      ret << "]";
    }
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
  /*
  for (unsigned i = 0; i < inputs_.size(); i++) {
    ret << ">" << inputs_[i].format();
  }
  */
  ret << ")";

  return ret.str();
}

