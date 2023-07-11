// Codegen.h
//
// Interface class for generating a dlopen() call
//

#if !defined(_INJECTOR_CODEGEN_H_)
#define _INJECTOR_CODEGEN_H_

#include <map>
#include <string>
#include <vector>
#include "PCProcess.h"
#include "Buffer.h"

namespace Dyninst {

namespace ProcControlAPI {

class Codegen {
  public:
   Codegen(ProcControlAPI::Process *proc,
           std::string libname);
   ~Codegen();

   // Override for assembly generation
   bool generate();

   const Buffer &buffer() { return buffer_; }
   unsigned startOffset() const;

  private:

   unsigned estimateSize();
   bool generateInt();
   Address findSymbolAddr(const std::string name, bool saveTOC = false);
   Address copyString(std::string);
   Address copyBuf(void *buf, unsigned size);
   Address copyByte(unsigned char);
   Address copyInt(unsigned int);
   Address copyLong(unsigned long);

#if defined(os_linux)
   Address buildLinuxArgStruct(Address libbase, unsigned mode);
   bool generateStackUnprotect(Address var, Address mprotect);
#endif

   bool generateCall(Address addr, const std::vector<Address> &args);

   bool generateCallIA32(Address addr, const std::vector<Address> &args);
   bool generateCallAMD64(Address addr, const std::vector<Address> &args);

   bool generateCallPPC32(Address addr, const std::vector<Address> &args);
   bool generateCallPPC64(Address addr, const std::vector<Address> &args);


   bool generatePreamble();
   bool generatePreambleIA32();
   bool generatePreambleAMD64();

   bool generatePreamblePPC32();
   bool generatePreamblePPC64();

   void generatePPC32(Address val, unsigned reg);
   void generatePPC64(Address val, unsigned reg);

   bool generatePreambleAARCH64();
   bool generateCallAARCH64(Address addr, const std::vector<Address> &args);

   bool generateTrap();
   bool generateNoops();

   ProcControlAPI::Process *proc_;
   std::string libname_;

   Address codeStart_;
   Buffer buffer_;

   // PPC64 only, but it's handy to stash it here
   std::map<Address, Address> toc_;
   int abimajversion_;
   int abiminversion_;
};

}
}

#endif
