#include "dyntypes.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"
#include "CodeObject.h"
#include <algorithm>
using namespace Dyninst::ParseAPI;

AST::Ptr SimplifyVisitor::visit(DataflowAPI::RoseAST *ast) {
        unsigned totalChildren = ast->numChildren();
	for (unsigned i = 0 ; i < totalChildren; ++i) {
	    ast->child(i)->accept(this);
	    ast->setChild(i, se.SimplifyRoot(ast->child(i), addr, keepMultiOne));
	}
	return AST::Ptr();
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::RoseAST *ast) {
    StridedInterval *astBound = boundFact.GetBound(ast);
    if (astBound != NULL) {
        bound.insert(make_pair(ast, new StridedInterval(*astBound)));
        return AST::Ptr();
    }
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }
    switch (ast->val().op) {
        case ROSEOperation::addOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {	    
		StridedInterval* val = new StridedInterval(*GetResultBound(ast->child(0)));		
		val->Add(*GetResultBound(ast->child(1)));
		if (*val != StridedInterval::top)
		    bound.insert(make_pair(ast, val));
	    }	    
	    break;
	case ROSEOperation::invertOp:
	    if (IsResultBounded(ast->child(0))) {
	        StridedInterval *val = new StridedInterval(*GetResultBound(ast->child(0)));
		val->Not();
		if (*val != StridedInterval::top)
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
	        parsing_printf("\tTry to generate bound for AND\n");
	        StridedInterval *val = NULL;
		if (IsResultBounded(ast->child(0)))
		    val = new StridedInterval(*GetResultBound(ast->child(0)));
		else
		    val = new StridedInterval(StridedInterval::top);
		if (IsResultBounded(ast->child(1)))
		    val->And(*GetResultBound(ast->child(1)));
		else
		    val->And(StridedInterval::top);
		// the result of an AND operation should not be
	        // the table lookup. Set all other values to default
	        if (*val != StridedInterval::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	}
	case ROSEOperation::sMultOp:
	case ROSEOperation::uMultOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        StridedInterval *val = new StridedInterval(*GetResultBound(ast->child(0)));
	        val->Mul(*GetResultBound(ast->child(1)));
	        if (*val != StridedInterval::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::shiftLOp:
	case ROSEOperation::rotateLOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        StridedInterval *val = new StridedInterval(*GetResultBound(ast->child(0)));
	        val->ShiftLeft(*GetResultBound(ast->child(1)));
	        if (*val != StridedInterval::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::shiftROp:
	case ROSEOperation::rotateROp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        StridedInterval *val = new StridedInterval(*GetResultBound(ast->child(0)));
	        val->ShiftRight(*GetResultBound(ast->child(1)));
	        if (*val != StridedInterval::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::derefOp: 
	    if (handleOneByteRead && ast->val().size == 8) {
	        // Any 8-bit value is bounded in [0,255]
		// But I should only do this when I know the read 
		// itself is not a jump table
	        bound.insert(make_pair(ast, new StridedInterval(1,0,255)));
	    }
	    break;
	case ROSEOperation::orOp: 
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        StridedInterval *val = new StridedInterval(*GetResultBound(ast->child(0)));
	        val->Or(*GetResultBound(ast->child(1)));
	        if (*val != StridedInterval::top)
	            bound.insert(make_pair(ast, val));
	    }
	    break;
	case ROSEOperation::ifOp:
	    if (IsResultBounded(ast->child(1)) && IsResultBounded(ast->child(2))) {
	        StridedInterval *val = new StridedInterval(*GetResultBound(ast->child(1)));
		val->Join(*GetResultBound(ast->child(2)));
		if (*val != StridedInterval::top)
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
    parsing_printf("\t\tGet a constant %ld\n", value);
    bound.insert(make_pair(ast, new StridedInterval(value)));
    return AST::Ptr();
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::VariableAST *ast) {
    StridedInterval *astBound = boundFact.GetBound(ast);
    if (astBound != NULL) 
        bound.insert(make_pair(ast, new StridedInterval(*astBound)));
    return AST::Ptr();
}

StridedInterval* BoundCalcVisitor::GetResultBound(AST::Ptr ast) {
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
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }
    if (ast->val().op == ROSEOperation::equalToZeroOp && !subtrahend) {
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
	    if (childRose->val().op == ROSEOperation::xorOp) {
	        minuendIsZero = false;
	        subtrahend = childRose->child(0);
		minuend = childRose->child(1);
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

JumpTableFormatVisitor::JumpTableFormatVisitor(ParseAPI::Block *bl) {
    b = bl;
    numOfVar = 0;
    memoryReadLayer = 0; 
    findIncorrectFormat = false;
    findTableBase = false;
    findIndex = false;
    firstAdd = true;
}

AST::Ptr JumpTableFormatVisitor::visit(DataflowAPI::RoseAST *ast) {
    if (ast->val().op == ROSEOperation::derefOp) {
        memoryReadLayer++;
	if (memoryReadLayer > 1) {
	    parsing_printf("More than one layer of memory accesses, not jump table format\n");
	    findIncorrectFormat = true;
	    return AST::Ptr();
	}
	ast->child(0)->accept(this);
	memoryReadLayer--;
	return AST::Ptr();
    }

    if (ast->val().op == ROSEOperation::addOp && memoryReadLayer > 0 && firstAdd) {
        firstAdd = false;
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
/*
	    if (!b->obj()->cs()->isReadOnly(tableBase)) {
	        parsing_printf("\ttableBase 0x%lx not read only, not jump table format\n", tableBase);
		findIncorrectFormat = true;
		return AST::Ptr();
	    }
*/	    
	    findTableBase = true;
       }
    } 
    
    if ((ast->val().op == ROSEOperation::uMultOp || 
         ast->val().op == ROSEOperation::sMultOp || 
	 ast->val().op == ROSEOperation::shiftLOp ||
	 ast->val().op == ROSEOperation::rotateLOp) && memoryReadLayer > 0) {
	if (ast->child(0)->getID() == AST::V_ConstantAST && ast->child(1)->getID() == AST::V_VariableAST) {
	    ConstantAST::Ptr constAst = boost::static_pointer_cast<ConstantAST>(ast->child(0));	   
	    if (!((ast->val().op == ROSEOperation::uMultOp || ast->val().op == ROSEOperation::sMultOp) &&
	        !findTableBase &&
		constAst->val().val == 1)) {
		findIndex = true;
		numOfVar++;
		VariableAST::Ptr varAst = boost::static_pointer_cast<VariableAST>(ast->child(1));
		index = varAst->val().reg;
		return AST::Ptr();
	    }
	}
	if (ast->child(1)->getID() == AST::V_ConstantAST && ast->child(0)->getID() == AST::V_VariableAST) {
	    ConstantAST::Ptr constAst = boost::static_pointer_cast<ConstantAST>(ast->child(1));	   
	    if (!((ast->val().op == ROSEOperation::uMultOp || ast->val().op == ROSEOperation::sMultOp) &&
	        !findTableBase &&
		constAst->val().val == 1)) {
		findIndex = true;
		numOfVar++;
		VariableAST::Ptr varAst = boost::static_pointer_cast<VariableAST>(ast->child(0));
		index = varAst->val().reg;
		return AST::Ptr();
	    }
	}
    }

    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }

    return AST::Ptr();
}

AST::Ptr JumpTableFormatVisitor::visit(DataflowAPI::VariableAST *) {
    numOfVar++;
    return AST::Ptr();
}

bool JumpTableFormatVisitor::PotentialIndexing(AST::Ptr ast) {
    if (ast->getID() == AST::V_VariableAST) return true;
    if (ast->getID() == AST::V_RoseAST) {
        RoseAST::Ptr r = boost::static_pointer_cast<RoseAST>(ast);
	if (r->val().op == ROSEOperation::uMultOp || 
	    r->val().op == ROSEOperation::sMultOp || 
	    r->val().op == ROSEOperation::shiftLOp || 
	    r->val().op == ROSEOperation::rotateLOp) {
	    if (r->child(0)->getID() == AST::V_RoseAST) {
	        return false;
	    }
	    return true;
	}
	if (r->val().op == ROSEOperation::addOp) {
	    // The index can be subtracted 
	    if (r->child(0)->getID() == AST::V_RoseAST && r->child(1)->getID() == AST::V_ConstantAST) {
	        RoseAST::Ptr lc = boost::static_pointer_cast<RoseAST>(r->child(0));
		ConstantAST::Ptr rc = boost::static_pointer_cast<ConstantAST>(r->child(1));
		if (lc->val().op == ROSEOperation::invertOp && rc->val().val == 1) {
		    return PotentialIndexing(lc->child(0));
		}
	    }
	}
    }
    return false;
}

JumpTableReadVisitor::JumpTableReadVisitor(AbsRegion i, int v, CodeSource *c, CodeRegion *r, bool ze, int m) {
    index = i;
    indexValue = v;
    cs = c;
    cr = r;
    isZeroExtend = ze;
    valid = true;
    memoryReadSize = m;
}

AST::Ptr JumpTableReadVisitor::visit(DataflowAPI::RoseAST *ast) {
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
	if (!valid) return AST::Ptr();
    }

    // As soon as we do not know the value of one child, we will return.
    // So, we will always have good values for each child at this point.
    switch (ast->val().op) {
        case ROSEOperation::addOp:
	    results.insert(make_pair(ast, results[ast->child(0).get()] + results[ast->child(1).get()]));
	    break;
	case ROSEOperation::invertOp:
	    results.insert(make_pair(ast, ~results[ast->child(0).get()]));
	    break;
	case ROSEOperation::andOp: 
	    results.insert(make_pair(ast, results[ast->child(0).get()] & results[ast->child(1).get()]));
	    break;
	case ROSEOperation::sMultOp:
	case ROSEOperation::uMultOp:
	    results.insert(make_pair(ast, results[ast->child(0).get()] * results[ast->child(1).get()]));
	    break;
	case ROSEOperation::shiftLOp:
	case ROSEOperation::rotateLOp:
	    results.insert(make_pair(ast, results[ast->child(0).get()] << results[ast->child(1).get()]));
	    break;
	case ROSEOperation::shiftROp:
	case ROSEOperation::rotateROp:
	    results.insert(make_pair(ast, results[ast->child(0).get()] >> results[ast->child(1).get()]));
	    break;
	case ROSEOperation::derefOp: {
	        int64_t v;
	        bool validRead = PerformMemoryRead(results[ast->child(0).get()], v);
		if (!validRead) {
		    valid = false;
		    // We encounter an invalid table entry
		    parsing_printf("WARNING: invalid table entry for index value %ld\n", indexValue);
		    return AST::Ptr();
		}
		results.insert(make_pair(ast, v));
	    }
	    break;
	case ROSEOperation::orOp: 
	    results.insert(make_pair(ast, results[ast->child(0).get()] | results[ast->child(1).get()]));
	    break;
	default:
	    parsing_printf("WARNING: unhandled operation in the jump table format AST!\n");
	    valid = false;
	    break;
    }
    targetAddress = results[ast];
    if (cs->getAddressWidth() == 4) {
        targetAddress &= 0xffffffff;
    }
#if defined(os_windows)
    targetAddress -= cs->loadAddress();
#endif

    return AST::Ptr();
   
}

AST::Ptr JumpTableReadVisitor::visit(DataflowAPI::ConstantAST *ast) {
    const Constant &v = ast->val();
    int64_t value = v.val;
    if (v.size != 1 && v.size != 64 && (value & (1ULL << (v.size - 1)))) {
        // Compute the two complements in bits of v.size
	// and change it to a negative number
        value = -(((~value) & ((1ULL << v.size) - 1)) + 1);
    }
    results.insert(make_pair(ast, value));
    return AST::Ptr();
}


AST::Ptr JumpTableReadVisitor::visit(DataflowAPI::VariableAST * var) {
    if (var->val().reg != index) {
        // The only variable in the jump table format AST should the index.
	// If it is not the case, something is wrong
	parsing_printf("WARNING: the jump table format AST contains a variable that is not the index\n");
        valid = false;
    }
    results.insert(make_pair(var, indexValue));
    return AST::Ptr();
}


bool JumpTableReadVisitor::PerformMemoryRead(Address addr, int64_t &v) {
    readAddress = addr;
    int addressWidth = cs->getAddressWidth();
    if (addressWidth == 4) {
        addr &= 0xffffffff;
    }

#if defined(os_windows)
    addr -= cs->loadAddress();
#endif
    if (!cs->isCode(addr) && !cs->isData(addr)) return false;
//    if (!cs->isReadOnly(addr)) return false;
    switch (memoryReadSize) {
        case 8:
	    v = *(const uint64_t *) cs->getPtrToInstruction(addr);
	    break;
	case 4:
	    v = *(const uint32_t *) cs->getPtrToInstruction(addr);
	    if (isZeroExtend) break;
	    if ((addressWidth == 8) && (v & 0x80000000)) {
	        v |= SIGNEX_64_32;
	    }
	    break;
	case 2:
	    v = *(const uint16_t *) cs->getPtrToInstruction(addr);
	    if (isZeroExtend) break;
	    if ((addressWidth == 8) && (v & 0x8000)) {
	        v |= SIGNEX_64_16;
	    }
	    if ((addressWidth == 4) && (v & 0x8000)) {
	        v |= SIGNEX_32_16;
	    }
	    break;
	case 1:
	    v = *(const uint8_t *) cs->getPtrToInstruction(addr);
	    if (isZeroExtend) break;
	    if ((addressWidth == 8) && (v & 0x80)) {
	        v |= SIGNEX_64_8;
	    }
	    if ((addressWidth == 4) && (v & 0x80)) {
	        v |= SIGNEX_32_8;
	    }
	    break;	    
	default:
	    parsing_printf("Invalid memory read size %d\n", memoryReadSize);
	    return false;
    }
    return true;
}
