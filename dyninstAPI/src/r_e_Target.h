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

#if !defined (_R_E_TARGET_H_)
#define _R_E_TARGET_H_

#include "r_e_Base.h"

namespace Dyninst {
namespace Relocation {

// Wraps an object that can serve as a  control flow target. This
// may include existing code objects (a block or function)
// or something that has been relocated. We wrap them with this
// template class (which will then be specialized as appropriate)
// so we don't pollute the base class with extraneous code (I'm
// looking at _you_, get_address_cr....
//
// Preliminary requirement: T must be persistent during the existence
// of this class so we can use a reference to it. 

// adjAddr is a hack during code generation so we can predict where a Target
// will be off known data. Anything that's not being generated is static for
// these purposes, but an element *within* code generation may move.

class TargetInt {
 public:
  typedef enum {
    Illegal,
    BlockTarget,
    BBLTarget,
    PLTTarget } type_t;

  virtual Address addr() const { return 0; }
  virtual Address adjAddr(int, int) const { return 0; }
  virtual bool valid() const { return false; }
 TargetInt() : necessary_(true) {};
  virtual ~TargetInt() {};
  virtual std::string format() const { return "<INVALID>"; }

  virtual bool necessary() const { return necessary_; };
  virtual void setNecessary(bool a) { necessary_ = a; };

  virtual type_t type() const { return Illegal; };

  virtual bool matches(Block::Ptr) const { return false; };
 private:
  bool necessary_;
};

template <typename T>
class Target : public TargetInt{
 public:
  Address addr() const;
  Address adjAddr(int iteration, int shift) const;
  bool valid() const;
 Target(const T t) : t_(t) {};
  ~Target() {};

  const T t() { return t_; }

 private:
  const T &t_;

};

template <>
  class Target<Block::Ptr> : public TargetInt {
 public:
  Address addr() const { return t_->curAddr(); }
  Address adjAddr(int iteration, int shift) const {
    return ((t_->iteration() >= iteration) ? t_->curAddr() : (t_->curAddr() + shift));
  }

  bool valid() const { return addr() != 0; }
 Target(Block::Ptr t) : t_(t) {}
  ~Target() {}
  const Block::Ptr &t() const { return t_; };

  virtual type_t type() const { return BlockTarget; };

  virtual string format() const { 
    stringstream ret;
    ret << "B{" << t_->id() << "/" << (necessary() ? "T" : "S") << "}";
    return ret.str();
  }

  virtual bool matches(Block::Ptr next) const { return t_ == next; }

 private:
  const Block::Ptr t_;
};

template <>
class Target<bblInstance *> : public TargetInt {
 public:
  Address addr() const { return t_->firstInsnAddr(); }
  Address adjAddr(int, int) const { return addr(); }
  bool valid() const { return true; }
 Target(bblInstance *t) : t_(t) {}
  ~Target() {}

  bblInstance *t() const { return t_; };

  virtual type_t type() const { return BBLTarget; };
  
  virtual string format() const { 
    stringstream ret;
    ret << "O{" << std::hex << t_->firstInsnAddr() << std::dec << "}";
    return ret.str();
  }

 private:
  bblInstance *t_;
};


// This is a standin for a "PLT Entry" type
typedef Address PLT_Entry;
template <>
class Target<PLT_Entry> : public TargetInt {
 public:
  Address addr() const { return t_; }
  Address adjAddr(int, int) const { return addr(); }
  bool valid() const { return true; }
 Target(PLT_Entry t) : t_(t) {}
  ~Target() {}
  const PLT_Entry &t() const { return t_; }

  virtual type_t type() const { return PLTTarget; };

  virtual string format() const {
    stringstream ret;
    ret << "A{" << std::hex << t_ << std::dec << "}";
    return ret.str();
  }

 private:
  const PLT_Entry t_;
};

};
};
#endif
