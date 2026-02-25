#include "addressSpace.h"
#include "ast_helpers.h"
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_collections.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "codegen.h"
#include "debug.h"
#include "instPoint.h"
#include "memoryAccessAST.h"
#include "registerSpace.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

memoryAccessAST::memoryAccessAST(memoryType mem, unsigned which, int size_) : mem_(mem), which_(which) {

  assert(BPatch::bpatch != NULL);
  assert(BPatch::bpatch->stdTypes != NULL);

  switch(mem_) {
    case memoryType::EffectiveAddr:
      switch(size_) {
        case 1:
          bptype = BPatch::bpatch->stdTypes->findType("char");
          break;
        case 2:
          bptype = BPatch::bpatch->stdTypes->findType("short");
          break;
        case 4:
          bptype = BPatch::bpatch->stdTypes->findType("int");
          break;
        default:
          bptype = BPatch::bpatch->stdTypes->findType("long");
      }
      break;
    case memoryType::BytesAccessed:
      bptype = BPatch::bpatch->stdTypes->findType("int");
      break;
    default:
      assert(!"Naah...");
  }
  size = bptype->getSize();
  doTypeCheck = BPatch::bpatch->isTypeChecked();
}

bool memoryAccessAST::generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                                        Dyninst::Register &retReg) {

  RETURN_KEPT_REG(retReg);

  const BPatch_memoryAccess *ma;
  const BPatch_addrSpec_NP *start;
  const BPatch_countSpec_NP *count;
  if(retReg == Dyninst::Null_Register) {
    retReg = allocateAndKeep(gen, noCost);
  }
  switch(mem_) {
    case memoryType::EffectiveAddr: {

      // VG(11/05/01): get effective address
      // VG(07/31/02): take care which one
      // 1. get the point being instrumented & memory access info
      assert(gen.point());

      BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
      BPatch_point *bpoint = bproc->findOrCreateBPPoint(
          NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));
      if(bpoint == NULL) {
        ast_printf("ERROR: Unable to find BPatch point for internal point %p/0x%lx\n",
                   (void *)gen.point(), gen.point()->insnAddr());
      }
      assert(bpoint);
      ma = bpoint->getMemoryAccess();
      if(!ma) {
        ast_printf("Memory access information not available at this point.\n");
        ast_printf("Make sure you create the point in a way that generates it.\n");
        ast_printf("E.g.: findPoint(const std::set<BPatch_opCode>& ops).\n");
        assert(0);
      }
      if(which_ >= ma->getNumberOfAccesses()) {
        ast_printf("Attempt to instrument non-existent memory access number.\n");
        ast_printf("Consider using filterPoints()...\n");
        assert(0);
      }
      start = ma->getStartAddr(which_);
      emitASload(start, retReg, 0, gen, noCost);
      break;
    }
    case memoryType::BytesAccessed: {
      // 1. get the point being instrumented & memory access info
      assert(gen.point());

      BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
      BPatch_point *bpoint = bproc->findOrCreateBPPoint(
          NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));
      ma = bpoint->getMemoryAccess();
      if(!ma) {
        ast_printf("Memory access information not available at this point.\n");
        ast_printf("Make sure you create the point in a way that generates it.\n");
        ast_printf("E.g.: findPoint(const std::set<BPatch_opCode>& ops).\n");
        assert(0);
      }
      if(which_ >= ma->getNumberOfAccesses()) {
        ast_printf("Attempt to instrument non-existent memory access number.\n");
        ast_printf("Consider using filterPoints()...\n");
        assert(0);
      }
      count = ma->getByteCount(which_);
      emitCSload(count, retReg, gen, noCost);
      break;
    }
    default:
      assert(0);
  }
  decUseCount(gen);
  return true;
}

std::string memoryAccessAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Mem/" << std::hex << this << "("
      << ((mem_ == memoryType::EffectiveAddr) ? "EffAddr" : "BytesAcc") << ")\n";

  return ret.str();
}

}}
