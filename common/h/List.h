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

/*
 * list.h - list ADT
 *
 * list.h,v
 * Revision 1.22  1995/02/16  09:27:07  markc
 * Modified code to remove compiler warnings.
 * Added #defines to simplify inlining.
 * Cleaned up Object file classes.
 *
 * Revision 1.21  1994/09/22  03:17:22  markc
 * added postfix ++ operator
 *
 * Revision 1.20  1994/08/17  18:22:54  markc
 * Moved the definitions of the << operator into the class declaration to
 * keep gcc happy.
 *
 * Revision 1.19  1994/07/26  20:07:42  hollings
 * added cast to ensure hash table pointers are positive.
 *
 * Revision 1.18  1994/07/11  23:00:57  jcargill
 * Fixed bug where added two lists with (+=) operator could result in
 * duplicate key entries
 *
 * Revision 1.17  1994/07/07  03:20:36  markc
 * Added removeAll function to list class.
 * Added machineType headers to specify pvm, cm5, ...
 *
 * Revision 1.16  1994/05/30  19:37:39  hollings
 * added pragma for external g++ functions.
 *
 */

#ifndef LIST_H
#define LIST_H

#include <iostream.h>

#if defined(external_templates)
#pragma interface
#endif

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

template <class Type> class ListItem {
    friend class List<Type>;
    friend class StringList<Type>;
    private:
	Type		data;
	void		*key;
	ListItem<Type>	*next;
};

template <class Type> class List {
    public:
        List() { head = NULL; }
	DO_INLINE_F int  empty();
	friend ostream &operator<<(ostream &os, List<Type> &data) {
	  List<Type> curr;
	  for (curr= data; *curr; ++curr) {
	    os << *curr << endl;
	  }
	  return os;
	}
	DO_INLINE_F void add(Type data, void *key);
	DO_INLINE_F void add(Type data);
	DO_INLINE_F bool addUnique(Type data);
	bool addUnique(Type data, void *key) {
	    Type temp;

	    temp = find(key);
	    if (!temp) {
		add(data, key);
		return(true);
	    } else {
		return(false);
	    }
	}
	DO_INLINE_F Type find(void *key);
	DO_INLINE_F bool remove(void *key);
	DO_INLINE_F void removeAll();
	int count()	{
	    int c;
	    ListItem<Type> *curr;

	    for (curr=head,c=0; curr; curr=curr->next) c++;
	    return(c);
	}
	Type operator *() { 
	    if (head) 
		return(head->data); 
	    else
		return((Type) NULL);
	}
	void operator +=(List<Type> mergee) {
	    ListItem<Type> *curr;

	    for (curr=mergee.head; curr; curr=curr->next) {
		addUnique(curr->data, curr->key);
	    }
	}
	// postfix - the beauty of c++ 
	Type operator ++(int) { 
	    Type ret = (Type) NULL;
	    if (head) {
		ret = head->data;
		head = head->next; 
	    }
	    return(ret); 
	}
	// prefix
	Type operator ++() { 
	    Type ret = (Type) NULL;
	    if (head) {
		ret = head->data;
		head = head->next; 
	    }
	    return(ret); 
	}
	void map (void (*map_function)(const Type item)) {
	  const ListItem<Type> *temp_ptr = 0;

	  if (!map_function) return;
	  for (temp_ptr = head; temp_ptr && temp_ptr->data; temp_ptr = temp_ptr->next)
	    map_function (temp_ptr->data);
	}
    protected:
	ListItem<Type>	*head;
};

template <class Type> DO_INLINE_F int List<Type>::empty()
{ 
    return (head == NULL);
}

template <class Type> DO_INLINE_F void List<Type>::add(Type data, void *key)
{
    ListItem<Type> *ni;

    ni = new(ListItem<Type>);
    ni->data = data;
    ni->key = key;

    ni->next = head;
    head = ni;
}

template <class Type> DO_INLINE_F  void List<Type>::add(Type data) 
{ 
    add(data, (void *) data); 
}

template <class Type> DO_INLINE_F  bool List<Type>::addUnique(Type data) 
{ 
    return(addUnique(data, (void *) data)); 
}

template <class Type> DO_INLINE_F  void List<Type>::removeAll()
{
  ListItem<Type> *curr, *nx;

  curr = head;
  while (curr) {
    nx = curr->next;
    delete (curr);
    curr = nx;
  }
  head = 0;
}

template <class Type> DO_INLINE_F  bool List<Type>::remove(void *key)
{
    ListItem<Type> *lag;
    ListItem<Type> *curr;

    for (curr=head, lag = NULL; curr; curr=curr->next) {
	if (curr->key == key) {
	    break;
	}
	lag = curr;
    }

    if (curr) {
	if (lag) {
	    lag->next = curr->next;
	} else {
	    head = curr->next;
	}
	delete(curr);
	return(true);
    } else {
	return(false);
    }
}

template <class Type> DO_INLINE_F  Type List<Type>::find(void *data)
{
    ListItem<Type> *curr;

    for (curr=head; curr; curr=curr->next) {
	if (curr->key == data) {
	    return(curr->data);
	}
    }
    return((Type) 0);
}

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

	    temp = find(key);
	    if (temp ) {
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
    ListItem<Type> *curr;

    for (curr=head; curr; curr=curr->next) {
	if (!strcmp((char *) curr->key, (char *) data)) {
	    return(curr->data);
	}
    }
    return((Type) 0);
}

#endif /* LIST_H */
