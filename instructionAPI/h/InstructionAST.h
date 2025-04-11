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

#if !defined(INSTRUCTIONAST_H)
#define INSTRUCTIONAST_H

#include "Architecture.h"
#include "dyninst_visibility.h"
#include "registers/MachRegister.h"
#include "Result.h"

#include <boost/enable_shared_from_this.hpp>
#include <set>
#include <string>
#include <vector>

namespace Dyninst { namespace InstructionAPI {

  enum formatStyle { defaultStyle, memoryAccessStyle };

  class DYNINST_EXPORT InstructionAST : public boost::enable_shared_from_this<InstructionAST> {
  public:
    using Ptr = boost::shared_ptr<InstructionAST>;

    InstructionAST() = default;
    InstructionAST(const InstructionAST &) = default;
    InstructionAST(InstructionAST &&) = default;
    InstructionAST &operator=(InstructionAST const &) = default;
    InstructionAST &operator=(InstructionAST &&) = default;
    virtual ~InstructionAST() = default;

    bool operator==(const InstructionAST &rhs) const {
      // isStrictEqual assumes rhs and this to be of the same derived type
      return ((typeid(*this) == typeid(rhs)) && isStrictEqual(rhs));
    }

    virtual void getChildren(std::vector<InstructionAST::Ptr> &children) const = 0;

    virtual void getUses(std::set<InstructionAST::Ptr> &uses) = 0;
    virtual bool isUsed(InstructionAST::Ptr findMe) const = 0;

    virtual std::string format(Dyninst::Architecture arch, formatStyle how = defaultStyle) const = 0;
    virtual std::string format(formatStyle how = defaultStyle) const = 0;

  protected:
    friend class MultiRegisterAST;
    friend class RegisterAST;
    friend class Immediate;

    virtual bool isStrictEqual(const InstructionAST &rhs) const = 0;

    virtual bool checkRegID(Dyninst::MachRegister, unsigned int = 0, unsigned int = 0) const {
      return false;
    }

    virtual const Result &eval() const = 0;
  };
}}

#endif
