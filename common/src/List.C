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

// $Id: list.C,v

#include <iostream.h>
#include "common/h/Vector.h"
#include "common/h/list.h"
#include "dyninstAPI/src/inst.h"

template <class Type> DO_INLINE_F typename List<Type>::node *List<Type>::getLastNode()
{
  node *lag = NULL;
  node *curNode = head;
  while(1) {
    if(curNode == NULL) break;
    lag = curNode;
    curNode = curNode->next;
  }
  return lag;
}

template <class Type> void List<Type>::push_front(const Type &data, void *key)
{
   List<Type>::node *ni = new typename List<Type>::node;
   ni->data = data;
   ni->key = key;
   ni->next = head;
   head = ni;
}

template <class Type> void List<Type>::push_front(const Type &data) 
{ 
   push_front(data, reinterpret_cast<void*>(const_cast<Type *>(&data))); 
}

template <class Type> void List<Type>::push_back(const Type &data, void *key)
{
   node *newNode = new node;
   newNode->data = data;
   newNode->key  = key;
   newNode->next = NULL;

   if(! isEmpty()) {
     node *lastNode = getLastNode();
     lastNode->next = newNode;
   } else {
     head = newNode;
   }
}

template <class Type> void List<Type>::push_back(const Type &data)
{ 
    push_back(data, reinterpret_cast<void*>(const_cast<Type *>(&data)));
}


template <class Type> bool List<Type>::addUnique(Type data) 
{ 
    return(addUnique(data, reinterpret_cast<void *>(&data))); 
}

template <class Type> DO_INLINE_F  void List<Type>::clear()
{
  node *curr, *nx;

  curr = head;
  while (curr) {
    nx = curr->next;
    delete (curr);
    curr = nx;
  }
  head = NULL;
}

template <class Type> DO_INLINE_F  bool List<Type>::remove(void *val)
{
    node *lag;
    node *curr;

    for (curr=head, lag = NULL; curr; curr=curr->next) {
	if (curr->data == val) {
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

template <class Type> DO_INLINE_F  bool List<Type>::remove_with_addr(void *key)
{
    node *lag;
    node *curr;

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

template <class Type> DO_INLINE_F bool List<Type>::find(void *data, 
							Type *saveVal)
{
   node *curr;

   for (curr=head; curr; curr=curr->next) {
      if (curr->key == data) {
	 (*saveVal) = curr->data;
	 return true;
      }
   }
   return false;
}

