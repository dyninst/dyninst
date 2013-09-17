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

// $Id: list.C,v

#include <ostream>
#include "common/src/List.h"

using namespace std;

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
   typename ListBase<DataType, KeyType>::node *ni = new node(data, key, head);
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


