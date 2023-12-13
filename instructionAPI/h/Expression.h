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

#if !defined(VALUECOMPUTATION_H)
#define VALUECOMPUTATION_H


#include "InstructionAST.h"
#include "Result.h"

#include <string>
#include <vector>
#include <sstream>

namespace Dyninst
{
  namespace InstructionAPI
  {
 
    class Expression;
    class Visitor;
    /// An %Expression is an AST representation of how the value of an
    /// operand is computed.
    ///
    /// The %Expression class extends the %InstructionAST class by
    /// adding the concept of evaluation to the nodes of an
    /// %InstructionAST.  Evaluation attempts to determine the Result
    /// of the computation that the AST being evaluated represents.
    /// It will fill in results of as many of the nodes in the tree as
    /// possible, and if full evaluation is possible, it will return
    /// the result of the computation performed by the tree.
    ///
    /// Permissible leaf nodes of a %Expression tree are %RegisterAST
    /// and %Immediate objects.  Permissible internal nodes are
    /// %BinaryFunction and %Dereference objects.  An %Expression may
    /// represent an immediate value, the contents of a register, or
    /// the contents of memory at a given address, interpreted as a
    /// particular type.
    /// 
    /// The %Results in an %Expression tree contain a type and a
    /// value.  Their values may be an undefined value or an instance
    /// of their associated type.  When two %Results are combined
    /// using a %BinaryFunction, the %BinaryFunction specifies the
    /// output type.  Sign extension, type promotion, truncation, and
    /// all other necessary conversions are handled automatically
    /// based on the input types and the output type.  If both of the
    /// %Results that are combined have defined values, the
    /// combination will also have a defined value; otherwise, the
    /// combination's value will be undefined.  For more information,
    /// see Result, BinaryFunction, and Dereference.
    ///
    /// A user may specify the result of evaluating a given
    /// %Expression.  This mechanism is designed to allow the user to
    /// provide a %Dereference or %RegisterAST with information about
    /// the state of memory or registers.  It may additionally be used
    /// to change the value of an %Immediate or to specify the result
    /// of a %BinaryFunction.  This mechanism may be used to support
    /// other advanced analyses.
    ///
    /// In order to make it more convenient to specify the results
    /// of particular subexpressions, the \c bind method is provided.
    /// \c bind allows the user to specify that a given subexpression
    /// has a particular value everywhere that it appears in an expression.
    /// For example, if the state of certain registers is known at the
    /// time an instruction is executed, a user can \c bind those registers
    /// to their known values throughout an %Expression.
    ///
    /// The evaluation mechanism, as mentioned above, will evaluate as
    /// many sub-expressions of an expression as possible.  Any
    /// operand that is more complicated than a single immediate
    /// value, however, will depend on register or memory values.  The
    /// %Results of evaluating each subexpression are cached
    /// automatically using the \c setValue mechanism.  The
    /// %Expression then attempts to determine its %Result based on
    /// the %Results of its children.  If this %Result can be
    /// determined (most likely because register contents have been
    /// filled in via \c setValue or \c bind), it will be returned from \c eval;
    /// if it can not be determined, a %Result with an undefined value
    /// will be returned.  See Figure 6 for an illustration of this
    /// concept; the operand represented is [ \c EBX + \c 4 * \c EAX
    /// ].  The contents of \c EBX and \c EAX have been determined
    /// through some outside mechanism, and have been defined with \c
    /// setValue.  The \c eval mechanism proceeds to determine the
    /// address being read by the %Dereference, since this information
    /// can be determined given the contents of the registers.  This
    /// address is available from the %Dereference through its child
    /// in the tree, even though calling \c eval on the %Dereference
    /// returns a %Result with an undefined value.  \dotfile
    /// deref-eval.dot "Applying \c eval to a Dereference tree with
    /// the state of the registers known and the state of memory
    /// unknown"
    ///
    class INSTRUCTION_EXPORT Expression : public InstructionAST
    {
    public:
      /// \brief A type definition for a reference counted pointer to a %Expression.
      typedef boost::shared_ptr<Expression> Ptr;
    protected:      
      Expression(Result_Type t);
      Expression(MachRegister r);
    public:
      virtual ~Expression();
      Expression(const Expression&) = default;

      /// \brief If the %Expression can be evaluated, returns a %Result containing its value.
      /// Otherwise returns an undefined %Result.
      virtual const Result& eval() const;
  
      /// \param knownValue Sets the result of \c eval for this %Expression
      /// to \c knownValue
      void setValue(const Result& knownValue);
  
      /// \c clearValue sets the contents of this %Expression to undefined.
      /// The next time \c eval is called, it will recalculate the value of the %Expression.
      void clearValue();

      /// \c size returns the size of this %Expression's %Result, in bytes.
      int size() const;
	  
      /// \c bind searches for all instances of the %Expression \c expr within
      /// this %Expression, and sets the result of \c eval for those subexpressions
      /// to \c value.  \c bind returns true if at least one instance of \c expr
      /// was found in this %Expression.
      ///
      /// \c bind does not operate on subexpressions that happen to evaluate to
      /// the same value.  For example, if a dereference of 0xDEADBEEF is bound to
      /// 0, and a register is bound to 0xDEADBEEF, a dereference of that register is not
      /// bound to 0.
      virtual bool bind(Expression* expr, const Result& value);


      /// \c apply applies a %Visitor to this expression.  %Visitors perform postfix-order
      /// traversal of the ASTs represented by an %Expression, with user-defined actions performed
      /// at each node of the tree.
      virtual void apply(Visitor*) {}

      /// \c getChildren may be called on an %Expression taking a vector of %Expression::Ptrs,
      /// rather than %InstructionAST::Ptrs.  All children which are %Expressions will be appended to \c children.
      virtual void getChildren(std::vector<Expression::Ptr>& children) const = 0;
      using InstructionAST::getChildren;
      
      
    protected:
      virtual bool isFlag() const;
      Result userSetValue;
      
    };
    class INSTRUCTION_EXPORT DummyExpr : public Expression
    {
        public:
            virtual void getChildren(vector<InstructionAST::Ptr>& ) const {}
            virtual void getChildren(vector<Expression::Ptr>& ) const {}
            virtual void getUses(set<InstructionAST::Ptr>& ) {}
            virtual bool isUsed(InstructionAST::Ptr ) const { return true;}
            virtual std::string format(Architecture, formatStyle) const { return "[WILDCARD]";}
            virtual std::string format(formatStyle) const { return "[WILDCARD]";}
            DummyExpr() : Expression(u8) {}
        protected:
            virtual bool checkRegID(MachRegister, unsigned int = 0, unsigned int = 0) const;
            virtual bool isStrictEqual(const InstructionAST& rhs) const;
    };

  }
}



#endif //!defined(VALUECOMPUTATION_H)

