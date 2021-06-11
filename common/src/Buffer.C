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
#include "Buffer.h"
#include <stdlib.h>

using namespace Dyninst;

const int Buffer::ALLOCATION_UNIT = 256;

Buffer::Buffer() : buffer_(NULL), size_(0), max_(0), start_(0) {}

Buffer::Buffer(Address addr, unsigned initial_size) : buffer_(NULL), size_(0), max_(0), start_(0) {
   initialize(addr, initial_size);
}

Buffer::~Buffer() {
   assert(buffer_);
   free(buffer_);
}

void Buffer::initialize(Address a, unsigned s) {
   assert(buffer_ == NULL);
   start_ = a;
   increase_allocation(s);
}
   

unsigned Buffer::size() const {
   return size_;
}
unsigned Buffer::max_size() const {
   return max_;
}
bool Buffer::empty() const {
   return (size_ == 0);
}

void Buffer::increase_allocation(int size) {
   if (size <= 0) return;
   // Round size up to the next allocation unit
   size = ((size / ALLOCATION_UNIT) + 1) * ALLOCATION_UNIT;
   max_ += size;
   buffer_ = (unsigned char *)::realloc(buffer_, max_);
   assert(buffer_);
}

unsigned char *Buffer::cur_ptr() const {
   return &(buffer_[size_]);
}

Buffer::byte_iterator Buffer::begin() const {
   return byte_iterator(buffer_);
}

Buffer::byte_iterator Buffer::end() const {
   return byte_iterator(cur_ptr());
}

namespace Dyninst {
   template<>
   void Buffer::copy(unsigned char *begin, unsigned char *end) {
      unsigned added_size = (long)end - (long)begin;
      if ((size_ + added_size) > max_) 
         increase_allocation(added_size);
      memcpy(cur_ptr(), begin, added_size);
      size_ += added_size;
   }
}   

void Buffer::copy(void *buf, unsigned size) {
   unsigned char *begin = (unsigned char *) buf;
   unsigned char *end = begin + size;

   copy(begin, end);
}
   
