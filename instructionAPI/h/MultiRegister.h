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

#include "Expression.h"
#include "Register.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "registers/MachRegister.h"
#include "Architecture.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    /// A %MultiRegisterAST object represents a ordered collection of registers as a single operand
    /// As a %MultiRegisterAST is a %Expression, it may contain the physical register's contents if
    /// they are known.
    ///


    class DYNINST_EXPORT MultiRegisterAST : public Expression
    {
    public:
      /// \brief A type definition for a reference-counted pointer to a %MultiRegisterAST.
      typedef boost::shared_ptr<MultiRegisterAST> Ptr;
      
      /// Construct a register, assigning it the ID \c id.
      MultiRegisterAST(MachRegister r, uint32_t num_elements = 1 );
      MultiRegisterAST(std::vector<RegisterAST::Ptr> _in);
  
      virtual ~MultiRegisterAST() = default;
      MultiRegisterAST(const MultiRegisterAST&) = default;
      
      /// By definition, a %MultiRegisterAST object has no children.
      /// \param children Since a %MultiRegisterAST has no children, the \c children parameter is unchanged by this method.
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const;
      virtual void getChildren(vector<Expression::Ptr>& children) const;

      /// By definition, the use set of a %MultiRegisterAST object is itself.
      /// \param uses This %MultiRegisterAST will be inserted into \c uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses);

      /// \c isUsed returns true if \c findMe is a %MultiRegisterAST that represents
      /// the same register as this %MultiRegisterAST, and false otherwise.
      virtual bool isUsed(InstructionAST::Ptr findMe) const;

      /// The \c format method on a %MultiRegisterAST object returns the name associated with its ID.
      virtual std::string format(Architecture, formatStyle how = defaultStyle) const;
      /// The \c format method on a %MultiRegisterAST object returns the name associated with its ID.
      virtual std::string format(formatStyle how = defaultStyle) const;

      /// We define a partial ordering on registers by their register number so that they may be placed into sets
      /// or other sorted containers.
      //  For multiRegisters, the partial order is determined by baseReg ID, followed by length
      bool operator<(const MultiRegisterAST& rhs) const;

      virtual void apply(Visitor* v);
      virtual bool bind(Expression* e, const Result& val);
      
      RegisterAST::Ptr getBaseRegAST() const { return m_Regs[0]; } 
      uint32_t length() const { return m_Regs.size(); }
      const std::vector<RegisterAST::Ptr> &getRegs() const { return m_Regs; }
      bool areConsecutive() const { return consecutive; }
    protected:

      virtual bool checkRegID(MachRegister id, unsigned int low, unsigned int high) const;
      virtual bool isStrictEqual(const InstructionAST& rhs) const;
      virtual bool isFlag() const;
      
      std::vector<RegisterAST::Ptr> m_Regs;
    private:
      bool consecutive{false};
    };
  }
}


  

#endif // !defined(MULTIREGISTER_H)
