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

#ifndef DYNINST_DATAFLOWAPI_STACKAST_H
#define DYNINST_DATAFLOWAPI_STACKAST_H

#include "AST.h"
#include "ASTVisitor.h"
#include "dyninst_visibility.h"
#include "stackanalysis.h"

#include <boost/make_shared.hpp>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

namespace Dyninst { namespace DataflowAPI {

  class StackAST : public AST {
  public:
    using Ptr = boost::shared_ptr<StackAST>;

    StackAST() = default;

    explicit StackAST(StackAnalysis::Height t) : t_{std::move(t)} {}

    static Ptr create(StackAnalysis::Height t) {
      return boost::make_shared<StackAST>(std::move(t));
    }

    std::string format() const override {
      std::stringstream ret;
      ret << "<" << t_ << ">";
      return ret.str();
    }

    AST::Ptr accept(ASTVisitor *v) override {
      return v->visit(this);
    }

    AST::ID getID() const override {
      return V_StackAST;
    }

    static Ptr convert(AST::Ptr a) {
      if(a->getID() == V_StackAST) {
        return boost::static_pointer_cast<StackAST>(a);
      }
      return Ptr{};
    }

    const StackAnalysis::Height &val() const {
      return t_;
    }

  private:
    bool isStrictEqual(AST const &rhs) const override {
      auto const *ptr = dynamic_cast<StackAST const *>(&rhs);
      if(!ptr) {
        return false;
      }
      return t_ == ptr->val();
    }

    StackAnalysis::Height t_;
  };

}}

#endif
