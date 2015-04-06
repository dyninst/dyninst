// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "codegen.h"
#include "Symbol.h"
#include "PCProcess.h"

#include "common/src/arch-aarch64.h"

using namespace Dyninst;
using namespace NS_aarch64;
using namespace ProcControlAPI;
using namespace std;

