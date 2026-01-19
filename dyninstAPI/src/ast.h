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

#ifndef DYNINST_DYNINSTAPI_AST_H
#define DYNINST_DYNINSTAPI_AST_H

#include "AstAddrNode.h"
#include "AstCallNode.h"
#include "AstMemoryNode.h"
#include "AstNode.h"
#include "AstNullNode.h"
#include "AstOperandNode.h"
#include "AstOperatorNode.h"
#include "AstSequenceNode.h"
#include "AstStackNode.h"
#include "AstStackGenericNode.h"
#include "AstStackInsertNode.h"
#include "AstStackRemoveNode.h"
#include "AstVariableNode.h"
#include "dyn_register.h"
#include "opcode.h"
#include "OperandType.h"
#include "Point.h"

#include <cassert>
#include <utility>
#include <vector>
#include <string>

class AddressSpace;
class BPatch_function;
class BPatch_snippet;
class BPatch_type;
class codeGen;
class instPoint;
class func_instance;
class image_variable;
class int_variable;

/* Stack Frame Modification */

class AstDynamicTargetNode : public AstNode {
 public:
    AstDynamicTargetNode() {}

    virtual ~AstDynamicTargetNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};
class AstScrambleRegistersNode : public AstNode {
 public:
    AstScrambleRegistersNode() {}

    virtual ~AstScrambleRegistersNode() {}

    virtual bool usesAppRegister() const { return true; }
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstSnippetNode : public AstNode {
   // This is a little odd, since an AstNode _is_
   // a Snippet. It's a compatibility interface to 
   // allow generic PatchAPI snippets to play nice
   // in our world. 
  public:
  AstSnippetNode(Dyninst::PatchAPI::SnippetPtr snip) : snip_(snip) {}
   
  private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    Dyninst::PatchAPI::SnippetPtr snip_;
};

class AstAtomicOperationStmtNode : public AstNode {
    // This corresponds to a single statement, and not an expression that can be nested among other
    // expressions.
  public:
    AstAtomicOperationStmtNode(opCode astOpcode, AstNodePtr variableNode, AstNodePtr constantNode);

    virtual std::string format(std::string indent);

    virtual bool canBeKept() const { return true; }

  private:
    virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    opCode opcode;
    AstNodePtr variable;
    AstNodePtr constant;
};




#endif /* AST_HDR */
