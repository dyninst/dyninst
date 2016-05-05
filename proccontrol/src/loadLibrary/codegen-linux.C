// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "loadLibrary/codegen.h"
#include <dlfcn.h>
#include <iostream>
#include "PCProcess.h"
#include <sys/mman.h>

using namespace Dyninst;
using namespace std;
using namespace ProcControlAPI;

static const int DLOPEN_MODE = RTLD_NOW | RTLD_GLOBAL;

// Note: this is an internal libc flag -- it is only used
// when libc and ld.so don't have symbols
#ifndef __RTLD_DLOPEN
#define __RTLD_DLOPEN 0x80000000
#endif

static const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
static const char DL_OPEN_LIBC_FUNC_EXPORTED[] = "__libc_dlopen_mode";
static const char DL_OPEN_FUNC_NAME[] = "do_dlopen";


bool Codegen::generateInt() {
   Address dlopen_addr = 0;

    int mode = DLOPEN_MODE;
    bool useHiddenFunction = false;
    bool needsStackUnprotect = false;
    Address var_addr = 0;
    Address mprotect_addr = 0;
    do {
       dlopen_addr = findSymbolAddr(DL_OPEN_FUNC_EXPORTED, true); 
       if (dlopen_addr) {
          break;
       }
       
       // This approach will work if libc and the loader have symbols
       // Note: this is more robust than the next approach
       useHiddenFunction = true;
       needsStackUnprotect = true;
       dlopen_addr = findSymbolAddr(DL_OPEN_FUNC_NAME, true);
       var_addr = findSymbolAddr("__stack_prot");
       mprotect_addr = findSymbolAddr("mprotect", true);
       if (dlopen_addr && var_addr && mprotect_addr) {
          break;
       }
       
       // If libc and the loader don't have symbols, we need to take a
       // different approach. We still need to the stack protection turned
       // off, but since we don't have symbols we use an undocumented flag
       // to turn off the stack protection
       useHiddenFunction = false;
       needsStackUnprotect = false;
       mode |= __RTLD_DLOPEN;
       dlopen_addr = findSymbolAddr(DL_OPEN_LIBC_FUNC_EXPORTED, true);
       if (dlopen_addr) {
          break;
       }
       fprintf(stderr, "Couldn't find dlopen address, bailing\n");
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
    
    generatePreamble();
    
    if (needsStackUnprotect) {
      if (!generateStackUnprotect(var_addr, mprotect_addr)) return false;
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

bool Codegen::generateStackUnprotect(Address var_addr, Address mprotect_addr) {
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


   if (!var_addr || !mprotect_addr) {
     fprintf(stderr, "Couldn't find symbols to unprotect stack, bailing\n");
     return false;
   }
   Address page_start;
   Address pagesize = getpagesize();

   page_start = var_addr & ~(pagesize - 1);


   std::vector<Address> args;
   args.push_back(page_start);
   args.push_back(pagesize);
   args.push_back(PROT_READ | PROT_WRITE | PROT_EXEC); // read | write | execute

   return generateCall(mprotect_addr, args);
}

