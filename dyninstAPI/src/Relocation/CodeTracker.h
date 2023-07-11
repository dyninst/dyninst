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

// Keeps a list of relocated Widgets (compessed where possible) for mapping
// addresses and types between original and relocated code. 

#if !defined(_R_CODE_TRACKER_H_)
#define _R_CODE_TRACKER_H_

#include "common/h/dyntypes.h"
#include <assert.h>
#include <map>
#include <vector>
#include <set>
#include <list>
#include "common/src/IntervalTree.h"
#include <iostream>

// Remove when I'm done debugging this...
//#include "dyninstAPI/src/baseTramp.h"

class baseTramp;
class func_instance;
class block_instance;
class AddressSpace;
class instPoint;

namespace Dyninst {
namespace Relocation {
class CodeTracker;

class TrackerElement {
  friend class CodeTracker;
 public:
  typedef enum {
    original,
    emulated,
    instrumentation,
    padding
  } type_t;

  TrackerElement(Address o, block_instance *b, func_instance *f) 
      : orig_(o), reloc_(0), size_(0), 
     block_(b), func_(f) {
     assert(b);
  }
  virtual ~TrackerElement() {
  }

  virtual Address relocToOrig(Address reloc) const = 0;
  virtual Address origToReloc(Address orig) const = 0;
  virtual type_t type() const = 0;

  Address orig() const { return orig_; }
  Address reloc() const { return reloc_; }
  unsigned size() const { return size_; }
  block_instance *block() const { return block_; }
  func_instance *func() const { return func_; }

  void setReloc(Address reloc) { reloc_ = reloc; }
  void setSize(unsigned size) { size_ = size; }

  virtual bool mergeable() const { return true; }

 protected:
  TrackerElement() { assert(0); }
  TrackerElement(const TrackerElement &)  { assert(0); }
  Address orig_;
  Address reloc_;
  unsigned size_;
  block_instance *block_;
  func_instance *func_;
};

class OriginalTracker : public TrackerElement {
 public:
  OriginalTracker(Address orig, block_instance *b, func_instance *f) :
   TrackerElement(orig, b, f) {}
  virtual ~OriginalTracker() {}

  virtual Address relocToOrig(Address reloc) const {
    assert(reloc >= reloc_);
    assert(reloc < (reloc_ + size_));
    return reloc - reloc_ + orig_;
  }

  virtual Address origToReloc(Address orig) const {
    assert(orig >= orig_);
    assert(orig < (orig_ + size_));
    return orig - orig_ + reloc_;
  }

  virtual type_t type() const { return TrackerElement::original; }


 private:
};
    std::ostream &
    operator<<(std::ostream &os, const Dyninst::Relocation::TrackerElement &e);

class EmulatorTracker : public TrackerElement {
 public:
  EmulatorTracker(Address orig, block_instance *b, func_instance *f) : 
   TrackerElement(orig, b, f) {}
  virtual ~EmulatorTracker() {}

  virtual Address relocToOrig(Address reloc) const {
    (void)reloc; // unused
    assert(reloc >= reloc_);
    assert(reloc < (reloc_ + size_));
    return orig_;
  }

  virtual Address origToReloc(Address orig) const {
    (void)orig; // unused
    assert(orig == orig_);
    return reloc_;
  }

  virtual type_t type() const { return TrackerElement::emulated; }



 private:
};

class InstTracker : public TrackerElement {
 public:
  InstTracker(Address orig, baseTramp *baseT, block_instance *b, func_instance *f) :
   TrackerElement(orig, b, f), baseT_(baseT) {}
  virtual ~InstTracker() {}

  virtual Address relocToOrig(Address reloc) const {
    (void)reloc; // unused
    assert(reloc >= reloc_);
    assert(reloc < (reloc_ + size_));
    return orig_;
  }

  virtual Address origToReloc(Address orig) const {
    (void)orig; // unused
    assert(orig == orig_);
    return reloc_;
  }

  virtual type_t type() const { return TrackerElement::instrumentation; }
  baseTramp *baseT() const { return baseT_; }
  
  virtual bool mergeable() const { return false; }

 private:
  baseTramp *baseT_;
};

class PaddingTracker : public TrackerElement {
 public:
  PaddingTracker(Address orig, unsigned pad, block_instance *b, func_instance *f) :
   TrackerElement(orig, b, f), pad_(pad) {}
   virtual ~PaddingTracker() {}

  virtual Address relocToOrig(Address reloc) const {
    (void)reloc; // unused
    assert(reloc >= reloc_);
    assert(reloc < (reloc_ + size_));
    return orig_;
  }

  virtual Address origToReloc(Address orig) const {
    (void)orig; // unused
    assert(orig == orig_);
    return reloc_;
  }

  virtual type_t type() const { return TrackerElement::padding; }
  unsigned pad() const { return pad_; }

  virtual bool mergeable() const { return false; }

 private:
  unsigned pad_; 
};

class CodeTracker {
 public:
  struct RelocatedElements {
     Address instruction;
     std::map<instPoint *, Address> instrumentation;
     Address pad;
  RelocatedElements() : instruction(0), pad(0) {}
  };

  // I'd like to use a block * as a unique key element, but
  // really can't because blocks can be deleted and recreated.
  // Instead, I'm using their entry address - that's the address
  // in BlockForwardsMap. 
  // The ForwardsMap address is a straightforward "what's the data at this
  // particular address".

  typedef Address FunctionEntryID;
  typedef Address BlockEntryID;

  typedef std::list<TrackerElement *> TrackerList;
  typedef std::map<Address, RelocatedElements> FwdMapInner;
  typedef std::map<FunctionEntryID, FwdMapInner> FwdMapMiddle;
  typedef std::map<BlockEntryID, FwdMapMiddle> ForwardMap;

  typedef class IntervalTree<Address, TrackerElement *> ReverseMap;


  CodeTracker();

  ~CodeTracker();

  static CodeTracker *fork(CodeTracker *parent, AddressSpace *child);

  struct RelocInfo {
     Address orig;
     Address reloc;
     block_instance *block;
     func_instance *func;
     baseTramp *bt;
     unsigned pad;
  RelocInfo() : orig(0), reloc(0), block(NULL), func(NULL), bt(NULL), pad(0) {}
  };

  bool origToReloc(Address origAddr, block_instance *block, func_instance *func, RelocatedElements &relocs) const;
  bool relocToOrig(Address relocAddr, RelocInfo &ri) const;

  TrackerElement *findByReloc(Address relocAddr) const;

  Address lowOrigAddr() const;
  Address highOrigAddr() const;
  Address lowRelocAddr() const;
  Address highRelocAddr() const;

  void addTracker(TrackerElement *);

  void createIndices();

  void debug();

  const TrackerList &trackers() { return trackers_; }

 private:

  // We make this block specific to handle shared
  // code
  ForwardMap origToReloc_;
  ReverseMap relocToOrig_;

  TrackerList trackers_;
};

}
}


#endif
