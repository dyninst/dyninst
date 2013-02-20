// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "codegen.h"
#include "Symbol.h"
#include "PCProcess.h"


using namespace Dyninst;
using namespace ProcControlAPI;

bool Codegen::generateInt() {
	return false;
}
