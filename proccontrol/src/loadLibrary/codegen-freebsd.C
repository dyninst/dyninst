// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "loadLibrary/codegen.h"
#include <dlfcn.h>
#include <iostream>
#include "PCProcess.h"
#include "int_process.h"
#include "common/src/pathName.h"
#include <sys/mman.h>

using namespace Dyninst;
using namespace std;
using namespace ProcControlAPI;

static const int DLOPEN_MODE = RTLD_NOW | RTLD_GLOBAL;

const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";

bool Codegen::generateInt() {
    // We need to make sure that the correct dlopen function is being used -- the
    // dlopen in the runtime linker. A symbol for dlopen exists in ld.so even
    // when it is stripped so we should always find that version of dlopen

    auto aout = proc_->libraries().getExecutable();
    SymReader *objSymReader = proc_->llproc()->getSymReader()->openSymbolReader(aout->getName());
    if (!objSymReader) {
      return false;
    }
    std::string interp = resolve_file_path(objSymReader->getInterpreterName());

    objSymReader = proc_->llproc()->getSymReader()->openSymbolReader(interp);
    if (!objSymReader) {
      return false;
    }
    auto lookupSym = objSymReader->getSymbolByName(DL_OPEN_FUNC_EXPORTED);
    if (!objSymReader->isValidSymbol(lookupSym)) {
      return false;
    }

    Address dlopenAddr = objSymReader->getSymbolOffset(lookupSym);

    // But we still need the load addr...
    bool found = false;
    for (auto li = proc_->libraries().begin(); li != proc_->libraries().end(); ++li) {
      std::string canonical = resolve_file_path((*li)->getName());
      if (canonical == interp) {
	found = true;
	dlopenAddr += (*li)->getLoadAddress();
	break;
      }
    }
    if (!found) {
      return false;
    }

    std::vector<Address> arguments;

    Address libbase = copyString(libname_);

    arguments.push_back(libbase);
    arguments.push_back(DLOPEN_MODE);

    generateNoops();
    codeStart_ = buffer_.curAddr();
    
    generatePreamble();
    
    if (!generateCall(dlopenAddr, arguments)) return false;
    
    return true;
}

