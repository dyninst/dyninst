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

#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "perfStream.h"

#include "BPatch.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"


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
    return (double)ast->cost() / cyclesPerSecond;
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


/*
 * BPatch_arithExpr::BPatch_arithExpr
 *
 * Construct a snippet representing a binary arithmetic operation.
 *
 * op		The desired operation.
 * lOperand	The left operand for the operation.
 * rOperand	The right operand.
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
	/* XXX Not yet implemented. */
	assert(0);
	break;
      case BPatch_seq:
	ast = new AstNode(lOperand.ast, rOperand.ast);
	ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
	return;
      default:
	/* XXX handle error */
	assert(0);
    };

    ast = new AstNode(astOp, lOperand.ast, rOperand.ast);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_boolExpr::BPatch_boolExpr
 *
 * Constructs a snippet representing a boolean expression.
 *
 * op		The operator for the boolean expression.
 * lOperand	The left operand.
 * rOperand	The right operand.
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
 * value	The desired value.
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
 * value	The desired constant string.
 */
BPatch_constExpr::BPatch_constExpr(const char *value)
{
    ast = new AstNode(AstNode::ConstantString, (void *)value);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("char *");
    assert(type != NULL);

    ast->setType(type);
}


/*
 * BPatch_funcCallExpr::BPatch_funcCallExpr
 *
 * Constructs a snippet representing a function call.
 *
 * func		Identifies the function to call.
 * args		A vector of the arguments to be passed to the function.
 */
BPatch_funcCallExpr::BPatch_funcCallExpr(
    const BPatch_function &func,
    const BPatch_Vector<BPatch_snippet *> &args)
{
    vector<AstNode *> ast_args;

    for (int i = 0; i < args.size(); i++)
	ast_args += assignAst(args[i]->ast);

    ast = new AstNode(func.func->prettyName(), ast_args);

    for (int i = 0; i < args.size(); i++)
	removeAst(ast_args[i]);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_ifExpr::BPatch_ifExpr
 *
 * Constructs a snippet representing a conditional expression.
 *
 * conditional		The conditional.
 * tClause		A snippet to execute if the conditional is true.
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
 * conditional		The conditional.
 * tClause		A snippet to execute if the conditional is true.
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
 * n	The position of the parameter (0 is the first parameter, 1 the second,
 * 	and so on).
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
 * items	The snippets that are to make up the sequence.
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
 * Construct a snippet representing a variable at the given address.
 *
 * in_address	The address of the variable in the inferior's address space.
 */
BPatch_variableExpr::BPatch_variableExpr(void *in_address,
					 const BPatch_type *type) :
    address(in_address)
{
    ast = new AstNode(AstNode::DataAddr, address);

    assert(BPatch::bpatch != NULL);
    ast->setTypeChecking(BPatch::bpatch->isTypeChecked());

    ast->setType(type);
}

/**************************************************************************
 * BPatch_function
 *************************************************************************/

/*
 * BPatch_function::getName
 *
 * Copies the name of the function into a buffer, up to a given maximum
 * length.  Returns a pointer to the beginning of the buffer that was
 * passed in.
 *
 * s		The buffer into which the name will be copied.
 * len		The size of the buffer.
 */
char *BPatch_function::getName(char *s, int len)
{
    assert(func);
    string name = func->prettyName();
    strncpy(s, name.string_of(), len);

    return s;
}
