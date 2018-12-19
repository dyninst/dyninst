// PPC code generation routines for library injection

#include "loadLibrary/codegen.h"
#include "common/src/arch-power.h"
#include <iostream>


using namespace Dyninst;
using namespace NS_power;
using namespace ProcControlAPI;
using namespace std;

// User code can use up to 288 bytes under the stack pointer;
// we skip past this so that we don't mess things up. 
//
// And system code can further use 224 bytes more under the stack pointer.
//
// To be safe, we move down the stack pointer by 512
#define STACKSKIP 512

bool Codegen::generateCallPPC32(Address addr, const std::vector<Address> &args) {
   // PPC32 on Linux is basically the same thing as x86; we do indirect because
   // we can

   // First, arguments
   unsigned reg = 3;
   for (auto iter = args.begin(); iter != args.end(); ++iter) {
      generatePPC32(*iter, reg);
      reg++;      
   }

   // And the call. We're going indirect, r0 -> LR -> call
   generatePPC32(addr, 0);

   instruction mtlr(MTLR0raw);
   copyInt(mtlr.asInt());

   instruction brl(BRLraw);
   copyInt(brl.asInt());
   return true;
}

bool Codegen::generateCallPPC64(Address addr, const std::vector<Address> &args) 
{
   // PPC64 is a little more complicated, because we also need a TOC register value.
   // That... is tricky. 
   //
   // Enter PPC64LE and ABIv2... The LittleEndian version of PPC64 requires ABIv2
   // and PPC64 will run with both ABIv1 and ABIv2.
   //
   // The main implication for this code is that ABIv2 no longer uses function
   // descriptors and no longer needs to be concerned with the TOC register.
   // Instead, ABIv2-compliant callers need place the address of the global
   // entry point to the function being called in r12 before jumping to the
   // function's global entry point.  The called function will then be able to
   // derive the TOC using the value stored in r12.
   //
   // ABIv1 compiant calls still needs to find and manipulate the TOC.

   // First, arguments
   unsigned reg = 3;
   for (auto iter = args.begin(); iter != args.end(); ++iter) {
      generatePPC64(*iter, reg);
      reg++;      
   }
   if (abimajversion_ < 2) {
      if (toc_[addr] == 0) return false;
      generatePPC64(toc_[addr], 2);
      generatePPC64(addr, 0);
      instruction mtlr(MTLR0raw); copyInt(mtlr.asInt());
      instruction brl(BRLraw); copyInt(brl.asInt());
   }
   else {
      generatePPC64(addr, 12);
      instruction mtctr(MR12CTR); copyInt(mtctr.asInt());
      instruction bctrl(BCTRLraw); copyInt(bctrl.asInt());
   }

   return true;
}

void Codegen::generatePPC32(Address val, unsigned reg) {

      instruction insn;
      insn.clear();
      DFORM_OP_SET(insn, CAUop);
      DFORM_RT_SET(insn, reg);
      DFORM_RA_SET(insn, 0);
      DFORM_SI_SET(insn, BOT_HI(val));
      copyInt(insn.asInt());

      insn.clear();
      DFORM_OP_SET(insn, ORILop);
      DFORM_RT_SET(insn, reg);
      DFORM_RA_SET(insn, reg);
      DFORM_SI_SET(insn, BOT_LO(val));
      copyInt(insn.asInt());
}

void Codegen::generatePPC64(Address val, unsigned reg) {
   generatePPC32((uint64_t)val >> 32, reg);

   instruction insn;
   insn.clear();
   int shift = 32;
   MDFORM_OP_SET( insn, RLDop);
   MDFORM_RS_SET( insn, reg);
   MDFORM_RA_SET( insn, reg);
   MDFORM_SH_SET( insn, shift % 32);
   MDFORM_MB_SET( insn, (63-shift) % 32);
   MDFORM_MB2_SET(insn, (63-shift) / 32);
   MDFORM_XO_SET( insn, ICRxop);
   MDFORM_SH2_SET(insn, shift / 32);
   MDFORM_RC_SET( insn, 0);
   copyInt(insn.asInt());

   // Can't just reuse the 32-bit generate, since CAUop zeroes the high bits...
   // Instead, use ORIU/ORIL

   insn.clear();
   DFORM_OP_SET(insn, ORIUop);
   DFORM_RT_SET(insn, reg);
   DFORM_RA_SET(insn, reg);
   DFORM_SI_SET(insn, BOT_HI(val));
   copyInt(insn.asInt());
   
   insn.clear();
   DFORM_OP_SET(insn, ORILop);
   DFORM_RT_SET(insn, reg);
   DFORM_RA_SET(insn, reg);
   DFORM_SI_SET(insn, BOT_LO(val));
   copyInt(insn.asInt());

}

bool Codegen::generatePreamblePPC32() {
  instruction insn;
  insn.clear();

  DFORM_OP_SET(insn, CALop);
  DFORM_RT_SET(insn, 1);
  DFORM_RA_SET(insn, 1);
  DFORM_SI_SET(insn, -STACKSKIP);
  copyInt(insn.asInt());

  return true;
}

bool Codegen::generatePreamblePPC64() {
  return generatePreamblePPC32();
}
