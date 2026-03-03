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
 
// $Id: parse-cfg.C,v 1.60 2008/11/03 15:19:24 jaw Exp $

#include "function.h"
#include "instPoint.h"

#include "instructionAPI/h/InstructionDecoder.h"

#include "image.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

parse_func::parse_func(
    SymtabAPI::Function *func, 
    pdmodule *m, 
    image *i, 
    CodeObject * obj,
    CodeRegion * reg,
    InstructionSource * isrc,
    FuncSource src):
  Function(func->getOffset(),func->getFirstSymbol()->getMangledName(),obj,reg,isrc),
  func_(func),
  mod_(m),
  image_(i),
  hasWeirdInsns_(false),
  prevBlocksUnresolvedCF_(0),
  unresolvedCF_(UNSET_CF),
  init_retstatus_(UNSET),
  saves_return_addr_(false),
  isPLTFunction_(false),
  containsPowerPreamble_(false),
  noPowerPreambleFunc_(NULL)
{
    _src = src;
    func->setData(this);
}	


parse_func::~parse_func() 
{
    /* FIXME */ 
  mal_printf("~image_func() for func at %lx\n",_start);
}

bool parse_func::addSymTabName(std::string name, bool isPrimary) 
{
    if(func_->addMangledName(name.c_str(), isPrimary)){
	return true;
    }

    return false;
}

bool parse_func::addPrettyName(std::string name, bool isPrimary) {
   if (func_->addPrettyName(name.c_str(), isPrimary)) {
      return true;
   }
   
   return false;
}

bool parse_func::addTypedName(std::string name, bool isPrimary) {
    // Count this as a pretty name in function lookup...
    if (func_->addTypedName(name.c_str(), isPrimary)) {
	return true;
    }

    return false;
}

bool parse_func::isInstrumentableByFunctionName()
{
    // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
    // they can't be instrumented or we would have an infinite loop
    if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
        || prettyName() == "GetProcessTimes")
        return false;
    return true;
}

Address parse_func::getEndOffset() {
    if (!parsed()) image_->analyzeIfNeeded();
    if(blocks().empty()) {
        fprintf(stderr,"error: end offset requested for empty function\n");
        return addr();
    } else {
        return extents().back()->end();
    }
}

bool parse_func::isPLTFunction() {
    if(isPLTFunction_) {
      return true;
    }
    return obj()->cs()->linkage().find(addr()) !=
           obj()->cs()->linkage().end();
}

void *parse_func::getPtrToInstruction(Address addr) const {
    return isrc()->getPtrToInstruction(addr);
}

bool parse_func::isLeafFunc() {
    if (!parsed())
        image_->analyzeIfNeeded();

    return callEdges().empty();
}

void parse_func::setinit_retstatus(ParseAPI::FuncReturnStatus rs)
{
    init_retstatus_ = rs;
    if (rs > retstatus()) {
        set_retstatus(rs);
    }
}
ParseAPI::FuncReturnStatus parse_func::init_retstatus() const
{
    if (UNSET == init_retstatus_) {
        assert(!obj()->defensiveMode()); // should have been set for defensive binaries
        return retstatus();
    }
    if (init_retstatus_ > retstatus()) {
        return retstatus();
    }
    return init_retstatus_;
}

void parse_func::setHasWeirdInsns(bool wi)
{
   hasWeirdInsns_ = wi;
}

bool parse_func::hasUnresolvedCF() {
   if (unresolvedCF_ == UNSET_CF) {
      for (blocklist::iterator iter = blocks().begin();
           iter != blocks().end(); ++iter) {
         for (Block::edgelist::const_iterator iter2 = (*iter)->targets().begin();
              iter2 != (*iter)->targets().end(); ++iter2) {
	   if ((*iter2)->sinkEdge())
	   {
	     if ((*iter2)->interproc()) {
	       continue;
	     }
	     if (((*iter2)->type() == ParseAPI::INDIRECT) ||
		 ((*iter2)->type() == ParseAPI::DIRECT))
	     {
	       unresolvedCF_ = HAS_UNRESOLVED_CF;
	       break;
	     }
	   }
         }
         if (unresolvedCF_ == HAS_UNRESOLVED_CF) break;
      }
      if (unresolvedCF_ == UNSET_CF)
         unresolvedCF_ = NO_UNRESOLVED_CF;
   }
   return (unresolvedCF_ == HAS_UNRESOLVED_CF);
}

bool parse_func::isInstrumentable() {
   if(!isInstrumentableByFunctionName() || img()->isUnlinkedObjectFile())
      return false;
   else {
      // Create instrumentation points for non-plt functions 
      if(obj()->cs()->linkage().find(getOffset()) != obj()->cs()->linkage().end()) { 
          return false;
      }
    }

   if (hasUnresolvedCF()) {
      return false;
   }
   return true;
}

parse_func *parse_func::plt_func(
   Dyninst::SymtabAPI::Function *func,
   pdmodule *m,
   image *i,
   Dyninst::ParseAPI::CodeObject * obj,
   Dyninst::ParseAPI::CodeRegion * reg,
   Dyninst::InstructionSource * isrc,
   Dyninst::ParseAPI::FuncSource src) {
  auto *f = new parse_func(func, m, i, obj, reg, isrc, src);
  f->isPLTFunction_ = true;
  return f;
}
