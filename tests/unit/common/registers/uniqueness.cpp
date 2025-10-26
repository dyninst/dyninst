#include "registers/MachRegister.h"
#include "Architecture.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>

/*
 * Ensure all registers for an architecture have unique representations
 */

// Some architectures have known aliases, so exclude them.

static std::vector<char const*> riscv64_aliases();

using alias_map = std::map<Dyninst::Architecture, std::vector<char const*>>;
alias_map known_aliases = {
  {Dyninst::Arch_aarch64,
    {
      "aarch64::x29", "aarch64::fp",
      "aarch64::x30", "aarch64::lr",
      "aarch64::Ip0", "aarch64::x16",
      "aarch64::Ip1", "aarch64::x17",
    }
  },
  {Dyninst::Arch_x86, {}},
  {Dyninst::Arch_x86_64, {}},
  {Dyninst::Arch_ppc64, {}},
  {Dyninst::Arch_amdgpu_gfx908, {}},
  {Dyninst::Arch_amdgpu_gfx90a, {}},
  {Dyninst::Arch_amdgpu_gfx940, {}},
  {Dyninst::Arch_riscv64, { riscv64_aliases() }},
};

bool check(Dyninst::Architecture arch) {
  auto const& regs = Dyninst::MachRegister::getAllRegistersForArch(arch);

  auto known_alias = [arch](std::string const& name) {
    auto const& aliases = known_aliases[arch];
    return std::find(aliases.begin(), aliases.end(), name) != aliases.end();
  };

  auto success = true;

  std::unordered_map<unsigned int, std::string> vals;
  for(auto r : regs) {
    auto res = vals.insert({r.val(), r.name()});
    auto itr = res.first;
    auto inserted = res.second;
    if(!inserted && !known_alias(r.name())) {
      auto val = itr->first;
      auto name = itr->second;
      std::cerr << "alias: 0x" << std::hex << val
                << "  " << r.name() << "\n";
      success = false;
    }
  }

  return success;
}


int main() {
  auto status = EXIT_SUCCESS;
  
  if(!check(Dyninst::Arch_aarch64)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_x86)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_x86_64)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_ppc64)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_amdgpu_gfx908)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_amdgpu_gfx90a)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_amdgpu_gfx940)) {
    status = EXIT_FAILURE;
  }
  if(!check(Dyninst::Arch_riscv64)) {
    status = EXIT_FAILURE;
  }

  return status;
}

std::vector<char const*> riscv64_aliases() {
  return {
    "riscv64::zero",     "riscv64::x0",
    "riscv64::ra",       "riscv64::x1",
    "riscv64::sp",       "riscv64::x2",
    "riscv64::gp",       "riscv64::x3",
    "riscv64::tp",       "riscv64::x4",
    "riscv64::t0",       "riscv64::x5",
    "riscv64::t1",       "riscv64::x6",
    "riscv64::t2",       "riscv64::x7",
    "riscv64::fp",       "riscv64::x8",
    "riscv64::s0",       "riscv64::x8",
    "riscv64::s1",       "riscv64::x9",
    "riscv64::a0",       "riscv64::x10",
    "riscv64::a1",       "riscv64::x11",
    "riscv64::a2",       "riscv64::x12",
    "riscv64::a3",       "riscv64::x13",
    "riscv64::a4",       "riscv64::x14",
    "riscv64::a5",       "riscv64::x15",
    "riscv64::a6",       "riscv64::x16",
    "riscv64::a7",       "riscv64::x17",
    "riscv64::s2",       "riscv64::x18",
    "riscv64::s3",       "riscv64::x19",
    "riscv64::s4",       "riscv64::x20",
    "riscv64::s5",       "riscv64::x21",
    "riscv64::s6",       "riscv64::x22",
    "riscv64::s7",       "riscv64::x23",
    "riscv64::s8",       "riscv64::x24",
    "riscv64::s9",       "riscv64::x25",
    "riscv64::s10",      "riscv64::x26",
    "riscv64::s11",      "riscv64::x27",
    "riscv64::t3",       "riscv64::x28",
    "riscv64::t4",       "riscv64::x29",
    "riscv64::t5",       "riscv64::x30",
    "riscv64::t6",       "riscv64::x31",
    "riscv64::ft0",      "riscv64::f0",
    "riscv64::ft1",      "riscv64::f1",
    "riscv64::ft2",      "riscv64::f2",
    "riscv64::ft3",      "riscv64::f3",
    "riscv64::ft4",      "riscv64::f4",
    "riscv64::ft5",      "riscv64::f5",
    "riscv64::ft6",      "riscv64::f6",
    "riscv64::ft7",      "riscv64::f7",
    "riscv64::fs0",      "riscv64::f8",
    "riscv64::fs1",      "riscv64::f9",
    "riscv64::fa0",      "riscv64::f10",
    "riscv64::fa1",      "riscv64::f11",
    "riscv64::fa2",      "riscv64::f12",
    "riscv64::fa3",      "riscv64::f13",
    "riscv64::fa4",      "riscv64::f14",
    "riscv64::fa5",      "riscv64::f15",
    "riscv64::fa6",      "riscv64::f16",
    "riscv64::fa7",      "riscv64::f17",
    "riscv64::fs2",      "riscv64::f18",
    "riscv64::fs3",      "riscv64::f19",
    "riscv64::fs4",      "riscv64::f20",
    "riscv64::fs5",      "riscv64::f21",
    "riscv64::fs6",      "riscv64::f22",
    "riscv64::fs7",      "riscv64::f23",
    "riscv64::fs8",      "riscv64::f24",
    "riscv64::fs9",      "riscv64::f25",
    "riscv64::fs10",     "riscv64::f26",
    "riscv64::fs11",     "riscv64::f27",
    "riscv64::ft8",      "riscv64::f28",
    "riscv64::ft9",      "riscv64::f29",
    "riscv64::ft10",     "riscv64::f30",
    "riscv64::ft11",     "riscv64::f31",
    "riscv64::f0_32",    "riscv64::f0",
    "riscv64::f1_32",    "riscv64::f1",
    "riscv64::f2_32",    "riscv64::f2",
    "riscv64::f3_32",    "riscv64::f3",
    "riscv64::f4_32",    "riscv64::f4",
    "riscv64::f5_32",    "riscv64::f5",
    "riscv64::f6_32",    "riscv64::f6",
    "riscv64::f7_32",    "riscv64::f7",
    "riscv64::f8_32",    "riscv64::f8",
    "riscv64::f9_32",    "riscv64::f9",
    "riscv64::f10_32",   "riscv64::f10",
    "riscv64::f11_32",   "riscv64::f11",
    "riscv64::f12_32",   "riscv64::f12",
    "riscv64::f13_32",   "riscv64::f13",
    "riscv64::f14_32",   "riscv64::f14",
    "riscv64::f15_32",   "riscv64::f15",
    "riscv64::f16_32",   "riscv64::f16",
    "riscv64::f17_32",   "riscv64::f17",
    "riscv64::f18_32",   "riscv64::f18",
    "riscv64::f19_32",   "riscv64::f19",
    "riscv64::f20_32",   "riscv64::f20",
    "riscv64::f21_32",   "riscv64::f21",
    "riscv64::f22_32",   "riscv64::f22",
    "riscv64::f23_32",   "riscv64::f23",
    "riscv64::f24_32",   "riscv64::f24",
    "riscv64::f25_32",   "riscv64::f25",
    "riscv64::f26_32",   "riscv64::f26",
    "riscv64::f27_32",   "riscv64::f27",
    "riscv64::f28_32",   "riscv64::f28",
    "riscv64::f29_32",   "riscv64::f29",
    "riscv64::f30_32",   "riscv64::f30",
    "riscv64::f31_32",   "riscv64::f31",
    "riscv64::f0_64",    "riscv64::f0",
    "riscv64::f1_64",    "riscv64::f1",
    "riscv64::f2_64",    "riscv64::f2",
    "riscv64::f3_64",    "riscv64::f3",
    "riscv64::f4_64",    "riscv64::f4",
    "riscv64::f5_64",    "riscv64::f5",
    "riscv64::f6_64",    "riscv64::f6",
    "riscv64::f7_64",    "riscv64::f7",
    "riscv64::f8_64",    "riscv64::f8",
    "riscv64::f9_64",    "riscv64::f9",
    "riscv64::f10_64",   "riscv64::f10",
    "riscv64::f11_64",   "riscv64::f11",
    "riscv64::f12_64",   "riscv64::f12",
    "riscv64::f13_64",   "riscv64::f13",
    "riscv64::f14_64",   "riscv64::f14",
    "riscv64::f15_64",   "riscv64::f15",
    "riscv64::f16_64",   "riscv64::f16",
    "riscv64::f17_64",   "riscv64::f17",
    "riscv64::f18_64",   "riscv64::f18",
    "riscv64::f19_64",   "riscv64::f19",
    "riscv64::f20_64",   "riscv64::f20",
    "riscv64::f21_64",   "riscv64::f21",
    "riscv64::f22_64",   "riscv64::f22",
    "riscv64::f23_64",   "riscv64::f23",
    "riscv64::f24_64",   "riscv64::f24",
    "riscv64::f25_64",   "riscv64::f25",
    "riscv64::f26_64",   "riscv64::f26",
    "riscv64::f27_64",   "riscv64::f27",
    "riscv64::f28_64",   "riscv64::f28",
    "riscv64::f29_64",   "riscv64::f29",
    "riscv64::f30_64",   "riscv64::f30",
    "riscv64::f31_64",   "riscv64::f31",
  };
}
