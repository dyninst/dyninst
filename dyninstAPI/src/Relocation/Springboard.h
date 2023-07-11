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

// Build the branches from previous versions of moved code to the new versions.

#if !defined(_R_SPRINGBOARD_H_)
#define _R_SPRINGBOARD_H_
#include <assert.h>
#include <list>
#include <set>
#include <map>
#include "common/src/IntervalTree.h"
#include "common/h/dyntypes.h"
#include "Transformers/Transformer.h" // Priority enum
#include "dyninstAPI/src/codegen.h"

class AddressSpace;

class func_instance;

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
   /* Currently we put suggested springboards at non-func-entry, 
    * non-indirect-jump-target block entry.
    * In case the jump table analysis under-approximate the jump targets (unlikely),
    * the control flow will goes back to instrumentation at other blocks*/
   Suggested,
   /* Indirect jump target block is very important, 
    * but is less important than func entry.
    *
    * Control flow can escape instrumentation by indirect jump (jump tables).
    * So, we install springboards at all indirect jump targets.
    * However, jump table analysis can overapproximate jump targets and
    * the bogus jump targets can be function entries. So, we put indirect
    * jump target as one priority lower than function entry
    */
   IndirBlockEntry,    
   /* FuncEntry represents springboards at function entries.
    * This is the highest priority because control flow enters
    * instrumentation at function entry
    */ 
   FuncEntry,    
   ORIG_MAX_PRIORITY,
   MAX_PRIORITY } Priority;

struct SpringboardReq {
   typedef std::map<func_instance *, Address> Destinations;

   Address from;
   Priority priority;
   func_instance *func;
   block_instance *block;
   Destinations destinations;
   bool checkConflicts;
   bool includeRelocatedCopies;
   bool fromRelocatedCode;
   bool useTrap;
   SpringboardReq(const Address from_, 
                  const Address to_, 
                  const Priority priority_, 
                  func_instance *func_,
                  block_instance *block_, 
                  bool checkConflicts_, 
                  bool includeRelocCopies_, 
                  bool fromRelocCode_,
                  bool useTrap_)
   : from(from_), 
      priority(priority_),
      func(func_),
      block(block_),
      checkConflicts(checkConflicts_), 
      includeRelocatedCopies(includeRelocCopies_),
      fromRelocatedCode(fromRelocCode_),
      useTrap(useTrap_) 
   {
      destinations[func_] = to_;
   }
SpringboardReq() 
: from(0), priority(NotRequired),
      func(NULL), 
      block(NULL),
      checkConflicts(false),
      includeRelocatedCopies(false),
      fromRelocatedCode(false),
      useTrap(false) {}
   void addReq (const Address from_, const Address to_,
                const Priority priority_, 
                func_instance *func_, block_instance *block_,
                bool checkConflicts_,
                bool includeRelocCopies_,
                bool fromRelocCode_, 
                bool useTrap_) 
   {
        // This mechanism handles overlapping functions, where
        // we might see springboards from the same address to
        // different targets. In this case only one can win,
        // but we want to track the different bbls so that
        // we can do the right thing with includeRelocatedCopies.
        if (from == 0) {
            // New version version
            assert(destinations.empty()); 
            from = from_;
            func = func_;
            block = block_;
            priority = priority_;
            destinations[func_] = to_;
            checkConflicts = checkConflicts_;
            includeRelocatedCopies = includeRelocCopies_;
            fromRelocatedCode = fromRelocCode_;
            useTrap = useTrap_;
        }
        else {
            assert(from == from_);
            destinations[func_] = to_;
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
                        Priority p, func_instance *func, block_instance *bbl) {
// This uses the default constructor if it isn't already there.
      sBoardMap_[p][from].addReq(from, to, p, func, bbl, true, true, false, false);
   }

   void addFromRelocatedCode(Address from, Address to,
                             Priority p) {
      assert(p < RELOC_MAX_PRIORITY);
      sBoardMap_[p][from] = SpringboardReq(from, to,
                                           p,
                                           NULL,
                                           NULL,
                                           true, 
                                           false,
                                           true, false);
   }
   
   void addRaw(Address from, Address to, Priority p, 
               func_instance *func, block_instance *bbl,
               bool checkConflicts, bool includeRelocatedCopies, bool fromRelocatedCode,
               bool useTrap) {
      sBoardMap_[p][from] = SpringboardReq(from, to, p, func, bbl,
                                           checkConflicts, includeRelocatedCopies,
                                           fromRelocatedCode, useTrap);
   }

   iterator begin(Priority p) { return sBoardMap_[p].begin(); }
   iterator end(Priority p) { return sBoardMap_[p].end(); }

   reverse_iterator rbegin(Priority p) { return sBoardMap_[p].rbegin(); }
   reverse_iterator rend(Priority p) { return sBoardMap_[p].rend(); }


 private:
   Springboards sBoardMap_;
 };

 // Persistent tracking of things that have already gotten springboards across multiple
 // calls to AddressSpace::relocateInt() and thus across multiple SpringboardFoo,
 // CodeTracker, CodeMover objects.

struct SpringboardInfo {
    int val;
    func_instance *func;
    Priority priority;

    SpringboardInfo(int v, func_instance* f) : val(v), func(f), priority(MIN_PRIORITY) {}
    SpringboardInfo(int v, func_instance* f, Priority p) : val(v), func(f), priority(p) {}
};

class InstalledSpringboards
{
 public:
  typedef boost::shared_ptr<InstalledSpringboards> Ptr;
  static const int Allocated;
  static const int UnallocatedStart;
 InstalledSpringboards() {}
  
  

  template <typename BlockIter> 
  bool addBlocks(func_instance* func, BlockIter begin, BlockIter end);
  bool addFunc(func_instance* f);
  bool conflict(Address start, Address end, bool inRelocatedCode, func_instance* func, Priority p);
  bool conflictInRelocated(Address start, Address end);

  void registerBranch(Address start, Address end, const SpringboardReq::Destinations &dest, bool inRelocatedCode, func_instance* func, Priority p);
  void registerBranchInRelocated(Address start, Address end, func_instance* func, Priority p);
  bool forceTrap(Address a) 
  {
    return relocTraps_.find(a) != relocTraps_.end();
  }

    
  
 private:
  // tracks relocation addresses that need trap-based springboards
  std::set<Address> relocTraps_; 
  

  // We don't really care about the payload; I just want an "easy to look up"
  // range data structure. 
  // Map this to an int because IntervalTree collapses similar ranges. Punks.
  IntervalTree<Address, SpringboardInfo*> validRanges_;

  // If we consume NOP-padding between functions to get room for a jump, that
  // padding may not exist in the relocation buffer.  Remember such ranges so
  // we can deal with that in reinstrumentation, if only to force a trap.
  IntervalTree<Address, SpringboardInfo*> paddingRanges_;

  // Like the previous, but for branches we put in relocated code. We
  // assume anything marked as "in relocated code" is a valid thing to write
  // to, since relocation size is >= original size. However, we still don't
  // want overlapping branches. 
  IntervalTree<Address, SpringboardInfo*> overwrittenRelocatedCode_;
  void debugRanges();
  
};
 
 

class SpringboardBuilder {
  typedef enum {
    Failed,
    MultiNeeded,
    Succeeded } generateResult_t;

 public:
  typedef boost::shared_ptr<SpringboardBuilder> Ptr;
  typedef std::set<func_instance *> FuncSet;

  static Ptr createFunc(FuncSet::const_iterator begin, FuncSet::const_iterator end, AddressSpace *addrSpace);

  bool generate(std::list<codeGen> &springboards,
		SpringboardMap &input);

 private:
  SpringboardBuilder(AddressSpace *a);

  bool generateInt(std::list<codeGen> &springboards,
                   SpringboardMap &input,
                   Priority p);

  generateResult_t generateSpringboard(std::list<codeGen> &gens,
				       const SpringboardReq &p);

  bool generateMultiSpringboard(std::list<codeGen> &input,
				const SpringboardReq &p);

  // Find all previous instrumentations and also overwrite 
  // them. 
  bool createRelocSpringboards(const SpringboardReq &r, bool useTrap, SpringboardMap &input);

  bool generateReplacements(std::list<codeGen> &input,
			    const SpringboardReq &p,
			    bool useTrap);


  void addMultiNeeded(const SpringboardReq &p);

  void generateBranch(Address from, Address to, codeGen &input);
  void generateTrap(Address from, Address to, codeGen &input);

  bool conflict(Address start, Address end, bool inRelocatedCode, func_instance* func, Priority p) { return installed_springboards_->conflict(start, end, inRelocatedCode, func, p); }

  void registerBranch(Address start, Address end, const SpringboardReq::Destinations &dest, bool inRelocatedCode, func_instance* func, Priority p)
  {
    return installed_springboards_->registerBranch(start, end, dest, inRelocatedCode, func, p);
  }

  bool isLegalShortBranch(Address from, Address to);
  Address shortBranchBack(Address from);


  AddressSpace *addrSpace_;

  InstalledSpringboards::Ptr installed_springboards_;
  
  std::list<SpringboardReq> multis_;

};

}
}

#endif
