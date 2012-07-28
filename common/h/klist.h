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

#ifndef _KLIST_H
#define _KLIST_H

/*
 * klist.h - list ADT - with a destructor and copy constructor
 *           implements copy semantics
 *           uses == for comparison
 *           this class has been purified
 *           
 * See the tests subdirectory for examples of its use.
 *
 * To use this class: 
 *       1) put 'pragma implementation "klist.h" ' in 
 *           a the file where you want the code generated
 *       2) put '#include "<prefix>/klist.h" after line 1
 *           in the same file
 *       3) this class is instantiated with class Type
 *          if class Type is not a basic Type (int, char)
 *          then you need to implement the following for
 *          class Type if copy semantics are to be upheld
 *          if this is not done, purify will complain and
 *          the class behavior will be incorrect
 *            a) a copy constructor --> Type::Type(Type &c)
 *            b) the = operator --> Type & Type::operator =(Type&)
 *            c) the == operator --> int Type::operator== (Type&)
 *
 *
 * $Log: klist.h,v $
 * Revision 1.1  1994/08/17 18:23:49  markc
 * Added new classes: Cstring KeyList, KList
 * Added new function: RPCgetArg
 * Changed typedefs in machineType.h to #defines
 *
 */

#define KL_TRUE 1
#define KL_FALSE 0
typedef int KL_BOOL;

#ifdef KL_PRINT
#include <iostream.h>
#endif

#pragma interface


template <class Type> class KList;

template <class Type>
class KListItem {
friend class KList<Type>;
public:
  KListItem(const Type &d) {
    data =d;
    next = (KListItem<Type>*) 0;
  }
  KListItem(const KListItem<Type> &it) {
    data = it.data;
    next = (KListItem<Type>*) 0;
  }
  ~KListItem() {;}
  KListItem<Type> & operator = (const KListItem<Type> &it) {
    if (this == &it)
      return (*this);
    else {
      data = it.data;
      return (*this);
    }
  }
private:
  Type data;
  KListItem<Type> *next;
};

template <class Type>
class KList {
public:
  KList() { head = (KListItem<Type>*) 0; }
  KList(const KList<Type> &from);
  ~KList() { destroy(); }

  KL_BOOL destroy();
  KL_BOOL remove(const Type &it) {
    int f;
    find(it, f, 1);
    return f;
  }

  int empty() const { return (head == (KListItem<Type>*) 0);}
  int count() const;

  void reverse();

  KL_BOOL append(const Type &data);
  KL_BOOL prepend(const Type &data);
  KL_BOOL appendUnique (const Type &i3);
  KL_BOOL prependUnique (const Type &i3);

  KList<Type> & operator += (const KList<Type> &mergee);
  KList<Type> & operator = (const KList<Type> &from);

  KList<Type> pure_map(Type (*map_f)(const Type &it)) const;
  void map (void (*map_function)(const Type &item)) const;

  // iterates until map_func(i1, i2) != 0
  void mapUntil (const Type &item, int (*map_func)(const Type &i1,
						   const Type &i2)) const;

  Type car(KL_BOOL &valid);
  KList<Type> cdr();

  // modifies the list elements in place
  void modify(Type (*mod_func)(Type &input));

  // gcc doesn't compile this when only the declaration is here
  // removeMe == 0 --> the matched element is removed
  // found --> signifies results of match
  // you provide the cmp function, or the class uses == 
  Type find(const Type &inE, KL_BOOL &found, int removeMe=0,
	    int (*cmp_f)(const Type &a, const Type &b)= 0)
    {
      KListItem<Type> *lag, *curr;
      Type ret;

      for (curr=head, lag = (KListItem<Type>*) 0; curr; curr=curr->next) {
	if (cmp_f) {
	  if (cmp_f(curr->data, inE))
	    break;
	} else if (curr->data == inE)
	  break;
	lag = curr;
      }

      if (curr) {
	found = KL_TRUE;
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


#ifdef KL_PRINT
  friend ostream &operator << (ostream& os, const KList<Type>& S) {
    KListItem<Type> *temp;
    for (temp=S.head; temp; temp=temp->next)
      os << " : " << temp->data << " : ";
    return os;
  }
#endif

protected:
  KListItem<Type> *last() {
    KListItem<Type> *tmp, *lag;
    for (tmp=head, lag=0; tmp; lag=tmp, tmp=tmp->next)
      ;
    return lag;
  }
  KListItem<Type>	*head;
};


template <class Type>
KList<Type>::KList (const KList<Type> &from) {
  KListItem<Type> *temp;
  head = 0;
  for (temp=from.head; temp; temp = temp-> next) {
    prepend(temp->data);
  }
  reverse();
}

template <class Type>
int KList<Type>::count() const
{
  int c=0;
  KListItem<Type> *temp;
  for (temp=head; temp; temp = temp->next)
    c++;
  return c;
}

template <class Type>
KL_BOOL KList<Type>::destroy()
{
  KListItem<Type> *temp, *next;
  
  for (temp=head; temp; temp = next) {
    next = temp->next;
    delete (temp);
  } 
  head = 0;
  return KL_TRUE; 
}

template <class Type>
KList<Type> &KList<Type>::operator = (const KList<Type> &from)
{
  KListItem<Type> *temp;

  if (this == &from)
    return (*this);

  destroy();
  
  for (temp=from.head; temp; temp = temp->next)
    prepend(temp->data);

  reverse();
  return (*this);
}


template <class Type>
KList<Type> &KList<Type>::operator +=(const KList<Type> &mergee)
{
  KListItem<Type> *curr;

  if (this == &mergee)
    return (*this);

  for (curr=mergee.head; curr; curr=curr->next) {
    append(curr->data);
  }
  return (*this);
}

template <class Type>
KList<Type> 
KList<Type>::pure_map(Type (*map_f)(const Type &it)) const
{
  KListItem<Type> *temp;
  KList<Type> Ret;
  for (temp=head; temp; temp=temp->next) {
    Ret.append(map_f(temp->data));
  }
  return Ret;
}

template <class Type>
void KList<Type>::map (void (*map_function)(const Type &item)) const
{
  const KListItem<Type> *temp = 0;

  if (!map_function) return;
  for (temp = head; temp; temp = temp->next)
    map_function (temp->data);
}

template <class Type>
void KList<Type>::modify (Type (*mod_f)(Type &item))
{
  KListItem<Type> *temp = 0;

  if (!mod_f) return;
  for (temp = head; temp; temp = temp->next) 
    temp->data = mod_f(temp->data);
}

template <class Type>
void KList<Type>::mapUntil (const Type &item,
			    int (*map_func)(const Type &i1,
					    const Type &i2)) const
{
  const KListItem<Type> *temp = 0;

  if (!map_func) return;
  for (temp = head; temp; temp = temp->next)
    if (map_func (temp->data, item))
      return;
}

template <class Type>
KL_BOOL KList<Type>::prependUnique (const Type &comp_me)
{
  KListItem<Type> *temp = 0;

  for (temp = head; temp; temp = temp->next)
    if (temp->data == comp_me) {
      temp->data = comp_me; 
      return KL_TRUE;
    }
  // not found on list, so add it
  return (prepend(comp_me));
}

template <class Type>
KL_BOOL KList<Type>::appendUnique (const Type &comp_me)
{
  KListItem<Type> *temp = 0;

  for (temp = head; temp; temp = temp->next)
    if (temp->data == comp_me) {
      temp->data = comp_me; 
      return KL_TRUE;
    }
  // not found on list, so add it
  return (append(comp_me));
}

template <class Type>
KL_BOOL KList<Type>::prepend (const Type &data) 
{ 
  KListItem<Type> *ni;

  ni = new KListItem<Type>(data);
  if (!ni)
    return KL_FALSE;

  ni->next = head;
  head = ni;
  return KL_TRUE;
}


template <class Type>
KL_BOOL KList<Type>::append(const Type &data) 
{ 
  KListItem<Type> *ni;
  KListItem<Type> *tl;

  ni = new KListItem<Type>(data);
  if (!ni)
    return KL_FALSE;

  tl = last();
  if (tl) {
    tl->next = ni;
  } else {
    ni->next = head;
    head = ni;
  }
  return KL_TRUE;
}

template <class Type>
Type KList<Type>::car(KL_BOOL &valid)
{
  KListItem<Type> *nx;
  Type ret;

  if (head) {
    nx = head;
    ret = nx->data;
    head = head->next;
    delete nx;
    valid = KL_TRUE;
    return ret;
  } else {
    valid = KL_FALSE;
    return ret;
  }
}


template <class Type>
KList<Type> KList<Type>::cdr()
{
  KList<Type> ret;
  int val;

  ret = *this;
  ret.car(val);
  return ret;
}

template <class Type>
void KList<Type>::reverse()
{
  KListItem<Type> *curr, *lag=0, *ahead;

  for (curr=head; curr; curr=ahead) {
    ahead = curr->next;
    curr->next = lag;
    lag = curr;
  }
  head = lag;
}

#endif 
