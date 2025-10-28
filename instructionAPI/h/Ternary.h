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

#if !defined(TERNARY_H)
#define TERNARY_H

#include "Architecture.h"
#include "Expression.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace Dyninst { namespace InstructionAPI {

  class DYNINST_EXPORT TernaryAST : public Expression {
  public:
    typedef boost::shared_ptr<TernaryAST> Ptr;

    TernaryAST(Expression::Ptr cond, Expression::Ptr first, Expression::Ptr second,
               Result_Type result_type);

    virtual ~TernaryAST();

    virtual bool isUsed(Expression::Ptr findMe) const;

    virtual std::string format(Architecture, formatStyle how = defaultStyle) const;
    virtual std::string format(formatStyle how = defaultStyle) const;

    bool operator<(const TernaryAST& rhs) const;

    virtual void apply(Visitor* v);
    virtual bool bind(Expression* e, const Result& val);
    Expression::Ptr cond;
    Expression::Ptr first;
    Expression::Ptr second;
    Result_Type result_type{};

  protected:
    virtual bool isStrictEqual(const Expression& rhs) const;
  };

}}

#endif
