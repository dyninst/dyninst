#include "Buffer.h"
#include <stdlib.h>

using namespace Dyninst;
using namespace PatchAPI;

const int Buffer::ALLOCATION_UNIT = 256;


Buffer::Buffer(Address addr, unsigned initial_size) : size_(initial_size), max_(initial_size + ALLOCATION_UNIT), start_(addr) {
   buffer_ = (unsigned char *)::malloc(max_);
};
Buffer::~Buffer() {
   assert(buffer_);
   free(buffer_);
};
unsigned Buffer::size() const {
   return size_;
};
unsigned Buffer::max_size() const {
   return max_;
};
bool Buffer::empty() const {
   return (size_ == 0);
};

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
   namespace PatchAPI {
      
      template<>
      void Buffer::copy(unsigned char *begin, unsigned char *end) {
         unsigned added_size = (long)end - (long)begin;
         if ((size_ + added_size) > max_) 
            increase_allocation(added_size);
         memcpy(cur_ptr(), begin, added_size);
      }
   }
}   
