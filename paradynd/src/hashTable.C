/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "hashTable.h"

hashTable::hashTable(unsigned size, 
		     unsigned initial_free_size,
		     unsigned initial_free_elem) : mapTable(CThash)
{
  tableSize = size;
  for (unsigned i=0; i<size; i++) {
    tableUsage += false;
  }
  for (unsigned i=0; i<initial_free_size; i++) {
    freeList += initial_free_size-1-i+initial_free_elem;
  }
}

hashTable::hashTable(hashTable *src) : mapTable(src->mapTable)
{
  assert(src);
  tableSize = src->tableSize;
  for (unsigned i=0; i<tableSize; i++) {
    tableUsage += src->tableUsage;
  }
  for (unsigned i=0; i<src->freeList.size(); i++) {
    freeList += src->freeList[i];
  }
}

hashTable::~hashTable() {}

void hashTable::addToFreeList(unsigned from, unsigned how_many)
{
  for (unsigned i=from; i<from+how_many; i++) {
    freeList += i;
    assert(tableUsage[i]==false);
  }
}

bool hashTable::add(unsigned id, unsigned &position)
{
    unsigned tmpSize;
    if (freeList.size() == 0)  {
      logLine("*** hashTable::add, freeList.size() == 0 ...\n") ;
      return(false);
    }
    if (mapTable.defines(id)) {
      logLine("*** hashTable::add, mapTable.defines(id) ...\n") ;
      assert(0) ;
      return false ;
    }
    mapTable[id] = freeList[freeList.size()-1];
    position = mapTable[id];
    tmpSize = freeList.size();
    freeList.resize(tmpSize-1);
    assert(!tableUsage[position]);
    tableUsage[position] = true;
    return(true);
}

void hashTable::remove(unsigned id)
{
    unsigned position;
    assert(mapTable.defines(id));
    position = mapTable[id];
    assert(position < tableUsage.size());
    assert(tableUsage[position]);
    tableUsage[position] = false;
    vector<unsigned> tempList ;

    //prepend to the front
    tempList += position ;
    for (unsigned k=0; k< freeList.size(); k++) {
      tempList += freeList[k] ;
    }
    freeList = tempList ;
    mapTable.undef(id);
}


