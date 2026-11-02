/*
 * See the dyninst/COPYRIGHT file for copyright information.
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

#ifndef DYNINST_DYNINSTAPI_BASETRAMP_AMDGPU_H
#define DYNINST_DYNINSTAPI_BASETRAMP_AMDGPU_H

#include "trampolines/baseTramp.h"

// SCC is live across an `s_cmp ... ; s_cbranch_scc*` pair. A snippet spliced between them can clobber
// SCC (any s_add/s_and/s_cmp/... it emits sets SCC), silently corrupting the branch. The base trampoline
// did not preserve SCC. generateSaves/Restores now save/restore SCC around the snippet body -- but ONLY
// when SCC is live at the point (Dyninst now tracks SCC in liveness), so SCC-dead points pay nothing.
class baseTramp_amdgpu final : public baseTramp {
public:
  bool generateSaves(codeGen &, registerSpace *) override;
  bool generateRestores(codeGen &, registerSpace *) override;

private:
  // Scratch SGPR id holding the saved SCC between generateSaves and generateRestores;
  // 0xFFFFFFFF = SCC was not saved (dead, or no scratch SGPR available).
  unsigned sccSaveReg_ = 0xFFFFFFFFu;
};

#endif
