
#if !defined(BINARYFUNCTION_H)
#define BINARYFUNCTION_H

#include "Expression.h"
#include "Register.h"
#include "Result.h"
#include <sstream>

namespace Dyninst
{
  namespace Instruction
  {
    using std::vector;

    /// A %BinaryFunction object represents a function that can combine two %Expressions and produce another %ValueComputation.
    ///
    /// For the purposes of representing a single operand of an instruction, the %BinaryFunctions of interest are addition and multiplication of
    /// integer values; this allows a %Expression to represent all addressing modes on the architectures currently
    /// supported by the %Instruction API.
    class BinaryFunction : public Expression
    {
      class funcT
      {
      public:
	funcT(std::string name) : m_name(name) 
	{
	}
	virtual ~funcT()
	{
	}
	
	virtual Result operator()(const Result& arg1, const Result& arg2) const = 0;
	std::string format() const 
	{
	  return m_name;
	}
	
      private:
	std::string m_name;
      };
      
      
      class addResult : public funcT
      {
      public:
	addResult() : funcT("+") 
	{
	}
	virtual ~addResult()
	{
	}
	Result operator()(const Result& arg1, const Result& arg2) const
	{
	  return arg1 + arg2;
	}
      };
      class multResult : public funcT
      {
      public:
	multResult() : funcT("*")
	{
	}
	virtual ~multResult()
	{
	}
	Result operator()(const Result& arg1, const Result& arg2) const
	{
	  // return arg1 * arg2;
	  return arg1 + arg2;
	}
      };      
      
      
    public:
      /// \param arg1 first input to function
      /// \param arg2 second input to function
      /// \param result_type type of the function's result
      /// \param func implementation of the function
      ///
      /// The constructor for a %BinaryFunction may take a reference-counted pointer or a plain C++ pointer to each of the
      /// child Expressions that represent its arguments.  Since the reference-counted implementation requires explicit construction,
      /// we provide overloads for all four combinations of plain and reference-counted pointers.  Note that regardless of which constructor
      /// is used, the pointers \c arg1 and \c arg2 become owned by the %BinaryFunction being constructed, and should not be deleted.
      /// They will be cleaned up when the %BinaryFunction object is destroyed.
      ///
      /// The \c func parameter is a binary functor on two Results.  It should be derived from \c %funcT.  \c addResult and
      /// \c multResult, which respectively add and multiply two %Results, are provided as part of the InstructionAPI, 
      /// as they are necessary for representing address calculations.  Other \c %funcTs may be implemented by the user if desired.
      /// %funcTs have names associated with them for output and debugging purposes.  The addition and multiplication functors
      /// provided with the %Instruction API are named "+" and "*", respectively.
      template <typename T1, typename T2>
      BinaryFunction(T1 arg1, T2 arg2, Result_Type result_type,
		     boost::shared_ptr<funcT> func) : 
      Expression(result_type), m_arg1(arg1), m_arg2(arg2), m_funcPtr(func)
      {
      }
              
      virtual ~BinaryFunction() 
      {
      }

      /// The %BinaryFunction version of \c eval allows the \c eval mechanism to handle complex addressing modes.  Like all of the %ValueComputation
      /// implementations, a %BinaryFunction's \c eval will return the result of evaluating the expression it represents
      /// if possible, or an empty %Result otherwise.
      /// A %BinaryFunction may have arguments that can be evaluated, or arguments that cannot.
      /// Additionally, it may have a real function pointer, or it may have a null function pointer.
      /// If the arguments can be evaluated and the function pointer is real, a result other than an empty %Result is guaranteed to be
      /// returned.  This result is cached after its initial calculation; the caching mechanism also allows
      /// outside information to override the results of the %BinaryFunction's internal computation.
      /// If the cached result exists, it is guaranteed to be returned even if the arguments or the function
      /// are not evaluable.
      virtual Result eval() const
      {
	Result x = m_arg1->eval();
	Result y = m_arg2->eval();
	Result oracularResult = Expression::eval();
	if(x.defined && y.defined && !oracularResult.defined)
	{
	  Result result = (*m_funcPtr)(x, y);
	  const_cast<BinaryFunction*>(this)->setValue(result);
	}
	return Expression::eval();
      }
    
      /// The children of a %BinaryFunction are its two arguments.
      /// \param children Appends the children of this %BinaryFunction to \c children.
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const
      {
	children.push_back(m_arg1);
	children.push_back(m_arg2);
	return;
      }
      /// The use set of a %BinaryFunction is the union of the use sets of its children.
      /// \param uses Appends the use set of this %BinaryFunction to \c uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses) const
      {
	if(boost::dynamic_pointer_cast<RegisterAST>(m_arg1))
	{
	  uses.insert(m_arg1);
	}
	else
	{
	  m_arg1->getUses(uses);
	}
	if(boost::dynamic_pointer_cast<RegisterAST>(m_arg2))
	{
	  uses.insert(m_arg2);
	}
	else
	{
	  m_arg2->getUses(uses);
	}
	return;
      }
      
      virtual bool isUsed(InstructionAST::Ptr findMe) const
      {
	return m_arg1->isUsed(findMe) || m_arg2->isUsed(findMe) 
	|| *m_arg1 == *findMe || *m_arg2 == *findMe;
      }
      virtual std::string format() const
      {
	std::stringstream retVal;
	retVal << m_arg1->format() << " " << m_funcPtr->format() << " " << m_arg2->format();
	return retVal.str();
      }
    protected:
      virtual bool isSameType(const InstructionAST& rhs) const
      {
	return dynamic_cast<const BinaryFunction*>(&rhs) != NULL;
      }
      virtual bool isStrictEqual(const InstructionAST& rhs) const
      {
	const BinaryFunction& other(dynamic_cast<const BinaryFunction&>(rhs));
	return *(other.m_arg1) == *m_arg1 &&
	(*other.m_arg2) == *m_arg2;// &&
	//other.m_funcPtr == m_funcPtr;
      }
  
    private:
      Expression::Ptr m_arg1;
      Expression::Ptr m_arg2;
      boost::shared_ptr<funcT> m_funcPtr;
      
    };
  };
};


#endif // !defined(BINARYFUNCTION_H)
