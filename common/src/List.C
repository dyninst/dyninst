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

// $Id: list.C,v

#include <iostream>
#include "common/h/List.h"


template <class DataType, class KeyType> DO_INLINE_F typename 
ListBase<DataType, KeyType>::node *ListBase<DataType, KeyType>::getLastNode()
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

template <class DataType, class KeyType> void 
ListBase<DataType, KeyType>::__push_front(DataType &data, 
					  const KeyType &key)
{
   ListBase<DataType, KeyType>::node *ni = new node(data, key, head);
   head = ni;
}


template <class DataType, class KeyType> 
void ListBase<DataType, KeyType>::__push_back(DataType &data, 
					      const KeyType &key)
{
   node *newNode = new node(data, key, NULL);

   if(! isEmpty()) {
     node *lastNode = getLastNode();
     lastNode->next = newNode;
   } else {
     head = newNode;
   }
}

template <class DataType, class KeyType>
void ListBase<DataType, KeyType>::clear()
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

template <class DataType, class KeyType>
bool ListBase<DataType, KeyType>::__remove_with_val(const DataType &dataVal)
{
    node *lag;
    node *curr;

    for (curr=head, lag = NULL; curr; curr=curr->next) {
	if (curr->data == dataVal) {
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

template <class DataType, class KeyType>
bool ListBase<DataType, KeyType>::__remove_with_key(const KeyType &key)
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

template <class DataType, class KeyType>
bool ListBase<DataType, KeyType>::__find_with_key(const KeyType &key, 
						  DataType *saveVal)
{
   node *curr;

   for (curr=head; curr; curr=curr->next) {
      if (curr->key == key) {
	 (*saveVal) = curr->data;
	 return true;
      }
   }
   return false;
}

template <class DataType, class KeyType>
bool ListBase<DataType, KeyType>::__find_with_val(const DataType &dataVal)
  const {
   node *curr;

   for (curr=head; curr; curr=curr->next) {
      if (curr->data == dataVal) {
	 return true;
      }
   }
   return false;
}


