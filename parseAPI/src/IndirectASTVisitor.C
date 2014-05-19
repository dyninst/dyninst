#include "IndirectASTVisitor.h"
#include <algorithm>


AST::Ptr SimplifyVisitor::visit(DataflowAPI::RoseAST *ast) {
        unsigned totalChildren = ast->numChildren();
	for (unsigned i = 0 ; i < totalChildren; ++i) {
	    ast->child(i)->accept(this);
	    ast->setChild(i, SimplifyRoot(ast->child(i), size));
	}
	return AST::Ptr();
}

AST::Ptr SimplifyRoot(AST::Ptr ast, uint64_t insnSize) {
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
		    uint64_t mask = (1LL << size) - 1;
		    val = (val & mask) ^ mask;
		    return ConstantAST::create(Constant(val, size));
		}
		break;
	    case ROSEOperation::extendMSBOp:
	    case ROSEOperation::extractOp:
	    case ROSEOperation::signExtendOp:
	    case ROSEOperation::concatOp:
	        return roseAST->child(0);

	    case ROSEOperation::addOp:
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
		if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr child = boost::static_pointer_cast<ConstantAST>(roseAST->child(0));
		    if (child->val().val == 0) return roseAST->child(1);
		}
		if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr child = boost::static_pointer_cast<ConstantAST>(roseAST->child(1));
		    if (child->val().val == 0) return roseAST->child(0);
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
	        return RoseAST::create(ROSEOperation(ROSEOperation::derefOp), ast->child(0));
	    default:
	        break;

	}
    } else if (ast->getID() == AST::V_VariableAST) {
        VariableAST::Ptr varAST = boost::static_pointer_cast<VariableAST>(ast);
	if (varAST->val().reg.absloc().isPC()) {
	    MachRegister pc = varAST->val().reg.absloc().reg();	   
	    return ConstantAST::create(Constant(varAST->val().addr + insnSize, getArchAddressWidth(pc.getArchitecture()) * 8));
	}
	return VariableAST::create(Variable(varAST->val().reg));
    }
    return ast;
}


AST::Ptr SimplifyAnAST(AST::Ptr ast, uint64_t size) {
    SimplifyVisitor sv(size);
    ast->accept(&sv);
    return SimplifyRoot(ast, size);
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::RoseAST *ast) {
    if (boundFact.CMPBoundMatch(ast)) {
        bound[ast] = BoundValue(LessThan, boundFact.cmpBound, 1, 0, 0, false, false); 
    }
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }
    switch (ast->val().op) {
        case ROSEOperation::addOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {	    
	        BoundValue child1 = GetResultBound(ast->child(0));
		BoundValue child2 = GetResultBound(ast->child(1));
		BoundValue &val = bound[ast];
		if (child1.type == Equal && child2.tableLookup) {
		    val = child2;
		    val.targetBase = child1.value;
		    val.tableOffset = true;
		    val.posi = child2.posi;
		} else if (child2.type == Equal && child1.tableLookup) {
		    val = child1;
		    val.targetBase = child2.value;
		    val.tableOffset = true;
		    val.posi = child1.posi;
		} else if (child2.CoeBounded()) {
		    val.type = LessThan;
		    val.value = child2.value;
	            val.tableBase = child1.value;
		    val.coe = child2.coe;
		} else if (child1.CoeBounded()) {  
		    val.type = LessThan;
		    val.value = child1.value;
	            val.tableBase = child2.value;
		    val.coe = child1.coe;
		} else {
		    val.value = child1.value + child2.value;
		    if (child1.type == Equal && child2.type == Equal)
		        val.type = Equal;
		    else
		        val.type = LessThan;
		}
		val.tableLookup = false;
	    }
	    // Now check if this subexpression is actually a subtraction
	    if (ast->child(0)->getID() == AST::V_RoseAST && ast->child(1)->getID() == AST::V_ConstantAST) {
	        RoseAST::Ptr child0 = boost::static_pointer_cast<RoseAST>(ast->child(0));
		ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(ast->child(1));
		if (child1->val().val == 1 && child0->val().op == ROSEOperation::invertOp)
		    if (IsResultBounded(child0->child(0))) {
		        BoundValue grandChild = GetResultBound(child0->child(0));
			BoundValue &val = bound[ast];
			val = grandChild;
			val.posi = false;
		    }
	    }
	    break;
	case ROSEOperation::andOp:
	    if (IsResultBounded(ast->child(0)) || IsResultBounded(ast->child(1))) {
	        if (IsResultBounded(ast->child(0))) bound[ast] = GetResultBound(ast->child(0)); else bound[ast] = GetResultBound(ast->child(1));
		bound[ast].type = LessThan;
		bound[ast].coe = 1;
		bound[ast].tableBase = 0;
		bound[ast].tableLookup = false;
	    }
	    break;
	case ROSEOperation::sMultOp:
	case ROSEOperation::uMultOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))){
	        BoundValue child1 = GetResultBound(ast->child(0));
		BoundValue child2 = GetResultBound(ast->child(1));
		BoundValue &val = bound[ast];
		if (child2.value < 10) {
		    val.value = child1.value;
		    val.coe = child1.coe * child2.value;
		}
		else {
		    val.value = child2.value;
		    val.coe = child2.coe * child1.value;
		}
		val.tableBase = 0;
		if (child1.type == Equal && child2.type == Equal)
		    val.type = Equal;
		else
		    val.type = LessThan;
		val.tableLookup = false;
	    }
	    break;

	case ROSEOperation::shiftLOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        BoundValue child1 = GetResultBound(ast->child(0));
		BoundValue child2 = GetResultBound(ast->child(1));
		BoundValue &val = bound[ast];
		val.value = child1.value;
		val.coe = child1.coe << child2.value;
		val.tableBase = 0;
		if (child1.type == Equal && child2.type == Equal)
		    val.type = Equal;
		else
		    val.type = LessThan;
		val.tableLookup = false;
	    }
	    break;
	case ROSEOperation::derefOp:
	    if (IsResultBounded(ast->child(0))) {
	        BoundValue child = GetResultBound(ast->child(0));
		BoundValue &val = bound[ast];

		if (child.type == LessThan && child.CoeBounded() && child.HasTableBase()) {
		    val = child;
		    val.tableLookup = true;
		}

	    }
	    break;
	case ROSEOperation::orOp:
	    if (IsResultBounded(ast->child(0)) && IsResultBounded(ast->child(1))) {
	        BoundValue child1 = GetResultBound(ast->child(0));
		BoundValue child2 = GetResultBound(ast->child(1));
		BoundValue &val = bound[ast];
		// TODO: A temporory method
		val.type = LessThan;
		val.coe = 1;
		val.value = (child1.value * child1.coe) | (child2.value * child2.coe);
	    }
	    break;
	default:
	    break;
    }
    return AST::Ptr();
   
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::ConstantAST *ast) { 
    bound[ast] = BoundValue(Equal, ast->val().val, 1, 0, 0, false, false);
    return AST::Ptr();
}

AST::Ptr BoundCalcVisitor::visit(DataflowAPI::VariableAST *ast) {
    if (boundFact.CMPBoundMatch(ast)) {
        bound[ast] = BoundValue(LessThan, boundFact.cmpBound, 1, 0, 0, false, false); 
    }

    const Absloc& al = ast->val().reg.absloc();
    if (boundFact.IsBounded(al)) bound[ast] = boundFact.GetBound(al);
    return AST::Ptr();
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
    // For cmp type instruction
    if (ast->val().op == ROSEOperation::extendMSBOp) {
        AST::Ptr child = ast->child(0);
	if (child->getID() == AST::V_RoseAST) {
	    RoseAST::Ptr childRose = boost::static_pointer_cast<RoseAST>(child);
	    if (childRose->val().op == ROSEOperation::invertOp) {
	        minuend = childRose->child(0);
		return AST::Ptr();
	    }
	}
	subtrahend = child;
	return AST::Ptr();
    }
    // For test type instruction
    if (ast->val().op == ROSEOperation::equalToZeroOp) {
        subtrahend = ast->child(0);
	minuend = ConstantAST::create(Constant(0));
    }
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        ast->child(i)->accept(this);
    }
    return AST::Ptr();
}


