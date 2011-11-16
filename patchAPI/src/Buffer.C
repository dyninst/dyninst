#include "Buffer.h"
#include <stdlib.h>

using namespace Dyninst;
using namespace PatchAPI;

const int Buffer::ALLOCATION_UNIT = 256;


Buffer::Buffer() : size_(), max_(ALLOCATION_UNIT) {
   buffer_ = (unsigned char *)::malloc(max_);
   assert(buffer_);
};
Buffer::Buffer(unsigned initial_size) : size_(initial_size), max_(initial_size + ALLOCATION_UNIT) {
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
   return (size_ > 0);
};

void Buffer::increase_allocation() {
   max_ += ALLOCATION_UNIT;
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


