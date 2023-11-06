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

/* $Id: infHeap.h,v 1.8 2008/02/23 02:09:05 jaw Exp $
 * Inferior heap primitives, moved from process.h to ease compilation
 */

#if !defined(infHeap_h)
#define infHeap_h

#include <string>
#include <vector>
#include <unordered_map>
#include "dyntypes.h"
#include "common/h/util.h"
#include "util.h"

typedef enum { HEAPfree, HEAPallocated } heapStatus;
// Bit pattern...
typedef enum { textHeap=0x01,
               dataHeap=0x02,
               uncopiedHeap=0x04, // not copied on fork
               anyHeap=0x7, // OR of the previous three
               lowmemHeap=0x1000 }
        inferiorHeapType;
typedef std::vector<Dyninst::Address> addrVecType;

class heapItem {
 public:
  heapItem() : 
    addr(0), length(0), 
      type(anyHeap), dynamic(true), 
      status(HEAPfree),
      buffer(NULL) {}
  heapItem(Dyninst::Address a, int n,
           inferiorHeapType t, 
           bool d = true, 
           heapStatus s = HEAPfree) :
    addr(a), length(n), 
      type(t), dynamic(d), 
      status(s),
      buffer(NULL) {}
  heapItem(const heapItem *h) :
    addr(h->addr), length(h->length), 
      type(h->type), 
    dynamic(h->dynamic), status(h->status),
      buffer(h->buffer) {}
  heapItem(const heapItem &h) :
      addr(h.addr), length(h.length), type(h.type), 
      dynamic(h.dynamic), status(h.status),
      buffer(h.buffer) {}
  heapItem &operator=(const heapItem &src) {
    addr = src.addr;
    length = src.length;
    type = src.type;
    dynamic = src.dynamic;
    status = src.status;
    buffer = src.buffer;
    return *this;
  }

  void setBuffer(void *b) { buffer = b; }

  Dyninst::Address addr;
  unsigned length;
  inferiorHeapType type;
  bool dynamic; // part of a dynamically allocated segment?
  heapStatus status;


  // For local...
  void *buffer;
};


// disabledItem: an item on the heap that we are trying to free.
// "pointsToCheck" corresponds to predecessor code blocks
// (i.e. prior minitramp/basetramp code)
class disabledItem {
 public:
  disabledItem() noexcept : block() {}

  disabledItem(heapItem *h, const std::vector<addrVecType> &preds) :
    block(h), pointsToCheck(preds) {}
  disabledItem(const disabledItem &src) :
    block(src.block), pointsToCheck(src.pointsToCheck) {}

  disabledItem &operator=(const disabledItem &src) {
    if (&src == this) return *this; // check for x=x    
    block = src.block;
    pointsToCheck = src.pointsToCheck;
    return *this;
  }


 ~disabledItem() {}
  
  heapItem block;                    // inferior heap block
  std::vector<addrVecType> pointsToCheck; // list of addresses to check against PCs

  Dyninst::Address getPointer() const {return block.addr;}
  inferiorHeapType getHeapType() const {return block.type;}
  const std::vector<addrVecType> &getPointsToCheck() const {return pointsToCheck;}
  std::vector<addrVecType> &getPointsToCheck() {return pointsToCheck;}
};

/* Dyninst heap class */
/*
  This needs a better name. Contains a name, and address, and a size.
  Any ideas?
*/
class heapDescriptor {
 public:
  heapDescriptor(const std::string name,
		 Dyninst::Address addr,
		 unsigned int size,
		 const inferiorHeapType type):
    name_(name),addr_(addr),size_(size), type_(type) {}
  heapDescriptor():
    name_{},addr_{},size_{},type_(anyHeap) {}
  const std::string &name() const {return name_;}
  const Dyninst::Address &addr() const {return addr_;}
  const unsigned &size() const {return size_;}
  const inferiorHeapType &type() const {return type_;}
 private:
  std::string name_;
  Dyninst::Address addr_;
  unsigned size_;
  inferiorHeapType type_;
};


class inferiorHeap {
 public:
    void clear();
    
  inferiorHeap() {
      freed = 0; disabledListTotalMem = 0; totalFreeMemAvailable = 0;
  }
  inferiorHeap(const inferiorHeap &src);  // create a new heap that is a copy
                                          // of src (used on fork)
  inferiorHeap& operator=(const inferiorHeap &src);
  std::unordered_map<Dyninst::Address, heapItem*> heapActive; // active part of heap
  std::vector<heapItem*> heapFree;           // free block of data inferior heap 
  std::vector<disabledItem> disabledList;    // items waiting to be freed.
  int disabledListTotalMem;             // total size of item waiting to free
  int totalFreeMemAvailable;            // total free memory in the heap
  int freed;                            // total reclaimed (over time)

  std::vector<heapItem *> bufferPool;        // distributed heap segments -- csserra
};
 
#endif
