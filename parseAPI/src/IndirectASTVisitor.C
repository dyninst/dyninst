#include "dyntypes.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"
#include "CodeObject.h"
#include <algorithm>
#include "SymbolicExpression.h"
using namespace Dyninst::ParseAPI;
#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00


AST::Ptr SimplifyVisitor::visit(DataflowAPI::RoseAST *ast) {
        unsigned totalChildren = ast->numChildren();
	for (unsigned i = 0 ; i < totalChildren; ++i) {
	    ast->child(i)->accept(this);
	    ast->setChild(i, SymbolicExpression::SimplifyRoot(ast->child(i), addr));
	}
	return AST::Ptr();
}
/*
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
*/
JumpTableFormatVisitor::JumpTableFormatVisitor(ParseAPI::Block *bl) {
    b = bl;
    numOfVar = 0;
    findIncorrectFormat = false;
    findTableBase = false;
    findIndex = false;
}

AST::Ptr JumpTableFormatVisitor::visit(DataflowAPI::RoseAST *ast) {
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }

    if (ast->val().op == ROSEOperation::derefOp) {
        // We only check the first memory read
	if (ast->child(0)->getID() == AST::V_RoseAST) {
	    RoseAST::Ptr roseAST = boost::static_pointer_cast<RoseAST>(ast->child(0));
	    if (roseAST->val().op == ROSEOperation::derefOp) {
	        // Two directly nested memory accesses cannot be jump tables
		parsing_printf("Two directly nested memory access, not jump table format\n");
	        findIncorrectFormat = true;
		return AST::Ptr();
	    }
	}
    } else if (ast->val().op == ROSEOperation::addOp) {
        Address tableBase = 0;
	if (ast->child(0)->getID() == AST::V_ConstantAST && PotentialIndexing(ast->child(1))) {
	    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(ast->child(0));
	    tableBase = (Address)constAST->val().val;
	}
	if (ast->child(1)->getID() == AST::V_ConstantAST && PotentialIndexing(ast->child(0))) {
	    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(ast->child(1));
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
		return AST::Ptr();
	    }
	    if (!b->obj()->cs()->isReadOnly(tableBase)) {
	        parsing_printf("\ttableBase 0x%lx not read only, not jump table format\n", tableBase);
		findIncorrectFormat = true;
		return AST::Ptr();
	    }
	    // Note that this table base may not be within a memory read.
	    // Functions with variable arguments often have an indirect jump with form:
	    // targetBase - index * 4
	    // We merge this special case with other general jump table cases.
	    findTableBase = true;
       }
    } else if (ast->val().op == ROSEOperation::uMultOp || ast->val().op == ROSEOperation::sMultOp) {
	if (ast->child(0)->getID() == AST::V_ConstantAST && ast->child(1)->getID() == AST::V_VariableAST) {
	    findIndex = true;
	    numOfVar++;
	    VariableAST::Ptr varAst = boost::static_pointer_cast<VariableAST>(ast->child(1));
	    index = varAst->val().reg;
	    return AST::Ptr();
	}
	if (ast->child(1)->getID() == AST::V_ConstantAST && ast->child(0)->getID() == AST::V_VariableAST) {
	    findIndex = true;
	    numOfVar++;
	    VariableAST::Ptr varAst = boost::static_pointer_cast<VariableAST>(ast->child(0));
	    index = varAst->val().reg;
	    return AST::Ptr();
	}
    }
    return AST::Ptr();
}

AST::Ptr JumpTableFormatVisitor::visit(DataflowAPI::VariableAST *ast) {
    numOfVar++;
    return AST::Ptr();
}

bool JumpTableFormatVisitor::PotentialIndexing(AST::Ptr ast) {
    if (ast->getID() == AST::V_VariableAST) return true;
    if (ast->getID() == AST::V_RoseAST) {
        RoseAST::Ptr r = boost::static_pointer_cast<RoseAST>(ast);
	if (r->val().op == ROSEOperation::uMultOp || r->val().op == ROSEOperation::sMultOp) return true;
    }
    return false;
}

/*
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
*/
