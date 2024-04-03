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

#if !defined(AST_H)
#define AST_H

#include <assert.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include "util.h"
#include "boost/enable_shared_from_this.hpp"

namespace Dyninst {

class ASTVisitor;  

 class AST;

 // SymEval...
 namespace DataflowAPI {
 class BottomAST;
 class ConstantAST;
 class VariableAST;
 class RoseAST;
 }
 // Stack analysis...
 class StackAST;

 // Concolic execution...
 class InputVariableAST;
 class ReferenceAST;
 class StpAST;
 class YicesAST;
 class SemanticsAST;

#define DEF_AST_LEAF_TYPE(name, type)					\
class name : public AST {						\
 public:								\
 typedef boost::shared_ptr<name> Ptr;			\
 static Ptr create(type t) { return Ptr(new name(t)); }			\
 virtual ~name() {}							\
 virtual const std::string format() const {				\
   std::stringstream ret;						\
   ret << "<" << t_ << ">";						\
   return ret.str();							\
 }									\
 virtual AST::Ptr accept(ASTVisitor *v) { return v->visit(this); }	\
 virtual ID getID() const { return V_##name; }				\
  static Ptr convert(AST::Ptr a) {					\
  return ((a->getID() == V_##name) ? boost::static_pointer_cast<name>(a) : Ptr()); \
  }									\
  const type &val() const { return t_; }				\
 private:								\
 name(type t) : t_(t) {}						\
 virtual bool isStrictEqual(const AST &rhs) const {			\
   const name &other(dynamic_cast<const name&>(rhs));			\
   return t_ == other.t_;						\
 }									\
 const type t_;								\
 }									\

#define DEF_AST_INTERNAL_TYPE(name, type)				\
class name : public AST {						\
 public:								\
  typedef boost::shared_ptr<name> Ptr;			\
  virtual ~name() {}							\
  static Ptr create(type t, AST::Ptr a) { return Ptr(new name(t, a)); }	\
  static Ptr create(type t, AST::Ptr a, AST::Ptr b) { return Ptr(new name(t, a, b)); } \
  static Ptr create(type t, AST::Ptr a, AST::Ptr b, AST::Ptr c) { return Ptr(new name(t, a, b, c)); } \
  static Ptr create(type t, Children c) { return Ptr(new name(t, c)); }	\
  virtual const std::string format() const {				\
    std::stringstream ret;						\
    ret << t_ << "(";                                                   \
    for (Children::const_iterator i = kids_.begin(); i != kids_.end(); ++i) {	\
      ret << (*i)->format() << ",";					\
    }									\
    ret << ")";								\
    return ret.str();							\
  }									\
  virtual AST::Ptr child(unsigned i) const { return kids_[i];}		\
  virtual unsigned numChildren() const { return kids_.size();}		\
  virtual AST::Ptr accept(ASTVisitor *v) { return v->visit(this); }	\
  virtual ID getID() const { return V_##name; }				\
  static Ptr convert(AST::Ptr a) {					\
    return ((a->getID() == V_##name) ? boost::static_pointer_cast<name>(a) : Ptr()); \
  }									\
  const type &val() const { return t_; }				\
  void setChild(int i, AST::Ptr a) { kids_[i] = a; }			\
 private:								\
 name(type t, AST::Ptr a) : t_(t) { kids_.push_back(a); }		\
 name(type t, AST::Ptr a, AST::Ptr b) : t_(t) {				\
    kids_.push_back(a);							\
    kids_.push_back(b);							\
  }									\
 name(type t, AST::Ptr a, AST::Ptr b, AST::Ptr c) : t_(t) {		\
    kids_.push_back(a);							\
    kids_.push_back(b);							\
    kids_.push_back(c);							\
  }									\
 name(type t, Children kids) : t_(t), kids_(kids) {}			\
  virtual bool isStrictEqual(const AST &rhs) const {			\
    const name &other(dynamic_cast<const name&>(rhs));                  \
    if (!(t_ == other.t_)) return false;				\
    if (kids_.size() != other.kids_.size()) return false;               \
    for (unsigned i = 0; i < kids_.size(); ++i)                         \
      if (!(kids_[i]->equals(other.kids_[i]))) return false;            \
    return true;                                                        \
  }									\
  const type t_;							\
  Children kids_;							\
 }									\

class COMMON_EXPORT AST : public boost::enable_shared_from_this<AST> {
 public:

  typedef enum {
    V_AST,
    // SymEval
    V_BottomAST,
    V_ConstantAST,
    V_VariableAST,
    V_RoseAST,
    // Stack analysis
    V_StackAST,
    // Concolic execution
    V_InputVariableAST,
    V_ReferenceAST,
    V_StpAST,
    V_YicesAST,
    V_SemanticsAST } ID;

  typedef boost::shared_ptr<AST> Ptr;
  typedef std::vector<AST::Ptr> Children;      

  AST() {}
  virtual ~AST() {}
  
  bool operator==(const AST &rhs) const {
    // make sure rhs and this have the same type
    return((typeid(*this) == typeid(rhs)) && isStrictEqual(rhs));
  }

  virtual unsigned numChildren() const { return 0; }		       

  virtual AST::Ptr child(unsigned) const {				
    assert(0);								
    return Ptr();							
  }								       

  bool equals(AST::Ptr rhs) {
    if (!rhs) return false;
    return *this == (*rhs);
  }

  virtual const std::string format() const = 0;

  static AST::Ptr substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b); 

  // Detects if ``in`` exists in the set of ``visited`` nodes.
  static void hasCycle(AST::Ptr in,std::map<AST::Ptr, int> &visited);
  
  virtual ID getID() const { return V_AST; }

  virtual Ptr accept(ASTVisitor *);

  Ptr ptr() { return shared_from_this(); }

  virtual void setChild(int, AST::Ptr) {
    assert(0);
  }

 protected:
  virtual bool isStrictEqual(const AST &rhs) const = 0;
};

 class COMMON_EXPORT ASTVisitor {
 public:
   typedef boost::shared_ptr<AST> ASTPtr;

   virtual ASTPtr visit(AST *) {return AST::Ptr();}
   virtual ASTPtr visit(DataflowAPI::BottomAST *) {return AST::Ptr();}
   virtual ASTPtr visit(DataflowAPI::ConstantAST *) {return AST::Ptr();}
   virtual ASTPtr visit(DataflowAPI::VariableAST *) {return AST::Ptr();}
   virtual ASTPtr visit(DataflowAPI::RoseAST *) {return AST::Ptr();}
   virtual ASTPtr visit(StackAST *) {return AST::Ptr();}
   virtual ASTPtr visit(InputVariableAST *) {return AST::Ptr();}
   virtual ASTPtr visit(ReferenceAST *) {return AST::Ptr();}
   virtual ASTPtr visit(StpAST *) {return AST::Ptr();}
   virtual ASTPtr visit(YicesAST *) {return AST::Ptr();}
   virtual ASTPtr visit(SemanticsAST *) {return AST::Ptr();}

   virtual ~ASTVisitor() {}
 };

}
#endif // AST_H

