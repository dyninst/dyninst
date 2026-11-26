#include "CFG.h"
#include "CodeObject.h"
#include "memory_code_source.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

/*
 * The parser's edge for each kind of control-flow instruction: its kind, whether
 * it is conditional, indirect or a tail call, and its target. The code is
 * hand-encoded, so each architecture is tested on any host. Every parsed edge is
 * also checked against the combinations that cannot occur, and type() and
 * interproc() against the kind and flags they are made from.
 *
 * Every combination a writer can produce is below, except a conditional indirect
 * call: the ppc decoder gives a conditional bcctrl no fall-through, so it is an
 * unconditional indirect call. System calls are control flow to the parser only
 * on x86; aarch64 svc and ppc sc are ordinary instructions to it.
 */

namespace {
  namespace pa = Dyninst::ParseAPI;
  namespace mcs = memory_code_source;
  using Dyninst::Address;

  constexpr Address sink = ~Address{0};

  enum class Kind { call, syscall, ret, branch, fallthrough, call_ft, catch_ };

  char const *name(Kind k) {
    switch(k) {
      case Kind::call: return "CALL";
      case Kind::syscall: return "SYS_CALL";
      case Kind::ret: return "RET";
      case Kind::branch: return "BRANCH";
      case Kind::fallthrough: return "FALLTHROUGH";
      case Kind::call_ft: return "CALL_FT";
      case Kind::catch_: return "CATCH";
    }
    return "?";
  }

  struct Edge {
    Kind kind;
    bool cond, ind, tail;
    Address trg;

    bool operator<(Edge const &o) const {
      return std::tie(kind, cond, ind, tail, trg) < std::tie(o.kind, o.cond, o.ind, o.tail, o.trg);
    }
    bool operator==(Edge const &o) const {
      return std::tie(kind, cond, ind, tail, trg) == std::tie(o.kind, o.cond, o.ind, o.tail, o.trg);
    }
  };

  // Shorthands for the expected edges
  Edge call(Address t) { return {Kind::call, false, false, false, t}; }
  Edge call_ind() { return {Kind::call, false, true, false, sink}; }
  Edge call_cond(Address t) { return {Kind::call, true, false, false, t}; }
  Edge syscall() { return {Kind::syscall, false, false, false, sink}; }
  Edge ret() { return {Kind::ret, false, true, false, sink}; }
  Edge ret_cond() { return {Kind::ret, true, true, false, sink}; }
  Edge jump(Address t) { return {Kind::branch, false, false, false, t}; }
  Edge jump_ind() { return {Kind::branch, false, true, false, sink}; }
  Edge jump_cond(Address t) { return {Kind::branch, true, false, false, t}; }
  Edge jump_cond_ind() { return {Kind::branch, true, true, false, sink}; }
  Edge tail(Address t) { return {Kind::branch, false, false, true, t}; }
  Edge tail_ind() { return {Kind::branch, false, true, true, sink}; }
  Edge tail_cond(Address t) { return {Kind::branch, true, false, true, t}; }
  Edge ft(Address t) { return {Kind::fallthrough, false, false, false, t}; }
  Edge ft_cond(Address t) { return {Kind::fallthrough, true, false, false, t}; }
  Edge call_ft(Address t) { return {Kind::call_ft, false, false, false, t}; }
  Edge call_ft_cond(Address t) { return {Kind::call_ft, true, false, false, t}; }
  Edge catch_(Address t) { return {Kind::catch_, false, false, false, t}; }

  std::string format(Edge const &e) {
    std::ostringstream s;
    s << name(e.kind) << (e.cond ? " cond" : "") << (e.ind ? " ind" : "") << (e.tail ? " tail" : "") << " -> ";
    if(e.trg == sink) {
      s << "sink";
    } else {
      s << "0x" << std::hex << e.trg;
    }
    return s.str();
  }

  // The out-edges of the block that ends at an instruction
  struct Case {
    Address insn;
    std::vector<Edge> edges;
  };

  bool kind_of(pa::Edge *e, Kind &k) {
    int n = 0;
    if(e->isCall()) { k = Kind::call; ++n; }
    if(e->isSyscall()) { k = Kind::syscall; ++n; }
    if(e->isReturn()) { k = Kind::ret; ++n; }
    if(e->isBranch()) { k = Kind::branch; ++n; }
    if(e->isFallthrough()) { k = Kind::fallthrough; ++n; }
    if(e->isCallFallthrough()) { k = Kind::call_ft; ++n; }
    if(e->isCatch()) { k = Kind::catch_; ++n; }
    return n == 1;
  }

  // The EdgeTypeEnum that type() returns for a kind and its flags
  pa::EdgeTypeEnum type_of(Kind k, bool cond, bool ind) {
    switch(k) {
      case Kind::call: return pa::CALL;
      case Kind::syscall: return pa::SYS_CALL;
      case Kind::ret: return cond ? pa::COND_TAKEN : pa::RET;
      case Kind::branch: return cond ? pa::COND_TAKEN : (ind ? pa::INDIRECT : pa::DIRECT);
      case Kind::fallthrough: return cond ? pa::COND_NOT_TAKEN : pa::FALLTHROUGH;
      case Kind::call_ft: return pa::CALL_FT;
      case Kind::catch_: return pa::CATCH;
    }
    return pa::NOEDGE;
  }

  // An edge's kind and flags are one of the combinations a writer can produce
  bool valid(pa::Edge *e, std::string const &where) {
    Kind k{};
    if(!kind_of(e, k)) {
      std::cerr << where << ": an edge has no kind or more than one\n";
      return false;
    }
    bool const cond = e->isConditional(), ind = e->isIndirect(), tail = e->isTailCall();
    bool ok = true;
    auto bad = [&](char const *what) {
      std::cerr << where << ": " << name(k) << " edge " << what << "\n";
      ok = false;
    };
    if(tail && k != Kind::branch) bad("is a tail call");
    switch(k) {
      case Kind::syscall:
        if(cond || ind) bad("is conditional or indirect");
        break;
      case Kind::ret:
        if(!ind) bad("is not indirect");
        break;
      case Kind::fallthrough:
      case Kind::call_ft:
        if(ind) bad("is indirect");
        break;
      case Kind::catch_:
        if(cond || ind) bad("is conditional or indirect");
        break;
      case Kind::call:
      case Kind::branch:
        break;
    }
    if(e->type() != type_of(k, cond, ind)) {
      bad("has the wrong type()");
    }
    bool const interproc = tail || k == Kind::call || k == Kind::syscall || k == Kind::ret;
    if(e->interproc() != interproc) {
      bad("has the wrong interproc()");
    }
    return ok;
  }

  pa::Block *block_ending_at(pa::CodeObject &co, pa::CodeRegion *r, Address insn) {
    std::set<pa::Block *> blocks;
    co.findBlocks(r, insn, blocks);
    for(auto *b : blocks) {
      if(b->last() == insn) {
        return b;
      }
    }
    return nullptr;
  }

  struct Arch {
    char const *name;
    Dyninst::Architecture arch;
    std::vector<mcs::Insn> insns;
    std::vector<mcs::Func> funcs;
    std::map<Address, Address> catches;
    std::vector<Case> cases;
  };

  bool check_edges(pa::CodeObject &co, mcs::Source &src, Arch const &a) {
    bool ok = true;
    for(auto const &c : a.cases) {
      std::ostringstream where;
      where << a.name << " 0x" << std::hex << c.insn;
      pa::Block *b = block_ending_at(co, src.region(), c.insn);
      if(!b) {
        std::cerr << where.str() << ": no block ends here\n";
        ok = false;
        continue;
      }
      std::vector<Edge> got;
      for(auto *e : b->targets()) {
        Kind k{};
        kind_of(e, k);
        got.push_back({k, e->isConditional(), e->isIndirect(), e->isTailCall(),
                       e->sinkEdge() ? sink : e->trg()->start()});
      }
      std::vector<Edge> want = c.edges;
      std::sort(got.begin(), got.end());
      std::sort(want.begin(), want.end());
      if(got != want) {
        std::cerr << where.str() << ": wrong out-edges\n  expected:\n";
        for(auto const &e : want) std::cerr << "    " << format(e) << "\n";
        std::cerr << "  parsed:\n";
        for(auto const &e : got) std::cerr << "    " << format(e) << "\n";
        ok = false;
      }
    }
    for(auto *f : co.funcs()) {
      for(auto *b : f->blocks()) {
        std::ostringstream where;
        where << a.name << " block 0x" << std::hex << b->start();
        for(auto *e : b->targets()) {
          ok = valid(e, where.str()) && ok;
        }
      }
    }
    return ok;
  }

  pa::Function *func_at(pa::CodeObject &co, mcs::Source &src, Address a) {
    return co.findFuncByEntry(src.region(), a);
  }

  bool has_block_ending_at(pa::Function::const_blocklist blocks, Address insn) {
    for(auto *b : blocks) {
      if(b->last() == insn) {
        return true;
      }
    }
    return false;
  }

  // clang-format off
  Arch x86_64() {
    return {"x86_64", Dyninst::Arch_x86_64,
      {
        // f_calls
        {0x000, {0xe8, 0x3b, 0x01, 0x00, 0x00}},  // call f_leaf
        {0x005, {0xff, 0xd0}},                    // call *%rax
        {0x007, {0x0f, 0x05}},                    // syscall
        {0x009, {0xe8, 0x32, 0x01, 0x00, 0x00}},  // call f_leaf    (in a try range, handler f_catch)
        {0x00e, {0xc3}},                          // ret
        // f_branches
        {0x040, {0x85, 0xff}},                    // test %edi, %edi
        {0x042, {0x74, 0x05}},                    // je 0x49
        {0x044, {0xb8, 0x01, 0x00, 0x00, 0x00}},  // mov $1, %eax    (0x49 is a branch target)
        {0x049, {0xeb, 0x01}},                    // jmp 0x4c
        {0x04b, {0x90}},                          // nop
        {0x04c, {0xc3}},                          // ret
        // f_tail
        {0x080, {0x85, 0xff}},                    // test %edi, %edi
        {0x082, {0x0f, 0x84, 0xb8, 0x00, 0x00, 0x00}},  // je f_leaf
        {0x088, {0xe9, 0xb3, 0x00, 0x00, 0x00}},  // jmp f_leaf
        // f_indtail
        {0x0c0, {0x55}},                          // push %rbp
        {0x0c1, {0x48, 0x89, 0xe5}},              // mov %rsp, %rbp
        {0x0c4, {0xc9}},                          // leave
        {0x0c5, {0xff, 0xe0}},                    // jmp *%rax
        // f_indjmp
        {0x100, {0x48, 0x89, 0xf8}},              // mov %rdi, %rax
        {0x103, {0x48, 0x83, 0xc0, 0x08}},        // add $8, %rax
        {0x107, {0xff, 0xe0}},                    // jmp *%rax
        // f_leaf
        {0x140, {0x31, 0xc0}},                    // xor %eax, %eax
        {0x142, {0xc3}},                          // ret
        // f_catch
        {0x1c0, {0x31, 0xc0}},                    // xor %eax, %eax
        {0x1c2, {0xc3}},                          // ret
      },
      {{0x000, "f_calls"}, {0x040, "f_branches"}, {0x080, "f_tail"}, {0x0c0, "f_indtail"},
       {0x100, "f_indjmp"}, {0x140, "f_leaf"}, {0x1c0, "f_catch"}},
      {{0x009, 0x1c0}},
      {
        {0x000, {call(0x140), call_ft(0x005)}},
        {0x005, {call_ind(), call_ft(0x007)}},
        {0x007, {syscall(), call_ft(0x009)}},
        {0x009, {call(0x140), call_ft(0x00e), catch_(0x1c0)}},
        {0x00e, {ret()}},
        {0x042, {jump_cond(0x049), ft_cond(0x044)}},
        {0x044, {ft(0x049)}},
        {0x049, {jump(0x04c)}},
        {0x04c, {ret()}},
        {0x082, {tail_cond(0x140), ft_cond(0x088)}},
        {0x088, {tail(0x140)}},
        {0x0c5, {tail_ind()}},
        {0x107, {jump_ind()}},
      }};
  }

  Arch aarch64() {
    return {"aarch64", Dyninst::Arch_aarch64,
      {
        // f_calls
        {0x000, {0x40, 0x00, 0x00, 0x94}},  // bl f_leaf
        {0x004, {0x20, 0x00, 0x3f, 0xd6}},  // blr x1
        {0x008, {0x01, 0x00, 0x00, 0xd4}},  // svc #0         (not control flow to the parser)
        {0x00c, {0xc0, 0x03, 0x5f, 0xd6}},  // ret
        // f_branches
        {0x040, {0x1f, 0x00, 0x00, 0xf1}},  // cmp x0, #0
        {0x044, {0x40, 0x00, 0x00, 0x54}},  // b.eq 0x4c
        {0x048, {0x21, 0x00, 0x80, 0xd2}},  // mov x1, #1      (0x4c is a branch target)
        {0x04c, {0x42, 0x00, 0x00, 0xb4}},  // cbz x2, 0x54
        {0x050, {0x01, 0x00, 0x00, 0x14}},  // b 0x54
        {0x054, {0xc0, 0x03, 0x5f, 0xd6}},  // ret
        // f_tail
        {0x080, {0x1f, 0x00, 0x00, 0xf1}},  // cmp x0, #0
        {0x084, {0xe0, 0x03, 0x00, 0x54}},  // b.eq f_leaf
        {0x088, {0x1e, 0x00, 0x00, 0x14}},  // b f_leaf
        // f_indjmp
        {0x0c0, {0x01, 0x20, 0x00, 0x91}},  // add x1, x0, #8
        {0x0c4, {0x20, 0x00, 0x1f, 0xd6}},  // br x1
        // f_leaf
        {0x100, {0x00, 0x00, 0x80, 0xd2}},  // mov x0, #0
        {0x104, {0xc0, 0x03, 0x5f, 0xd6}},  // ret
      },
      {{0x000, "f_calls"}, {0x040, "f_branches"}, {0x080, "f_tail"}, {0x0c0, "f_indjmp"},
       {0x100, "f_leaf"}},
      {},
      {
        {0x000, {call(0x100), call_ft(0x004)}},
        {0x004, {call_ind(), call_ft(0x008)}},
        {0x00c, {ret()}},
        {0x044, {jump_cond(0x04c), ft_cond(0x048)}},
        {0x048, {ft(0x04c)}},
        {0x04c, {jump_cond(0x054), ft_cond(0x050)}},
        {0x050, {jump(0x054)}},
        {0x054, {ret()}},
        {0x084, {tail_cond(0x100), ft_cond(0x088)}},
        {0x088, {tail(0x100)}},
        // The heuristics take an indirect jump after an add for a tail call.
        {0x0c4, {tail_ind()}},
      }};
  }

  Arch ppc64le() {
    return {"ppc64le", Dyninst::Arch_ppc64,
      {
        // f_calls
        {0x000, {0x01, 0x01, 0x00, 0x48}},  // bl f_leaf
        {0x004, {0x21, 0x04, 0x80, 0x4e}},  // bctrl
        {0x008, {0xf9, 0x00, 0x9e, 0x41}},  // beql cr7, f_leaf
        {0x00c, {0x21, 0x04, 0x9e, 0x4d}},  // beqctrl cr7     (decoded as unconditional, with no fall-through)
        {0x010, {0x02, 0x00, 0x00, 0x44}},  // sc              (not control flow to the parser)
        {0x014, {0x20, 0x00, 0x80, 0x4e}},  // blr
        // f_branches
        {0x040, {0x00, 0x00, 0xa3, 0x2f}},  // cmpdi cr7, r3, 0
        {0x044, {0x08, 0x00, 0x9e, 0x41}},  // beq cr7, 0x4c
        {0x048, {0x01, 0x00, 0x80, 0x38}},  // li r4, 1         (0x4c is a branch target)
        {0x04c, {0x08, 0x00, 0x00, 0x48}},  // b 0x54
        {0x050, {0x00, 0x00, 0x00, 0x60}},  // nop
        {0x054, {0x20, 0x04, 0x9e, 0x4c}},  // bnectr cr7
        {0x058, {0x20, 0x00, 0x9e, 0x4d}},  // beqlr cr7
        {0x05c, {0x20, 0x00, 0x00, 0x4e}},  // bdnzlr
        {0x060, {0x20, 0x00, 0x80, 0x4e}},  // blr
        // f_tail
        {0x080, {0x00, 0x00, 0xa3, 0x2f}},  // cmpdi cr7, r3, 0
        {0x084, {0x7c, 0x00, 0x9e, 0x41}},  // beq cr7, f_leaf
        {0x088, {0x78, 0x00, 0x00, 0x48}},  // b f_leaf
        // f_indjmp
        {0x0c0, {0x78, 0x1b, 0x6c, 0x7c}},  // mr r12, r3
        {0x0c4, {0x08, 0x00, 0x8c, 0x39}},  // addi r12, r12, 8
        {0x0c8, {0xa6, 0x03, 0x89, 0x7d}},  // mtctr r12
        {0x0cc, {0x20, 0x04, 0x80, 0x4e}},  // bctr
        // f_leaf
        {0x100, {0x00, 0x00, 0x60, 0x38}},  // li r3, 0
        {0x104, {0x20, 0x00, 0x80, 0x4e}},  // blr
        // f_condret_only: its only return is conditional. The parser takes a
        // function whose second instruction is an unresolved branch for one that
        // returns anyway, so the beqlr is the third.
        {0x140, {0x01, 0x00, 0x80, 0x38}},  // li r4, 1
        {0x144, {0x00, 0x00, 0xa3, 0x2f}},  // cmpdi cr7, r3, 0
        {0x148, {0x20, 0x00, 0x9e, 0x4d}},  // beqlr cr7
        {0x14c, {0x00, 0x00, 0x00, 0x48}},  // b 0x14c
      },
      {{0x000, "f_calls"}, {0x040, "f_branches"}, {0x080, "f_tail"}, {0x0c0, "f_indjmp"},
       {0x100, "f_leaf"}, {0x140, "f_condret_only"}},
      {},
      {
        {0x000, {call(0x100), call_ft(0x004)}},
        {0x004, {call_ind(), call_ft(0x008)}},
        {0x008, {call_cond(0x100), call_ft_cond(0x00c)}},
        {0x00c, {call_ind(), call_ft(0x010)}},
        {0x014, {ret()}},
        {0x044, {jump_cond(0x04c), ft_cond(0x048)}},
        {0x048, {ft(0x04c)}},
        {0x04c, {jump(0x054)}},
        {0x054, {jump_cond_ind(), ft_cond(0x058)}},
        {0x058, {ret_cond(), ft_cond(0x05c)}},
        {0x05c, {ret_cond(), ft_cond(0x060)}},
        {0x060, {ret()}},
        {0x084, {tail_cond(0x100), ft_cond(0x088)}},
        {0x088, {tail(0x100)}},
        {0x0cc, {jump_ind()}},
        {0x148, {ret_cond(), ft_cond(0x14c)}},
        {0x14c, {jump(0x14c)}},
      }};
  }
  // clang-format on

  // A block that ends in a ppc conditional return is a return and exit block, and
  // a function whose only return is conditional returns.
  bool check_ppc_conditional_returns(pa::CodeObject &co, mcs::Source &src) {
    bool ok = true;
    pa::Function *f = func_at(co, src, 0x040);
    if(!f) {
      std::cerr << "ppc64le: no f_branches\n";
      return false;
    }
    for(Address insn : {0x058, 0x05c, 0x060}) {
      if(!has_block_ending_at(f->returnBlocks(), insn) || !has_block_ending_at(f->exitBlocks(), insn)) {
        std::cerr << "ppc64le: the block ending at 0x" << std::hex << insn << std::dec
                  << " is not a return and exit block of f_branches\n";
        ok = false;
      }
    }
    pa::Function *g = func_at(co, src, 0x140);
    if(!g) {
      std::cerr << "ppc64le: no f_condret_only\n";
      return false;
    }
    if(g->retstatus() != pa::RETURN) {
      std::cerr << "ppc64le: f_condret_only has return status " << g->retstatus() << ", not RETURN\n";
      ok = false;
    }
    if(!has_block_ending_at(g->returnBlocks(), 0x148)) {
      std::cerr << "ppc64le: beqlr's block is not a return block of f_condret_only\n";
      ok = false;
    }
    return ok;
  }

  bool run(Arch const &a) {
    auto *src = new mcs::Source(a.arch, a.insns, a.funcs, a.catches);
    auto *co = new pa::CodeObject(src);
    co->parse();
    bool ok = check_edges(*co, *src, a);
    if(a.arch == Dyninst::Arch_ppc64) {
      ok = check_ppc_conditional_returns(*co, *src) && ok;
    }
    delete co;
    delete src;
    std::cerr << a.name << ": " << (ok ? "ok" : "FAILED") << "\n";
    return ok;
  }
}

int main() {
  bool ok = true;
  ok = run(x86_64()) && ok;
  ok = run(aarch64()) && ok;
  ok = run(ppc64le()) && ok;
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
