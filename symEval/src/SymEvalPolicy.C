#include "SymEvalPolicy.h"

#include "dyn_regs.h"

using namespace Dyninst;
using namespace Dyninst::SymbolicEvaluation;
using namespace Dyninst::InstructionAPI;

SymEvalPolicy::SymEvalPolicy(SymEval::Result &r, Architecture a) :
  res(r),
  arch(a) {
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
  MachRegister m;
  switch (r) {
    case x86_gpr_ax:
      m = x86::eax;
      break;
    case x86_gpr_cx:
      m = x86::ecx;
      break;
    case x86_gpr_dx:
      m = x86::edx;
      break;
    case x86_gpr_bx:
      m = x86::ebx;
      break;
    case x86_gpr_sp:
      m = x86::esp;
      break;
    case x86_gpr_bp:
      m = x86::ebp;
      break;
    case x86_gpr_si:
      m = x86::esi;
      break;
    case x86_gpr_di:
      m = x86::edi;
      break;
    default:
      assert(0);
      break;
  }
  return Absloc(m);
}

Absloc SymEvalPolicy::convert(X86SegmentRegister r)
{
  MachRegister m;
  switch (r) {
    case x86_segreg_es:
      m = x86::es;
      break;
    case x86_segreg_cs:
      m = x86::cs;
      break;
    case x86_segreg_ss:
      m = x86::ss;
      break;
    case x86_segreg_ds:
      m = x86::ds;
      break;
    case x86_segreg_fs:
      m = x86::fs;
      break;
    case x86_segreg_gs:
      m = x86::gs;
      break;
    default:
      assert(0);
      break;
  }
  return Absloc(m);
}

Absloc SymEvalPolicy::convert(X86Flag f)
{
  return Absloc(x86::flags);
#if 0
  int id;
  switch (f) {
    case x86_flag_cf:
      m = x86::CF;
      break;
    case x86_flag_pf:
      m = x86::PF;
      break;
    case x86_flag_af:
      m = x86::AF;
      break;
    case x86_flag_zf:
      m = x86::ZF;
      break;
    case x86_flag_sf:
      m = x86::SF;
      break;
    case x86_flag_tf:
      m = x86::TF;
      break;
    case x86_flag_if:
      m = x86::IF;
      break;
    case x86_flag_df:
      m = x86::DF;
      break;
    case x86_flag_of:
      m = x86::OF;
      break;
    case x86_flag_nt:
      m = x86::NT;
      break;
    case x86_flag_rf:
      m = x86::RF;
      break;
    default:
      std::cerr << "Failed to find flag " << f << std::endl;
      assert(0);
      id = 0; // error
  }
#endif
}

