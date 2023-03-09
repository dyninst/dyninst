/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: BPatch_snippet.C,v 1.108 2008/06/19 19:52:55 legendre Exp $

#define BPATCH_FILE

#include <string.h>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "BPatch_function.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_libInfo.h"
#include "BPatch_point.h"

#include "addressSpace.h"
#include "mapped_object.h" // for savetheworld
#include "mapped_module.h"
#include "ast.h"
#include "function.h"
#include "instPoint.h"
#include "registerSpace.h"
#include "debug.h"
#include "dynProcess.h"
#include "pcEventHandler.h"

#include "RegisterConversion.h"

#include "symtabAPI/h/Type.h"
#include "symtabAPI/h/Variable.h"


using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

// Need REG_MT_POS, defined in inst-<arch>...

#if defined(arch_x86) || defined(arch_x86_64)
#include "inst-x86.h"
#elif defined(arch_power)
#include "inst-power.h"
#elif defined(arch_aarch64)
#include "inst-aarch64.h"
#else
#error "Unknown architecture, expected x86, x86_64, power or aarch64"
#endif


/*
 * BPatch_snippet::BPatch_snippet
 *
 * The default constructor was exposed as public, so we're
 * stuck with it even though that _should_ be an error.
 * For now, make it a null node (and hope nobody ever
 * tries to generate code)
 */
BPatch_snippet::BPatch_snippet() {
    ast_wrapper = AstNodePtr(AstNode::nullNode());
}

/*
 * BPatch_snippet::BPatch_snippet
 *
 * Copy constructor for BPatch_snippet.
 */
BPatch_snippet::BPatch_snippet(const BPatch_snippet &src)
{
    ast_wrapper = src.ast_wrapper;
}

BPatch_snippet::BPatch_snippet(const AstNodePtr &node)
{
   ast_wrapper = node;
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

    // We'll now contain another reference to the ast in the other snippet
    ast_wrapper = src.ast_wrapper;

    return *this;
}

/*
 * BPatch_snippet::getType
 */

BPatch_type *BPatch_snippet::getType(){
   return ast_wrapper->getType();
}

bool BPatch_snippet::is_trivial()
{
  return (ast_wrapper == NULL);
}

/*
 * BPatch_snippet::~BPatch_snippet
 *
 * Destructor for BPatch_snippet.  Deallocates memory allocated by the
 * snippet.  Well, decrements a reference count.
 */
BPatch_snippet::~BPatch_snippet()
{
    //if (ast_wrapper) delete ast_wrapper;
}


AstNodePtr generateVariableBase(const BPatch_snippet &lOperand)
{
  AstNodePtr variableBase;
  if(lOperand.ast_wrapper->getoType() == AstNode::operandType::variableValue)
  {
    variableBase = AstNode::operandNode(AstNode::operandType::variableAddr, lOperand.ast_wrapper->getOVar());
  }
  else if(lOperand.ast_wrapper->getoType() == AstNode::operandType::variableAddr)
  {
    variableBase = lOperand.ast_wrapper;
  }
  else
  {
    variableBase = AstNode::operatorNode(getAddrOp, lOperand.ast_wrapper);
  }
  return variableBase;
}


//
// generateArrayRef - Construct an Ast expression for an array.
//
AstNodePtr generateArrayRef(const BPatch_snippet &lOperand,
                            const BPatch_snippet &rOperand)
{
        if (lOperand.ast_wrapper == AstNodePtr()) return AstNodePtr();
        if (rOperand.ast_wrapper == AstNodePtr()) return AstNodePtr();

        if (lOperand.ast_wrapper->getType() == NULL)
        {
                BPatch_reportError(BPatchSerious, 109,
                                "array reference has no type information");
        }

        //  We have to be a little forgiving of the

        if (lOperand.ast_wrapper->getType() == NULL)
        {
                BPatch_reportError(BPatchSerious, 109,
                                "array reference has no type information");
                assert(0);
                return AstNodePtr();
        }

        typeArray *arrayType = lOperand.ast_wrapper->getType()->getSymtabType(Type::share)->getArrayType();
        if (!arrayType)
        {
			fprintf(stderr, "%s[%d]:  error here: type is %s\n", FILE__, __LINE__,
							lOperand.ast_wrapper->getType()->getName());
			BPatch_reportError(BPatchSerious, 109,
							"array reference has array reference to non-array type");
			assert(0);
			return AstNodePtr();
        }

        auto elementType = arrayType->getBaseType(Type::share);
        assert(elementType);
        long int elementSize = elementType->getSize();

        // check that the type of the right operand is an integer.
        //  We have to be a little forgiving of this parameter, since we could
        //  be indexing using a funcCall snippet, for which no return type is available.
        //  Ideally we could always know this information, but until then, if no
        //  type information is available, assume that the user knows what they're doing
        //  (just print a warning, don't fail).

        BPatch_type *indexType = const_cast<BPatch_type *>(rOperand.ast_wrapper->getType());

        if (!indexType)
        {
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
                        && strcmp(indexType->getName(), "signed")
                        && strcmp(indexType->getName(), "unsigned int")
                        && strcmp(indexType->getName(), "unsigned short")
                        && strcmp(indexType->getName(), "unsigned long")
                        && strcmp(indexType->getName(), "unsigned"))
        {
                char err_buf[256];
                sprintf(err_buf, "%s[%d]: non-integer array index type %s\n",
                                __FILE__, __LINE__,  indexType->getName());
                fprintf(stderr, "%s\n", err_buf);
                BPatch_reportError(BPatchSerious, 109, err_buf);
                assert(0);
                return AstNodePtr();
        }

        //fprintf(stderr, "%s[%d]:  indexing with type %s\n", __FILE__, __LINE__,
        //        indexType->getName());

        //
        // Convert a[i] into *(&a + (* i sizeof(element)))
        //

        AstNodePtr arrayBase = generateVariableBase(lOperand);
        AstNodePtr ast = AstNode::operandNode(AstNode::operandType::DataIndir,
                        AstNode::operatorNode(plusOp,
                                arrayBase,
                                AstNode::operatorNode(timesOp,
                                        AstNode::operandNode(AstNode::operandType::Constant,
                                                (void *)elementSize),
                                        rOperand.ast_wrapper)));

        BPatch_type *elem_bptype = NULL;
        extern AnnotationClass<BPatch_type> TypeUpPtrAnno;

        if (!elementType->getAnnotation(elem_bptype, TypeUpPtrAnno))
        {
                //  BPatch_type ctor adds itself as an anonotation to the symtab
                //  type, so no need to do it here.
                elem_bptype = new BPatch_type(elementType);
        }
        else
        {
                assert(elem_bptype);
        }

        ast->setType(elem_bptype);

        return AstNodePtr(ast);
}


//
// generateFieldRef - Construct an Ast expression for an structure field.
//
AstNodePtr generateFieldRef(const BPatch_snippet &lOperand,
                const BPatch_snippet &rOperand)
{
        if (lOperand.ast_wrapper == AstNodePtr()) return AstNodePtr();
        if (rOperand.ast_wrapper == AstNodePtr()) return AstNodePtr();

        if (lOperand.ast_wrapper->getType() == NULL)
        {
                BPatch_reportError(BPatchSerious, 109,
                                "array reference has no type information");
        }

        typeStruct *structType = lOperand.ast_wrapper->getType()->getSymtabType(Type::share)->getStructType();

        if (!structType)
        {
                BPatch_reportError(BPatchSerious, 109,
                                "structure reference has no type information, or structure reference to non-structure type");
                assert(0);
                return AstNodePtr();
        }

        // check that the type of the right operand is a string.

        BPatch_type *fieldType = const_cast<BPatch_type *>(rOperand.ast_wrapper->getType());

        if (rOperand.ast_wrapper->getoType()!=AstNode::operandType::ConstantString
                        || !fieldType
                        || strcmp(fieldType->getName(), "char *"))
        {
                // XXX - Should really check if this is a short/long too
                BPatch_reportError(BPatchSerious, 109,
                                "field name is not of type char *");
                assert(0);
                return AstNodePtr();
        }

        Field *field = NULL;

        // check that the name of the right operand is a field of the left operand

        auto fields = structType->getComponents();

        unsigned int i;

        for (i=0; i < fields->size(); i++)
        {
                field = (*fields)[i];
                if (!strcmp(field->getName().c_str(), (const char *) rOperand.ast_wrapper->getOValue()))
                        break;
        }

        if (i==fields->size())
        {
                BPatch_reportError(BPatchSerious, 109,
                                "field name not found in structure");
                assert(0);
                return AstNodePtr();
        }

        if (! field ) {
           assert(0);
           return AstNodePtr();
        }

        long int offset = (field->getOffset() / 8);

        //
        // Convert s.f into *(&s + offset(s,f))
        //

        AstNodePtr structBase = generateVariableBase(lOperand);

        AstNodePtr ast = AstNode::operandNode(AstNode::operandType::DataIndir,
                        AstNode::operatorNode(plusOp,
                                structBase,
                                AstNode::operandNode(AstNode::operandType::Constant,
                                        (void *)offset)));

        extern AnnotationClass<BPatch_type> TypeUpPtrAnno;
        BPatch_type *field_bptype = NULL;
        auto field_type = field->getType(Type::share);
        assert(field_type);


        if (!field_type->getAnnotation(field_bptype, TypeUpPtrAnno))
        {
                //  BPatch_type ctor adds itself as an annotation of the symtab type
                //  so no need to do it here.

                field_bptype = new BPatch_type(field_type);
        }
        else
        {
                assert(field_bptype);
        }

        ast->setType(field_bptype);

        return AstNodePtr(ast);
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

   std::vector<BPatch_snippet *> argVect;

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
                case BPatch_xor:
                        astOp = xorOp;
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
         ast_wrapper = generateArrayRef(lOperand, rOperand);
                        if (!ast_wrapper) {
            BPatch_reportError(BPatchSerious, 100 /* what # to use? */,
                                                "could not generate array reference.");
                                BPatch_reportError(BPatchSerious, 100,
                                                "resulting snippet is invalid.");
                        }
                        return;
                        break;
                case BPatch_fieldref:
                        ast_wrapper = generateFieldRef(lOperand, rOperand);
                        if (!ast_wrapper) {
                                BPatch_reportError(BPatchSerious, 100 /* what # to use? */,
                                                "could not generate field reference.");
                                BPatch_reportError(BPatchSerious, 100,
                                                "resulting snippet is invalid.");
                        }
                        return;
                        break;
                case BPatch_seq:
                        {
                                std::vector<AstNodePtr > sequence;
                                sequence.push_back(lOperand.ast_wrapper);
                                sequence.push_back(rOperand.ast_wrapper);

                                ast_wrapper = AstNode::sequenceNode(sequence);
                                ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
                                return;
                        }
                default:
                        /* XXX handle error */
                        assert(0);
        };

        ast_wrapper = AstNodePtr(AstNode::operatorNode(astOp,
                                lOperand.ast_wrapper,
                                rOperand.ast_wrapper));
        ast_wrapper->setType(lOperand.ast_wrapper->getType());
        ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
                                                                AstNodePtr negOne = AstNode::operandNode(AstNode::operandType::Constant,
                                                                                (void *)-1);
          BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");
          assert(type != NULL);
          negOne->setType(type);
          ast_wrapper =  AstNodePtr(AstNode::operatorNode(timesOp,
                                                          negOne,
                                                          lOperand.ast_wrapper));
          break;
      }
   case BPatch_addr:  {
     ast_wrapper = AstNodePtr(generateVariableBase(lOperand));
       // create a new type which is a pointer to type
       BPatch_type *baseType = const_cast<BPatch_type *>
           (lOperand.ast_wrapper->getType());
       BPatch_type *type = BPatch::bpatch->createPointer("<PTR>",
                                                         baseType, sizeof(void *));
       assert(type);
       ast_wrapper->setType(type);
       break;
   }

   case BPatch_deref: {
#if 0
       // Handle constant addresses...
       if (lOperand.ast_wrapper->getoType() == AstNode::Constant) {
           ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::DataAddr,
                                                         const_cast<void *>(lOperand.ast_wrapper->getOValue())));
       }
       else {
           ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::DataIndir, lOperand.ast_wrapper));
       }
#endif
          ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::DataIndir, lOperand.ast_wrapper));

          BPatch_type *type = const_cast<BPatch_type *> (lOperand.ast_wrapper->getType());
          if (!type || (type->getDataClass() != BPatch_dataPointer)) {
              ast_wrapper->setType(BPatch::bpatch->stdTypes->findType("long"));
          } else {
              ast_wrapper->setType(type->getConstituentType());
//              ast_wrapper->setType(dynamic_cast<BPatch_typePointer *>(type)->getConstituentType());
          }
          break;
      }

      default:
         /* XXX handle error */
         assert(0);
   };

   ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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

    ast_wrapper = AstNodePtr(AstNode::operatorNode(astOp, lOperand.ast_wrapper, rOperand.ast_wrapper));
    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_constExpr::BPatch_constExpr
 *
 * Constructs a snippet representing a constant integer value.
 *
 * value        The desired value.
 */

BPatch_constExpr::BPatch_constExpr( signed int value ) {
        assert( BPatch::bpatch != NULL );

        ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant,
                                                                 (void *)(uintptr_t) value));
        ast_wrapper->setTypeChecking( BPatch::bpatch->isTypeChecked() );

        BPatch_type * type = BPatch::bpatch->stdTypes->findType( "int" );
        assert( type != NULL );
        ast_wrapper->setType( type );
}

BPatch_constExpr::BPatch_constExpr( unsigned int value ) {
        assert( BPatch::bpatch != NULL );

        ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant,
                                                                 (void *)(uintptr_t) value));
        ast_wrapper->setTypeChecking( BPatch::bpatch->isTypeChecked() );

        BPatch_type * type = BPatch::bpatch->stdTypes->findType( "unsigned int" );
        assert( type != NULL );
        ast_wrapper->setType( type );
        }

BPatch_constExpr::BPatch_constExpr( signed long value ) {
        assert( BPatch::bpatch != NULL );

        ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant,
                                                                 (void *)(uintptr_t) value));
        ast_wrapper->setTypeChecking( BPatch::bpatch->isTypeChecked() );

        BPatch_type * type = BPatch::bpatch->stdTypes->findType( "long" );
        assert( type != NULL );
        ast_wrapper->setType( type );
        }

BPatch_constExpr::BPatch_constExpr( unsigned long value ) {
        assert( BPatch::bpatch != NULL );

        ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant,
                                                                 (void *)(uintptr_t) value));
        ast_wrapper->setTypeChecking( BPatch::bpatch->isTypeChecked() );
        BPatch_type * type = BPatch::bpatch->stdTypes->findType( "unsigned long" );
        assert( type != NULL );
        ast_wrapper->setType( type );
        }

BPatch_constExpr::BPatch_constExpr(unsigned long long value) {
	assert(BPatch::bpatch != NULL);

	ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant,
		(void *)(uintptr_t)value));
	ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
	BPatch_type * type = BPatch::bpatch->stdTypes->findType("unsigned long long");
	assert(type != NULL);
	ast_wrapper->setType(type);
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
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::ConstantString, (void *)const_cast<char *>(value)));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("char *");
    assert(type != NULL);

    ast_wrapper->setType(type);
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
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant, (void *)const_cast<void *>(value)));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *type = BPatch::bpatch->stdTypes->findType("void *");
    assert(type != NULL);

    ast_wrapper->setType(type);
}

BPatch_constExpr::BPatch_constExpr(long long value)
{
   ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::Constant, (void *)(uintptr_t)value));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type* type = BPatch::bpatch->stdTypes->findType("long long");

    assert(type != NULL);
    ast_wrapper->setType(type);
}

/*
 * BPatch_whileExpr::BPatch_whileExpr
 *
 * Creates a while loop; the first parameter is the
 * conditional (true indicates executing the body of the loop)
 * and the second is the body of the loop.
 */

BPatch_whileExpr::BPatch_whileExpr(const BPatch_snippet &conditional,
                                    const BPatch_snippet &body) {
   ast_wrapper = AstNodePtr(AstNode::operatorNode(whileOp, 
                                                  conditional.ast_wrapper, 
                                                  body.ast_wrapper));
   assert(BPatch::bpatch != NULL);
   ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
    std::vector<AstNodePtr> ast_args;

    unsigned int i;
    for (i = 0; i < args.size(); i++) {
        assert(args[i]->ast_wrapper);
        ast_args.push_back(args[i]->ast_wrapper);
    }

    //  jaw 08/03  part of cplusplus bugfix -- using pretyName
    //  to generate function calls can lead to non uniqueness probs
    //  in the case of overloaded callee functions.

    ast_wrapper = AstNodePtr(AstNode::funcCallNode(func.lowlevel_func(), ast_args));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

    BPatch_type *ret_type = const_cast<BPatch_function &>(func).getReturnType();
      ast_wrapper->setType(ret_type);
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
    ast_wrapper = AstNodePtr(AstNode::operatorNode(ifOp, conditional.ast_wrapper, tClause.ast_wrapper));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
    ast_wrapper = AstNodePtr(AstNode::operatorNode(ifOp,
                                                   conditional.ast_wrapper,
                                                   tClause.ast_wrapper,
                                                   fClause.ast_wrapper));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_nullExpr::BPatch_nullExpr
 *
 * Construct a null snippet that can be used as a placeholder.
 */
BPatch_nullExpr::BPatch_nullExpr()
{
    ast_wrapper = AstNodePtr(AstNode::nullNode());

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
BPatch_paramExpr::BPatch_paramExpr(int n, BPatch_ploc loc)
{
    AstNode::operandType opType;
    switch(loc) {
        case (BPatch_ploc_guess):
            opType = AstNode::operandType::Param;
            break;
        case (BPatch_ploc_call):
            opType = AstNode::operandType::ParamAtCall;
            break;
        case (BPatch_ploc_entry):
            opType = AstNode::operandType::ParamAtEntry;
            break;
        default:
            assert(0);
            break;
    }

    ast_wrapper = AstNodePtr(AstNode::operandNode(opType,
                                                  (void *)(long)n));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::ReturnVal, (void *)0));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_retAddrExpr::BPatch_retAddrExpr
 *
 * Construct a snippet representing a return value from the function in which
 * the snippet is inserted.
 *
 */
BPatch_retAddrExpr::BPatch_retAddrExpr()
{
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::ReturnAddr, (void *)0));
    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_registerExpr::BPatch_registerExpr
 *
 * Construct a snippet representing a register in original code. Can be read
 * or written.
 *
 */
BPatch_registerExpr::BPatch_registerExpr(BPatch_register reg)
{
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::origRegister,
                                                      (void *)(long)reg.number_));

    assert(BPatch::bpatch != NULL);

    // Registers can hold a lot of different types...
    ast_wrapper->setTypeChecking(false);
    //ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

BPatch_registerExpr::BPatch_registerExpr(Dyninst::MachRegister mach) {
   bool whocares;
   Register reg = convertRegID(mach, whocares);
   ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::origRegister,
                                                 (void *)(intptr_t)reg));
    assert(BPatch::bpatch != NULL);

    // Registers can hold a lot of different types...
    ast_wrapper->setTypeChecking(false);
    //ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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

    std::vector<AstNodePtr >sequence;
    for (unsigned i = 0; i < items.size(); i++) {
        assert(items[i]->ast_wrapper);
        sequence.push_back(items[i]->ast_wrapper);
    }
    ast_wrapper = AstNodePtr(AstNode::sequenceNode(sequence));

    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
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
BPatch_variableExpr::BPatch_variableExpr(const char *in_name,
                                         BPatch_addressSpace *in_addSpace,
                                         AddressSpace *in_lladdSpace,
                                         AstNodePtr ast_wrapper_,
                                         BPatch_type *typ,
                                         void* in_address) :
  name(in_name),
  appAddSpace(in_addSpace),
  lladdrSpace(in_lladdSpace),
  address(in_address),
  scope(NULL),
  isLocal(false),
  type(typ),
  intvar(NULL)
{
    ast_wrapper = ast_wrapper_;
    assert(ast_wrapper);

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

    ast_wrapper->setType(type);

    size = type->getSize();
}

BPatch_variableExpr::BPatch_variableExpr(BPatch_addressSpace *in_addSpace,
                                         AddressSpace *ll_addSpace, int_variable *iv,
                                         BPatch_type *type_)
  : name(),
    appAddSpace(in_addSpace),
    lladdrSpace(ll_addSpace),
    address(NULL),
    scope(NULL),
    isLocal(false),
    type(type_),
    intvar(NULL)
{
  const image_variable* img_var = NULL;
  if(iv)
  {
    name = iv->symTabName();
    address = reinterpret_cast<void*>(iv->getAddress());
    intvar = iv;
    img_var = iv->ivar();
  }
  size = type_->getSize();
  if(img_var)
  {
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::variableValue, img_var));
  }
  else
  {
    ast_wrapper = AstNodePtr(AstNode::operandNode(AstNode::operandType::DataAddr, (void*)(NULL)));
  }


  ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
  ast_wrapper->setType(type_);

}


BPatch_variableExpr* BPatch_variableExpr::makeVariableExpr(BPatch_addressSpace* in_addSpace,
                                                 int_variable* v,
                                                 BPatch_type* type)
{
  AddressSpace* llAS = v->mod()->proc();
  return new BPatch_variableExpr(in_addSpace, llAS, v, type);
}

BPatch_variableExpr* BPatch_variableExpr::makeVariableExpr(BPatch_addressSpace* in_addSpace,
                                                           AddressSpace* in_llAddSpace,
                                                           std::string name,
                                                           void* offset,
                                                           BPatch_type* type)
{

    int_variable* v = in_llAddSpace->getAOut()->getDefaultModule()->createVariable(name,
                                                                                   reinterpret_cast<Address>(offset),
                                                                                   type->getSize());
    return new BPatch_variableExpr(in_addSpace, in_llAddSpace, v, type);
}


unsigned int BPatch_variableExpr::getSize() const
{
  return size;
}

/*
 * BPatch_variableExpr::getType
 *
 *    Return the variable's type
 *
*/
const BPatch_type *BPatch_variableExpr::getType()
{
  if (!type){
     return BPatch::bpatch->type_Untyped;
  }
  return type;
}

/*
 * BPatch_variableExpr::setType
 *
 *    Set the variable's type
 *
*/
bool BPatch_variableExpr::setType(BPatch_type *newType)
{
    size = newType->getSize();
    type = newType;
    ast_wrapper->setType(newType);
    return true;
}
/*
 * BPatch_variableExpr::seSize
 *
 *    Set the variable's size
 *
*/
bool BPatch_variableExpr::setSize(int sz)
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
 * in_addSpace  The BPatch_addressSpace that the variable resides in.
 * in_address   The address of the variable in the inferior's address space.
 * in_register  The register of the variable in the inferior's address space.
 * type         The type of the variable.
 * in_storage   Enum of how this variable is stored.
 */
BPatch_variableExpr::BPatch_variableExpr(BPatch_addressSpace *in_addSpace,
                                         AddressSpace *in_lladdrSpace,
                                         void *in_address,
                                         int in_register,
                                         BPatch_type *typ,
                                         BPatch_storageClass in_storage,
                                         BPatch_point *scp) :
   name(),
   appAddSpace(in_addSpace),
   lladdrSpace(in_lladdrSpace),
   address(in_address),
   type(typ),
   intvar(NULL)
{
   vector<AstNodePtr> variableASTs;
   AstNodePtr variableAst;

   if (!type)
     type = BPatch::bpatch->type_Untyped;
   switch (in_storage) {
      case BPatch_storageAddr:
          variableAst = AstNodePtr(AstNode::operandNode(AstNode::operandType::DataAddr, address));
          isLocal = false;
         break;
      case BPatch_storageAddrRef:
         assert( 0 ); // Not implemented yet.
         isLocal = false;
         break;
      case BPatch_storageReg:
         variableAst = AstNodePtr(AstNode::operandNode(AstNode::operandType::origRegister,
                                                           (void *)(long)in_register));
         isLocal = true;
         break;
      case BPatch_storageRegRef:
         assert( 0 ); // Not implemented yet.
         isLocal = true;
         break;
      case BPatch_storageRegOffset:
         variableAst = AstNodePtr(AstNode::operandNode(AstNode::operandType::RegOffset,
                                        AstNode::operandNode(AstNode::operandType::DataAddr,
                                                             address)));
         variableAst->setOValue( (void *)(long int)in_register );
         isLocal = true;
         break;
      case BPatch_storageFrameOffset:
         variableAst = AstNodePtr(AstNode::operandNode(AstNode::operandType::FrameAddr, address));
         isLocal = true;
         break;
   }

   variableAst->setTypeChecking(BPatch::bpatch->isTypeChecked());
   variableAst->setType(type);
   variableASTs.push_back(variableAst);
   ast_wrapper = AstNode::variableNode(variableASTs);
   //    ast_wrapper = variableAst;
   assert(BPatch::bpatch != NULL);
   ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

   size = type->getSize();
   ast_wrapper->setType(type);

   scope = scp;
}

/*
 * BPatch_variableExpr::BPatch_variableExpr
 *
 * Construct a snippet representing a variable of the given type at the given
 * address.
 *
 * in_addSpace  The BPatch_addressSpace that the variable resides in.
 * lv           The local variable handle
 * type             The type of the variable.
 *
 */
BPatch_variableExpr::BPatch_variableExpr(BPatch_addressSpace *in_addSpace,
                                         AddressSpace *in_lladdSpace,
                                         BPatch_localVar *lv, BPatch_type *typ,
                                         BPatch_point *scp):
   name(),
   appAddSpace(in_addSpace),
   lladdrSpace(in_lladdSpace),
   address(NULL),
   isLocal(false),
   type(typ),
   intvar(NULL)
{

        //Create Ast's for all members in the location list.
        //This will likely be done only for local variables within a function
        if (!type)
                type = BPatch::bpatch->type_Untyped;

        Address baseAddr =  scp->getFunction()->lowlevel_func()->obj()->codeBase();
        vector<AstNodePtr> variableASTs;
        vector<pair<Offset, Offset> > *ranges = new vector<pair<Offset, Offset> >;
        vector<Dyninst::VariableLocation> &locs = lv->getSymtabVar()->getLocationLists();
        for (unsigned i=0; i<locs.size(); i++)
        {
                AstNodePtr variableAst;
                BPatch_storageClass in_storage = lv->convertToBPatchStorage(& locs[i]);
                void *in_address = (void *) locs[i].frameOffset;
                bool ignored;
                int in_register = convertRegID(locs[i].mr_reg, ignored);
                switch (in_storage) {
                        case BPatch_storageAddr:
                                variableAst = AstNode::operandNode(AstNode::operandType::DataAddr, in_address);
                                isLocal = false;
                                address = in_address;
                                break;
                        case BPatch_storageAddrRef:
                                //assert( 0 ); // Not implemented yet.
                                continue;
                        case BPatch_storageReg:
                                variableAst = AstNode::operandNode(AstNode::operandType::origRegister,
                                                (void *)(long)in_register);
                                isLocal = true;
                                break;
                        case BPatch_storageRegRef:
                                //assert( 0 ); // Not implemented yet.
                                continue;
                        case BPatch_storageRegOffset:
                                variableAst = AstNode::operandNode(AstNode::operandType::RegOffset,
                                                AstNode::operandNode(AstNode::operandType::DataAddr,
                                                        in_address));
                                variableAst->setOValue( (void *)(long int)in_register );
                                isLocal = true;
                                break;
                        case BPatch_storageFrameOffset:
                                variableAst = AstNode::operandNode(AstNode::operandType::FrameAddr, in_address);
                                isLocal = true;
                                break;
                }
                variableAst->setTypeChecking(BPatch::bpatch->isTypeChecked());
                variableAst->setType(type);
                variableASTs.push_back(variableAst);
                   
				Address low, hi;
				if (locs[i].lowPC == 0 && locs[i].hiPC == (Address) -1) {
					low = 0;
					hi = (Address) -1;
				}
				else {
					low = locs[i].lowPC + baseAddr;
					hi = locs[i].hiPC + baseAddr;
				}

                ranges->push_back(pair<Address, Address>(low, hi));
        }
        ast_wrapper = AstNodePtr(AstNode::variableNode(variableASTs, ranges));
        //    ast_wrapper = variableAst;
        assert(BPatch::bpatch != NULL);
        ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());

        ast_wrapper->setType(type);
        size = type->getSize();

        scope = scp;
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
	if (isLocal) {
		char msg[2048];
		sprintf(msg, "variable %s is not a global variable, cannot read using readValue()",name.c_str());
		BPatch_reportError(BPatchWarning, 109,msg);
		return false;
	}

	if (size == 2 || size == 4 || size == 8) {
		// XXX - We should be going off of type here, not just size.
		if (!lladdrSpace->readDataWord(address, size, dst, true)) return false;
		return true;
	} else if (size) {
		if (!lladdrSpace->readDataSpace(address, size, dst, true)) return false;
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
bool BPatch_variableExpr::readValue(void *dst, int len)
{
        if (isLocal) {
                char msg[2048];
                sprintf(msg, "variable %s is not a global variable, cannot read using readValue()",name.c_str());
                BPatch_reportError(BPatchWarning, 109,msg);
                return false;
        }

        return lladdrSpace->readDataSpace(address, len, dst, true);
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

bool BPatch_variableExpr::writeValue(const void *src, bool /* saveWorld */)
{
  if (isLocal) {
    char msg[2048];
    sprintf(msg, "variable %s is not a global variable, cannot write",name.c_str());
    BPatch_reportError(BPatchWarning, 109,msg);
    return false;
  }

  if (size) {
      bool writeStatus;

      if (size == 2 || size == 4 || size == 8)
          writeStatus = lladdrSpace->writeDataWord(address, size, src);
      else
          writeStatus = lladdrSpace->writeDataSpace(address, size, src);

      if (!writeStatus) {
        std::stringstream errorMsg;
        errorMsg << "variable " << name << " in .bss section, cannot write";
        BPatch_reportError(BPatchWarning, 109, errorMsg.str().c_str());
        return false;
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
bool BPatch_variableExpr::writeValue(const void *src, int len, bool /*saveWorld*/)
{
  if (isLocal) {
    char msg[2048];
    sprintf(msg, "variable %s is not a global variable, cannot write",name.c_str());
    BPatch_reportError(BPatchWarning, 109,msg);
    return false;
  }

  if (!lladdrSpace->writeDataSpace(address, len, src)) {
    std::stringstream errorMsg;
    errorMsg << "variable " << name << " in .bss section, cannot write";
    BPatch_reportError(BPatchWarning, 109, errorMsg.str().c_str());
    return false;
  }
  return true;
}

AddressSpace *BPatch_variableExpr::getAS()
{
   return lladdrSpace;
}

const char *BPatch_variableExpr::getName()
{
  return name.empty() ? NULL : name.c_str();
}

void *BPatch_variableExpr::getBaseAddr()
{
  return address;
}
/*
 * getComponents() - return variable expressions for all of the fields
 *     in the passed structure/union.
 */
BPatch_Vector<BPatch_variableExpr *> *BPatch_variableExpr::getComponents()
{
    const BPatch_Vector<BPatch_field *> *fields;
    BPatch_Vector<BPatch_variableExpr *> *retList = new BPatch_Vector<BPatch_variableExpr *>;
    fields = getType()->getComponents();
    if (fields == NULL) return NULL;
    for (unsigned int i=0; i < fields->size(); i++) {

       BPatch_field *field = (*fields)[i];
       long int offset = (field->getOffset() / 8);
       BPatch_variableExpr *newVar;

       // convert to *(&basrVar + offset)
       AstNodePtr fieldExpr = AstNode::operandNode(AstNode::operandType::DataIndir,
                                                   AstNode::operatorNode(plusOp,
                                                                         generateVariableBase(*this),
                                                   AstNode::operandNode(AstNode::operandType::Constant, (void *)offset)));
       if( field->getType() != NULL ) {
           AstNodePtr newAst = fieldExpr;
           newVar = new BPatch_variableExpr(const_cast<char *> (field->getName()),
                                            appAddSpace, lladdrSpace,
                                            newAst,
                                            field->getType(),
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
BPatch_breakPointExpr::BPatch_breakPointExpr()
{
    std::vector<AstNodePtr > null_args;

    ast_wrapper = AstNodePtr(AstNode::funcCallNode("DYNINST_snippetBreakpoint", null_args));

    assert(BPatch::bpatch != NULL);

    ast_wrapper->setType(BPatch::bpatch->type_Untyped);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


/*
 * BPatch_effectiveAddressExpr::BPatch_effectiveAddressExpr
 *
 * Construct a snippet representing an effective address.
 */
BPatch_effectiveAddressExpr::BPatch_effectiveAddressExpr(int _which, int size)
{
#if defined(i386_unknown_nt4_0)
  assert(_which >= 0 && _which <= 2);
#else
  assert(_which >= 0 && _which <= (int) BPatch_instruction::nmaxacc_NP);
#endif
  ast_wrapper = AstNodePtr(AstNode::memoryNode(AstNode::EffectiveAddr, _which, size));
}


/*
 * BPatch_bytesAccessedExpr::BPatch_bytesAccessedExpr
 *
 * Construct a snippet representing the number of bytes accessed.
 */
BPatch_bytesAccessedExpr::BPatch_bytesAccessedExpr(int _which)
{
#if defined(i386_unknown_nt4_0)
  assert(_which >= 0 && _which <= 2);
#else
  assert(_which >= 0 && _which <= (int)BPatch_instruction::nmaxacc_NP);
#endif
  ast_wrapper = AstNodePtr(AstNode::memoryNode(AstNode::BytesAccessed, _which));
}


BPatch_ifMachineConditionExpr::BPatch_ifMachineConditionExpr(const BPatch_snippet &tClause)
{
    ast_wrapper = AstNodePtr(AstNode::operatorNode(ifMCOp, tClause.ast_wrapper));

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

BPatch_threadIndexExpr::BPatch_threadIndexExpr()
{
    ast_wrapper = AstNodePtr(AstNode::threadIndexNode());

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
    BPatch_type *type = BPatch::bpatch->stdTypes->findType("int");
    assert(type != NULL);
    //ast_wrapper->setType(type);

}

BPatch_tidExpr::BPatch_tidExpr(BPatch_process *proc)
{
  BPatch_Vector<BPatch_function *> thread_funcs;
  proc->getImage()->findFunction("dyn_pthread_self", thread_funcs);
  if (thread_funcs.size() != 1)
  {
    fprintf(stderr, "[%s:%d] - Internal Dyninst error.  Found %lu copies of "
            "DYNINSTthreadIndex.  Expected 1\n", __FILE__, __LINE__, thread_funcs.size());
    if (!thread_funcs.size())
      return;
  }
  BPatch_function *thread_func = thread_funcs[0];

  std::vector<AstNodePtr> args;
  ast_wrapper = AstNodePtr(AstNode::funcCallNode(thread_func->lowlevel_func(), args));

  assert(BPatch::bpatch != NULL);
  ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
  BPatch_type *type = BPatch::bpatch->stdTypes->findType("long");
  assert(type != NULL);
  ast_wrapper->setType(type);
}

// BPATCH INSN EXPR


/* Causes us to create only one StopThreadCallback per
   BPatchStopThreadCallback, though we create a function call snippet
   to DYNINST_stopThread for each individual stopThreadExpr.  It's not
   necessary that we limit StopThreadCallbacks creations like this. */
static std::set<BPatchStopThreadCallback> *stopThread_cbs=NULL;

static void constructorHelper(
   const BPatchStopThreadCallback &bp_cb,
   bool useCache,
   BPatch_stInterpret interp,
   AstNodePtr &idNode,
   AstNodePtr &icNode)
{
    //register the callback if it's new
    if (stopThread_cbs == NULL) {
        stopThread_cbs = new std::set<BPatchStopThreadCallback>;
    }

    std::set<BPatchStopThreadCallback>::iterator cbIter = 
        stopThread_cbs->find(bp_cb);
    if (cbIter == stopThread_cbs->end()) {
       stopThread_cbs->insert(bp_cb);
       BPatch::bpatch->registerStopThreadCallback(bp_cb);
    }

    // create callback ID argument
    intptr_t cb_id = BPatch::bpatch->getStopThreadCallbackID(bp_cb); 
    idNode = AstNode::operandNode(AstNode::operandType::Constant, (void*) cb_id );
    BPatch_type *inttype = BPatch::bpatch->stdTypes->findType("int");
    assert(inttype != NULL);
    idNode->setType(inttype);

    // create interpret/usecache argument
    intptr_t ic = 0;
    if (useCache)
        ic += 1;
    if (interp == BPatch_interpAsTarget)
        ic += 2;
    else if (interp == BPatch_interpAsReturnAddr)
        ic += 4;
    icNode = AstNode::operandNode(AstNode::operandType::Constant, (void*) ic );
    icNode->setType(inttype);
}

/* BPatch_stopThreadExpr
 *
 *  This snippet type stops the thread that executes it.  It
 *  evaluates a calculation snippet and triggers a callback to the
 *  user program with the result of the calculation and a pointer to
 *  the BPatch_point at which the snippet was inserted
 */
BPatch_stopThreadExpr::BPatch_stopThreadExpr
      (const BPatchStopThreadCallback &bp_cb,
       const BPatch_snippet &calculation,
       bool useCache,
       BPatch_stInterpret interp)
{
    AstNodePtr idNode;
    AstNodePtr icNode;
    constructorHelper(bp_cb, useCache, interp, idNode, icNode);

    // set up funcCall args
    std::vector<AstNodePtr> ast_args;
    ast_args.push_back(AstNode::actualAddrNode());
    ast_args.push_back(idNode);
    ast_args.push_back(icNode);
    ast_args.push_back(calculation.ast_wrapper);

    // create func call & set type
    ast_wrapper = AstNodePtr(AstNode::funcCallNode("DYNINST_stopThread", ast_args));
    ast_wrapper->setType(BPatch::bpatch->type_Untyped);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


  // for internal use in conjunction with memory emulation and defensive
  // mode analysis
BPatch_stopThreadExpr::BPatch_stopThreadExpr(
   const BPatchStopThreadCallback &bp_cb,
   const BPatch_snippet &calculation,
   const mapped_object &obj,
   bool useCache,
   BPatch_stInterpret interp)
{
    AstNodePtr idNode;
    AstNodePtr icNode;
    constructorHelper(bp_cb, useCache, interp, idNode, icNode);

    Address objStart = obj.codeBase();
    Address objEnd = objStart + obj.imageSize();
    AstNodePtr objStartNode = AstNode::operandNode(
        AstNode::operandType::Constant, (void*) objStart);
    AstNodePtr objEndNode = AstNode::operandNode(
        AstNode::operandType::Constant, (void*) objEnd);
    BPatch_type *ulongtype = BPatch::bpatch->stdTypes->findType("unsigned long");
    objStartNode->setType(ulongtype);
    objEndNode->setType(ulongtype);

    // set up funcCall args
    std::vector<AstNodePtr> ast_args;
    ast_args.push_back(AstNode::actualAddrNode());
    ast_args.push_back(idNode);
    ast_args.push_back(icNode);
    ast_args.push_back(calculation.ast_wrapper);
    ast_args.push_back(objStartNode);
    ast_args.push_back(objEndNode);

    // create func call & set type
    ast_wrapper = AstNodePtr(AstNode::funcCallNode("DYNINST_stopInterProc", ast_args));
    ast_wrapper->setType(BPatch::bpatch->type_Untyped);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}


BPatch_shadowExpr::BPatch_shadowExpr
      (bool entry,
      const BPatchStopThreadCallback &bp_cb,
       const BPatch_snippet &calculation,
       bool useCache,
       BPatch_stInterpret interp)
{
    AstNodePtr idNode;
    AstNodePtr icNode;
    constructorHelper(bp_cb, useCache, interp, idNode, icNode);

    // set up funcCall args
    std::vector<AstNodePtr> ast_args;
    if (entry) {
        ast_args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *)1));
    }
    else {
        ast_args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *)0));
    }
    ast_args.back()->setType(BPatch::bpatch->type_Untyped);

    ast_args.push_back(AstNode::actualAddrNode());
    ast_args.push_back(idNode);
    ast_args.push_back(icNode);
    ast_args.push_back(calculation.ast_wrapper);

    // create func call & set type
    ast_wrapper = AstNodePtr(AstNode::funcCallNode("RThandleShadow", ast_args));
    ast_wrapper->setType(BPatch::bpatch->type_Untyped);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
}

BPatch_originalAddressExpr::BPatch_originalAddressExpr() {
    ast_wrapper = AstNodePtr(AstNode::originalAddrNode());

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
    BPatch_type *type = BPatch::bpatch->stdTypes->findType("long");
    assert(type != NULL);
    ast_wrapper->setType(type);
}

BPatch_actualAddressExpr::BPatch_actualAddressExpr() {
    ast_wrapper = AstNodePtr(AstNode::actualAddrNode());

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
    BPatch_type *type = BPatch::bpatch->stdTypes->findType("long");
    assert(type != NULL);
    ast_wrapper->setType(type);
}

BPatch_dynamicTargetExpr::BPatch_dynamicTargetExpr() {
    ast_wrapper = AstNodePtr(AstNode::dynamicTargetNode());

    assert(BPatch::bpatch != NULL);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
    BPatch_type *type = BPatch::bpatch->stdTypes->findType("long");
    assert(type != NULL);
    ast_wrapper->setType(type);
}

BPatch_scrambleRegistersExpr::BPatch_scrambleRegistersExpr(){


    ast_wrapper = AstNodePtr(AstNode::scrambleRegistersNode());
    ast_wrapper->setType(BPatch::bpatch->type_Untyped);
    ast_wrapper->setTypeChecking(BPatch::bpatch->isTypeChecked());
   
}

// Conversions
Dyninst::PatchAPI::Snippet::Ptr Dyninst::PatchAPI::convert(const BPatch_snippet *snip) {
   // TODO when this class exists
   return snip->ast_wrapper;
}

bool BPatch_snippet::checkTypesAtPoint(BPatch_point* p) const
{
  if(!p) return true;
  
  return ast_wrapper->checkType(p->getFunction()) != BPatch::bpatch->type_Error;
}

