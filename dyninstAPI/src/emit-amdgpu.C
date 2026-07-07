/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * emit-amdgpu.C - AMD GPU mi{25,50,100,200,300}  code generators (emitters)
 */

#include "codegen/RegControl.h"
#include "dyninstAPI/src/emit-amdgpu.h"
#include "registerSpace/registerSpace.h"
#include "arch-amdgpu.h"
#include "patching/function.h"

// For bumpCallerKdForCallee: reading/writing a kernel descriptor and reading a
// callee's exported register-usage symbols.
#include "mapped_object.h"
#include "addressSpace.h"
#include "Symtab.h"
#include "Symbol.h"
#include "AmdgpuKernelDescriptor.h"
#include "external/amdgpu/AMDGPUEFlags.h"
#include "amdgpu-scratch-abi.h"
#include "amdgpu-abi-sgpr.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

using namespace Dyninst;
using namespace AmdgpuGfx908;

// ===== EmitterAmdgpuGfx908 implementation begin =====

// ==== Helper functions begin
bool EmitterAmdgpuGfx908::isValidSgpr(Register reg) const {
  return reg.isScalar() &&
         reg.getCount() == 1 && reg.getId() >= NS_amdgpu::MIN_SGPR_ID &&
         reg.getId() <= NS_amdgpu::MAX_SGPR_ID;
}

bool EmitterAmdgpuGfx908::isValidSgprBlock(Register regBlock) const {
  auto firstRegId = regBlock.getId();
  auto numRegs = regBlock.getCount();

  if (numRegs <= 1)
    return false;

  auto lastRegId = firstRegId + numRegs - 1;

  return regBlock.isScalar() &&
         firstRegId >= MIN_SGPR_ID && lastRegId <= MAX_SGPR_ID;
}

// Register pairs must be even aligned
bool EmitterAmdgpuGfx908::isValidSgprPair(Register regBlock) const {
  return isValidSgprBlock(regBlock) && regBlock.getCount() == 2 && regBlock.getId() % 2 == 0;
}
// ==== Helper functions end

unsigned EmitterAmdgpuGfx908::emitIf(Register expr_reg, Register target, Dyninst::DyninstAPI::RegControl /* rc */,
                                     codeGen &gen) {
  assert(isValidSgprPair(target) && "target must be a valid SGPR pair");
  assert(isValidSgpr(expr_reg) && "expr_reg must be a valid SGPR");

  emitSopK(S_CMPK_EQ_U32, expr_reg.getId(), 0, gen);
  emitConditionalBranch(/* onConditionTrue = */ false, 0, gen);

  size_t setPcInstOffset = gen.getIndex();
  emitSop1(S_SETPC_B64, /* dest= */ 0, target.getId(), /* hasLiteral= */ false, 0, gen);

  return setPcInstOffset;
}

// EmitterAmdgpuGfx908 implementation
void EmitterAmdgpuGfx908::emitOp(unsigned opcode, Register dest, Register src1, Register src2,
                                 codeGen &gen) {
  // TODO: We eventually want to get rid of this assert and generate code based on operand kind.
  assert(isValidSgpr(dest) && isValidSgpr(src1) && isValidSgpr(src2) && "each operand must be a valid SGPR");

  uint32_t opcodeSop2 = 0;
  switch (opcode) {
  case plusOp:
    opcodeSop2 = S_ADD_I32;
    break;

  case minusOp:
    opcodeSop2 = S_SUB_I32;
    break;

  case timesOp:
    opcodeSop2 = S_MUL_I32;
    break;

  case divOp:
    assert(!"opcode must correspond to a supported SOP2 operation");
    break;

  case andOp:
    opcodeSop2 = S_AND_B32;
    break;

  case orOp:
    opcodeSop2 = S_OR_B32;
    break;

  case xorOp:
    opcodeSop2 = S_XOR_B32;
    break;

  default:
    assert(!"opcode must correspond to a supported SOP2 operation");
  }

  emitSop2(opcodeSop2, dest.getId(), src1.getId(), src2.getId(), gen);
}

void EmitterAmdgpuGfx908::emitOpImmSimple(unsigned op, Register dest, Register src1,
                                          RegValue src2imm, codeGen &gen) {

  assert(isValidSgpr(dest) && isValidSgpr(src1) && "dest and src1 must be valid SGPRs");
  assert(dest == src1 && "dest and src1 must be the same for SOPK");

  uint32_t opcodeSopK = 0;
  switch (op) {
  case plusOp:
    opcodeSopK = S_ADDK_I32;
    break;
  case timesOp:
    opcodeSopK = S_MULK_I32;
    break;
  case lessOp:
    opcodeSopK = S_CMPK_LT_I32;
    break;
  case leOp:
    opcodeSopK = S_CMPK_LE_I32;
    break;
  case greaterOp:
    opcodeSopK = S_CMPK_GT_I32;
    break;
  case geOp:
    opcodeSopK = S_CMPK_GE_I32;
    break;
  case eqOp:
    opcodeSopK = S_CMPK_EQ_I32;
    break;
  case neOp:
    opcodeSopK = S_CMPK_LG_I32;
    break;
  default:
    assert(!"opcode must correspond to a supported SOPK operation");
  }

  emitSopK(opcodeSopK, src1.getId(), src2imm, gen);
}

void EmitterAmdgpuGfx908::emitOpImm(unsigned /* opcode1 */, unsigned /* opcode2 */,
                                    Register /* dest */, Register /* src1 */,
                                    RegValue /* src2imm */, codeGen & /* gen */) {
  assert(!"not implemented yet");
}

void EmitterAmdgpuGfx908::emitRelOp(unsigned opcode, Register /* dest */, Register src1,
                                    Register src2, codeGen &gen, bool /* s */) {
  assert(isValidSgpr(src1) && isValidSgpr(src1) && "src1 and src2 must be valid SGPRs");

  uint32_t opcodeSopC = 0;
  switch (opcode) {
  case lessOp:
    opcodeSopC = S_CMP_LT_I32;
    break;

  case leOp:
    opcodeSopC = S_CMP_LE_I32;
    break;

  case greaterOp:
    opcodeSopC = S_CMP_GT_I32;
    break;

  case geOp:
    opcodeSopC = S_CMP_GE_I32;
    break;

  case eqOp:
    opcodeSopC = S_CMP_EQ_I32;
    break;

  case neOp:
    opcodeSopC = S_CMP_LG_I32;
    break;

  default:
    assert(!"opcode must correspond to a supported SOPC operation");
  }

  emitSopC(opcodeSopC, src1.getId(), src2.getId(), gen);
}

void EmitterAmdgpuGfx908::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
                                       codeGen &gen, bool /* s */) {
  switch (op) {
  case lessOp:
  case leOp:
  case greaterOp:
  case geOp:
  case eqOp:
  case neOp:
    emitOpImmSimple(op, dest, src1, src2imm, gen);
    break;
  default:
    assert(!"opcode must correspond to a supported SOPK relational operation");
  }
}

void EmitterAmdgpuGfx908::emitDiv(Register /* dest */, Register /* src1 */, Register /* src2 */,
                                  codeGen & /* gen */, bool /* s */) {
  assert(!"emitDiv not implemented yet");
}

void EmitterAmdgpuGfx908::emitTimesImm(Register dest, Register src1, RegValue src2imm,
                                       codeGen &gen) {
  assert(isValidSgpr(dest) && "dest must be a valid SGPR");
  assert(dest == src1 && "SOPK instructions require dest = src1");

  AmdgpuGfx908::emitSopK(AmdgpuGfx908::S_MULK_I32, dest.getId(), src2imm, gen);
}

void EmitterAmdgpuGfx908::emitDivImm(Register /* dest */, Register /* src1 */,
                                     RegValue /* src2imm */, codeGen & /* gen */, bool /* s */) {
  assert(!"emitDivImm not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoad(Register /* dest */, Address /* addr */, int /* size */,
                                   codeGen & /* gen */) {
  assert(!"emitLoad not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadConst(Register dest, Address imm, codeGen &gen) {
  // Caller must ensure that dest is a SGPR pair beginning at an even reg id.
  assert(isValidSgprPair(dest) && "dest must be a valid SGPR pair");
  assert(sizeof(Address) == 8); // must be a 64-bit address

  uint32_t lowerAddress = imm;
  uint32_t upperAddress = (imm >> 32);

  std::vector<Register> regs = dest.getIndividualRegisters();

  emitMovLiteral(regs[0], lowerAddress, gen);
  emitMovLiteral(regs[1], upperAddress, gen);
}

void EmitterAmdgpuGfx908::emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen) {
  emitLoadRelative(dest, /* offset =*/0, addr_reg, size, gen);
}

bool EmitterAmdgpuGfx908::emitCallRelative(Register, Address, Register, codeGen &) {
  assert(!"emitCallRelative not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitLoadRelative(Register dest, Address offset, Register base, int size,
                                           codeGen &gen) {
  // Caller must ensure the following:
  //
  // 1. base is even aligned and base, base + 1 contain the address.
  // 2. <size> registers starting from dest are available.
  // 3. offset must be a 21-bit signed value. Sign-extend to int64_t and cast to
  // uint64_t before calling if necessary (this is bad).
  // 3. Alignment requirement for dest:
  //    size = 1  : 1
  //    size = 2  : 2
  //    size >= 4 : 4
  // size = 1, 2, 4, 8, 16

  assert(size == 1 || size == 2 || size == 4 || size == 8 || size == 16);

  if (size == 1) {
    assert(isValidSgpr(dest) && "dest must be a valid SGPR");
  } else {
    assert(isValidSgprBlock(dest) && dest.getCount() == static_cast<uint32_t>(size) &&
           "dest must be a register block of size 'size'");
  }
  uint32_t alignment = size >= 4 ? 4 : size;
  assert(dest.getId() % alignment == 0 && "dest must be properly aligned");

  assert(isValidSgprPair(base) && "base must be a valid SGPR pair");

  unsigned loadOpcode = 0;
  switch (size) {
  case 1:
    loadOpcode = S_LOAD_DWORD;
    break;
  case 2:
    loadOpcode = S_LOAD_DWORDX2;
    break;
  case 4:
    loadOpcode = S_LOAD_DWORDX4;
    break;
  case 8:
    loadOpcode = S_LOAD_DWORDX8;
    break;
  case 16:
    loadOpcode = S_LOAD_DWORDX16;
    break;
  default:
    assert(!"size can only be 1, 2, 4, 8 or 16");
  }

  emitSmem(loadOpcode, dest.getId(), (base.getId() >> 1), (uint64_t)offset, gen);

  // As per page 32 in the manual, 0 is the only legitimate value for scalar
  // memory reads. However, placement of waitcnt can be optimized later. Right
  // now set all counters to 0.
  //
  // TODO:
  // 1. Only set LGKM_CNT (simm16[11:8]) to 0 specifically.
  // 2. Optimize placement of waitcnt.
  emitSopP(S_WAITCNT, /* simm16 = */ 0, gen);

  return false;
}

void EmitterAmdgpuGfx908::emitLoadShared(opCode /* op */, Register /* dest */,
                                         const image_variable * /* var */,
                                         bool
                                         /* is_local */,
                                         int /* size */, codeGen & /* gen */,
                                         Address /* offset */) {
  assert(!"emitLoadShared not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadFrameAddr(Register /* dest */, Address /* offset */,
                                            codeGen & /* gen */) {
  assert(!"emitLoadFrameAddr not implemented yet");
}

// These implicitly use the stored original/non-inst value
void EmitterAmdgpuGfx908::emitLoadOrigFrameRelative(Register /* dest */, Address /* offset */,
                                                    codeGen & /* gen */) {
  assert(!"emitLoadOrigFrameRelative not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadOrigRegRelative(Register /* dest */, Address /* offset */,
                                                  Register /* base */, codeGen & /* gen */,
                                                  bool /* store */) {
  assert(!"emitLoadOrigRegRelative not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadOrigRegister(Address /* register_num */, Register /* dest */,
                                               codeGen & /* gen */) {
  assert(!"emitLoadOrigRegister not implemented yet");
}

void EmitterAmdgpuGfx908::emitStoreOrigRegister(Address /* register_num */, Register /* dest */,
                                                codeGen & /* gen */) {
  assert(!"emitStoreOrigRegister not implemented yet");
}

void EmitterAmdgpuGfx908::emitStore(Address /* addr */, Register /* src */, int /* size */,
                                    codeGen & /* gen */) {
  assert(!"emitStore not implemented yet");
}

void EmitterAmdgpuGfx908::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen) {
  emitStoreRelative(src, /*offset =*/0, addr_reg, size, gen);
}

void EmitterAmdgpuGfx908::emitStoreFrameRelative(Address /* offset */, Register /* src */,
                                                 Register /* scratch */, int /* size */,
                                                 codeGen & /* gen */) {
  assert(!"emitStoreFrameRelative not implemented yet");
}

void EmitterAmdgpuGfx908::emitStoreRelative(Register source, Address offset, Register base,
                                            int size, codeGen &gen) {
  // Caller must ensure the following:
  //
  // 1. base is even aligned and base, base + 1 contain the address.
  // 2. <size> registers starting from source hold the value to be stored.
  // 3. offset must be a 21-bit signed value. Sign-extend to int64_t and cast to
  // uint64_t before calling if necessary (this is bad).
  // 3. Alignment requirement for source:
  //     alignment = size
  //     size = 1, 2, 4

  assert(size == 1 || size == 2 || size == 4);

  if (size == 1)
    assert(isValidSgpr(source) && "source must be a valid SGPR");
  else
    assert(isValidSgprBlock(source) && source.getCount() == static_cast<uint32_t>(size) &&
           "source must be a register block of size 'size'");

  assert(isValidSgprPair(base) && "base must be a valid SGPR pair");

  uint32_t alignment = static_cast<uint32_t>(size);
  assert(source.getId() % alignment == 0 && "source must be properly aligned");

  unsigned storeOpcode = 0;
  switch (size) {
  case 1:
    storeOpcode = S_STORE_DWORD;
    break;
  case 2:
    storeOpcode = S_STORE_DWORDX2;
    break;
  case 4:
    storeOpcode = S_STORE_DWORDX4;
    break;
  default:
    assert(!"size can only be 1, 2, or 4");
  }

  emitSmem(storeOpcode, source.getId(), (base.getId() >> 1), (uint64_t)offset, gen);

  emitSopP(S_WAITCNT, /* simm16 = */ 0, gen);
}

void EmitterAmdgpuGfx908::emitStoreShared(Register /* source */, const image_variable * /* var */,
                                          bool /* is_local */, int /* size */,
                                          codeGen & /* gen */) {
  assert(!"emitStoreShared not implemented yet");
}

bool EmitterAmdgpuGfx908::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
  // TODO:
  // Make this work with VGPR-VGPR movs

  bool bothSingleSgprs = isValidSgpr(src) && isValidSgpr(dest);
  bool bothSgprBlocks = isValidSgprBlock(src) && isValidSgprBlock(dest);
  bool bothHaveSameCount = src.getCount() == dest.getCount();

  assert((bothSingleSgprs || bothSgprBlocks) && bothHaveSameCount &&
         "src and dest must be valid sgprs or sgpr blocks of same size");

  assert(std::abs(static_cast<int64_t>(src.getId() - dest.getId())) >= src.getCount() &&
         "src and dest must not overlap");

  std::vector<Register> srcRegisters = src.getIndividualRegisters();
  std::vector<Register> destRegisters = dest.getIndividualRegisters();

  assert(srcRegisters.size() == destRegisters.size());
  for (size_t i = 0; i < srcRegisters.size(); ++i) {
    emitSop1(S_MOV_B32, destRegisters[i].getId(), srcRegisters[i].getId(),
             /*hasLiteral*/ false, /*literal*/ 0, gen);
  }

  return false;
}

bool EmitterAmdgpuGfx908::emitMoveRegToReg(registerSlot * /* src */, registerSlot * /* dest */,
                                           codeGen & /* gen */) {
  assert(!"emitMoveRegToReg -- slot not implemented yet");
  return false;
}

// Live granted-VGPR reader (reads the possibly-mutated .kd); defined below.
static uint32_t readCallerGrantedVgpr(func_instance *caller, codeGen &gen);
// Whole-object max of ".dyninst.*.<key>" (transitive footprint); defined below.
static uint32_t readCalleeMaxAbsSym(func_instance *callee, const char *key);

// bumpCallerKdForCallee MUTATES the caller's .kd VGPR grant. Multiple inserted
// calls into the same kernel (hc_open/hc_write/hc_close) would each read the
// already-raised grant and raise it again — the grant ratchets up to gran 31
// (128 VGPRs) and the spill scratch register (vAddr) drifts call-to-call.
// To keep the bump idempotent and vAddr stable, cache each caller's ORIGINAL
// granted VGPR count the first time we see it, and derive both the bump target
// and vAddr from that fixed value rather than the live (growing) grant.
static std::map<func_instance *, uint32_t> g_origGrantedVgpr;

// Scalars packed per pack-VGPR in the SGPR spill = wavefront width (wave64 = 64
// lanes; one v_writelane per lane). Overridable (clamped to <=64) purely to force
// numPacks>1 for testing the multi-pack path with a small callee; production = 64.
static uint32_t spillPackLanes() {
  const char *e = getenv("DYNINST_PACK_LANES");
  if (!e) return 64;
  uint32_t v = (uint32_t)atoi(e);
  return (v == 0 || v > 64) ? 64 : v;
}
static uint32_t readCallerOriginalGrantedVgpr(func_instance *caller, codeGen &gen) {
  auto it = g_origGrantedVgpr.find(caller);
  if (it != g_origGrantedVgpr.end())
    return it->second;
  uint32_t v = readCallerGrantedVgpr(caller, gen);
  g_origGrantedVgpr[caller] = v;
  return v;
}

// When we splice an inter-module call into a kernel, the callee (a separately
// compiled device function) may use more registers/scratch than the caller
// kernel's descriptor granted for its own code. The wave's register allocation
// is fixed by the caller's kernel descriptor, so the callee must fit inside it.
//
// SGPR is already handled: AmdgpuGfx908PointHandler::maximizeSgprAllocationIfKernel
// maxes the SGPR grant for every instrumented kernel. Here we bump the VGPR grant
// PRECISELY (to preserve occupancy) using the callee's exported register-usage
// symbols, and reserve scratch only if the callee actually uses it.
//
// The callee's footprint is read from SHN_ABS symbols ".dyninst.<callee>.num_vgpr"
// / ".dyninst.<callee>.private_seg_size" whose VALUE is the compiler's
// post-register-allocation count (injected by add_object_aliases.py --asm).
static void bumpCallerKdForCallee(func_instance *caller, func_instance *callee,
                                  codeGen &gen) {
  if (!caller || !callee)
    return;

  AddressSpace *aSpace = gen.addrSpace();
  mapped_object *callerObj = caller->obj();
  mapped_object *calleeObj = callee->obj();
  if (!aSpace || !callerObj || !calleeObj)
    return;

  // Only kernels have a descriptor "<mangled>.kd". If it's absent, the caller is
  // an ordinary device function and there is no descriptor to size.
  int_symbol kdSym;
  if (!callerObj->getSymbolInfo(caller->symTabName() + ".kd", kdSym))
    return;

  SymtabAPI::Symtab *calleeSymtab = calleeObj->parse_img()->getObject();
  if (!calleeSymtab)
    return;

  const std::string prefix = ".dyninst." + callee->symTabName() + ".";
  auto readAbs = [&](const std::string &key, uint32_t &out) -> bool {
    std::vector<SymtabAPI::Symbol *> syms;
    if (!calleeSymtab->findSymbol(syms, prefix + key) || syms.empty())
      return false;
    out = static_cast<uint32_t>(syms[0]->getOffset());
    return true;
  };

  uint32_t calleeVgpr = 0;
  if (!readAbs("num_vgpr", calleeVgpr)) {
    fprintf(stderr,
            "[amdgpu] warning: no '.dyninst.%s.num_vgpr' symbol for callee; cannot "
            "size caller '%s' kernel-descriptor VGPR grant — leaving as-is\n",
            callee->symTabName().c_str(), caller->symTabName().c_str());
    return;
  }
  // Grant must cover the callee's TRANSITIVE footprint (wrapper + everything it
  // calls), so use the whole-object max — same set the VGPR save/restore in
  // emitCall preserves.
  { uint32_t m = readCalleeMaxAbsSym(callee, "num_vgpr"); if (m > calleeVgpr) calleeVgpr = m; }
  uint32_t calleeScratch = 0;
  readAbs("private_seg_size", calleeScratch);  // optional; absent => 0
  { uint32_t m = readCalleeMaxAbsSym(callee, "private_seg_size"); if (m > calleeScratch) calleeScratch = m; }

  // Read the caller's kernel descriptor from the binary being rewritten.
  const Address kdAddr = kdSym.getAddr();
  const size_t kdSize = sizeof(llvm::amdhsa::kernel_descriptor_t);
  uint8_t kdBytes[sizeof(llvm::amdhsa::kernel_descriptor_t)];
  if (!aSpace->readDataSpace(reinterpret_cast<const void *>(kdAddr),
                             static_cast<u_int>(kdSize), kdBytes, true)) {
    fprintf(stderr, "[amdgpu] warning: failed to read kernel descriptor for '%s'\n",
            caller->symTabName().c_str());
    return;
  }

  Dyninst::AmdgpuKernelDescriptor kd(kdBytes, kdSize, EF_AMDGPU_MACH_AMDGCN_GFX908);

  // 0.4: the SGPR spill packs {calleeSgpr numbered SGPRs, VCC_lo, VCC_hi} one-per-
  // lane across ceil((calleeSgpr+2)/64) pack VGPRs (must match numPacks in emitCall).
  // The spill ALWAYS runs (VCC alone needs a pack even if calleeSgpr==0).
  const uint32_t calleeSgpr = readCalleeMaxAbsSym(callee, "numbered_sgpr");
  const uint32_t packLanes  = spillPackLanes();   // must match emitCall
  const uint32_t numPacks   = (calleeSgpr + 2u + packLanes - 1u) / packLanes;
  const bool useScratch     = (getenv("DYNINST_SPILL_SCRATCH") != nullptr);

  // VGPR: gfx9/wave64 granule = 4; granted = (granulated + 1) * 4. To fit N regs,
  // granulated = ceil(N/4) - 1. Only ever raise. The grant must cover the callee's
  // and kernel's VGPRs plus the spill's scratch VGPRs: numPacks SGPR-pack regs, and
  // (global path only) the vAddr lane-offset reg (the scratch path reclaims it).
  {
    // Use the kernel's ORIGINAL granted VGPR count (cached), NOT the live grant read
    // from kdBytes — otherwise each inserted call reads the previously-raised grant
    // and ratchets the count upward.
    const uint32_t curVgpr = readCallerOriginalGrantedVgpr(caller, gen);
    const uint32_t vAddr   = (curVgpr > calleeVgpr ? curVgpr : calleeVgpr);
    const uint32_t needVgpr = vAddr + (useScratch ? numPacks : 1u + numPacks);
    const uint32_t neededGran = (needVgpr + 3u) / 4u - 1u;
    if (neededGran > kd.getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount())
      kd.setCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount(neededGran);

    // 0.3: size the scratch spill slot to the footprint = numPacks SGPR-pack dwords +
    // calleeVgpr VGPR dwords = 4*(numPacks+nvgpr) bytes/lane. The hardcoded 256 in
    // enableScratchInKD overflows for large footprints -> scratch aperture violation.
    if (useScratch) {
      const uint32_t spillSlot = 4u * (numPacks + calleeVgpr);
      if (spillSlot > kd.getPrivateSegmentFixedSize())
        kd.setPrivateSegmentFixedSize(spillSlot);
    }
  }

  // Scratch: reserve only if the callee actually uses private/scratch memory.
  if (calleeScratch > 0) {
    if (calleeScratch > kd.getPrivateSegmentFixedSize())
      kd.setPrivateSegmentFixedSize(calleeScratch);
    kd.setKernelCodeProperty_EnableSgprFlatScratchInit(true);
    fprintf(stderr,
            "[amdgpu] warning: callee '%s' uses %u bytes of scratch; caller '%s' "
            "ALSO needs call-ABI prologue setup (FLAT_SCRATCH/s32/scratch "
            "descriptor) which is NOT emitted here — scratch-using callees are not "
            "yet fully supported.\n",
            callee->symTabName().c_str(), calleeScratch, caller->symTabName().c_str());
  }

  // Write the modified descriptor back.
  kd.writeToMemory(kdBytes);
  if (!aSpace->writeDataSpace(reinterpret_cast<void *>(kdAddr),
                             static_cast<u_int>(kdSize), kdBytes)) {
    fprintf(stderr, "[amdgpu] warning: failed to write kernel descriptor for '%s'\n",
            caller->symTabName().c_str());
  }
}

// Read a callee's exported ".dyninst.<callee>.<key>" SHN_ABS value (the
// compiler's per-function register footprint, injected by add_object_aliases.py).
// Returns 0 if absent.
static uint32_t readCalleeAbsSym(func_instance *callee, const char *key) {
  if (!callee || !callee->obj() || !callee->obj()->parse_img())
    return 0;
  SymtabAPI::Symtab *st = callee->obj()->parse_img()->getObject();
  if (!st)
    return 0;
  std::vector<SymtabAPI::Symbol *> syms;
  std::string name = ".dyninst." + callee->symTabName() + "." + key;
  if (!st->findSymbol(syms, name) || syms.empty())
    return 0;
  return static_cast<uint32_t>(syms[0]->getOffset());
}

// Largest ".dyninst.*.<key>" value across ALL functions in the callee's object.
//
// A dyninst-inserted call transfers into the callee wrapper, which may itself
// make ordinary (non-inlined) calls into other functions in the SAME library
// (e.g. hc_write -> gpu_fwrite). Each nested call clobbers its own register
// frame, so the wrapper's *own* ".dyninst.<callee>.num_vgpr" undercounts the
// registers actually destroyed by the whole call chain — leaving live caller
// registers above the wrapper's frame unsaved (observed: the kernel's v[4:5]
// address pair trashed by gpu_fwrite, faulting on the next load). The per-call
// save/restore set is independent per call site, so accumulation across other
// sites can't fix it — this one site must save everything it transitively
// clobbers. We don't do call-graph analysis; conservatively take the maximum
// footprint over EVERY function in the callee's address space, which upper-bounds
// any function the inserted call could transitively reach. Returns 0 if none.
static uint32_t readCalleeMaxAbsSym(func_instance *callee, const char *key) {
  if (!callee || !callee->obj() || !callee->obj()->parse_img())
    return 0;
  SymtabAPI::Symtab *st = callee->obj()->parse_img()->getObject();
  if (!st)
    return 0;
  std::vector<SymtabAPI::Symbol *> all;
  if (!st->getAllSymbols(all))
    return 0;
  const std::string prefix = ".dyninst.";
  const std::string suffix = std::string(".") + key;
  uint32_t maxVal = 0;
  for (SymtabAPI::Symbol *s : all) {
    if (!s) continue;
    const std::string &nm = s->getMangledName();
    if (nm.size() > prefix.size() + suffix.size() &&
        nm.compare(0, prefix.size(), prefix) == 0 &&
        nm.compare(nm.size() - suffix.size(), suffix.size(), suffix) == 0) {
      uint32_t v = static_cast<uint32_t>(s->getOffset());
      if (v > maxVal) maxVal = v;
    }
  }
  return maxVal;
}

// Granted VGPR count of the caller kernel (from its .kd), or 0 if not a kernel.
// Used to place the VGPR-spill scratch register above the kernel's own usage.
static uint32_t readCallerGrantedVgpr(func_instance *caller, codeGen &gen) {
  AddressSpace *aSpace = gen.addrSpace();
  if (!caller || !caller->obj() || !aSpace)
    return 0;
  int_symbol kdSym;
  if (!caller->obj()->getSymbolInfo(caller->symTabName() + ".kd", kdSym))
    return 0;
  const size_t kdSize = sizeof(llvm::amdhsa::kernel_descriptor_t);
  uint8_t kdBytes[sizeof(llvm::amdhsa::kernel_descriptor_t)];
  if (!aSpace->readDataSpace(reinterpret_cast<const void *>(kdSym.getAddr()),
                             static_cast<u_int>(kdSize), kdBytes, true))
    return 0;
  Dyninst::AmdgpuKernelDescriptor kd(kdBytes, kdSize, EF_AMDGPU_MACH_AMDGCN_GFX908);
  return (kd.getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount() + 1) * 4;
}

// Number of numbered SGPRs granted to the caller kernel (= one past the highest
// allocatable SGPR id). SGPR blocks are 16 wide: granulated = 2*((count/16)-1) =>
// count = ((granulated/2)+1)*16. In scratch mode the reserved spill registers live
// at the TOP of this grant (grantTop-4 .. grantTop-1), so emitCall and
// generateBranch both derive the same reservedBase from it (maximizeSgprAlloc-
// IfKernel sizes the grant for scratch). Returns 0 if the KD can't be read.
static uint32_t readCallerGrantedSgprTop(func_instance *caller, codeGen &gen) {
  AddressSpace *aSpace = gen.addrSpace();
  if (!caller || !caller->obj() || !aSpace)
    return 0;
  int_symbol kdSym;
  if (!caller->obj()->getSymbolInfo(caller->symTabName() + ".kd", kdSym))
    return 0;
  const size_t kdSize = sizeof(llvm::amdhsa::kernel_descriptor_t);
  uint8_t kdBytes[sizeof(llvm::amdhsa::kernel_descriptor_t)];
  if (!aSpace->readDataSpace(reinterpret_cast<const void *>(kdSym.getAddr()),
                             static_cast<u_int>(kdSize), kdBytes, true))
    return 0;
  Dyninst::AmdgpuKernelDescriptor kd(kdBytes, kdSize, EF_AMDGPU_MACH_AMDGCN_GFX908);
  return ((kd.getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount() / 2) + 1) * 16;
}

// ===========================================================================
// gfx908 (CDNA1) ScratchAbi — the per-arch seam for scratch-based register spill
// (see amdgpu-scratch-abi.h). gfx908 is the worst case: manual FLAT_SCRATCH init
// (flat_scratch_init is a user SGPR whose enablement SHIFTS the system SGPRs the
// kernel reads), so enableScratchInKD returns "shifted" and the entry prologue
// relocates them back ("records the register update"). Spills use scratch_*
// (FLAT enc, SEG=1): per-lane swizzle + per-wave base are hardware-provided, so
// no mbcnt lane math or global buffer is needed.
// ===========================================================================
namespace {
// gfx908 special/inline operand encodings.
enum { GFX908_FLAT_SCRATCH_LO = 102, GFX908_FLAT_SCRATCH_HI = 103,
       GFX908_INLINE_0 = 128, GFX908_SADDR_BASE = 94 };

class Gfx908ScratchAbi : public Dyninst::DyninstAPI::ScratchAbi {
public:
  uint32_t waveSize() const override { return 64; }

  bool enableScratchInKD(Dyninst::AmdgpuKernelDescriptor &kd, uint32_t slotBytes) override {
    if (slotBytes > kd.getPrivateSegmentFixedSize())
      kd.setPrivateSegmentFixedSize(slotBytes);
    // Already scratch-enabled (compiler set flat_scratch_init up): reuse its
    // FLAT_SCRATCH, only grew the size, no new shift.
    if (kd.getKernelCodeProperty_EnableSgprFlatScratchInit())
      return false;
    const uint32_t uc = kd.getCOMPUTE_PGM_RSRC2_UserSgprCount();
    kd.setKernelCodeProperty_EnableSgprFlatScratchInit(true);  // +2 user SGPRs at s[uc:uc+1]
    kd.setCOMPUTE_PGM_RSRC2_UserSgprCount(uc + 2);
    kd.setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(true);        // wave-offset system SGPR
    return true;                                               // system SGPRs shifted by +2
  }

  void emitScratchEntryPrologue(const Dyninst::AmdgpuKernelDescriptor &kd,
                                codeGen &gen) override {
    using namespace AmdgpuGfx908;
    // Read the ABI SGPR layout from the (already scratch-enabled) KD: exact indices
    // of flat_scratch_init (per-queue base) and the scratch wavefront offset.
    Dyninst::DyninstAPI::AbiSgprLayout L = Dyninst::DyninstAPI::computeAbiSgprLayout(kd);
    const uint32_t fsi     = (uint32_t)L.flatScratchInit;   // e.g. s[6:7]
    const uint32_t waveoff = (uint32_t)L.waveOffset;        // e.g. s9

    // Set FLAT_SCRATCH (s[102:103]) = flat_scratch_init + wave_offset — the exact
    // per-wave base the compiler computes (verified vs scratch_probe:
    // `s_add_u32 flat_scratch_lo, s6, s9`). gfx908 does NOT pre-init FLAT_SCRATCH, so
    // we compute it here. It's a SPECIAL register (above MAX_SGPR_ID), NOT from the
    // allocatable pool, so it costs zero GP SGPRs. wave_offset (s9) is CONSUMED here
    // and dead afterward — the scratch_* spills read the base implicitly from
    // FLAT_SCRATCH, so no s[94:95] pair and no per-lane vAddr are needed.
    // MUST be an ADD (base + per-wave offset), not a mov, or every wave aliases.
    emitSop2Raw(S_ADD_U32,  GFX908_FLAT_SCRATCH_LO, fsi,     waveoff,         gen); // FS_LO = fsi_lo + wave_off
    emitSop2Raw(S_ADDC_U32, GFX908_FLAT_SCRATCH_HI, fsi + 1, GFX908_INLINE_0, gen); // FS_HI = fsi_hi + carry
    // NOTE: the scratch SADDR register (=0) is NOT set here — it lives at the top of
    // the tight grant (reservedBase+2, kernel-relative) and is zeroed per-trampoline
    // in emitCall, so the prologue stays decoupled from reservedBase (FLAT_SCRATCH is
    // a fixed special reg). This is what lets the grant be sized down to the actual
    // requirement without the prologue needing to know it.

    // Enabling flat_scratch_init shifted the SYSTEM SGPRs up by 2 (Flat Scratch
    // Init occupies 2 user SGPRs). Move the ones the un-shifted kernel body reads
    // (wgid/info) back down by 2. Do AFTER the add (which consumes s[fsi],
    // s[waveoff]); ascending order is collision-free.
    const uint32_t SHIFT = 2;
    const int sysRegs[4] = { L.wgidX, L.wgidY, L.wgidZ, L.wgInfo };
    for (int p : sysRegs)
      if (p >= 0)
        emitSop1Raw(/*S_MOV_B32=*/0, (uint32_t)p - SHIFT, (uint32_t)p, gen);
    emitSopP(S_WAITCNT, /*simm16=*/0, gen);
  }

  void emitScratchStore(uint32_t vreg, int32_t offset, uint32_t saddr, codeGen &gen) override {
    using namespace AmdgpuGfx908;
    // SADDR = a reserved SGPR the trampoline sets to 0; the per-wave 64-bit base
    // comes implicitly from FLAT_SCRATCH; per-lane swizzle is automatic. addr(VADDR)
    // is ignored in SADDR mode (SADDR != 0x7f). glc for L2-coherence across the call.
    emitFlatGlobal(GLOBAL_STORE_DWORD, /*vdst=*/0, /*addr=*/0, /*data=*/vreg,
                   saddr, offset, gen, /*glc=*/true, /*seg=*/1);
  }
  void emitScratchLoad(uint32_t vreg, int32_t offset, uint32_t saddr, codeGen &gen) override {
    using namespace AmdgpuGfx908;
    emitFlatGlobal(GLOBAL_LOAD_DWORD, /*vdst=*/vreg, /*addr=*/0, /*data=*/0,
                   saddr, offset, gen, /*glc=*/true, /*seg=*/1);
  }
};
}  // namespace

namespace Dyninst { namespace DyninstAPI {
ScratchAbi &gfx908ScratchAbi() { static Gfx908ScratchAbi abi; return abi; }
}}

Register EmitterAmdgpuGfx908::emitCall(opCode op, codeGen &gen,
                                       const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands,
                                       func_instance *callee) {
  assert(op == callOp);
  assert(callee && "emitCall: callee is null");

  // Immediate call arguments (if any) are materialized into the AMDGPU function-CC
  // argument registers just before the call — see below, after the register spill.

  // Determine whether the callee lives in a different mapped object (i.e.
  // a different module / shared object). The static-rewriting flow has us
  // here only when there is a current function; matches the x86 idiom in
  // EmitterIA32Stat::emitCallInstruction.
  func_instance *caller = gen.func();
  const bool isExternal = caller && (caller->obj() != callee->obj());

  if (!isExternal) {
    // Intra-module direct call would be a PC-relative branch via
    // insnCodeGen::generateBranch. Leaving that path for future work so this
    // commit stays scoped to the inter-module case.
    assert(!"AMDGPU emitCall: intra-module direct call not implemented yet");
    return Null_Register;
  }

  // The link-register pair receives the return address from S_SWAPPC_B64.
  // It MUST be s[30:31]: the AMDGPU non-kernel-function ABI returns via
  // `s_setpc_b64 s[30:31]`, so a separately-compiled callee reads its return
  // address from s[30:31]. Using an arbitrary allocated pair here leaves
  // s[30:31] untouched and the callee returns to garbage (jump to ~0 / fault).
  // s30/s31 is the ABI return-address register (caller-clobbered by a call), so
  // it is fixed, not taken from the allocator.
  registerSpace *rs = gen.rs();
  assert(rs && "AMDGPU emitCall: codeGen has no registerSpace");
  Register lrPair(OperandRegId(30), RegKind::SCALAR, BlockSize(2));

  // Preserve the caller's registers across the call. The AMDGPU function ABI
  // lets a callee clobber any caller-saved SGPR (e.g. hc_open uses s[4:5] for
  // its exec-mask save, but the kernel holds its kernarg pointer there). So
  // spill the SGPRs the callee will clobber to this wave's scratch slot (base in
  // s[94:95], set up by the prologue) before the call, and reload them after.
  // The clobber count is the callee's TRANSITIVE "numbered_sgpr" — the whole-
  // object max, not just this wrapper's frame, because the wrapper may call
  // other functions in its library that clobber more SGPRs (see
  // readCalleeMaxAbsSym).
  // Register footprint the callee (transitively) clobbers — whole-object max, so
  // it covers everything the wrapper reaches (e.g. hc_write -> gpu_fwrite).
  const uint32_t nsgpr = readCalleeMaxAbsSym(callee, "numbered_sgpr");
  const uint32_t nvgpr = readCalleeMaxAbsSym(callee, "num_vgpr");

  // --- roadmap 1.0/1.1: liveness probe -------------------------------------
  // AMDGPU register liveness IS wired in this dyninst (ABI.C dispatches the
  // gfx908 register->bitArray map; the decoder marks per-operand read/written;
  // registerSpace::actualRegSpace(point) -> instPoint::liveRegisters() populates
  // liveState). This diagnostic dumps, at each insertion point, which of the
  // callee-clobbered registers s0..s(nsgpr-1)/vcc/v0..v(nvgpr-1) are actually
  // LIVE in the caller — the set the efficient spill (1.1) may restrict itself
  // to. Env-gated so it costs nothing by default.
  if (getenv("DYNINST_DUMP_LIVE") && gen.point()) {
    registerSpace *lrs = registerSpace::actualRegSpace(gen.point());
    if (lrs) {
      auto isLive = [&](Register r) -> bool {
        registerSlot *sl = (*lrs)[r];
        return sl && sl->liveState == registerSlot::live;
      };
      std::string liveS, liveV;
      for (uint32_t i = 0; i < nsgpr; i++)
        if (isLive(Register::makeScalarRegister(OperandRegId(i), BlockSize(1))))
          liveS += " s" + std::to_string(i);
      if (isLive(Register::makeScalarRegister(OperandRegId(106), BlockSize(1)))) liveS += " vcc_lo";
      if (isLive(Register::makeScalarRegister(OperandRegId(107), BlockSize(1)))) liveS += " vcc_hi";
      for (uint32_t i = 0; i < nvgpr; i++)
        if (isLive(Register::makeVectorRegister(OperandRegId(i), BlockSize(1))))
          liveV += " v" + std::to_string(i);
      fprintf(stderr,
              "[amdgpu][live] @0x%lx call %s: callee clobbers s0..s%u,vcc,v0..v%u; "
              "LIVE-at-point SGPR:{%s } VGPR:{%s }\n",
              (unsigned long)gen.point()->insnAddr(),
              callee ? callee->symTabName().c_str() : "?", nsgpr ? nsgpr - 1 : 0,
              nvgpr ? nvgpr - 1 : 0, liveS.c_str(), liveV.c_str());
    }
  }

  // Two spill backends (env-gated, so the proven global-buffer path stays default
  // while the scratch path is brought up):
  //   default        : global_store/load into the launcher's per-wave buffer via
  //                     s[94:95] base + mbcnt lane offset (vAddr).
  //   DYNINST_SPILL_SCRATCH : hardware SCRATCH via the ScratchAbi seam. The per-wave
  //                     base lives in FLAT_SCRATCH (s[102:103], a SPECIAL register —
  //                     not from the allocatable pool, so 0 GP cost) and the per-lane
  //                     swizzle is hardware-provided, so there is NO mbcnt lane math
  //                     (no vAddr) and each register needs only 4 bytes of the slot.
  //                     gfx908 scratch still needs a SADDR *register* (both-operands-
  //                     off is illegal), but it's just a constant 0 in s94 (set by the
  //                     entry prologue) — one SGPR instead of the s[94:95] pair.
  const bool useScratch = (getenv("DYNINST_SPILL_SCRATCH") != nullptr);
  Dyninst::DyninstAPI::ScratchAbi &sabi = Dyninst::DyninstAPI::gfx908ScratchAbi();

  // Scratch VGPRs for the spill machinery, placed ABOVE both the kernel's and the
  // callee's VGPR usage (bumpCallerKdForCallee grows the grant to cover them):
  //   vPack = holds SGPRs packed one-per-lane for the SGPR spill (below)
  //   vAddr = this lane's byte offset (lane*4) — GLOBAL path only; the scratch path
  //           gets per-lane addressing from hardware, so it reuses vBase for vPack.
  // ORIGINAL granted VGPR (cached) keeps these the same fixed regs for every call.
  const uint32_t kernelVgpr = readCallerOriginalGrantedVgpr(caller, gen);
  const uint32_t vBase = (kernelVgpr > nvgpr ? kernelVgpr : nvgpr);
  const uint32_t vAddr = vBase;                          // global: lane-offset reg
  const uint32_t vPack = useScratch ? vBase : vBase + 1; // scratch reclaims vAddr; base of the pack VGPRs
  // The SGPR spill packs {nsgpr numbered SGPRs, vcc_lo, vcc_hi} one-per-lane. Each
  // pack VGPR holds 64 lanes, so cover them with ceil((nsgpr+2)/64) pack VGPRs
  // (vPack .. vPack+numPacks-1) — no 62-SGPR limit; each just needs one more dword
  // of scratch. numPacks==1 for typical callees, reproducing the single-pack path.
  const uint32_t packLanes = spillPackLanes();   // 64 (wave64); overridable for testing
  const uint32_t numPacks = (nsgpr + 2u + packLanes - 1u) / packLanes;

  // Reserved SGPRs for the trampoline. GLOBAL: fixed high s[92:93] (EXEC save) +
  // s[94:95] (base) — safe only because the SGPR grant is MAXED (nothing is that
  // high). SCRATCH: no maxing — the reserved block sits near the TOP of a TIGHT
  // grant sized by maximizeSgprAllocationIfKernel (gran 4 = 48 SGPRs). CRITICAL: the
  // wavefront SGPR count INCLUDES the special regs (per AMDGPU ABI) — FLAT_SCRATCH =
  // s[grantTop-4:grantTop-3], VCC = s[grantTop-2:grantTop-1] (xnack- => no XNACK
  // pair). So the reserved block must go BELOW those 4 specials: reservedBase =
  // grantTop-8. EXEC save at [reservedBase,+1], SADDR at reservedBase+2 (zeroed in
  // the trampoline below). generateBranch reserves the SAME [reservedBase,+3] block
  // so the springboard/call-target allocation can't collide. reservedBase is above
  // the callee's clobber range because the grant top is sized above it.
  const uint32_t sgprTop = useScratch ? readCallerGrantedSgprTop(caller, gen) : 0;
  const uint32_t reservedBase = useScratch ? (sgprTop >= 10 ? sgprTop - 10 : 0) : 92;
  const uint32_t EXEC_SAVE_REG = reservedBase;               // scratch top-4 / global 92
  const uint32_t SADDR_REG = useScratch ? reservedBase + 2 : 94; // scratch SADDR=0 / global base
  const uint32_t SADDR_S94 = SADDR_REG;  // global s[94:95] base name
  const uint32_t SGPR_AREA = 0;     // global: SGPR pack area  [0, 256)  (64 lanes * 4B)
  const uint32_t VGPR_AREA = numPacks * 256;   // global: VGPR spill area, after the packs (256B/pack)
  const uint32_t S_SGPR_SLOT = 0;   // scratch: SGPR pack dword (per-lane, 4B)
  const uint32_t S_VGPR_SLOT = numPacks * 4; // scratch: VGPR spill area, after the packs

  // vAddr = lane*4 (absolute lane id via mbcnt over an all-ones mask: -1=193, 0=128).
  auto emitLaneByteOffset = [&]() {
    emitVop3a(V_MBCNT_LO_U32_B32, vAddr, /*src0=*/193, /*src1=*/128, /*src2=*/0, gen);
    emitVop3a(V_MBCNT_HI_U32_B32, vAddr, /*src0=*/193, /*src1=*/256 + vAddr, /*src2=*/0, gen);
    emitVop2(V_LSHLREV_B32, vAddr, /*vsrc1=*/vAddr, /*src0=*/130 /*inline 2*/, gen);
  };

  // SGPR (+VCC) preservation the way LLVM does it — NEVER through SMEM/K$. A
  // scalar-cache spill is corrupted by the callee's s_dcache_inv (issued for the
  // hostcall mailbox): observed as C-base reverting to a stale value -> +4 GiB
  // fault, timing-dependent and immune to wb/inv. Instead move each SGPR into a
  // lane of vPack with v_writelane, spill vPack through the VECTOR pipe
  // (global_store), and reverse with global_load + v_readlane. The vector pipe
  // (vmcnt / L1-L2) is untouched by s_dcache_inv — the same path the VGPR spill
  // already uses successfully. Lanes: s0..s(nsave-1) in lanes 0.., VCC_LO/HI next.
  // NOTE: assumes EXEC is full at the insertion point (true here); a narrowed EXEC
  // would drop the store/load of inactive packing lanes (future: force EXEC=-1).
  // Spill through the global-memory path: s[94:95] is the base and vAddr = lane*4
  // gives each lane its own slice (INTRA-wave separation — required because
  // global_store is per-lane and vPack holds a different value per lane). The
  // per-wave base in s[94:95] (scratch mode) gives INTER-wave separation. Same
  // code for both modes; only the prologue differs in what base it puts in
  // s[94:95] (per-wave scratch region vs. the launcher's shared buffer).
  // Pack the callee-clobbered scalars {s0..s(nsgpr-1), vcc_lo, vcc_hi} one-per-lane
  // across numPacks VGPRs (64 lanes each): scalar index i -> s_i (i<nsgpr), else
  // vcc_lo (i==nsgpr) / vcc_hi (i==nsgpr+1). Each pack VGPR spills to its own dword
  // slot, so there is no 62-SGPR limit. VCC(106/107) and s_i are fixed operand
  // encodings the HW maps to the wave's physical SGPRs regardless of the grant.
  // roadmap 1.1: restrict the spill to registers actually LIVE at the insertion
  // point (intersect the callee clobber footprint with actualRegSpace liveness).
  // Default (no DYNINST_LIVE_SPILL, or no point liveness) builds the FULL clobber
  // list -> byte-identical to the proven full-footprint spill. When enabled,
  // liveScalars/liveVgprs hold only the live subset, so a point with nothing live
  // (e.g. kernel exit) emits ZERO spill traffic. The LAYOUT/reservation sizing
  // above (numPacks, VGPR_AREA, S_VGPR_SLOT, reservedBase, the KD grant) stays
  // conservative — computed from nsgpr/nvgpr — so all the delicate grant and
  // reserved-block logic is untouched; only the emitted save/restore ops shrink.
  // liveScalars holds SGPR operand ids (0..nsgpr-1, then 106/107 for vcc_lo/hi);
  // liveVgprs holds VGPR ids. See [[dyninst-amdgpu-liveness-works]].
  // DYNINST_LIVE_SPILL: reduce the register spill to the callee clobber footprint
  // INTERSECTED with the registers LIVE at the insertion point. Both SGPR and VGPR
  // liveness are reliable now that the aliased special-register liveness bug is
  // fixed (dataflowAPI/src/liveness.C: getBaseRegister maps vcc_lo/hi -> vcc,
  // exec_lo/hi -> exec, flat_scratch_lo/hi -> flat_scratch_all, none of which are
  // in the liveness index map, so their reads/writes were silently dropped and VCC
  // was reported dead across a live carry chain -> a reduced SGPR spill clobbered
  // VCC -> address fault). Validated: SGPR+VGPR reduction PASSES N=64/256/1024.
  // DYNINST_FULL_SGPR / DYNINST_FULL_VGPR force-spill that class in full (escape
  // hatches for A/B testing). See [[dyninst-amdgpu-liveness-works]].
  const bool liveSpill = (getenv("DYNINST_LIVE_SPILL") != nullptr);
  const bool reduceSgpr = liveSpill && !getenv("DYNINST_FULL_SGPR");
  const bool reduceVgpr = liveSpill && !getenv("DYNINST_FULL_VGPR");
  std::vector<uint32_t> liveScalars, liveVgprs;
  {
    registerSpace *lrs = (liveSpill && gen.point())
                             ? registerSpace::actualRegSpace(gen.point())
                             : nullptr;
    auto live = [&](registerSpace *rs, Register r) -> bool {
      if (!rs) return true;                // no liveness -> conservative: save all
      registerSlot *sl = (*rs)[r];
      return sl && sl->liveState == registerSlot::live;
    };
    for (uint32_t i = 0; i < nsgpr; i++)
      if (live(reduceSgpr ? lrs : nullptr, Register::makeScalarRegister(OperandRegId(i), BlockSize(1))))
        liveScalars.push_back(i);
    if (live(reduceSgpr ? lrs : nullptr, Register::makeScalarRegister(OperandRegId(106), BlockSize(1)))) liveScalars.push_back(106);
    if (live(reduceSgpr ? lrs : nullptr, Register::makeScalarRegister(OperandRegId(107), BlockSize(1)))) liveScalars.push_back(107);
    for (uint32_t i = 0; i < nvgpr; i++)
      if (live(reduceVgpr ? lrs : nullptr, Register::makeVectorRegister(OperandRegId(i), BlockSize(1))))
        liveVgprs.push_back(i);
  }
  const uint32_t nScalars = (uint32_t)liveScalars.size();
  // Packs actually needed for the LIVE scalars (<= numPacks reserved above), so the
  // pack VGPRs vPack..vPack+numPacksLive-1 and scratch slots p*4 stay a subset of
  // the conservatively reserved region.
  const uint32_t numPacksLive = (nScalars + packLanes - 1u) / packLanes;
  auto packSrc = [&](uint32_t idx) -> uint32_t { return liveScalars[idx]; };
  auto spillFill = [&](bool save) {
    if (nScalars == 0) return;             // nothing live to preserve (e.g. exit)
    if (save) {
      for (uint32_t p = 0; p < numPacksLive; p++)
        for (uint32_t l = 0; l < packLanes; l++) {
          const uint32_t idx = p * packLanes + l;
          if (idx >= nScalars) break;
          emitVop3a(V_WRITELANE_B32, /*vdst=*/vPack + p, /*src0=*/packSrc(idx),
                    /*src1=*/128 + l, /*src2=*/0, gen);
        }
      if (!useScratch)
        emitLaneByteOffset();
      for (uint32_t p = 0; p < numPacksLive; p++) {
        if (useScratch)
          sabi.emitScratchStore(vPack + p, S_SGPR_SLOT + p * 4, SADDR_REG, gen);
        else
          emitFlatGlobal(GLOBAL_STORE_DWORD, /*vdst=*/0, /*addr=*/vAddr, /*data=*/vPack + p,
                         SADDR_S94, SGPR_AREA + p * 256, gen, /*glc=*/true);
      }
      emitSopP(S_WAITCNT, /*simm16=*/0, gen);
    } else {
      if (!useScratch)
        emitLaneByteOffset();
      for (uint32_t p = 0; p < numPacksLive; p++) {
        if (useScratch)
          sabi.emitScratchLoad(vPack + p, S_SGPR_SLOT + p * 4, SADDR_REG, gen);
        else
          emitFlatGlobal(GLOBAL_LOAD_DWORD, /*vdst=*/vPack + p, /*addr=*/vAddr, /*data=*/0,
                         SADDR_S94, SGPR_AREA + p * 256, gen, /*glc=*/true);
      }
      emitSopP(S_WAITCNT, /*simm16=*/0, gen);
      for (uint32_t p = 0; p < numPacksLive; p++)
        for (uint32_t l = 0; l < packLanes; l++) {
          const uint32_t idx = p * packLanes + l;
          if (idx >= nScalars) break;
          emitVop3a(V_READLANE_B32, /*vdst=*/packSrc(idx), /*src0=*/256 + (vPack + p),
                    /*src1=*/128 + l, /*src2=*/0, gen);
        }
    }
  };

  // Preserve the callee-clobbered VGPRs (e.g. v[4:5] address pair). Per-lane, via
  // the global path. TRANSITIVE footprint (whole-object max): the wrapper's own
  // num_vgpr undercounts what it clobbers via nested calls.
  auto vgprSpill = [&](bool save) {
    if (liveVgprs.empty())                 // roadmap 1.1: only live VGPRs
      return;
    if (!useScratch)
      emitLaneByteOffset();
    for (uint32_t i : liveVgprs) {
      if (i >= 256) continue;
      if (useScratch) {
        if (save) sabi.emitScratchStore(/*vreg=*/i, S_VGPR_SLOT + i * 4, SADDR_REG, gen);
        else      sabi.emitScratchLoad(/*vreg=*/i, S_VGPR_SLOT + i * 4, SADDR_REG, gen);
      } else if (save) {
        emitFlatGlobal(GLOBAL_STORE_DWORD, /*vdst=*/0, /*addr=*/vAddr, /*data=*/i,
                       SADDR_S94, VGPR_AREA + i * 256, gen, /*glc=*/true);
      } else {
        emitFlatGlobal(GLOBAL_LOAD_DWORD, /*vdst=*/i, /*addr=*/vAddr, /*data=*/0,
                       SADDR_S94, VGPR_AREA + i * 256, gen, /*glc=*/true);
      }
    }
    emitSopP(S_WAITCNT, /*simm16=*/0, gen);   // let the VMEM group complete
  };

  // Drain ALL in-flight memory before saving registers. dyninst can splice this
  // call between an async load (s_load/global_load into a register) and the
  // compiler's s_waitcnt that guards that register's first use — so a register we
  // are about to spill may still be the target of an outstanding load. Without
  // this wait the save reads the STALE pre-load value (observed: A/B/C base
  // pointers saved as garbage while an s_load_dwordx4 from kernarg was in flight
  // -> nil/wild-address fault; deterministic, hidden by any halt that lets the
  // load retire). vmcnt+lgkmcnt=0 covers both VMEM and scalar/LDS loads.
  emitSopP(S_WAITCNT, /*simm16=*/0, gen);

  // Preserve EXEC across the call and force ALL lanes on for the register spill.
  // The trampoline never saved EXEC before; the callee (hostcall wrapper) narrows
  // EXEC for lane election and restores it, but the caller's live EXEC must be
  // guaranteed intact across the whole trampoline — AND the SGPR pack's per-lane
  // store/load must cover every lane (writelane is EXEC-independent, but the
  // store/load are not). Save the caller EXEC in a reserved pair (s[92:93], above
  // the callee's clobber range), run the spill under EXEC=-1, restore the caller
  // EXEC for the call and again for the post-call continuation. (No-op at full
  // EXEC, e.g. single-wave N<=64.)  S_MOV_B64=1; EXEC_LO=126; inline -1=193.
  // EXEC_SAVE_REG is the reserved pair: s[92:93] (global, maxed grant) or the top
  // of the tight scratch grant (reservedBase).
  const uint32_t S_MOV_B64_OP = 1, EXEC_LO = 126, INLINE_NEG1 = 193;

  // Scratch SADDR must be 0 (whole per-wave base is in FLAT_SCRATCH). Set it in the
  // trampoline (reservedBase+2, above the callee clobber range so it survives the
  // call). S_MOV_B32=0; inline 0=128.
  if (useScratch)
    AmdgpuGfx908::emitSop1Raw(/*S_MOV_B32=*/0, SADDR_REG, /*inline 0=*/128, gen);

  AmdgpuGfx908::emitSop1Raw(S_MOV_B64_OP, EXEC_SAVE_REG, EXEC_LO,    gen);  // save exec
  AmdgpuGfx908::emitSop1Raw(S_MOV_B64_OP, EXEC_LO,   INLINE_NEG1, gen); // exec = -1 (all lanes)

  spillFill(/*save=*/true);
  vgprSpill(/*save=*/true);

  AmdgpuGfx908::emitSop1Raw(S_MOV_B64_OP, EXEC_LO, EXEC_SAVE_REG, gen);     // exec = caller (for call)

  // Materialize immediate call arguments into the AMDGPU function-CC arg registers.
  // Per the ABI, VGPR args are consecutive from v0 (a plain `int` lands in v0). HIP
  // can't express inreg/SGPR args, so we pass a uniform per-wave value in the arg
  // VGPR (v_mov writes the same immediate to every active lane; the callee may
  // readfirstlane it to a true scalar). v0.. are saved/restored by the spill above,
  // so overwriting them here (after save, before call) is safe. Done under the
  // caller EXEC so the active lanes the callee runs see the value. Only compile-time
  // Constant operands are supported for now.
  for (uint32_t i = 0; i < operands.size(); i++) {
    const auto &op = operands[i];
    assert(op && op->getoType() == operandType::Constant &&
           "AMDGPU emitCall: only immediate (Constant) call arguments are supported");
    const uint32_t imm = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(op->getOValue()));
    AmdgpuGfx908::emitVop1Imm(/*V_MOV_B32=*/1u, /*vdst=*/i, imm, gen);
  }

  Address slot = getInterModuleFuncAddr(callee, gen);
  emitIndirectCall(slot, lrPair, gen);
  AmdgpuGfx908::emitSop1Raw(S_MOV_B64_OP, EXEC_LO, INLINE_NEG1, gen);   // exec = -1 (for restore)

  vgprSpill(/*save=*/false);
  spillFill(/*save=*/false);

  AmdgpuGfx908::emitSop1Raw(S_MOV_B64_OP, EXEC_LO, EXEC_SAVE_REG, gen);     // exec = caller (continue)

  // Grow the caller kernel's register/scratch grant to fit the callee.
  bumpCallerKdForCallee(caller, callee, gen);

  // No return-value convention pinned down yet for AMDGPU calls.
  return Null_Register;
}

void EmitterAmdgpuGfx908::emitGetRetVal(Register /* dest */, bool /* addr_of */,
                                        codeGen & /* gen */) {
  assert(!"emitGetRetVal not implemented yet");
}

void EmitterAmdgpuGfx908::emitGetRetAddr(Register /* dest */, codeGen & /* gen */) {
  assert(!"emitGetRetAddr not implemented yet");
}

void EmitterAmdgpuGfx908::emitGetParam(Register /* dest */, Register /* param_num */,
                                       instPoint::Type /* pt_type */, opCode /* op */,
                                       bool /* addr_of */, codeGen & /* gen */) {
  assert(!"emitGetParam not implemented yet");
}

void EmitterAmdgpuGfx908::emitASload(int /* ra */, int /* rb */, int /* sc */, long /* imm */,
                                     Register /* dest */, int /* stackShift */,
                                     codeGen & /* gen */) {
  assert(!"emitASload not implemented yet");
}

void EmitterAmdgpuGfx908::emitAddrSpecLoad(const BPatch_addrSpec_NP * /* as */, Dyninst::Register /* dest */,
                                           int /* stackShift */, codeGen & /* gen */) {
  assert(!"emitAddrSpecLoad not implemented yet");
}

void EmitterAmdgpuGfx908::emitCSload(int /* ra */, int /* rb */, int /* sc */, long /* imm */,
                                     Register /* dest */, codeGen & /* gen */) {
  assert(!"emitCSload not implemented yet");
}

void EmitterAmdgpuGfx908::emitCountSpecLoad(const BPatch_addrSpec_NP *, Register, codeGen &) {
  assert(!"Not imeplemented for AMDGPU");
}

void EmitterAmdgpuGfx908::emitPushFlags(codeGen & /* gen */) {
  assert(!"emitPushFlags not implemented yet");
}

void EmitterAmdgpuGfx908::emitRestoreFlags(codeGen & /* gen */, unsigned /* offset */) {
  assert(!"emitRestoreFlags not implemented yet");
}

// Built-in offset...
void EmitterAmdgpuGfx908::emitRestoreFlagsFromStackSlot(codeGen & /* gen */) {
  assert(!"emitRestoreFlagsFromStackSlot not implemented yet");
}

bool EmitterAmdgpuGfx908::emitBTSaves(baseTramp * /* bt */, codeGen & /* gen */) {
  assert(!"emitBTSaves not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitBTRestores(baseTramp * /* bt */, codeGen & /* gen */) {
  assert(!"emitBTRestores not implemented yet");
  return false;
}

void EmitterAmdgpuGfx908::emitStoreImm(Address /* addr */, int /* imm */, codeGen & /* gen */) {
  assert(!"emitStoreImm not implemented yet");
}

void EmitterAmdgpuGfx908::emitAddSignedImm(Address /* addr */, int /* imm */, codeGen & /* gen */) {
  assert(!"emitAddSignedImm not implemented yet");
}

bool EmitterAmdgpuGfx908::emitPush(codeGen &, Register) {
  assert(!"emitPush not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitPop(codeGen &, Register) {
  assert(!"emitPop not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitAdjustStackPointer(int /* index */, codeGen & /* gen */) {
  assert(!"emitAdjustStackPointer not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::clobberAllFuncCall(registerSpace *rs, func_instance *callee) {
  if (!callee) {
    return true;
  }
  if (clobbered_functions.contains(callee)) {
    return true;
  }
  clobbered_functions.insert(callee);

  // No AMDGPU calling convention is pinned down in dyninst yet, so assume the
  // callee may write any allocatable SGPR or VGPR. Both kinds live in
  // registerSpace::GPRs_ on AMDGPU (registerSpace.C:170-181, SGPR/VGPR
  // fall through into the GPR case), so a single loop covers them.
  // numFPRs() is 0 on AMDGPU.
  for (int i = 0; i < rs->numGPRs(); i++) {
  //  rs->GPRs()[i]->beenUsed = true;
  }
  return true;
}

// Additional interfaces

void EmitterAmdgpuGfx908::emitNops(unsigned numNops, codeGen &gen) {
  assert(numNops >= 1 && numNops <= 16);
  // 0x0 inserts 1 nop, and 0xFF (15) inserts 16 nops, so subtract 1
  emitSopP(S_NOP, /* simm16 = */ (numNops - 1), gen);
}

void EmitterAmdgpuGfx908::emitEndProgram(codeGen &gen) {
  // Passing 0 as immediate value.
  // Value of immediate passed here doesn't matter as the instruction won't have
  // an immediate.
  emitSopP(S_ENDPGM, /* simm16 = */ 0, gen);
}

void EmitterAmdgpuGfx908::emitMovLiteral(Register reg, uint32_t literal, codeGen &gen) {
  assert(isValidSgpr(reg) && "reg must be a valid SGPR");
  // s_mov_b32 reg, < 32-bit constant literal >
  // The literal follows the instruction, so set src0 = 0xFF just like the
  // assembler does.
  emitSop1(S_MOV_B32, /* dest = */ reg.getId(), /* src0 = */ 0xFF, /* hasLiteral = */ true, literal,
           gen);
}

void EmitterAmdgpuGfx908::emitConditionalBranch(bool onConditionTrue, int16_t wordOffset,
                                                codeGen &gen) {
  unsigned opcode = onConditionTrue ? S_CBRANCH_SCC0 : S_CBRANCH_SCC1;
  emitSopP(opcode, /* simm16 = */ wordOffset, gen);
}

void EmitterAmdgpuGfx908::emitShortJump(int16_t wordOffset, codeGen &gen) {
  emitSopP(S_BRANCH, /* simm16 = */ wordOffset, gen);
}

// For this we need reg, reg+1, reg+2, reg+3.
void EmitterAmdgpuGfx908::emitLongJump(Register reg, uint64_t fromAddress, uint64_t toAddress,
                                       codeGen &gen) {
  assert(isValidSgprBlock(reg) && "reg must be a valid SGPR block");
  assert(reg.getCount() == 4 && "reg must have 4 registers");

  // s_getpc_b64 will give us beginning of next instruction.
  // So our fromAddress must be incremented by 4 to accomodate for this.
  int64_t signedFromAddress = (int64_t)fromAddress + 4;
  int64_t signedToAddress = (int64_t)toAddress;

  assert(signedFromAddress > 0 && signedToAddress > 0 && "Both addresses must be positive");
  int64_t diff = signedToAddress - signedFromAddress;

  emitSop1(S_GETPC_B64, /* dest= */ reg.getId(), /* src0 =*/0, /* hasLiteral = */ false,
           /* literal=*/0, gen);

  // we store the diff = to - from in this register pair
  Register diffRegPair(OperandRegId(reg.getId() + 2), RegKind::SCALAR, BlockSize(2));
  this->emitLoadConst(diffRegPair, (uint64_t)diff, gen);
  // Now we have:
  // reg+2 = lower bits of diff
  // reg+3 = upper bits of diff

  // reg = reg + <reg+2>, SCC = carry
  emitSop2(S_ADD_U32, reg.getId(), reg.getId(), reg.getId() + 2, gen);
  // <reg+1> = <reg+1> + <reg+3> + carry
  emitSop2(S_ADDC_U32, reg.getId() + 1, reg.getId() + 1, reg.getId() + 3, gen);

  // S_SETPC_B64 writes to the PC, so dest = 0 just like the assembler does.
  emitSop1(S_SETPC_B64, /* dest = */ 0, reg.getId(), /* hasLiteral = */ false, /* literal=*/0, gen);
}

void EmitterAmdgpuGfx908::emitIndirectCall(Address slotAddr, Register lrPair, codeGen &gen) {
  assert(isValidSgprPair(lrPair) && "lrPair must be a valid SGPR pair");

  // Need two scratch SGPR pairs adjacent and pair-aligned:
  //   pair0 (reg, reg+1)   -- slot address (PC-relative), then SMEM base
  //   pair1 (reg+2, reg+3) -- PC-relative addend, then the loaded callee entry
  registerSpace *rs = gen.rs();
  assert(rs && "emitIndirectCall: codeGen has no registerSpace");

  Register block = rs->allocateGprBlock(Dyninst::RegKind::SCALAR,
                                        /*numRegs=*/4,
                                        NS_amdgpu::PAIR_ALIGNMENT);
  assert(block != Null_Register && "emitIndirectCall: failed to allocate scratch SGPR block");

  Register slotAddrPair(OperandRegId(block.getId()),     RegKind::SCALAR, BlockSize(2));
  Register tgtPair     (OperandRegId(block.getId() + 2), RegKind::SCALAR, BlockSize(2));

  // 1. Compute the slot's runtime address PC-relative. The slot lives in the
  //    position-independent .dyninstInst region, so an absolute immediate would
  //    be wrong once the loader relocates the code object. Same getpc+add idiom
  //    as emitLongJump: S_GETPC_B64 returns (addr-of-getpc + 4), so the addend
  //    is slotAddr - (currAddr + 4).
  int64_t getpcAddr = (int64_t)gen.currAddr();
  int64_t diff = (int64_t)slotAddr - (getpcAddr + 4);

  emitSop1(S_GETPC_B64, /*dest=*/slotAddrPair.getId(), /*src0=*/0,
           /*hasLiteral=*/false, /*literal=*/0, gen);

  // diff -> tgtPair (reg+2, reg+3), then 64-bit add into slotAddrPair.
  emitLoadConst(tgtPair, (uint64_t)diff, gen);
  emitSop2(S_ADD_U32,  slotAddrPair.getId(),     slotAddrPair.getId(),     tgtPair.getId(),     gen);
  emitSop2(S_ADDC_U32, slotAddrPair.getId() + 1, slotAddrPair.getId() + 1, tgtPair.getId() + 1, gen);

  // 2. Load the 64-bit callee entry from *slotAddr. size=2 means
  //    S_LOAD_DWORDX2 (2 dwords = 8 bytes); emitLoadRelative emits the
  //    required S_WAITCNT for us.
  emitLoadRelative(tgtPair, /*offset=*/0, slotAddrPair, /*size=*/2, gen);

  // 3. Indirect call: PC <- tgtPair, lrPair <- return address (PC+4).
  emitSop1(S_SWAPPC_B64,
           /*dest=*/lrPair.getId(),
           /*src0=*/tgtPair.getId(),
           /*hasLiteral=*/false, /*literal=*/0, gen);

  rs->freeGprBlock(block);
}

void EmitterAmdgpuGfx908::emitAddConstantToRegPair(Register reg, int constant, codeGen &gen) {
  assert(isValidSgprPair(reg) && "reg must be a valid SGPR pair");

  // reg has lower bits
  emitSop2WithSrc1Literal(S_ADD_U32, reg.getId(), reg.getId(), constant, gen);

  // reg+1 has upper bits. Add 0 with carry.
  emitSop2WithSrc1Literal(S_ADDC_U32, reg.getId() + 1, reg.getId() + 1, 0, gen);
}

void EmitterAmdgpuGfx908::emitScalarDataCacheWriteback(codeGen &gen) {
  emitSmem(S_DCACHE_WB, 0, 0, 0, gen);
}

void EmitterAmdgpuGfx908::emitAtomicAdd(Register baseAddrReg, Register src0, codeGen &gen) {
  assert(isValidSgprPair(baseAddrReg) && "baseAddrReg must be a valid SGPR pair");
  assert(isValidSgpr(src0) && "src0 must be a valid SGPR");

  emitSmem(S_ATOMIC_ADD, src0.getId(), (baseAddrReg.getId() >> 1), /* offset= */ 0, gen);
}

void EmitterAmdgpuGfx908::emitAtomicSub(Register baseAddrReg, Register src0, codeGen &gen) {
  assert(isValidSgprPair(baseAddrReg) && "baseAddrReg must be a valid SGPR pair");
  assert(isValidSgpr(src0) && "src0 must be a valid SGPR");

  emitSmem(S_ATOMIC_SUB, src0.getId(), (baseAddrReg.getId() >> 1), /* offset = */ 0, gen);
}
// ===== EmitterAmdgpuGfx908 implementation end =====
