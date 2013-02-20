// x86-specific routines

#include "loadLibrary/codegen.h"
#include <iostream>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

bool Codegen::generateCallIA32(Address addr, const std::vector<Address> &args) {
   for (auto iter = args.rbegin(); iter != args.rend(); ++iter) {
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

   // Zero rax
   copyByte(0x48);
   copyByte(0x31);
   copyByte(0xc0);

   // mov addr, rbx
   copyByte(0x48);
   copyByte(0xbb);
   copyLong(addr);

   copyByte(0xff);
   copyByte(0xd3);
   return true;
}

bool Codegen::generatePreambleIA32() {
   return true;
}

bool Codegen::generatePreambleAMD64() {
   // Round the stack to word alignment
   // I copied this from _start
   copyByte(0x48); // rex 
   copyByte(0x83);
   copyByte(0xe4); // opcode: and %rsp
   copyByte(0xf0); // operand

   // Also, push it down to ensure we don't overwrite anything 
   // live on the stack
   // We need -128 (-0x80), but that's a signed byte so we do
   // it in halves. 

   copyByte(0x48); // rex
   copyByte(0x83); // sub
   copyByte(0xec); // register
   copyByte(0x40); // -0x80 

   copyByte(0x48); // rex
   copyByte(0x83); // sub
   copyByte(0xec); // register
   copyByte(0x40); // -0x80 

   return true;
}
