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

// $Id: list.h,v

#ifndef LIST_H
#define LIST_H

#include <iostream.h>
#include "common/h/Vector.h"

#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif

#define ListHash(ptr, size) (((unsigned int)(ptr) % (unsigned int)(size)))

template <class Type> class List;
template <class Type> class StringList;
template <class Type> class HTable;


template <class Type> class _list_node {
 public: 
   friend class List<Type>;
   friend class StringList<Type>;
   void		*key;
   Type		data;
   _list_node<Type>   *next;

   _list_node() : key(NULL), next(NULL) { }
   explicit _list_node(const _list_node<Type> &from) : key(NULL), 
      data(from.data), next(NULL) {
      // key and next need to be set after constructor
   }
   _list_node<Type> *get_next_node() { return next; }
};


template <class Type> class _list_iterator {
  typedef _list_node<Type> node;
  mutable node *cur;

  void move_to_next() const {
    cur = cur->get_next_node();
  }

 public:
  _list_iterator(node *cur_) :
     cur(cur_) {    
  }
  node *getNode() { return cur; }
  // returns undefined result if iterator is the ending iterator
  Type &operator*() {
    return cur->data;
  }
  const Type &operator*() const {
    return cur->data;
  }

  _list_iterator operator++(int) const {  // postfix
    _list_iterator result = *this;
    move_to_next();
    return result;
  }

  _list_iterator operator++() const { // prefix
    move_to_next();
    return *this;
  }
  _list_iterator operator+(unsigned n) const {
     _list_iterator cur = *this;
     for(unsigned i=0; i<n; i++) {
	cur++;
     }
     return cur;
  }
  bool operator==(const _list_iterator &iter) const {
     return (cur == iter.cur);
  }
  bool operator!=(const _list_iterator &iter) const {
     return (cur != iter.cur);
  }
};


template <class Type> class List {
 public:
   typedef _list_iterator<Type> iterator;
   typedef const _list_iterator<Type> const_iterator;
   typedef _list_node<Type> node;

   List()  { head = NULL; }
   List(const List &fromList) {
      node *lastCopiedNode = NULL;
      node *headCopiedNode = NULL;
      for(const node *cur = fromList.head; cur; cur=cur->next) {
	 node *curCopiedNode = new node(*cur);  // copy constructor
	 curCopiedNode->key = curCopiedNode;
	 if(lastCopiedNode)
	    lastCopiedNode->next = curCopiedNode;
	 else
	    headCopiedNode = curCopiedNode;
      }
      head = headCopiedNode;
   }
   ~List() {
      clear();
   }
   friend ostream &operator<<(ostream &os, List<Type> &data) {
      List<Type>::iterator curr = data.begin();
      List<Type>::iterator endMarker = data.end();

      for(; curr != endMarker; ++curr) {
	 os << *curr << endl;
      }
      return os;
   }
   // returns the first element
   Type &front() { return *(begin()); }  
   const Type &front() const { return *(begin()); }
   
   // returns the last element
   Type &back() { return getLastNode()->data; }

   void push_front(const Type &data, void *key);
   void push_front(const Type &data);
   void push_back(const Type &data, void *key);
   void push_back(const Type &data);
   DO_INLINE_F bool addUnique(Type data);
   bool addUnique(Type data, void *key) {
      Type temp;
    
      bool foundIt = find(key, &temp);
      if (!foundIt) {
	 push_front(data, key);
	 return(true);
      } else {
	 return(false);
      }
   }
   DO_INLINE_F bool find(void *key, Type *saveVal);
   DO_INLINE_F bool remove_with_addr(void *key);
   DO_INLINE_F bool remove(void *val);
   DO_INLINE_F void clear();
   iterator begin() {
      return iterator(head);
   }
   const_iterator begin() const {
      return iterator(head);
   }

   iterator end() {
      return iterator(NULL);  // a hypothetical element after the last element
   }
   const_iterator end() const {
      return iterator(NULL);  // a hypothetical element after the last element
   }

   int count()	const {
      int c;
      node *curr;
      
      for (curr=head,c=0; curr; curr=curr->next) c++;
      return(c);
   }
   bool isEmpty() { return (head == NULL); }
   void operator +=(List<Type> mergee) {
      node *curr;
      
      for (curr=mergee.head; curr; curr=curr->next) {
	 addUnique(curr->data, curr->key);
      }
   }
   // inserts an item before position at pos
   void insert(iterator &, Type &) {
      // not implemented yet
   }
   void getItems(vector<Type> *buf) {
      for(node *curr=head; curr; curr=curr->next)
	 (*buf).push_back(curr->data);
   }
   void map (void (*map_function)(const Type item)) {
      const node *temp_ptr = 0;
      
      if (!map_function) return;
      for (temp_ptr = head; temp_ptr; temp_ptr = temp_ptr->next)
	 map_function (temp_ptr->data);
   }
   
 protected:
   node *getLastNode();

   node *head;
};


template <class Type> ostream &operator<<(ostream &os, HTable<Type> &data);

template <class Type> class HTable {
    public:
	// placing function def here makes gcc happy 
  // VG(06/15/02): that nonstandard hack doesn't work with gcc 3.1...
  // let's do this properly:
  // (1) the function needs to be already declared (above)
  // (2) add <> after name here, so only that instantiation is friended
  // (3) the function is defined after this class
  // Of course, old broken compilers don't like the standard, so we just
  // write something that compiles (as was the case before).
  // BTW, is this operator used anywhere?
#if (defined(i386_unknown_nt4_0) && _MSC_VER < 1300) || defined(mips_sgi_irix6_4)
  friend ostream& operator<< (ostream &os, HTable<Type> &data);
#else
  friend ostream& operator<< <> (ostream &os, HTable<Type> &data);
#endif

	HTable(Type data) { (void) HTable(); add(data, (void *) data); }
	DO_INLINE_F void add(Type data, void *key);
	DO_INLINE_F HTable(); 
	bool addUnique(Type data, void *key) {
	    Type temp;

	    bool foundIt = find(key, &temp);
	    if (foundIt) {
		return(false);
	    } else {
		add(data, key);
		return(true);
	    }
	}
	DO_INLINE_F Type find(void *key);
	DO_INLINE_F bool remove(void *key);
        DO_INLINE_F HTable<Type> operator =(HTable<Type> arg); 
        Type operator *() {
            return(*currList);
        }
	// postfix
	Type operator ++(int) {
	  Type curr;
	  
	  ++currList;
	  curr = *currList;
	  if (curr) return(curr);
	  for (currHid++; currHid < tableSize; currHid++) {
	    if (table[currHid]) {
	      currList = *table[currHid];
	      curr = *currList;
	      if (curr) return(curr);
	    }
	  }
	  return(NULL);
	}
	// prefix
	Type operator ++() {
	    Type curr;

	    ++currList;
            curr = *currList;
            if (curr) return(curr);
            for (currHid++; currHid < tableSize; currHid++) {
                if (table[currHid]) {
                    currList = *table[currHid];
                    curr = *currList;
                    if (curr) return(curr);
                }
	    }
	    return(NULL);
	}
	int count()	{
	    int i, total;

	    total = 0;
	    for (i=0; i < tableSize; i++) {
		if (table[i]) {
		    total += table[i]->count();
		}
	    }
	    return(total);
	}

    private:
	List<Type> **table;
	List<Type> currList;
	int currHid;
	int tableSize;
};


template <class Type> ostream &operator<<(ostream &os,
                                          HTable<Type> &data) {
  int i, total;
  
  total = 0;
  for (i=0; i < data.tableSize; i++) {
    if (data.table[i]) {
      os << *data.table[i];
    }
  }
  return os;
}


template <class Type> DO_INLINE_F HTable<Type> HTable<Type>::operator =(HTable<Type> arg) {
    table = arg.table;
    tableSize = arg.tableSize;

    // find the first item.
    currHid = -1;
    ++(*this);
    return(*this);
}

template <class Type> DO_INLINE_F  HTable<Type>::HTable()
{ 
    table = NULL;
    currHid = 0;
    tableSize = 0;
}

template <class Type> DO_INLINE_F  Type HTable<Type>::find(void *key)
{
    int hid;

    if (!tableSize) return(NULL);
    hid = ListHash(key, tableSize);
    if (hid <0 || hid > tableSize) abort();
    if (!table[hid]) {
	return(NULL);
    } else {
	return(table[hid]->find(key));
    }
}

template <class Type> DO_INLINE_F  void HTable<Type>::add(Type data, void *key)
{
    int hid;

    if (!tableSize) {
	tableSize = 97;
	table = (List<Type>**) calloc(tableSize, sizeof(List<Type>*));
    }
    hid = ListHash(key, tableSize);
    if (hid <0 || hid > tableSize) abort();
    if (!table[hid]) {
	table[hid] = new(List<Type>);
    }
    table[hid]->add(data, key);
}


template <class Type> DO_INLINE_F  bool HTable<Type>::remove(void *key)
{
    int hid;

    hid = ListHash(key, tableSize);
    if (hid <0 || hid > tableSize) abort();
    if (table[hid]) {
	return(table[hid]->remove(key));
    }
    return(false);
}

template <class Type> class StringList: public List<Type> {
    public:
	DO_INLINE_F Type find(void *key);
};

template <class Type> DO_INLINE_F Type StringList<Type>::find(void *data) 
{
    node *curr;

    for (curr=head; curr; curr=curr->next) {
	if (!strcmp((char *) curr->key, (char *) data)) {
	    return(curr->data);
	}
    }
    return((Type) 0);
}

#endif /* LIST_H */
