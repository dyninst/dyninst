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
      "x29", "fp",
      "x30", "lr",
      "Ip0", "x16",
      "Ip1", "x17",
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
    "zero",     "x0",
    "ra",       "x1",
    "sp",       "x2",
    "gp",       "x3",
    "tp",       "x4",
    "t0",       "x5",
    "t1",       "x6",
    "t2",       "x7",
    "fp",       "x8",
    "s0",       "x8",
    "s1",       "x9",
    "a0",       "x10",
    "a1",       "x11",
    "a2",       "x12",
    "a3",       "x13",
    "a4",       "x14",
    "a5",       "x15",
    "a6",       "x16",
    "a7",       "x17",
    "s2",       "x18",
    "s3",       "x19",
    "s4",       "x20",
    "s5",       "x21",
    "s6",       "x22",
    "s7",       "x23",
    "s8",       "x24",
    "s9",       "x25",
    "s10",      "x26",
    "s11",      "x27",
    "t3",       "x28",
    "t4",       "x29",
    "t5",       "x30",
    "t6",       "x31",
    "ft0",      "f0",
    "ft1",      "f1",
    "ft2",      "f2",
    "ft3",      "f3",
    "ft4",      "f4",
    "ft5",      "f5",
    "ft6",      "f6",
    "ft7",      "f7",
    "fs0",      "f8",
    "fs1",      "f9",
    "fa0",      "f10",
    "fa1",      "f11",
    "fa2",      "f12",
    "fa3",      "f13",
    "fa4",      "f14",
    "fa5",      "f15",
    "fa6",      "f16",
    "fa7",      "f17",
    "fs2",      "f18",
    "fs3",      "f19",
    "fs4",      "f20",
    "fs5",      "f21",
    "fs6",      "f22",
    "fs7",      "f23",
    "fs8",      "f24",
    "fs9",      "f25",
    "fs10",     "f26",
    "fs11",     "f27",
    "ft8",      "f28",
    "ft9",      "f29",
    "ft10",     "f30",
    "ft11",     "f31",
    "f0_32",    "f0",
    "f1_32",    "f1",
    "f2_32",    "f2",
    "f3_32",    "f3",
    "f4_32",    "f4",
    "f5_32",    "f5",
    "f6_32",    "f6",
    "f7_32",    "f7",
    "f8_32",    "f8",
    "f9_32",    "f9",
    "f10_32",   "f10",
    "f11_32",   "f11",
    "f12_32",   "f12",
    "f13_32",   "f13",
    "f14_32",   "f14",
    "f15_32",   "f15",
    "f16_32",   "f16",
    "f17_32",   "f17",
    "f18_32",   "f18",
    "f19_32",   "f19",
    "f20_32",   "f20",
    "f21_32",   "f21",
    "f22_32",   "f22",
    "f23_32",   "f23",
    "f24_32",   "f24",
    "f25_32",   "f25",
    "f26_32",   "f26",
    "f27_32",   "f27",
    "f28_32",   "f28",
    "f29_32",   "f29",
    "f30_32",   "f30",
    "f31_32",   "f31",
    "f0_64",    "f0",
    "f1_64",    "f1",
    "f2_64",    "f2",
    "f3_64",    "f3",
    "f4_64",    "f4",
    "f5_64",    "f5",
    "f6_64",    "f6",
    "f7_64",    "f7",
    "f8_64",    "f8",
    "f9_64",    "f9",
    "f10_64",   "f10",
    "f11_64",   "f11",
    "f12_64",   "f12",
    "f13_64",   "f13",
    "f14_64",   "f14",
    "f15_64",   "f15",
    "f16_64",   "f16",
    "f17_64",   "f17",
    "f18_64",   "f18",
    "f19_64",   "f19",
    "f20_64",   "f20",
    "f21_64",   "f21",
    "f22_64",   "f22",
    "f23_64",   "f23",
    "f24_64",   "f24",
    "f25_64",   "f25",
    "f26_64",   "f26",
    "f27_64",   "f27",
    "f28_64",   "f28",
    "f29_64",   "f29",
    "f30_64",   "f30",
    "f31_64",   "f31",
  };
}
