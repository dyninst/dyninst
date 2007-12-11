#if !defined(INSTRUC_ITER_FUNCTION_H)
#define INSTRUC_ITER_FUNCTION_H

#include "common/h/Types.h"
#include <vector>
#include "InstrucIter.h"


class image_func;
class int_function;

class InstrucIterFunction : public InstrucIter
{
 public:
  InstrucIterFunction(int_function* func);
  InstrucIterFunction(Address start, int_function* func);
  InstrucIterFunction(const InstrucIterFunction& ii);
  
  virtual ~InstrucIterFunction() {};
  
  virtual Address operator++();
  virtual Address operator--();
  virtual Address operator++(int);
  virtual Address operator--(int);
  virtual Address operator* () const;
  virtual instruction getInstruction();
  virtual instruction* getInsnPtr();
  virtual bool hasMore();
  virtual bool hasPrev();
  virtual Address peekNext();
  virtual Address peekPrev();
  virtual void setCurrentAddress(Address a);
  virtual Address getCurrentAddress();
  
 private:
  typedef std::vector<InstrucIter> subIterContT;
  subIterContT subIters;
  subIterContT::iterator currentBlock;
  void debugPrint() const;
  
  
};

#endif //!defined(INSTRUC_ITER_FUNCTION_H)
