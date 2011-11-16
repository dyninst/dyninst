#if !defined(PATCHAPI_H_BUFFER_H_)
#define PATCHAPI_H_BUFFER_H_

#include <assert.h>

namespace Dyninst {
namespace PatchAPI {

// A class to support multiple forms of code generation. The design of this class is as 
// a tiered model:

// Tier 1: A buffer of bytes that represent position-dependent executable code
// Tier 2: A buffer that supports linker-style relocations of raw instructions
// Tier 3: A buffer that supports relocatable and optimizer-friendly instruction objects

// The current implementation supports tier 1. In that, it is a cut-down version of the
// Dyninst internal codeGen structure that aims to be more user-friendly. Tiers 2 and 3 
// are TODO. 

class Buffer {
  public:
   Buffer();
   Buffer(unsigned initial_size);
   ~Buffer();

   static const int ALLOCATION_UNIT;

   template <class InputIterator>
      void copy(InputIterator begin, InputIterator end);

   unsigned size() const;
   unsigned max_size() const;
   bool empty() const;

   template <class Input>
      void push_back(const Input &);

   template <class storage>
      class iterator {
      friend class Buffer;

     public:
     iterator() : pos((storage *)-1) {};
      ~iterator() {};
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
      iterator<storage> operator++() {
         assert(valid);
         iterator<storage> i = *this;
         ++pos;
         return i;
      }
         
      iterator<storage> operator++(int) {
         assert(valid);
         ++pos;
         return *this;
      }

     private:
      bool valid() const { return pos != (storage *)-1; };
     iterator(storage *start) : pos(start) {};
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

  private:
   // May call realloc();
   void increase_allocation();
   unsigned char * cur_ptr() const;

   unsigned char * buffer_;
   unsigned size_;
   unsigned max_;
};

template <class InputIterator> 
   void copy(InputIterator begin, InputIterator end) {
   while (begin != end) {
      push_back(*begin);
      ++begin;
   }
};

template <class Input> 
   void Buffer::push_back(const Input &i) {
   while (size_ + sizeof(i) >= max_) {
      increase_allocation();
   }
   Input *ptr = (Input *)cur_ptr();
   *ptr = i;
   size_ += sizeof(i);
};

}
}

#endif
