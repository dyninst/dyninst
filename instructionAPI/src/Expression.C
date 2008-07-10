
#include "../h/Expression.h"

namespace Dyninst
{
  namespace Instruction
  {
    Expression::Expression(Result_Type t) :
      userSetValue(t)
    {
    } 
    Expression::~Expression() 
    {
    }
    Result Expression::eval() const
    {
      return userSetValue;
    }
    void Expression::setValue(Result knownValue) 
    {
      userSetValue = knownValue;
    }
    void Expression::clearValue()
    {
      userSetValue.defined = false;
    }
    int Expression::size() const
    {
      return userSetValue.size();
    }
  };
};
