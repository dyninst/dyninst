
#include "../h/InstructionAST.h"
#include <string>

namespace Dyninst
{
  namespace Instruction
  {
    InstructionAST::InstructionAST()
    {
    }
    InstructionAST::~InstructionAST()
    {
    }
    

    bool InstructionAST::operator==(const InstructionAST& rhs) const
    {
      // isStrictEqual assumes rhs and this to be of the same derived type
      // so isSameType enforces this restriction
      return(isSameType(rhs) && isStrictEqual(rhs));
    }
  };
};

  
