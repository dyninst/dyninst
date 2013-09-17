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
#include "common/h/util.h"
#include "StackTamperVisitor.h"
#include "dataflowAPI/h/slicing.h"
#include "dataflowAPI/h/stackanalysis.h"
#include "instructionAPI/h/Instruction.h"
#include "CFG.h"

using namespace std;
using namespace Dyninst;
using namespace DataflowAPI;
using namespace ParseAPI;

StackTamperVisitor::StackTamperVisitor(const Absloc &retaddr) :
  tamper_(TAMPER_UNSET),
  modpc_(0),
  origRetAddr_(retaddr)
{
}

AST::Ptr StackTamperVisitor::visit(AST *a) {
  // Should never be able to get this
  tamper_ = TAMPER_NONZERO;
  return a->ptr();
}

AST::Ptr StackTamperVisitor::visit(BottomAST *b) {
  tamper_ = TAMPER_NONZERO;
  return b->ptr();
}

AST::Ptr StackTamperVisitor::visit(ConstantAST *c) {
  diffs_.push(DiffVar((int)c->val().val, 0));
  return c->ptr();
}

AST::Ptr StackTamperVisitor::visit(VariableAST *v) {
  const AbsRegion &reg = v->val().reg;
  const Absloc &aloc = reg.absloc();

  if (aloc == origRetAddr_) {
      diffs_.push(DiffVar(0, 1));
  }
  else {
      tamper_ = TAMPER_NONZERO;
  }

  return v->ptr();
}

AST::Ptr StackTamperVisitor::visit(StackAST *s) {
  // This is a special case of the above; a StackAST is esp with
  // a known (analyzed) value. 
  if (s->val().isBottom()) {
    tamper_ = TAMPER_NONZERO;
  }
  else {
    diffs_.push(DiffVar(s->val().height(), 0));
  }
  return s->ptr();
}

AST::Ptr StackTamperVisitor::visit(RoseAST *r) {
  // Abort (ish) if we're done
  if (tamper_ != TAMPER_UNSET) return r->ptr();
  
  // Simplify children to the stack. 
  // Discard the pointers because we really don't care.
  // Go backwards so that the stack order matches the child order.
  // (that is, child 1 on top)
  for (unsigned i = r->numChildren(); i > 0; --i) {
    r->child(i-1)->accept(this);
  }
  // return immediately if we're done
  if (tamper_ != TAMPER_UNSET) return r->ptr();

  // Okay, let's see what's goin' on...
  switch(r->val().op) {
  case ROSEOperation::derefOp: {
    // A dereference is a decision point: either we're externally
    // sensitive (if the calculated difference depends on the PC at all)
    // or we reset the difference to 0.
    if (diffs_.top().b != 0) {
      tamper_ = TAMPER_NONZERO;
    }
    // Ignore the other entries... might be conditional loads, etc.
    for (unsigned i = 0; i < r->numChildren(); i++) {
      diffs_.pop();
    }
    // A non-modified dereference resets our "what's the difference" to 0. 
    diffs_.push(DiffVar(0, 0));
    break;
  }
  case ROSEOperation::addOp: {
    DiffVar sum(0,0);
    for (unsigned i = 0; i < r->numChildren(); ++i) {
      sum += diffs_.top(); diffs_.pop();
    }
    diffs_.push(sum);
    break;
  }
  case ROSEOperation::invertOp: {
    diffs_.top() *= -1;
    break;
  }
  case ROSEOperation::extendMSBOp:
  case ROSEOperation::extractOp: {
    DiffVar tmp = diffs_.top();
    for (unsigned i = 0; i < r->numChildren(); ++i) {
      diffs_.pop();
    }
    diffs_.push(tmp);
    break;
  }
  default:
    for (unsigned i = 0; i < r->numChildren(); i++) {
      if (diffs_.top().b != 0) {
    	tamper_ = TAMPER_NONZERO;
      }
      diffs_.pop();
    }
    diffs_.push(DiffVar(0, 0));
    break;
  }
  return r->ptr();
}

StackTamper StackTamperVisitor::tampersStack(AST::Ptr a, Address &newAddr) {

    if(tamper_ != TAMPER_UNSET) {
        newAddr = modpc_;
        return tamper_;
    }

    a->accept(this);

    if (tamper_ == TAMPER_NONZERO) {
        return tamper_;
    }

    assert(diffs_.size() == 1);

    modpc_ = diffs_.top().a.x;

    switch(diffs_.top().b.x) {
    case 0:
        tamper_ = TAMPER_ABS;
        break;
    case 1:
        if (modpc_) {
            tamper_ = TAMPER_REL;
        } else {
            tamper_ = TAMPER_NONE;
        }
        break;
    default:
        tamper_ = TAMPER_NONZERO;
        break;
    }
    newAddr = modpc_;
    return tamper_;
}

