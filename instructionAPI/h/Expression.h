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

#if !defined(VALUECOMPUTATION_H)
#define VALUECOMPUTATION_H

#include "InstructionAST.h"
#include "Result.h"

#include <sstream>
#include <string>
#include <vector>

namespace Dyninst { namespace InstructionAPI {

  class Expression;
  class Visitor;

  class DYNINST_EXPORT Expression : public InstructionAST {
  public:
    typedef boost::shared_ptr<Expression> Ptr;

  protected:
    Expression(Result_Type t);
    Expression(uint32_t total_size);
    Expression(MachRegister r);
    Expression(MachRegister r, uint32_t len);
    Expression(std::vector<MachRegister> rs);

  public:
    virtual ~Expression();
    Expression(const Expression&) = default;

    virtual const Result& eval() const;

    void setValue(const Result& knownValue);
    void clearValue();

    int size() const;

    virtual bool bind(Expression* expr, const Result& value);
    virtual void apply(Visitor*) {}

    virtual void getChildren(std::vector<Expression::Ptr>& children) const = 0;
    using InstructionAST::getChildren;

  protected:
    virtual bool isFlag() const;
    Result userSetValue;
  };

  class DYNINST_EXPORT DummyExpr : public Expression {
  public:
    virtual void getChildren(vector<InstructionAST::Ptr>&) const {}
    virtual void getChildren(vector<Expression::Ptr>&) const {}
    virtual void getUses(set<InstructionAST::Ptr>&) {}
    virtual bool isUsed(InstructionAST::Ptr) const { return true; }
    virtual std::string format(Architecture, formatStyle) const { return "[WILDCARD]"; }
    virtual std::string format(formatStyle) const { return "[WILDCARD]"; }
    DummyExpr() : Expression(u8) {}

  protected:
    virtual bool checkRegID(MachRegister, unsigned int = 0, unsigned int = 0) const;
    virtual bool isStrictEqual(const InstructionAST& rhs) const;
  };

}}

#endif
