/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: infHeap.h,v 1.2 2005/08/05 22:23:07 bernat Exp $
 * Inferior heap primitives, moved from process.h to ease compilation
 */

#if !defined(infHeap_h)
#define infHeap_h

#include "common/h/String.h"
#include "common/h/vectorSet.h"
#include "common/h/Dictionary.h"
#include "common/h/Types.h"

typedef enum { HEAPfree, HEAPallocated } heapStatus;
// Bit pattern...
typedef enum { textHeap=0x01,
               dataHeap=0x02,
               uncopiedHeap=0x04, // AIX -- not copied on fork
               anyHeap=0x7, // OR of the previous three
               lowmemHeap=0x1000 }
        inferiorHeapType;
typedef pdvector<Address> addrVecType;

class heapItem {
 public:
  heapItem() : 
    addr(0), length(0), type(anyHeap), dynamic(true), status(HEAPfree) {}
  heapItem(Address a, int n, inferiorHeapType t, 
           bool d = true, heapStatus s = HEAPfree) :
    addr(a), length(n), type(t), dynamic(d), status(s) {}
  heapItem(const heapItem *h) :
    addr(h->addr), length(h->length), type(h->type), 
    dynamic(h->dynamic), status(h->status) {}
  heapItem(const heapItem &h) :
    addr(h.addr), length(h.length), type(h.type), 
    dynamic(h.dynamic), status(h.status) {}
  heapItem &operator=(const heapItem &src) {
    addr = src.addr;
    length = src.length;
    type = src.type;
    dynamic = src.dynamic;
    status = src.status;
    return *this;
  }

  Address addr;
  unsigned length;
  inferiorHeapType type;
  bool dynamic; // part of a dynamically allocated segment?
  heapStatus status;
};


// disabledItem: an item on the heap that we are trying to free.
// "pointsToCheck" corresponds to predecessor code blocks
// (i.e. prior minitramp/basetramp code)
class disabledItem {
 public:
  disabledItem() : block() {}

  disabledItem(heapItem *h, const pdvector<addrVecType> &preds) :
    block(h), pointsToCheck(preds) {}
  disabledItem(const disabledItem &src) :
    block(src.block), pointsToCheck(src.pointsToCheck) {}

  // TODO: unused?
  disabledItem(Address ip, inferiorHeapType iht,
               const pdvector<addrVecType> &ipts) :
    block(ip, 0, iht), pointsToCheck(ipts) { 
    bperr( "error: unused disabledItem ctor\n");
    assert(0);
  }
  disabledItem &operator=(const disabledItem &src) {
    if (&src == this) return *this; // check for x=x    
    block = src.block;
    pointsToCheck = src.pointsToCheck;
    return *this;
  }


 ~disabledItem() {}
  
  heapItem block;                    // inferior heap block
  pdvector<addrVecType> pointsToCheck; // list of addresses to check against PCs

  Address getPointer() const {return block.addr;}
  inferiorHeapType getHeapType() const {return block.type;}
  const pdvector<addrVecType> &getPointsToCheck() const {return pointsToCheck;}
  pdvector<addrVecType> &getPointsToCheck() {return pointsToCheck;}
};

/* Dyninst heap class */
/*
  This needs a better name. Contains a name, and address, and a size.
  Any ideas?
*/
class heapDescriptor {
 public:
  heapDescriptor(const pdstring name,
		 Address addr,
		 unsigned int size,
		 const inferiorHeapType type):
    name_(name),addr_(addr),size_(size), type_(type) {}
  heapDescriptor():
    name_(pdstring("")),addr_(0),size_(0),type_(anyHeap) {}
  ~heapDescriptor() {}
  heapDescriptor &operator=(const heapDescriptor& h)
    {
      name_ = h.name();
      addr_ = h.addr();
      size_ = h.size();
      type_ = h.type();
      return *this;
    }
  const pdstring &name() const {return name_;}
  const Address &addr() const {return addr_;}
  const unsigned &size() const {return size_;}
  const inferiorHeapType &type() const {return type_;}
 private:
  pdstring name_;
  Address addr_;
  unsigned size_;
  inferiorHeapType type_;
};


class inferiorHeap {
 public:
    void clear();
    
  inferiorHeap(): heapActive(addrHash16) {
      freed = 0; disabledListTotalMem = 0; totalFreeMemAvailable = 0;
  }
  inferiorHeap(const inferiorHeap &src);  // create a new heap that is a copy
                                          // of src (used on fork)
  dictionary_hash<Address, heapItem*> heapActive; // active part of heap 
  pdvector<heapItem*> heapFree;           // free block of data inferior heap 
  pdvector<disabledItem> disabledList;    // items waiting to be freed.
  int disabledListTotalMem;             // total size of item waiting to free
  int totalFreeMemAvailable;            // total free memory in the heap
  int freed;                            // total reclaimed (over time)

  pdvector<heapItem *> bufferPool;        // distributed heap segments -- csserra
};
 
#endif
