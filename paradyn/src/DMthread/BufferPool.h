/*
 * $Log: BufferPool.h,v $
 * Revision 1.2  1995/12/08 05:47:53  tamches
 * added cvs log; added some comments
 *
 */

#ifndef _BUFFER_POOL_H_
#define _BUFFER_POOL_H_

#if defined(external_templates)
#pragma interface
#endif

#include "util/h/Vector.h"

/*
 * Using a BufferPool:
 *
 * 1) producer calls alloc() to obtain a buffer to work with, passing
 *    a desired buffer size (num-entries)
 * 2) producer fills this buffer as desired
 * 3) producer hands the (pointer to) buffer off to the consumer.
 *    note: the producer must _never_ use the buffer after this point!!!
 * 4) the consumer consumes the buffer as desired
 * 5) the consumer calls dealloc() on the (pointer to) buffer.
 *    note: now the consumer must _never_ use the buffer after this point!!!
 */

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
   return result;
}

template <class BUFFERITEM>
void 
BufferPool<BUFFERITEM>::dealloc(vector<BUFFERITEM> *buffer) {
   delete buffer;
}

#endif
