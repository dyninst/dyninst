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

// Maps between original program addresses and their relocated equivalents.
// Useful for translating looked up addresses (e.g., stack walks).
// Serves as a replacement for the multiTramp/baseTrampInstance/miniTrampInstance
// tree (in their role as address lookup/codeRangeTree entries).

//
// Background: the vast majority of our code relocation simply copies each
// instruction over untouched; so in many cases the appropriate way
// to map addresses in a block [x..y] is [x+n..y+n]. In rarer cases, we
// replace a single instruction with a sequence, or insert a new sequence.
// This results in a collection of ranges, with each range either mapping
// 1:1 or 1:N. So we write compact representations for those two, and 
// lump it all into a big IntervalTree to handle lookups. 

// Should be in Dyninst::Relocation, but isn't so that we
// can include it in a CodeRange tree.

#if !defined(_R_ADDRESS_MAPPER_H_)
#define _R_ADDRESS_MAPPER_H_

#include "dynutil/h/dyntypes.h"
#include <list>
#include "common/h/IntervalTree.h"


namespace Dyninst {

class AddressMapper {
 public:
  struct entry;

  struct accumulator;
  typedef std::list<accumulator> accumulatorList;

  AddressMapper() {};
  ~AddressMapper() {};

  bool origToReloc(Address origAddr, Address &relocAddr) const;
  bool relocToOrig(Address relocAddr, Address &origAddr) const;

  Address lowOrigAddr() const;
  Address highOrigAddr() const;
  Address lowRelocAddr() const;
  Address highRelocAddr() const;

  bool createTrees(accumulatorList &accum, Address relocBase);

  void debug();

  struct accumulator {
    Address origAddr;
    Offset relocOffset; // From start of code generation
    unsigned origSize;
    unsigned relocSize;
  accumulator(Address a, Offset b, unsigned c, unsigned d) :
    origAddr(a), relocOffset(b), origSize(c), relocSize(d) {};
  };


  bool addEquivalentRange(Address baseOrigAddr, Address baseRelocAddr, unsigned size);
  bool addInsertedRange(Address baseOrigAddr, Address baseRelocAddr, unsigned origSize, unsigned relocSize);

  struct entry {
  entry() : fromAddr(0), fromSize(0), toAddr(0), toSize(0) {};
  entry(Address a, unsigned b, Address c, unsigned d) :
    fromAddr(a), fromSize(b), toAddr(c), toSize(d) {};

    Address fromAddr;
    unsigned fromSize;
    Address toAddr;
    unsigned toSize;

    Address map(Address in) {
      assert(in >= fromAddr);
      assert(in < (fromAddr + fromSize));
      if (fromSize == toSize) {
	return (in - fromAddr + toAddr);
      }
      else {
	return toAddr;
      }
    };

    bool empty() const { return fromSize == 0; };

    bool operator==(const entry &rhs) {
      return ((fromAddr == rhs.fromAddr) &&
	      (fromSize == rhs.fromSize) &&
	      (toAddr == rhs.toAddr) &&
	      (toSize == rhs.toSize));
    };

  };


  typedef class IntervalTree<Address, entry> Ranges;

 private:
  // I think I need both here...
  Ranges origToReloc_;
  Ranges relocToOrig_;
};

};

std::ostream &
operator<<(std::ostream &os, const Dyninst::AddressMapper::entry &e);

#endif
