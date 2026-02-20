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

#include "AmdgpuPointHandler.h"

#include "BPatch_addressSpace.h"
#include "BPatch_module.h"
#include "RegisterConversion.h"
#include "instPoint.h"
#include "liveness.h"

// The raw kernel descriptor
#include "external/amdgpu/AMDHSAKernelDescriptor.h"

// Our wrapper around it
#include "AmdgpuKernelDescriptor.h"

#include "ast.h"
#include "debug.h"

#include <cassert>
#include <fstream>

namespace Dyninst {

void AmdgpuGfx908PointHandler::handlePoints(std::vector<BPatch_point *> const &points) {
  for (BPatch_point *point : points) {
    BPatch_function *f = point->getFunction();
    auto result = instrumentedFunctions.insert(f);
    if (result.second) {
      insertPrologueIfKernel(f);
      insertEpilogueIfKernel(f);
      maximizeSgprAllocationIfKernel(f);
    }
  }
}

BPatch_variableExpr* AmdgpuGfx908PointHandler::getKernelDescriptorVariable(BPatch_function *f) {
  // The kernel descriptor symbols have global visibility. So Dyninst will see them as global
  // variables.

  std::string kdName = f->getMangledName() + std::string(".kd");

  BPatch_module *m = f->getModule();
  BPatch_variableExpr* kernelDescriptorVariable = m->findVariable(kdName.c_str());

  if (kernelDescriptorVariable) {
      // FIXME : getSize is broken. Returns 4 instead of 64.
      // && kernelDescriptorVariable->getSize() == sizeof(llvm::amdhsa::kernel_descriptor_t)) {
    return kernelDescriptorVariable;
  }
  return nullptr;
}

uint32_t AmdgpuGfx908PointHandler::getMaxGranulatedWavefrontSgprCount() const {
  // This math comes from LLVM AMDGPUUsage documentation
  const uint32_t maxSgprCount = 112; // Set SGPRs to 112 (max value)
  return 2 * ((maxSgprCount / 16) - 1);
}

// This returns the maximum SGPR ID by looking at the number of SGPR blocks allocated in the
// kernel.
uint32_t AmdgpuGfx908PointHandler::getMaxUsedSgprId(const AmdgpuKernelDescriptor &kd) const {
  // The below math comes from LLVM AMDGPUUsage documentation
  // numSgprBlocks = 2 * ((maxUsedSgprId / 16) - 1)
  //
  // Therefore, maxUsedSgprId = ((numSgprBlocks/2) + 1) * 16

  const uint32_t numSgprBlocks = kd.getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount();
  const uint32_t maxUsedSgprId = ((numSgprBlocks/2) + 1) * 16;
  return maxUsedSgprId;
}

bool AmdgpuGfx908PointHandler::canInstrument(const AmdgpuKernelDescriptor &kd) const {
  return (kd.getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount() != getMaxGranulatedWavefrontSgprCount());
}

bool AmdgpuGfx908PointHandler::isScalarRegAvailable(Register reg, const AmdgpuKernelDescriptor &kd) const {
  // Since we max out register usage, the maximum SPGR is 101. AMDGPU GFX908
  // aliases the VCC (opcode register ids 106 and 107) to highest allocated
  // register pair (100 and 101 in this case), so the actual maximum available
  // is 99.

  assert(reg.isScalar() && "reg must be scalar");

  // We can only use upto s99
  const uint32_t maxUsableSgprId = 99;

  const uint32_t firstRegId = reg.getId();
  const uint32_t numRegs = reg.getCount();
  const uint32_t lastRegId = firstRegId + numRegs - 1;

  const uint32_t maxUsedSgprIdInKernel = getMaxUsedSgprId(kd);

  const bool isPair = numRegs == 2;
  const bool isEvenAligned = firstRegId % 2 == 0;
  const bool isNotUsedInKernel = firstRegId > maxUsedSgprIdInKernel && lastRegId <= maxUsableSgprId;

  return isPair && isEvenAligned && isNotUsedInKernel;
}

void AmdgpuGfx908PointHandler::insertPrologueIfKernel(BPatch_function *function) {
  // If this function is a kernel, insert a prologue that loads s[94:95] with the address of memory
  // for instrumentation variables. This address is at address [kernargPtrRegister] + kernargBufferSize.
  // We get this information from the kernel descriptor. Each kernel has a kernel descriptor.

  BPatch_variableExpr *kdVariable = getKernelDescriptorVariable(function);
  if (!kdVariable) {
    return;
  }

  llvm::amdhsa::kernel_descriptor_t rawKd;
  bool success = kdVariable->readValue(&rawKd, sizeof rawKd);
  assert(success);

  AmdgpuKernelDescriptor kd(rawKd, this->eflag);

  if (!canInstrument(kd)) {
    std::cerr << "Can't instrument " << function->getMangledName() << '\n' << "exiting...\n";
    exit(1);
  }

  Register regPair = Register::makeScalarRegister(OperandRegId(94), BlockSize(2));
  if (!isScalarRegAvailable(regPair, kd)) {
    std::cerr << "Can't instrument " << function->getMangledName()
              << " as s94 and s95 are not available.\n"
              << "exiting...\n";
    exit(1);
  }

  std::vector<BPatch_point *> entryPoints;
  function->getEntryPoints(entryPoints);

  auto prologuePtr =
      boost::make_shared<AmdgpuPrologue>(regPair, kd.getKernargPtrRegisterPair(), kd.getKernargSize());

  AstNodePtr prologueNodePtr =
        boost::make_shared<AmdgpuPrologueNode>(prologuePtr);

  AmdgpuPrologueSnippet prologueSnippet(prologueNodePtr);
  insertPrologueAtPoints(prologueSnippet, entryPoints);
}

void AmdgpuGfx908PointHandler::insertEpilogueIfKernel(BPatch_function *function) {
  // If this function is a kernel, insert a s_dcache_wb instruction at its exit points.
  // TODO : A s_dcache_wb must be inserted before every s_endpgm instruction on an instrumented path
  // from a kernel.
  BPatch_variableExpr *kdVariable = getKernelDescriptorVariable(function);
  if (!kdVariable)
    return;

  std::vector<BPatch_point *> exitPoints;
  function->getExitPoints(exitPoints);

  auto epiloguePtr = boost::make_shared<AmdgpuEpilogue>();

  AstNodePtr epilogueNodePtr = boost::make_shared<AmdgpuEpilogueNode>(epiloguePtr);

  AmdgpuEpilogueSnippet epilogueSnippet(epilogueNodePtr);
  insertEpilogueAtPoints(epilogueSnippet, exitPoints);
}

static constexpr uint32_t roundUpTo8(uint32_t x) {
  return ((x + 7) / 8) * 8;
}

void AmdgpuGfx908PointHandler::maximizeSgprAllocationIfKernel(BPatch_function *function) {
  BPatch_variableExpr *kdVariable = getKernelDescriptorVariable(function);
  assert(kdVariable);

  llvm::amdhsa::kernel_descriptor_t rawKd;
  bool success = kdVariable->readValue(&rawKd, sizeof rawKd);
  if(!success) {
    inst_printf("Unable to read kernel descriptor for %s\n", kdVariable->getName());
    assert(0);
  }

  AmdgpuKernelDescriptor kd(rawKd, this->eflag);

  uint32_t newValue = this->getMaxGranulatedWavefrontSgprCount();
  kd.setCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount(newValue);

  uint32_t kernargSize = kd.getKernargSize();
  uint32_t newKernargSize = roundUpTo8(kernargSize) + 8;
  kd.setKernargSize(newKernargSize);

  // We have modified the kernel descriptor. Now overwrite the original one with it.
  success = kdVariable->writeValue(kd.getRawPtr(), sizeof(rawKd), false);
  // false is passed explicitly only so the compiler can do overload resolution between:
  // writeValue(const void *src, bool saveWorld=false)
  //      and
  // writeValue(const void *src, int len,bool saveWorld=false)

  if(!success) {
    inst_printf("Unable to write kernel descriptor for %s\n", kdVariable->getName());
    assert(0);
  }
}

void AmdgpuGfx908PointHandler::insertPrologueAtPoints(AmdgpuPrologueSnippet &snippet,
                                                      std::vector<BPatch_point *> &points) {
  for (BPatch_point *point : points) {
    auto *iPoint = dynamic_cast<instPoint *>(PatchAPI::convert(point, BPatch_callBefore));
    assert(iPoint);
    iPoint->pushBack(snippet.ast_wrapper);
  }
}

void AmdgpuGfx908PointHandler::insertEpilogueAtPoints(AmdgpuEpilogueSnippet &snippet,
                                                      std::vector<BPatch_point *> &points) {
  for (BPatch_point *point : points) {
    auto *iPoint = dynamic_cast<instPoint *>(PatchAPI::convert(point, BPatch_callAfter));
    assert(iPoint);
    iPoint->pushBack(snippet.ast_wrapper);
  }
}

void AmdgpuGfx908PointHandler::writeInstrumentedKernelNames(const std::string &filePath) {
  std::ofstream outFile(filePath);

  for (auto *function : instrumentedFunctions) {
    BPatch_variableExpr *kdVar = getKernelDescriptorVariable(function);
    if(!kdVar)
      continue;

    llvm::amdhsa::kernel_descriptor_t rawKd;
    bool success = kdVar->readValue(&rawKd, sizeof rawKd);
    assert(success);

    AmdgpuKernelDescriptor kd(rawKd, this->eflag);
    outFile << function->getMangledName() << ' ' << kd.getKernargSize() << '\n';
  }

  outFile.close();
}

void AmdgpuGfx908PointHandler::writeInstrumentationVarTable(const std::string &filePath) {
  std::ofstream outFile(filePath);

  // To get all instrumentation variables sorted by offsets
  std::map<int, std::string> reverseAllocTable;

  for (auto it : AstOperandNode::allocTable) {
    auto name = it.first;
    if (name == "--init--") // ignore this one as it was only used to initialize the table
      continue;

    reverseAllocTable[it.second] = it.first;
  }

  for (auto it : reverseAllocTable) {
    outFile << it.first << ' ' << it.second << '\n';
  }

  outFile.close();
}

} // namespace Dyninst
