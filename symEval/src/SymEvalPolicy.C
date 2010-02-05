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
      aaMap[Absloc(Absloc::Heap, 0)] = a;
    }
  }
}
  

void SymEvalPolicy::startInstruction(SgAsmx86Instruction *) {
}

void SymEvalPolicy::finishInstruction(SgAsmx86Instruction *) {
}

Absloc SymEvalPolicy::convert(X86GeneralPurposeRegister r)
{
  int id;
  switch (r) {
    case x86_gpr_ax:
      id = r_EAX;
      break;
    case x86_gpr_cx:
      id = r_ECX;
      break;
    case x86_gpr_dx:
      id = r_EDX;
      break;
    case x86_gpr_bx:
      id = r_EBX;
      break;
    case x86_gpr_sp:
      id = r_ESP;
      break;
    case x86_gpr_bp:
      id = r_EBP;
      break;
    case x86_gpr_si:
      id = r_ESI;
      break;
    case x86_gpr_di:
      id = r_EDI;
      break;
    default:
      id = 0; // error
  }

  return Absloc(Absloc::Register, id);;
}

Absloc SymEvalPolicy::convert(X86SegmentRegister r)
{
  int id;
  switch (r) {
    case x86_segreg_es:
      id = r_ES;
      break;
    case x86_segreg_cs:
      id = r_CS;
      break;
    case x86_segreg_ss:
      id = r_SS;
      break;
    case x86_segreg_ds:
      id = r_DS;
      break;
    case x86_segreg_fs:
      id = r_FS;
      break;
    case x86_segreg_gs:
      id = r_GS;
      break;
    default:
      id = 0; //error
  }

  return Absloc(Absloc::Register, id);
}

Absloc SymEvalPolicy::convert(X86Flag f)
{
  int id;
  switch (f) {
    case x86_flag_cf:
      id = r_CF;
      break;
    case x86_flag_pf:
      id = r_PF;
      break;
    case x86_flag_af:
      id = r_AF;
      break;
    case x86_flag_zf:
      id = r_ZF;
      break;
    case x86_flag_sf:
      id = r_SF;
      break;
    case x86_flag_tf:
      id = r_TF;
      break;
    case x86_flag_if:
      id = r_IF;
      break;
    case x86_flag_df:
      id = r_DF;
      break;
    case x86_flag_of:
      id = r_OF;
      break;
    case x86_flag_nt:
      id = r_NT;
      break;
    case x86_flag_rf:
      id = r_RF;
      break;
    default:
      std::cerr << "Failed to find flag " << f << std::endl;
      assert(0);
      id = 0; // error
  }

  return Absloc(Absloc::Register, id);
}

