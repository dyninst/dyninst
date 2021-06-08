// Platform-independent code generation methods; mainly function
// lookup

#include "loadLibrary/codegen.h"
#include <iostream>
#include "int_process.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;


Codegen::Codegen(Process *proc, std::string libname)
   : proc_(proc), libname_(libname), codeStart_(0) {}

Codegen::~Codegen() {
   int_process *llproc = proc_->llproc();
   if (codeStart_ && llproc) {
      llproc->infFree(buffer_.startAddr());
   }
}

bool Codegen::generate() {
   unsigned size = estimateSize();
   int_process *proc = proc_->llproc();
   if (!proc)
      return false;

   codeStart_ = proc->infMalloc(size, false, (unsigned int) 0);
   if (!codeStart_) {
      return false;
   }

   buffer_.initialize(codeStart_, size);

   abimajversion_ = abiminversion_ = 0;
   auto exe = proc_->libraries().getExecutable();
   SymReader *objSymReader = proc_->llproc()->getSymReader()->openSymbolReader(exe->getName());
   if (objSymReader)
      objSymReader->getABIVersion(abimajversion_, abiminversion_);

   if (!generateInt()) return false;

   generateTrap();
   generateTrap();

   assert(buffer_.size() <= size);

   return true;
}

unsigned Codegen::startOffset() const {
   return codeStart_ - buffer_.startAddr();
}

unsigned Codegen::estimateSize() {
   // Major overestimation...
   return 256 + libname_.length();
}

Address Codegen::findSymbolAddr(const std::string name, bool saveTOC) {
   LibraryPool& libs = proc_->libraries();
   for (auto li = libs.begin(); li != libs.end(); li++) {
      if ((*li)->getName().empty()) continue;

      SymReader *objSymReader = proc_->llproc()->getSymReader()->openSymbolReader((*li)->getName());
      if (!objSymReader) continue;

      Symbol_t lookupSym = objSymReader->getSymbolByName(name);
      if (!objSymReader->isValidSymbol(lookupSym)) continue;

      Address addr = (*li)->getLoadAddress() + objSymReader->getSymbolOffset(lookupSym);

      if (saveTOC) {
         toc_[addr] = (*li)->getLoadAddress() + objSymReader->getSymbolTOC(lookupSym);
      }
      return addr;
   }
   return 0;
}

Address Codegen::copyString(std::string name) {
   Address ret = buffer_.curAddr();
   unsigned strsize = name.length() + 1;
   // Round to multiple of 4
   strsize += 3; strsize -= (strsize % 4);
   buffer_.copy(name.begin(), name.end());
   for (unsigned i = 0; i < (strsize - name.length()); ++i) {
      buffer_.push_back((unsigned char) 0x0);
   }
   return ret;
}

Address Codegen::copyBuf(void *buf, unsigned size) {
   Address ret = buffer_.curAddr();
   // Round size to multiple of 4
   size += 3; size -= (size % 4);
   buffer_.copy(buf, size);
   return ret;
}

Address Codegen::copyByte(unsigned char c) {
   Address ret = buffer_.curAddr();
   buffer_.push_back(c);
   return ret;
}

Address Codegen::copyInt(unsigned int i) {
   Address ret = buffer_.curAddr();
   buffer_.push_back(i);
   return ret;
}

Address Codegen::copyLong(unsigned long l) {
   Address ret = buffer_.curAddr();
   buffer_.push_back(l);
   return ret;
}

bool Codegen::generateCall(Address addr, const std::vector<Address> &args) {
   switch (proc_->getArchitecture()) {
      case Arch_x86:
         return generateCallIA32(addr, args);
      case Arch_x86_64:
         return generateCallAMD64(addr, args);
#if !defined(os_windows)
	  case Arch_ppc32:
         return generateCallPPC32(addr, args);
      case Arch_ppc64:
         return generateCallPPC64(addr, args);
#endif //!defined(os_windows)
#if defined(arch_aarch64)
      case Arch_aarch64:
         return generateCallAARCH64(addr, args);
#endif
	  default:
         return false;
   }
}

bool Codegen::generateNoops() {
   // Linux has an annoying habit of rewinding the PC before executing code
   // if you're in a system call; so 8-byte pad no matter what.
   switch(proc_->getArchitecture()) {
      case Arch_x86:
      case Arch_x86_64:
         copyInt(0x90909090);
         copyInt(0x90909090);
         break;
      case Arch_ppc32:
      case Arch_ppc64:  // MJMTODO - Assumes HostArch == TargetArch
         copyInt(0x60000000);
         copyInt(0x60000000);
         break;
      case Arch_aarch64:
         copyInt(0xd503201f);
	 break;
      default:
         return false;
         break;
   }
   return true;
}

bool Codegen::generateTrap() {
   switch(proc_->getArchitecture()) {
      case Arch_x86:
      case Arch_x86_64:
         copyByte(0xcc);
         break;
      case Arch_ppc32:
      case Arch_ppc64:
         copyInt(0x7d821008); // MJMTODO - Assumes HostArch == TargetArch
         break;
      case Arch_aarch64:
         copyInt(0xd4200000);
	 break;
      default:
         return false;
         break;
   }
   return true;
}


bool Codegen::generatePreamble() {
   switch (proc_->getArchitecture()) {
      case Arch_x86:
         return generatePreambleIA32();
      case Arch_x86_64:
         return generatePreambleAMD64();
#if !defined(os_windows)
      case Arch_ppc32:
         return generatePreamblePPC32();
      case Arch_ppc64:
         return generatePreamblePPC64();
#endif //!defined(os_windows)
#if defined(arch_aarch64)
      case Arch_aarch64:
         return generatePreambleAARCH64();
#endif
	  default:
         return false;
   }
}
