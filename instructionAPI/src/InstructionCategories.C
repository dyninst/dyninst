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

#include "InstructionCategories.h"
#include "entryIDs.h"

namespace Dyninst { namespace InstructionAPI {

  InsnCategory entryToCategory(entryID e) {
    switch(e) {
      case aarch64_op_ret:
        return c_ReturnInsn;

      case amdgpu_gfx908_op_S_ENDPGM: // special treatment for endpgm
      case amdgpu_gfx90a_op_S_ENDPGM: // special treatment for endpgm
      case amdgpu_gfx940_op_S_ENDPGM: // special treatment for endpgm
        return c_GPUKernelExitInsn;

      case aarch64_op_bl:
      case aarch64_op_blr:
        return c_CallInsn;

      case aarch64_op_b_uncond:
      case aarch64_op_b_cond:
      case aarch64_op_tbz:
      case aarch64_op_tbnz:
      case aarch64_op_cbz:
      case aarch64_op_cbnz:
      case aarch64_op_br:
      case power_op_b:
      case power_op_bc:
      case power_op_bcctr:
      case power_op_bclr:
#include "amdgpu_branchinsn_table.h"
        return c_BranchInsn;

      case power_op_cmp:
      case power_op_cmpi:
      case power_op_cmpl:
      case power_op_cmpli:
        return c_CompareInsn;

      case aarch64_op_brk:
      case aarch64_op_hlt:
      case aarch64_op_wfe_hint:
      case aarch64_op_wfi_hint:
        return c_SoftwareExceptionInsn;
      default:
      	return c_NoCategory;
    }
  }

}}
