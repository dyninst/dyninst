#include "Absloc.h"
#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "debug.h"
#include "DynAST.h"
#include "find_main.h"
#include "Function.h"
#include "InstructionAST.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"
#include "Result.h"
#include "slicing.h"
#include "SymEval.h"
#include "Symtab.h"
#include "unaligned_memory_access.h"

#include <set>
#include <vector>

namespace df = Dyninst::DataflowAPI;
namespace st = Dyninst::SymtabAPI;
namespace ia = Dyninst::InstructionAPI;
namespace pa = Dyninst::ParseAPI;

namespace {

  class Default_Predicates : public Dyninst::Slicer::Predicates {};

  /* This visitor is capable of simplifying constant value computations
     that involve additions and concatenations (lis instruction). This
     is sufficient to handle the startup struct address calculation in
     GLIBC that we have seen; if additional variants are introduced
     (refer to start.S in glibc or equivalently to the compiled library)
     this visitor should be expanded to handle any new operations */

  class SimpleArithVisitor : public Dyninst::ASTVisitor {

    using ASTVisitor::visit;

    ASTPtr visit(Dyninst::AST *a) override {
      return a->ptr();
    }

    ASTPtr visit(df::BottomAST *a) override {
      return a->ptr();
    }

    ASTPtr visit(df::ConstantAST *c) override {
      return c->ptr();
    }

    ASTPtr visit(df::VariableAST *v) override {
      return v->ptr();
    }

    ASTPtr visit(df::RoseAST *r) override {

      Dyninst::AST::Children newKids;
      for(unsigned i = 0; i < r->numChildren(); ++i) {
        newKids.push_back(r->child(i)->accept(this));
      }

      switch(r->val().op) {
        case df::ROSEOperation::addOp: {
          assert(newKids.size() == 2);
          const auto constAST = Dyninst::AST::V_ConstantAST;
          if(newKids[0]->getID() == constAST && newKids[1]->getID() == constAST) {
            df::ConstantAST::Ptr c1 = df::ConstantAST::convert(newKids[0]);
            df::ConstantAST::Ptr c2 = df::ConstantAST::convert(newKids[1]);
            return df::ConstantAST::create(df::Constant(c1->val().val + c2->val().val));
          }
        } break;
        case df::ROSEOperation::concatOp: {
          assert(newKids.size() == 2);
          const auto constAST = Dyninst::AST::V_ConstantAST;
          if(newKids[0]->getID() == constAST && newKids[1]->getID() == constAST) {
            df::ConstantAST::Ptr c1 = df::ConstantAST::convert(newKids[0]);
            df::ConstantAST::Ptr c2 = df::ConstantAST::convert(newKids[1]);
            unsigned long result = c1->val().val;
            result |= (c2->val().val << c2->val().size);
            return df::ConstantAST::create(df::Constant(result));
          }
        } break;
        default:
          startup_printf("%s[%d] unhandled operation in simplification\n", FILE__, __LINE__);
      }

      return df::RoseAST::create(r->val(), newKids);
    }
  };

  // Pointer to reg's file contents of [addr, addr + size), or nullptr when
  // reg does not hold those bytes on disk (no region, or NOBITS such as .bss).
  void *get_raw_symtab_ptr(st::Region *reg, Dyninst::Address addr, Dyninst::Address size) {
    if(reg == nullptr || reg->getPtrToRawData() == nullptr) {
      return nullptr;
    }
    Dyninst::Address off = addr - reg->getMemOffset();
    if(off + size > reg->getDiskSize()) {
      return nullptr;
    }
    return reinterpret_cast<char *>(reg->getPtrToRawData()) + off;
  }

  // Read one word of the binary's own width (not the host's) at addr,
  // which lies in reg. Returns ADDRESS_INVALID if it cannot be read.
  Dyninst::Address read_target_word(st::Symtab *linkedFile, st::Region *reg,
                                    Dyninst::Address addr) {
    unsigned width = linkedFile->getAddressWidth();
    void *data = get_raw_symtab_ptr(reg, addr, width);
    if(data == nullptr) {
      return Dyninst::ADDRESS_INVALID;
    }
    if(width == 8) {
      return static_cast<Dyninst::Address>(Dyninst::read_memory_as<uint64_t>(data));
    }
    if(width == 4) {
      return static_cast<Dyninst::Address>(Dyninst::read_memory_as<uint32_t>(data));
    }
    return Dyninst::ADDRESS_INVALID;
  }

  Dyninst::Address read_target_word(st::Symtab *linkedFile, Dyninst::Address addr) {
    return read_target_word(linkedFile, linkedFile->findEnclosingRegion(addr), addr);
  }

  Dyninst::Address deref_opd(st::Symtab *linkedFile, Dyninst::Address addr) {
    st::Region *reg = linkedFile->findEnclosingRegion(addr);
    if(reg && reg->getRegionName() == ".opd") {
      // opd symbol needing dereference
      Dyninst::Address target = read_target_word(linkedFile, reg, addr);
      if(target != Dyninst::ADDRESS_INVALID) {
        return target;
      }
    }
    return addr;
  }

  /*
   * b ends with a call to libc_start_main. We are looking for the
   * value in GR8, which is the address of a structure that contains
   * the address to main
   */
  Dyninst::Address evaluate_main_address(st::Symtab *linkedFile, pa::Function *f, pa::Block *b) {
    // looking for the *last* instruction in the block
    // that defines GR8

    ia::Instruction r8_def;
    Dyninst::Address r8_def_addr;
    bool find = false;

    ia::InstructionDecoder dec(b->region()->getPtrToInstruction(b->start()), b->end() - b->start(),
                               b->region()->getArch());

    // Register operands produced by the decoder are normalized to the
    // decoding architecture (ppc64) but do not necessarily carry the
    // same bit range as a RegisterAST built directly from a ppc64
    // register. RegisterAST equality -- used by Instruction::isWritten/
    // isRead and Expression::bind -- compares the register id AND the
    // bit range, so those queries silently never match here. Match and
    // bind registers by id instead.
    auto usesRegID = [](std::set<ia::RegisterAST::Ptr> const &regs, Dyninst::MachRegister reg) {
      for(auto const &i : regs) {
        if(i->getID() == reg) {
          return true;
        }
      }
      return false;
    };

    Dyninst::Address cur_addr = b->start();
    while(cur_addr < b->end()) {
      ia::Instruction cur = dec.decode();
      std::set<ia::RegisterAST::Ptr> written;
      cur.getWriteSet(written);
      if(usesRegID(written, Dyninst::ppc64::r8)) {
        find = true;
        r8_def = cur;
        r8_def_addr = cur_addr;
      }
      cur_addr += cur.size();
    }
    if(!find) {
      return 0;
    }

    Dyninst::Address ss_addr = 0;

    // Try a TOC-based lookup first
    std::set<ia::RegisterAST::Ptr> readRegs;
    r8_def.getReadSet(readRegs);
    if(usesRegID(readRegs, Dyninst::ppc64::r2)) {
      std::set<ia::Expression::Ptr> memReads;
      r8_def.getMemoryReadOperands(memReads);
      Dyninst::Address TOC = f->obj()->cs()->getTOC(r8_def_addr);
      // ELFv2 (ppc64le) has no .opd section, so the code source's TOC
      // table is empty and getTOC() returns 0 for every address.
      // Derive the TOC from the function's global entry point instead.
      // The ABI-prescribed entry sequence
      //     addis r2,r12,H ; addi r2,r2,L
      // with r12 holding the entry address gives
      //     TOC = entry + (H << 16) + L,
      // and the linker may relax it (static links below 2 GB) to the
      // absolute form
      //     lis r2,H ; addi r2,r2,L    =>    TOC = (H << 16) + L.
      // (POWER10 pc-relative code sets up no TOC at all; its r8 load
      // does not read r2, so this branch is never reached for it.)
      if(TOC == 0) {
        const uint32_t *entry_code = (const uint32_t *)b->region()->getPtrToInstruction(f->addr());
        if(entry_code && f->addr() + 8 <= b->region()->high() &&
           (entry_code[1] & 0xffff0000) == 0x38420000) // addi r2,r2,L
        {
          Dyninst::Address hi = (Dyninst::Address)(int16_t)(entry_code[0] & 0xffff) << 16;
          Dyninst::Address lo = (Dyninst::Address)(int16_t)(entry_code[1] & 0xffff);
          if((entry_code[0] & 0xffff0000) == 0x3c4c0000) { // addis r2,r12,H
            TOC = f->addr() + hi + lo;
          } else if((entry_code[0] & 0xffff0000) == 0x3c400000) { // lis r2,H
            TOC = hi + lo;
          }
        }
      }
      if(TOC != 0 && memReads.size() == 1) {
        ia::Expression::Ptr expr = *memReads.begin();
        // Bind the r2 instance used by the expression itself so the
        // bind's equality test matches it.
        for(ia::RegisterAST::Ptr const &ru : getUsedRegisters(expr)) {
          if(ru->getID() == Dyninst::ppc64::r2) {
            expr->bind(ru.get(), ia::Result(ia::u64, TOC));
          }
        }
        const ia::Result &res = expr->eval();
        if(res.defined) {
          Dyninst::Address word = read_target_word(linkedFile, res.convert<Dyninst::Address>());
          if(word != Dyninst::ADDRESS_INVALID) {
            ss_addr = word;
          }
        }
      }
    }

    if(ss_addr == 0) {
      // Get all of the assignments that happen in this instruction
      Dyninst::AssignmentConverter conv(true, true);
      std::vector<Dyninst::Assignment::Ptr> assigns;
      conv.convert(r8_def, r8_def_addr, f, b, assigns);

      // find the one we care about (r8)
      std::vector<Dyninst::Assignment::Ptr>::iterator ait = assigns.begin();
      for(; ait != assigns.end(); ++ait) {
        Dyninst::AbsRegion &outReg = (*ait)->out();
        Dyninst::Absloc const &loc = outReg.absloc();
        if(loc.reg() == Dyninst::ppc64::r8) {
          break;
        }
      }
      if(ait == assigns.end()) {
        return 0;
      }

      // Slice back to the definition of R8, and, if possible, simplify
      // to a constant
      Dyninst::Slicer slc(*ait, b, f);
      Default_Predicates preds;
      Dyninst::Graph::Ptr slg = slc.backwardSlice(preds);
      df::Result_t sl_res;
      df::SymEval::expand(slg, sl_res);
      Dyninst::AST::Ptr calculation = sl_res[*ait];
      SimpleArithVisitor visit;
      Dyninst::AST::Ptr simplified = calculation->accept(&visit);
      // printf("after simplification:\n%s\n",simplified->format().c_str());
      if(simplified->getID() == Dyninst::AST::V_ConstantAST) {
        df::ConstantAST::Ptr cp = df::ConstantAST::convert(simplified);
        ss_addr = cp->val().val;
      }
    }

    // ss_addr points at glibc's struct startup_info, four words of the
    // binary's width: sda_base, main, init, fini. Read main.
    if(ss_addr != 0) {
      Dyninst::Address main_addr =
          read_target_word(linkedFile, ss_addr + linkedFile->getAddressWidth());
      if(main_addr != Dyninst::ADDRESS_INVALID) {
        return main_addr;
      }
    }

    return 0;
  }

}

namespace Dyninst { namespace DyninstAPI { namespace ppc {

  /*
   *  On PPC GLIBC (32 & 64 bit) the address of main is in a structure
   *  located in either .data or .rodata, depending on whether the
   *  binary is PIC. The structure has the following format:
   *
   *  struct
   *  {
   *    void * // "small data area base"
   *    main   // pointer to main
   *    init   // pointer to init
   *    fini   // pointer to fini
   *  }
   *
   *  This structure is passed in GR8 as an argument to libc_start_main.
   *  Annoyingly, the value in GR8 is computed in several different ways,
   *  depending on how GLIBC was compiled.
   *
   *  This code follows the i386 linux version closely otherwise.
   *
   *  `b` ends with a call to libc_start_main
   */
  Dyninst::Address find_main(st::Symtab *linkedFile, pa::Block *entryBlock,
                             pa::SymtabCodeSource const &scs, pa::Function *f) {
    // Candidate blocks for the __libc_start_main call setup:
    // glibc's dynamic _start makes the call from its entry block,
    // but a tail-branching _start (static link) parses into one
    // function with several call edges further in.
    // evaluate_main_address() is fail-to-zero per block, so rather
    // than guessing the one right block, try the entry block and
    // then each call-edge source until one yields a valid address.
    std::vector<pa::Block *> candidates;
    if(entryBlock) {
      candidates.push_back(entryBlock);
    }

    for(pa::Edge *e : f->callEdges()) {
      if(e->src() && e->src() != entryBlock) {
        candidates.push_back(e->src());
      }
    }

    Dyninst::Address mainAddress = Dyninst::ADDRESS_INVALID;

    for(pa::Block *b : candidates) {
      Address cand = evaluate_main_address(linkedFile, f, b);
      cand = deref_opd(linkedFile, cand);
      if(cand != 0 && scs.isValidAddress(cand)) {
        mainAddress = cand;
        break;
      }
    }

    return mainAddress;
  }

}}}
