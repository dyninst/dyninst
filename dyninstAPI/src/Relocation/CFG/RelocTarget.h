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

#if !defined (_R_E_TARGET_H_)
#define _R_E_TARGET_H_

#include <assert.h>
#include <string>
#include "../Widgets/Widget.h"
#include "RelocBlock.h"

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

// predictedAddr takes into account things moving during code generation

class CodeBuffer;
struct RelocEdge;

class TargetInt {
 public:
  typedef enum {
    Illegal,
    RelocBlockTarget,
    BlockTarget,
    AddrTarget, } type_t;

  TargetInt() : necessary_(true) {}
  virtual ~TargetInt() {}
  virtual std::string format() const { return "<INVALID>"; }

  virtual Address origAddr() const = 0;

  // It would be nice to eventually move these into the code generator loop, but
  // for now it's okay to keep them here. 
  virtual bool necessary() const { return necessary_; }
  virtual void setNecessary(bool a) { necessary_ = a; }

  virtual type_t type() const { return Illegal; }
  
  virtual bool matches(RelocBlock *) const { return false; }
  virtual int label(CodeBuffer *) const { return -1; }

  virtual void addTargetEdge(RelocEdge *) {}
  virtual void addSourceEdge(RelocEdge *) {}
  virtual void removeTargetEdge(RelocEdge *) {}
  virtual void removeSourceEdge(RelocEdge *) {}
  virtual block_instance *block() { return NULL; }

  virtual TargetInt *copy() const {return NULL; }

  protected:

  bool necessary_;
};

template <typename T>
class Target : public TargetInt{
 public:
   //Address addr() const;
 Target(const T t) : t_(t) {}
  ~Target() {}

  const T t() { return t_; }

 private:
  const T &t_;
};

template <>
  class Target<RelocBlock *> : public TargetInt {
 public:
   //Address addr() const { return t_->curAddr(); }

  Target(RelocBlock * t) : t_(t) { assert(t_); }
   ~Target() {}
   RelocBlock * t() const { return t_; }
  Address origAddr() const { return t_->origAddr(); }
  
  virtual type_t type() const { return RelocBlockTarget; }
  
  virtual string format() const { 
     stringstream ret;
     ret << "T{" << t_->id() << "/" << (necessary() ? "+" : "-") << "}";
     return ret.str();
  }
  
  virtual bool matches(RelocBlock *t) const { return (t_ == t); }

  int label(CodeBuffer *) const { return t_->getLabel(); }
  
  virtual void addTargetEdge(RelocEdge *e);
  virtual void addSourceEdge(RelocEdge *e);
  virtual void removeTargetEdge(RelocEdge *e);
  virtual void removeSourceEdge(RelocEdge *e);
  virtual block_instance *block();

  virtual TargetInt *copy() const { return new Target<RelocBlock *>(t_); }

 private:
  RelocBlock * t_;
};

template <>
class Target<block_instance *> : public TargetInt {
 public:
   //Address addr() const { return t_->firstInsnAddr(); }
  Target(block_instance *t) : t_(t) { assert(t_); }
  ~Target() {}

  block_instance *t() const { return t_; }

  virtual type_t type() const { return BlockTarget; }

  Address origAddr() const { return t_->start(); }
  
  virtual string format() const { 
    stringstream ret;
    ret << "B{" << std::hex << t_->start() << "/" << (necessary() ? "+" : "-") << std::dec << "}";
    return ret.str();
  }

  int label(CodeBuffer *) const;
  virtual block_instance *block() { return t_; }

  virtual TargetInt *copy() const { return new Target<block_instance *>(t_); }

 private:
  block_instance *t_;
};


template <>
class Target<Address> : public TargetInt {
 public:
   //Address addr() const { return t_; }
  Target(Address t) : t_(t) {}
  ~Target() {}
  const Address &t() const { return t_; }

  virtual type_t type() const { return AddrTarget; }

  Address origAddr() const { return t_; }

  virtual string format() const {
    stringstream ret;
    ret << "A{" << std::hex << t_ << "/" << (necessary() ? "+" : "-") <<  std::dec << "}";
    return ret.str();
  }

  int label(CodeBuffer *) const;

  virtual TargetInt *copy() const { return new Target<Address>(t_); }

 private:
  const Address t_;
};

}
}
#endif
