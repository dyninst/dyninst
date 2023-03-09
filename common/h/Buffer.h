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
#if !defined(_BUFFER_H_)
#define _BUFFER_H_

#include <assert.h>
#include <string.h>
#include "util.h"
#include "dyntypes.h"
#include "unaligned_memory_access.h"
namespace Dyninst {

// A class to support multiple forms of code generation. The design of this class is as 
// a tiered model:

// Tier 1: A buffer of bytes that represent position-dependent executable code
// Tier 2: A buffer that supports linker-style relocations of raw instructions
// Tier 3: A buffer that supports relocatable and optimizer-friendly instruction objects

// The current implementation supports tier 1. In that, it is a cut-down version of the
// Dyninst internal codeGen structure that aims to be more user-friendly. Tiers 2 and 3 
// are TODO. 

class COMMON_EXPORT Buffer {
  public:
   Buffer(Address addr, unsigned initial_size);
   Buffer();
   void initialize(Address addr, unsigned initial_size);
   ~Buffer();

   static const int ALLOCATION_UNIT;

   template <class InputIterator>
      void copy(InputIterator begin, InputIterator end);
   void copy(void *buffer, unsigned size);

   unsigned size() const;
   unsigned max_size() const;
   bool empty() const;

   template <class Input>
      void push_back(const Input &);

   template <class storage>
      class iterator {
      friend class Buffer;

     public:
     iterator() : pos((storage *)-1) {}
      ~iterator() {}
      iterator(const iterator&) = default;
      storage operator*() const {
         assert(valid);
         return *pos;
      }

      bool operator==(const iterator<storage> &rhs) const {
         return rhs.pos == pos; 
      }
      bool operator!=(const iterator<storage> &rhs) const {
         return rhs.pos != pos;
      }
      iterator<storage> operator++() { // prefix
         assert(valid);
         ++pos;
         return *this;
      }
         
      iterator<storage> operator++(int) { // postfix
         assert(valid);
         iterator<storage> i = *this;
         ++pos;
         return i;
      }

     private:
      bool valid() const { return pos != (storage *)-1; }
     iterator(storage *start) : pos(start) {}
      storage *pos;
   };

   typedef iterator<unsigned char> byte_iterator;
   typedef iterator<unsigned int> word_iterator;
   typedef iterator<unsigned long> long_iterator;

   byte_iterator begin() const;
   byte_iterator end() const;

   word_iterator w_begin() const;
   word_iterator w_end() const;
   
   long_iterator l_begin() const;
   long_iterator l_end() const;

   unsigned char *start_ptr() const { return buffer_; }

   Address startAddr() const { return start_; }
   Address curAddr() const { return start_ + size_; }

  private:
   // May call realloc();
   void increase_allocation(int added);
   unsigned char * cur_ptr() const;

   unsigned char * buffer_;
   unsigned size_;
   unsigned max_;

   Address start_;
};

template <class InputIterator> 
   void Buffer::copy(InputIterator begin, InputIterator end) {
   while (begin != end) {
      push_back(*begin);
      ++begin;
   }
}



template <class Input> 
   void Buffer::push_back(const Input &i) {
   if (size_ + sizeof(i) >= max_) {
      increase_allocation(sizeof(i));
   }
   write_memory_as(cur_ptr(), i);
   size_ += sizeof(i);
}

}

#endif
