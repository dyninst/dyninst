/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: BPatch_snippet.C,v 1.28 2000/10/17 17:42:14 schendel Exp $

#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "process.h"
#include "instPoint.h"

#include "BPatch.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "common/h/Time.h"
#include "common/h/timing.h"

/*
 * BPatch_snippet::BPatch_snippet
 *
 * Copy constructor for BPatch_snippet.
 */
BPatch_snippet::BPatch_snippet(const BPatch_snippet &src)
{
    ast = assignAst(src.ast);
}


/*
 * BPatch_snippet::operator=
 *
 * Assignment operator for BPatch_snippet.  Needed to ensure that the
 * reference counts for the asts contained in the snippets is correct.
 */
BPatch_snippet &BPatch_snippet::operator=(const BPatch_snippet &src)
{
    // Check for x = x
    if (&src == this)
        return *this;

    // Since we're copying over this snippet, release the old AST
    if (ast != NULL)
        removeAst(ast);

    // We'll now contain another reference to the ast in the other snippet
    ast = assignAst(src.ast);

    return *this;
}


/*
 * BPatch_snippet:getCost
 *
 * Returns the estimated cost of executing the snippet, in seconds.
 */
float BPatch_snippet::getCost()
{
  timeLength costv = timeLength(ast->cost(), getCyclesPerSecond());
  float retCost = static_cast<float>(costv.getD(timeUnit::sec()));
  return retCost;
}


/*
 * BPatch_snippet::~BPatch_snippet
 *
 * Destructor for BPatch_snippet.  Deallocates memory allocated by the
 * snippet.
 */
BPatch_snippet::~BPatch_snippet()
{
    // if (ast != NULL)
        // removeAst(ast);
}


//
// generateArrayRef - Construct an Ast expression for an array.
//
AstNode *generateArrayRef(const BPatch_snippet &lOperand, 
			  const BPatch_snippet &rOperand)
{
    AstNode *ast;

    if (!lOperand.ast || !rOperand.ast) {
	return NULL;
    }
    BPatch_type *arrayType = const_cast<BPatch_type *>(lOperand.ast->getType());
    if (!arrayType) {
	BPatch_reportError(BPatchSerious, 109,
	       "array reference has not type information");
	return NULL;
    }
    if (arrayType->getDataClass() != BPatch_array) {
	BPatch_reportError(BPatchSerious, 109,
	       "array reference to non-array type");
	return NULL;
    }
    BPatch_type *elementType = arrayType->getConstituentType();

    assert(elementType);
    int elementSize = elementType->getSize();

    // check that the type of the right operand is an integer.
    BPatch_type *indexType = const_cast<BPatch_type *>(rOperand.ast->getType());
    if (!indexType || strcmp(indexType->getName(), "int")) {
	// XXX - Should really check if this is a short/long too
	BPatch_reportError(BPatchSerious, 109,
			   "array index is not of type int");
	return NULL;
    }

    //
    // Convert a[i] into *(&a + (* i sizeof(element)))
    //
    AstNode *elementExpr = new AstNode(AstNode::Constant, (void *) elementSize);
    AstNode *offsetExpr = new AstNode(timesOp, elementExpr, rOperand.ast);
    AstNode *addrExpr = new AstNode(plusOp, 
	new AstNode(getAddrOp, lOperand.ast), offsetExpr);
    ast = new AstNode(AstNode::DataIndir, addrExpr);
    ast->setType(elementType);

    return ast;
}

/*
 * BPatch_arithExpr::BPatch_arithExpr
 *
 * Construct a snippet representing a binary arithmetic operation.
 *
 * op           The desired operation.
 * lOperand     The left operand for the operation.
 * rOperand     The right operand.
 */
BPatch_arithExpr::BPatch_arithExpr(BPatch_binOp op,
        const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)
{
    assert(BPatch::bpatch != NULL);

    opCode astOp;
    switch(op) {
      case BPatch_assign:
        astOp = storeOp;
        break;
      case BPatch_plus:
        astOp = plusOp;
        break;
      case BPatch_minus:
        astOp = minusOp;
        break;
      case BPatch_divide:
        astOp = divOp;
        break;
      case BPatch_times:
        astOp = timesOp;
        break;
      case BPatch_mod:
        /* XXX Not yet implemented. */
        assert(0);
        break;
      case BPatch_ref:
	ast = generateArrayRef(lOperand, rOperand);
	return;

	break;

      case BPatch_seq:
        ast = new AstNode(lOperand.ast, rOperand.ast);
        ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
        return;
      default:
        /* XXX handle error */
        assert(0);
    };

#ifdef alpha_dec_osf4_0
    /* XXX
     * A special kludge for the Alpha: there's no hardware divide on the
     * Alpha, and our code in inst-alpha.C to call a software divide doesn't
     * work right (yet?).  So, we never generate a divOp AstNode; instead we
     * generate a callNode AST that calls a divide function.
     */
    if (astOp == divOp) {
        vector<AstNode *> args;

        args += assignAst(lOperand.ast);
        args += assignAst(rOperand.ast);

        ast = new AstNode("divide", args);

        removeAst(args[0]);
        removeAst(args[1]);
    } else {
        ast = new AstNode(astOp, lOperand.ast, rOperand.ast);
    }
#else
    ast = new AstNode(astOp, lOperand.ast, rOperand.ast);
#endif

    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_arithExpr::BPatch_arithExpr
 *
 * Construct a snippet representing a unary arithmetic operation.
 *
 * op           The desired operation.
 * lOperand     The left operand for the operation.
 */
BPatch_arithExpr::BPatch_arithExpr(BPatch_unOp op, 
    const BPatch_snippet &lOperand)
{
    assert(BPatch::bpatch != NULL);

    switch(op) {
      case BPatch_negate: {
	AstNode *negOne = new AstNode(AstNode::Constant, (void *) -1);
	BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");
	assert(type != NULL);

	negOne->setType(type);
        ast = new AstNode(timesOp, negOne, lOperand.ast);
        break;
      }

      case BPatch_addr:  {
	ast = new AstNode(getAddrOp, lOperand.ast);
	// create a new type which is a pointer to type 
	BPatch_type *baseType = const_cast<BPatch_type *> 
	    (lOperand.ast->getType());
	BPatch_type *type = BPatch::bpatch->createPointer("<PTR>", 
	    baseType, sizeof(void *));
	assert(type);
	ast->setType(type);
        break;
      }

      case BPatch_deref: {
	ast = new AstNode(AstNode::DataIndir, lOperand.ast);
	BPatch_type *type = const_cast<BPatch_type *> ((lOperand.ast)->getType());
	if (!type || (type->getDataClass() != BPatch_pointer)) {
	    ast->setType(BPatch::bpatch->stdTypes->findType("int"));
	} else {
	    ast->setType(type->getConstituentType());
	}
	break;
      }

      default:
        /* XXX handle error */
        assert(0);
    };

    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

/*
 * BPatch_boolExpr::BPatch_boolExpr
 *
 * Constructs a snippet representing a boolean expression.
 *
 * op           The operator for the boolean expression.
 * lOperand     The left operand.
 * rOperand     The right operand.
 */
BPatch_boolExpr::BPatch_boolExpr(BPatch_relOp op,
                                 const BPatch_snippet &lOperand,
                                 const BPatch_snippet &rOperand)
{
    opCode astOp;
    switch(op) {
      case BPatch_lt:
        astOp = lessOp;
        break;
      case BPatch_eq:
        astOp = eqOp;
        break;
      case BPatch_gt:
        astOp = greaterOp;
        break;
      case BPatch_le:
        astOp = leOp;
        break;
      case BPatch_ne:
        astOp = neOp;
        break;
      case BPatch_ge:
        astOp = geOp;
        break;
      case BPatch_and:
        astOp = andOp;
        break;
      case BPatch_or:
        astOp = orOp;
        break;
      default:
        /* XXX Handle the error case here */
        assert( 0 );
    };
    
    ast = new AstNode(astOp, lOperand.ast, rOperand.ast);
    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant integer value.
 *
 * value        The desired value.
 */
BPatch_constExpr::BPatch_constExpr(int value)
{
    ast = new AstNode(AstNode::Constant, (void *)value);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");

    assert(type != NULL);

    ast->setType(type);
}


/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant string value.
 *
 * value        The desired constant string.
 */
BPatch_constExpr::BPatch_constExpr(const char *value)
{
    ast = new AstNode(AstNode::ConstantString, (void*)const_cast<char*>(value));

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("char *");
    assert(type != NULL);

    ast->setType(type);
}


/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant pointer.
 *
 * value        The desired constant pointer.
 */
BPatch_constExpr::BPatch_constExpr(const void *value)
{
    ast = new AstNode(AstNode::Constant, (void*)const_cast<void*>(value));

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("void *");
    assert(type != NULL);

    ast->setType(type);
}


/*
 * BPatch_funcCallExpr::BPatch_funcCallExpr
 *
 * Constructs a snippet representing a function call.
 *
 * func         Identifies the function to call.
 * args         A vector of the arguments to be passed to the function.
 */
BPatch_funcCallExpr::BPatch_funcCallExpr(
    const BPatch_function &func,
    const BPatch_Vector<BPatch_snippet *> &args)
{
    vector<AstNode *> ast_args;

    int i;
    for (i = 0; i < args.size(); i++)
        ast_args += assignAst(args[i]->ast);

    ast = new AstNode(func.func->prettyName(), ast_args);

    for (i = 0; i < args.size(); i++)
        removeAst(ast_args[i]);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

/*
 * BPatch_funcJumpExpr::BPatch_funcJumpExpr
 *
 * Constructs a snippet representing a jump to a function without
 * linkage.
 *
 * func Identifies the function to jump to.  */
BPatch_funcJumpExpr::BPatch_funcJumpExpr(
    const BPatch_function &func)
{
#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
    ast = new AstNode(func.func);
    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
#else
    BPatch_reportError(BPatchSerious, 109,
                       "BPatch_funcJumpExpr is not implemented on this platform");
#endif
}


/*
 * BPatch_ifExpr::BPatch_ifExpr
 *
 * Constructs a snippet representing a conditional expression.
 *
 * conditional          The conditional.
 * tClause              A snippet to execute if the conditional is true.
 */
BPatch_ifExpr::BPatch_ifExpr(const BPatch_boolExpr &conditional,
                             const BPatch_snippet &tClause)
{
    ast = new AstNode(ifOp, conditional.ast, tClause.ast);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_ifExpr::BPatch_ifExpr
 *
 * Constructs a snippet representing a conditional expression with an else
 * clause.
 *
 * conditional          The conditional.
 * tClause              A snippet to execute if the conditional is true.
 */
BPatch_ifExpr::BPatch_ifExpr(const BPatch_boolExpr &conditional,
                             const BPatch_snippet &tClause,
                             const BPatch_snippet &fClause)
{
    ast = new AstNode(ifOp, conditional.ast, tClause.ast, fClause.ast);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_nullExpr::BPatch_nullExpr
 *
 * Construct a null snippet that can be used as a placeholder.
 */
BPatch_nullExpr::BPatch_nullExpr()
{
    ast = new AstNode;

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_paramExpr::BPatch_paramExpr
 *
 * Construct a snippet representing a parameter of the function in which
 * the snippet is inserted.
 *
 * n    The position of the parameter (0 is the first parameter, 1 the second,
 *      and so on).
 */
BPatch_paramExpr::BPatch_paramExpr(int n)
{
    ast = new AstNode(AstNode::Param, (void *)n);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_retExpr::BPatch_retExpr
 *
 * Construct a snippet representing a return value from the function in which
 * the snippet is inserted.
 *
 */
BPatch_retExpr::BPatch_retExpr()
{
    ast = new AstNode(AstNode::ReturnVal, (void *)0);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

/*
 * BPatch_sequence::BPatch_sequence
 *
 * Construct a snippet representing a sequence of snippets.
 *
 * items        The snippets that are to make up the sequence.
 */
BPatch_sequence::BPatch_sequence(const BPatch_Vector<BPatch_snippet *> &items)
{
    if (items.size() == 0) {
        // XXX do something to indicate an error
        return;
    }

    assert(BPatch::bpatch != NULL);

    ast = new AstNode(items[0]->ast);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    for (int i = 1; i < items.size(); i++) {
        AstNode *tempAst = new AstNode(ast, items[i]->ast);
        tempAst->setTypeChecking(BPatch::bpatch->isTypeChecked());
        removeAst(ast);
        ast = tempAst;
    }
}


/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type at the given
 * address.
 *
 * in_process	The process that the variable resides in.
 * in_address	The address of the variable in the inferior's address space.
 * type		The type of the variable.
 */
BPatch_variableExpr::BPatch_variableExpr(char *in_name,
					 process *in_process,
					 void *in_address,
					 const BPatch_type *type) :
    name(in_name), proc(in_process), address(in_address), scope(NULL)
{
    ast = new AstNode(AstNode::DataAddr, address);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    ast->setType(type);

    size = type->getSize();
}

/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type and the passed
 *   ast.
 *
 * in_process   The process that the variable resides in.
 * in_address   The address of the variable in the inferior's address space.
 * type         The type of the variable.
 * ast          The ast expression for the variable
 */
BPatch_variableExpr::BPatch_variableExpr(char *in_name,
                                         process *in_process,
                                         AstNode *_ast,
                                         const BPatch_type *type) :
    name(in_name), proc(in_process), address(NULL), scope(NULL)
{
    ast = _ast;

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    ast->setType(type);

    size = type->getSize();
}


/*
 * BPatch_variableExpr::getType
 *
 *    Return the variable's type
 *
*/
const BPatch_type *BPatch_variableExpr::getType()
{
    return (ast->getType());
}

/*
 * BPatch_variableExpr::setType
 *
 *    Return the variable's type
 *
*/
void BPatch_variableExpr::setType(BPatch_type *newType)
{
    size = newType->getSize();
    ast->setType(newType);
}

/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type at the given
 * address.
 *
 * in_process	The process that the variable resides in.
 * in_address	The address of the variable in the inferior's address space.
 * type		The type of the variable.
 * frameRelative	Is the address a frame offset, not absolute address
 *
 */
BPatch_variableExpr::BPatch_variableExpr(process *in_process,
                                         void *in_address,
                                         const BPatch_type *type,
                                         bool frameRelative,
					 BPatch_point *scp) :
    proc(in_process), address(in_address)
{
    if (frameRelative) {
        ast = new AstNode(AstNode::FrameAddr, address);
    } else {
        ast = new AstNode(AstNode::DataAddr, address);
    }

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    size = type->getSize();
    ast->setType(type);

    scope = scp;
}

/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing an untyped variable of a given size at the
 * given address.
 *
 * in_address   The address of the variable in the inferior's address space.
 */
BPatch_variableExpr::BPatch_variableExpr(process *in_process,
                                         void *in_address,
                                         int in_size) :
    proc(in_process), address(in_address), scope(NULL)
{
    ast = new AstNode(AstNode::DataAddr, address);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    ast->setType(BPatch::bpatch->type_Untyped);

    size = in_size;
}


/*
 * BPatch_variableExpr::readValue
 *
 * Read the value of a variable in a thread's address space.
 *
 * dst          A pointer to a buffer in which to place the value of the
 *              variable.  It is assumed to be the same size as the variable.
 */
bool BPatch_variableExpr::readValue(void *dst)
{
    if (size) {
	proc->readDataSpace(address, size, dst, true);
	return true;
    } else {
	return false;
    }
}


/*
 * BPatch_variableExpr::readValue
 *
 * Read the a given number of bytes starting at the base address of a variable
 * in the a thread's address space.
 *
 * dst          A pointer to a buffer in which to place the value of the
 *              variable.  It is assumed to be the same size as the variable.
 * len          Number of bytes to read.
 */
void BPatch_variableExpr::readValue(void *dst, int len)
{
    proc->readDataSpace(address, len, dst, true);
}


/*
 * BPatch_variableExpr::writeValue
 *
 * Write a value into a variable in a thread's address space.
 *
 * dst          A pointer to a buffer in which to place the value of the
 *              variable.  It is assumed to be the same size as the variable.
 *
 * returns false if the type info isn't available (i.e. we don't know the size)
 */
bool BPatch_variableExpr::writeValue(const void *src)
{
    if (size) {
	proc->writeDataSpace(address, size, src);
	return true;
    } else {
	return false;
    }
}


/*
 * BPatch_variableExpr::writeValue
 *
 * Write the a given number of bytes starting at the base address of a
 * variable in the a thread's address space.
 *
 * dst          A pointer to a buffer in which to place the value of the
 *              variable.  It is assumed to be the same size as the variable.
 */
void BPatch_variableExpr::writeValue(const void *src, int len)
{
    proc->writeDataSpace(address, len, src);
}

/*
 * getComponents() - return variable expressions for all of the fields
 *     in the passed structure/union.
 */
BPatch_Vector<BPatch_variableExpr *> *BPatch_variableExpr::getComponents()
{
    BPatch_type *type;
    BPatch_Vector<BPatch_field *> *fields;
    BPatch_Vector<BPatch_variableExpr *> *retList;

    type = const_cast<BPatch_type *>(getType());
    if ((type->getDataClass() != BPatch_structure) && (type->getDataClass() != BPatch_union)) {
	return NULL;
    }

    retList = new BPatch_Vector<BPatch_variableExpr *>;
    assert(retList);

    fields = type->getComponents();
    for (int i=0; i < fields->size(); i++) {

	BPatch_field *field = (*fields)[i];
	int offset = (field->offset / 8);

	BPatch_variableExpr *newVar;

	// convert to *(&basrVar + offset)
	AstNode *offsetExpr = new AstNode(AstNode::Constant, (void *) offset);
        AstNode *addrExpr = new AstNode(plusOp,
		new AstNode(getAddrOp, ast), offsetExpr);
	AstNode *fieldExpr = new AstNode(AstNode::DataIndir, addrExpr);

	newVar = new BPatch_variableExpr(const_cast<char *> (field->getName()),
	    proc, fieldExpr, const_cast<BPatch_type *>(field->type));
	retList->push_back(newVar);
    }

    return retList;
}

/*
 * BPatch_breakPointExpr::BPatch_breakPointExpr
 *
 * Construct a snippet representing a breakpoint.
 *
 */
BPatch_breakPointExpr::BPatch_breakPointExpr()
{
    vector<AstNode *> null_args;

    ast = new AstNode("DYNINSTbreakPoint", null_args);

    assert(BPatch::bpatch != NULL);

    ast->setType(BPatch::bpatch->type_Untyped);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}
