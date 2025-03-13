#include "Absloc.h"
#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "debug.h"
#include "DynAST.h"
#include "Function.h"
#include "InstructionAST.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/ppc64_regs.h"
#include "Result.h"
#include "slicing.h"
#include "SymEval.h"
#include "Symtab.h"
#include "unaligned_memory_access.h"

#include <set>
#include <vector>

namespace {

  namespace df = Dyninst::DataflowAPI;
  namespace st = Dyninst::SymtabAPI;
  namespace ia = Dyninst::InstructionAPI;
  namespace pa = Dyninst::ParseAPI;

  class Default_Predicates : public Dyninst::Slicer::Predicates {};

  /* This visitor is capable of simplifying constant value computations
     that involve additions and concatenations (lis instruction). This
     is sufficient to handle the startup struct address calculation in
     GLIBC that we have seen; if additional variants are introduced
     (refer to start.S in glibc or equivalently to the compiled library)
     this visitor should be expanded to handle any new operations */

  class SimpleArithVisitor : public Dyninst::ASTVisitor {

    using ASTVisitor::visit;

    ASTPtr visit(Dyninst::AST* a) override {
      return a->ptr();
    }

    ASTPtr visit(df::BottomAST* a) override {
      return a->ptr();
    }

    ASTPtr visit(df::ConstantAST* c) override {
      return c->ptr();
    }

    ASTPtr visit(df::VariableAST* v) override {
      return v->ptr();
    }

    ASTPtr visit(df::RoseAST* r) override {

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

  void* get_raw_symtab_ptr(st::Symtab* linkedFile, Dyninst::Address addr) {
    st::Region* reg = linkedFile->findEnclosingRegion(addr);
    if(!reg) {
      return nullptr;
    }
    char* data = static_cast<char*>(reg->getPtrToRawData());
    data += addr - reg->getMemOffset();
    return data;
  }

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
  Dyninst::Address evaluate_main_address(st::Symtab* linkedFile, pa::Function* f, pa::Block* b) {
    ia::InstructionDecoder dec(b->region()->getPtrToInstruction(b->start()), b->end() - b->start(),
                               b->region()->getArch());

    // Register operands produced by the decoder are normalized to the
    // decoding architecture (ppc64) but do not necessarily carry the
    // same bit range as a RegisterAST built directly from a ppc64
    // register. RegisterAST equality -- used by Instruction::isWritten/
    // isRead and Expression::bind -- compares the register id AND the
    // bit range, so those queries silently never match here. Match and
    // bind registers by id instead.
    auto usesRegID = [](const std::set<ia::RegisterAST::Ptr>& regs, Dyninst::MachRegister reg) {
      for(const auto& i : regs) {
        if(i->getID() == reg) {
          return true;
        }
      }
      return false;
    };

    // looking for the *last* instruction in the block that defines GR8
    ia::Instruction r8_def;
    Dyninst::Address r8_def_addr;
    bool find = false;

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
      return Dyninst::ADDR_NULL;
    }

    Dyninst::Address ss_addr = Dyninst::ADDR_NULL;

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
        const uint32_t* entry_code = (const uint32_t*)b->region()->getPtrToInstruction(f->addr());
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
      if(TOC != Dyninst::ADDR_NULL && memReads.size() == 1) {
        ia::Expression::Ptr expr = *memReads.begin();
        // Bind the r2 instance used by the expression itself so the
        // bind's equality test matches it.
        for(const ia::RegisterAST::Ptr& ru : ia::getUsedRegisters(expr)) {
          if(ru->getID() == Dyninst::ppc64::r2) {
            expr->bind(ru.get(), ia::Result(ia::u64, TOC));
          }
        }
        const ia::Result& res = expr->eval();
        if(res.defined) {
          void* res_addr = get_raw_symtab_ptr(linkedFile, res.convert<Dyninst::Address>());
          if(res_addr) {
            ss_addr = Dyninst::read_memory_as<Dyninst::Address>(res_addr);
          }
        }
      }
    }

    if(ss_addr == Dyninst::ADDR_NULL) {
      // Get all of the assignments that happen in this instruction
      Dyninst::AssignmentConverter conv(true, true);
      std::vector<Dyninst::Assignment::Ptr> assigns;
      conv.convert(r8_def, r8_def_addr, f, b, assigns);

      // find the one we care about (r8)
      std::vector<Dyninst::Assignment::Ptr>::iterator ait = assigns.begin();
      for(; ait != assigns.end(); ++ait) {
        Dyninst::AbsRegion& outReg = (*ait)->out();
        const Dyninst::Absloc& loc = outReg.absloc();
        if(loc.reg() == Dyninst::ppc64::r8) {
          break;
        }
      }

      if(ait == assigns.end()) {
        return Dyninst::ADDR_NULL;
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

      if(simplified->getID() == Dyninst::AST::V_ConstantAST) {
        df::ConstantAST::Ptr cp = df::ConstantAST::convert(simplified);
        ss_addr = cp->val().val;
      }
    }

    struct libc_startup_info {
      void* sda;
      void* main_addr;
      void* init_addr;
      void* fini_addr;
    };

    // need a pointer to the image data
    auto ptr = get_raw_symtab_ptr(linkedFile, ss_addr);
    if(ptr) {
      auto sup = Dyninst::read_memory_as<libc_startup_info>(ptr);
      return reinterpret_cast<Dyninst::Address>(sup.main_addr);
    }

    return Dyninst::ADDR_NULL;
  }

}

namespace Dyninst { namespace DyninstAPI { namespace ppc {

  Dyninst::Address find_main(st::Symtab* linkedFile, pa::Function* f) {
    // Candidate blocks for the __libc_start_main call setup:
    // glibc's dynamic _start makes the call from its entry block,
    // but a tail-branching _start (static link) parses into one
    // function with several call edges further in.
    // evaluate_main_address() is fail-to-zero per block, so rather
    // than guessing the one right block, try the entry block and
    // then each call-edge source until one yields a valid address.
    std::vector<pa::Block*> candidates;
    pa::Block* entry_block = f->entry();
    if(entry_block) {
      candidates.push_back(entry_block);
    }
    for(const auto* e : f->callEdges()) {
      if(e->src() && e->src() != entry_block) {
        candidates.push_back(e->src());
      }
    }

    for(auto* b : candidates) {
      auto main_addr = evaluate_main_address(linkedFile, f, b);
      if(main_addr != Dyninst::ADDR_NULL && f->obj()->cs()->isValidAddress(main_addr)) {
        return main_addr;
      }
    }

    return Dyninst::ADDR_NULL;
  }

}}}
