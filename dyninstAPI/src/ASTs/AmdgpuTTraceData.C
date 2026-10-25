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

#include "AmdgpuTTraceData.h"
#include "emit-amdgpu.h"
#include "amdgpu-gfx908-details.h"
#include "registerSpace/registerSpace.h"
#include "patching/instPoint.h"

namespace Dyninst { namespace DyninstAPI {

// Mirror of AmdgpuPrologue::generate's non-scratch branch: build the bytes with a codeGen and copy them
// into the PatchAPI Buffer. The emit is register-only (m0 + a SOPP), so no scratch/liveness setup is
// strictly required; we still hand the codeGen the point's registerSpace for consistency.
bool AmdgpuTTraceData::generate(Dyninst::PatchAPI::Point *point, Dyninst::Buffer &buffer) {
  codeGen gen(16);
  if (instPoint *iP = dynamic_cast<instPoint *>(point)) {
    gen.setPoint(iP);
    gen.setAddrSpace(iP->proc());
    if (registerSpace *rs = registerSpace::actualRegSpace(iP))
      gen.setRegisterSpace(rs);
  }
  if (fromReg_)
    AmdgpuGfx908::emitTTraceDataReg(srcReg_, gen);
  else
    AmdgpuGfx908::emitTTraceData(marker_, gen);
  buffer.copy(gen.start_ptr(), gen.used());
  return true;
}

}}
