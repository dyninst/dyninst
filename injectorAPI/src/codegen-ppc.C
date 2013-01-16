// PPC code generation routines for library injection

#include "codegen.h"
#include "common/h/arch-power.h"


using namespace Dyninst;
using namespace InjectorAPI;
using namespace NS_power;

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

bool Codegen::generateCallPPC64(Address addr, const std::vector<Address> &args) {
   // PPC64 is a little more complicated, because we also need a TOC register value.
   // That... is tricky. 

   // First, arguments
   unsigned reg = 3;
   for (auto iter = args.begin(); iter != args.end(); ++iter) {
      generatePPC32(*iter, reg);
      reg++;      
   }

   if (toc_ == 0) return false;
   generatePPC64(toc_, 2);

   generatePPC64(addr, 0);

   instruction mtlr(MTLR0raw);
   copyInt(mtlr.asInt());

   instruction brl(BRLraw);
   copyInt(brl.asInt());
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
   generatePPC32(val >> 32, reg);

   instruction insn;
   insn.clear();
   MDFORM_RS_SET( insn, reg);
   MDFORM_RA_SET( insn, reg);
   MDFORM_SH_SET( insn, 0);
   MDFORM_MB_SET( insn, 31);
   MDFORM_MB2_SET(insn, 0);
   MDFORM_XO_SET( insn, ICRxop);
   MDFORM_SH2_SET(insn, 1);
   MDFORM_RC_SET( insn, 0);
   copyInt(insn.asInt());

   generatePPC32(val, reg);
}
