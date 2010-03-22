#include "SymEvalPolicy.h"

#include "dyn_regs.h"

using namespace Dyninst;
using namespace Dyninst::SymbolicEvaluation;
using namespace Dyninst::InstructionAPI;

SymEvalPolicy::SymEvalPolicy(SymEval::Result &r, 
			     Address a,
			     Architecture ac) :
  res(r),
  addr(a),
  arch(ac),
  ip_(Handle<32>(wrap(Absloc::makePC(arch)))) {

  // We also need to build aaMap FTW!!!
  for (SymEval::Result::iterator iter = r.begin();
       iter != r.end(); ++iter) {
    Assignment::Ptr a = iter->first;
    // For a different instruction...
    if (a->addr() != addr) continue; 
    AbsRegion &o = a->out();

    if (o.containsOfType(Absloc::Register)) {
      // We're assuming this is a single register...
      aaMap[o.absloc()] = a;
    }
    else {
      // Use sufficiently-unique (Heap,0) Absloc
      // to represent a definition to a memory absloc
      aaMap[Absloc(0)] = a;
    }
  }
}
  

void SymEvalPolicy::startInstruction(SgAsmx86Instruction *) {
}

void SymEvalPolicy::finishInstruction(SgAsmx86Instruction *) {
}

Absloc SymEvalPolicy::convert(X86GeneralPurposeRegister r)
{

  MachRegister mreg;
  switch (r) {
    case x86_gpr_ax:
      mreg = x86::eax;
      break;
    case x86_gpr_cx:
      mreg = x86::ecx;
      break;
    case x86_gpr_dx:
      mreg = x86::edx;
      break;
    case x86_gpr_bx:
      mreg = x86::ebx;
      break;
    case x86_gpr_sp:
      mreg = x86::esp;
      break;
    case x86_gpr_bp:
      mreg = x86::ebp;
      break;
    case x86_gpr_si:
      mreg = x86::esi;
      break;
    case x86_gpr_di:
      mreg = x86::edi;
      break;
    default:
      break;
  }

  return Absloc(mreg);;
}

Absloc SymEvalPolicy::convert(X86SegmentRegister r)
{
  MachRegister mreg;
  switch (r) {
    case x86_segreg_es:
      mreg = x86::es;
      break;
    case x86_segreg_cs:
      mreg = x86::cs;
      break;
    case x86_segreg_ss:
      mreg = x86::ss;
      break;
    case x86_segreg_ds:
      mreg = x86::ds;
      break;
    case x86_segreg_fs:
      mreg = x86::fs;
      break;
    case x86_segreg_gs:
      mreg = x86::gs;
      break;
    default:
      break;
  }

  return Absloc(mreg);
}

Absloc SymEvalPolicy::convert(X86Flag f)
{
  switch (f) {
    case x86_flag_cf:
      return Absloc(x86::cf);
    case x86_flag_pf:
      return Absloc(x86::pf);
    case x86_flag_af:
      return Absloc(x86::af);
    case x86_flag_zf:
      return Absloc(x86::zf);
    case x86_flag_sf:
      return Absloc(x86::sf);
    case x86_flag_tf:
      return Absloc(x86::tf);
    case x86_flag_if:
      return Absloc(x86::if_);
    case x86_flag_df:
      return Absloc(x86::df);
    case x86_flag_of:
      return Absloc(x86::of);
    case x86_flag_nt:
    default:
      assert(0);
      return Absloc();
  }
}

std::ostream &operator<<(std::ostream &os, const ROSEOperation &o) {
  os << o.format();
  return os;
}

std::ostream &operator<<(std::ostream &os, const Constant &o) {
  os << o.format();
  return os;
}


std::ostream &operator<<(std::ostream &os, const Variable &v) {
  os << v.format();
  return os;
}

