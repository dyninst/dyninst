#ifndef __libthread_syncqueue_C__
#define __libthread_syncqueue_C__

#include<pthread.h>
#include<iostream.h>

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
  
  void print(ostream &os) const;

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
void sync_queue<obj,sync>::print(ostream &os) const {
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
