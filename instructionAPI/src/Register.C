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

#include "../../common/h/compiler_diagnostics.h"
#include "ArchSpecificFormatters.h"
#include "Architecture.h"
#include "InstructionDecoder-power.h"
#include "Register.h"
#include "Visitor.h"
#include "registers/MachRegister.h"
#include "registers/x86_regs.h"

#include <boost/make_shared.hpp>
#include <set>
#include <sstream>
#include <vector>

using namespace std;

namespace Dyninst { namespace InstructionAPI {
  RegisterAST::RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit,
                           uint32_t num_elements)
      : Expression(r), m_Reg(r), m_Low(lowbit), m_High(highbit), m_num_elements(num_elements) {}

  RegisterAST::RegisterAST(MachRegister r, uint32_t num_elements)
      : Expression(r), m_Reg(r), m_Low(0), m_num_elements(num_elements) {

    m_High = r.size() * 8;
  }

  RegisterAST::RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit,
                           Result_Type regType, uint32_t num_elements)
      : Expression(regType), m_Reg(r), m_Low(lowbit), m_High(highbit),
        m_num_elements(num_elements) {}

  RegisterAST::~RegisterAST() {}

  void RegisterAST::getChildren(vector<Expression::Ptr>& /*children*/) const { return; }

  void RegisterAST::getUses(set<Expression::Ptr>& uses) {
    uses.insert(shared_from_this());
    return;
  }

  bool RegisterAST::isUsed(Expression::Ptr findMe) const {
    return findMe->checkRegID(m_Reg, m_Low, m_High);
  }

  MachRegister RegisterAST::getID() const { return m_Reg; }

  std::string RegisterAST::format(Architecture arch, formatStyle) const {
    if(arch == Arch_amdgpu_gfx908 || arch == Arch_amdgpu_gfx90a || arch == Arch_amdgpu_gfx940) {
      return AmdgpuFormatter::formatRegister(m_Reg, m_num_elements, m_Low, m_High);
    }
    return ArchSpecificFormatter::getFormatter(arch).formatRegister(m_Reg.name());
  }

  std::string RegisterAST::format(formatStyle) const {
    std::string name = m_Reg.name();
    std::string::size_type substr = name.rfind("::");
    if(substr != std::string::npos) {
      name = name.substr(substr + 2, name.length());
    }
    /* we have moved to AT&T syntax (lowercase registers) */
    for(char& c : name)
      c = std::toupper(c);

    return name;
  }

  std::string MaskRegisterAST::format(Architecture, formatStyle f) const { return format(f); }

  std::string MaskRegisterAST::format(formatStyle) const {
    std::string name = m_Reg.name();
    std::string::size_type substr = name.rfind(':');
    if(substr != std::string::npos) {
      name = name.substr(substr + 1, name.length());
    }

    /* The syntax for a masking register is {kX} in AT&T syntax. */
    return "{" + name + "}";
  }

  RegisterAST RegisterAST::makePC(Dyninst::Architecture arch) {
    return RegisterAST(MachRegister::getPC(arch), 0, MachRegister::getPC(arch).size());
  }

  bool RegisterAST::operator<(const RegisterAST& rhs) const {
    if(m_Reg < rhs.m_Reg)
      return true;
    if(rhs.m_Reg < m_Reg)
      return false;
    if(m_Low < rhs.m_Low)
      return true;
    if(m_Low > rhs.m_Low)
      return false;
    return m_High < rhs.m_High;
  }

  bool RegisterAST::isStrictEqual(const Expression& rhs) const {
    if(rhs.checkRegID(m_Reg, m_Low, m_High)) {
      const RegisterAST& rhs_reg = dynamic_cast<const RegisterAST&>(rhs);
      if((m_Low == rhs_reg.m_Low) && (m_High == rhs_reg.m_High)) {
        return true;
      } else {
        return false;
      }
    }
    return false;
  }

  RegisterAST::Ptr RegisterAST::promote(const Expression::Ptr regPtr) {
    const RegisterAST::Ptr r = boost::dynamic_pointer_cast<RegisterAST>(regPtr);
    return RegisterAST::promote(r.get());
  }

  MachRegister RegisterAST::getPromotedReg() const { return m_Reg.getBaseRegister(); }

  RegisterAST::Ptr RegisterAST::promote(const RegisterAST* regPtr) {
    if(!regPtr)
      return RegisterAST::Ptr();

    // We want to upconvert the register ID to the maximal containing
    // register for the platform - either EAX or RAX as appropriate.

    return boost::make_shared<RegisterAST>(regPtr->getPromotedReg(), 0,
                                           regPtr->getPromotedReg().size());
  }

  bool RegisterAST::isFlag() const { return m_Reg.isFlag(); }

  bool RegisterAST::checkRegID(MachRegister r, unsigned int low, unsigned int high) const {
    return (r.getBaseRegister() == m_Reg.getBaseRegister()) && (low <= m_High) && (high >= m_Low);
  }

  void RegisterAST::apply(Visitor* v) { v->visit(this); }

  bool RegisterAST::bind(Expression* e, const Result& val) {
    if(Expression::bind(e, val)) {
      return true;
    }

    if(e->checkRegID(m_Reg, m_Low, m_High)) {
      setValue(val);
      return true;
    }
    return false;
  }
}}
