// x86-specific routines

#include "codegen.h"

using namespace Dyninst;
using namespace InjectorAPI;

bool Codegen::generateCallIA32(Address addr, const std::vector<Address> &args) {
   for (auto iter = args.begin(); iter != args.end(); ++iter) {
      copyByte(0x68);
      copyInt(*iter);
   }
   
   unsigned offset = addr - (buffer_.curAddr() + 5);
   copyByte(0xe8);
   copyInt(offset);
   return true;
}

bool Codegen::generateCallAMD64(Address addr, const std::vector<Address> &args) {
   if (args.size() >= 1) {
      copyByte(0x48);
      copyByte(0xbf);
      copyLong(args[0]);
   }
   if (args.size() >= 2) {
      copyByte(0x48);
      copyByte(0xbe);
      copyLong(args[1]);
   }
   if (args.size() >= 3) {
      copyByte(0x48);
      copyByte(0xba);
      copyLong(args[2]);
   }
   if (args.size() >= 4) {
      copyByte(0x48);
      copyByte(0xb9);
      copyLong(args[3]);
   }
   if (args.size() >= 5) return false;

   // mov addr, rax
   copyByte(0x48);
   copyByte(0xb8);
   copyLong(addr);

   copyByte(0xff);
   copyByte(0xd0);
   return true;
}


