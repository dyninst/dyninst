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

#include "ArchSpecificFormatters.h"
#include "InstructionDecoder-power.h"
#include "Ternary.h"
#include "Visitor.h"

#include <set>
#include <sstream>
#include <vector>

using namespace std;

namespace Dyninst { namespace InstructionAPI {
  TernaryAST::TernaryAST(Expression::Ptr c, Expression::Ptr f, Expression::Ptr s,
                         Result_Type result_type_)
      : Expression(result_type_), cond(c), first(f), second(s) {}

  TernaryAST::~TernaryAST() {}

  void TernaryAST::getChildren(vector<InstructionAST::Ptr>& /*children*/) const { return; }

  void TernaryAST::getChildren(vector<Expression::Ptr>& /*children*/) const { return; }

  void TernaryAST::getUses(set<InstructionAST::Ptr>& uses) {
    uses.insert(shared_from_this());
    return;
  }

  bool TernaryAST::isUsed(InstructionAST::Ptr) const {
    return false; // TODO
  }

  std::string TernaryAST::format(Architecture, formatStyle f) const {
    return TernaryAST::format(f); // TODO
  }

  std::string TernaryAST::format(formatStyle) const {
    std::string name = "(" + cond->format() + "?" + first->format() + ":" + second->format() + ")";
    for(auto& c : name)
      c = ::toupper(c);
    return name;
  }

  bool TernaryAST::operator<(const TernaryAST&) const { return false; }

  bool TernaryAST::isStrictEqual(const InstructionAST&) const { return false; }

  void TernaryAST::apply(Visitor*) {}

  bool TernaryAST::bind(Expression*, const Result&) {
    return false; // TODO
  }
}}
