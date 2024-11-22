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

#if !defined(REGISTER_H)
#define REGISTER_H

#include "Architecture.h"
#include "Expression.h"
#include "registers/MachRegister.h"

#include <map>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

namespace Dyninst { namespace InstructionAPI {

  class DYNINST_EXPORT RegisterAST : public Expression {
    friend class MultiRegisterAST;

  public:
    typedef boost::shared_ptr<RegisterAST> Ptr;

    RegisterAST(MachRegister r, uint32_t num_elements = 1);
    RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit,
                uint32_t num_elements = 1);
    RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, Result_Type regType,
                uint32_t num_elements = 1);

    virtual ~RegisterAST();
    RegisterAST(const RegisterAST&) = default;

    virtual void getChildren(vector<InstructionAST::Ptr>& children) const;
    virtual void getChildren(vector<Expression::Ptr>& children) const;

    virtual void getUses(set<InstructionAST::Ptr>& uses);

    virtual bool isUsed(InstructionAST::Ptr findMe) const;

    virtual std::string format(Architecture, formatStyle how = defaultStyle) const;
    virtual std::string format(formatStyle how = defaultStyle) const;

    static RegisterAST makePC(Dyninst::Architecture arch);

    bool operator<(const RegisterAST& rhs) const;

    MachRegister getID() const;

    unsigned int lowBit() const { return m_Low; }

    unsigned int highBit() const { return m_High; }

    static RegisterAST::Ptr promote(const InstructionAST::Ptr reg);
    static RegisterAST::Ptr promote(const RegisterAST* reg);

    virtual void apply(Visitor* v);
    virtual bool bind(Expression* e, const Result& val);

    virtual bool isStrictEqual(const InstructionAST& rhs) const;

  protected:
    virtual bool isFlag() const;
    virtual bool checkRegID(MachRegister id, unsigned int low, unsigned int high) const;
    MachRegister getPromotedReg() const;

    MachRegister m_Reg;
    unsigned int m_Low;
    unsigned int m_High;
    unsigned int m_num_elements;
  };

  class DYNINST_EXPORT MaskRegisterAST : public RegisterAST {
  public:
    MaskRegisterAST(MachRegister r) : RegisterAST(r) {}

    MaskRegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit)
        : RegisterAST(r, lowbit, highbit) {}

    MaskRegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, Result_Type regType)
        : RegisterAST(r, lowbit, highbit, regType) {}

    virtual std::string format(Architecture, formatStyle how = defaultStyle) const;

    virtual std::string format(formatStyle how = defaultStyle) const;
  };
}}

#endif
