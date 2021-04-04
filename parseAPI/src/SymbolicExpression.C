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
                    uint64_t val = child->val().val;
                    if (size < 64) {
                        uint64_t mask = (1ULL << size) - 1;
                        val = (~val) & mask;
                    } else
                        val = ~val;
                    return ConstantAST::create(Constant(val, size));
                }
                break;
            case ROSEOperation::extendMSBOp: {
                                                 return roseAST->child(0);
                                             }
            case ROSEOperation::extractOp: {
                                               if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                                   size_t size = roseAST->val().size;
                                                   ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                   uint64_t val = child0->val().val;
                                                   // clip value according to size, but do not clip size 0 values
                                                   // and size 64 values
                                                   uint64_t clipped_value = (size ==0 ||size ==64 )? \ 
                                                       val : val & ((1ULL << size) -1);

                                                   return ConstantAST::create(Constant(clipped_value, size));
                                               }
                                               return roseAST->child(0);
                                           }
            case ROSEOperation::signExtendOp: {
                                                  if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                                      ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                      ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                      uint64_t val = child0->val().val;
                                                      if (val & (1 << (child0->val().size - 1))) {
                                                          switch (child0->val().size) {
                                                              case 16:
                                                                  val = val | SIGNEX_64_16;
                                                                  break;
                                                              case 32:
                                                                  val = val | SIGNEX_64_32;
                                                                  break;
                                                              default:
                                                                  break;
                                                          }
                                                      } 
                                                      size_t size = child1->val().val;
                                                      return ConstantAST::create(Constant(val,size));
                                                  }		    
                                                  return roseAST->child(0);
                                              }
            case ROSEOperation::concatOp: {	    
                                              if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                                  ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                                  ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                                  uint64_t val = (child1->val().val << child0->val().size) + child0->val().val;
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
                                              return ConstantAST::create(Constant(val,size));
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
                                          break;
            case ROSEOperation::sMultOp:
            case ROSEOperation::uMultOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              return ConstantAST::create(Constant(child0->val().val * child1->val().val, 64));
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
                                              return ConstantAST::create(Constant(child0->val().val ^ child1->val().val, 64));
                                          }

                                          break;
            case ROSEOperation::derefOp:
                                          // Any 8-bit value is bounded in [0,255].
                                          // Need to keep the length of the dereference if it is 8-bit.
                                          // However, dereference longer than 8-bit should be regarded the same.
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              uint64_t val = 0;
                                              ConstantAST::Ptr c = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              Address addr = c->val().val;
                                              if (ReadMemory(addr, val, roseAST->val().size / 8)) {
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
                                              return ConstantAST::create(Constant(child0->val().val << child1->val().val, 64));
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
                                              return ConstantAST::create(Constant(child0->val().val & child1->val().val, 64));
                                          }
                                          break;
            case ROSEOperation::orOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
                                              return ConstantAST::create(Constant(child0->val().val | child1->val().val, 64));
                                          }
                                          break;
            case ROSEOperation::ifOp:
                                          if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
                                              ConstantAST::Ptr c = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
                                              if (c->val().val != 0) {
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
                                              return ConstantAST::create(Constant(child0->val().val >> child1->val().val, 64));
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
                                              if(child0->val().val == 0)
                                                  return ConstantAST::create(Constant(1, 64));
                                              else
                                                  return ConstantAST::create(Constant(0, 64));
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
        parsing_printf("\t\tExpanding instruction @ %x: %s, assignment %s\n",
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
        case Arch_amdgpu_vega:
            return cur + insnSize;
        case Arch_aarch64:
        case Arch_amdgpu_rdna:
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
