#include "SymbolicExpression.h"
#include "SymEval.h"
#include "Absloc.h"
#include "debug_parse.h"
#include "IndirectASTVisitor.h"
#include "CFG.h"
#include "CodeSource.h"
#include "CodeObject.h"
using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::DataflowAPI;

namespace {
    // A constant's value is its low 'size' bits; size 0 means 64.
    size_t bitsOf(const Constant &c) {
        return (c.size == 0 || c.size > 64) ? 64 : c.size;
    }

    uint64_t lowBits(uint64_t v, size_t size) {
        return size >= 64 ? v : v & ((1ULL << size) - 1);
    }

    uint64_t signExtendBits(uint64_t v, size_t from) {
        if (from >= 64) return v;
        v = lowBits(v, from);
        return ((v >> (from - 1)) & 1) ? v | ~((1ULL << from) - 1) : v;
    }

    // The 128-bit product of two 64-bit values, as hi:lo.
    void multiply128(uint64_t a, uint64_t b, bool isSigned, uint64_t &hi, uint64_t &lo) {
        uint64_t const a0 = a & 0xffffffff, a1 = a >> 32, b0 = b & 0xffffffff, b1 = b >> 32;
        uint64_t const p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
        uint64_t const mid = (p00 >> 32) + (p01 & 0xffffffff) + (p10 & 0xffffffff);
        lo = (p00 & 0xffffffff) | (mid << 32);
        hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
        if (isSigned) {
            if (static_cast<int64_t>(a) < 0) hi -= b;
            if (static_cast<int64_t>(b) < 0) hi -= a;
        }
    }

    // The product of a multiply of two constants, as hi:lo with its width; false if ast is not one.
    bool constantProduct(const AST::Ptr &ast, uint64_t &hi, uint64_t &lo, size_t &width) {
        if (ast->getID() != AST::V_RoseAST) return false;
        RoseAST::Ptr r = boost::static_pointer_cast<RoseAST>(ast);
        bool const isSigned = r->val().op == ROSEOperation::sMultOp;
        if (!isSigned && r->val().op != ROSEOperation::uMultOp) return false;
        if (r->child(0)->getID() != AST::V_ConstantAST || r->child(1)->getID() != AST::V_ConstantAST) return false;
        Constant const c0 = boost::static_pointer_cast<ConstantAST>(r->child(0))->val();
        Constant const c1 = boost::static_pointer_cast<ConstantAST>(r->child(1))->val();
        uint64_t const v0 = isSigned ? signExtendBits(c0.val, bitsOf(c0)) : lowBits(c0.val, bitsOf(c0));
        uint64_t const v1 = isSigned ? signExtendBits(c1.val, bitsOf(c1)) : lowBits(c1.val, bitsOf(c1));
        multiply128(v0, v1, isSigned, hi, lo);
        width = bitsOf(c0) + bitsOf(c1);
        return true;
    }
}

bool SymbolicExpression::ReadMemory(Address addr, uint64_t &v, int ) {
    int addressWidth = cs->getAddressWidth();
    if (addressWidth == 4) {
        addr &= 0xffffffff;
    }

#if defined(os_windows)
    addr -= cs->loadAddress();
#endif
    if (!cs->isReadOnly(addr)) return false;
    v = *(const uint64_t *) cs->getPtrToInstruction(addr);
    /*
       switch (memoryReadSize) {
       case 0:
       case 8:
       v = *(const uint64_t *) cs->getPtrToInstruction(addr);
       break;
       case 4:
       v = *(const uint32_t *) cs->getPtrToInstruction(addr);
       break;
       case 2:
       v = *(const uint16_t *) cs->getPtrToInstruction(addr);
       break;
       case 1:
       v = *(const uint8_t *) cs->getPtrToInstruction(addr);
       break;	    
       default:
       parsing_printf("Invalid memory read size %d\n", memoryReadSize);
       return false;
       }
       */
    return true;
}

/*AST::Ptr SymbolicExpression::SimplifyRecursive(AST::Ptr ast, Address addr, bool keepMultiOne) {

  }*/
AST::Ptr SymbolicExpression::SimplifyRoot(AST::Ptr ast, Address addr, bool keepMultiOne) {
    if (ast->getID() == AST::V_RoseAST) {
        RoseAST::Ptr roseAST = boost::static_pointer_cast<RoseAST>(ast); 

        switch (roseAST->val().op) {
            case ROSEOperation::invertOp:
                if (roseAST->child(0)->getID() == AST::V_RoseAST) {
                    RoseAST::Ptr child = boost::static_pointer_cast<RoseAST>(roseAST->child(0));
                    if (child->val().op == ROSEOperation::invertOp) return child->child(0);
                } else if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                    ConstantAST::Ptr child = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                    size_t size = child->val().size;
                    return ConstantAST::create(Constant(lowBits(~child->val().val, bitsOf(child->val())), size));
                }
                break;
            case ROSEOperation::extendMSBOp: {
                                                 return roseAST->child(0);
                                             }
            case ROSEOperation::extendOp: {
                                              if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                                  ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                  ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                  return ConstantAST::create(Constant(lowBits(child0->val().val, bitsOf(child0->val())), child1->val().val));
                                              }
                                              return roseAST->child(0);
                                          }
            case ROSEOperation::extractOp: {
                                               uint64_t hi, lo;
                                               size_t width;
                                               if (constantProduct(roseAST->child(0), hi, lo, width) &&
                                                   roseAST->child(1)->getID() == AST::V_ConstantAST &&
                                                   roseAST->child(2)->getID() == AST::V_ConstantAST) {
                                                   // A product wider than 64 bits is folded where its bits are taken.
                                                   uint64_t const from = ConstantAST::convert(roseAST->child(1))->val().val;
                                                   uint64_t const to = ConstantAST::convert(roseAST->child(2))->val().val;
                                                   // Bits that do not fit a constant stay a multiply, for an
                                                   // enclosing extract to take.
                                                   if (to - from > 64) return roseAST->child(0);
                                                   uint64_t v = 0;
                                                   if (from < 64)
                                                       v = (lo >> from) | (from ? hi << (64 - from) : 0);
                                                   else if (from < 128)
                                                       v = hi >> (from - 64);
                                                   return ConstantAST::create(Constant(lowBits(v, to - from), to - from));
                                               }
                                               if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                                    assert(roseAST->child(1)->getID() == AST::V_ConstantAST);
                                                    assert(roseAST->child(2)->getID() == AST::V_ConstantAST);


                                                    ConstantAST::Ptr from = ConstantAST::convert(roseAST->child(1));
                                                    ConstantAST::Ptr to = ConstantAST::convert(roseAST->child(2));
                                                    ConstantAST::Ptr val = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));

                                                    auto lowBitPos{from->val().val};
                                                    auto highBitPos{to->val().val};
                                                    uint64_t newValue{val->val().val};
                                                    if(highBitPos < 64)
                                                        newValue &= ((1ULL << highBitPos) - 1);  // zero highBitPos and higher
                                                    // A constant holds 64 bits, so bits from 64 up are zero (and a shift by 64 is undefined).
                                                    newValue = lowBitPos < 64 ? newValue >> lowBitPos : 0;

                                                    return ConstantAST::create(Constant(newValue, highBitPos - lowBitPos));
                                               }

                                               return roseAST->child(0);
                                           }
            case ROSEOperation::signExtendOp: {
                                                  if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                                      ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                      ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                      // The sign is extended through all 64 bits, whatever the new size: a
                                                      // 32-bit value added to a 64-bit address (Power reads its registers as
                                                      // 32 bits, and extracts of variables are dropped) must keep its sign.
                                                      size_t size = child1->val().val;
                                                      uint64_t val = signExtendBits(child0->val().val, bitsOf(child0->val()));
                                                      return ConstantAST::create(Constant(val, size));
                                                  }		    
                                                  return roseAST->child(0);
                                              }
            case ROSEOperation::concatOp: {	    
                                              if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                                  ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                  ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                  size_t const low = bitsOf(child0->val());
                                                  uint64_t val = (low < 64 ? child1->val().val << low : 0) + lowBits(child0->val().val, low);
                                                  size_t size = child1->val().size + child0->val().size;
                                                  return ConstantAST::create(Constant(val,size));
                                              }		    
                                              if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                                  ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                  if (child1->val().val == 0) {
                                                      return roseAST->child(0);
                                                  }
                                              }
                                              if (roseAST->child(1)->getID() == AST::V_RoseAST) {
                                                  RoseAST::Ptr child1 = boost::static_pointer_cast<RoseAST>(roseAST->child(1));
                                                  if (child1->val().op == ROSEOperation::ifOp) break;
                                              }
                                              return roseAST->child(0);
                                          }
            case ROSEOperation::addOp:
                                          // We simplify the addition as much as we can
                                          // Case 1: two constants
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              uint64_t val = child0->val().val + child1->val().val;
                                              size_t size;
                                              if (child0->val().size > child1->val().size)
                                                  size = child0->val().size;
                                              else
                                                  size = child1->val().size;
                                              return ConstantAST::create(Constant(size ? lowBits(val, size) : val, size));
                                          }
                                          // Case 2: anything adding zero stays the same
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              if (child->val().val == 0) return roseAST->child(1);
                                          }
                                          if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              if (child->val().val == 0) return roseAST->child(0);
                                          }
                                          // Case 3: if v + v * c = v * (c+1), where v is a variable and c is a constant
                                          if (roseAST->child(0)->getID() == AST::V_VariableAST && roseAST->child(1)->getID() == AST::V_RoseAST) {
                                              RoseAST::Ptr rOp = boost::static_pointer_cast<RoseAST>(roseAST->child(1));
                                              if (rOp->val().op == ROSEOperation::uMultOp || rOp->val().op == ROSEOperation::sMultOp) {
                                                  if (rOp->child(0)->getID() == AST::V_VariableAST && rOp->child(1)->getID() == AST::V_ConstantAST) {
                                                      VariableAST::Ptr varAST1 = boost::static_pointer_cast<VariableAST>(roseAST->child(0));
                                                      VariableAST::Ptr varAST2 = boost::static_pointer_cast<VariableAST>(rOp->child(0));
                                                      if (varAST1->val().reg == varAST2->val().reg) {
                                                          ConstantAST::Ptr oldC = boost::static_pointer_cast<ConstantAST>(rOp->child(1));
                                                          ConstantAST::Ptr newC = ConstantAST::create(Constant(oldC->val().val + 1, oldC->val().size));
                                                          RoseAST::Ptr newRoot = RoseAST::create(ROSEOperation(rOp->val()), varAST1, newC);
                                                          return newRoot;
                                                      }
                                                  }
                                              }
                                          }
                                          // Case 4a: if (c1 + x) + c2 = x + (c1 + c2), where x is anything, c1 and c2 are constants
                                          // Case 4b: if (x + c1) + c2 = x + (c1 + c2), where x is anything, c1 and c2 are constants
                                          // Case 4c: if c1 + (c2 + x) = x + (c1 + c2), where x is anything, c1 and c2 are constants
                                          // Case 4d: if c1 + (x + c2) = x + (c1 + c2), where x is anything, c1 and c2 are constants
                                          if ((roseAST->child(0)->getID() == AST::V_RoseAST && roseAST->child(1)->getID() == AST::V_ConstantAST) ||
                                              (roseAST->child(1)->getID() == AST::V_RoseAST && roseAST->child(0)->getID() == AST::V_ConstantAST)) {
                                              RoseAST::Ptr rOp;
                                              RoseAST::Ptr oldX = nullptr;
                                              ConstantAST::Ptr oldC1 = nullptr, oldC2 = nullptr;
                                              if (roseAST->child(0)->getID() == AST::V_RoseAST) {
                                                  rOp = boost::static_pointer_cast<RoseAST>(roseAST->child(0));
                                                  if (rOp->val().op == ROSEOperation::addOp) {
                                                      if (rOp->child(0)->getID() == AST::V_ConstantAST) {
                                                          // Case 4a
                                                          oldX = boost::static_pointer_cast<RoseAST>(rOp->child(1));
                                                          oldC1 = boost::static_pointer_cast<ConstantAST>(rOp->child(0));
                                                          oldC2 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                      } else if (rOp->child(1)->getID() == AST::V_ConstantAST) {
                                                          // Case 4b
                                                          oldX = boost::static_pointer_cast<RoseAST>(rOp->child(0));
                                                          oldC1 = boost::static_pointer_cast<ConstantAST>(rOp->child(1));
                                                          oldC2 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                      }
                                                  }
                                              } else {
                                                  rOp = boost::static_pointer_cast<RoseAST>(roseAST->child(1));
                                                  if (rOp->val().op == ROSEOperation::addOp) {
                                                      if (rOp->child(0)->getID() == AST::V_ConstantAST) {
                                                          // Case 4c
                                                          oldX = boost::static_pointer_cast<RoseAST>(rOp->child(1));
                                                          oldC2 = boost::static_pointer_cast<ConstantAST>(rOp->child(0));
                                                          oldC1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                      } else if (rOp->child(1)->getID() == AST::V_ConstantAST) {
                                                          // Case 4d
                                                          oldX = boost::static_pointer_cast<RoseAST>(rOp->child(0));
                                                          oldC2 = boost::static_pointer_cast<ConstantAST>(rOp->child(1));
                                                          oldC1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                      }
                                                  }
                                              }
                                              if (oldX && oldC1 && oldC2) {
                                                  size_t size = std::max(oldC1->val().size, oldC2->val().size);
                                                  ConstantAST::Ptr newC = ConstantAST::create(Constant(oldC1->val().val + oldC2->val().val, size));
                                                  RoseAST::Ptr newRoot = RoseAST::create(ROSEOperation(roseAST->val()), oldX, newC);
                                                  return newRoot;
                                              }
                                          }
                                          break;
            case ROSEOperation::sMultOp:
            case ROSEOperation::uMultOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              uint64_t hi, lo;
                                              size_t width;
                                              constantProduct(ast, hi, lo, width);
                                              // A product that needs more than 64 bits stays a multiply, for an
                                              // enclosing extract to take its bits.
                                              if (width <= 64)
                                                  return ConstantAST::create(Constant(lowBits(lo, width), width));
                                              bool const isSigned = roseAST->val().op == ROSEOperation::sMultOp;
                                              if (hi == 0 && !(isSigned && (lo >> 63)))
                                                  return ConstantAST::create(Constant(lo, width));
                                              break;
                                          }
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              if (child0->val().val == 1 && !keepMultiOne) return roseAST->child(1);
                                          }

                                          if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              if (child1->val().val == 1 && !keepMultiOne) return roseAST->child(0);
                                          }
                                          break;
            case ROSEOperation::xorOp:
                                          if (roseAST->child(0)->getID() == AST::V_VariableAST && roseAST->child(1)->getID() == AST::V_VariableAST) {
                                              VariableAST::Ptr child0 = boost::static_pointer_cast<VariableAST>(roseAST->child(0)); 
                                              VariableAST::Ptr child1 = boost::static_pointer_cast<VariableAST>(roseAST->child(1)); 
                                              if (child0->val() == child1->val()) {
                                                  return ConstantAST::create(Constant(0 , 32));
                                              }
                                          }
                                          else if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              return ConstantAST::create(Constant(child0->val().val ^ child1->val().val,
                                                                                  std::max(child0->val().size, child1->val().size)));
                                          }

                                          break;
            case ROSEOperation::derefOp:
                                          // Any 8-bit value is bounded in [0,255].
                                          // Need to keep the length of the dereference if it is 8-bit.
                                          // However, dereference longer than 8-bit should be regarded the same.
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              uint64_t val = 0;
                                              ConstantAST::Ptr c = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              Address a = c->val().val;
                                              if (ReadMemory(a, val, roseAST->val().size / 8)) {
                                                  return ConstantAST::create(Constant(val, 64));
                                              }
                                          }
                                          if (roseAST->val().size == 8)
                                              return ast;
                                          else
                                              return RoseAST::create(ROSEOperation(ROSEOperation::derefOp), ast->child(0));
                                          break;
            case ROSEOperation::shiftLOp:
            case ROSEOperation::rotateLOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              // The result has the width of the shifted value.
                                              size_t const size = bitsOf(child0->val());
                                              uint64_t const v = lowBits(child0->val().val, size);
                                              uint64_t n = child1->val().val;
                                              uint64_t val;
                                              if (roseAST->val().op == ROSEOperation::rotateLOp) {
                                                  n %= size;
                                                  val = n ? (v << n) | (v >> (size - n)) : v;
                                              } else {
                                                  val = n < size ? v << n : 0;
                                              }
                                              return ConstantAST::create(Constant(lowBits(val, size), child0->val().size));
                                          }
                                          if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              parsing_printf("keep multi one %d\n", keepMultiOne);
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              if (child1->val().val == 0 && !keepMultiOne) return roseAST->child(0);
                                          }
                                          break;
            case ROSEOperation::andOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              auto size0{child0->val().size};
                                              auto size1{child1->val().size};
                                              uint64_t newSize{std::max(size0,size1)};
                                              return ConstantAST::create(Constant(child0->val().val & child1->val().val, newSize));
                                          }
                                          break;
            case ROSEOperation::orOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              auto size0{child0->val().size};
                                              auto size1{child1->val().size};
                                              uint64_t newSize{std::max(size0,size1)};
                                              return ConstantAST::create(Constant(child0->val().val | child1->val().val, newSize));
                                          }
                                          break;
            case ROSEOperation::ifOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr c = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              if (lowBits(c->val().val, bitsOf(c->val())) != 0) {
                                                  return roseAST->child(1);
                                              } else {
                                                  return roseAST->child(2);
                                              }

                                          }
                                          break;
            case ROSEOperation::shiftROp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              size_t const size = bitsOf(child0->val());
                                              uint64_t const n = child1->val().val;
                                              uint64_t const val = n < size ? lowBits(child0->val().val, size) >> n : 0;
                                              return ConstantAST::create(Constant(val, child0->val().size));
                                          }
                                          if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              parsing_printf("keep multi one %d\n", keepMultiOne);
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              if (child1->val().val == 0 && !keepMultiOne) return roseAST->child(0);
                                          }
                                          break;

            case ROSEOperation::equalToZeroOp:

                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              if(lowBits(child0->val().val, bitsOf(child0->val())) == 0)
                                                  return ConstantAST::create(Constant(1, 1));
                                              else
                                                  return ConstantAST::create(Constant(0, 1));
                                          }

                                          break;
            case ROSEOperation::negateOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              return ConstantAST::create(Constant(~child0->val().val, 64));
                                          }

                                          break;

            default:
                                          //std::cerr << " TODO: unkown case of rose operation " << roseAST->val().op;
                                          break;

        }
    } else if (ast->getID() == AST::V_VariableAST) {
        VariableAST::Ptr varAST = boost::static_pointer_cast<VariableAST>(ast);
        if (varAST->val().reg.absloc().isPC()) {
            MachRegister pc = varAST->val().reg.absloc().reg();	    
            return ConstantAST::create(Constant(addr, getArchAddressWidth(pc.getArchitecture()) * 8));
        }
        // We do not care about the address of the a-loc
        // because we will keep tracking the changes of 
        // each a-loc. Also, this brings a benefit that
        // we can directly use ast->isStrictEqual() to 
        // compare two ast.
        return VariableAST::create(Variable(varAST->val().reg));
    } else if (ast->getID() == AST::V_ConstantAST) {
        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(ast);
        size_t size = constAST->val().size;
        uint64_t val = constAST->val().val;	

        uint64_t clipped_value =val & ((1ULL << size) -1);

        if (size == 32)
            if (!(val & (1ULL << (size - 1))))
                return ConstantAST::create(Constant(clipped_value, 64));


    }

    return ast;
}


AST::Ptr SymbolicExpression::SimplifyAnAST(AST::Ptr ast, Address addr, bool keepMultiOne) {
    SimplifyVisitor sv(addr, keepMultiOne, *this);
    ast->accept(&sv);
    return SimplifyRoot(ast, addr, keepMultiOne);
}

bool SymbolicExpression::ContainAnAST(AST::Ptr root, AST::Ptr check) {
    if (*root == *check) return true;
    bool ret = false;
    unsigned totalChildren = root->numChildren();
    for (unsigned i = 0 ; i < totalChildren && !ret; ++i) {
        ret |= ContainAnAST(root->child(i), check);
    }
    return ret;
}


AST::Ptr SymbolicExpression::DeepCopyAnAST(AST::Ptr ast) {
    if (ast->getID() == AST::V_RoseAST) {
        RoseAST::Ptr roseAST = boost::static_pointer_cast<RoseAST>(ast);
        AST::Children kids;
        unsigned totalChildren = ast->numChildren();
        for (unsigned i = 0 ; i < totalChildren; ++i) {
            kids.push_back(DeepCopyAnAST(ast->child(i)));
        }
        return RoseAST::create(ROSEOperation(roseAST->val()), kids);
    } else if (ast->getID() == AST::V_VariableAST) {
        VariableAST::Ptr varAST = boost::static_pointer_cast<VariableAST>(ast);
        return VariableAST::create(Variable(varAST->val()));
    } else if (ast->getID() == AST::V_ConstantAST) {
        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(ast);
        return ConstantAST::create(Constant(constAST->val()));
    } else if (ast->getID() == AST::V_BottomAST) {
        BottomAST::Ptr bottomAST = boost::static_pointer_cast<BottomAST>(ast);
        return BottomAST::create(bottomAST->val());
    }
    fprintf(stderr, "ast type %d, %s\n", ast->getID(), ast->format().c_str());
    assert(0);
    return AST::Ptr();
}

pair<AST::Ptr, bool> SymbolicExpression::ExpandAssignment(Assignment::Ptr assign, bool keepMultiOne) {
    if (expandCache.find(assign) != expandCache.end()) {
        AST::Ptr ast = expandCache[assign];
        if (ast) {
            if (!keepMultiOne) ast = SimplifyAnAST(ast, 0, keepMultiOne);
            return make_pair(ast, true);
        } 
        else {
            return make_pair(ast, false);
        }
    } else {
        parsing_printf("\t\tExpanding instruction @ %lx: %s, assignment %s\n",
                assign->addr(), assign->insn().format().c_str(), assign->format().c_str());
        pair<AST::Ptr, bool> expandRet = SymEval::expand(assign, false);
        if (expandRet.second && expandRet.first) {
            parsing_printf("Original expand: %s\n", expandRet.first->format().c_str());
            AST::Ptr calculation = SimplifyAnAST(expandRet.first, 
                    PCValue(assign->addr(),
                        assign->insn().size(),
                        assign->block()->obj()->cs()->getArch()),
                    true);
            expandCache[assign] = calculation;
        } else {
            if (expandRet.first == NULL) {
                parsing_printf("\t\t\t expansion returned null ast\n");
            }
            if (expandRet.second == false) {
                parsing_printf("\t\t\t expansion returned false\n");
            }
            expandCache[assign] = AST::Ptr();
        }
        AST::Ptr ast = expandCache[assign];
        if (ast && !keepMultiOne) ast = SimplifyAnAST(ast, 0, keepMultiOne);
        return make_pair( ast, expandRet.second );
    }
}

AST::Ptr SymbolicExpression::SubstituteAnAST(AST::Ptr ast, const map<AST::Ptr, AST::Ptr> &aliasMap) {
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait)
        if (*ast == *(ait->first)) {
            return ait->second;
        }
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->setChild(i, SubstituteAnAST(ast->child(i), aliasMap));
    }
    if (ast->getID() == AST::V_VariableAST) {
        // If this variable is not in the aliasMap yet,
        // this variable is from the input.
        VariableAST::Ptr varAST = boost::static_pointer_cast<VariableAST>(ast);
        return VariableAST::create(Variable(varAST->val().reg, 1));
    }
    return ast;
}

Address SymbolicExpression::PCValue(Address cur, size_t insnSize, Architecture a) {
    switch (a) {
        case Arch_x86:
        case Arch_x86_64:
        case Arch_amdgpu_gfx908:
        case Arch_amdgpu_gfx90a:
        case Arch_amdgpu_gfx940:
        case Arch_amdgpu_gfx950:
            return cur + insnSize;
        case Arch_aarch64:
        case Arch_riscv64:
        case Arch_ppc32:
        case Arch_ppc64:
            return cur;
        case Arch_aarch32:
        case Arch_intelGen9:
        case Arch_cuda:
        case Arch_none:
            assert(0);
    }    
    return cur + insnSize;
}
