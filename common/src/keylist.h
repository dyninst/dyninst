/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _KEYLIST_H
#define _KEYLIST_H

/*
 * keylist.h - list ADT - with a destructor and copy constructor
 *             implements copy semantics
 *             uses void * key for comparison
 *             this class has been purified
 *           
 * See the tests subdirectory for examples of its use.
 *
 * To use this class: 
 *       1) put 'pragma implementation "keylist.h" ' in 
 *           a the file where you want the code generated
 *       2) put '#include "<prefix>/keylist.h" after line 1
 *           in the same file
 *       3) this class is instantiated with class Type
 *          if class Type is not a basic Type (int, char)
 *          then you need to implement the following for
 *          class Type if copy semantics are to be upheld
 *          if this is not done, purify will complain and
 *          the class behavior will be incorrect
 *            a) a copy constructor --> Type::Type(Type &c)
 *            b) the = operator --> Type & Type::operator =(Type&)
 *
 *
 * $Log: keylist.h,v $
 * Revision 1.1  1994/08/17 18:23:48  markc
 * Added new classes: Cstring KeyList, KList
 * Added new function: RPCgetArg
 * Changed typedefs in machineType.h to #defines
 *
 */

#define KYL_TRUE 1
#define KYL_FALSE 0
typedef int KYL_BOOL;

#ifdef KYL_PRINT
#include <iostream.h>
#endif

#pragma interface

#define ListHash(ptr, size) (((unsigned int)(intptr_t)(ptr) % (unsigned int)(size)))

template <class Type> class KeyList;

template <class Type>
class KeyListItem {
friend class KeyList<Type>;
public:
  KeyListItem(const Type &d, const void *k) {
    data =d;
    next = (KeyListItem<Type>*) 0;
    key = k;
  }
  KeyListItem(const KeyListItem<Type> &it) {
    data = it.data;
    key = it.key;
    next = (KeyListItem<Type>*) 0;
  }
  ~KeyListItem() {;}
  KeyListItem<Type> & operator = (const KeyListItem<Type> &it) {
    if (this == &it)
      return (*this);
    else {
      data = it.data;
      key = it.key;
      return (*this);
    }
  }
private:
  Type data;
  void *key;
  KeyListItem<Type> *next;
};

template <class Type>
class KeyList {
public:
  KeyList() { head = (KeyListItem<Type>*) 0; }
  KeyList(const KeyList<Type> &from);
  ~KeyList() { destroy(); }

  KYL_BOOL destroy();
  KYL_BOOL remove (void *key) {
    int f;
    find(key, f, 1);
    return f;
  }

  int empty() const { return (head == (KeyListItem<Type>*) 0);}
  int count() const;

  KYL_BOOL inList(const void *key);
  void reverse();

  KYL_BOOL append(const Type &data, const void *key);
  KYL_BOOL prepend(const Type &data, const void *key);
  KYL_BOOL appendUnique (const Type &i3, const void *key);
  KYL_BOOL prependUnique (const Type &i3, const void *key);

  KeyList<Type> & operator += (const KeyList<Type> &mergee);
  KeyList<Type> & operator = (const KeyList<Type> &from);

  void map (void (*map_function)(const Type &item)) const;

  Type car(KYL_BOOL &valid);
  KeyList<Type> cdr();

  // modifies the list elements in place
  void modify(Type (*mod_func)(Type &input));

  // removeMe == 0 --> the matched element is removed
  // found --> signifies results of match
  Type find(const void *key, KYL_BOOL &found, const int removeMe=0);

#ifdef KYL_PRINT
  friend ostream &operator << (ostream& os, const KeyList<Type>& S) {
    KeyListItem<Type> *temp;
    for (temp=S.head; temp; temp=temp->next)
      os << " : d=" << temp->data << "  k=" << temp->key << " : " << endl;
    return os;
  }
#endif

protected:
  KeyListItem<Type> *last() {
    KeyListItem<Type> *tmp, *lag;
    for (tmp=head, lag=0; tmp; lag=tmp, tmp=tmp->next)
      ;
    return lag;
  }
  KeyListItem<Type>	*head;
};


template <class Type>
KeyList<Type>::KeyList (const KeyList<Type> &from) {
  KeyListItem<Type> *temp;
  head = 0;
  for (temp=from.head; temp; temp = temp-> next) {
    prepend(temp->data, temp->key);
  }
  reverse();
}

template <class Type>
void KeyList<Type>::reverse() 
{
  KeyListItem<Type> *curr, *lag=0, *ahead;

  for (curr=head; curr; curr=ahead) {
    ahead = curr->next;
    curr->next = lag;
    lag = curr;
  }
  head = lag;
}

template <class Type>
int KeyList<Type>::count() const
{
  int c=0;
  KeyListItem<Type> *temp;
  for (temp=head; temp; temp = temp->next)
    c++;
  return c;
}

template <class Type>
KYL_BOOL KeyList<Type>::destroy()
{
  KeyListItem<Type> *temp, *next;
  
  for (temp=head; temp; temp = next) {
    next = temp->next;
    delete (temp);
  } 
  head = 0;
  return KYL_TRUE;
}

template <class Type>
KeyList<Type> &KeyList<Type>::operator = (const KeyList<Type> &from)
{
  KeyListItem<Type> *temp;

  if (this == &from)
    return (*this);

  destroy();
  
  for (temp=from.head; temp; temp = temp->next)
    prepend(temp->data, temp->key);
  reverse();
  return (*this);
}


template <class Type>
KeyList<Type> &KeyList<Type>::operator +=(const KeyList<Type> &mergee)
{
  KeyListItem<Type> *curr;

  if (this == &mergee)
    return (*this);

  for (curr=mergee.head; curr; curr=curr->next) {
    append(curr->data, curr->key);
  }
  return (*this);
}

template <class Type>
void KeyList<Type>::map (void (*map_function)(const Type &item)) const
{
  const KeyListItem<Type> *temp = 0;

  if (!map_function) return;
  for (temp = head; temp; temp = temp->next)
    map_function (temp->data);
}

template <class Type>
void KeyList<Type>::modify (Type (*mod_f)(Type &item))
{
  KeyListItem<Type> *temp = 0;

  if (!mod_f) return;
  for (temp = head; temp; temp = temp->next) 
    temp->data = mod_f(temp->data);
}

template <class Type>
KYL_BOOL KeyList<Type>::prependUnique (const Type &comp_me, const void *key)
{
  KeyListItem<Type> *temp = 0;

  for (temp = head; temp; temp = temp->next)
    if (temp->key == key) {
      temp->data = comp_me; 
      return KYL_TRUE;
    }
  // not found on list, so add it
  return (prepend(comp_me, key));
}

template <class Type>
KYL_BOOL KeyList<Type>::appendUnique (const Type &comp_me, const void *key)
{
  KeyListItem<Type> *temp = 0;

  for (temp = head; temp; temp = temp->next)
    if (temp->key == key) {
      temp->data = comp_me; 
      return KYL_TRUE;
    }
  // not found on list, so add it
  return (append(comp_me, key));
}

template <class Type>
KYL_BOOL KeyList<Type>::prepend (const Type &data, const void *key) 
{ 
  KeyListItem<Type> *ni;

  ni = new KeyListItem<Type>(data, key);
  if (!ni)
    return KYL_FALSE;

  ni->next = head;
  head = ni;
  return KYL_TRUE;
}


template <class Type>
KYL_BOOL KeyList<Type>::append(const Type &data, const void *key) 
{ 
  KeyListItem<Type> *ni;
  KeyListItem<Type> *tl;

  ni = new KeyListItem<Type>(data, key);
  if (!ni)
    return KYL_FALSE;

  tl = last();
  if (tl) {
    tl->next = ni;
  } else {
    ni->next = head;
    head = ni;
  }
  return KYL_TRUE;
}

template <class Type>
KYL_BOOL KeyList<Type>::inList (const void *key)
{
  KeyListItem<Type> *curr;
  for (curr=head; curr; curr=curr->next) {
    if (curr->key == key)
      return KYL_TRUE;
  }
  return KYL_FALSE;
}

template <class Type>
Type KeyList<Type>::find(const void *key, KYL_BOOL &found, const int removeMe)
{
  KeyListItem<Type> *lag, *curr;
  Type ret;

  for (curr=head, lag = (KeyListItem<Type>*) 0; curr; curr=curr->next) {
    if (curr->key == key)
      break;
    lag = curr;
  }

  if (curr) {
    found = KYL_TRUE;
    ret = curr->data;
    if (removeMe) {
      if (lag)
	lag->next = curr->next;
      else
	head = curr->next;
      delete(curr);
    }
  }
  // this may not be data from a found element
  return ret;
}

template <class Type>
Type KeyList<Type>::car(KYL_BOOL &valid)
{
  KeyListItem<Type> *nx;
  Type ret;

  if (head) {
    nx = head;
    ret = nx->data;
    head = head->next;
    delete nx;
    valid = KYL_TRUE;
    return ret;
  } else {
    valid = KYL_FALSE;
    return ret;
  }
}


template <class Type>
KeyList<Type> KeyList<Type>::cdr()
{
  KeyList<Type> ret;
  int val;

  ret = *this;
  ret.car(val);
  return ret;
}


// Hash table using KLists
template <class Type> class KHTable {
public:
  // placing function def here makes gcc happy 
#ifdef KYL_PRINT
friend ostream &operator<<(ostream &os, KHTable<Type> &data) {
  int i;
  for (i=0; i < data.tableSize; i++) {
    if (data.table[i]) {
      os << " list # " << i << endl;
      os << *data.table[i];
    }
  }
  return os;
}
#endif

  KHTable(const int size=97) {
    int s;
    tableSize = size;
    if (tableSize <= 0) tableSize = 97;
    table = new KeyList<Type>*[tableSize];
    for (s=0; s<size; ++s)
      table[s] = 0;
  }

  KHTable(const KHTable<Type> &data) {
    int j;
    tableSize = data.tableSize;
    table = new KeyList<Type>*[tableSize];
    for (j=0; j < tableSize; ++j)
      table[j] = data.table[j];
  }

  ~KHTable() {delete [] table;}
  KYL_BOOL destroy();

  Type find(const void *key, int &valid, const int removeMe=0);
  KYL_BOOL add(const Type &data, const void *key);
  KYL_BOOL addUnique(const Type &data, const void *key);

  KYL_BOOL remove(const void *key) {
    int val;
    find(key, val, 1);
    return(val);
  }

  KHTable<Type> & operator =(const KHTable<Type> & arg);
  KYL_BOOL operator == (const KHTable<Type> & arg);

  KYL_BOOL inTable(const void *key);
  int count() const;

private:
  KeyList<Type> **table;
  int tableSize;
};


template <class Type>
KYL_BOOL KHTable<Type>::destroy()
{
  int j;
  for (j=0; j<tableSize; ++j) 
    if (table[j]) {
      delete (table[j]);
      table[j] = 0;
    }
  return KYL_TRUE;
}

template <class Type>
Type KHTable<Type>::find(const void *key, int &valid, const int removeMe)
{
  int hid;
  Type ret;

  hid = ListHash(key, tableSize);
  if ((hid < 0) ||
      (hid > tableSize)) {
    valid = KYL_FALSE;
    return ret;
  } else if (!table[hid]) {
    valid = KYL_FALSE;
    return ret;
  } else {
    return(table[hid]->find(key, valid, removeMe));
  }
}

template <class Type>
KYL_BOOL KHTable<Type>::add(const Type &data, const void *key)
{
  int hid;

  hid = ListHash(key, tableSize);
  if (hid <0 || hid > tableSize) 
    return KYL_FALSE;

  if (!table[hid])
    table[hid] = new(KeyList<Type>);

  return (table[hid]->prepend(data, key));
}

template <class Type>
KHTable<Type> & KHTable<Type>::operator = (const KHTable<Type> &arg)
{
  int j;

  if (this == &arg)
    return (*this);

  destroy();
  tableSize = arg.tableSize;
  table = new KeyList<Type>*[tableSize];
  for (j=0; j < arg.tableSize; ++j)
    table[j] = arg.table[j];
  return(*this);
}

template <class Type>
KYL_BOOL KHTable<Type>::operator == (const KHTable<Type> &arg)
{
  int i;
  if (tableSize != arg.tableSize)
    return KYL_FALSE;
  else {
    for (i=0; i<tableSize; ++i) {
      if (!(table[i] == arg.table[i]))
	return KYL_FALSE;
    }
  }
  return KYL_TRUE;
}

template <class Type> 
KYL_BOOL KHTable<Type>::addUnique(const Type &data, const void *key)
{
  if (inTable(key))
    return KYL_FALSE;
  else
    return (add(data, key));
}

template <class Type>
int KHTable<Type>::count() const {
  int i, total=0;
  for (i=0; i < tableSize; i++) {
    if (table[i]) {
      total += table[i]->count();
    }
  }
  return(total);
}

template <class Type>
KYL_BOOL KHTable<Type>::inTable(const void *key)
{
  int hid;

  hid = ListHash(key, tableSize);
  if ((hid < 0) ||
      (hid > tableSize))
    return KYL_FALSE;
  else if (!table[hid])
    return KYL_FALSE;
  else
    return(table[hid]->inList(key));
}

#endif 
