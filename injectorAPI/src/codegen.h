// Codegen.h
//
// Interface class for generating a dlopen() call
//

#if !defined(_INJECTOR_CODEGEN_H_)
#define _INJECTOR_CODEGEN_H_

#include "PCProcess.h"
#include "Buffer.h"

namespace Dyninst {

   namespace SymtabAPI {
      class Symbol;
   };

namespace InjectorAPI {

class Codegen {
  public:
   Codegen(ProcControlAPI::Process::ptr proc, 
           std::string libname);
   ~Codegen();

   // Override for assembly generation
   bool generate();

   const Buffer &buffer() { return buffer_; }
   unsigned startOffset() const;

  private:
   
   unsigned estimateSize();
   bool generateLinux();
   bool generateWindows();
   Address findSymbolAddr(const std::string name, bool func, bool saveTOC);
   Address copyString(std::string);
   Address copyBuf(void *buf, unsigned size);
   Address copyByte(unsigned char);
   Address copyInt(unsigned int);
   Address copyLong(unsigned long);
   Address buildLinuxArgStruct(Address libbase, unsigned mode);
   bool generateStackUnprotect();
   bool generateCall(Address addr, const std::vector<Address> &args);

   bool generateCallIA32(Address addr, const std::vector<Address> &args);
   bool generateCallAMD64(Address addr, const std::vector<Address> &args);
   bool generateCallPPC32(Address addr, const std::vector<Address> &args);
   bool generateCallPPC64(Address addr, const std::vector<Address> &args);

   void generatePPC32(Address val, unsigned reg);
   void generatePPC64(Address val, unsigned reg);
   bool findTOC(SymtabAPI::Symbol *sym, ProcControlAPI::Library::ptr lib);

   bool generateTrap();
   bool generateNoops();

   ProcControlAPI::Process::ptr proc_;
   std::string libname_;

   Address codeStart_;
   Buffer buffer_;   

   // PPC64 only, but it's handy to stash it here
   Address toc_;
};

};
};

#endif
