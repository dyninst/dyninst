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

#include "registers/LivenessSet.h"
#include "registers/abi.h"
#include "registers/abstract_regs.h"
#include "registers/aarch64_regs.h"
#include "registers/x86_64_regs.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {

  bool failed = false;

  void expect(bool cond, std::string const& what) {
    if(!cond) {
      std::cerr << "FAILED: " << what << "\n";
      failed = true;
    }
  }

  void check_layout() {
    namespace x = Dyninst::x86_64;
    auto const& abi = Dyninst::ABI::get(Dyninst::Arch_x86_64);

    expect(&abi == &Dyninst::ABI::get(Dyninst::Arch_x86_64), "get() returns a singleton");
    expect(abi.size() > 0, "x86_64 tracks registers");
    expect(abi.getIndex(x::rax) >= 0, "rax is tracked");
    expect(abi.getRegister(abi.getIndex(x::rax)) == x::rax, "getRegister inverts getIndex");

    // Aliases share the bit of their base register.
    expect(abi.getIndex(x::eax) == abi.getIndex(x::rax), "eax aliases rax");
    expect(abi.getIndex(x::al) == abi.getIndex(x::rax), "al aliases rax");
    expect(abi.getIndex(x::xmm3) == abi.getIndex(x::zmm3), "xmm3 aliases zmm3");
    expect(abi.getIndex(x::ymm3) == abi.getIndex(x::zmm3), "ymm3 aliases zmm3");
    expect(abi.getIndex(x::zf) == abi.getIndex(x::flags), "zf aliases flags");
    expect(abi.getIndex(x::rax) != abi.getIndex(x::rbx), "rax and rbx are distinct");

    // Registers of another architecture, and invalid ones, are not tracked.
    expect(abi.getIndex(Dyninst::aarch64::x0) == -1, "aarch64 x0 is not tracked by x86_64");
    expect(abi.getIndex(Dyninst::InvalidReg) == -1, "InvalidReg is not tracked");

    namespace a = Dyninst::aarch64;
    auto const& arm = Dyninst::ABI::get(Dyninst::Arch_aarch64);
    expect(arm.getIndex(a::w0) == arm.getIndex(a::x0), "w0 aliases x0");
    expect(arm.getIndex(a::s0) == arm.getIndex(a::q0), "s0 aliases q0");
  }

  void check_set_algebra() {
    namespace x = Dyninst::x86_64;
    auto const& abi = Dyninst::ABI::get(Dyninst::Arch_x86_64);

    Dyninst::LivenessSet a(abi, Dyninst::register_set{x::rax, x::rbx});
    Dyninst::LivenessSet b(abi, Dyninst::register_set{x::ebx, x::rcx});

    expect(a.contains(x::rax) && a.contains(x::eax) && a.contains(x::ax), "contains by alias");
    expect(!a.contains(x::rcx), "does not contain rcx");
    expect(a.count() == 2, "two registers");

    auto const u = a | b;
    expect(u.count() == 3 && u.contains(x::rcx), "union");

    auto const i = a & b;
    expect(i.count() == 1 && i.contains(x::rbx), "intersection");

    auto const d = a - b;
    expect(d.count() == 1 && d.contains(x::rax), "difference");

    auto const s = a ^ b;
    expect(s.count() == 2 && s.contains(x::rax) && s.contains(x::rcx), "symmetric difference");

    auto const c = ~a;
    expect(c.count() == abi.size() - 2 && !c.contains(x::rax), "complement");
    expect((a | c) == abi.getAllRegs(), "a | ~a is everything");

    expect(i.isSubsetOf(a) && !a.isSubsetOf(b), "subset");
    expect(a.intersects(b) && !d.intersects(b), "intersects");

    auto untracked = a;
    untracked.insert(Dyninst::aarch64::x0);
    expect(untracked == a, "inserting an untracked register changes nothing");

    auto e = a;
    e.erase(x::eax);
    expect(!e.contains(x::rax) && e.count() == 1, "erase by alias");
    e.clear();
    expect(e.none() && e.size() == abi.size(), "clear keeps the layout");

    auto const regs = u.registers();
    expect(regs.size() == 3 && regs.contains(x::rax) && regs.contains(x::rbx) &&
               regs.contains(x::rcx),
           "registers() lists base registers");
    expect(Dyninst::LivenessSet(abi, regs) == u, "register_set round trip");

    std::ostringstream os;
    os << Dyninst::LivenessSet(abi, Dyninst::register_set{x::rcx});
    expect(os.str() == "{rcx}", "printing: " + os.str());
  }

  // Relations every ABI's layout and calling-convention sets must satisfy.
  void check_invariants(Dyninst::Architecture arch) {
    auto const& abi = Dyninst::ABI::get(arch);
    std::string const name = Dyninst::getArchitectureName(arch);

    for(std::size_t i = 0; i < abi.size(); i++) {
      auto const r = abi.getRegister(i);
      expect(r.getBaseRegister() == r, name + ": layout holds only base registers: " + r.name());
      expect(abi.getIndex(r) == static_cast<int>(i), name + ": getIndex inverts getRegister: " + r.name());
    }

    expect(abi.getCallReadRegisters().isSubsetOf(abi.getAllRegs()), name + ": call read");
    expect(!abi.getCallWrittenRegisters().intersects(abi.getCalleeSavedRegisters()),
           name + ": a call clobbers no callee-saved register");
    expect(abi.getParameterRegisters().isSubsetOf(abi.getCallReadRegisters()),
           name + ": a call reads its parameters");
    expect(abi.getReturnRegisters().isSubsetOf(abi.getReturnReadRegisters()),
           name + ": a return reads the return registers");
    expect(abi.getCalleeSavedRegisters().isSubsetOf(abi.getReturnReadRegisters()),
           name + ": a return reads the callee-saved registers");
  }

  // Spot checks of each architecture's calling convention: a parameter, a
  // callee-saved register and a scratch register.
  void check_convention(Dyninst::Architecture arch, Dyninst::MachRegister param,
                        Dyninst::MachRegister saved, Dyninst::MachRegister scratch) {
    auto const& abi = Dyninst::ABI::get(arch);
    std::string const name = Dyninst::getArchitectureName(arch);

    expect(abi.getParameterRegisters().contains(param), name + ": parameter " + param.name());
    expect(abi.getCalleeSavedRegisters().contains(saved), name + ": callee-saved " + saved.name());
    expect(!abi.getCallWrittenRegisters().contains(saved), name + ": call preserves " + saved.name());
    expect(abi.getCallWrittenRegisters().contains(scratch), name + ": caller-saved " + scratch.name());
  }

}

int main() {
  check_layout();
  check_set_algebra();

  for(auto arch : {Dyninst::Arch_x86, Dyninst::Arch_x86_64, Dyninst::Arch_ppc64,
                   Dyninst::Arch_aarch64, Dyninst::Arch_riscv64, Dyninst::Arch_amdgpu_gfx908,
                   Dyninst::Arch_amdgpu_gfx90a, Dyninst::Arch_amdgpu_gfx940,
                   Dyninst::Arch_amdgpu_gfx950}) {
    check_invariants(arch);
  }

  // An architecture whose ABI is not modelled keeps every register live.
  auto const& cuda = Dyninst::ABI::get(Dyninst::Arch_cuda);
  check_invariants(Dyninst::Arch_cuda);
  expect(cuda.size() > 0, "cuda tracks registers");
  expect(cuda.getCallReadRegisters() == cuda.getAllRegs(), "unmodelled: a call reads every register");
  expect(cuda.getReturnReadRegisters() == cuda.getAllRegs(), "unmodelled: a return reads every register");
  check_convention(Dyninst::Arch_x86_64, Dyninst::x86_64::rdi, Dyninst::x86_64::rbx,
                   Dyninst::x86_64::r11);

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
