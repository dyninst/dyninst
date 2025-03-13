#include "Absloc.h"
#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "debug.h"
#include "DynAST.h"
#include "Function.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/ppc32_regs.h"
#include "Result.h"
#include "slicing.h"
#include "SymEval.h"
#include "Symtab.h"
#include "unaligned_memory_access.h"

#include <set>
#include <vector>
#include "find_main.h"

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
            return df::ConstantAST::create(result);
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
      Dyninst::Address find_main_by_toc(st::Symtab* linkedFile, pa::Function* f, pa::Block* b) {
        // looking for the *last* instruction in the block that defines GR8
        ia::Instruction r8_def;
        Dyninst::Address r8_def_addr;
        bool find = false;

        ia::InstructionDecoder dec(b->region()->getPtrToInstruction(b->start()), b->end() - b->start(),
                                   b->region()->getArch());

        ia::RegisterAST::Ptr r2(new ia::RegisterAST(Dyninst::ppc32::r2));
        ia::RegisterAST::Ptr r8(new ia::RegisterAST(Dyninst::ppc32::r8));

        Dyninst::Address cur_addr = b->start();
        while(cur_addr < b->end()) {
          ia::Instruction cur = dec.decode();
          if(cur.isWritten(r8)) {
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
        if(r8_def.isRead(r2)) {
          std::set<ia::Expression::Ptr> memReads;
          r8_def.getMemoryReadOperands(memReads);
          Dyninst::Address TOC = f->obj()->cs()->getTOC(r8_def_addr);
          if(TOC != Dyninst::ADDR_NULL && memReads.size() == 1) {
            ia::Expression::Ptr expr = *memReads.begin();
            expr->bind(r2.get(), ia::Result(ia::u64, TOC));
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
            if(loc.reg() == r8->getID()) {
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

}}}
