
#if !defined(FRAMECHECKER_H)
#define FRAMECHECKER_H

#if defined (cap_instruction_api)
#include "instructionAPI/h/Instruction.h"

class frameChecker
{
 public:
  frameChecker(const unsigned char* addr, size_t max_length);
  virtual ~frameChecker();
  
  bool isReturn() const;
  bool isStackPreamble() const;
  bool isStackFrameSetup() const;
  
 private:
  bool isMovStackToBase(unsigned index_to_check) const;
  std::vector<Dyninst::InstructionAPI::Instruction> m_Insns;
};
#endif


#endif //!defined(FRAMECHECKER_H)
