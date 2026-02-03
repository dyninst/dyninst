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

#ifndef DYNINST_DATAFLOWAPI_AST_H
#define DYNINST_DATAFLOWAPI_AST_H

#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"
#include "dyninst_visibility.h"

#include <cassert>
#include <vector>

namespace Dyninst {

  class ASTVisitor;

  class DYNINST_EXPORT AST : public boost::enable_shared_from_this<AST> {
  public:
    // This is a global list of all AST types.
    // If you add an AST type you should update this list.
    typedef enum {
      V_AST,
      // SymEval
      V_BottomAST,
      V_ConstantAST,
      V_VariableAST,
      V_RoseAST,
      // Stack analysis
      V_StackAST
    } ID;

    using Ptr = boost::shared_ptr<AST>;
    using Children = std::vector<AST::Ptr>;

    AST() = default;

    virtual ~AST() = default;

    bool operator==(const AST &rhs) const {
      // make sure rhs and this have the same type
      return (this->getID() == rhs.getID()) && isStrictEqual(rhs);
    }

    virtual unsigned numChildren() const {
      return 0;
    }

    virtual AST::Ptr child(unsigned) const {
      assert(0);
      return Ptr();
    }

    bool equals(AST::Ptr rhs) {
      if(!rhs) {
        return false;
      }
      return *this == (*rhs);
    }

    virtual std::string format() const = 0;

    // Substitute every occurrence of `a` with `b` in AST `in`.
    // Returns a new AST.
    static AST::Ptr substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b);

    virtual ID getID() const {
      return V_AST;
    }

    virtual Ptr accept(ASTVisitor *);

    Ptr ptr() {
      return shared_from_this();
    }

    virtual void setChild(int, AST::Ptr) {
      assert(0);
    }

  protected:
    virtual bool isStrictEqual(const AST &rhs) const = 0;
  };

}

#endif
