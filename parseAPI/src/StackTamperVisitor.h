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



#if !defined(_STACK_TAMPER_VISITOR_H_)
#define _STACK_TAMPER_VISITOR_H_

#include <map>
#include <stack>
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/AbslocInterface.h" // And more of the same
#include "dataflowAPI/h/SymEval.h" // Variable class
#include "DynAST.h"
#include "parseAPI/h/CFG.h"
#include "common/src/LinearVariable.h"

// A representation of a variable x = x + var1 + var2 + var3 + ...
// where int is an integer and var1...varN are unknown variables.

namespace Dyninst {

typedef enum {
  NotRequired,
  Suggested,
  Required,
  OffLimits,
  MaxPriority } Priority;
 

// ROSE symeval AST types
namespace SymbolicEvaluation {
 class BottomAST;
 class ConstantAST;
 class AbsRegionAST;
 class RoseAST;
 class StackAST;
 }

class StackTamperVisitor : public ASTVisitor {
 public:
  StackTamperVisitor(const Absloc &);

  virtual AST::Ptr visit(AST *);
  virtual AST::Ptr visit(DataflowAPI::BottomAST *);
  virtual AST::Ptr visit(DataflowAPI::ConstantAST *);
  virtual AST::Ptr visit(DataflowAPI::VariableAST *);
  virtual AST::Ptr visit(DataflowAPI::RoseAST *);
  virtual AST::Ptr visit(DataflowAPI::StackAST *);
  virtual ~StackTamperVisitor() {}
  
  ParseAPI::StackTamper tampersStack(AST::Ptr a, Address &modAddr);

 private:
  ParseAPI::StackTamper tamper_;
  Address modpc_;
  Absloc origRetAddr_;

  typedef linVar<DataflowAPI::Variable> DiffVar;
  std::stack<DiffVar > diffs_;
};

}
#endif

