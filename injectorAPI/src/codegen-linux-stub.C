// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "codegen.h"
#include "Symbol.h"
#include "PCProcess.h"


using namespace Dyninst;
using namespace InjectorAPI;
using namespace SymtabAPI;
using namespace ProcControlAPI;

bool Codegen::generateLinux() {
	return false;
}

Address Codegen::buildLinuxArgStruct(Address, unsigned) {
	return 0;
}

bool Codegen::generateStackUnprotect() {
	return false;
}

bool Codegen::findTOC(Symbol *, Library::ptr) {
	return false;
}
