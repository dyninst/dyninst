
#if !defined(INSTRUCTIONAST_H)
#define INSTRUCTIONAST_H

#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include <iostream>

namespace Dyninst
{
  namespace Instruction 
  {
  
    class InstructionAST;
  
    using std::vector;
    using std::set;
    
    /// The %InstructionAST class is the base class for all nodes in the ASTs used by the %Operand class.
    /// It defines the necessary interfaces for traversing and searching
    /// an abstract syntax tree representing an operand.
    /// For the purposes of searching an %InstructionAST, we provide two related interfaces.  The first,
    /// \c getUses, will return the registers that appear in a given tree.  The second, \c isUsed, will
    /// take as input another tree and return true if that tree is a (not necessarily proper) subtree of this one.
    /// \c isUsed requires us to define an equality relation on these abstract
    /// syntax trees, and the equality operator is provided by the %InstructionAST, with the details
    /// implemented by the classes derived from %InstructionAST.  Two AST nodes are equal if the following conditions hold:
    /// - They are of the same type
    /// - If leaf nodes, they represent the same immediate value or the same register
    /// - If non-leaf nodes, they represent the same operation and their corresponding children are equal
    class InstructionAST
    {
    public:
      typedef boost::shared_ptr<InstructionAST> Ptr;

      InstructionAST();
      virtual ~InstructionAST();

      /// Compare two AST nodes for equality.  
      ///
      /// Non-leaf nodes are equal
      /// if they are of the same type and their children are equal.  %RegisterASTs
      /// are equal if they represent the same register.  %Immediates are equal if they
      /// represent the same value.
      bool operator==(const InstructionAST& rhs) const;

      /// Children of this node are appended to the vector \c children
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const = 0;

      /// \param[out] uses The use set of this node is appended to the vector \c uses
      ///
      /// The use set of an %InstructionAST is defined as follows:
      ///   - A %RegisterAST uses itself
      ///   - A %BinaryFunction uses the use sets of its children
      ///   - An %Immediate uses nothing
      ///   - A %Dereference uses the use set of its child
      virtual void getUses(set<InstructionAST::Ptr>& uses) const = 0;

      /// \return Returns true if \c findMe is used by this AST node.
      /// \param[in] findMe AST node to find in the use set of this node
      ///
      /// Unlike \c getUses, \c isUsed looks for \c findMe as a subtree
      /// of the current tree.  \c getUses is designed to return a minimal
      /// set of registers used in this tree, whereas \c isUsed is designed
      /// to allow searches for arbitrary subexpressions
      virtual bool isUsed(InstructionAST::Ptr findMe) const = 0;

      /// The \c format interface returns the contents of an %InstructionAST
      /// object as a string.  By default, \c format() produces assembly language.
      virtual std::string format() const = 0;
  
    protected:
      virtual bool isSameType(const InstructionAST& rhs) const = 0;
      virtual bool isStrictEqual(const InstructionAST& rhs) const= 0;
    };
  };
};


#endif //!defined(INSTRUCTIONAST_H)
