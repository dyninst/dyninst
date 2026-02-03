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

#include "BPatch_function.h"
#include "PointHandler.h"
#include "common/src/dyn_register.h"

// We use std::shared_ptr for prologue and epilogue asts because AstNodePtr is a boost::shared_ptr
#include "boost/shared_ptr.hpp"

#include "AmdgpuEpilogue.h"
#include "AmdgpuPrologue.h"

#include "AmdgpuKernelDescriptor.h"
#include "external/amdgpu/AMDGPUEFlags.h"

#include <string>
#include <unordered_set>

namespace Dyninst {
// This implements:
// 1. Prologue/epilogue insertion at function entry/exit respectively.
// 2. Kernel descriptor rewriting for instrumented kernels.
// 3. Functions that write information about instrumented kernels and instrumentation variables to disk.
//    (These functions are called before the rewritten binary is written to disk)
struct AmdgpuGfx908PointHandler : PointHandler {

  unsigned eflag = EF_AMDGPU_MACH_AMDGCN_GFX908;

  std::unordered_set<BPatch_function *> instrumentedFunctions;

  void handlePoints(std::vector<BPatch_point *> const &points);

  BPatch_variableExpr* getKernelDescriptorVariable(BPatch_function *f);
  uint32_t getMaxGranulatedWavefrontSgprCount() const;

  bool canInstrument(const AmdgpuKernelDescriptor &kd) const;
  bool isRegPairAvailable(Register reg, BPatch_function *function);

  void insertPrologueIfKernel(BPatch_function *function);
  void insertEpilogueIfKernel(BPatch_function *function);
  void maximizeSgprAllocationIfKernel(BPatch_function *function);

  void insertPrologueAtPoints(AmdgpuPrologueSnippet &snippet, std::vector<BPatch_point *> &points);
  void insertEpilogueAtPoints(AmdgpuEpilogueSnippet &snippet, std::vector<BPatch_point *> &points);

  void writeInstrumentedKernelNames(const std::string &filePath);
  void writeInstrumentationVarTable(const std::string &filePath);
};

} // namespace Dyninst
