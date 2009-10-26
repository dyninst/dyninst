#ifndef _get_trap_instruction_h
#define _get_trap_instruction_h

namespace Dyninst {
  namespace Stackwalker {

    void getTrapInstruction(char *buffer, unsigned buf_size, unsigned &actual_len, bool include_return);

  } // Stackwalker
} // Dyninst

#endif // _get_trap_instruction_h
