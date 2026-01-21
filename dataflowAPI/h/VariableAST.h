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

#ifndef DYNINST_DATAFLOWAPI_VARIABLEAST_H
#define DYNINST_DATAFLOWAPI_VARIABLEAST_H

#include "Absloc.h"
#include "AST.h"
#include "ASTVisitor.h"
#include "dyninst_visibility.h"
#include "dyntypes.h"

#include <boost/make_shared.hpp>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

namespace Dyninst { namespace DataflowAPI {

  struct DYNINST_EXPORT Variable {

    Variable() = default;

    explicit Variable(Dyninst::AbsRegion r) : reg(std::move(r)), addr(0UL) {}

    Variable(Dyninst::AbsRegion r, Dyninst::Address a) : reg(std::move(r)), addr(a) {}

    bool operator==(const Variable &rhs) const {
      return ((rhs.addr == addr) && (rhs.reg == reg));
    }

    bool operator<(const Variable &rhs) const {
      if(addr < rhs.addr) {
        return true;
      }
      if(reg < rhs.reg) {
        return true;
      }
      return false;
    }

    std::string format() const {
      std::stringstream ret;
      ret << "V(" << reg;
      if(addr) {
        ret << ":" << std::hex << addr << std::dec;
      }
      ret << ")";
      return ret.str();
    }

    friend std::ostream &operator<<(std::ostream &stream, const Variable &c) {
      return stream << c.format();
    }

    Dyninst::AbsRegion reg{};
    Dyninst::Address addr{Dyninst::ADDR_NULL};
  };

  class DYNINST_EXPORT VariableAST : public Dyninst::AST {
  public:
    using Ptr = boost::shared_ptr<VariableAST>;

    VariableAST() = default;

    explicit VariableAST(Variable t) : t_(std::move(t)) {}

    static Ptr create(Variable t) {
      return boost::make_shared<VariableAST>(std::move(t));
    }

    std::string format() const override {
      std::stringstream ret;
      ret << "<" << t_ << ">";
      return ret.str();
    }

    AST::Ptr accept(Dyninst::ASTVisitor *v) override {
      return v->visit(this);
    }

    AST::ID getID() const override {
      return V_VariableAST;
    }

    static Ptr convert(Dyninst::AST::Ptr a) {
      if(a->getID() == V_VariableAST) {
        return boost::static_pointer_cast<VariableAST>(a);
      }
      return Ptr{};
    }

    Variable const &val() const {
      return t_;
    }

  private:
    bool isStrictEqual(Dyninst::AST const &rhs) const override {
      auto const *ptr = dynamic_cast<VariableAST const *>(&rhs);
      if(!ptr) {
        return false;
      }
      return t_ == ptr->val();
    }

    Variable t_;
  };

}}

#endif
