#ifndef __libthread_dllist_C__
#define __libthread_dllist_C__

#include <assert.h>
#include <stdio.h>
#include "predicate.h"
#include "xplat/h/Monitor.h"
#include "common/h/language.h"

namespace pdthr
{

#define DLLIST_EMPTY 0
#define DLLIST_FULL 1

#ifndef MAXIMUM_LIBTHREAD_DLLIST_SIZE
#define MAXIMUM_LIBTHREAD_DLLIST_SIZE 16384
#endif

#ifndef NODEPOOL_SIZE
#define NODEPOOL_SIZE 1024
#endif

class list_types {
  public:
    enum type { fifo, lifo };
};

template<class Element>
class dllist_visitor
{
public:
    dllist_visitor( void ) { }
    virtual ~dllist_visitor( void ) { }

    virtual bool visit( Element item ) = NULL;
};


template<class Element, class Sync=XPlat::Monitor>
class dllist
{
  private:
    class node {
      public:
        Element data;
        node* next;
        node* prev;
    };

    class pool_entry {
      public:
        node* nodes;
        pool_entry* next;
    };
    
    node* head;
    node* tail;

    node* free_list;
    pool_entry* free_pool;

    unsigned size;

    list_types::type behavior;

    XPlat::Monitor* s;

    node* find(Element e);

    node* find(predicate<Element>* pred);

    inline node* new_node();
    inline void free_node(node* n);
    inline void add_pool_entry();
    inline void remove(node* n);

  public:
    dllist(list_types::type type=list_types::fifo);
    ~dllist();

    bool empty();

    unsigned get_size() {
        return size;
    }

    /* Inserts element e into the list.  Where e is inserted depends
       on this list's type (lifo/fifo) 
       
       put will block if the list is full.
    */
    void put(Element e);

    /* Removes (and returns) either the last-in or first-in element from 
       the list, depending on this->type 
       
       take will block if the list is empty.
    */
    Element take();

    /* These four methods will "yank" out the first available element that
          a) is equal (i.e. operator==() ) to to_yank, or
          b) satisfies pred
       "first available" is defined as either firstmost-in or lastmost-in, 
       depending on whether this is a fifo or a lifo. 
       
       the "_nb" variants will not block if no suitable element is found; the
       others will `*/
    Element yank(Element to_yank);
    bool yank_nb(Element to_yank, Element* result);

    Element yank(predicate<Element>* pred);

    bool yank_nb(predicate<Element>* pred, Element* result);

    bool contains(Element e);

    bool contains(predicate<Element>* pred);

    void visit(dllist_visitor<Element>* visitor);

};

template<class Element, class Sync>
TYPENAME dllist<Element,Sync>::node*
dllist<Element,Sync>::find(Element e) {
    // NB!  you MUST lock s before entering this function
    node* result = NULL;
    node* current = head;

    while(current) {
        if(current->data == e) {
            result = current;
            goto done;
        }
        
        current = current->next;
    }
    
  done:
    return result;
}

template<class Element, class Sync>
TYPENAME dllist<Element,Sync>::node*
dllist<Element,Sync>::find(predicate<Element>* pred) {
    // NB!  you MUST lock s before entering this function
    node* result = NULL;
    node* current = head;

    while(current) {
        if(pred->satisfied_by(current->data)) {
            result = current;
            goto done;
        }
        
        current = current->next;
    }
    
  done:
    return result;
}

template<class Element, class Sync>
inline void dllist<Element,Sync>::remove(node* n) {
    // NB!  you MUST lock s before entering this function
    bool is_head = false, is_tail = false;
    node* next = NULL;
    node* prev = NULL;
    
    if (n == head) is_head = true;
    if (n == tail) is_tail = true;

    next = n->next;
    prev = n->prev;
    
    if(is_head) {
        head = next;
    } else {
        prev->next = next;
    }
    
    if(is_tail) {
        tail = prev;
    } else {
        next->prev = prev;
    }
    
    free_node(n);
}

template<class Element, class Sync>
inline void dllist<Element,Sync>::add_pool_entry() {
    pool_entry* new_entry = new pool_entry;
    new_entry->next = free_pool;
    new_entry->nodes = new node[NODEPOOL_SIZE];
    for(int i = 0; i < NODEPOOL_SIZE; i++)
        (new_entry->nodes[i]).next = &(new_entry->nodes[i+1]);
    (new_entry->nodes[NODEPOOL_SIZE - 1]).next = NULL;
    free_list = new_entry->nodes;
    free_pool = new_entry;
}

template<class Element, class Sync>
inline 
TYPENAME dllist<Element,Sync>::node*
dllist<Element,Sync>::new_node() {
  start:
    if(free_list) {
        node* n = free_list;
        free_list = free_list->next;
        n->next = NULL;
        return n;
    } else {
        add_pool_entry();
        goto start;
    }
    return NULL;
}

template<class Element, class Sync>
inline void dllist<Element,Sync>::free_node(dllist<Element,Sync>::node* n) {
    n->next = free_list;
    n->prev = NULL;
    free_list = n;
}

template<class Element, class Sync>
dllist<Element,Sync>::dllist(list_types::type type)
  : head( NULL ),
    tail( NULL ),
    free_list( NULL ),
    free_pool( NULL ),
    size( 0 ),
    behavior( type ),
    s( new Sync )
{
    s->RegisterCondition(DLLIST_EMPTY);
    s->RegisterCondition(DLLIST_FULL);    
}


template<class Element, class Sync>
dllist<Element,Sync>::~dllist() {
    s->Lock();
    pool_entry* p = free_pool;

    while(p) {
        pool_entry* temp = p->next;
        delete [] p->nodes;
        delete p;

        p = temp;
    }
    s->Unlock();
}

template<class Element, class Sync>
bool dllist<Element,Sync>::empty() {
   s->Lock();
    bool retval = size == 0;
   s->Unlock();
    return retval;
}   

template<class Element, class Sync>
void dllist<Element,Sync>::put(Element e) {
#if LIBTHREAD_DEBUG == 1
    fprintf(stderr, "acquiring monitor for dllist %p...\n", this);
#endif
   s->Lock();
#if LIBTHREAD_DEBUG == 1
    fprintf(stderr, "done acquiring monitor for dllist %p\n", this);
#endif

    while(size == MAXIMUM_LIBTHREAD_DLLIST_SIZE) {
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "put() blocking for dllist %p\n", this);
#endif
       s->WaitOnCondition(DLLIST_FULL);
    }
    
    node* n;
    switch(behavior) {
        case list_types::fifo:
            n = new_node();
            
            n->next = NULL;
            n->prev = tail;
            n->data = e;
            if(tail) {
                this->tail->next = n;
                this->tail = n;
            } else {
                this->tail = n;
                this->head = n;
            }
            
            break;
        case list_types::lifo:
            n = new_node();
            
            n->next = head;
            n->prev = NULL;
            n->data = e;
            if(head) {
                this->head->prev = n;
                this->head = n;
            } else {
                this->tail = n;
                this->head = n;
            }
            
            break;
    }
    size++;
    
#if LIBTHREAD_DEBUG == 1
    fprintf(stderr, "releasing monitor for dllist %p...\n", this);
#endif
    assert(head);
    assert(this->find(e) != NULL);

   s->Unlock();
#if LIBTHREAD_DEBUG == 1
    fprintf(stderr, "done releasing monitor for dllist %p\n", this);
#endif    
    
   s->SignalCondition(DLLIST_EMPTY);
}

template<class Element, class Sync>
Element dllist<Element,Sync>::take() {
    Element retval;
    node* tmp;
    
   s->Lock();

    // ensure that there is an item to take
    while (size == 0) {
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "take() blocking for dllist %p\n", this);
#endif        
       s->WaitOnCondition(DLLIST_EMPTY);
    }
    
    // at this point, we are guaranteed that head is non-null
    assert(head);
    
    if(tail == head) tail = NULL;
    
    retval = head->data;
    tmp = head->next;

    free_node(head);
    
    head = tmp;
    if(head)
        head->prev = NULL;

    size--;

   s->Unlock();
    
   s->SignalCondition(DLLIST_FULL);    

   return retval;
}

template<class Element, class Sync>
Element dllist<Element,Sync>::yank(Element e) {
    Element retval;
    node* to_yank;
    
   s->Lock();
    
    /* if there's nothing that we want to remove, sleep
       until a new element is placed in the list */
    while((to_yank = this->find(e)) == NULL) {
#if DLLIST_DEBUG
        fprintf(stderr, "blocking on yank()\n");
#endif
       s->WaitOnCondition(DLLIST_EMPTY);
    }
    
    retval = to_yank->data;
    
    this->remove(to_yank);

    size--;
    
   s->Unlock();

    return retval;
}

template<class Element, class Sync>
Element dllist<Element,Sync>::yank(predicate<Element>* pred) {
    Element retval;
    node* to_yank;
    
   s->Lock();
    
    /* if there's nothing that we want to remove, sleep
       until a new element is placed in the list */
    while((to_yank = this->find(pred)) == NULL) {
#if DLLIST_DEBUG
        fprintf(stderr, "blocking on yank()\n");
#endif
       s->WaitOnCondition(DLLIST_EMPTY);
    }
    
    retval = to_yank->data;
    
    this->remove(to_yank);

    size--;

   s->Unlock();
    
    return retval;
}

template<class Element, class Sync>
bool dllist<Element,Sync>::yank_nb(Element e, Element* result) {
    node* to_yank;
    
   s->Lock();
    
    if((to_yank = this->find(e)) == NULL)
        return false;

    if(result)
        *result = to_yank->data;
    
    this->remove(to_yank);

   s->Unlock();

    return true;
}

template<class Element, class Sync>
bool dllist<Element,Sync>::yank_nb(predicate<Element>* pred, Element* result) {
    node* to_yank;
    
   s->Lock();
    
    if((to_yank = this->find(pred)) == NULL)
        return false;

    if(result)
        *result = to_yank->data;
    
    this->remove(to_yank);

   s->Unlock();
    
    return true;
}

template<class Element, class Sync>
bool dllist<Element,Sync>::contains(predicate<Element>* pred) {
    bool retval;
   s->Lock();
    retval = (this->find(pred) != NULL);
   s->Unlock();
    return retval;
}

template<class Element, class Sync>
bool dllist<Element,Sync>::contains(Element e) {
    bool retval;
   s->Lock();
    retval = (this->find(e) != NULL);
   s->Unlock();
    return retval;
}

template<class Element, class Sync>
void dllist<Element,Sync>::visit(dllist_visitor<Element>* visitor) {
    s->Lock();
    
    node* result = NULL;
    node* current = head;

	bool continue_visiting = true;
    while(current && continue_visiting) {
        continue_visiting = visitor->visit(current->data);
        current = current->next;
    }

    s->Unlock();
}

} // namespace pdthr

#endif /* __libthread_dllist_C__ */

