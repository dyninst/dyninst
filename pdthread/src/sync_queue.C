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

#ifndef __libthread_syncqueue_C__
#define __libthread_syncqueue_C__

#include <pthread.h>
#include "common/h/std_namesp.h"


#define MAX_QUEUE_SIZE 32768
#define EMPTY 0
#define FULL 1

template <class obj, class sync>
class sync_queue {

private:
  volatile int front;
  volatile int back;
  volatile int size;
  obj *items;
  sync *s;

public:
  void enqueue(obj &i);
  obj dequeue();
  
  void print(std::ostream &os) const;

  sync_queue() : front(0), back(0), size(0) {
    s = new sync();

    s->register_cond(EMPTY);
    s->register_cond(FULL);

    items = new obj[MAX_QUEUE_SIZE]; 
  }
  
  ~sync_queue() { delete [] items; }

  int empty() const;
  int full() const;

};

template <class obj, class sync>
void sync_queue<obj,sync>::print(std::ostream &os) const {
  int i;
  if(this->back < this->front) {
    
    for(i = this->front; i < MAX_QUEUE_SIZE; i++) {
      os << items[i] << ", ";
    }
    
    for(i = 0; i < this->back; i++) {
      os << items[i] << (i< (this->back - 1) ? ", " : "");
    }
    
  } else {
    
    for(i = this->front; i < this->back; i++) {
      os << items[i] << (i< (this->back - 1) ? ", " : "");
    }
    
  }

  os << endl;
}

template <class obj, class sync>
void sync_queue<obj,sync>::enqueue(obj &i) {
#if DEBUG
  cerr << "sync_queue<obj,sync>::enqueue() with arg of " << i << endl;
#endif
  s->lock();
  while (full()) s->wait(FULL);
#if DEBUG
    cerr << "about to insert  " << i << " into items[" << this->back << "]" << endl;
#endif
    this->items[this->back] = i;
    this->size++;
    if (this->back == MAX_QUEUE_SIZE - 1){
      this->back = 0;
    } else{
      this->back++;
    };

    s->unlock();
    s->signal(EMPTY);
}

template <class obj, class sync>
obj sync_queue<obj,sync>::dequeue() {
  s->lock();

  while (empty()) s->wait(EMPTY);

  obj i = this->items[this->front];   
  
  this->size--;
  if (this->front == MAX_QUEUE_SIZE - 1){
    this->front = 0;
  } else{
    this->front++;
  }

  s->unlock();
  s->signal(FULL);

  return i;
}

template <class obj, class sync>
int sync_queue<obj,sync>::empty() const {
  return this->size == 0;
}

template <class obj, class sync>
int sync_queue<obj,sync>::full() const {
  return this->size == MAX_QUEUE_SIZE;
}

#endif
