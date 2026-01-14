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
#include <set>

#include "Absloc.h"
#include "DynAST.h"
#include "Graph.h"
#include "Instruction.h"
#include "dyninst_visibility.h"

namespace Dyninst {

class SliceNode;

namespace DataflowAPI {

// compare assignment shared pointers by value.
typedef std::map<Assignment::Ptr, AST::Ptr, AssignmentPtrValueComp> Result_t;

class SymEvalPolicy;

class  SymEval {
public:
    typedef boost::shared_ptr<SliceNode> SliceNodePtr;
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
    DYNINST_EXPORT static std::pair<AST::Ptr, bool> expand(const Assignment::Ptr &assignment, bool applyVisitors = true);

  // Hand in a set of Assignments
  // get back a map of Assignments->ASTs
  // We assume the assignments are prepped in the input; whatever
  // they point to is discarded.
  DYNINST_EXPORT static bool expand(Result_t &res, 
                                     std::set<InstructionAPI::Instruction> &failedInsns,
                                     bool applyVisitors = true);

  // Hand in a Graph (of SliceNodes, natch) and get back a Result;
  // prior results from the Graph
  // are substituted into anything that uses them.
  DYNINST_EXPORT static Retval_t expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t &res);
};

}
}

#endif
