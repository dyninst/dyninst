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

#if !defined(IMMEDIATE_H)
#define IMMEDIATE_H

#include "Expression.h"
#include <map>
#include <string>
#include <sstream>

namespace Dyninst
{
  namespace InstructionAPI
  {
    /// The %Immediate class represents an immediate value in an operand
    ///
    /// Since an %Immediate represents a constant value, the \c setValue
    /// and \c clearValue interface are disabled on %Immediate objects.
    /// If an immediate value is being modified, a new %Immediate object should
    /// be created to represent the new value.
    class INSTRUCTION_EXPORT Immediate : public Expression
    {
    public:
      Immediate(const Result& val);
      
      virtual ~Immediate();

      /// By definition, an %Immediate has no children.
      virtual void getChildren(vector<InstructionAST::Ptr>& /*children*/) const;
      virtual void getChildren(vector<Expression::Ptr>& /*children*/) const;

      /// By definition, an %Immediate uses no registers.
      virtual void getUses(set<InstructionAST::Ptr>& /*uses*/);

      /// \c isUsed, when called on an %Immediate, will return true if \c findMe represents an %Immediate with the same value.
      /// While this convention may seem arbitrary, it allows \c isUsed to follow a natural rule: an %InstructionAST is used 
      /// by another %InstructionAST if and only if the first %InstructionAST is a subtree of the second one.
      virtual bool isUsed(InstructionAST::Ptr findMe) const;

      virtual std::string format(Architecture, formatStyle) const;
      virtual std::string format(formatStyle) const;
      static Immediate::Ptr makeImmediate(const Result& val);
      virtual void apply(Visitor* v);
      
    protected:
      virtual bool isStrictEqual(const InstructionAST& rhs) const;
    };

    class INSTRUCTION_EXPORT NamedImmediate : public Immediate
    {
    public:
        NamedImmediate(std::string name, const Result &val);

        static NamedImmediate::Ptr makeNamedImmediate(std::string name, const Result &val);
        virtual std::string format(Architecture, formatStyle) const;
        virtual std::string format(formatStyle) const;

    private:
        std::string name_;
    };


    class INSTRUCTION_EXPORT ArmConditionImmediate : public Immediate
    {
    public:
        ArmConditionImmediate(const Result &val);

        static ArmConditionImmediate::Ptr makeArmConditionImmediate(const Result &val);
        virtual std::string format(Architecture, formatStyle) const;
        virtual std::string format(formatStyle) const;

    private:
        std::map<unsigned int, std::string> m_condLookupMap;
    };

    class INSTRUCTION_EXPORT ArmPrfmTypeImmediate : public Immediate
    {
    public:
	ArmPrfmTypeImmediate(const Result &val);

	static Immediate::Ptr makeArmPrfmTypeImmediate(const Result &val);
	virtual std::string format(Architecture, formatStyle) const;
    virtual std::string format(formatStyle) const;

    private:
	std::map<unsigned int, std::string> m_prfmTypeLookupMap;
    };
  }
}




  

#endif // !defined(IMMEDIATE_H)
