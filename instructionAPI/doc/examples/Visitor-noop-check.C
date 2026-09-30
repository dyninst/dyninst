// Not typeset.  Visitor-noop.C is printed "without surrounding use code",
// as the manual says, so it names types it never includes; this gives it
// that context so the docs build can compile it like the other examples.

#include "Instruction.h"
#include "Visitor.h"

using namespace Dyninst;
using namespace InstructionAPI;

#include "Visitor-noop.C"
