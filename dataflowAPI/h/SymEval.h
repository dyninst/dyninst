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

// Stubs for now

#if !defined(SymEval_h)
#define SymEval_h

#include <map>
#include <ostream>
#include <sstream>
#include <set>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <utility>

#include "Absloc.h"
#include "DynAST.h"

#include "Graph.h"
#include "util.h"
#include "Node.h"
#include "Edge.h"

class SgAsmx86Instruction;
class SgAsmExpression;
class SgAsmPowerpcInstruction;
class SgAsmOperandList;
class SgAsmx86RegisterReferenceExpression;
class SgAsmPowerpcRegisterReferenceExpression;

namespace Dyninst {

   namespace ParseAPI {
      class Function;
      class Block;
   }

namespace DataflowAPI {

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

struct Variable {
  DATAFLOW_EXPORT Variable() : reg(), addr(0) {}
  DATAFLOW_EXPORT Variable(AbsRegion r) : reg(r), addr(0) {}
  DATAFLOW_EXPORT Variable(AbsRegion r, Address a) : reg(r), addr(a) {}

  DATAFLOW_EXPORT bool operator==(const Variable &rhs) const { 
    return ((rhs.addr == addr) && (rhs.reg == reg));
  }

  DATAFLOW_EXPORT bool operator<(const Variable &rhs) const { 
    if (addr < rhs.addr) return true;
    if (reg < rhs.reg) return true;
    return false;
  }

  DATAFLOW_EXPORT const std::string format() const {
    std::stringstream ret;
    ret << "V(" << reg;
    if (addr) ret << ":" << std::hex << addr << std::dec;
    ret << ")";
    return ret.str();
  }
    friend std::ostream& operator<<(std::ostream& stream, const Variable& c)
    {
        stream << c.format();
        return stream;
    }

   AbsRegion reg;
   Address addr;
};

struct Constant {
  DATAFLOW_EXPORT Constant() : val(0), size(0) {}
  DATAFLOW_EXPORT Constant(uint64_t v) : val(v), size(0) {}
  DATAFLOW_EXPORT Constant(uint64_t v, size_t s) : val(v), size(s) {}

 DATAFLOW_EXPORT  bool operator==(const Constant &rhs) const {
    return ((rhs.val == val) && (rhs.size == size));
  }

  DATAFLOW_EXPORT bool operator<(const Constant &rhs) const {
    if (val < rhs.val) return true;
    if (size < rhs.size) return true;
    return false;
  }

  DATAFLOW_EXPORT const std::string format() const {
    std::stringstream ret;
    ret << val;
    if (size) {
    ret << ":" << size;
    }
    return ret.str();
  }
friend std::ostream& operator<<(std::ostream& stream, const Constant& c)
{
    stream << c.format();
    return stream;
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

DATAFLOW_EXPORT ROSEOperation(Op o) : op(o), size(0) {}
DATAFLOW_EXPORT ROSEOperation(Op o, size_t s) : op(o), size(s) {}

DATAFLOW_EXPORT bool operator==(const ROSEOperation &rhs) const {
    return ((rhs.op == op) && (rhs.size == size));
}

DATAFLOW_EXPORT const std::string format() const {
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
}
    friend std::ostream& operator<<(std::ostream& stream, const ROSEOperation& c)
    {
        stream << c.format();
        return stream;
    }

Op op;
size_t size;
};

}

}


namespace Dyninst {

namespace InstructionAPI {
  class Instruction;
}

class SliceNode;

namespace DataflowAPI {

// compare assignment shared pointers by value.
typedef std::map<Assignment::Ptr, AST::Ptr, AssignmentPtrValueComp> Result_t;
    
DEF_AST_LEAF_TYPE(BottomAST, bool);
DEF_AST_LEAF_TYPE(ConstantAST, Constant);
DEF_AST_LEAF_TYPE(VariableAST, Variable);
DEF_AST_INTERNAL_TYPE(RoseAST, ROSEOperation);

class SymEvalPolicy;

class  SymEval {
public:
    typedef boost::shared_ptr<SliceNode> SliceNodePtr;
    typedef boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;
public:
  typedef enum {
     FAILED,
     WIDEN_NODE,
     FAILED_TRANSLATION,
     SKIPPED_INPUT,
     SUCCESS } Retval_t;

  // Return type: mapping AbsRegions to ASTs
  // We then can map Assignment::AbsRegions to 
  // SymEval::AbsRegions and come up with the answer
  // static const AST::Ptr Placeholder;
  //
  // Single version: hand in an Assignment, get an AST
    DATAFLOW_EXPORT static std::pair<AST::Ptr, bool> expand(const Assignment::Ptr &assignment, bool applyVisitors = true);

  // Hand in a set of Assignments
  // get back a map of Assignments->ASTs
  // We assume the assignments are prepped in the input; whatever
  // they point to is discarded.
  DATAFLOW_EXPORT static bool expand(Result_t &res, 
                                     std::set<InstructionAPI::Instruction> &failedInsns,
                                     bool applyVisitors = true);

  // Hand in a Graph (of SliceNodes, natch) and get back a Result;
  // prior results from the Graph
  // are substituted into anything that uses them.
  DATAFLOW_EXPORT static Retval_t expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t &res);
  
 private:

  // Symbolically evaluate an instruction and assign 
  // an AST representation to every written absloc
 static bool expandInsn(const InstructionAPI::Instruction &insn,
                        const uint64_t addr,
                        Result_t &res);

 static Retval_t process(SliceNodePtr ptr, Result_t &dbase, std::set<Edge::Ptr> &skipEdges);
  
 static AST::Ptr simplifyStack(AST::Ptr ast, Address addr, ParseAPI::Function *func, ParseAPI::Block *block);
};

}
}

#endif
