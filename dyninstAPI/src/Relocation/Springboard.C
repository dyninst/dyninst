/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "Springboard.h"
#include "dyninstAPI/src/addressSpace.h"

#include "function.h"
#include "debug.h"
#include "codegen.h"

using namespace Dyninst;
using namespace Relocation;

const int SpringboardBuilder::Allocated(0);
const int SpringboardBuilder::UnallocatedStart(1);

template <typename TraceIter> 
SpringboardBuilder::Ptr SpringboardBuilder::create(TraceIter begin,
						   TraceIter end,
						   AddressSpace *as) {
  Ptr ret = Ptr(new SpringboardBuilder(as));
  if (!ret) return ret;

  if (!ret->addTraces(begin, end)) return Ptr();
  return ret;
}

SpringboardBuilder::Ptr SpringboardBuilder::createFunc(FuncSet::const_iterator begin,
						       FuncSet::const_iterator end,
						       AddressSpace *as) {
  Ptr ret = Ptr(new SpringboardBuilder(as));
  if (!ret) return ret;
  for (; begin != end; ++begin) {
    int_function *func = *begin;
    if (!ret->addTraces(func->blocks().begin(), func->blocks().end())) {
      return Ptr();
    }
  }
  return ret;
}

bool SpringboardBuilder::generate(std::list<codeGen> &springboards,
				  const SpringboardMap &input) {
  // The SpringboardMap gives us a, well, mapping of desired
  // from: addresses and to: addresses. Our job is to create a series
  // of branches (and traps) that will take care of that. 

  // Currently we use a greedy algorithm rather than some sort of scheduling thing.
  // It's a heck of a lot easier that way. 

  for (SpringboardMap::const_iterator iter = input.begin(); 
       iter != input.end(); ++iter) {
    const SpringboardReq &req = iter->second;

    switch (generateSpringboard(springboards, req)) {
    case Failed:
      return false;
      break;
    case MultiNeeded:
      // We want to try some multi-step jump to get to the relocated code. 
      // We have to wait until all the primaries are done first; effectively
      // a greedy algorithm of largest first.
      multis_.push_back(req);
      break;
    case Succeeded:
      // Good!
      break;
    }
  }

  bool ret = true;

  for (std::list<SpringboardReq>::iterator iter = multis_.begin();
       iter != multis_.end(); ++iter) {
    if (!generateMultiSpringboard(springboards, *iter)) {
      if (iter->priority == Required) {
	cerr << "Failed required springboard @ " << hex << iter->from << endl;
	ret = false;
      }
      // Otherwise it was a suggested, no worries.
    }
  }
  
  return ret;
}

template <typename TraceIter>
bool SpringboardBuilder::addTraces(TraceIter begin, TraceIter end) {
  // TODO: map these addresses to relocated blocks as well so we 
  // can do our thang.
  for (; begin != end; ++begin) {
    bblInstance *bbl = (*begin)->origInstance();
    validRanges_.insert(bbl->firstInsnAddr(), bbl->endAddr(), curRange_++);
  }
  return true;
}


SpringboardBuilder::generateResult_t 
SpringboardBuilder::generateSpringboard(std::list<codeGen> &springboards,
					const SpringboardReq &r) {
  codeGen gen;

  bool usedTrap = false;

  generateBranch(r.from, r.to, gen);

  if (conflict(r.from, r.from + gen.used())) {
    return MultiNeeded;
#if 0
    // Errr...
    // Fine. Let's do the trap thing. 
    usedTrap = true;
    generateTrap(r.from, r.to, gen);
    if (conflict(r.from, r.from + gen.used())) {
      cerr << "Error: conflict in range " << hex << r.from << ".." << r.from+gen.used() << dec << endl;
      debugRanges();
      assert(0);
    }
#endif
  }

  registerBranch(r.from, r.from + gen.used());
  springboards.push_back(gen);

  // Now catch all the previously relocated copies.
  generateReplacements(springboards, r, usedTrap);

  return Succeeded;
}

bool SpringboardBuilder::generateMultiSpringboard(std::list<codeGen> &input,
						  const SpringboardReq &r) {
  cerr << "Request to generate multi-branch springboard skipped @ " << hex << r.from << dec << endl;
  // FIXME Override
  return true;

  // Much like the above, except try to do it in multiple steps. This works 
  // well on x86 where we have 2-byte dinky jumps. Less so on fixed-length
  // architectures...
  Address from = r.from;
  
  // Let's assume that r.from is not really available. Let's find somewhere that
  // is.
  // Let's try walking forward first. 
  Address tramp = r.from;
  bool done = false;
  codeGen gen;
  do {
    generateBranch(tramp, r.to, gen);
    if (!conflict(tramp, tramp+gen.used())) {
      // Cool
      done = true;
    }
    else {
      // FIXME POWER
      // What happened to the "legal offset for the platform" function?
      tramp++;
    }
    if (!isLegalShortBranch(r.from, tramp)) {
      // Start at -offset
      tramp = shortBranchBack(r.from);
    }
    if (tramp == (r.from - 1)) {
      break;
    }
  } while (!done);
  
  if (!done) return false; 

  // Okay, we've got a branch there. 
  input.push_back(gen);
  registerBranch(tramp, tramp + gen.used());
  
  // And catch its relocated copies. Argh.
  SpringboardReq tmp(tramp, r.to, r.priority);
  generateReplacements(input, tmp, false);

  // Okay. Now we need to get _to_ the tramp jump.
  codeGen shortie;
  generateBranch(r.from, tramp, shortie);
  if (conflict(r.from, r.from + shortie.used())) {
    // Sucks to be us
    return false;
  }
  
  input.push_back(shortie);
  registerBranch(r.from, r.from + shortie.used());

  SpringboardReq tmp2(r.from, tramp, r.priority);
  generateReplacements(input, tmp2, false);

  return true;
}

bool SpringboardBuilder::conflict(Address start, Address end) {
  // We require springboards to stay within a particular block
  // so that we don't have issues with jumping into the middle
  // of a branch (... ew). Therefore, there is a conflict if
  // the end address lies within a different range than the start.
  //
  // Technically, end can be the start of another range; therefore
  // we search for (end-1).

  Address startLB, startUB, endLB, endUB;
  int tmp1, tmp2;


  if (!validRanges_.find(start, startLB, startUB, tmp1)) {
    return true;
  }
  if (!validRanges_.find(end-1, endLB, endUB, tmp2)) {

    return true;
  }
  
  if (tmp1 == Allocated) {
    return true;
  }
  if (tmp2 == Allocated) {
    return true;
  }
  if (startLB != endLB) {
    return true;
  }
  return false;
}


void SpringboardBuilder::registerBranch(Address start, Address end) {
  // We require branches to be smaller than their containing block.
  // Let's get the validRange containing this branch, split it, and
  // put it back in.
  int tmp;
  Address lb, ub;
  if (!validRanges_.find(start, lb, ub, tmp)) {
    assert(0);
  }
  

  assert(tmp != Allocated);

  validRanges_.remove(lb);
  // Add three ranges:
  // [lb..start] as true
  // [start..end] as false
  // [end..ub] as true
  if (lb < start) {
    validRanges_.insert(lb, start, curRange_++);
  }
  validRanges_.insert(start, end, Allocated);
  if (ub > end) {
    validRanges_.insert(end, ub, curRange_++);
  }
}

void SpringboardBuilder::debugRanges() {
  std::vector<std::pair<std::pair<Address, Address>, int> > elements;
  validRanges_.elements(elements);
  cerr << "Range debug: " << endl;
  for (unsigned i = 0; i < elements.size(); ++i) {
    cerr << "\t" << hex << elements[i].first.first
	 << ".." << elements[i].first.second << dec
	 << " -> " << elements[i].second << endl;
  }
  cerr << "-------------" << endl;
}

void SpringboardBuilder::generateBranch(Address from, Address to, codeGen &gen) {
  assert (from != 0x3124c7);

  gen.invalidate();
  gen.allocate(16);

  gen.setAddrSpace(addrSpace_);
  gen.setAddr(from);

  insnCodeGen::generateBranch(gen, from, to);
}

void SpringboardBuilder::generateTrap(Address from, Address to, codeGen &gen) {
  assert(0);
  gen.invalidate();
  gen.allocate(4);
  gen.setAddrSpace(addrSpace_);
  gen.setAddr(from);

  addrSpace_->trapMapping.addTrapMapping(from, to, true);
  insnCodeGen::generateTrap(gen);
}

bool SpringboardBuilder::generateReplacements(std::list<codeGen> &springboards,
					      const SpringboardReq &r, 
					      bool useTrap) {
  bool ret = true;
  std::list<Address> relocAddrs;
  addrSpace_->getRelocAddrs(r.from, relocAddrs);
  for (std::list<Address>::const_iterator iter = relocAddrs.begin();
       iter != relocAddrs.end(); ++iter) {
    if (*iter == r.to) {
      // Been here before
      continue;
    }
    assert(*iter != r.to);

    codeGen gen;
    if (useTrap) {
      generateTrap(*iter, r.to, gen);
    }
    else {
      generateBranch(*iter, r.to, gen);
    }
    springboards.push_back(gen);
  }
  return ret;
}

// Used in generating multi-branch springboards
bool SpringboardBuilder::generateReplacementPairs(std::list<codeGen> &springboards,
						  Address from,
						  Address to) {
  bool ret = true;
  std::list<std::pair<Address, Address> > pairs;
  addrSpace_->getRelocAddrPairs(from, to, pairs);
  for (std::list<std::pair<Address, Address> >::const_iterator iter = pairs.begin();
       iter != pairs.end(); ++iter) {
    codeGen gen;
    generateBranch(iter->first, iter->second, gen);
    springboards.push_back(gen);
  }
  return ret;
}

bool SpringboardBuilder::isLegalShortBranch(Address from, Address to) {
  // FIXME POWER...
  int disp = to - (from + 2);
  return ((disp >= -128) && (disp < 127));
}

Address SpringboardBuilder::shortBranchBack(Address from) {
  return from + 2 - 128;
};

