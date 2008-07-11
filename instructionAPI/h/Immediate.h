

#if !defined(IMMEDIATE_H)
#define IMMEDIATE_H

#include "Expression.h"
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
    class Immediate : public Expression
    {
    public:
      Immediate(Result val) : Expression(val.type)
      {
	setValue(val);
      }
      
      virtual ~Immediate() 
      {
      }
      /// By definition, an %Immediate has no children.
      virtual void getChildren(vector<InstructionAST::Ptr>& /*children*/) const
      {
	return;
      }
      /// By definition, an %Immediate uses no registers.
      virtual void getUses(set<InstructionAST::Ptr>& /*uses*/) const
      {
	return;
      }

      /// \c isUsed, when called on an %Immediate, will return true if \c findMe represents an %Immediate with the same value.
      /// While this convention may seem arbitrary, it allows \c isUsed to follow a natural rule: an %InstructionAST is used 
      /// by another %InstructionAST if and only if the first %InstructionAST is a subtree of the second one.
      virtual bool isUsed(InstructionAST::Ptr findMe) const
      {
	return *findMe == *this;
      }
      virtual std::string format() const
      {
	return eval().format();
      }
    protected:
      virtual bool isSameType(const InstructionAST& rhs) const
      {
	return dynamic_cast<const Immediate*>(&rhs) != NULL;
      }
      virtual bool isStrictEqual(const InstructionAST& rhs) const
      {
	const Immediate& other(dynamic_cast<const Immediate&>(rhs));
	return other.eval() == Expression::eval();
      }  
    };
  };
};




  

#endif // !defined(IMMEDIATE_H)
