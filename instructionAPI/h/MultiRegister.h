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

#if !defined(MULTIREGISTER_H)
#define MULTIREGISTER_H

#include "Architecture.h"
#include "Expression.h"
#include "Register.h"
#include "registers/MachRegister.h"

#include <map>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

namespace Dyninst { namespace InstructionAPI {
  class DYNINST_EXPORT MultiRegisterAST : public Expression {
  public:
    typedef boost::shared_ptr<MultiRegisterAST> Ptr;

    MultiRegisterAST(MachRegister r, uint32_t num_elements = 1);
    MultiRegisterAST(std::vector<RegisterAST::Ptr> _in);

    virtual ~MultiRegisterAST() = default;
    MultiRegisterAST(const MultiRegisterAST&) = default;

    virtual void getChildren(vector<InstructionAST::Ptr>& children) const;
    virtual void getChildren(vector<Expression::Ptr>& children) const;

    virtual void getUses(set<InstructionAST::Ptr>& uses);

    virtual bool isUsed(InstructionAST::Ptr findMe) const;

    virtual std::string format(Architecture, formatStyle how = defaultStyle) const;
    virtual std::string format(formatStyle how = defaultStyle) const;

    bool operator<(const MultiRegisterAST& rhs) const;

    virtual void apply(Visitor* v);
    virtual bool bind(Expression* e, const Result& val);

    RegisterAST::Ptr getBaseRegAST() const { return m_Regs[0]; }

    uint32_t length() const { return m_Regs.size(); }

    const std::vector<RegisterAST::Ptr>& getRegs() const { return m_Regs; }

    bool areConsecutive() const { return consecutive; }

  protected:
    virtual bool checkRegID(MachRegister id, unsigned int low, unsigned int high) const;
    virtual bool isStrictEqual(const InstructionAST& rhs) const;
    virtual bool isFlag() const;

    std::vector<RegisterAST::Ptr> m_Regs;

  private:
    bool consecutive{false};
  };
}}

#endif
