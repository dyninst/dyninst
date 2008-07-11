
#if !defined(DEREFERENCE_H)
#define DEREFERENCE_H

#include "Expression.h"
#include "Register.h"
#include <sstream>

namespace Dyninst
{
  namespace InstructionAPI
  {
    using std::vector;

    /// A %Dereference object is a %Expression that dereferences another %ValueComputation.
    ///
    /// A %Dereference contains an %Expression representing an effective address computation.
    /// Its use set is the same as the use set of the
    /// %Expression being dereferenced.
    ///
    /// It is not possible, given the information in a single instruction, to evaluate
    /// the result of a dereference.  \c eval may still be called on a %Expression
    /// that includes dereferences, but the expected use case is as follows:
    ///   - Determine the address being used in a dereference via the \c eval mechanism
    ///   - Perform analysis to determine the contents of that address
    ///   - If necessary, fill in the %Dereference node with the contents of that addresss, using \c setValue
    ///
    /// The type associated with a %Dereference node will be the type of the value \em read \em from \em memory,
    /// not the type used for the address computation.  Two %Dereferences that access the same address but interpret
    /// the contents of that memory as different types will produce different values.  The children of a %Dereference at
    /// a given address are identical, regardless of the type of dereference being performed at that address.
    /// For example, the %Expression shown in Figure 6 could have its root %Dereference, which interprets the memory being dereferenced
    /// as a unsigned 16-bit integer, replaced with a %Dereference that
    /// interprets the memory being dereferenced as any other type.  The remainder of the %Expression tree would, however, remain unchanged.
    class Dereference : public Expression
    {

    public:
      /// A %Dereference is constructed from a %Expression pointer (raw or shared) representing the address
      /// to be dereferenced and a type
      /// indicating how the memory at the address in question is to be interpreted.
      Dereference(Expression::Ptr addr, Result_Type result_type) : Expression(result_type), addressToDereference(addr)
      {
      }
      virtual ~Dereference() 
      {
      }
      /// A %Dereference has one child, which represents the address being dereferenced.
      /// \param[out] children Appends the child of this %Dereference to \c children.
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const
      {
	children.push_back(addressToDereference);
	return;
      }
      /// The use set of a %Dereference is the same as the use set of its children.
      /// \paramp[out] uses The use set of this %Dereference is inserted into \c uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses) const
      {
	if(boost::dynamic_pointer_cast<RegisterAST>(addressToDereference))
	{
	  uses.insert(addressToDereference);
	}
	else
	{
	  addressToDereference->getUses(uses);
	}
	
	return;
      }
      /// An %InstructionAST is used by a %Dereference if it is equivalent to the 
      /// %Dereference or it is used by the lone child of the %Dereference
      virtual bool isUsed(InstructionAST::Ptr findMe) const
      {
	return addressToDereference->isUsed(findMe) || *findMe == *this;
      }
      virtual std::string format() const
      {
	std::stringstream retVal;
	retVal << "[" << addressToDereference->format() << "]";
	return retVal.str();
      }
    protected:
      virtual bool isSameType(const InstructionAST& rhs) const
      {
	return dynamic_cast<const Dereference*>(&rhs) != NULL;
      }
      virtual bool isStrictEqual(const InstructionAST& rhs) const
      {
	const Dereference& other(dynamic_cast<const Dereference&>(rhs));
	return *(other.addressToDereference) == *addressToDereference;
      }
  
  
    private:
      Expression::Ptr addressToDereference;
  
    };
  };
};



#endif // !defined(DEREFERENCE_H)
