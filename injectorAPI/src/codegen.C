// Platform-independent code generation methods; mainly function
// lookup

#include "codegen.h"
#include "Symtab.h"
#include "Symbol.h"
#include <iostream>

using namespace Dyninst;
using namespace InjectorAPI;
using namespace ProcControlAPI;
using namespace SymtabAPI;
using namespace std;


Codegen::Codegen(Process::ptr proc, std::string libname) 
   : proc_(proc), libname_(libname), codeStart_(0), toc_(0) {}

Codegen::~Codegen() {
   if (codeStart_) {
      proc_->freeMemory(buffer_.startAddr());
   }
}

bool Codegen::generate() {
   unsigned size = estimateSize();
   
   codeStart_ = proc_->mallocMemory(size);
   if (!codeStart_) {
      return false;
   }


   buffer_.initialize(codeStart_, size);

   bool ret = false;
   switch(proc_->getOS()) {
      case Windows:
         ret = generateWindows();
         break;
      case Linux:
         ret = generateLinux();
         break;
      default:
         break;
   }
   if (!ret) {
      return false;
   }
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

Address Codegen::findSymbolAddr(const std::string name, bool func, bool saveTOC) {
   LibraryPool& libs = proc_->libraries();

   for (auto li = libs.begin(); li != libs.end(); li++) {
      if ((*li)->getName().empty()) continue;
      Symtab *obj = NULL;
      bool ret = Symtab::openFile(obj, (*li)->getName());
      if (!ret || !obj) continue;
 
      std::vector<Symbol *> syms;
      obj->findSymbol(syms, name, 
                      func ? Symbol::ST_FUNCTION : Symbol::ST_OBJECT,
                      mangledName);
      // This would be nice but causes faults
      //Symtab::closeSymtab(obj);

      if (syms.empty()) continue;

      // UGLY HACK!
      if (saveTOC) {
         findTOC(syms[0], (*li));
      }

      return syms[0]->getOffset() + (*li)->getLoadAddress();
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
      case Arch_ppc32:
         return generateCallPPC32(addr, args);
      case Arch_ppc64:
         return generateCallPPC64(addr, args);
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
      case Arch_ppc64:
         copyInt(0x60000000);
         copyInt(0x60000000);
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
         copyInt(0x7d821008);
         break;
      default:
         return false;
         break;
   }
   return true;
}

