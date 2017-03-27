#include "dyntypes.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"
#include "CodeObject.h"
#include <algorithm>

using namespace Dyninst::ParseAPI;
#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00

Address PCValue(Address cur, size_t insnSize, Architecture a) {
    switch (a) {
        case Arch_x86:
	case Arch_x86_64:
	    return cur + insnSize;
	case Arch_aarch64:
	    return cur;
        case Arch_aarch32:
        case Arch_ppc32:
        case Arch_ppc64:
        case Arch_none:
            assert(0);
    }    
    return cur + insnSize;
}

AST::Ptr SimplifyVisitor::visit(DataflowAPI::RoseAST *ast) {
        unsigned totalChildren = ast->numChildren();
	for (unsigned i = 0 ; i < totalChildren; ++i) {
	    ast->child(i)->accept(this);
	    ast->setChild(i, SimplifyRoot(ast->child(i), addr));
	}
	return AST::Ptr();
}

AST::Ptr SimplifyRoot(AST::Ptr ast, Address addr) {
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
	    case ROSEOperation::extendMSBOp:
	    case ROSEOperation::extractOp:
	    case ROSEOperation::signExtendOp:
	    case ROSEOperation::concatOp:
	        return roseAST->child(0);

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
	        if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
		    if (child0->val().val == 1) return roseAST->child(1);
		}

	        if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
		    if (child1->val().val == 1) return roseAST->child(0);
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
		break;
	    case ROSEOperation::derefOp:
	        // Any 8-bit value is bounded in [0,255].
		// Need to keep the length of the dereference if it is 8-bit.
		// However, dereference longer than 8-bit should be regarded the same.
	        if (roseAST->val().size == 8)
		    return ast;
		else
		    return RoseAST::create(ROSEOperation(ROSEOperation::derefOp), ast->child(0));
		break;
	    case ROSEOperation::shiftLOp:
	        if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr child0 = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
		    ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
		    return ConstantAST::create(Constant(child0->val().val << child1->val().val, 64));
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

	    default:
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
	if (size == 32)
	    if (!(val & (1ULL << (size - 1))))
	        return ConstantAST::create(Constant(val, 64));
    }

    return ast;
}


AST::Ptr SimplifyAnAST(AST::Ptr ast, Address addr) {
    SimplifyVisitor sv(addr);
    ast->accept(&sv);
    return SimplifyRoot(ast, addr);
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::RoseAST *ast) {
    BoundValue *astBound = boundFact.GetBound(ast);
    if (astBound != NULL) {
        bound.insert(make_pair(ast, new BoundValue(*astBound)));
        return AST::Ptr();
    }
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }
    switch (ast->val().op) {
        case ROSEOperation::addOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {	    
		BoundValue* val = new BoundValue(*GetResultBound(ast->child(0)));		
		val->Add(*GetResultBound(ast->child(1)));
		if (*val != BoundValue::top)
		    bound.insert(make_pair(ast, val));
	    }	    
	    break;
	case ROSEOperation::invertOp:
	    if (IsResultBounded(ast->child(0))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(0)));
		if (val->tableReadSize)
		    val->isInverted = true;
		else {
		    val->Invert();
		}
		if (*val != BoundValue::top)
		    bound.insert(make_pair(ast,val));
	    }
	    break;
	case ROSEOperation::andOp: {
	    // For and operation, even one of them is a top,
	    // we may still produce bound result.
	    // For example, top And 0[3,3] => 1[0,3]
	    //
	    // the bound produced by and may be more relaxed than
	    // a cmp bound not found yet. So we only apply and
	    // bound when this is the last attempt
	    if (handleOneByteRead) {
	        BoundValue *val = NULL;
		if (IsResultBounded(ast->child(0)))
		    val = new BoundValue(*GetResultBound(ast->child(0)));
		else
		    val = new BoundValue(BoundValue::top);
		if (IsResultBounded(ast->child(1)))
		    val->And(*GetResultBound(ast->child(1)));
		else
		    val->And(StridedInterval::top);
		// the result of an AND operation should not be
	        // the table lookup. Set all other values to default
	        val->ClearTableCheck();
	        if (*val != BoundValue::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	}
	case ROSEOperation::sMultOp:
	case ROSEOperation::uMultOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(0)));
	        val->Mul(*GetResultBound(ast->child(1)));
	        if (*val != BoundValue::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::shiftLOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(0)));
	        val->ShiftLeft(*GetResultBound(ast->child(1)));
	        if (*val != BoundValue::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::shiftROp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(0)));
	        val->ShiftRight(*GetResultBound(ast->child(1)));
	        if (*val != BoundValue::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::derefOp: 
	    if (IsResultBounded(ast->child(0))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(0)));
		val->MemoryRead(block, derefSize);
	        if (*val != BoundValue::top)
	            bound.insert(make_pair(ast, val));
	    } else if (handleOneByteRead && ast->val().size == 8) {
	        // Any 8-bit value is bounded in [0,255]
		// But I should only do this when I know the read 
		// itself is not a jump table
	        bound.insert(make_pair(ast, new BoundValue(StridedInterval(1,0,255))));
	    }
	    break;
	case ROSEOperation::orOp: 
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(0)));
	        val->Or(*GetResultBound(ast->child(1)));
	        if (*val != BoundValue::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::ifOp:
	    if (IsResultBounded(ast->child(1)) && IsResultBounded(ast->child(2))) {
	        BoundValue *val = new BoundValue(*GetResultBound(ast->child(1)));
		val->Join(*GetResultBound(ast->child(2)), block);
		if (*val != BoundValue::top)
		    bound.insert(make_pair(ast, val));
	    }
	default:
	    break;
    }
    return AST::Ptr();
   
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::ConstantAST *ast) {
    const Constant &v = ast->val();
    int64_t value = v.val;
    if (v.size != 1 && v.size != 64 && (value & (1ULL << (v.size - 1)))) {
        // Compute the two complements in bits of v.size
	// and change it to a negative number
        value = -(((~value) & ((1ULL << v.size) - 1)) + 1);
    }
    bound.insert(make_pair(ast, new BoundValue(value)));
    return AST::Ptr();
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::VariableAST *ast) {
    BoundValue *astBound = boundFact.GetBound(ast);
    if (astBound != NULL) 
        bound.insert(make_pair(ast, new BoundValue(*astBound)));
    return AST::Ptr();
}

BoundValue* BoundCalcVisitor::GetResultBound(AST::Ptr ast) {
    if (IsResultBounded(ast)) {
	return bound.find(ast.get())->second;
    } else {
	return NULL;
    }	    
}

BoundCalcVisitor::~BoundCalcVisitor() {
    for (auto bit = bound.begin(); bit != bound.end(); ++bit)
        if (bit->second != NULL)
	    delete bit->second;
    bound.clear();
}


AST::Ptr JumpCondVisitor::visit(DataflowAPI::RoseAST *ast) {
    if (ast->val().op == ROSEOperation::invertOp) {
        invertFlag = true;
	return AST::Ptr();
    }
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }
    return AST::Ptr();
}

AST::Ptr ComparisonVisitor::visit(DataflowAPI::RoseAST *ast) {
    // For cmp type instruction setting zf
    // Looking like <eqZero?>(<add>(<V([x86_64::rbx])>,<Imm:8>,),)
    // Assuming ast has been simplified
    if (ast->val().op == ROSEOperation::equalToZeroOp) {
        bool minuendIsZero = true;
        AST::Ptr child = ast->child(0);	
	if (child->getID() == AST::V_RoseAST) {
	    RoseAST::Ptr childRose = boost::static_pointer_cast<RoseAST>(child);
	    if (childRose->val().op == ROSEOperation::addOp) {
	        minuendIsZero = false;
	        subtrahend = childRose->child(0);
		minuend = childRose->child(1);
		// If the minuend is a constant, then
		// the minuend is currently in its two-complement form
		if (minuend->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(minuend);
		    uint64_t val = constAST->val().val;
		    int size = constAST->val().size;
		    if (size < 64) 
		        val = ((~val)+ 1) & ((1ULL << size) - 1);
		    else if (size == 64)
		        val = (~val) + 1;
		    else
		        parsing_printf("WARNING: constant bit size %d exceeds 64!\n", size);
		    minuend = ConstantAST::create(Constant(val, size));
		} else if (minuend->getID() == AST::V_RoseAST) {
		    RoseAST::Ptr sub = boost::static_pointer_cast<RoseAST>(minuend);
		    minuend = AST::Ptr();
		    if (sub->val().op == ROSEOperation::addOp && sub->child(0)->getID() == AST::V_RoseAST) {
		        sub = boost::static_pointer_cast<RoseAST>(sub->child(0));
			if (sub->val().op == ROSEOperation::invertOp) {
			    // Otherwise, the minuend ast is in the form of add(invert(minuend), 1)
  		            // Need to extract the real minuend
		             minuend = sub->child(0);
			}
		    }
		}
	    } 	
	} 
	if (minuendIsZero) {
            // The minuend is 0, thus the add operation is subsume.
             subtrahend = ast->child(0);
	     minuend = ConstantAST::create(Constant(0));
	}
    }
    return AST::Ptr();
}

AST::Ptr SubstituteAnAST(AST::Ptr ast, const BoundFact::AliasMap &aliasMap) {
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

bool ContainAnAST(AST::Ptr root, AST::Ptr check) {
    if (*root == *check) return true;
    bool ret = false;
    unsigned totalChildren = root->numChildren();
    for (unsigned i = 0 ; i < totalChildren && !ret; ++i) {
        ret |= ContainAnAST(root->child(i), check);
    }
    return ret;
}


AST::Ptr DeepCopyAnAST(AST::Ptr ast) {
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

AST::Ptr JumpTableFormatVisitor::visit(DataflowAPI::RoseAST *ast) {

    bool findIncorrectFormat = false;
    if (ast->val().op == ROSEOperation::derefOp) {
        // We only check the first memory read
	if (ast->child(0)->getID() == AST::V_RoseAST) {
	    RoseAST::Ptr roseAST = boost::static_pointer_cast<RoseAST>(ast->child(0));
	    if (roseAST->val().op == ROSEOperation::derefOp) {
	        // Two directly nested memory accesses cannot be jump tables
		parsing_printf("Two directly nested memory access, not jump table format\n");
	        findIncorrectFormat = true;
	    } else if (roseAST->val().op == ROSEOperation::addOp) {
	        Address tableBase = 0;
		if (roseAST->child(0)->getID() == AST::V_ConstantAST && roseAST->child(1)->getID() == AST::V_VariableAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
		    tableBase = (Address)constAST->val().val;
		}
		if (roseAST->child(1)->getID() == AST::V_ConstantAST && roseAST->child(0)->getID() == AST::V_VariableAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
		    tableBase = (Address)constAST->val().val;
		}
		if (tableBase) {
		    Architecture arch = b->obj()->cs()->getArch();
		    if (arch == Arch_x86) {
		        tableBase &= 0xffffffff;
		    }
#if defined(os_windows)
                    tableBase -= b->obj()->cs()->loadAddress();
#endif
                    if (!b->obj()->cs()->isValidAddress(tableBase)) {
		        parsing_printf("\ttableBase 0x%lx invalid, not jump table format\n", tableBase);
			findIncorrectFormat = true;
		    }
                    if (!b->obj()->cs()->isReadOnly(tableBase)) {
		        parsing_printf("\ttableBase 0x%lx not read only, not jump table format\n", tableBase);
			findIncorrectFormat = true;
		    }

		}
	    }
	}
	if (findIncorrectFormat) {
	    format = false;
	}
	return AST::Ptr();
    }
    if (!findIncorrectFormat) {
        unsigned totalChildren = ast->numChildren();
	for (unsigned i = 0 ; i < totalChildren; ++i) {
	    ast->child(i)->accept(this);
	}
    } 
    return AST::Ptr();
}

bool PerformTableRead(BoundValue &target, set<int64_t> & jumpTargets, CodeSource *cs) {
    if (target.tableReadSize > 0 && target.interval.stride == 0) {
        // This is a PC-relative read to variable, not a table read
        return false;
    }
    Address tableBase = (Address)target.interval.low;
    Address tableLastEntry = (Address)target.interval.high;
    int addressWidth = cs->getAddressWidth();
    if (addressWidth == 4) {
        tableBase &= 0xffffffff;
	tableLastEntry &= 0xffffffff;
    }

#if defined(os_windows)
    tableBase -= cs->loadAddress();
    tableLastEntry -= cs->loadAddress();
#endif

    if (!cs->isCode(tableBase) && !cs->isData(tableBase)) {
        parsing_printf("\ttableBase 0x%lx invalid, returning false\n", tableBase);
	parsing_printf("Not jump table format!\n");
	return false;
    }
    if (!cs->isReadOnly(tableBase)) {
        parsing_printf("\ttableBase 0x%lx not read only, returning false\n", tableBase);
	parsing_printf("Not jump table format!\n");
        return false;
    }


    for (Address tableEntry = tableBase; tableEntry <= tableLastEntry; tableEntry += target.interval.stride) {
	if (!cs->isCode(tableEntry) && !cs->isData(tableEntry)) continue;
	if (!cs->isReadOnly(tableEntry)) continue;
	int64_t targetAddress = 0;
	if (target.tableReadSize > 0) {
	    switch (target.tableReadSize) {
	        case 8:
		    targetAddress = *(const uint64_t *) cs->getPtrToInstruction(tableEntry);
		    break;
		case 4:
		    targetAddress = *(const uint32_t *) cs->getPtrToInstruction(tableEntry);
		    if (target.isZeroExtend) break;
		    if ((addressWidth == 8) && (targetAddress & 0x80000000)) {
		        targetAddress |= SIGNEX_64_32;
		    }
		    break;
		case 2:
		    targetAddress = *(const uint16_t *) cs->getPtrToInstruction(tableEntry);
		    if (target.isZeroExtend) break;
		    if ((addressWidth == 8) && (targetAddress & 0x8000)) {
		        targetAddress |= SIGNEX_64_16;
		    }
		    if ((addressWidth == 4) && (targetAddress & 0x8000)) {
		        targetAddress |= SIGNEX_32_16;
		    }

		    break;
		case 1:
		    targetAddress = *(const uint8_t *) cs->getPtrToInstruction(tableEntry);
		    if (target.isZeroExtend) break;
		    if ((addressWidth == 8) && (targetAddress & 0x80)) {
		        targetAddress |= SIGNEX_64_8;
		    }
		    if ((addressWidth == 4) && (targetAddress & 0x80)) {
		        targetAddress |= SIGNEX_32_8;
		    }

		    break;

		default:
		    parsing_printf("Invalid memory read size %d\n", target.tableReadSize);
		    return false;
	    }
	    targetAddress *= target.multiply;
	    if (target.targetBase != 0) {
	        if (target.isSubReadContent) 
		    targetAddress = target.targetBase - targetAddress;
		else 
		    targetAddress += target.targetBase; 

	    }
#if defined(os_windows)
            targetAddress -= cs->loadAddress();
#endif
	} else targetAddress = tableEntry;

	if (addressWidth == 4) targetAddress &= 0xffffffff;
	parsing_printf("Jumping to target %lx,", targetAddress);
	if (cs->isCode(targetAddress)) {
	    // Jump tables may contain may repetitious entries.
	    // We only want to create one edge for disctinct each jump target.
	    jumpTargets.insert(targetAddress);
	}
	// If the jump target is resolved to be a constant, 
	if (target.interval.stride == 0) break;
    }
    return true;
}
