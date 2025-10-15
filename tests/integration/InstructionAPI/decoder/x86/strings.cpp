#include "memory_tests.h"
#include "register_tests.h"
#include "Architecture.h"
#include "InstructionDecoder.h"
#include "opcode_tests.h"
#include "Register.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  Intel 64 and IA-32 Architectures Software Developer’s Manual (SDM)
 *  June 2025
 *  5.1.8 String Instructions
 *
 *  This also includes the string-like I/O instructions (e.g., 'insb').
 */

namespace di = Dyninst::InstructionAPI;

struct strings_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<strings_test> make_tests32();
static std::vector<strings_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<strings_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests32());

  if(!run(Dyninst::Arch_x86_64, make_tests64())) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<strings_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'strings' in " << sarch << " mode\n";
  for(auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.regs)) {
      failed = true;
    }
    if(!di::verify(insn, t.mem)) {
      failed = true;
    }
    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }

    std::clog << "\n";
  }
  return !failed;
}

std::vector<strings_test> make_tests32() {
  auto al = Dyninst::x86::al;
  auto dx = Dyninst::x86::dx;

  auto eax = Dyninst::x86::eax;
  auto ecx = Dyninst::x86::ecx;
  auto edi = Dyninst::x86::edi;
  auto esi = Dyninst::x86::esi;

  auto es = Dyninst::x86::es;
  auto ds = Dyninst::x86::ds;

  auto eflags = Dyninst::x86::flags;
  auto af = Dyninst::x86::af;
  auto cf = Dyninst::x86::cf;
  auto df = Dyninst::x86::df;
  auto of = Dyninst::x86::of;
  auto pf = Dyninst::x86::pf;
  auto sf = Dyninst::x86::sf;
  auto zf = Dyninst::x86::zf;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // movsb byte ptr es:[edi], byte ptr ds:[esi]
      {0xa4},
      di::opcode_test(e_movsb, "movsb (%esi),edi(%es)"),  // WRONG: 'movsb  (%esi), %es:(%edi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df},
        reg_set{edi, esi}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{edi, es}
        }
      }
    },
    { // movsd dword ptr es:[edi], dword ptr ds:[esi]
      {0xa5},
      di::opcode_test(e_movsd, "movsd (%esi),edi(%es)"),  // WRONG: 'movsd (%esi), %es:(%edi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df},
        reg_set{edi, esi}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{edi, es}
        }
      }
    },
    { // REP movsd dword ptr es:[edi], dword ptr ds:[esi]
      {0xf3, 0xa5},
      di::opcode_test(e_movsd, "rep movsd (%esi),edi(%es)"),  // WRONG: 'rep movsl (%esi), %es:(%edi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df, ecx},
        reg_set{edi, esi, ecx}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{edi, es}
        }
      }
    },
    { // cmpsb byte ptr es:[edi], byte ptr ds:[esi]
      {0xa6},
      di::opcode_test(e_cmpsb, "cmpsb edi(%es),(%esi)"),  // WRONG: 'cmpsb %es:(%edi), (%esi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df},
        reg_set{edi, esi, eflags, cf, of, sf, zf, af, pf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi, edi, es},
          reg_set{}
        }
      }
    },
    { // cmpsd dword ptr es:[edi], dword ptr ds:[esi]
      {0xa7},
      di::opcode_test(e_cmpsd, "cmpsd edi(%es),(%esi)"), // WRONG: 'cmpsl %es:(%edi), (%esi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df},
        reg_set{edi, esi, eflags, cf, of, sf, zf, af, pf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi, edi, es},
          reg_set{}
        }
      }
    },
    { // REPE cmpsd dword ptr es:[edi], dword ptr ds:[esi]
      {0xf3, 0xa7},
      di::opcode_test(e_cmpsd, "repe cmpsd edi(%es),(%esi)"), // WRONG: 'repe cmpsl %es:(%edi), (%esi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df, ecx},
        reg_set{edi, esi, eflags, cf, of, sf, zf, af, pf, ecx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi, edi, es},
          reg_set{}
        }
      }
    },
    { // REPNE cmpsd dword ptr es:[edi], dword ptr ds:[esi]
      {0xf2, 0xa7},
      di::opcode_test(e_cmpsd, "repne cmpsd edi(%es),(%esi)"),  // WRONG: 'repne cmpsl %es:(%edi), (%esi)'
      di::register_rw_test{
        reg_set{edi, ds, esi, es, eflags, df, ecx},
        reg_set{edi, esi, eflags, cf, of, sf, zf, af, pf, ecx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi, edi, es},
          reg_set{}
        }
      }
    },
    { // scasb byte ptr es:[edi], ax
      {0xae},
      di::opcode_test(e_scasb, "scasb edi(%es),%al"), // WRONG: 'scasb %es:(%edi), %al'
      di::register_rw_test{
        reg_set{edi, es, al, eflags, df},
        reg_set{eflags, edi, of, sf, zf, af, pf, cf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{edi, es},
          reg_set{}
        }
      }
    },
    { // scasd dword ptr es:[edi], eax
      {0xaf},
      di::opcode_test(e_scasd, "scasd edi(%es),%eax"),  // WRONG: 'scasl %es:(%edi), %eax'
      di::register_rw_test{
        reg_set{edi, es, eax, eflags, df},
        reg_set{eflags, edi, of, sf, zf, af, pf, cf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{edi, es},
          reg_set{}
        }
      }
    },
    { // REPE scasd dword ptr es:[edi], eax
      {0xf3, 0xaf},
      di::opcode_test(e_scasd, "repe scasd edi(%es),%eax"), // WRONG: 'repe scasl %es:(%edi), %eax'
      di::register_rw_test{
        reg_set{edi, es, eax, eflags, df, ecx},
        reg_set{eflags, edi, of, sf, zf, af, pf, cf, ecx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{edi, es},
          reg_set{}
        }
      }
    },
    { // REPNE scasd dword ptr es:[edi], eax
      {0xf2, 0xaf},
      di::opcode_test(e_scasd, "repne scasd edi(%es),%eax"),  // WRONG: 'repne scasl %es:(%edi), %eax'
      di::register_rw_test{
        reg_set{edi, es, eax, eflags, df, ecx},
        reg_set{eflags, edi, of, sf, zf, af, pf, cf, ecx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{edi, es},
          reg_set{}
        }
      }
    },
    { // lodsb al, byte ptr ds:[esi]
      {0xac},
      di::opcode_test(e_lodsb, "lodsb (%esi),%al"),
      di::register_rw_test{
        reg_set{esi, ds, eflags, df},
        reg_set{al, esi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{}
        }
      }
    },
    { // lodsd eax, dword ptr ds:[esi]
      {0xad},
      di::opcode_test(e_lodsd, "lodsd (%esi),%eax"),
      di::register_rw_test{
        reg_set{esi, ds, eflags, df},
        reg_set{eax, esi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{}
        }
      }
    },
    { // REP lodsd eax, dword ptr ds:[esi]
      {0xf3, 0xad},
      di::opcode_test(e_lodsd, "rep lodsd (%esi),%eax"),
      di::register_rw_test{
        reg_set{esi, ds, eflags, df, ecx},
        reg_set{eax, esi, ecx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{}
        }
      }
    },
    { // stosb byte ptr es:[edi], al
      {0xaa},
      di::opcode_test(e_stosb, "stosb %al,edi(%es)"), // WRONG: 'stosb %al, %es:(%edi)'
      di::register_rw_test{
        reg_set{al, edi, es, df, eflags},
        reg_set{edi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi, es}
        }
      }
    },
    { // stosd dword ptr es:[edi], eax
      {0xab},
      di::opcode_test(e_stosd, "stosd %eax,edi(%es)"), // WRONG: 'stosl %eax, %es:(%edi)'
      di::register_rw_test{
        reg_set{eax, edi, es, df, eflags},
        reg_set{edi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi, es}
        }
      }
    },
    { // REP stosd dword ptr es:[edi], eax
      {0xf3, 0xab},
      di::opcode_test(e_stosd, "rep stosd %eax,edi(%es)"), // WRONG: 'rep stosl %eax, %es:(%edi)'
      di::register_rw_test{
        reg_set{eax, edi, es, df, eflags, ecx},
        reg_set{edi, ecx}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi, es}
        }
      }
    },
    { // insb byte ptr es:[edi]
      {0x6c},
      di::opcode_test(e_insb, "insb %dx,edi(%es)"), // WRONG: 'insb %dx, %es:(%edi)'
      di::register_rw_test{
        reg_set{edi, es, df, eflags, dx},
        reg_set{edi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi, es}
        }
      }
    },
    { // insd dword ptr es:[edi]
      {0x6d},
      di::opcode_test(e_insd, "insd %dx,edi(%es)"), // WRONG: 'insl %dx, %es:(%edi)'
      di::register_rw_test{
        reg_set{edi, es, df, eflags, dx},
        reg_set{edi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi, es}
        }
      }
    },
    { // REP insd dword ptr es:[edi]
      {0xf3, 0x6d},
      di::opcode_test(e_insd, "rep insd %dx,edi(%es)"), // WRONG: 'rep insl %dx, %es:(%edi)'
      di::register_rw_test{
        reg_set{edi, es, df, eflags, ecx, dx},
        reg_set{edi, ecx}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi, es}
        }
      }
    },
    { // outsb byte ptr es:[edi]
      {0x6e},
      di::opcode_test(e_outsb, "outsb (%esi),%dx"), // WRONG: 'outsb (%esi), %dx'
      di::register_rw_test{
        reg_set{esi, ds, df, eflags, dx},
        reg_set{esi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{}
        }
      }
    },
    { // outsd dword ptr es:[edi]
      {0x6f},
      di::opcode_test(e_outsd, "outsd (%esi),%dx"), // WRONG: 'outsl (%esi), %dx'
      di::register_rw_test{
        reg_set{esi, ds, df, eflags, dx},
        reg_set{esi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{}
        }
      }
    },
    { // repne outsd dword ptr es:[edi]
      {0xf2, 0x6f},
      di::opcode_test(e_outsd, "repne outsd (%esi),%dx"), // WRONG: 'repne outsl (%esi), %dx'
      di::register_rw_test{
        reg_set{esi, ds, df, eflags, dx, ecx},
        reg_set{esi, ecx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{esi},
          reg_set{}
        }
      }
    },
    // clang-format on
  };
}

std::vector<strings_test> make_tests64() {
  auto dx = Dyninst::x86_64::dx;
  auto eax = Dyninst::x86_64::eax;
  auto rcx = Dyninst::x86_64::rcx;
  auto rax = Dyninst::x86_64::rax;
  auto rdi = Dyninst::x86_64::rdi;
  auto rsi = Dyninst::x86_64::rsi;

  auto rflags = Dyninst::x86_64::flags;
  auto af = Dyninst::x86_64::af;
  auto cf = Dyninst::x86_64::cf;
  auto df = Dyninst::x86_64::df;
  auto of = Dyninst::x86_64::of;
  auto pf = Dyninst::x86_64::pf;
  auto sf = Dyninst::x86_64::sf;
  auto zf = Dyninst::x86_64::zf;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { //  movsd dword ptr es:[rdi], dword ptr [rsi]
      {0xa5},
      di::opcode_test(e_movsd, "movsd (%rsi),(%rdi)"), // WRONG: 'movsl (%esi), %es:(%edi)'
      di::register_rw_test{
        reg_set{rdi, rsi, rflags, df},
        reg_set{rdi, rsi}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{rdi}
        }
      }
    },
    { //  REX.W movsq qword ptr [rdi], qword ptr [rsi]
      {0x48, 0xa5},
      di::opcode_test(e_movsq, "movsq (%rsi),(%rdi)"),
      di::register_rw_test{
        reg_set{rdi, rsi, rflags, df},
        reg_set{rdi, rsi}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{rdi}
        }
      }
    },
    { // cmpsd dword ptr [rdi], dword ptr [rsi]
      {0xa7},
      di::opcode_test(e_cmpsd, "cmpsd (%rdi),(%rsi)"),
      di::register_rw_test{
        reg_set{rdi, rsi, rflags, df},
        reg_set{rdi, rsi, rflags, cf, of, sf, zf, af, pf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi, rdi},
          reg_set{}
        }
      }
    },
    { // REX.W cmpsq qword ptr [rdi], qword ptr [rsi]
      {0x48, 0xa7},
      di::opcode_test(e_cmpsq, "cmpsq (%rdi),(%rsi)"),
      di::register_rw_test{
        reg_set{rdi, rsi, rflags, df},
        reg_set{rdi, rsi, rflags, cf, of, sf, zf, af, pf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi, rdi},
          reg_set{}
        }
      }
    },
    { // REPE REX.W cmpsq qword ptr [rdi], qword ptr [rsi]
      {0Xf3, 0x48, 0xa7},
      di::opcode_test(e_cmpsq, "repe cmpsq (%rdi),(%rsi)"),
      di::register_rw_test{
        reg_set{rdi, rsi, rflags, df, rcx},
        reg_set{rdi, rsi, rflags, cf, of, sf, zf, af, pf, rcx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi, rdi},
          reg_set{}
        }
      }
    },
    { // REPNE REX.W cmpsq qword ptr [rdi], qword ptr [rsi]
      {0Xf2, 0x48, 0xa7},
      di::opcode_test(e_cmpsq, "repne cmpsq (%rdi),(%rsi)"),
      di::register_rw_test{
        reg_set{rdi, rsi, rflags, df, rcx},
        reg_set{rdi, rsi, rflags, cf, of, sf, zf, af, pf, rcx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi, rdi},
          reg_set{}
        }
      }
    },
    { // scasd dword ptr [edi], eax
      {0xaf},
      di::opcode_test(e_scasd, "scasd (%rdi),%eax"),
      di::register_rw_test{
        reg_set{rdi, eax, rflags, df},
        reg_set{rflags, rdi, of, sf, zf, af, pf, cf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // REX.W scasq qword ptr [rdi], rax
      {0x48, 0xaf},
      di::opcode_test(e_scasq, "scasq (%rdi),%rax"),
      di::register_rw_test{
        reg_set{rdi, rax, rflags, df},
        reg_set{rflags, rdi, of, sf, zf, af, pf, cf}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // REPE REX.W scasq qword ptr [rdi], rax
      {0Xf3, 0x48, 0xaf},
      di::opcode_test(e_scasq, "repe scasq (%rdi),%rax"),
      di::register_rw_test{
        reg_set{rdi, rax, rflags, df, rcx},
        reg_set{rflags, rdi, of, sf, zf, af, pf, cf, rcx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // REPNE REX.W scasq qword ptr [edi], rax
      {0Xf2, 0x48, 0xaf},
      di::opcode_test(e_scasq, "repne scasq (%rdi),%rax"),
      di::register_rw_test{
        reg_set{rdi, rax, rflags, df, rcx},
        reg_set{rflags, rdi, of, sf, zf, af, pf, cf, rcx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // lodsd eax, dword ptr [rsi]
      {0xad},
      di::opcode_test(e_lodsd, "lodsd (%rsi),%eax"),
      di::register_rw_test{
        reg_set{rsi, rflags, df},
        reg_set{eax, rsi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{}
        }
      }
    },
    { // REX.W lodsq rax, qword ptr ds:[rsi]
      {0x48, 0xad},
      di::opcode_test(e_lodsq, "lodsq (%rsi),%rax"),
      di::register_rw_test{
        reg_set{rsi, rflags, df},
        reg_set{rax, rsi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{}
        }
      }
    },
    { // stosd dword ptr [rdi], eax
      {0xab},
      di::opcode_test(e_stosd, "stosd %eax,(%rdi)"),
      di::register_rw_test{
        reg_set{eax, rdi, df, rflags},
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
    { // REX.W stosq qord ptr [rdi], rax
      {0x48, 0xab},
      di::opcode_test(e_stosq, "stosq %rax,(%rdi)"),
      di::register_rw_test{
        reg_set{rax, rdi, df, rflags},
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
    { // insd dword ptr [rdi]
      {0x6d},
      di::opcode_test(e_insd, "insd %dx,(%rdi)"),
      di::register_rw_test{
        reg_set{rdi, df, rflags, dx},
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
    { // REX.W insq qword ptr [rdi]
      {0x48, 0x6d},
      di::opcode_test(e_insd, "insd %dx,(%rdi)"),
      di::register_rw_test{
        reg_set{rdi, df, rflags, dx},
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
    { // REX.W repne insq qword ptr [rdi]
      {0xf2, 0x48, 0x6d},
      di::opcode_test(e_insd, "repne insd %dx,(%rdi)"),
      di::register_rw_test{
        reg_set{rdi, df, rflags, rcx, dx},
        reg_set{rdi, rcx}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
    { // outsd dword ptr [rsi]
      {0x6f},
      di::opcode_test(e_outsd, "outsd (%rsi),%dx"),
      di::register_rw_test{
        reg_set{rsi, df, rflags, dx},
        reg_set{rsi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{}
        }
      }
    },
    { // REX.W outsq qword ptr [rsi]
      {0x48, 0x6f},
      di::opcode_test(e_outsd, "outsd (%rsi),%dx"),
      di::register_rw_test{
        reg_set{rsi, df, rflags, dx},
        reg_set{rsi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{}
        }
      }
    },
    { // rep REX.W outsq qword ptr [rsi]
      {0xf2, 0x48, 0x6f},
      di::opcode_test(e_outsd, "repne outsd (%rsi),%dx"),
      di::register_rw_test{
        reg_set{rsi, df, rflags, dx, rcx},
        reg_set{rsi, rcx}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rsi},
          reg_set{}
        }
      }
    },
    // clang-format on
  };
}
