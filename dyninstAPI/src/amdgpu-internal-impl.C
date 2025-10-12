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

#include "amdgpu-internal-impl.h"

// PatchAPI includes
#include "Point.h"

// DyninstAPI includes
#include "instPoint.h"

using namespace Dyninst;
using namespace Dyninst::PatchAPI;

// KERNEL-INFO-START
const int NumAmdgpuKernelElems = 3;

AmdgpuKernelInfo::AmdgpuKernelInfo(std::vector<std::string> &words) {
  assert(words.size() == NumAmdgpuKernelElems);
  kdName = words[0];
  kernargBufferSize = std::stoul(words[1]);
  kernargPtrRegister = std::stoul(words[2]);
}

std::string AmdgpuKernelInfo::getKernelName() const {
  assert(kdName.length() > 3);
  return kdName.substr(0, kdName.length() - 3);
}
// KERNEL-INFO-END

// INTERNAL-IMPL-START
void AmdgpuInternalImpl::insertPrologueAtPoints(AmdgpuPrologueSnippet &snippet,
                                                std::vector<BPatch_point *> &points) {
  for (size_t i = 0; i < points.size(); ++i) {
    instPoint *iPoint = static_cast<instPoint *>(points[i]->getPoint(BPatch_callBefore));
    iPoint->pushBack(snippet.ast_wrapper, SnippetType::PROLOGUE);
  }
}

void AmdgpuInternalImpl::insertPrologueIfKernel(BPatch_function *function) {
  // Go through information for instrumented kernels (functions) and insert a prologue
  // that loads s[94:95] with address of memory for instrumentation variables. This address
  // is at address [kernargPtrRegister] + kernargBufferSize.
  std::vector<BPatch_point *> entryPoints;
  function->getEntryPoints(entryPoints);

  for (auto &kernelInfo : kernelInfos) {
    if (kernelInfo.getKernelName() == function->getMangledName()) {

      auto prologuePtr = boost::make_shared<AmdgpuPrologue>(94, kernelInfo.kernargPtrRegister,
                                                            kernelInfo.kernargBufferSize);

      AstNodePtr prologueNodePtr = boost::make_shared<AmdgpuPrologueNode>(prologuePtr);

      AmdgpuPrologueSnippet prologueSnippet(prologueNodePtr);
      insertPrologueAtPoints(prologueSnippet, entryPoints);
      break;
    }
  }
}

void AmdgpuInternalImpl::insertEpilogueAtPoints(AmdgpuEpilogueSnippet &snippet,
                                                std::vector<BPatch_point *> &points) {
  for (size_t i = 0; i < points.size(); ++i) {
    instPoint *iPoint = static_cast<instPoint *>(points[i]->getPoint(BPatch_callAfter));
    iPoint->pushBack(snippet.ast_wrapper, SnippetType::EPILOGUE);
  }
}

void AmdgpuInternalImpl::insertEpilogueIfKernel(BPatch_function *function) {
  // Go through information for instrumented kernels (functions) and insert a s_dcache_wb
  // instruction at exit points.
  std::vector<BPatch_point *> exitPoints;
  function->getExitPoints(exitPoints);

  for (auto &kernelInfo : kernelInfos) {
    if (kernelInfo.getKernelName() == function->getMangledName()) {

      auto epiloguePtr = boost::make_shared<AmdgpuEpilogue>();

      AstNodePtr epilogueNodePtr = boost::make_shared<AmdgpuEpilogueNode>(epiloguePtr);

      AmdgpuEpilogueSnippet epilogueSnippet(epilogueNodePtr);
      insertEpilogueAtPoints(epilogueSnippet, exitPoints);
      break;
    }
  }
}
// INTERNAL-IMPL-END

static std::unordered_set<BPatch_function *> instrumentedFunctions = {};
static std::vector<AmdgpuKernelInfo> kernelInfos = {};
