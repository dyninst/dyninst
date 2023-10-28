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

#include "Expression.h"
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
    /// A %RegisterAST object represents a register contained in an operand.
    /// As a %RegisterAST is a %Expression, it may contain the physical register's contents if
    /// they are known.
    ///


    class INSTRUCTION_EXPORT RegisterAST : public Expression
    {
    public:
      /// \brief A type definition for a reference-counted pointer to a %RegisterAST.
      typedef boost::shared_ptr<RegisterAST> Ptr;
      
      /// Construct a register, assigning it the ID \c id.
      RegisterAST(MachRegister r, uint32_t num_elements = 1 );
      RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, uint32_t num_elements = 1);
      RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, Result_Type regType, uint32_t num_elements = 1);
  
      virtual ~RegisterAST();
      RegisterAST(const RegisterAST&) = default;
      
      /// By definition, a %RegisterAST object has no children.
      /// \param children Since a %RegisterAST has no children, the \c children parameter is unchanged by this method.
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const;
      virtual void getChildren(vector<Expression::Ptr>& children) const;

      /// By definition, the use set of a %RegisterAST object is itself.
      /// \param uses This %RegisterAST will be inserted into \c uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses);

      /// \c isUsed returns true if \c findMe is a %RegisterAST that represents
      /// the same register as this %RegisterAST, and false otherwise.
      virtual bool isUsed(InstructionAST::Ptr findMe) const;

      /// The \c format method on a %RegisterAST object returns the name associated with its ID.
      virtual std::string format(Architecture, formatStyle how = defaultStyle) const;
      /// The \c format method on a %RegisterAST object returns the name associated with its ID.
      virtual std::string format(formatStyle how = defaultStyle) const;

      /// Utility function to get a Register object that represents the program counter.
      ///
      /// \c makePC is provided to support platform-independent control flow analysis.
      static RegisterAST makePC(Dyninst::Architecture arch);

      /// We define a partial ordering on registers by their register number so that they may be placed into sets
      /// or other sorted containers.
      bool operator<(const RegisterAST& rhs) const;

      /// The \c getID function returns the ID number of a register.
      MachRegister getID() const;
      unsigned int lowBit() const {
          return m_Low; }
    unsigned int highBit() const {
        return m_High; }

      /// Utility function to hide aliasing complexity on platforms (IA-32) that allow addressing part 
      /// or all of a register
      // Note: not const because it may return *this...
      static RegisterAST::Ptr promote(const InstructionAST::Ptr reg);
      static RegisterAST::Ptr promote(const RegisterAST* reg);
      
      virtual void apply(Visitor* v);
      virtual bool bind(Expression* e, const Result& val);

    protected:
      virtual bool isStrictEqual(const InstructionAST& rhs) const;
      virtual bool isFlag() const;
      virtual bool checkRegID(MachRegister id, unsigned int low, unsigned int high) const;
      MachRegister getPromotedReg() const;
      
      MachRegister m_Reg;
      unsigned int m_Low;
      unsigned int m_High;
      unsigned int m_num_elements;
    };

    /**
     * Class for mask register operands. This class is the same as the RegisterAST
     * class except it handles the syntactial differences between register operands
     * and mask register operands.
     */
    class INSTRUCTION_EXPORT MaskRegisterAST : public RegisterAST
    {
        public:
            MaskRegisterAST(MachRegister r) : RegisterAST(r) {}
            MaskRegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit)
                : RegisterAST(r, lowbit, highbit) {}
            MaskRegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, Result_Type regType)
                : RegisterAST(r, lowbit, highbit, regType) {}
           virtual std::string format(Architecture, formatStyle how = defaultStyle) const;

            virtual std::string format(formatStyle how = defaultStyle) const;
    };
  }
}


  

#endif // !defined(REGISTER_H)
