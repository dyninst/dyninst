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

#include "MultiRegister.h"
#include <vector>
#include <set>
#include <sstream>
#include "Visitor.h"
#include "InstructionDecoder-power.h"
#include "registers/MachRegister.h"
#include "Architecture.h"
#include "ArchSpecificFormatters.h"
#include "../../common/h/compiler_diagnostics.h"
#include "registers/x86_regs.h"
#include <boost/make_shared.hpp>

using namespace std;

namespace Dyninst
{
  namespace InstructionAPI
  {
    MultiRegisterAST::MultiRegisterAST(MachRegister r, uint32_t num_elements) :
            Expression(r,num_elements), consecutive(true)
    {
        uint32_t regVal = r.val();
        for (uint32_t i = 0; i < num_elements; i++){
            m_Regs.push_back(boost::make_shared<RegisterAST>(MachRegister(regVal+i), 0, r.size() * 8,num_elements));
        }
    }

    MultiRegisterAST::MultiRegisterAST(std::vector<RegisterAST::Ptr> inputRegASTs) : Expression(inputRegASTs[0]->getID(),inputRegASTs.size()), m_Regs{std::move(inputRegASTs)}
    {
    }

    void MultiRegisterAST::getChildren(vector<InstructionAST::Ptr>& /*children*/) const
    {
        return;
    }
    void MultiRegisterAST::getChildren(vector<Expression::Ptr>& /*children*/) const
    {
        return;
    }
    void MultiRegisterAST::getUses(set<InstructionAST::Ptr>& uses)
    {
        for (const auto & m_Reg : m_Regs) {
            m_Reg->getUses(uses);
        }
    }
    bool MultiRegisterAST::isUsed(InstructionAST::Ptr findMe) const
    {
        return isStrictEqual(*findMe);
    }

    std::string MultiRegisterAST::format(Architecture arch, formatStyle) const
    {
        if(arch == Arch_amdgpu_gfx908 || arch == Arch_amdgpu_gfx90a || arch == Arch_amdgpu_gfx940){
            return AmdgpuFormatter::formatMultiRegister(m_Regs[0]->getID(),m_Regs.size());
        }
        assert(0 && " multi register currently defined for amdgpu formats only ");
        return std::string("");
    }

    std::string MultiRegisterAST::format(formatStyle) const
    {
        bool isFirstEntry{true};
        std::string ret("[");
        for (auto & m_Reg : m_Regs) {
            if (isFirstEntry)  {
                isFirstEntry = false;
            }  else  {
                ret += ",";
            }
            ret += m_Reg->format();            
        }
        ret += "]";
        return ret;
    }

    bool MultiRegisterAST::operator<(const MultiRegisterAST& rhs) const
    {
        return m_Regs < rhs.m_Regs;
    }
    bool MultiRegisterAST::isStrictEqual(const InstructionAST& rhs) const
    {
        try {
            const MultiRegisterAST& rhs_reg =  dynamic_cast<const MultiRegisterAST&>(rhs);
            return m_Regs == rhs_reg.m_Regs;
        }
        catch(bad_cast &b){
            return false;
        }
    }
    bool MultiRegisterAST::isFlag() const
    {
        return false; // TODO
    }
    bool MultiRegisterAST::checkRegID(MachRegister r, unsigned int low, unsigned int high) const
    {
        for (uint32_t i =0; i< m_Regs.size(); i++){
            if(m_Regs[i]->checkRegID(r,low,high))
                return true;
        }
        return false;
    }
    void MultiRegisterAST::apply(Visitor* v)
    {
        v->visit(this);
    }
    static Result sizeToMask(uint32_t size)
    {
        switch(size)
        {
            case 1:
                return Result(u8,0xff);
            case 2:
                return Result(u16,0xffff);
            case 4:
                return Result(u32,0xffffffff);
            case 6:
                return Result(u48,0xffffffffffff);
            case 8:
                return Result(u64,0xffffffffffffffff);
            default:
                assert(!"sizeToMask unexpected machine register size!");
                return Result(u8);
        }
    }

    bool MultiRegisterAST::bind(Expression* e, const Result& val)
    {
        Result copiedVal = val;
        bool ret = false;
        for (auto & m_Reg : m_Regs) {
            Result mask = sizeToMask(m_Reg->size());
            Result extractedVal = copiedVal & mask;
            ret |= (m_Reg->bind(e,extractedVal));
            copiedVal = copiedVal >> Result(u32,m_Reg->size()*8);
        }
        return ret;
    }
  }
}
