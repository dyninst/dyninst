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

#include "ASTs/AmdgpuEpilogue.h"
#include "emit-amdgpu.h"
#include "mapped_object.h"

namespace Dyninst { namespace DyninstAPI {

// Similar approach to prologue
bool AmdgpuEpilogue::generate(Dyninst::PatchAPI::Point *point, Dyninst::Buffer &buffer) {
  // We need 8 bytes for the epilogue (a s_dcache_wb instruction).
  codeGen gen(20);

  // Use the emitter for the instrumented object's architecture (gfx908/gfx940/...): setAddrSpace installs
  // the arch-correct emitter via AddressSpace::getEmitter(); emitScalarDataCacheWriteback is on the base.
  auto *obj = dynamic_cast<mapped_object *>(point->block()->obj());
  assert(obj && "AMDGPU epilogue: expected a mapped_object");
  gen.setAddrSpace(obj->proc());
  auto *emitter = dynamic_cast<EmitterAmdgpu *>(gen.emitter());
  assert(emitter && "AMDGPU epilogue: expected an EmitterAmdgpu");

  emitter->emitScalarDataCacheWriteback(gen);

  buffer.copy(gen.start_ptr(), gen.used());

  return true;
}

}}
