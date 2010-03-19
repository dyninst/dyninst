/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// Stubs for now

#if !defined(SymEval_h)
#define SymEval_h

#include <map>

#include "Instruction.h"
#include "BinaryFunction.h"
#include "Dereference.h"
#include "Immediate.h"
#include "Absloc.h"
#include "AST.h"

#include "slicing.h"

#include "external/rose/rose-compat.h"

#include "Graph.h"

class SgAsmx86Instruction;
class SgAsmExpression;

namespace Dyninst {
namespace SymbolicEvaluation {

// The ROSE symbolic evaluation engine wants a data type that
// is template parametrized on the number of bits in the data
// type. However, our ASTs don't have this, and a shared_ptr
// to an AST _definitely_ doesn't have it. Instead, we use
// a wrapper class (Handle) that is parametrized appropriately
// and contains a shared pointer. 

// This uses a pointer to a shared pointer. This is ordinarily a really
// bad idea, but stripping the pointer part makes the compiler allocate
// all available memory and crash. No idea why. 

// Define the operations used by ROSE


struct Constant {

  Constant(uint64_t v) : val(v), size(0) {};
  Constant(uint64_t v, size_t s) : val(v), size(s) {};

  bool operator==(const Constant &rhs) const {
    return ((rhs.val == val) && (rhs.size == size));
  }

  const std::string format() const {
    std::stringstream ret;
    ret << val;
    if (size) {
      ret << ":" << size;
    }
    return ret.str();
  }

  uint64_t val;
  size_t size;
};

// Define the operations used by ROSE

struct ROSEOperation {
  typedef enum {
    nullOp,
    extractOp,
    invertOp,
    negateOp,
    signExtendOp,
    equalToZeroOp,
    generateMaskOp,
    LSBSetOp,
    MSBSetOp,
    concatOp,
    andOp,
    orOp,
    xorOp,
    addOp,
    rotateLOp,
    rotateROp,
    shiftLOp,
    shiftROp,
    shiftRArithOp,
    derefOp,
    writeRepOp,
    writeOp,
    ifOp,
    sMultOp,
    uMultOp,
    sDivOp,
    sModOp,
    uDivOp,
    uModOp,
    extendOp,
    extendMSBOp
  } Op;

  ROSEOperation(Op o) : op(o), size(0) {};
  ROSEOperation(Op o, size_t s) : op(o), size(s) {};

  bool operator==(const ROSEOperation &rhs) const {
    return ((rhs.op == op) && (rhs.size == size));
  }

  const std::string format() const {
    std::stringstream ret;
    ret << "<";
    switch(op) {
    case nullOp:
      ret << "null";
      break;
    case extractOp:
      ret << "extract";
      break;
    case invertOp:
      ret << "invert";
      break;
    case negateOp:
      ret << "negate";
      break;
    case signExtendOp:
      ret << "signExtend";
      break;
    case equalToZeroOp:
      ret << "eqZero?";
      break;
    case generateMaskOp:
      ret << "genMask";
      break;
    case LSBSetOp:
      ret << "LSB?";
      break;
    case MSBSetOp:
      ret << "MSB?";
      break;
    case concatOp:
      ret << "concat";
      break;
    case andOp:
      ret << "and";
      break;
    case orOp:
      ret << "or";
      break;
    case xorOp:
      ret << "xor";
      break;
    case addOp:
      ret << "add";
      break;
    case rotateLOp:
      ret << "rotL";
      break;
    case rotateROp:
      ret << "rotR";
      break;
    case shiftLOp:
      ret << "shl";
      break;
    case shiftROp:
      ret << "shr";
      break;
    case shiftRArithOp:
      ret << "shrA";
      break;
    case derefOp:
      ret << "deref";
      break;
    case writeRepOp:
      ret << "writeRep";
      break;
    case writeOp:
      ret << "write";
      break;
    case ifOp:
      ret << "if";
      break;
    case sMultOp:
      ret << "sMult";
      break;
    case uMultOp:
      ret << "uMult";
      break;
    case sDivOp:
      ret << "sDiv";
      break;
    case sModOp:
      ret << "sMod";
      break;
    case uDivOp:
      ret << "uDiv";
      break;
    case uModOp:
      ret << "uMod";
      break;
    case extendOp:
      ret << "ext";
      break;
    case extendMSBOp:
      ret << "extMSB";
      break;
    default:
      ret << " ??? ";
      break;
    };
    if (size) {
      ret << ":" << size;
    }
    ret << ">";
    return ret.str();
  };

  Op op;
  size_t size;
};

};

};

// Get this out of the Dyninst namespace...
std::ostream &operator<<(std::ostream &os, const Dyninst::SymbolicEvaluation::ROSEOperation &o);
std::ostream &operator<<(std::ostream &os, const Dyninst::SymbolicEvaluation::Constant &o);

namespace Dyninst {

namespace SymbolicEvaluation {

DEF_AST_LEAF_TYPE(BottomAST, bool);
DEF_AST_LEAF_TYPE(ConstantAST, Constant);
DEF_AST_LEAF_TYPE(AbsRegionAST, AbsRegion);
DEF_AST_INTERNAL_TYPE(RoseAST, ROSEOperation);

class SymEval {
 public:
  // Return type: mapping AbsRegions to ASTs
  // We then can map Assignment::AbsRegions to 
  // SymEval::AbsRegions and come up with the answer

  typedef std::map<Assignment::Ptr, AST::Ptr> Result;
  static const AST::Ptr Placeholder;
  
  // Single version: hand in an Assignment, get an AST
  static AST::Ptr expand(const Assignment::Ptr &assignment);

  // Hand in a set of Assignments
  // get back a map of Assignments->ASTs
  // We assume the assignments are prepped in the input; whatever
  // they point to is discarded.
  static void expand(Result &res);

  // Hand in a Graph (of AssignNodes, natch) and get back a Result;
  // prior results from the Graph
  // are substituted into anything that uses them.
  static void expand(Graph::Ptr slice, Result &res);
  
 private:
  static void process(AssignNode::Ptr, SymEval::Result &res);

  static SgAsmx86Instruction convert(const InstructionAPI::Instruction::Ptr &insn, uint64_t addr);
  static X86InstructionKind convert(entryID opcode);
  static SgAsmExpression *convert(const InstructionAPI::Operand &operand);
  static SgAsmExpression *convert(const InstructionAPI::Expression::Ptr expression);

  // Symbolically evaluate an instruction and assign 
  // an AST representation to every written absloc
  static void expandInsn(const InstructionAPI::Instruction::Ptr insn,
			 const uint64_t addr,
			 Result &res);
  
  friend class ExpressionConversionVisitor;
};

class ExpressionConversionVisitor : public InstructionAPI::Visitor {
 public:
    ExpressionConversionVisitor() { roseExpression = NULL; }

    SgAsmExpression *getRoseExpression() { return roseExpression; }

    virtual void visit(InstructionAPI::BinaryFunction *binfunc);
    virtual void visit(InstructionAPI::Immediate *immed);
    virtual void visit(InstructionAPI::RegisterAST *regast);
    virtual void visit(InstructionAPI::Dereference *deref);

 private:
    SgAsmExpression *roseExpression;
};

};
};

#endif
