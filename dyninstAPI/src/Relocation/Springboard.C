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

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/codegen.h"

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

bool SpringboardBuilder::generateInt(std::list<codeGen> &springboards,
                                     SpringboardMap &input,
                                     Priority p) {
   // We want to do a reverse iteration so that we don't have a situation
   // where an earlier springboard overlaps a later one.
   //

   for (SpringboardMap::reverse_iterator iter = input.rbegin(p); 
        iter != input.rend(p); ++iter) {
      const SpringboardReq &req = iter->second;
      
      switch (generateSpringboard(springboards, req)) {
         case Failed:
            if (p == Required) {
               return false;
            }
            // Otherwise we didn't need it anyway.
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
   return true;
}

bool SpringboardBuilder::generate(std::list<codeGen> &springboards,
				  SpringboardMap &input) {
  // The SpringboardMap gives us a, well, mapping of desired
  // from: addresses and to: addresses. Our job is to create a series
  // of branches (and traps) that will take care of that. 

  // Currently we use a greedy algorithm rather than some sort of scheduling thing.
  // It's a heck of a lot easier that way. 

   createRelocSpringboards(Suggested, input);
   createRelocSpringboards(Required, input);

   //cerr << "Generating required springboards" << endl;
   if (!generateInt(springboards, input, Required))
      return false;
   //cerr << "Generating suggested springboards" << endl;
   if (!generateInt(springboards, input, Suggested))
      return false;

   bool ret = true;
   
   for (std::list<SpringboardReq>::iterator iter = multis_.begin();
        iter != multis_.end(); ++iter) {
      if (!generateMultiSpringboard(springboards, *iter)) {
         if (iter->priority == Required) {
            //cerr << "Failed required springboard @ " << hex << iter->from << endl;
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
    bool useBlock = true;
    bblInstance *bbl = (*begin)->origInstance();
    // don't add block if it's shared and the entry point of another function
    if (bbl->block()->llb()->isShared()) {
        using namespace ParseAPI;
        Block *llb = bbl->block()->llb();
        Address base = bbl->firstInsnAddr() - llb->start();
        std::vector<Function*> funcs;
        llb->getFuncs(funcs);
        for (vector<Function*>::iterator fit = funcs.begin();
             useBlock && fit != funcs.end();
             fit++) 
        {
            if ((*fit)->entry() == llb && 
                bbl->func() != addrSpace_->findFuncByInternalFunc(
                    static_cast<image_func*>(*fit)))
            {
                useBlock = false;
            }
        }
    }
    if (useBlock) {
        validRanges_.insert(bbl->firstInsnAddr(), bbl->endAddr(), curRange_++);
    }
  }
  return true;
}


SpringboardBuilder::generateResult_t 
SpringboardBuilder::generateSpringboard(std::list<codeGen> &springboards,
					const SpringboardReq &r) {
   codeGen gen;
   
   bool usedTrap = false;
   
   generateBranch(r.from, r.to, gen);
   
   if (conflict(r.from, r.from + gen.used(), r.fromRelocatedCode)) {
      //return MultiNeeded;
      
      // Errr...
      // Fine. Let's do the trap thing. 
      usedTrap = true;
      generateTrap(r.from, r.to, gen);
      if (conflict(r.from, r.from + gen.used(), r.fromRelocatedCode)) {
         // Someone could already be there; omit the trap. 
         return Failed;
      }
   }


  registerBranch(r.from, r.from + gen.used(), r.fromRelocatedCode);
  springboards.push_back(gen);

#if 0
  // Now explicitly included
  if (r.includeRelocatedCopies) {
    // Now catch all the previously relocated copies.
    generateReplacements(springboards, r, usedTrap);
  }
#endif

  return Succeeded;
}

bool SpringboardBuilder::generateMultiSpringboard(std::list<codeGen> &,
						  const SpringboardReq &) {
   //debugRanges();
   //cerr << "Request to generate multi-branch springboard skipped @ " << hex << r.from << dec << endl;
   // For now we give up and hope it all works out for the best. 
   return true;

#if 0
   // Much like the above, except try to do it in multiple steps. This works 
   // well on x86 where we have 2-byte dinky jumps. Less so on fixed-length
   // architectures...
  
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
   SpringboardReq tmp(tramp, r.to, r.priority, r.bbl, false, true);
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

   SpringboardReq tmp2(r.from, tramp, r.priority, r.bbl, false, true);
   generateReplacements(input, tmp2, false);

   return true;
#endif
}

bool SpringboardBuilder::conflict(Address start, Address end, bool inRelocated) {
   if (inRelocated) return conflictInRelocated(start, end);

   // We require springboards to stay within a particular block
   // so that we don't have issues with jumping into the middle
   // of a branch (... ew). Therefore, there is a conflict if
   // the end address lies within a different range than the start.
   //
   // Technically, end can be the start of another range; therefore
   // we search for (end-1).

   Address working = start;
   Address LB, UB;
   int state;
   //cerr << "Conflict called for " << hex << start << "->" << end << dec << endl;
   
   while (end > working) {
      if (!validRanges_.find(working, LB, UB, state)) {
         //cerr << "\t Conflict: unable to find entry for " << hex << working << dec << endl;
         return true;
      }
      
      if (state == Allocated) {
         //cerr << "\t Starting range already allocated, ret conflict" << endl;
         return true;
      }
      working = UB;
   }
   //cerr << "\t No conflict, we're good" << endl;
   return false;
}

bool SpringboardBuilder::conflictInRelocated(Address start, Address end) {
   // Much simpler case: do we overlap something already in the range set.
   for (Address i = start; i < end; ++i) {
      Address lb, ub;
      bool val;
      if (overwrittenRelocatedCode_.find(i, lb, ub, val)) {
         // oops!
         return true;
      }
   }
   return false;
}

void SpringboardBuilder::registerBranch(Address start, Address end, bool inRelocated) {
   // Remove the valid ranges for everything between start and end, using much the 
   // same logic as above.
   if (inRelocated) {
      return registerBranchInRelocated(start, end);
   }

   Address working = start;
   Address LB = 0, UB = 0;
   Address lb = 0, ub = 0;

   while (end > working) {
      int state;
      validRanges_.find(working, lb, ub, state);
      validRanges_.remove(lb);
      
      if (LB == 0) LB = lb;
      working = ub;
   }
   if (UB == 0) UB = ub;

   // Add three ranges:
   // [lb..start] as true
   // [start..end] as false
   // [end..ub] as true
   if (LB < start) {
      validRanges_.insert(LB, start, curRange_++);
   }
   validRanges_.insert(start, end, Allocated);
   if (UB > end) {
      validRanges_.insert(end, UB, curRange_++);
   }
}

void SpringboardBuilder::registerBranchInRelocated(Address start, Address end) {
   overwrittenRelocatedCode_.insert(start, end, true);
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
  gen.invalidate();
  gen.allocate(16);

  gen.setAddrSpace(addrSpace_);
  gen.setAddr(from);

  insnCodeGen::generateBranch(gen, from, to);
  cerr << "Springboard branch " << hex << from << "->" << to << dec << endl;
}

void SpringboardBuilder::generateTrap(Address from, Address to, codeGen &gen) {
  gen.invalidate();
  gen.allocate(4);
  gen.setAddrSpace(addrSpace_);
  gen.setAddr(from);

  addrSpace_->trapMapping.addTrapMapping(from, to, true);
  insnCodeGen::generateTrap(gen);
}

bool SpringboardBuilder::createRelocSpringboards(Priority p, SpringboardMap &input) {
   for (SpringboardMap::iterator iter = input.begin(p);
        iter != input.end(p); ++iter) {
      const SpringboardReq &req = iter->second;
      if (req.fromRelocatedCode) continue;
      assert(req.bbl);

      std::list<Address> relocAddrs;
      addrSpace_->getRelocAddrs(req.from, req.bbl->func(), relocAddrs, true);
      for (std::list<Address>::const_iterator addr = relocAddrs.begin(); 
           addr != relocAddrs.end(); ++addr) {
         if (*addr == req.to) continue;
         input.addRaw(*addr, req.to, 
                      p, req.bbl,
                      req.checkConflicts, 
                      false, true);
      }
   }
   return true;
}

bool SpringboardBuilder::generateReplacements(std::list<codeGen> &springboards,
					      const SpringboardReq &r, 
					      bool useTrap) {
   if (!r.includeRelocatedCopies) return true;

  bool ret = true;
  std::list<Address> relocAddrs;
  addrSpace_->getRelocAddrs(r.from, r.bbl->func(), relocAddrs, true);
  for (std::list<Address>::const_iterator iter = relocAddrs.begin();
       iter != relocAddrs.end(); ++iter) {
    if (*iter == r.to) {
      // Been here before
      continue;
    }
    assert(*iter != r.to);

    codeGen gen;
    switch (useTrap) {
       case false:
          generateBranch(*iter, r.to, gen);
          if (!conflict(*iter, (*iter) + gen.used(), true)) {
             break;
          }
       case true:
          // Or fallthrough from a conflict
          gen.setIndex(0);
          generateTrap(*iter, r.to, gen);
          if (conflict(*iter, (*iter) + gen.used(), true)) {
             // SUUUUCKTASTIC
             ret = false;
             continue;
          }
    }
    registerBranch(*iter, (*iter) + gen.used(), true);

    springboards.push_back(gen);
  }
  return ret;
}

#if 0
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
#endif

bool SpringboardBuilder::isLegalShortBranch(Address from, Address to) {
  // FIXME POWER...
  int disp = to - (from + 2);
  return ((disp >= -128) && (disp < 127));
}

Address SpringboardBuilder::shortBranchBack(Address from) {
  return from + 2 - 128;
};

