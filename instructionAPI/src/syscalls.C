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

#include "BinaryFunction.h"
#include "Dereference.h"
#include "Immediate.h"
#include "Register.h"
#include "Result.h"
#include "Visitor.h"
#include "debug.h"
#include "registers/x86_regs.h"
#include "syscalls.h"

namespace di = Dyninst::InstructionAPI;

namespace x86 {

  /* Thread Control Block syscall visitor
   *
   *  Used to detect 'call gs:[0x10]' syscalls in 32-bit code.
   */
  struct tcb_syscall_visitor : di::Visitor {
    di::Result value;
    int num_imm{0};
    bool valid{true};
    bool found_deref{false};

    void visit(di::BinaryFunction*) override { valid = false; }

    void visit(di::Immediate* imm) override {
      num_imm++;
      value = imm->eval();
    }

    void visit(di::RegisterAST*) override { valid = false; }

    void visit(di::Dereference*) override { found_deref = true; }
  };

  namespace {
    di::RegisterAST::Ptr gs(new di::RegisterAST(Dyninst::x86::gs));
  }

  /* Check for system call idioms
   *
   * Idioms checked:
   *
   *  syscall
   *  int <vector> (e.g., 'int 0x80')
   *  call DWORD PTR gs:[0x10]
   *
   *  Calls to Linux vdso functions (e.g., __vdso_clock_gettime)
   *  are not considered system calls because they use the standard
   *  call mechanism to explicitly bypass the kernel.
   *
   *  'sysenter' is checked by IA_IAPI::isSysEnter().
   */
  bool isSystemCall(di::Instruction const& ins) {
    auto const id = ins.getOperation().getID();

    /*
     *  'syscall' is only available in 64-bit mode, but is the most likely
     *  instruction to be encountered for system calls so check it first.
     */
    if(id == e_syscall) {
      return true;
    }

    /* Software interrupts
     *
     *  All interrupts that specify a vector go directly through the kernel,
     *  so they are system calls.
     *
     *  Interrupts for traps/breakpoints (notable int3, int1, and into) don't
     *  directly go through the kernel, so aren't considered system calls.
     */
    if(id == e_int) {
      return true;
    }

    /* !! Everything below here is for 32-bit code only !!
     *
     *  Although technically possible, these idioms are very unlikely to
     *  be encountered in 64-bit mode. Because they use generic instructions
     *  like 'call', we don't want to introduce the overhead of checking them
     *  unless it's truly necessary.
     */
    if(ins.getArch() != Dyninst::Arch_x86) {
      return false;
    }

    /* Check 'call gs:[0x10]'
     *
     *  In some old 32-bit libc's, it was cheaper for the loader to put the
     *  value of the kernel's system call entry point (AT_SYSINFO) into a fixed
     *  location. One particular place was at 'gs:0x10' (in the Thread Control Block).
     *  That value was likely taken from https://articles.manugarg.com/systemcallinlinux2_6.html.
     *  In this case, 'call gs:[0x10]' is really a system call.
     *
     *  Dyninst doesn't correctly represent segment registers in AST, so the instruction
     *  it produces is 'call [0x10]'. This isn't something a compiler would generate,
     *  so we'll consider it as our special case here.
     */
    if(id == e_call) {
      std::vector<di::Operand> operands;
      ins.getOperands(operands);

      if(operands.size() != 1) {
        di::decode_printf("[%s:%d] Incorrect number of arguments to 'call'. Found %lu, expected 1\n",
                          __FILE__, __LINE__, operands.size());
        return false;
      }

      auto in = ins; // 'isRead' isn't const
      if(!in.getOperation().isRead(gs)) {
        return false;
      }

      tcb_syscall_visitor v;
      operands[0].getValue()->apply(&v);

      // The addressing mode should have a single dereference and one immediate value
      if(!v.valid || !v.found_deref || v.num_imm != 1) {
        return false;
      }

      // That value should be 0x10.
      auto result = v.value;
      if(result.defined && result.convert<int64_t>() == 0x10) {
        return true;
      }
    }

    return false;
  }
}

namespace ppc {
  bool isSystemCall(di::Instruction const& ins) {
    auto const id = ins.getOperation().getID();

    // There is also a vectorized form (scv), but Dyninst can't decode it.
    return id == power_op_sc;
  }
}

namespace aarch64 {
  bool isSystemCall(di::Instruction const& ins) {
    auto const id = ins.getOperation().getID();

    return id == aarch64_op_svc;
  }
}

bool di::isSystemCall(Instruction const& ins) {
  switch(ins.getArch()) {
    case Arch_x86:
    case Arch_x86_64:
      return ::x86::isSystemCall(ins);
    case Arch_ppc32:
    case Arch_ppc64:
      return ::ppc::isSystemCall(ins);
    case Arch_aarch64:
      return ::aarch64::isSystemCall(ins);
    case Arch_none:
    case Arch_aarch32:
    case Arch_cuda:
    case Arch_amdgpu_gfx908:
    case Arch_amdgpu_gfx90a:
    case Arch_amdgpu_gfx940:
    case Arch_intelGen9:
      return false;
  }
  return false;
}
