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
#include "instPoint.h"

#include <cassert>

namespace Dyninst {

void AmdgpuGfx908PointHandler::handlePoints(std::vector<BPatch_point *> const &points) {
  for (BPatch_point *point : points) {
    BPatch_function *f = point->getFunction();
    auto result = instrumentedFunctions.insert(f);
    if (result.second) {
      insertPrologueIfKernel(f);
      insertEpilogueIfKernel(f);
    }
  }
}

bool AmdgpuGfx908PointHandler::isKernel(BPatch_function *f) {
  // The kernel descriptor symbols have global visibility. So Dyninst will see them as global
  // variables.

  std::string kdName = f->getMangledName() + ".kd";

  BPatch_module *m = f->getModule();
  BPatch_variableExpr* kernelDescriptor = m->findVariable(kdName.c_str());

  return kernelDescriptor;
}

void AmdgpuGfx908PointHandler::insertPrologueIfKernel(BPatch_function *function) {
  // Go through kernel descriptors for instrumented kernels (functions) and insert a prologue
  // that loads s[94:95] with address of memory for instrumentation variables. This address
  // is at address [kernargPtrRegister] + kernargBufferSize.

  if (!isKernel(function))
    return;

  std::vector<BPatch_point *> entryPoints;
  function->getEntryPoints(entryPoints);

  auto prologuePtr = boost::make_shared<AmdgpuPrologue>(
        94, /* kernargPtrRegister = */4, /* kernargBufferSize = */288);

  AstNodePtr prologueNodePtr =
        boost::make_shared<AmdgpuPrologueNode>(prologuePtr);

  AmdgpuPrologueSnippet prologueSnippet(prologueNodePtr);
  insertPrologueAtPoints(prologueSnippet, entryPoints);
}

void AmdgpuGfx908PointHandler::insertEpilogueIfKernel(BPatch_function *function) {
  // Go through information for instrumented kernels (functions) and insert a s_dcache_wb
  // instruction at exit points.
  if (!isKernel(function))
    return;

  std::vector<BPatch_point *> exitPoints;
  function->getExitPoints(exitPoints);

  auto epiloguePtr = boost::make_shared<AmdgpuEpilogue>();

  AstNodePtr epilogueNodePtr = boost::make_shared<AmdgpuEpilogueNode>(epiloguePtr);

  AmdgpuEpilogueSnippet epilogueSnippet(epilogueNodePtr);
  insertEpilogueAtPoints(epilogueSnippet, exitPoints);
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

} // namespace Dyninst
