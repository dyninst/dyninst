#ifndef _QUEUE_H_
#define _QUEUE_H_

#include<pthread.h>
#include<iostream.h>


class dummy_sync {
public:
  void lock() {}
  void unlock() {}
  void register_cond(unsigned) {}
  void signal(unsigned) {}
  void wait(unsigned) {}
};


#define MAX_CONDS 512

class pthread_sync {
 private:
  pthread_mutex_t *mutex;
  pthread_cond_t *conds[MAX_CONDS];
  int registered_conds[MAX_CONDS];
  int num_registered_conds;
  
 public:
  pthread_sync() {
    num_registered_conds = 0;
    mutex = static_cast<pthread_mutex_t*>(calloc(1, sizeof(pthread_mutex_t)));
    pthread_mutex_init(mutex, NULL);
  }
  
  ~pthread_sync() {
    for (int i = 0; i < num_registered_conds ; i++) {
      pthread_cond_destroy(conds[registered_conds[i]]);
      free(conds[registered_conds[i]]);
    }
    pthread_mutex_destroy(mutex);
    free(mutex);
  }
  
  void lock() {
    pthread_mutex_lock(mutex);
  }
  
  void unlock() {
    pthread_mutex_unlock(mutex);
  }
  
  void register_cond(unsigned cond_num) {
    registered_conds[num_registered_conds++] = cond_num;
    
    conds[cond_num] = static_cast<pthread_cond_t*>(
					 calloc(1, sizeof(pthread_cond_t)));
    pthread_cond_init(conds[cond_num], NULL);
  }
  
  void signal(unsigned cond_num) {
    pthread_cond_signal(conds[cond_num]);
  }
  
  void wait(unsigned cond_num) {
    pthread_cond_wait(conds[cond_num], mutex);
  }
};


#define MAX_QUEUE_SIZE 32768
#define EMPTY 0
#define FULL 1

template <class obj, class sync=dummy_sync> 
class Queue {
 private:
  volatile int frontI;
  volatile int backI;
  volatile int _size;
  obj *items;
  sync *s;
  
 public:
  void enqueue(const obj &i);
  obj dequeue();
  obj front();

  void print(ostream &os) const;
  
  Queue() : frontI(0), backI(0), _size(0) {
    s = new sync();

    s->register_cond(EMPTY);
    s->register_cond(FULL);

    items = new obj[MAX_QUEUE_SIZE]; 
  }
  
  ~Queue() { delete [] items; }

  int size()  const {  return _size;  }
  int empty() const {  return (this->_size == 0);  }
  int full()  const {  return (this->_size == MAX_QUEUE_SIZE);  }
};


template <class obj, class sync>
void Queue<obj,sync>::print(ostream &os) const {
  int i;
  if(this->backI < this->frontI) {
    
    for(i = this->frontI; i < MAX_QUEUE_SIZE; i++) {
      os << items[i] << ", ";
    }
    
    for(i = 0; i < this->backI; i++) {
      os << items[i] << (i< (this->backI - 1) ? ", " : "");
    }
    
  } else {
    
    for(i = this->frontI; i < this->backI; i++) {
      os << items[i] << (i< (this->backI - 1) ? ", " : "");
    }
    
  }

  os << endl;
}


template <class obj, class sync>
inline void Queue<obj,sync>::enqueue(const obj &i) {
#if QUEUE_DEBUG
  cerr << "Queue<obj,sync>::enqueue() with arg of " << i << endl;
#endif
  s->lock();
  while (full()) s->wait(FULL);
#if QUEUE_DEBUG
    cerr << "about to insert  " << i << " into items[" << this->backI << "]" << endl;
#endif
    this->items[this->backI] = i;
    this->_size++;
    if (this->backI == MAX_QUEUE_SIZE - 1){
      this->backI = 0;
    } else{
      this->backI++;
    };

    s->unlock();
    s->signal(EMPTY);
}

template <class obj, class sync>
inline obj Queue<obj,sync>::dequeue() {
  s->lock();

  while (empty()) s->wait(EMPTY);

  obj i = this->items[this->frontI];   
  
  this->_size--;
  if (this->frontI == MAX_QUEUE_SIZE - 1){
    this->frontI = 0;
  } else{
    this->frontI++;
  }

  s->unlock();
  s->signal(FULL);

  return i;
}

template <class obj, class sync>
inline obj Queue<obj,sync>::front() {
  s->lock();

  while (empty()) s->wait(EMPTY);

  obj i = this->items[this->frontI];   
  
  s->unlock();
  s->signal(FULL);

  return i;
}


#endif

