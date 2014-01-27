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

#include <vector>
#include <assert.h>
#include "dyntypes.h"

#if !defined LRUCache_h_
#define LRUCache_h_

template<class K, class V>
class LRUCache {
 public:
   typedef int (*lru_hash_func)(K key);
 private:
   struct LRUCacheElement {
      int next;
      int prev;
      K key;
      V value;
   };
   //Using vectors for storage so that we can have tight
   // control over the memory used.  We don't want unnecessary
   // dynamic allocation, as this may be used under a signal handler.
   std::vector<LRUCacheElement> list_elems;
   std::vector<int> map_elems;
   int next_free;
   int max_size;
   int max_hash_size;
   int head;
   int tail;
   lru_hash_func hash_func;

   static const int lru_undefined = -1;
   static const int lru_tombstone = -2;

 private:
   void hash_reorg()
   {
      assert(next_free == max_size);
      //Clear out tombstone values for faster searches
      for (int i=0; i<max_hash_size; i++) {
         map_elems[i] = lru_undefined;
      }
      int cur = head;
      while (cur != lru_undefined) {
         hash_insert(list_elems[cur].key, cur);
         cur = list_elems[cur].next;
      }
   }

   int hash_find(K key)
   {
      int index = ((unsigned) hash_func(key)) % max_hash_size;
      int start = index;
      for (;;) {
         if (map_elems[index] == lru_undefined) {
            return lru_undefined;
         }
         if (map_elems[index] != lru_tombstone) {
            int elem = map_elems[index];
            if (list_elems[elem].key == key)
               return index;
         }
         if (++index == max_hash_size)
            index = 0;
         if (start == index) {
            hash_reorg();
         }
      }
   }

   void hash_insert(K key, int val) 
   {
      int index = ((unsigned) hash_func(key)) % max_hash_size;
      int start = index;
      for (;;) {
         if ((map_elems[index] == lru_undefined) || 
             (map_elems[index] == lru_tombstone))
         {
            map_elems[index] = val;
            return;
         }
         if (++index == max_hash_size)
            index = 0;
         assert(start != index);
	 if(start == index) return;
      }
   }

   void hash_remove(K key)
   {
      int index = hash_find(key);
      assert(index != lru_undefined);
      map_elems[index] = lru_tombstone;
   }
   
   void list_move_to_front(int index)
   {
      assert(head != lru_undefined);
      assert(tail != lru_undefined);
      assert(index < max_size);

      if (index == head)
         return;

      int prev_elem = list_elems[index].prev;
      int next_elem = list_elems[index].next;
      //Disconnect from current place
      if (prev_elem != lru_undefined)
         list_elems[prev_elem].next = next_elem;
      if (next_elem != lru_undefined)
         list_elems[next_elem].prev = prev_elem;
      
      //Move to front
      list_elems[index].prev = lru_undefined;
      list_elems[index].next = head;
      list_elems[head].prev = index;

      //Update head and tail
      head = index;
      if (tail == index && prev_elem != lru_undefined)
         tail = prev_elem;
   }

   int list_delete_last() {
      assert(head != lru_undefined);
      assert(tail != lru_undefined);
      assert(next_free == max_size);

      int elem_to_delete = tail;
      int prev_elem = list_elems[elem_to_delete].prev;
      if (prev_elem != lru_undefined)
         list_elems[prev_elem].next = lru_undefined;
      tail = prev_elem;

      return elem_to_delete;
   }

   void list_insert_new(int pos) {
      if (head == lru_undefined) {
         assert(tail == lru_undefined);
         head = pos;
         tail = pos;
         list_elems[pos].next = lru_undefined;
         list_elems[pos].prev = lru_undefined;
         return;
      }
      
      list_elems[pos].prev = lru_undefined;
      list_elems[pos].next = head;
      list_elems[head].prev = pos;
      head = pos;
   }
   
   void list_set_keyval(int pos, K key, V value) {
      list_elems[pos].key = key;
      list_elems[pos].value = value;
   }

   K get_key(int index) {
      return list_elems[index].key;
   }
   
   V get_value(int index) {
      return list_elems[index].value;
   }

 public:
   LRUCache(int initial_size, lru_hash_func f) :
      next_free(0),
      max_size(initial_size),
      head(lru_undefined),
      tail(lru_undefined),
      hash_func(f)
   {
      list_elems.reserve(max_size);
      //Leave some empty space for better hash performance
      max_hash_size = (int) (max_size * 1.5);
      map_elems.reserve(max_hash_size);
      map_elems.resize(max_hash_size);
      for (int i=0; i<max_hash_size; i++) {
         map_elems[i] = lru_undefined;
      }
   }

   void insert(K key, V value)
   {
      int result = hash_find(key);
      if (result != lru_undefined) {
         int list_elem = map_elems[result];
         list_set_keyval(list_elem, key, value);
         list_move_to_front(list_elem);
      }
      
      int elem_to_insert;
      if (next_free < max_size)
         elem_to_insert = next_free++;
      else {
         //We have to delete an old item.
         int lru_item = list_delete_last();
         assert(lru_item != lru_undefined);
         hash_remove(get_key(lru_item));
         elem_to_insert = lru_item;
      }
         
      list_insert_new(elem_to_insert);
      list_set_keyval(elem_to_insert, key, value);
      hash_insert(key, elem_to_insert);
   }

   bool lookup(K key, V &value)
   {
      int result = hash_find(key);
      if (result == lru_undefined) {
         return false;
      }
      int list_elem = map_elems[result];

      list_move_to_front(list_elem);
      value = get_value(list_elem);
      return true;
   }
};

#endif
