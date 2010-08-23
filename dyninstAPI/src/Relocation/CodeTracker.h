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

// Keeps a list of relocated Atoms (compessed where possible) for mapping
// addresses and types between original and relocated code. 

#if !defined(_R_CODE_TRACKER_H_)
#define _R_CODE_TRACKER_H_

#include "dynutil/h/dyntypes.h"
#include <vector>
#include <set>
#include <list>
#include "common/h/IntervalTree.h"

class bblInstance;
class baseTrampInstance;

namespace Dyninst {
namespace Relocation {
class CodeTracker;

class TrackerElement {
  friend class CodeTracker;
 public:
  typedef enum {
    original,
    emulated,
    instrumentation
  } type_t;
 TrackerElement(Address o, unsigned s = 0) :
  orig_(o), reloc_(0), size_(s), block_(NULL) {assert(o);};
  virtual ~TrackerElement() {};

  virtual Address relocToOrig(Address reloc) const = 0;
  virtual Address origToReloc(Address orig) const = 0;
  virtual type_t type() const = 0;

  Address orig() const { return orig_; };
  Address reloc() const { return reloc_; };
  unsigned size() const { return size_; };
  bblInstance *block() const { return block_; };

  void setReloc(Address reloc) { reloc_ = reloc; };
  void setSize(unsigned size) { size_ = size; }
  void setBlock(bblInstance *b) { block_ = b; }

 protected:
  TrackerElement() {};
  Address orig_;
  Address reloc_;
  unsigned size_;
  bblInstance *block_;
};

class OriginalTracker : public TrackerElement {
 public:
 OriginalTracker(Address orig, unsigned size = 0) :
  TrackerElement(orig, size) {};
  virtual ~OriginalTracker() {};

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

  virtual type_t type() const { return TrackerElement::original; };

 private:
};

class EmulatorTracker : public TrackerElement {
 public:
 EmulatorTracker(Address orig, unsigned size = 0) : 
  TrackerElement(orig, size) {};
  virtual ~EmulatorTracker() {};

  virtual Address relocToOrig(Address reloc) const {
    assert(reloc >= reloc_);
    assert(reloc < (reloc_ + size_));
    return orig_;
  }

  virtual Address origToReloc(Address orig) const {
    assert(orig == orig_);
    return reloc_;
  }

  virtual type_t type() const { return TrackerElement::emulated; };

 private:
};

class InstTracker : public TrackerElement {
 public:
 InstTracker(Address orig, baseTrampInstance *baseT) :
  TrackerElement(orig), baseT_(baseT) {};
  virtual ~InstTracker() {};

  virtual Address relocToOrig(Address reloc) const {
    assert(reloc >= reloc_);
    assert(reloc < (reloc_ + size_));
    return orig_;
  }

  virtual Address origToReloc(Address orig) const {
    assert(orig == orig_);
    return reloc_;
  }

  virtual type_t type() const { return TrackerElement::instrumentation; };
  baseTrampInstance *baseT() const { return baseT_; };
 private:
  baseTrampInstance *baseT_;
};

class CodeTracker {
 public:
  typedef std::list<TrackerElement *> TrackerList;
  typedef class IntervalTree<Address, TrackerElement *> NonBlockRange;
  typedef std::map<bblInstance *, NonBlockRange> BlockRange;


  CodeTracker() {};
  ~CodeTracker() {};

  bool origToReloc(Address origAddr, bblInstance *origBBL, Address &reloc) const;
  bool relocToOrig(Address relocAddr, Address &orig, bblInstance *&origBBL) const;

  baseTrampInstance *getBaseT(Address relocAddr) const;

  Address lowOrigAddr() const;
  Address highOrigAddr() const;
  Address lowRelocAddr() const;
  Address highRelocAddr() const;

  void addTracker(TrackerElement *);

  void createIndices();

  void debug();


 private:

  BlockRange origToReloc_;
  NonBlockRange relocToOrig_;

  TrackerList trackers_;
};

};
};

std::ostream &
operator<<(std::ostream &os, const Dyninst::Relocation::TrackerElement &e);

#endif
