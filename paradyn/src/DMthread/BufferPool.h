#if defined(external_templates)
#pragma interface
#endif

#ifndef _BUFFER_POOL_H_
#define _BUFFER_POOL_H_


#include "util/h/Vector.h"

template <class BUFFERITEM>
class BufferPool {
 private:
 public:
   BufferPool();
  ~BufferPool();

   vector<BUFFERITEM> *alloc(unsigned req_size);
   void dealloc(vector<BUFFERITEM> *);
};

template <class BUFFERITEM>
BufferPool<BUFFERITEM>::BufferPool() {
}

template <class BUFFERITEM>
BufferPool<BUFFERITEM>::~BufferPool() {
}

template <class BUFFERITEM>
vector<BUFFERITEM> *
BufferPool<BUFFERITEM>::alloc(unsigned req_size) {
   vector <BUFFERITEM> *result = new vector<BUFFERITEM>(req_size);
   assert(result);
   return result; // returning references is nicer than returning pointers
}

template <class BUFFERITEM>
void 
BufferPool<BUFFERITEM>::dealloc(vector<BUFFERITEM> *buffer) {
   delete buffer;
}

#endif
