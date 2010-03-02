#include "SymEvalPolicy.h"

using namespace Dyninst;
using namespace Dyninst::SymbolicEvaluation;
using namespace Dyninst::InstructionAPI;

SymEvalPolicy::SymEvalPolicy(SymEval::Result &r) :
  res(r) {
  // We also need to build aaMap FTW!!!
  for (SymEval::Result::iterator iter = r.begin();
       iter != r.end(); ++iter) {
    Assignment::Ptr a = iter->first;
    AbsRegion &o = a->out();

    if (o.containsOfType(Absloc::Register)) {
      // We're assuming this is a single register...
      for (std::set<Absloc>::const_iterator a_iter = o.abslocs().begin();
	   a_iter != o.abslocs().end(); ++a_iter) {
	aaMap[*a_iter] = a;
      }
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
  return Absloc(x86::flags);
  
  switch (f) {
    case x86_flag_cf:
      break;
    case x86_flag_pf:
      break;
    case x86_flag_af:
      break;
    case x86_flag_zf:
      break;
    case x86_flag_sf:
      break;
    case x86_flag_tf:
      break;
    case x86_flag_if:
      break;
    case x86_flag_df:
      break;
    case x86_flag_of:
      break;
    case x86_flag_nt:
      break;
    case x86_flag_rf:
      break;
    default:
      std::cerr << "Failed to find flag " << f << std::endl;
      assert(0);
  }

  
}

