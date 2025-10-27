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

// We use std::shared_ptr for prologue and epilogue asts because AstNodePtr is a boost::shared_ptr
#include "boost/shared_ptr.hpp"

#include "AmdgpuEpilogue.h"
#include "AmdgpuPrologue.h"

#include <unordered_set>

namespace Dyninst {
// This implements prologue/epilogue insertion at function entry/exit respectively.
struct AmdgpuGfx908PointHandler : PointHandler {

  std::unordered_set<BPatch_function *> instrumentedFunctions;

  void handlePoints(std::vector<BPatch_point *> const &points);

  bool isKernel(BPatch_function *f);

  void insertPrologueIfKernel(BPatch_function *f);
  void insertEpilogueIfKernel(BPatch_function *f);

  void insertPrologueAtPoints(AmdgpuPrologueSnippet &snippet, std::vector<BPatch_point *> &points);
  void insertEpilogueAtPoints(AmdgpuEpilogueSnippet &snippet, std::vector<BPatch_point *> &points);
};

} // namespace Dyninst
