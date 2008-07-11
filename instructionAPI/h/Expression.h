
#if !defined(VALUECOMPUTATION_H)
#define VALUECOMPUTATION_H


#include "InstructionAST.h"
#include "Result.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <sstream>

namespace Dyninst
{
  namespace InstructionAPI
  {
 
    class Expression;

    /// An %Expression is an AST representation of how the value of an operand is computed.
    ///
    /// The %Expression class extends the %InstructionAST class by adding the concept of evaluation
    /// to the nodes of an %InstructionAST.  Evaluation attempts to determine the Result of the computation that the
    /// AST being evaluated represents.  It will fill in results of as many of the nodes in the tree as possible,
    /// and if full evaluation is possible, it will return the result of the computation performed by the tree.
    ///
    /// Permissible leaf nodes of a %Expression tree are %RegisterAST and %Immediate objects.  Permissible internal
    /// nodes are %BinaryFunction and %Dereference objects.  An %Expression may represent an
    /// immediate value, the contents of a register, or the contents of memory at a given address, interpreted as a particular type.
    /// 
    /// The %Results in an %Expression tree contain a type and a value.
    /// Their values may be an undefined value or an instance of their associated type.  When two %Results are combined using a %BinaryFunction,
    /// the %BinaryFunction specifies the output type.  Sign extension, type promotion, truncation, and all other necessary conversions are
    /// handled automatically based on the input types and the output type.
    /// If both of the %Results that are combined
    /// have defined values, the combination will also have a defined value; otherwise, the combination's value will be undefined.
    /// For more information, see Result, BinaryFunction, and Dereference.
    ///
    /// A user may specify the result of evaluating a given %Expression.  This mechanism is designed to allow the user to provide
    /// a %Dereference or %RegisterAST with information about the state of memory or registers.
    /// It may additionally be used to change the value of an %Immediate or to specify the result of a %BinaryFunction.
    /// This mechanism may be used to support other advanced analyses.
    ///
    /// The evaluation mechanism, as mentioned above, will evaluate as many sub-expressions of an expression as possible.
    /// Any operand that is more complicated than a single immediate value, however, will depend on register or memory
    /// values.
    /// The %Results of evaluating each subexpression are cached automatically using the \c setValue mechanism.
    /// The %Expression then attempts to determine its %Result based on the %Results of its children.
    /// If this %Result can be determined (most likely because register contents have been filled in via \c setValue), it will be returned from \c eval;
    /// if it can not be determined, a %Result with an undefined value will be returned.
    /// See Figure 6 for an illustration of this concept; the operand represented is
    /// [ \c EBX + \c 4 * \c EAX ].  The contents of \c EBX and \c EAX have been determined through some outside mechanism,
    /// and have been defined with \c setValue.  The \c eval mechanism proceeds to determine the address being read by the %Dereference,
    /// since this information can be determined given the contents of the registers.
    /// This address is available from the %Dereference through its child in the tree, even though calling \c eval 
    /// on the %Dereference returns a %Result with an undefined value.
    /// \dotfile deref-eval.dot "Applying \c eval to a Dereference tree with the state of the registers known and the state of memory unknown"
    ///
    class Expression : public InstructionAST
    {
    public:
      /// \brief A type definition for a reference counted pointer to a %Expression.
      typedef boost::shared_ptr<Expression> Ptr;

    protected:      
      Expression(Result_Type t);
    public:
      virtual ~Expression();

      /// \brief If the %Expression can be evaluated, returns a %Result containing its value.
      /// Otherwise returns an undefined %Result.
      virtual Result eval() const;
  
      /// \param knownValue Sets the result of \c eval for this %Expression
      /// to \c knownValue
      void setValue(Result knownValue);
  
      /// \c clearValue sets the contents of this %Expression to undefined.
      /// The next time \c eval is called, it will recalculate the value of the %Expression.
      void clearValue();

      int size() const;
  
    private:
      Result userSetValue;
    };

  };
};



#endif //!defined(VALUECOMPUTATION_H)

