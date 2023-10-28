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

#include "Expression.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "Architecture.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    /// A %TernaryAST object represents the value of a ternary assignment
    /// As a %TernaryAST is a %Expression, it may contain the physical register's contents if
    /// they are known.
    ///


    class INSTRUCTION_EXPORT TernaryAST : public Expression
    {
    public:
      /// \brief A type definition for a reference-counted pointer to a %TernaryAST.
      typedef boost::shared_ptr<TernaryAST> Ptr;
      
      /// Construct a register, assigning it the ID \c id.
      TernaryAST(Expression::Ptr cond , Expression::Ptr first , Expression::Ptr second, Result_Type result_type);
  
      virtual ~TernaryAST();
      
      /// By definition, a %TernaryAST has three children, the condition, the first and the second child.
      /// and should all be added to the \param children.
      //
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const;
      virtual void getChildren(vector<Expression::Ptr>& children) const;

      /// By definition, add the use of all its children to.
      /// \param uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses);

      /// \c isUsed returns true if \c findMe is a %TernaryAST that represents
      /// the same register as this %TernaryAST, and false otherwise.
      virtual bool isUsed(InstructionAST::Ptr findMe) const;

      /// The \c format method on a %TernaryAST object returns the name associated with its ID.
      virtual std::string format(Architecture, formatStyle how = defaultStyle) const;
      /// The \c format method on a %TernaryAST object returns the name associated with its ID.
      virtual std::string format(formatStyle how = defaultStyle) const;

      static TernaryAST makePC(Dyninst::Architecture arch);

      /// We define a partial order by the order of its children
      bool operator<(const TernaryAST& rhs) const;

      
           
      virtual void apply(Visitor* v);
      virtual bool bind(Expression* e, const Result& val);
      Expression::Ptr cond;
      Expression::Ptr first;
      Expression::Ptr second;
      Result_Type result_type{};

    protected:
      virtual bool isStrictEqual(const InstructionAST& rhs) const;
      
      
    };

  }
}


  

#endif // !defined(TERNARY_H)
