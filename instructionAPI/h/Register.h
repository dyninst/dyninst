
#if !defined(REGISTER_H)
#define REGISTER_H

#include "Expression.h"
#include <vector>
#include <map>
#include <boost/assign/list_of.hpp>
#include <sstream>

#include "RegisterIDs-x86.h"


namespace Dyninst
{
  namespace InstructionAPI
  {
    /// A %RegisterAST object represents a register contained in an operand.
    /// As a %RegisterAST is a %Expression, it may contain the physical register's contents if
    /// they are known.
    ///
    class RegisterAST : public Expression
    {
    public:
      /// \brief A type definition for a reference-counted pointer to a %RegisterAST.
      typedef boost::shared_ptr<RegisterAST> Ptr;
      
      /// Construct a register, assigning it the ID \c id.
      RegisterAST(int id);
  
      virtual ~RegisterAST();
      

      /// By definition, a %RegisterAST object has no children.
      /// \param[out] children Since a %RegisterAST has no children, the \c children parameter is unchanged by this method.
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const;

      /// By definition, the use set of a %RegisterAST object is itself.
      /// \param[out] uses This %RegisterAST will be inserted into \c uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses) const;

      /// \c isUsed returns true if \c findMe is a %RegisterAST that represents
      /// the same register as this %RegisterAST, and false otherwise.
      virtual bool isUsed(InstructionAST::Ptr findMe) const;

      virtual std::string format() const;

      /// Utility function to get a Register object that represents the program counter.
      ///
      /// \c makePC is provided to support platform-independent control flow analysis.
      static RegisterAST makePC();

      /// We define a partial ordering on registers by their register number so that they may be placed into sets
      /// or other sorted containers.
      bool operator<(const RegisterAST& rhs) const;

      unsigned int getID() const;
      

    protected:
      virtual bool isSameType(const InstructionAST& rhs) const;
      virtual bool isStrictEqual(const InstructionAST& rhs) const;
    private:
      unsigned int registerID;
    };
    
  };
};


  

#endif // !defined(REGISTER_H)
