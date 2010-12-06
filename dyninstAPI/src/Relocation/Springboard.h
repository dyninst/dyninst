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

// Build the branches from previous versions of moved code to the new versions.

#if !defined(_R_SPRINGBOARD_H_)
#define _R_SPRINGBOARD_H_
#include <map>
#include "common/h/IntervalTree.h"
#include "dynutil/h/dyntypes.h"
#include "Transformers/Transformer.h" // Priority enum

class AddressSpace;

namespace Dyninst {
namespace Relocation {

typedef enum {
   MIN_PRIORITY,
   RELOC_MIN_PRIORITY,
   RelocNotRequired,
   RelocSuggested,
   RelocRequired,
   RelocOffLimits,
   RELOC_MAX_PRIORITY,
   ORIG_MIN_PRIORITY,
   NotRequired,
   Suggested,
   Required,
   OffLimits,
   ORIG_MAX_PRIORITY,
   MAX_PRIORITY } Priority;

struct SpringboardReq {
   Address from;
   Priority priority;
   typedef std::map<int_block *, Address> Destinations;
   Destinations destinations;
   bool checkConflicts;
   bool includeRelocatedCopies;
   bool fromRelocatedCode;
   bool useTrap;
   SpringboardReq(const Address a, const Address b, 
                  const Priority c, int_block *d, 
                  bool e, 
                  bool f, 
                  bool g,
                  bool i)
   : from(a), 
      priority(c), 
      checkConflicts(e), 
      includeRelocatedCopies(f),
      fromRelocatedCode(g),
      useTrap(i) 
   {
      destinations[d] = b;
   }
   SpringboardReq() 
   : from(0), priority(NotRequired), 
      checkConflicts(false),
      includeRelocatedCopies(false),
      fromRelocatedCode(false),
      useTrap(false) {};
    void addReq (const Address a, const Address b,
                        const Priority c, int_block *d,
                        bool e, bool f, bool g, bool i) 
    {
        // This mechanism handles overlapping functions, where
        // we might see springboards from the same address to
        // different targets. In this case only one can win,
        // but we want to track the different bbls so that
        // we can do the right thing with includeRelocatedCopies.
        if (from == 0) {
            // New version version
            assert(destinations.empty()); 
            from = a;
            priority = c;
            destinations[d] = b;
            checkConflicts = e;
            includeRelocatedCopies = f;
            fromRelocatedCode = g;
            useTrap = i;
        }
        else {
            assert(from == a);
            destinations[d] = b;
        }
    }
};

class SpringboardBuilder;

 class SpringboardMap {
   friend class CodeMover;
 public:

   
   typedef std::map<Address, SpringboardReq> SpringboardsAtPriority;
   typedef std::map<Priority, SpringboardsAtPriority> Springboards;
   typedef SpringboardsAtPriority::iterator iterator;
   typedef SpringboardsAtPriority::const_iterator const_iterator;
   typedef SpringboardsAtPriority::reverse_iterator reverse_iterator;

   bool empty() const { 
     return sBoardMap_.empty();
   }

   void addFromOrigCode(Address from, Address to, 
                        Priority p, int_block *bbl) {
// This uses the default constructor if it isn't already there.
      sBoardMap_[p][from].addReq(from, to, p, bbl, true, true, false, false);
   }

   void addFromRelocatedCode(Address from, Address to,
                             Priority p) {
      assert(p < RELOC_MAX_PRIORITY);
      sBoardMap_[p][from] = SpringboardReq(from, to,
                                           p,
                                           NULL,
                                           true, 
                                           false,
                                           true, false);
   };
   
   void addRaw(Address from, Address to, Priority p, int_block *bbl,
               bool checkConflicts, bool includeRelocatedCopies, bool fromRelocatedCode,
               bool useTrap) {
      sBoardMap_[p][from] = SpringboardReq(from, to, p, bbl,
                                           checkConflicts, includeRelocatedCopies,
                                           fromRelocatedCode, useTrap);
   }

   iterator begin(Priority p) { return sBoardMap_[p].begin(); };
   iterator end(Priority p) { return sBoardMap_[p].end(); };

   reverse_iterator rbegin(Priority p) { return sBoardMap_[p].rbegin(); };
   reverse_iterator rend(Priority p) { return sBoardMap_[p].rend(); };


 private:
   Springboards sBoardMap_;
 };

class SpringboardBuilder {
  typedef enum {
    Failed,
    MultiNeeded,
    Succeeded } generateResult_t;

 public:
  typedef dyn_detail::boost::shared_ptr<SpringboardBuilder> Ptr;
  typedef std::set<int_function *> FuncSet;

  template <typename TraceIter> 
    static Ptr create(TraceIter begin, TraceIter end, AddressSpace *addrSpace); 
  static Ptr createFunc(FuncSet::const_iterator begin, FuncSet::const_iterator end, AddressSpace *addrSpace);

  bool generate(std::list<codeGen> &springboards,
		SpringboardMap &input);

 private:

  static const int Allocated;
  static const int UnallocatedStart;

 SpringboardBuilder(AddressSpace *a) : addrSpace_(a), curRange_(UnallocatedStart) {};
  template <typename TraceIter> 
    bool addTraces(TraceIter begin, TraceIter end, int funcID);

  bool generateInt(std::list<codeGen> &springboards,
                   SpringboardMap &input,
                   Priority p);

  generateResult_t generateSpringboard(std::list<codeGen> &gens,
				       const SpringboardReq &p,
                                       SpringboardMap &);

  bool generateMultiSpringboard(std::list<codeGen> &input,
				const SpringboardReq &p);

  // Find all previous instrumentations and also overwrite 
  // them. 
  bool createRelocSpringboards(const SpringboardReq &r, bool useTrap, SpringboardMap &input);

  bool generateReplacements(std::list<codeGen> &input,
			    const SpringboardReq &p,
			    bool useTrap);

  bool conflict(Address start, Address end, bool inRelocatedCode);
  bool conflictInRelocated(Address start, Address end);

  void registerBranch(Address start, Address end, bool inRelocatedCode);
  void registerBranchInRelocated(Address start, Address end);

  void addMultiNeeded(const SpringboardReq &p);

  void generateBranch(Address from, Address to, codeGen &input);
  void generateTrap(Address from, Address to, codeGen &input);

  bool isLegalShortBranch(Address from, Address to);
  Address shortBranchBack(Address from);

  void debugRanges();

  AddressSpace *addrSpace_;

  

  // We don't really care about the payload; I just want an "easy to look up"
  // range data structure. 
  // Map this to an int because IntervalTree collapses similar ranges. Punks.
  IntervalTree<Address, int> validRanges_;
  int curRange_;

  // Like the previous, but for branches we put in relocated code. We
  // assume anything marked as "in relocated code" is a valid thing to write
  // to, since relocation size is >= original size. However, we still don't
  // want overlapping branches. 
  IntervalTree<Address, bool> overwrittenRelocatedCode_;

  std::list<SpringboardReq> multis_;

};

};
};

#endif
