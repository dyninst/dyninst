/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/* $Id: shmMgr.C,v 1.5 2002/06/10 19:45:03 bernat Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#include <iostream.h>
#include "shmMgr.h"
#include "shmSegment.h"

const unsigned shmMgr::cookie = 0xabcdefab;

shmMgr::shmMgr()
{
}

shmMgr::shmMgr(process *proc, key_t shmSegKey, unsigned shmSize_) :
  shmSize(shmSize_), baseAddrInDaemon(0), baseAddrInApplic(0),
  num_allocated(0)
{
  // Try to allocate shm segment now
  key_t key = shmSegKey;

  theShm = ShmSegment::Create( key, shmSize);
  if( theShm == NULL )
  {
    cerr << "  we failed to create the shared memory segment\n";
    return;
  }
  keyUsed = key;

  // Now, let's initialize some meta-data: cookie, process pid, paradynd pid,
  // cost.
  baseAddrInDaemon = reinterpret_cast<Address>(theShm->GetMappedAddress());
  unsigned *ptr = reinterpret_cast<unsigned *>(baseAddrInDaemon);
  *ptr++ = cookie;
  *ptr++ = (unsigned)proc;
  *ptr++ = (unsigned)getpid();     // paradynd pid
  *ptr++ = 0;            // initialize observed cost
#if defined(MT_THREAD)
  // HACK, FIX!
  ptr += 4096;
#endif
  Address endAddr = reinterpret_cast<Address>(ptr);
  freespace = shmSize - (endAddr - baseAddrInDaemon);  
  highWaterMark = endAddr;
}

shmMgr::~shmMgr()
{
  assert(num_allocated == 0);
  for (unsigned i = 0; i < prealloc.size(); i++)
    delete prealloc[i];
}

static unsigned align(unsigned num, unsigned alignmentFactor) {
  unsigned retnum;
  if (num % alignmentFactor != 0)
    retnum = num - (alignmentFactor - (num % alignmentFactor));

  assert(retnum % alignmentFactor == 0);
  return retnum;
}

Address shmMgr::malloc(unsigned size) {
  if (freespace < size)
    return 0;
  num_allocated++;
  // Next, check to see if this size matches any of the preallocated
  // chunks
  for (unsigned i = 0; i < prealloc.size(); i++)
    if ((size == prealloc[i]->size_) &&
	(prealloc[i]->oneAvailable()))
      return prealloc[i]->malloc();

  // Grump. Nothing available.. so do it the hard way
  // Cheesed, again
  Address retAddr = highWaterMark;
  highWaterMark += size;
  return retAddr;
}

void shmMgr::free(Address addr) 
{
  // First, check if the addr is within any of the preallocated
  // chunks
  for (unsigned i = 0; i < prealloc.size(); i++)
    if ((addr >= prealloc[i]->baseAddr_) &&
	(addr < (prealloc[i]->baseAddr_ + (prealloc[i]->size_*prealloc[i]->numElems_)))) {
      prealloc[i]->free(addr);
      break;
    }
  num_allocated--;
  // Otherwise ignore (for now)
}

// Preallocates a chunk of memory for a certain number of elements,
// all of size 'size'. This is a mechanism for speeding up common
// cases for malloc and free.
// Question... is allocating more than one 'chunk' with a given 
// data size allowable? For now I'll say yes.

void shmMgr::preMalloc(unsigned size, unsigned num)
{
  Address baseAddr = this->malloc(size*num);
  shmMgrPreallocInternal *new_prealloc = new shmMgrPreallocInternal(size, num, baseAddr);
  prealloc.push_back(new_prealloc);
}

shmMgrPreallocInternal::shmMgrPreallocInternal(unsigned size, unsigned num, Address baseAddr)
{
  baseAddr_ = baseAddr;
  size_ = size;
  numElems_ = num;
  currAlloc_ = 0;
  // Round up to 8 for purposes of bitmapping.
  unsigned rounded_num = num;
  if (rounded_num % 8) {
    rounded_num = num + 8;
    rounded_num -= (rounded_num % 8);
  }
  bitmap_size_ = rounded_num/8;
  assert((bitmap_size_ * 8) >= rounded_num);
  bitmap_ = new char[bitmap_size_];
  for (unsigned i = 0; i < rounded_num/8; i++)
    bitmap_[i] = 0;
  // If num wasn't a multiple of 8, mark the final slots as taken.
  if (rounded_num != num) {
    // rounded is going to be bigger...
    int difference = rounded_num - num;
    int counter = 7;
    while (difference > 0) {
      bitmap_[(rounded_num/8)-1] += 1 << counter;
      counter--;
      difference--;
    }
  }
}

shmMgrPreallocInternal::~shmMgrPreallocInternal()
{
  assert(currAlloc_ == 0);
  delete [] bitmap_;
}

bool shmMgrPreallocInternal::oneAvailable()
{
  return (currAlloc_ < numElems_);
}

Address shmMgrPreallocInternal::malloc()
{
  if (!oneAvailable()) return 0;
  // Well, there's one here... let's try and find it. Scan the bitmaps
  // for one that is less than 0xff
  unsigned next_free_block = 0;
  for (next_free_block = 0; next_free_block < bitmap_size_; next_free_block++)
    if (bitmap_[next_free_block] < 0xff)
      break;
  assert(next_free_block < bitmap_size_); // Should have been bounced by oneAvailable
  // Next: find slot within that is empty
  unsigned next_free_slot = 0;
  for (next_free_slot = 0; next_free_slot < 8; next_free_slot++)
    if (!(bitmap_[next_free_block] &(0x1 << next_free_slot))) // logical AND, right?
      break;
  // Okay, we have our man... mark it as active, decrease available count,
  // and calculate the return address
  bitmap_[next_free_block] += 0x1 << next_free_slot;
  currAlloc_++;
  Address retAddr = baseAddr_ + (((next_free_block * 8) + next_free_slot) * size_);
  return retAddr;
}

void shmMgrPreallocInternal::free(Address addr)
{
  // Reverse engineer allocation, above...
  unsigned freed_block;
  unsigned freed_slot;
  freed_block = ((addr - baseAddr_)/size_)/8;
  freed_slot =  ((addr - baseAddr_)/size_) % 8;
  assert(bitmap_[freed_block] & (0x1 << freed_slot));
  bitmap_[freed_block] = bitmap_[freed_block] - (0x1 << freed_slot);
  currAlloc_--;
}
