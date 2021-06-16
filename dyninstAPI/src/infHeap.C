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

// $Id: infHeap.C,v 1.2 2008/02/07 16:07:55 jaw Exp $

#include "infHeap.h"

using namespace Dyninst;

// create a new inferior heap that is a copy of src. This is used when a process
// we are tracing forks.
inferiorHeap::inferiorHeap(const inferiorHeap &src)
{
    for (unsigned u1 = 0; u1 < src.heapFree.size(); u1++) {
      heapFree.push_back(new heapItem(src.heapFree[u1]));
    }

    for (auto iter = src.heapActive.begin(); iter != src.heapActive.end(); ++iter) {
       heapActive[iter->first] = new heapItem(*(iter->second));
    }

    for (unsigned u3 = 0; u3 < src.disabledList.size(); u3++) {
      disabledList.push_back(src.disabledList[u3]);
    }

    for (unsigned u4 = 0; u4 < src.bufferPool.size(); u4++) {
      bufferPool.push_back(new heapItem(src.bufferPool[u4]));
    }

    disabledListTotalMem = src.disabledListTotalMem;
    totalFreeMemAvailable = src.totalFreeMemAvailable;
    freed = 0;
}

inferiorHeap& inferiorHeap::operator=(const inferiorHeap &src)
{
    clear();
    for (unsigned u1 = 0; u1 < src.heapFree.size(); u1++) {
      heapFree.push_back(new heapItem(src.heapFree[u1]));
    }

    for (auto iter = src.heapActive.begin(); iter != src.heapActive.end(); ++iter) {
       heapActive[iter->first] = new heapItem(*(iter->second));
    }

    for (unsigned u3 = 0; u3 < src.disabledList.size(); u3++) {
      disabledList.push_back(src.disabledList[u3]);
    }

    for (unsigned u4 = 0; u4 < src.bufferPool.size(); u4++) {
      bufferPool.push_back(new heapItem(src.bufferPool[u4]));
    }

    disabledListTotalMem = src.disabledListTotalMem;
    totalFreeMemAvailable = src.totalFreeMemAvailable;
    freed = 0;

    return *this;
}


// For exec/process deletion
void inferiorHeap::clear() {
    for (auto iter = heapActive.begin(); iter != heapActive.end(); ++iter) {
       delete iter->second;
    }
    heapActive.clear();
    
    for (unsigned i = 0; i < heapFree.size(); i++)
        delete heapFree[i];
    heapFree.clear();

    disabledList.clear();

    disabledListTotalMem = 0;
    totalFreeMemAvailable = 0;
    freed = 0;

    for (unsigned j = 0; j < bufferPool.size(); j++)
        delete bufferPool[j];
    bufferPool.clear();
}

int heapItemCmpByAddr(const heapItem **A, const heapItem **B)
{
  heapItem *a = *(heapItem **)const_cast<heapItem **>(A);
  heapItem *b = *(heapItem **)const_cast<heapItem **>(B);

  if (a->addr < b->addr) {
      return -1;
  } else if (a->addr > b->addr) {
      return 1;
  } else {
      return 0;
  }
}

