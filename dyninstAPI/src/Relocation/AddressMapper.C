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

#include "AddressMapper.h"

#include <iostream>

using namespace Dyninst;
using namespace std;

bool AddressMapper::origToReloc(Address origAddr, Address &relocAddr) const {
  entry e;
  if (!origToReloc_.find(origAddr, e))
    return false;
  relocAddr = e.map(origAddr); 
  return true;
}

bool AddressMapper::relocToOrig(Address relocAddr, Address &origAddr) const {
  entry e;
  if (!relocToOrig_.find(relocAddr, e))
    return false;
  origAddr = e.map(relocAddr); 
  return true;
}

bool AddressMapper::createTrees(accumulatorList &accum, Address relocBase) {
  entry current;

  for (accumulatorList::const_iterator iter = accum.begin(); iter != accum.end(); ++iter) {
    const accumulator &a = *iter;

    if (a.origSize == a.relocSize) {
      // An equivalence entry. Add to current. 
      if (current.empty()) {
	current = entry(a.origAddr, 
			a.origSize,
			a.relocOffset + relocBase,
			a.relocSize);
      }
      else if ((current.fromAddr + current.fromSize) != a.origAddr) {
	// Cap off current and restart
	addEquivalentRange(current.fromAddr, current.toAddr, current.fromSize);
	
	current = entry(a.origAddr,
			a.origSize,
			a.relocOffset + relocBase,
			a.relocSize);
      }
      else {
	current.fromSize += a.origSize;
	current.toSize = a.relocSize;
      }
    }
    else {
      // Cap off the current one, insert a collapse range, then start a 
      // new current
      if (!current.empty()) {
	addEquivalentRange(current.fromAddr, current.toAddr, current.fromSize);
	current = entry();
      }
      addInsertedRange(a.origAddr, a.relocOffset + relocBase, a.origSize, a.relocSize);
    }
  }
  if (!current.empty()) {
    addEquivalentRange(current.fromAddr, current.toAddr, current.fromSize);
  }
  return true;
}

void AddressMapper::debug() {
  cerr << "************ FORWARD MAPPING ****************" << endl;
  std::vector<std::pair<std::pair<Address, Address>, entry> > elements;
  origToReloc_.elements(elements);
  for (unsigned i = 0; i < elements.size(); ++i) {
    cerr << elements[i].second << endl;
  }
  cerr << endl;
  cerr << "************ BACKWARD MAPPING ****************" << endl;
  elements.clear();
  relocToOrig_.elements(elements);
  for (unsigned i = 0; i < elements.size(); ++i) {
    cerr << elements[i].second << endl;
  }
  cerr << endl;
}
    
bool AddressMapper::addEquivalentRange(Address baseOrigAddr, 
				      Address baseRelocAddr,
				      unsigned size) {

  entry forward(baseOrigAddr, size, baseRelocAddr, size);
  origToReloc_.insert(baseOrigAddr,
		      baseOrigAddr + size,
		      forward);

  entry backward(baseRelocAddr, size, baseOrigAddr, size);
  relocToOrig_.insert(baseRelocAddr,
		      baseRelocAddr + size,
		      backward);
  return true;
}

bool AddressMapper::addInsertedRange(Address baseOrigAddr, 
				    Address baseRelocAddr,
				    unsigned origSize,
				     unsigned relocSize)  {

  entry forward(baseOrigAddr, origSize, baseRelocAddr, relocSize);
  origToReloc_.insert(baseOrigAddr,
		      baseOrigAddr + origSize,
		      forward);

  entry backward(baseRelocAddr, relocSize, baseOrigAddr, origSize);
  relocToOrig_.insert(baseRelocAddr,
		      baseRelocAddr + relocSize,
		      backward);
  return true;
}

std::ostream &operator<<(std::ostream &os, const AddressMapper::entry &e) {
  os << hex << "("
     << e.fromAddr << ".." << e.fromAddr + e.fromSize << "->"
     << e.toAddr << ".." << e.toAddr + e.toSize << ")" << dec;
  return os;
}

