
#if !defined(FRAMECHECKER_H)
#define FRAMECHECKER_H

#include "../../instructionAPI/h/Instruction.h"

class frameChecker
{
 public:
  frameChecker(const unsigned char* addr, size_t max_length);
  virtual ~frameChecker();
  
  bool isReturn() const;
  bool isStackPreamble() const;
  bool isStackFrameSetup() const;
  
 private:
  bool isMovStackToBase(int index_to_check) const;
  vector<Dyninst::Instruction::Instruction> m_Insns;
};



#endif //!defined(FRAMECHECKER_H)
