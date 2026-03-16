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

#ifndef DYNINST_DYNINSTAPI_CODEGENAST_H
#define DYNINST_DYNINSTAPI_CODEGENAST_H

#include "dyn_register.h"
#include "opcode.h"
#include "OperandType.h"
#include "Point.h"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

class AddressSpace;
class BPatch_function;
class BPatch_snippet;
class BPatch_type;
class codeGen;
class instPoint;
class func_instance;
class image_variable;
class registerSpace;

// Dyninst::Register retention mechanism...
// If we've already calculated a result, then we want to reuse it if it's
// still available. This means it was calculated along a path that reaches the
// current point (not inside a conditional) and the register hasn't been
// reused. We handle this so:
//
// 1) Iterate over the AST tree and see if any node is reached more than once;
// if so, mark it as potentially being worth keeping around. We can do this
// because we use pointers; a better approach would be a comparison operator.
// 2) Start generation at "level 0".
// 3) When a conditional AST is reached, generate each child at level+1.
// 4) When the AST is reached during code generation, and doesn't have a register:
// 4a) Allocate a register for it;
// 4b) Enter that register, the AST node, and the current level in the global table.
// 5) If it does have a register, reuse it.
// 6) When the conditionally executed branch is finished, clean all entries in
// the table with that level value (undoing all kept registers along that
// path).
// 7) If we need a register, the register allocator (registerSpace) can forcibly
// undo this optimization and grab a register. Grab the register from the codeGenAST
// with the lowest usage count.

namespace Dyninst { namespace DyninstAPI {

class codeGenAST;
typedef boost::shared_ptr<codeGenAST> codeGenASTPtr;

class codeGenAST : public Dyninst::PatchAPI::Snippet {
public:
  virtual std::string format(std::string indent);

  codeGenAST() = default;

  virtual ~codeGenAST() = default;

  virtual bool generateCode(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                            Dyninst::Register &retReg);

  virtual bool generateCode(codeGen &gen, bool noCost);

  virtual bool generateCode(codeGen &gen, bool noCost, Dyninst::Register &retReg) {
    Dyninst::Address unused = Dyninst::ADDR_NULL;
    return generateCode(gen, noCost, unused, retReg);
  }

  virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                                   Dyninst::Register &retReg) = 0;

  // Perform whatever pre-processing steps are necessary.
  virtual bool initRegisters(codeGen &gen);

  // Select the appropriate Variable AST as part of pre-processing
  // steps before code generation.
  virtual void setVariableAST(codeGen &g) {
    for(auto &&c : children) {
      c->setVariableAST(g);
    }
  }

  bool previousComputationValid(Dyninst::Register &reg, codeGen &gen);

  virtual codeGenASTPtr operand() const {
    return codeGenASTPtr();
  }

  virtual bool containsFuncCall() const {
    for(auto &&c : children) {
      if(c->containsFuncCall()) {
        return true;
      }
    }
    return false;
  }

  virtual bool usesAppRegister() const {
    for(auto &&c : children) {
      if(c->usesAppRegister()) {
        return true;
      }
    }
    return false;
  }

  int useCount{};     // Reference count for generating code
  void setUseCount(); // Set values for useCount

  int getSize() {
    return size;
  }

  void cleanUseCount();

  // Occasionally, we do not call .generateCode_phase2 for the
  // referenced node, but generate code by hand. This routine decrements
  // its use count properly
  void decUseCount(codeGen &gen);

  // Our children may have incorrect useCounts (most likely they
  // assume that we will not bother them again, which is wrong)
  void fixChildrenCounts();

  // Check if the node can be kept at all. Some nodes (e.g., storeOp)
  // can not be cached
  virtual bool canBeKept() const {
    return false;
  }

  // Allocate a register and make it available for sharing if our
  // node is shared
  Dyninst::Register allocateAndKeep(codeGen &gen, bool noCost);

  // Return all children of this node ([lre]operand, ..., operands[])
  std::vector<codeGenASTPtr> const &getChildren() const {
    return children;
  }

  virtual void setOValue(void *) {
    assert(0);
  }

  virtual const void *getOValue() const {
    assert(0);
    return NULL;
  }

  virtual const image_variable *getOVar() const {
    return NULL;
  }

  virtual void emitVariableStore(opCode, Dyninst::Register, Dyninst::Register, codeGen &, bool,
                                 registerSpace *, int, const instPoint *, AddressSpace *) {
    assert(!"Never call this on anything but an operand");
  }

  virtual void emitVariableLoad(opCode, Dyninst::Register, Dyninst::Register, codeGen &, bool,
                                registerSpace *, int, const instPoint *, AddressSpace *) {
    assert(!"Never call this on anything but an operand");
  }

  virtual operandType getoType() const {
    return operandType::undefOperandType;
  }

protected:
  BPatch_type *bptype{};  // type of corresponding BPatch_snippet
  bool doTypeCheck{true}; // should operands be type checked
  int size{4};            // size of the operations (in bytes)
  std::vector<codeGenASTPtr> children{};

public:
  // Functions for getting and setting type decoration used by the
  // dyninst API library
  BPatch_type *getType() {
    return bptype;
  }

  void setType(BPatch_type *t);

  void setTypeChecking(bool x) {
    doTypeCheck = x;
  }

  virtual BPatch_type *checkType(BPatch_function *func = NULL);

  // PatchAPI compatibility
  virtual bool generate(Dyninst::PatchAPI::Point *, Dyninst::Buffer &);
};

}}

#endif
