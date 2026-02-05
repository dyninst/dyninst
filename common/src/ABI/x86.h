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

#ifndef DYNINST_COMMON_X86_ABI_H
#define DYNINST_COMMON_X86_ABI_H

#include "registers/x86_regs.h"
#include "ABI/architecture.h"

namespace Dyninst { namespace abi {

/* i386 ABI
 *
 * System V Application Binary Interface
 * Intel386 Architecture Processor Supplement
 * Version 1.0
 * February 3, 2015
 * Section 2.2 Function Calling Sequence
 */
architecture make_x86() {
  architecture arch;
  arch.address_width = getArchAddressWidth(Arch_x86);

  arch.function.params = {
    /*
    * For purposes of parameter passing and function return,
    * xmmN and ymmN refer to the same register. Only one of
    * them can be used at the same time.
    */
    x86::xmm0,  // __m128
    x86::xmm1,
    x86::xmm2,
    x86::ymm0,  // __m256
    x86::ymm1,
    x86::ymm2,
    x86::mm0,   // __m64
    x86::mm1,
    x86::mm2,
    x86::edx,   // used by fastcall convention
    x86::ecx    // used by fastcall convention
  };

  arch.function.returns = {
    x86::eax,   // ints, pointers, address of struct/union
    x86::edx,   // upper 32 bits of some 64-bit return types
    x86::xmm0,  // __m128
    x86::ymm0,  // __m256
    x86::mm0,   // __m64
    x86::st0    // float, double, long double, __float80
  };

  arch.function.preserved = {
    x86::ebx,
    x86::esp,
    x86::ebp,     // can be used as frame pointer
    x86::esi,
    x86::edi,
    x86::mxcsr,   // only control bits, but we don't break it down
    x86::fcw      // x87 control word
  };

  arch.function.globals = {
    /*
    * The direction flag DF in the %EFLAGS register must be clear (set to “forward”
    * direction) on function entry and return. Other user flags have no specified role
    * in the standard calling sequence and are not preserved across calls.
    */
    x86::df,
    x86::gs,    // Reserved for system (as thread specific data register)
    x86::ebx    // Used to hold GOT pointer when making calls via PLT
  };

  /*
  * Emulated IA32 system calls via `int 0x80` for Linux v4.1
  *
  *   https://github.com/torvalds/linux/blob/v4.1/arch/x86/ia32/ia32entry.S#L498
  *
  * Support for IA32 system calls was removed in Linux 4.2
  *
  *  We only care about parameters here because the caller must save all registers
  *  except eax, and there are no return values or globals.
  */
  arch.syscall.params = {
    x86::eax,   // system call number
    x86::ebx,   // arg1
    x86::ecx,   // arg2
    x86::edx,   // arg3
    x86::esi,   // arg4
    x86::edi,   // arg5
    x86::ebp    // arg6
  };

#ifdef os_windows
#  error "Windows ABI is unimplemented"
#endif

  return arch;
  }
}}

#endif
