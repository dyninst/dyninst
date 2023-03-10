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

#if !defined(_SYM_EVAL_VISITORS_H_)
#define _SYM_EVAL_VISITORS_H_

#include "../h/stackanalysis.h"
#include "common/h/DynAST.h"
#include "dataflowAPI/h/Absloc.h"
#include "SymEvalPolicy.h"
#include <string>

// A collection of visitors for SymEval AST classes

namespace Dyninst {

namespace DataflowAPI {

class StackVisitor : public ASTVisitor {
 public:
  StackVisitor(Address a,
	       ParseAPI::Function *func,
	       StackAnalysis::Height &stackHeight,
	       StackAnalysis::Height &frameHeight) :
    addr_(a), func_(func), stack_(stackHeight), frame_(frameHeight) {}

    DATAFLOW_EXPORT virtual AST::Ptr visit(AST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(BottomAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(ConstantAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(VariableAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(RoseAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(StackAST *);
    DATAFLOW_EXPORT virtual ASTPtr visit(InputVariableAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(ReferenceAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(StpAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(YicesAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(SemanticsAST *) {return AST::Ptr();}

    DATAFLOW_EXPORT virtual ~StackVisitor() {}

  private:
  Address addr_;
  ParseAPI::Function *func_;
  StackAnalysis::Height stack_;
  StackAnalysis::Height frame_;
};

  // Simplify boolean expressions for PPC
class BooleanVisitor : public ASTVisitor {
 public:
    BooleanVisitor() {}

    DATAFLOW_EXPORT virtual AST::Ptr visit(AST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(BottomAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(ConstantAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(VariableAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(RoseAST *);
    DATAFLOW_EXPORT virtual AST::Ptr visit(StackAST *);
    DATAFLOW_EXPORT virtual ASTPtr visit(InputVariableAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(ReferenceAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(StpAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(YicesAST *) {return AST::Ptr();}
    DATAFLOW_EXPORT virtual ASTPtr visit(SemanticsAST *) {return AST::Ptr();}

  
    DATAFLOW_EXPORT virtual ~BooleanVisitor() {}
    
  private:
};

}
}

#endif
