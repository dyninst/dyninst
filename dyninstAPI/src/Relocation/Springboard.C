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

#include "CFG.h"
#include "Springboard.h"
#include "patchapi_debug.h"
#include "dyninstAPI/src/codegen.h"

#include "dyninstAPI/src/addressSpace.h"

using namespace Dyninst;
using namespace PatchAPI;

const int SpringboardBuilder::Allocated(0);
const int SpringboardBuilder::UnallocatedStart(1);
std::set<Address> SpringboardBuilder::relocTraps_; 

template <typename TraceIter> 
SpringboardBuilder::Ptr SpringboardBuilder::create(TraceIter begin,
						   TraceIter end,
						   AddressSpace *as) {
  Ptr ret = Ptr(new SpringboardBuilder(as));
  if (!ret) return ret;

  if (!ret->addTraces(begin, end, UnallocatedStart)) return Ptr();
  return ret;
}

SpringboardBuilder::Ptr SpringboardBuilder::createFunc(FuncSet::const_iterator begin,
						       FuncSet::const_iterator end,
						       AddressSpace *as) 
{
  Ptr ret = Ptr(new SpringboardBuilder(as));
  if (!ret) return ret;
  int id = UnallocatedStart;
  for (; begin != end; ++begin) {
     Function *func = *begin;
     if (!ret->addTraces(func->blocks().begin(), func->blocks().end(), id++)) {
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
      
      switch (generateSpringboard(springboards, req, input)) {
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

   if (patch_debug_springboard) {
      cerr << "SPRINGBOARD GENERATION" << endl;
      debugRanges();
   }


   if (!generateInt(springboards, input, Required))
      return false;

   if (!generateInt(springboards, input, Suggested))
      return false;

   // Catch up with instrumentation
   if (!generateInt(springboards, input, RelocRequired)) 
      return false;
   if (!generateInt(springboards, input, RelocSuggested))
      return false;

   return true;
}

template <typename TraceIter>
bool SpringboardBuilder::addTraces(TraceIter begin, TraceIter end, int funcID) {
  // TODO: map these addresses to relocated blocks as well so we 
  // can do our thang.
  for (; begin != end; ++begin) {
    bool useBlock = true;
    Block *bbl = (*begin);

    // don't add block if it's shared and the entry point of another function
    if (bbl->isShared()) {
        using namespace ParseAPI;
        ParseAPI::Block *llb = bbl->block();

        std::vector<ParseAPI::Function*> funcs;
        llb->getFuncs(funcs);
        for (vector<ParseAPI::Function*>::iterator fit = funcs.begin();
             fit != funcs.end(); ++fit) 
        {
           if ((*fit) == bbl->func()->func()) continue;
           if ((*fit)->entry() == llb) {
              useBlock = false;
              break;
           }
        }
        int dontcare;
        if (useBlock && validRanges_.find(bbl->start(),dontcare)) {
            // if we're replacing a shared block that is already
            // in validRanges_, remove it before adding bbl
            validRanges_.remove(bbl->start());
        }
    }
    if (useBlock) {
        // Check for overlapping blocks. Lovely.
        Address LB, UB; int id;
        Address lastRangeStart = bbl->start();
        for (Address lookup = bbl->start(); lookup < bbl->end(); ) 
        {/* there may be more than one range that overlaps with bbl, 
          * so we update lookup and lastRangeStart to after each conflict
          * to match UB, and loop until lookup >= bbl->end()
          */
            if (validRanges_.find(lookup, LB, UB, id)) 
            {
                /* The ranges overlap and we must split them into non-
                 * overlapping ranges, possible range splits are listed
                 * below:
                 *
                 * [LB UB)                     already in validRanges_, remove if it gets split
                 * [LB bbl->start())
                 * [bbl->start() LB)
                 * [lastRangeStart LB)         when bbl overlaps multiple ranges and LB is for an N>1 range
                 * [bbl->start() UB)           possible if LB < bbl->start()
                 * [lastRangeStart UB)         possible if LB < bbl->start() and bbl overlaps multiple ranges
                 * [LB bbl->end())             if last existing range includes bbl->end()
                 * [bbl->end() UB)             if last existing range includes bbl->end()
                 * [UB bbl->end())             don't add until after loop exits as there might be more overlapping ranges
                 */
                if (LB < bbl->start()) { 
                    validRanges_.remove(LB); // replace [LB UB)...
                    validRanges_.insert(LB, bbl->start(), funcID); // with  [LB bbl->start())
                    if (UB <= bbl->end()) { // [bbl->start() UB)
                         validRanges_.insert(bbl->start(), UB, funcID);
                    } else { // [bbl->start() bbl->end()) or [lastRangeStart bbl->end()) and [bbl->end() UB) 
                         validRanges_.insert(bbl->start(), bbl->end(), funcID);
                         validRanges_.insert(bbl->end(), UB, funcID);
                    }
                } 
                else {
                    if (lastRangeStart < LB) { // add [bbl->start() LB) or [lastRangeStart LB)
                        validRanges_.insert(lastRangeStart, LB, funcID);
                    }
                    if (UB > bbl->end()) { // [LB bbl->end()) and [bbl->end() UB) 
                         validRanges_.insert(LB, bbl->end(), funcID);
                         validRanges_.insert(bbl->end(), UB, funcID);
                    } // otherwise [LB UB) is already in validRanges_
                }
                lookup = UB;
                lastRangeStart = UB;
            }
            else {
                lookup++;
            }
        }
        if (lastRangeStart < bbl->end()) { // [bbl->start() bbl->end()) or [UB bbl->end())
            validRanges_.insert(lastRangeStart, bbl->end(), funcID);
        }
    }
  }
  return true;
}

extern bool disassemble_reloc;
SpringboardBuilder::generateResult_t 
SpringboardBuilder::generateSpringboard(std::list<codeGen> &springboards,
					const SpringboardReq &r,
                                        SpringboardMap &input) {
   codeGen gen;
   
   bool usedTrap = false;
   if (disassemble_reloc) cerr << "Springboard: " << hex << r.from << " -> " << r.destinations.begin()->second << dec << endl;

   generateBranch(r.from, r.destinations.begin()->second, gen);

   if (r.useTrap || conflict(r.from, r.from + gen.used(), r.fromRelocatedCode)) {
      // Errr...
      // Fine. Let's do the trap thing. 
      usedTrap = true;
      generateTrap(r.from, r.destinations.begin()->second, gen);
	  //cerr << hex << "Generated springboard trap: " << hex << r.from << " -> " << r.destinations.begin()->second << dec << endl;
	  if (conflict(r.from, r.from + gen.used(), r.fromRelocatedCode)) {
         // Someone could already be there; omit the trap. 
         return Failed;
      }
   }

   if (r.includeRelocatedCopies) {
      createRelocSpringboards(r, usedTrap, input);
   }

  registerBranch(r.from, r.from + gen.used(), r.destinations, r.fromRelocatedCode);
  springboards.push_back(gen);

  return Succeeded;
}

bool SpringboardBuilder::generateMultiSpringboard(std::list<codeGen> &,
						  const SpringboardReq &) {
   //debugRanges();
   //if (false) cerr << "Request to generate multi-branch springboard skipped @ " << hex << r.from << dec << endl;
   // For now we give up and hope it all works out for the best. 
   return true;
}

bool SpringboardBuilder::conflict(Address start, Address end, bool inRelocated) {
   if (inRelocated) 
       return conflictInRelocated(start, end);

   // We require springboards to stay within a particular block
   // so that we don't have issues with jumping into the middle
   // of a branch (... ew). Therefore, there is a conflict if
   // the end address lies within a different range than the start.
   //
   // Technically, end can be the start of another range; therefore
   // we search for (end-1).

    // We also don't want to start in one function's dead space and cross
   // into another's. So check to see if state suddenly changes.

   Address working = start;
   Address LB;
   Address UB = 0;
   int state = -1;
   int lastState = state;
   springboard_cerr << "Conflict called for " << hex << start << "->" << end << dec << endl;
   
   while (end > working) {
        springboard_cerr << "\t looking for " << hex << working << dec << endl;
       if (!validRanges_.find(working, LB, UB, state)) {
         springboard_cerr << "\t Conflict: unable to find entry for " << hex << working << dec << endl;
         return true;
      }
      springboard_cerr << "\t\t Found " << hex << LB << " -> " << UB << " /w/ state " << state << dec << endl;
      if (state == Allocated) {
         springboard_cerr << "\t Starting range already allocated, ret conflict" << endl;
         return true;
      }
      if (lastState != -1 &&
          state != lastState) {
          springboard_cerr << "\t Crossed into a different function, ret conflict" << endl;
          return true;
      }
      working = UB;
      lastState = state;
   }
   if (UB < end) {
       return true;
   }
   springboard_cerr << "\t No conflict, we're good" << endl;
   return false;
}

bool SpringboardBuilder::conflictInRelocated(Address start, Address end) {
   // Much simpler case: do we overlap something already in the range set, 
   // or did we use a trap for this block initially
   for (Address i = start; i < end; ++i) {
      Address lb, ub;
      bool val;
      if (overwrittenRelocatedCode_.find(i, lb, ub, val)) {
         // oops!
         return true;
      }
   }
   if ( (end-start) > 1 && relocTraps_.end() != relocTraps_.find(start) ) {
#if 0
       malware_cerr << "Springboard conflict for " << hex << start  
           << " our previous springboard here needed a trap, "
           << "but due to overwrites we may (erroneously) think "
           << "a branch can fit" << dec << endl;
       springboard_cerr << "Springboard conflict for " << hex << start  
           << " our previous springboard here needed a trap, "
           << "but due to overwrites we may (erroneously) think "
           << "a branch can fit" << dec << endl;
#endif
       return true;
   }

   return false;
}

void SpringboardBuilder::registerBranch
(Address start, Address end, const SpringboardReq::Destinations & dest, bool inRelocated) 
{
   // Remove the valid ranges for everything between start and end, using much the 
   // same logic as above.

   if ( 1 == (end - start) ) {
       for (SpringboardReq::Destinations::const_iterator dit=dest.begin();
            dit != dest.end();
            dit++)
       {
           relocTraps_.insert(start);
           relocTraps_.insert(dit->second);// if we relocate again it will need a trap too
       }
   }
    
   if (inRelocated) {
      return registerBranchInRelocated(start, end);
   }

   Address working = start;
   Address LB = 0, UB = 0;
   Address lb = 0, ub = 0;
   springboard_cerr << "Adding branch: " << hex << start << " -> " << end << dec << endl;
   int idToUse = -1;
   while (end > working) {
      int state = 0;
      validRanges_.find(working, lb, ub, state);
      validRanges_.remove(lb);

      if (idToUse == -1) idToUse = state;
        else assert(idToUse = state);

      if (LB == 0) LB = lb;
      working = ub;
   }
   if (UB == 0) UB = ub;

   // Add three ranges:
   // [lb..start] as true
   // [start..end] as false
   // [end..ub] as true
   if (LB < start) {
        springboard_cerr << "\tInserting prior space " << hex << LB << " -> " << start << " /w/ range " << idToUse << dec << endl;
       validRanges_.insert(LB, start, idToUse);
   }
    springboard_cerr << "\t Inserting taken space " << hex << start << " -> " << end << " /w/ range " << Allocated << dec << endl;
   validRanges_.insert(start, end, Allocated);
   if (UB > end) {
        springboard_cerr << "\tInserting post space " << hex << end << " -> " << UB << " /w/ range " << idToUse << dec << endl;
      validRanges_.insert(end, UB, idToUse);
   }
}

void SpringboardBuilder::registerBranchInRelocated(Address start, Address end) {
   overwrittenRelocatedCode_.insert(start, end, true);
}


void SpringboardBuilder::debugRanges() {
  std::vector<std::pair<std::pair<Address, Address>, int> > elements;
  validRanges_.elements(elements);
  if (false) cerr << "Range debug: " << endl;
  for (unsigned i = 0; i < elements.size(); ++i) {
     if (false) cerr << "\t" << hex << elements[i].first.first
	 << ".." << elements[i].first.second << dec
	 << " -> " << elements[i].second << endl;
  }
  if (false) cerr << "-------------" << endl;
}

void SpringboardBuilder::generateBranch(Address from, Address to, codeGen &gen) {
  gen.invalidate();
  gen.allocate(16);

  gen.setAddrSpace(addrSpace_);
  gen.setAddr(from);

  insnCodeGen::generateBranch(gen, from, to);

  springboard_cerr << "Springboard branch " << hex << from << "->" << to << dec << endl;
}

void SpringboardBuilder::generateTrap(Address from, Address to, codeGen &gen) {
	//cerr << "Springboard: generateTrap " << hex << from << " -> " << to << dec << endl;
	gen.invalidate();
  gen.allocate(4);
  gen.setAddrSpace(addrSpace_);
  gen.setAddr(from);
  springboard_cerr << "YUCK! Springboard trap at: "<< hex << from << "->" << to << dec << endl;
  addrSpace_->trapMapping.addTrapMapping(from, to, true);
  insnCodeGen::generateTrap(gen);
}

bool SpringboardBuilder::createRelocSpringboards(const SpringboardReq &req, bool useTrap, SpringboardMap &input) {
#if TODO
   assert(!req.fromRelocatedCode);
   // Just the requests for now.
   //cerr << "\t createRelocSpringboards for " << hex << req.from << dec << endl;
   std::list<Address> relocAddrs;
   for (SpringboardReq::Destinations::const_iterator b_iter = req.destinations.begin(); 
       b_iter != req.destinations.end(); ++b_iter) {

      addrSpace_->getRelocAddrs(req.from, b_iter->first->func(), relocAddrs, true);
      for (std::list<Address>::const_reverse_iterator addr = relocAddrs.rbegin(); 
           addr != relocAddrs.rend(); ++addr) { 
         if (*addr == b_iter->second) continue;
         Priority newPriority;
         switch(req.priority) {
            case Suggested:
               newPriority = RelocSuggested;
               break;
            case Required:
               newPriority = RelocRequired;
               break;
            default:
               assert(0);
               break;
         }
         bool curUseTrap = useTrap;
         if ( !useTrap && relocTraps_.end() != relocTraps_.find(*addr)) {
            springboard_cerr << "Springboard conflict for " << hex 
                             << req.from << "[" << (*addr) 
                             << "] our previous springboard here needed a trap, "
                             << "but due to overwrites we may (erroneously) think "
                             << "a branch can fit" << dec << endl;
            curUseTrap = true;
         }
         
         input.addRaw(*addr, b_iter->second, 
                      newPriority, b_iter->first,
                      req.checkConflicts, 
                      false, true, curUseTrap);
         
      }
   }
#endif         
   return true;
}


