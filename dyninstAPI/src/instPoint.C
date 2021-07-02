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

// $Id: instPoint.C,v 1.55 2008/09/08 16:44:03 bernat Exp $
// instPoint code


#include <assert.h>
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/dynThread.h"

#include "instructionAPI/h/InstructionDecoder.h"
#include "Location.h"
#include "liveness.h"
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::ParseAPI;

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/parse-cfg.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emitter.h"
#if defined(arch_x86_64)
// For 32/64-bit mode knowledge
#include "dyninstAPI/src/emit-x86.h"
#endif

using namespace Dyninst;
using namespace PatchAPI;

instPoint *instPoint::funcEntry(func_instance *f) {
  return f->funcEntryPoint(true);
}

instPoint *instPoint::funcExit(func_instance *f, block_instance *b) {
  return f->funcExitPoint(b, true);
}

instPoint *instPoint::blockEntry(func_instance *f, block_instance *b) {
  return f->blockEntryPoint(b, true);
}

instPoint *instPoint::blockExit(func_instance *f, block_instance *b) {
  return f->blockExitPoint(b, true);
}

instPoint *instPoint::preCall(func_instance *f, block_instance *b) {
  return f->preCallPoint(b, true);
}

instPoint *instPoint::postCall(func_instance *f, block_instance *b) {
  return f->postCallPoint(b, true);
}

instPoint *instPoint::edge(func_instance *f, edge_instance *e) {
  //   return f->findPoint(EdgeDuring, e, true);
  return f->edgePoint(e, true);
}

instPoint *instPoint::preInsn(func_instance *f,
                              block_instance *b,
                              Address a,
                              Instruction insn,
                              bool trusted) {
  return f->preInsnPoint(b, a, insn, trusted, true);
}

instPoint *instPoint::postInsn(func_instance *f,
                               block_instance *b,
                               Address a,
                               Instruction insn,
                               bool trusted) {
  return f->postInsnPoint(b, a, insn, trusted, true);
}

instPoint::instPoint(Type t, 
                     PatchMgrPtr mgr,
                     func_instance *f) :
   Point(t, mgr, f),
   baseTramp_(NULL) {
}

instPoint::instPoint(Type          t,
                     PatchMgrPtr   mgr,
                     func_instance *f,
                     block_instance *b) :
  Point(t, mgr, f, b),
  baseTramp_(NULL) {
}

instPoint::instPoint(Type          t,
                     PatchMgrPtr   mgr,
                     block_instance *b,
                     func_instance *f) :
  Point(t, mgr, b, f),
  baseTramp_(NULL) {
}

instPoint::instPoint(Type t,
                     PatchMgrPtr mgr,
                     block_instance *b,
                     Address a,
                     Instruction i,
                     func_instance *f) :
  Point(t, mgr, b, a, i, f),
  baseTramp_(NULL) {
}

instPoint::instPoint(Type          t,
                     PatchMgrPtr   mgr,
                     edge_instance *e,
                     func_instance *f) :
  Point(t, mgr, e, f),
  baseTramp_(NULL) {
}


// If there is a logical "pair" (e.g., before/after) of instPoints return them.
// The return result is a pair of <before, after>
std::pair<instPoint *, instPoint *> instPoint::getInstpointPair(instPoint *i) {
   switch(i->type()) {
      case None:
         assert(0);
         return std::pair<instPoint *, instPoint *>((instPoint*)NULL, (instPoint*)NULL);
      case PreInsn:
         return std::pair<instPoint *, instPoint *>(i,
                                                    postInsn(i->func(),
                                                             i->block(),
                                                             i->insnAddr(),
                                                             i->insn(),
                                                             true));
      case PostInsn:
         return std::pair<instPoint *, instPoint *>(preInsn(i->func(),
                                                            i->block(),
                                                            i->insnAddr(),
                                                            i->insn(),
                                                            true),
                                                    i);
      case PreCall:
         return std::pair<instPoint *, instPoint *>(i,
                                                    postCall(i->func(),
                                                             i->block()));
      case PostCall:
         return std::pair<instPoint *, instPoint *>(preCall(i->func(),
                                                            i->block()),
                                                    i);
      default:
         return std::pair<instPoint *, instPoint *>(i, (instPoint*)NULL);
   }
   assert(0);
   return std::pair<instPoint *, instPoint *>((instPoint*)NULL, (instPoint*)NULL);
}

instPoint *instPoint::fork(instPoint *parent, AddressSpace *child) {
   // Return the equivalent instPoint within the child process
  func_instance *f = parent->func() ? child->findFunction(parent->func()->ifunc()) : NULL;
  block_instance *b = parent->block() ? child->findBlock(parent->block()->llb()) : NULL;
  edge_instance *e = parent->edge() ? child->findEdge(parent->edge()->edge()) : NULL;
   Instruction i = parent->insn_;
   Address a = parent->addr_;

   instPoint *point = NULL;

   switch(parent->type_) {
      case None:
         assert(0);
         break;
      case FuncEntry:
         point = funcEntry(f);
         break;
      case FuncExit:
         point = funcExit(f, b);
         break;
      case BlockEntry:
         point = blockEntry(f, b);
         break;
      case BlockExit:
         point = blockExit(f, b);
         break;
      case EdgeDuring:
         point = edge(f, e);
         break;
      case PreInsn:
         point = preInsn(f, b, a, i, true);
         break;
      case PostInsn:
         point = postInsn(f, b, a, i, true);
         break;
      case PreCall:
         point = preCall(f, b);
         break;
      case PostCall:
         point = postCall(f, b);
         break;
      case OtherPoint:
         //case InsnTaken:
   case BlockDuring:
   case FuncDuring:
   case LoopStart:
   case LoopEnd:
   case LoopIterStart:
   case LoopIterEnd:
   case InsnTypes:
   case BlockTypes:
   case FuncTypes:
   case LoopTypes:
   case CallTypes:
         assert(0);
         break;
   }
   assert(point->empty() ||
          point->size() == parent->size());
   if (point->empty()) {
      for (instance_iter iter = parent->begin(); iter != parent->end(); ++iter) {
         InstancePtr inst = point->pushBack((*iter)->snippet());
         if (!(*iter)->recursiveGuardEnabled()) {
            inst->disableRecursiveGuard();
         }
      }
   }

   point->liveRegs_ = parent->liveRegs_;

   return point;
}

instPoint::~instPoint() {
  // Delete miniTramps?
  // Uninstrument?
  //for (iterator iter = begin(); iter != end(); ++iter)
  //  delete *iter;
  if (baseTramp_) delete baseTramp_;
}


AddressSpace *instPoint::proc() const {
   if (func()) return func()->proc();
   else if (block()) return block()->proc();
   else if (edge()) return (edge()->proc());
   assert(0); return NULL;
}

func_instance *instPoint::func() const {
  return SCAST_FI(the_func_);
}

block_instance *instPoint::block() const {
  return SCAST_BI(the_block_);
}

edge_instance *instPoint::edge() const {
  return SCAST_EI(the_edge_);
}

bool instPoint::checkInsn(block_instance *b,
                          Instruction &insn,
                          Address a) {
   block_instance::Insns insns;
   b->getInsns(insns);
   block_instance::Insns::iterator iter = insns.find(a);
   if (iter != insns.end()) {
      insn = iter->second;
      return true;
   }
   return false;
}


baseTramp *instPoint::tramp() {
   if (!baseTramp_) {
      baseTramp_ = baseTramp::create(this);
   }

   return baseTramp_;
}

// Returns the current block (if there is one)
// or the next block we're going to execute (if not).
// In some cases we may not know; function exit points
// and the like. In this case we return the current block
// as a "well, this is what we've got..."
block_instance *instPoint::block_compat() const {
   switch (type_) {
      case FuncEntry:
	return func()->entryBlock();
      case EdgeDuring:
	return edge()->trg();
      case PreInsn:
      case PostInsn:
      case FuncExit:
      case BlockEntry:
      case BlockExit:
      case PreCall:
	return block();
      case PostCall: {
	edge_instance *ftE = block()->getFallthrough();
         if (ftE &&
             (!ftE->sinkEdge()))
            return ftE->trg();
         else
            return NULL;
      }
      default:
         return NULL;
   }
}

Address instPoint::addr_compat() const {
   // As the above, but our best guess at an address
   switch (type_) {
      case FuncEntry:
	return func()->addr();
      case FuncExit:
      case BlockExit:
         // Not correct, but as close as we can get
	return block()->last();
      case BlockEntry:
	return block()->start();
      case EdgeDuring:
	return edge()->trg()->start();
      case PreInsn:
         return addr_;
      case PostInsn:
         // This gets weird for things like jumps...
         return addr_ + insn_.size();
      case PreCall:
	return block()->last();
      case PostCall: {
	edge_instance *ftE = block()->getFallthrough();
         if (ftE &&
             (!ftE->sinkEdge()))
            return ftE->trg()->start();
         else
	   return block()->end();
      }
      default:
          mal_printf("ERROR: returning 0 from instPoint::nextExecutedAddr "
                  "for point of type %x in block [%lx %lx)\n", (unsigned int)type(),
                  block()->start(), block()->end());
         return 0;
   }
}

std::string instPoint::format() const {
   stringstream ret;
   ret << "iP(";
   switch(type_) {
      case FuncEntry:
         ret << "FEntry";
         break;
      case FuncExit:
         ret << "FExit";
         break;
      case BlockEntry:
         ret << "BEntry";
         break;
      case BlockExit:
         ret << "BExit";
         break;
      case EdgeDuring:
         ret << "E";
         break;
      case PreInsn:
         ret << "PreI";
         break;
      case PostInsn:
         ret << "PostI";
         break;
      case PreCall:
         ret << "PreC";
         break;
      case PostCall:
         ret << "PostC";
         break;
      default:
         ret << "???";
         break;
   }
   if (func()) {
     ret << ", Func(" << func()->name() << ")";
   }
   if (block()) {
     ret << ", Block(" << hex << block()->start() << dec << ")";
   }
   if (edge()) {
      ret << ", Edge";
   }
   if (addr_) {
      ret << ", Addr(" << hex << addr_ << dec << ")";
   }
   if (insn_.isValid()) {
      ret << ", Insn(" << insn_.format() << ")";
   }
   ret << ")";
   return ret.str();
}

Dyninst::PatchAPI::InstancePtr getChildInstance(Dyninst::PatchAPI::InstancePtr parentInstance,
                                                AddressSpace *childProc) {
   instPoint *pPoint = IPCONV(parentInstance->point());
   instPoint *cPoint = instPoint::fork(pPoint, childProc);
   // Find the equivalent miniTramp...
   assert(pPoint->size() == cPoint->size());
   instPoint::instance_iter c_iter = cPoint->begin();
   for (instPoint::instance_iter iter = pPoint->begin();
        iter != pPoint->end();
        ++iter) {
      if (*iter == parentInstance) return *c_iter;
      ++c_iter;
   }

   assert(0);
   return Dyninst::PatchAPI::InstancePtr();
}

InstancePtr instPoint::pushFront(SnippetPtr snip) {
   InstancePtr ret = Point::pushFront(snip);
   if (!ret) return ret;
   markModified();
   return ret;
}

InstancePtr instPoint::pushBack(SnippetPtr snip) {
   InstancePtr ret = Point::pushBack(snip);
   if (!ret) return ret;
   markModified();
   return ret;
}

void instPoint::markModified() {
   if (func()) {
      proc()->addModifiedFunction(func());
   }
   else if (block()) {
      proc()->addModifiedBlock(block());
   }
   else if (edge()) {
      proc()->addModifiedBlock(edge()->src());
   }
   else {
      assert(0);
   }
}
         
bitArray instPoint::liveRegisters(){
	stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);
	static LivenessAnalyzer live1(4);
	static LivenessAnalyzer live2(8);
	LivenessAnalyzer *live;
	if (func()->function()->region()->getAddressWidth() == 4) live = &live1; else live = &live2;
	if (liveRegs_.size() && liveRegs_.size() == live->getABI()->getAllRegs().size()){
		return liveRegs_;
	}	
	switch(type()) {
		case FuncEntry:
			if (!live->query(ParseAPI::Location(EntrySite(func()->function(), func()->function()->entry())), LivenessAnalyzer::Before, liveRegs_)) assert(0);
			break;
		case BlockEntry:
			if (!live->query(ParseAPI::Location(BlockSite(func()->function(), block()->block())), LivenessAnalyzer::Before, liveRegs_)) assert(0);
			break;
		case BlockExit:
			if (!live->query(ParseAPI::Location(BlockSite(func()->function(), block()->block())), LivenessAnalyzer::After, liveRegs_)) assert(0);
			break;
		case EdgeDuring:
			if (!live->query(ParseAPI::Location(EdgeLoc(func()->function(), edge()->edge())), LivenessAnalyzer::After, liveRegs_)) assert(0);
			break;
		case FuncExit:
			if (!live->query(ParseAPI::Location(ExitSite(func()->function(), block()->block())), LivenessAnalyzer::After, liveRegs_)) assert(0);
			break;
		case PostCall:
			if (!live->query(ParseAPI::Location(CallSite(func()->function(), block()->block())), LivenessAnalyzer::After, liveRegs_)) assert(0);
			break;
		case PreCall:
			if (!live->query(ParseAPI::Location(CallSite(func()->function(), block()->block())), LivenessAnalyzer::Before, liveRegs_)) assert(0);
			break;
		case PreInsn:
			if (!live->query(ParseAPI::Location(func()->function(), InsnLoc(block()->block(), insnAddr() - func()->obj()->codeBase(), insn())), LivenessAnalyzer::Before, liveRegs_)) assert(0);
			break;
		case PostInsn:
		        if (!live->query(ParseAPI::Location(func()->function(), InsnLoc(block()->block(), insnAddr() - func()->obj()->codeBase(), insn())), LivenessAnalyzer::After, liveRegs_)) assert(0);
			break;
		default:
			assert(0);  
	}
	stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
	return liveRegs_;

}
