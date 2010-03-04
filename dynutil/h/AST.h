/*
 * Copyright (c) 2007-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(AST_H)
#define AST_H

#include <vector>
#include <string>
#include <sstream>
#include <dyn_detail/boost/shared_ptr.hpp>
#include <dyn_detail/boost/enable_shared_from_this.hpp>
#include "util.h"

namespace Dyninst {
using std::vector;

// We fully template the three types of nodes we have so that
// users can specify their own. This basically makes the AST
// a fully generic class. 
//
// TODO: do we want Variable and Constant to be different classes?
// I'm using the Absloc case as the basis here; EAX and '5' are
// very different things...
//
// Possible fourth template type: Type
// though I'm currently arguing that Type is an artifact of the
// Eval method you apply here. 
// ... and are Eval methods independent of Operation/Variable/Constant?
// I think they are...x

class AST {
 public:
  typedef dyn_detail::boost::shared_ptr<AST> Ptr;

  AST() {};
  virtual ~AST() {};
  
  bool operator==(const AST &rhs) const {
    // make sure rhs and this have the same type
    return((typeid(*this) == typeid(rhs)) && isStrictEqual(rhs));
  }
  
  virtual void getChildren(std::vector<Ptr> &) const = 0;

  virtual const std::string format() const = 0;

 protected:
  virtual bool isStrictEqual(const AST &rhs) const = 0;
};

class BottomAST : public AST {
 public:
  typedef dyn_detail::boost::shared_ptr<AST> Ptr;
  BottomAST() {};
  virtual ~BottomAST() {};

  virtual void getChildren(std::vector<AST::Ptr> &) const {};
  static AST::Ptr create() {
    return AST::Ptr(new BottomAST());
  }

  virtual const std::string format() const { return "<bottom>"; }

 protected:
  virtual bool isStrictEqual(const AST &) const {
    return true;
  }

};

template <typename V>
class VariableAST : public AST {
 public:
  VariableAST(const V &v) : v_(v) {};
  virtual ~VariableAST() {};

  virtual void getChildren(std::vector<AST::Ptr>&) const {};

  static AST::Ptr create(const V &v) {
    return AST::Ptr(new VariableAST(v));
  }

  virtual const std::string format() const { return v_.format(); }
 protected:
  virtual bool isStrictEqual(const AST &rhs) const {
    const VariableAST<V> &other(dynamic_cast<const VariableAST<V>&>(rhs));
    return (v_ == other.v_);
  }
  
 private:
  const V v_;
};

template <typename C>
class ConstantAST : public AST {
 public:
  ConstantAST(const C &c) : c_(c) {};
  virtual ~ConstantAST() {};

  virtual void getChildren(std::vector<AST::Ptr>&) const {};

  static AST::Ptr create(const C &c) {
    return AST::Ptr(new ConstantAST(c));
  }

  virtual const std::string format() const {
    std::stringstream ret;
    ret << "<" << c_ << ">";
    return ret.str();
 }

 protected:
  virtual bool isStrictEqual(const AST &rhs) const {
    const ConstantAST<C> &other(dynamic_cast<const ConstantAST<C>&>(rhs));
    return (c_ == other.c_);
  }
  
 private:
  const C c_;
};

template <typename O>
class UnaryAST : public AST {
 public:
  UnaryAST(const O &o, 
	   AST::Ptr a) :
    o_(o), a_(a) {};
  virtual ~UnaryAST() {};

  virtual void getChildren(std::vector<AST::Ptr>&kids) const {
    kids.push_back(a_);
  }

  static AST::Ptr create(const O &o, AST::Ptr a) {
    return AST::Ptr(new UnaryAST(o, a));
  }

  virtual const std::string format() const {
    std::stringstream ret;
    ret << o_.format() << "(" << a_->format() << ")";
    return ret.str();
  }


 protected:
  virtual bool isStrictEqual(const AST &rhs) const {
    const UnaryAST<O> &other(dynamic_cast<const UnaryAST<O>&>(rhs));
    return ((o_ == other.o_) && ((*a_) == (*(other.a_))));
  }
  
 private:
  const O o_;
  AST::Ptr a_;
};

template <typename O>
class BinaryAST : public AST {
 public:
  BinaryAST(const O &o, 
	    AST::Ptr a,
	    AST::Ptr b) :
    o_(o), a_(a), b_(b) {};
  virtual ~BinaryAST() {};

  virtual void getChildren(std::vector<AST::Ptr> &kids) const {
    kids.push_back(a_);
    kids.push_back(b_);
  }

  static AST::Ptr create(const O &o, AST::Ptr a, AST::Ptr b) {
    return AST::Ptr(new BinaryAST(o, a, b));
  }

  virtual const std::string format() const {
    std::stringstream ret;
    ret << o_.format() << "(" << a_->format() << "," << b_->format() << ")";
    return ret.str();
  }

 protected:
  virtual bool isStrictEqual(const AST &rhs) const {
    const BinaryAST<O> &other(dynamic_cast<const BinaryAST<O>&>(rhs));
    return ((o_ == other.o_) && 
	    ((*a_) == (*(other.a_))) &&
	    ((*b_) == (*(other.b_))));
  }
  
 private:
  const O o_;
  AST::Ptr a_;
  AST::Ptr b_;
};

template <typename O>
class TernaryAST : public AST {
 public:
  TernaryAST(const O &o, 
	     AST::Ptr a,
	     AST::Ptr b,
	     AST::Ptr c) : 
    o_(o), a_(a), b_(b), c_(c) {};
  virtual ~TernaryAST() {};

  virtual void getChildren(std::vector<AST::Ptr>&kids) const {
    kids.push_back(a_);
    kids.push_back(b_);
    kids.push_back(c_);
  }

  static AST::Ptr create(const O &o, AST::Ptr a, AST::Ptr b, AST::Ptr c) {
    return AST::Ptr(new TernaryAST(o, a, b, c));
  }

  virtual const std::string format() const {
    std::stringstream ret;
    ret << o_.format() << "(" << a_->format() << "," << b_->format() << "," << c_->format() << ")";
    return ret.str();
  }

 protected:
  virtual bool isStrictEqual(const AST &rhs) const {
    const TernaryAST<O> &other(dynamic_cast<const TernaryAST<O>&>(rhs));
    return ((o_ == other.o_) && 
	    ((*a_) == (*(other.a_))) &&
	    ((*b_) == (*(other.b_))) &&
	    ((*c_) == (*(other.c_))));
  }
  
 private:
  const O o_;
  AST::Ptr a_;
  AST::Ptr b_;
  AST::Ptr c_;
};

};
#endif // AST_H

