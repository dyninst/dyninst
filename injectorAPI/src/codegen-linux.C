// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "codegen.h"
#include <dlfcn.h>
#include <iostream>
#include "Symbol.h"
#include "Symtab.h"
#include "PCProcess.h"

using namespace Dyninst;
using namespace InjectorAPI;
using namespace std;
using namespace ProcControlAPI;
using namespace SymtabAPI;

static const int DLOPEN_MODE = RTLD_NOW | RTLD_GLOBAL;

// Note: this is an internal libc flag -- it is only used
// when libc and ld.so don't have symbols
#ifndef __RTLD_DLOPEN
#define __RTLD_DLOPEN 0x80000000
#endif

const char *DL_OPEN_FUNC_USER = NULL;
const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_LIBC_FUNC_EXPORTED[] = "__libc_dlopen_mode";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";


bool Codegen::generateLinux() {
   Address dlopen_addr = 0;

    int mode = DLOPEN_MODE;

    bool useHiddenFunction = false;
    bool needsStackUnprotect = false;
    
    do {
       dlopen_addr = findSymbolAddr(DL_OPEN_FUNC_EXPORTED, true, true); 
       if (dlopen_addr) break;
       
       // This approach will work if libc and the loader have symbols
       // Note: this is more robust than the next approach
       useHiddenFunction = true;
       needsStackUnprotect = true;
       dlopen_addr = findSymbolAddr(DL_OPEN_FUNC_NAME, true, true);
       if (dlopen_addr) break;
       
       // If libc and the loader don't have symbols, we need to take a
       // different approach. We still need to the stack protection turned
       // off, but since we don't have symbols we use an undocumented flag
       // to turn off the stack protection
       useHiddenFunction = false;
       needsStackUnprotect = false;
       mode |= __RTLD_DLOPEN;
       dlopen_addr = findSymbolAddr(DL_OPEN_LIBC_FUNC_EXPORTED, true, true);
       if (dlopen_addr) break;
       
       // We can't go farther without parsing
       return false;
    } while(0);

    assert(dlopen_addr);

    std::vector<Address> arguments;

    Address libbase = copyString(libname_);

    if (useHiddenFunction) {
       // The argument is a pointer to a struct rather than
       // arguments directly
       Address structbase = buildLinuxArgStruct(libbase, mode);
       arguments.push_back(structbase);
    }
    else {
       arguments.push_back(libbase);
       arguments.push_back(mode);
    }
    generateNoops();
    codeStart_ = buffer_.curAddr();

    if (needsStackUnprotect) {
       if (!generateStackUnprotect()) return false;
    }

    if (!generateCall(dlopen_addr, arguments)) return false;
    
    return true;
}

Address Codegen::buildLinuxArgStruct(Address libbase, unsigned mode) {
   struct libc_dlopen_args_32 {
      uint32_t namePtr;
      uint32_t mode;
      uint32_t linkMapPtr;
   };
   
   struct libc_dlopen_args_64 {
      uint64_t namePtr;
      uint32_t mode;
      uint64_t linkMapPtr;
   };
   
   // Construct the argument to the internal function
   struct libc_dlopen_args_32 args32;
   struct libc_dlopen_args_64 args64;
   unsigned argsSize = 0;
   void *argsPtr = NULL;

   if (proc_->getArchitecture() == Arch_x86 ||
       proc_->getArchitecture() == Arch_ppc32) {
      args32.namePtr = (uint32_t) libbase;
      args32.mode = mode;
      args32.linkMapPtr = 0;
      argsSize = sizeof(args32);
      argsPtr = &args32;
   }
   else {
      args64.namePtr = libbase;
      args64.mode = mode;
      args64.linkMapPtr = 0;
      argsSize = sizeof(args64);
      argsPtr = &args64;
   }

   return copyBuf(argsPtr, argsSize);
}

bool Codegen::generateStackUnprotect() {
   // Since we are punching our way down to an internal function, we
   // may run into problems due to stack execute protection. Basically,
   // glibc knows that it needs to be able to execute on the stack in
   // in order to load libraries with dl_open(). It has code in
   // _dl_map_object_from_fd (the workhorse of dynamic library loading)
   // that unprotects a global, exported variable (__stack_prot), sets
   // the execute flag, and reprotects it. This only happens, however,
   // when the higher-level dl_open() functions (which we skip) are called,
   // as they append an undocumented flag to the library open mode. Otherwise,
   // assignment to the variable happens without protection, which will
   // cause a fault.
   //
   // Instead of chasing the value of the undocumented flag, we will
   // unprotect the __stack_prot variable ourselves (if we can find it).

   Address var_addr = findSymbolAddr("__stack_prot", false, false);
   Address mprotect_addr = findSymbolAddr("mprotect", true, false);

   if (!var_addr || !mprotect_addr) return false;

   Address page_start;
   unsigned size;
   unsigned pagesize = getpagesize();

   page_start = var_addr & ~(pagesize - 1);
   size = var_addr - page_start + sizeof(int);

   std::vector<Address> args;
   args.push_back(page_start);
   args.push_back(size);
   return generateCall(mprotect_addr, args);
}

bool Codegen::findTOC(Symbol *sym, Library::ptr lib) {
   if (proc_->getArchitecture() != Arch_ppc64) return true;


   Address baseTOC = sym->getSymtab()->getTOCoffset(sym->getOffset());
   baseTOC += lib->getDataLoadAddress();
   toc_ = baseTOC;
   return toc_ != 0;
}
