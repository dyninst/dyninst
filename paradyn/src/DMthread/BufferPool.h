/*
 * Copyright (c) 1996 Barton P. Miller
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

/* $Id: BufferPool.h,v 1.4 2000/07/28 17:21:41 pcroth Exp $ */

#ifndef _BUFFER_POOL_H_
#define _BUFFER_POOL_H_

#if defined(external_templates)
#pragma interface
#endif

#include "common/h/Vector.h"

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
