/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: BPatch_snippet.C,v 1.100 2007/10/26 17:17:46 bernat Exp $

#define BPATCH_FILE

#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "process.h"
#include "instPoint.h"

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "BPatch_function.h"
#include "BPatch_typePrivate.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "common/h/Time.h"
#include "common/h/timing.h"

#include "mapped_object.h" // for savetheworld

// Need REG_MT_POS, defined in inst-<arch>...

#if defined(arch_x86) || defined(arch_86_64)
#include "inst-x86.h"
#elif defined(arch_power)
#include "inst-power.h"
#elif defined(arch_ia64)
#include "inst-ia64.h"
#elif defined(arch_sparc)
#include "inst-sparc.h"
#endif


//  This will be removed:
int BPatch_snippet::PDSEP_astMinCost()
{
  return (*ast_wrapper)->minCost();
}

/*
 * BPatch_snippet::BPatch_snippet
 *
 * The default constructor was exposed as public, so we're
 * stuck with it even though that _should_ be an error.
 * For now, make it a null node (and hope nobody ever
 * tries to generate code)
 */
BPatch_snippet::BPatch_snippet() {
    ast_wrapper = new AstNodePtr(AstNode::nullNode());
}


/*
 * BPatch_snippet::BPatch_snippet
 *
 * Copy constructor for BPatch_snippet.
 */
void BPatch_snippet::BPatch_snippetInt(const BPatch_snippet &src)
{
    if (src.ast_wrapper) {
        ast_wrapper = new AstNodePtr(*(src.ast_wrapper));
    }
    else
        ast_wrapper = NULL;
}


/*
 * BPatch_snippet::operator=
 *
 * Assignment operator for BPatch_snippet.  Needed to ensure that the
 * reference counts for the asts contained in the snippets is correct.
 */
BPatch_snippet &BPatch_snippet::operator_equals(const BPatch_snippet &src)
{
    // Check for x = x
    if (&src == this)
        return *this;

    // We'll now contain another reference to the ast in the other snippet
    if (ast_wrapper == NULL) ast_wrapper = new AstNodePtr();

    (*ast_wrapper) = (*src.ast_wrapper);

    return *this;
}


/*
 * BPatch_snippet:getCost
 *
 * Returns the estimated cost of executing the snippet, in seconds.
 */
float BPatch_snippet::getCostInt()
{
  // Currently represents the maximum possible cost of the snippet.  For
  // instance, for the statement "if(cond) <stmtA> ..."  the cost of <stmtA>
  // is currently included, even if it's actually not called.  Feel free to
  // change the maxCost call below to ast->minCost or ast->avgCost if the
  // semantics need to be changed.
  timeLength costv = timeLength((*ast_wrapper)->maxCost(), getCyclesPerSecond());
  float retCost = static_cast<float>(costv.getD(timeUnit::sec()));
  return retCost;
}
/*
 * BPatch_snippet:getCostAtPoint
 *
 * Returns the estimated cost of executing the snippet at the provided point, in seconds.
 */
float BPatch_snippet::getCostAtPointInt(BPatch_point *pt)
{
  // Currently represents the maximum possible cost of the snippet.  For
  // instance, for the statement "if(cond) <stmtA> ..."  the cost of <stmtA>
  // is currently included, even if it's actually not called.  Feel free to
  // change the maxCost call below to ast->minCost or ast->avgCost if the
  // semantics need to be changed.
    if (!pt) return 0.0;
    if (!pt->point) return 0.0;

    int unitCostInCycles = (*ast_wrapper)->maxCost()
                           + pt->point->getPointCost() 
                           + getInsnCost(trampPreamble) 
                           + getInsnCost(trampTrailer);

    timeLength unitCost(unitCostInCycles, getCyclesPerSecond());
    float frequency = getPointFrequency(pt->point);
    timeLength value = unitCost * frequency;

  float retCost = static_cast<float>(value.getD(timeUnit::sec()));
  return retCost;
}

bool BPatch_snippet::is_trivialInt()
{
  return (ast_wrapper == NULL);
}


/*
 * BPatch_snippet::~BPatch_snippet
 *
 * Destructor for BPatch_snippet.  Deallocates memory allocated by the
 * snippet.  Well, decrements a reference count.
 */
void BPatch_snippet::BPatch_snippet_dtor()
{
    if (ast_wrapper) delete ast_wrapper;
}


//
// generateArrayRef - Construct an Ast expression for an array.
//
AstNodePtr *generateArrayRef(const BPatch_snippet &lOperand, 
                            const BPatch_snippet &rOperand)
{
    if (*(lOperand.ast_wrapper) == AstNodePtr()) return new AstNodePtr();
    if (*(rOperand.ast_wrapper) == AstNodePtr()) return new AstNodePtr();

    //  We have to be a little forgiving of the
    const BPatch_typeArray *arrayType = dynamic_cast<const BPatch_typeArray *>((*(lOperand.ast_wrapper))->getType());
    if (!arrayType) {
        if ((*(lOperand.ast_wrapper))->getType() == NULL) 
            BPatch_reportError(BPatchSerious, 109,
                               "array reference has no type information");
        else {
            fprintf(stderr, "%s[%d]:  error here: type is %s\n", FILE__, __LINE__,(*(lOperand.ast_wrapper))->getType()->getName());
            BPatch_reportError(BPatchSerious, 109,
                               "array reference has array reference to non-array type");
        }
        return new AstNodePtr();
    }

    BPatch_type *elementType = arrayType->getConstituentType();

    assert(elementType);
    long int elementSize = elementType->getSize();

    // check that the type of the right operand is an integer.
    //  We have to be a little forgiving of this parameter, since we could 
    //  be indexing using a funcCall snippet, for which no return type is available.
    //  Ideally we could always know this information, but until then, if no
    //  type information is available, assume that the user knows what they're doing
    //  (just print a warning, don't fail).

    BPatch_type *indexType = const_cast<BPatch_type *>((*(rOperand.ast_wrapper))->getType());
    if (!indexType) {
        char err_buf[512];
        sprintf(err_buf, "%s[%d]:  %s %s\n",
		__FILE__, __LINE__,
		"Warning:  cannot ascertain type of index parameter is of integral type, ",
		"This is not a failure... but be warned that type-checking has failed. ");
        BPatch_reportError(BPatchWarning, 109, err_buf);
      
    }
    else if (strcmp(indexType->getName(), "int")
        && strcmp(indexType->getName(), "short")
        && strcmp(indexType->getName(), "long")
        && strcmp(indexType->getName(), "unsigned int")
        && strcmp(indexType->getName(), "unsigned short")
        && strcmp(indexType->getName(), "unsigned long")
        && strcmp(indexType->getName(), "unsigned")) {
        char err_buf[256];
        sprintf(err_buf, "%s[%d]: non-integer array index type %s\n",
                __FILE__, __LINE__,  indexType->getName());
        fprintf(stderr, "%s\n", err_buf);
	BPatch_reportError(BPatchSerious, 109, err_buf);
        return new AstNodePtr();
    }
    //fprintf(stderr, "%s[%d]:  indexing with type %s\n", __FILE__, __LINE__, 
    //        indexType->getName());

    //
    // Convert a[i] into *(&a + (* i sizeof(element)))
    //

    AstNodePtr ast = AstNode::operandNode(AstNode::DataIndir,
                               AstNode::operatorNode(plusOp,
                                                     AstNode::operatorNode(getAddrOp,
                                                                           *(lOperand.ast_wrapper)),
                                                     AstNode::operatorNode(timesOp,
                                                                           AstNode::operandNode(AstNode::Constant,
                                                                                                (void *)elementSize),
                                                                           *(rOperand.ast_wrapper))));
    ast->setType(elementType);

    return new AstNodePtr(ast);
}


//
// generateFieldRef - Construct an Ast expression for an structure field.
//
AstNodePtr *generateFieldRef(const BPatch_snippet &lOperand, 
                                   const BPatch_snippet &rOperand)
{
    if (*(lOperand.ast_wrapper) == AstNodePtr()) return new AstNodePtr();
    if (*(rOperand.ast_wrapper) == AstNodePtr()) return new AstNodePtr();

    const BPatch_typeStruct *structType = dynamic_cast<const BPatch_typeStruct *>((*(lOperand.ast_wrapper))->getType());
    if (!structType) {
	BPatch_reportError(BPatchSerious, 109,
	       "structure reference has no type information, or structure reference to non-structure type");
        return new AstNodePtr();
    }

    // check that the type of the right operand is a string.
    BPatch_type *fieldType = const_cast<BPatch_type *>((*(rOperand.ast_wrapper))->getType());
    if ((*(rOperand.ast_wrapper))->getoType()!=AstNode::ConstantString 
	|| !fieldType
	|| strcmp(fieldType->getName(), "char *")) {
	// XXX - Should really check if this is a short/long too
	BPatch_reportError(BPatchSerious, 109,
			   "field name is not of type char *");
        return new AstNodePtr();
    }

    const BPatch_Vector<BPatch_field *> *fields;
    BPatch_field *field = NULL;

    // check that the name of the right operand is a field of the left operand
    fields = structType->getComponents();

    unsigned int i;

    for (i=0; i < fields->size(); i++) {
      field = (*fields)[i];
      if (!strcmp(field->getName(), (const char *) (*(rOperand.ast_wrapper))->getOValue()))
	break;
    }
    if (i==fields->size()) {
      BPatch_reportError(BPatchSerious, 109,
			 "field name not found in structure");
      return new AstNodePtr();
    }

    if (! field ) return new AstNodePtr();
    long int offset = (field->getOffset() / 8);

    //
    // Convert s.f into *(&s + offset(s,f))
    //
    AstNodePtr ast = AstNode::operandNode(AstNode::DataIndir,
                               AstNode::operatorNode(plusOp,
                                                     AstNode::operatorNode(getAddrOp, *(lOperand.ast_wrapper)),
                                                     AstNode::operandNode(AstNode::Constant,
                                                                          (void *)offset)));
    ast->setType(field->getType());

    return new AstNodePtr(ast);
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
void BPatch_arithExpr::BPatch_arithExprBin(BPatch_binOp op,
        const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)
{
#if 0
    fprintf(stderr, "%s[%d]:  welcome to BPatch_arithExprBin: types (l,r):", FILE__, __LINE__);
    fprintf(stderr, " %s, %s\n",*(lOperand.ast_wrapper)->getType() ? *(lOperand.ast_wrapper)->getType()->getName() : "<no type>",  *(rOperand.ast_wrapper)->getType() ? *(rOperand.ast_wrapper)->getType()->getName() : "<no type>");
#endif
    assert(BPatch::bpatch != NULL);

    opCode astOp = undefOp; // Quiet the compiler
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
          if (ast_wrapper) delete ast_wrapper;
          ast_wrapper = generateArrayRef(lOperand, rOperand);
          if ((*ast_wrapper) == NULL) {
              BPatch_reportError(BPatchSerious, 100 /* what # to use? */,
                                 "could not generate array reference.");
              BPatch_reportError(BPatchSerious, 100,
                                 "resulting snippet is invalid.");
          }
          return;         
          break;
      case BPatch_fieldref:
          if (ast_wrapper) delete ast_wrapper;
          ast_wrapper = generateFieldRef(lOperand, rOperand);
          if ((*ast_wrapper) == NULL) {
              BPatch_reportError(BPatchSerious, 100 /* what # to use? */,
                                 "could not generate field reference.");
              BPatch_reportError(BPatchSerious, 100,
                                 "resulting snippet is invalid.");
          }
          return;
          break;
      case BPatch_seq:
          {
              if (ast_wrapper) delete ast_wrapper;
              ast_wrapper = new AstNodePtr;
              pdvector<AstNodePtr > sequence;
              sequence.push_back(*(lOperand.ast_wrapper));
              sequence.push_back(*(rOperand.ast_wrapper));
              
              (*ast_wrapper) = AstNode::sequenceNode(sequence);
              (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
              return;
          }
    default:
        /* XXX handle error */
        assert(0);
    };

    ast_wrapper = new AstNodePtr(AstNode::operatorNode(astOp,
                                                              *(lOperand.ast_wrapper),
                                                              *(rOperand.ast_wrapper)));

    (*ast_wrapper)->setType((*(lOperand.ast_wrapper))->getType());
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_arithExpr::BPatch_arithExpr
 *
 * Construct a snippet representing a unary arithmetic operation.
 *
 * op           The desired operation.
 * lOperand     The left operand for the operation.
 */
void BPatch_arithExpr::BPatch_arithExprUn(BPatch_unOp op, 
    const BPatch_snippet &lOperand)
{
   assert(BPatch::bpatch != NULL);
   
   switch(op) {
      case BPatch_negate: {
          AstNodePtr negOne = AstNode::operandNode(AstNode::Constant, 
                                                 (void *)-1);
          BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");
          assert(type != NULL);         
          negOne->setType(type);
          ast_wrapper = new AstNodePtr(AstNode::operatorNode(timesOp,
                                                                    negOne,
                                                                    *(lOperand.ast_wrapper)));
          break;
      }         
   case BPatch_addr:  {
       ast_wrapper = new AstNodePtr(AstNode::operatorNode(getAddrOp, *(lOperand.ast_wrapper)));
       // create a new type which is a pointer to type 
       BPatch_type *baseType = const_cast<BPatch_type *> 
           ((*(lOperand.ast_wrapper))->getType());
       BPatch_type *type = BPatch::bpatch->createPointer("<PTR>", 
                                                         baseType, sizeof(void *));
       assert(type);
       (*ast_wrapper)->setType(type);
       break;
   }

   case BPatch_deref: {
#if 0
       // Handle constant addresses...
          if (*(lOperand.ast_wrapper)->getoType() == AstNode::Constant) {
              ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataAddr,
                                                                       const_cast<void *>(*(lOperand.ast_wrapper)->getOValue())));
          }
          else {
              ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataIndir, *(lOperand.ast_wrapper)));
          }
#endif
          ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataIndir, *(lOperand.ast_wrapper)));

          BPatch_type *type = const_cast<BPatch_type *> ((*(lOperand.ast_wrapper))->getType());
          if (!type || (type->getDataClass() != BPatch_dataPointer)) {
              (*ast_wrapper)->setType(BPatch::bpatch->stdTypes->findType("int"));
          } else {
              (*ast_wrapper)->setType(dynamic_cast<BPatch_typePointer *>(type)->getConstituentType());
          }
          break;
      }
         
      default:
         /* XXX handle error */
         assert(0);
   };
   
   (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
void BPatch_boolExpr::BPatch_boolExprInt(BPatch_relOp op,
                                         const BPatch_snippet &lOperand,
                                         const BPatch_snippet &rOperand)
{
    opCode astOp = undefOp;
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
    
    ast_wrapper = new AstNodePtr(AstNode::operatorNode(astOp, *(lOperand.ast_wrapper), *(rOperand.ast_wrapper)));
    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant integer value.
 *
 * value        The desired value.
 */
 
void BPatch_constExpr::BPatch_constExprSignedInt( signed int value ) {
	assert( BPatch::bpatch != NULL );

        ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant,
                                                                 (void *)(unsigned long) value));
	(*ast_wrapper)->setTypeChecking( BPatch::bpatch->isTypeChecked() );
	
	BPatch_type * type = BPatch::bpatch->stdTypes->findType( "int" );
	assert( type != NULL );
	(*ast_wrapper)->setType( type );
}

void BPatch_constExpr::BPatch_constExprUnsignedInt( unsigned int value ) {
	assert( BPatch::bpatch != NULL );
	
        ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant,
                                                                 (void *)(unsigned long) value));
	(*ast_wrapper)->setTypeChecking( BPatch::bpatch->isTypeChecked() );
	
	BPatch_type * type = BPatch::bpatch->stdTypes->findType( "unsigned int" );
	assert( type != NULL );
	(*ast_wrapper)->setType( type );
	}

void BPatch_constExpr::BPatch_constExprSignedLong( signed long value ) {
	assert( BPatch::bpatch != NULL );
	
        ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant,
                                                                 (void *)(unsigned long) value));
	(*ast_wrapper)->setTypeChecking( BPatch::bpatch->isTypeChecked() );
	
	BPatch_type * type = BPatch::bpatch->stdTypes->findType( "long" );
	assert( type != NULL );
	(*ast_wrapper)->setType( type );
	}

void BPatch_constExpr::BPatch_constExprUnsignedLong( unsigned long value ) {
	assert( BPatch::bpatch != NULL );
	
        ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant,
                                                                 (void *)(unsigned long) value));
	(*ast_wrapper)->setTypeChecking( BPatch::bpatch->isTypeChecked() );
	
	BPatch_type * type = BPatch::bpatch->stdTypes->findType( "unsigned long" );
	assert( type != NULL );
	(*ast_wrapper)->setType( type );
	}

/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant string value.
 *
 * value        The desired constant string.
 */
void BPatch_constExpr::BPatch_constExprCharStar(const char *value)
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::ConstantString, (void *)const_cast<char *>(value)));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("char *");
    assert(type != NULL);

    (*ast_wrapper)->setType(type);
}


/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant pointer.
 *
 * value        The desired constant pointer.
 */
void BPatch_constExpr::BPatch_constExprVoidStar(const void *value)
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant, (void *)const_cast<void *>(value)));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("void *");
    assert(type != NULL);

    (*ast_wrapper)->setType(type);
}

void BPatch_constExpr::BPatch_constExprLongLong(long long value) 
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant, (void *)value));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
    
    BPatch_type* type = BPatch::bpatch->stdTypes->findType("long long");

    assert(type != NULL);
    (*ast_wrapper)->setType(type);
}

#ifdef IBM_BPATCH_COMPAT

char *BPatch_variableExpr::getNameWithLength(char *buffer, int max)
{
  if (max > strlen(name)) {
    strcpy (buffer, name);
    return buffer;
  } else {
    strncpy (buffer, name, max-1)[max-1]='\0';
  }
  return NULL;
}

void *BPatch_variableExpr::getAddressInt()
{
  //  for AIX this may be broken, in the case where the mutator is 32b
  //  and the mutatee is 64b. 
  return address;
}


void BPatch_constExpr::BPatch_constExprFloat(float value)
{
	// XXX fix me, puting value into int register.
	int ivalue = (int) value;
	BPatch_constExpr((int) ivalue);
}

#endif



/*
 * BPatch_regExpr::BPatch_regExpr
 *
 * Constructs a snippet representing a register.
 *
 * value        index of register.
 *
 * Note:  introduced for paradyn-seperation (paradyn needs access
 *        to register REG_MT_POS)  -- there are other ways to do this.
 *        This happens to be expedient -- not sure if we want to be
 *        really exposing this to API users.  Thus this class may be
 *        temporary -- avoid using it.
 */

void BPatch_regExpr::BPatch_regExprInt(unsigned int value)
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataReg, (void *)(unsigned long int) value));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");
    assert(type != NULL);

    (*ast_wrapper)->setType(type);
}

/*
 * BPatch_funcCallExpr::BPatch_funcCallExpr
 *
 * Constructs a snippet representing a function call.
 *
 * func         Identifies the function to call.
 * args         A vector of the arguments to be passed to the function.
 */
void BPatch_funcCallExpr::BPatch_funcCallExprInt(
    const BPatch_function &func,
    const BPatch_Vector<BPatch_snippet *> &args)
{
    pdvector<AstNodePtr> ast_args;

    unsigned int i;
    for (i = 0; i < args.size(); i++) {
        assert(args[i]->ast_wrapper);
        ast_args.push_back(*(args[i]->ast_wrapper));
    }

    //  jaw 08/03  part of cplusplus bugfix -- using pretyName
    //  to generate function calls can lead to non uniqueness probs
    //  in the case of overloaded callee functions.

    ast_wrapper = new AstNodePtr(AstNode::funcCallNode(func.lowlevel_func(), ast_args));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *ret_type = const_cast<BPatch_function &>(func).getReturnType();
      (*ast_wrapper)->setType(ret_type);
	/*** ccw 24 jul 2003 ***/
	/* 	at this point, if saveworld is turned on, check
		to see if func is in a shared lib. if it
		is marked that shared lib as dirtyCalled()
	*/
#if defined(cap_save_the_world)

      AddressSpace *as = func.getAddSpace()->getAS();
      process* proc = dynamic_cast<process *>(as);

      //	process *proc = func.getProc()->llproc;
	
	// We can't define isSharedLib as constant everywhere, so strip
	// the const definition here.
	BPatch_function &stripFunc = const_cast<BPatch_function &> (func);
	if( proc->collectSaveWorldData && stripFunc.isSharedLib()){
            stripFunc.lowlevel_func()->obj()->setDirtyCalled();
	}
#endif
}

/*
 * BPatch_funcJumpExpr::BPatch_funcJumpExpr
 *
 * Constructs a snippet representing a jump to a function without
 * linkage.
 *
 * func Identifies the function to jump to.  */
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0) \
 || defined(os_linux)
void BPatch_funcJumpExpr::BPatch_funcJumpExprInt(
    const BPatch_function &func)
{
    ast_wrapper = new AstNodePtr(AstNode::funcReplacementNode(func.lowlevel_func()));
    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}
#else
void BPatch_funcJumpExpr::BPatch_funcJumpExprInt(
    const BPatch_function & /* func */)
{
    BPatch_reportError(BPatchSerious, 109,
                       "BPatch_funcJumpExpr is not implemented on this platform");
}
#endif


/*
 * BPatch_ifExpr::BPatch_ifExpr
 *
 * Constructs a snippet representing a conditional expression.
 *
 * conditional          The conditional.
 * tClause              A snippet to execute if the conditional is true.
 */
void BPatch_ifExpr::BPatch_ifExprInt(const BPatch_boolExpr &conditional,
                                     const BPatch_snippet &tClause)
{
    ast_wrapper = new AstNodePtr(AstNode::operatorNode(ifOp, *(conditional.ast_wrapper), *(tClause.ast_wrapper)));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
void BPatch_ifExpr::BPatch_ifExprWithElse(const BPatch_boolExpr &conditional,
                                          const BPatch_snippet &tClause,
                                          const BPatch_snippet &fClause)
{
    ast_wrapper = new AstNodePtr(AstNode::operatorNode(ifOp, *(conditional.ast_wrapper), *(tClause.ast_wrapper), *(fClause.ast_wrapper)));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_nullExpr::BPatch_nullExpr
 *
 * Construct a null snippet that can be used as a placeholder.
 */
void BPatch_nullExpr::BPatch_nullExprInt()
{
    ast_wrapper = new AstNodePtr(AstNode::nullNode());

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
void BPatch_paramExpr::BPatch_paramExprInt(int n)
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Param,
                                                             (void *)(long)n));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_retExpr::BPatch_retExpr
 *
 * Construct a snippet representing a return value from the function in which
 * the snippet is inserted.
 *
 */
void BPatch_retExpr::BPatch_retExprInt()
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::ReturnVal, (void *)0));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

/*
 * BPatch_sequence::BPatch_sequence
 *
 * Construct a snippet representing a sequence of snippets.
 *
 * items        The snippets that are to make up the sequence.
 */
void BPatch_sequence::BPatch_sequenceInt(const BPatch_Vector<BPatch_snippet *> &items)
{
    if (items.size() == 0) {
        // XXX do something to indicate an error
        return;
    }

    assert(BPatch::bpatch != NULL);

    pdvector<AstNodePtr >sequence;
    for (unsigned i = 0; i < items.size(); i++) {
        assert(items[i]->ast_wrapper);
        sequence.push_back(*(items[i]->ast_wrapper));
    }
    ast_wrapper = new AstNodePtr(AstNode::sequenceNode(sequence));

    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type at the given
 * address.
 *
 * in_process	The BPatch_process that the variable resides in.
 * in_address	The address of the variable in the inferior's address space.
 * type		The type of the variable.
 */
void BPatch_variableExpr::BPatch_variableExprInt(char *in_name,
						 BPatch_addressSpace * in_addSpace,
						 void *in_address,
						 BPatch_type *type) 
{
    name = in_name;
    appAddSpace = in_addSpace;
    address = in_address;
    scope = NULL;
    isLocal = false;

    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataAddr, address));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    (*ast_wrapper)->setType(type);

    size = type->getSize();
}

/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type and the passed
 *   ast.
 *
 * in_process   The BPatch_process that the variable resides in.
 * in_address   The address of the variable in the inferior's address space.
 * type         The type of the variable.
 * ast          The ast expression for the variable
 */
BPatch_variableExpr::BPatch_variableExpr(char *in_name,
                                         /*BPatch_process *in_process,*/
					 BPatch_addressSpace *in_addSpace,
                                         AstNodePtr *ast_wrapper_,
                                         BPatch_type *type,
                                         void* in_address) :
  name(in_name), /*appProcess(in_process),*/ appAddSpace(in_addSpace), address(in_address), scope(NULL), isLocal(false)
{
    ast_wrapper = ast_wrapper_;
    assert(ast_wrapper);

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    (*ast_wrapper)->setType(type);

    size = type->getSize();
}

BPatch_variableExpr::BPatch_variableExpr(char *in_name,
                                         //BPatch_process *in_process,
                                         BPatch_addressSpace *in_addSpace,
					 AstNodePtr *ast_wrapper_,
                                         BPatch_type *type) :
  name(in_name), /*appProcess(in_process),*/ appAddSpace(in_addSpace), address(NULL), scope(NULL), isLocal(false)
{
    
    ast_wrapper = ast_wrapper_;
    assert(ast_wrapper);

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    (*ast_wrapper)->setType(type);

    size = type->getSize();
}

unsigned int BPatch_variableExpr::getSizeInt() CONST_EXPORT
{
  return size;
}

/*
 * BPatch_variableExpr::getType
 *
 *    Return the variable's type
 *
*/
const BPatch_type *BPatch_variableExpr::getTypeInt()
{
    //return (const_cast<BPatch_type *>((*ast_wrapper)->getType()));
    return (*ast_wrapper)->getType();
}
#ifdef NOTDEF
const BPatch_type *BPatch_variableExpr::getTypeConst() CONST_EXPORT
{
    return (*ast_wrapper)->getType();
}
#endif

/*
 * BPatch_variableExpr::setType
 *
 *    Set the variable's type
 *
*/
bool BPatch_variableExpr::setTypeInt(BPatch_type *newType)
{
    size = newType->getSize();
    (*ast_wrapper)->setType(newType);
    return true;
}
/*
 * BPatch_variableExpr::seSize
 *
 *    Set the variable's size
 *
*/
bool BPatch_variableExpr::setSizeInt(int sz)
{
    size = sz;
    return true;
}


/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type at the given
 * address.
 *
 * in_addSpace	The BPatch_addressSpace that the variable resides in.
 * in_address	The address of the variable in the inferior's address space.
 * in_register	The register of the variable in the inferior's address space.
 * type		The type of the variable.
 * in_storage	Enum of how this variable is stored.
 *
 */
BPatch_variableExpr::BPatch_variableExpr(//BPatch_process *in_process,
                                         BPatch_addressSpace *in_addSpace,
					 void *in_address,
					 int in_register,
                                         BPatch_type *type,
                                         BPatch_storageClass in_storage,
					 BPatch_point *scp) :
  /*appProcess(in_process),*/ appAddSpace(in_addSpace), address(in_address)
{
    switch (in_storage) {
	case BPatch_storageAddr:
            ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataAddr, address));
	    isLocal = false;
	    break;
	case BPatch_storageAddrRef:
	    assert( 0 ); // Not implemented yet.
	    isLocal = false;
	    break;
	case BPatch_storageReg:
	    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::PreviousStackFrameDataReg, (void *)in_register));
	    isLocal = true;
	    break;
	case BPatch_storageRegRef:
	    assert( 0 ); // Not implemented yet.
	    isLocal = true;
	    break;
	case BPatch_storageRegOffset:
	    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::RegOffset, 
                                       AstNode::operandNode(AstNode::DataAddr,
                                                            address)));
	    (*ast_wrapper)->setOValue( (void *)(long int)in_register );
	    isLocal = true;
	    break;
	case BPatch_storageFrameOffset:
            ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::FrameAddr, address));
	    isLocal = true;
	    break;
    }

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    size = type->getSize();
    (*ast_wrapper)->setType(type);

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
BPatch_variableExpr::BPatch_variableExpr(/*BPatch_process *in_process,*/
					 BPatch_addressSpace *in_addSpace,
                                         void *in_address,
                                         int in_size) :
    appAddSpace(in_addSpace), address(in_address), scope(NULL), isLocal(false)
{
    ast_wrapper = new AstNodePtr(AstNode::operandNode(AstNode::DataAddr, address));

    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());

    (*ast_wrapper)->setType(BPatch::bpatch->type_Untyped);

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
bool BPatch_variableExpr::readValueInt(void *dst)
{
  if (isLocal) {
    char msg[2048];
    sprintf(msg, "variable %s is not a global variable, cannot read using readValue()",name);
    BPatch_reportError(BPatchWarning, 109,msg);
    return false;
  }

  if (size) {
      appAddSpace->getAS()->readDataSpace(address, size, dst, true);
      return true;
  }
  else {
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
bool BPatch_variableExpr::readValueWithLength(void *dst, int len)
{
  if (isLocal) {
    char msg[2048];
    sprintf(msg, "variable %s is not a global variable, cannot read using readValue()",name);
    BPatch_reportError(BPatchWarning, 109,msg);
    return false;
  }

  appAddSpace->getAS()->readDataSpace(address, len, dst, true);
  return true;
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

bool BPatch_variableExpr::writeValueInt(const void *src, bool /* saveWorld */)
{
  if (isLocal) {
    char msg[2048];
    sprintf(msg, "variable %s is not a global variable, cannot write",name);
    BPatch_reportError(BPatchWarning, 109,msg);
    return false;
  }

  if (size) {
      if (!appAddSpace->getAS()->writeDataSpace(address, size, src)) {
          fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
      }          
      return true;
  }
  else {
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
bool BPatch_variableExpr::writeValueWithLength(const void *src, int len, bool /*saveWorld*/)
{
  if (isLocal) {
    char msg[2048];
    sprintf(msg, "variable %s is not a global variable, cannot write",name);
    BPatch_reportError(BPatchWarning, 109,msg);
    return false;
  }

  if (!appAddSpace->getAS()->writeDataSpace(address, len, src)) {
      fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
  }          
  return true;
}

char *BPatch_variableExpr::getNameInt()
{
  return name;
}

void *BPatch_variableExpr::getBaseAddrInt()
{
  return address;
}
/*
 * getComponents() - return variable expressions for all of the fields
 *     in the passed structure/union.
 */
BPatch_Vector<BPatch_variableExpr *> *BPatch_variableExpr::getComponentsInt()
{
    const BPatch_fieldListInterface *type;
    const BPatch_Vector<BPatch_field *> *fields;
    BPatch_Vector<BPatch_variableExpr *> *retList;

    type = dynamic_cast<const BPatch_fieldListInterface *>(getType());
    if (type == NULL) {
	return NULL;
    }

    retList = new BPatch_Vector<BPatch_variableExpr *>;
    assert(retList);

    fields = type->getComponents();
    if (fields == NULL) return NULL;
    for (unsigned int i=0; i < fields->size(); i++) {

	BPatch_field *field = (*fields)[i];
	long int offset = (field->offset / 8);

	BPatch_variableExpr *newVar;

	// convert to *(&basrVar + offset)
        AstNodePtr fieldExpr = AstNode::operandNode(AstNode::DataIndir,
                                                  AstNode::operatorNode(plusOp,
                                                                        AstNode::operatorNode(getAddrOp, (*ast_wrapper)),
                                                                        AstNode::operandNode(AstNode::Constant, (void *)offset)));

        // VG(03/02/02): What about setting the base address??? Here we go:
	if( field->_type != NULL ) {
            AstNodePtr *newAst = new AstNodePtr(fieldExpr);
	    newVar = new BPatch_variableExpr(const_cast<char *> (field->getName()),
					     /*appProcess,*/ appAddSpace, newAst,
                                             const_cast<BPatch_type *>(field->_type),
					     (char*)address + offset);
	    retList->push_back(newVar);
	} else {
	    bperr( "Warning: not returning field '%s' with NULL type.\n", field->getName() );
	}
    }

    return retList;
}

/*
 * BPatch_breakPointExpr::BPatch_breakPointExpr
 *
 * Construct a snippet representing a breakpoint.
 *
 */
void BPatch_breakPointExpr::BPatch_breakPointExprInt()
{
    pdvector<AstNodePtr > null_args;

    ast_wrapper = new AstNodePtr(AstNode::funcCallNode("DYNINST_snippetBreakpoint", null_args));

    assert(BPatch::bpatch != NULL);

    (*ast_wrapper)->setType(BPatch::bpatch->type_Untyped);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_effectiveAddressExpr::BPatch_effectiveAddressExpr
 *
 * Construct a snippet representing an effective address.
 */
void BPatch_effectiveAddressExpr::BPatch_effectiveAddressExprInt(int _which)
{
#if defined(i386_unknown_nt4_0)
  assert(_which >= 0 && _which <= 2);
#elif defined (__XLC__) || defined(__xlC__)
  assert(_which >= 0 && _which <= 1);
#else
  assert(_which >= 0 && _which <= (int) BPatch_instruction::nmaxacc_NP);
#endif
  ast_wrapper = new AstNodePtr(AstNode::memoryNode(AstNode::EffectiveAddr, _which));
};


/*
 * BPatch_bytesAccessedExpr::BPatch_bytesAccessedExpr
 *
 * Construct a snippet representing the number of bytes accessed.
 */
void BPatch_bytesAccessedExpr::BPatch_bytesAccessedExprInt(int _which)
{
#if defined(i386_unknown_nt4_0)
  assert(_which >= 0 && _which <= 2);
#elif defined (__XLC__) || defined(__xlC__)
  assert(_which >= 0 && _which <= 1);
#else
  assert(_which >= 0 && _which <= (int)BPatch_instruction::nmaxacc_NP);
#endif
  ast_wrapper = new AstNodePtr(AstNode::memoryNode(AstNode::BytesAccessed, _which));
};


void BPatch_ifMachineConditionExpr::BPatch_ifMachineConditionExprInt(const BPatch_snippet &tClause)
{
    ast_wrapper = new AstNodePtr(AstNode::operatorNode(ifMCOp, *(tClause.ast_wrapper)));
    
    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
};

void BPatch_threadIndexExpr::BPatch_threadIndexExprInt()
{
    ast_wrapper = new AstNodePtr(AstNode::threadIndexNode());
    
    assert(BPatch::bpatch != NULL);
    (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
    BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");
    assert(type != NULL);
    //(*ast_wrapper)->setType(type);
    
}

void BPatch_tidExpr::BPatch_tidExprInt(BPatch_process *proc)
{
  BPatch_Vector<BPatch_function *> thread_funcs;
  proc->getImage()->findFunction("dyn_pthread_self", thread_funcs);
  if (thread_funcs.size() != 1)
  {
    fprintf(stderr, "[%s:%u] - Internal Dyninst error.  Found %u copies of "
	    "DYNINSTthreadIndex.  Expected 1\n", __FILE__, __LINE__, thread_funcs.size());
    if (!thread_funcs.size())
      return;
  }
  BPatch_function *thread_func = thread_funcs[0];
  
  pdvector<AstNodePtr> args;
  ast_wrapper = new AstNodePtr(AstNode::funcCallNode(thread_func->lowlevel_func(), args));

  assert(BPatch::bpatch != NULL);
  (*ast_wrapper)->setTypeChecking(BPatch::bpatch->isTypeChecked());
  BPatch_type *type = BPatch::bpatch->stdTypes->findType("long");
  assert(type != NULL);
  (*ast_wrapper)->setType(type);
}

// BPATCH INSN EXPR

void BPatch_insnExpr::BPatch_insnExprInt(BPatch_instruction *insn) {
    ast_wrapper = new AstNodePtr(AstNode::insnNode(insn));
}

bool BPatch_insnExpr::overrideLoadAddressInt(BPatch_snippet &l) {
    // We can assert our AST is an insn type...
    // Don't hand back insnAst to anyone else
    AstInsnNode *insnAst = dynamic_cast<AstInsnNode *>((*ast_wrapper).get());
    assert(insnAst);

    return insnAst->overrideLoadAddr(*(l.ast_wrapper));
}

bool BPatch_insnExpr::overrideStoreAddressInt(BPatch_snippet &s) {
    // We can assert our AST is an insn type...
    AstInsnNode *insnAst = dynamic_cast<AstInsnNode *>((*ast_wrapper).get());
    assert(insnAst);

    return insnAst->overrideStoreAddr(*(s.ast_wrapper));
}
