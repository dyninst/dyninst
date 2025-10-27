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

#include "AmdgpuPrologue.h"
#include "emit-amdgpu.h"

bool AmdgpuPrologue::generate(Dyninst::PatchAPI::Point * /* point */, Dyninst::Buffer &buffer) {
  // To avoid any code duplication or refactoring right now, we use a 'codeGen'
  // object to generate the code and copy what we get there into the
  // 'Dyninst::Buffer' object passed here.

  // We need 12 bytes for the prologue (a s_load_dwordx2, followed by a waitcnt)
  codeGen gen(20);
  EmitterAmdgpuGfx908 emitter;

  emitter.emitLoadRelative(dest_, offset_, base_, /* size= */ 2, gen);

  buffer.copy(gen.start_ptr(), gen.used());

  return true;
}
