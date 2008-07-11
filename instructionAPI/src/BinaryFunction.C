#include "../h/BinaryFunction.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    Result operator+(const Result& arg1, const Result& arg2)
    {
      Result_Type resultT = (arg1.type > arg2.type) ? arg1.type : arg2.type;
      Result retVal(resultT);
      if(!arg1.defined || !arg2.defined)
      {
	retVal.defined = false;
      }
      else
      {
	retVal.defined = true;
      }
      return retVal;
    }    

  };
};

