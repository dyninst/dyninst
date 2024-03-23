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

#ifndef _addrRange_h_
#define _addrRange_h_

#include <assert.h>
#include <stdlib.h>
#include <string>
#include <vector>

typedef enum { TREE_RED, TREE_BLACK } color_t;

class addrRange {
 public:
   virtual Dyninst::Address get_address() const = 0;
   virtual unsigned long get_size() const = 0;
   virtual std::string get_name() const {
      return std::string("UNNAMED");
   }
   virtual ~addrRange() {
   }
};

template <class T>
class addrRangeTree {

   typedef struct entry {
      Dyninst::Address key;
      T *value;
      color_t color;
      struct entry* left;
      struct entry* right;
      struct entry* parent;

      entry() 
         : key(0), value(NULL), color(TREE_BLACK),
           left(NULL), right(NULL), parent(NULL)
      {
      }

      entry(entry* e)
         : key(0), value(NULL), color(TREE_RED),
           left(e), right(e), parent(NULL)
      {
      }

      entry(Dyninst::Address key_, T *value_, entry* e)
         : key(key_), value(value_), color(TREE_RED), left(e),
           right(e), parent(NULL) 
      {
      }

      entry(const entry& e) : key(e.key),value(e.value),color(e.color),
                              left(NULL),right(NULL),parent(NULL) 
      {
      }
   } entry;

   entry* nil;

   int setSize;

   entry* setData;

   void leftRotate(entry* pivot)
   {
      if(!pivot || (pivot == nil))
         return;
      entry* y = pivot->right;
      if(y == nil)
         return;
      pivot->right = y->left;
      if(y->left != nil)
         y->left->parent = pivot;
      y->parent = pivot->parent;
      if(!pivot->parent) {
         setData = y;
      }
      else if(pivot == pivot->parent->left)
         pivot->parent->left = y;
      else
         pivot->parent->right = y;
      y->left = pivot;
      pivot->parent = y;
   }

   void rightRotate(entry *pivot)
   {
      if(!pivot || (pivot == nil))
         return;
      entry* x = pivot->left;
      if(x == nil)
         return;
      pivot->left = x->right;
      if(x->right != nil)
         x->right->parent = pivot;
      x->parent = pivot->parent;
      if(!pivot->parent) {
         setData = x;
      }
      else if(pivot == pivot->parent->left)
         pivot->parent->left = x;
      else
         pivot->parent->right = x;
      x->right = pivot;
      pivot->parent = x;
   }

   void deleteFixup(entry *x)
   {
      while((x != setData) && 
            (x->color == TREE_BLACK))
      {
         if(x == x->parent->left){
            entry* w = x->parent->right;
            if(w->color == TREE_RED){
               w->color = TREE_BLACK;
               x->parent->color = TREE_RED;
               leftRotate(x->parent);
               w = x->parent->right;
            }
            if((w->left->color == TREE_BLACK) &&
               (w->right->color == TREE_BLACK)){
               w->color = TREE_RED;
               x = x->parent;
            }
            else{
               if(w->right->color == TREE_BLACK){
                  w->left->color = TREE_BLACK;
                  w->color = TREE_RED;
                  rightRotate(w);
                  w = x->parent->right;
               }
               w->color = x->parent->color;
               x->parent->color = TREE_BLACK;
               w->right->color = TREE_BLACK;
               leftRotate(x->parent);
               x = setData;
            }
         }
         else{
            entry* w = x->parent->left;
            if(w->color == TREE_RED){
               w->color = TREE_BLACK;
               x->parent->color = TREE_RED;
               rightRotate(x->parent);
               w = x->parent->left;
            }
            if((w->right->color == TREE_BLACK) &&
               (w->left->color == TREE_BLACK)){
               w->color = TREE_RED;
               x = x->parent;
            }
            else{
               if(w->left->color == TREE_BLACK){
                  w->right->color = TREE_BLACK;
                  w->color = TREE_RED;
                  leftRotate(w);
                  w = x->parent->left;
               }
               w->color = x->parent->color;
               x->parent->color = TREE_BLACK;
               w->left->color = TREE_BLACK;
               rightRotate(x->parent);
               x = setData;
            }
         }
      }
      x->color = TREE_BLACK;
   }


   entry* treeInsert(Dyninst::Address key, T *value)
   {
      entry* y = NULL;
      entry* x = setData;
      while(x != nil){
         y = x;
         if (key < x->key) 
            x = x->left;
         else if(key > x->key)
            x = x->right;
         else
            return NULL;
      }	
      entry* z = new entry(key, value, nil);
      z->parent = y;
      if(!y) {
         setData = z;
      }
      else {
         if (key < y->key)
            y->left = z;
         else if (key > y->key)
            y->right = z;
      }
      setSize++;
      return z;
   }


   entry* treeSuccessor(entry *x) const
   {
      if(!x || (x == nil))
         return NULL;
      if(x->right != nil){
         entry* z = x->right;
         while(z->left != nil) z = z->left;
         return z;
      }
      entry* y = x->parent;
      while(y && (x == y->right)){
         x = y;
         y = y->parent;
      }
      return y;
   }

   entry* find_internal(Dyninst::Address element) const
   {
      entry* x = setData;
      while(x != nil){
         if (element < x->key) {
            x = x->left;
         }
         else if (element > x->key) {
            x = x->right;
         }
         else
            return x;
      }	
      return NULL;
   }


   void traverse(T **all, entry *node, int &n) const
   {
      if(node == nil)
         return;
      if(node->left != nil)
         traverse(all,node->left,n);
      if(all)
         all[n++] = node->value;
      if(node->right != nil)
         traverse(all,node->right,n);
   }


   void traverse(std::vector<T *> &all, entry*node) const
   {
      if(node == nil)
         return;
      if(node->left != nil)
         traverse(all,node->left);
      all.push_back(node->value);
      if(node->right != nil)
         traverse(all,node->right);
   }


   void destroy(entry *node)
   {
      if(!node || (node == nil))
         return;
      if(node->left != nil)
         destroy(node->left);
      if(node->right != nil)
         destroy(node->right);
      delete node;
   }

   addrRangeTree(const addrRangeTree &/* y */) 
   {
   }

   bool precessor_internal(Dyninst::Address key, entry * &value) const
   {
      entry *x = setData;
      entry *last = nil;
      while (x != nil) {
         assert(x != NULL);
         if (x->key == key) {
            value = x;
            return true;
         }
         else if (key < x->key) {
            x = x->left;
         }
         else { // key > x->key
            last = x;
            x = x->right;
         }
      }
      if (x == nil) {
         // Ran out of tree to search... get the parent
         assert(last != NULL);
         if (last != nil) {
            value = last;
            return true;
         }
         else return false;
      }
      // Should never hit here
      assert(0);
      return false;
   }


   bool successor_internal(Dyninst::Address key, entry * &value) const
   {
      entry *x = setData;
      entry *last = nil;
      while (x != nil) {
         if (x->key == key) {
            value = x;
            return true;
         }
         else if (key > x->key) {
            x = x->right;
         }
         else { // key < x->key
            last = x;
            x = x->left;
         }
      }
      if (x == nil) {
         // Ran out of tree to search... get the parent
         if (last != nil) {
            value = last;
            return true;
         }
         else return false;
      }
      // Should never reach this point
      assert(0);
      return false;
   }


 public:

   addrRangeTree() : 
      setSize(0) 
   { 
      nil = new entry;
      setData = nil;
   }
    
      virtual ~addrRangeTree()
      {
         destroy(setData);
         delete nil;
      }

      int size() const 
      { 
         return setSize; 
      }
    
      bool empty() const 
      { 
         return (setData == nil); 
      }

      void insert(T *value)
      {
         entry* x = treeInsert(value->get_address(), value);
         if(!x) {
            // We're done.
            return;
         }
         x->color = TREE_RED;
         while((x != setData) && (x->parent->color == TREE_RED)){
            if(x->parent == x->parent->parent->left){
               entry* y = x->parent->parent->right;
               if(y->color == TREE_RED){
                  x->parent->color = TREE_BLACK;
                  y->color = TREE_BLACK;
                  x->parent->parent->color = TREE_RED;
                  x = x->parent->parent;
               }
               else{
                  if(x == x->parent->right){
                     x = x->parent;
                     leftRotate(x);
                  }
                  x->parent->color = TREE_BLACK;
                  x->parent->parent->color = TREE_RED;
                  rightRotate(x->parent->parent);
               }
            }
            else{
               entry* y = x->parent->parent->left;
               if(y->color == TREE_RED){
                  x->parent->color = TREE_BLACK;
                  y->color = TREE_BLACK;
                  x->parent->parent->color = TREE_RED;
                  x = x->parent->parent;
               }
               else{
                  if(x == x->parent->left){
                     x = x->parent;
                     rightRotate(x);
                  }
                  x->parent->color = TREE_BLACK;
                  x->parent->parent->color = TREE_RED;
                  leftRotate(x->parent->parent);
               }
            }
         }
         setData->color = TREE_BLACK;
      }

      void remove(Dyninst::Address key)
      {
         entry* z = find_internal(key);
         if(!z)
            return;
         if (z->key != key)
            return;

         entry* y=((z->left == nil)||(z->right == nil)) ? z : treeSuccessor(z);
         entry* x=(y->left != nil) ? y->left : y->right;
         x->parent = y->parent;
         if(!y->parent) {
            setData = x;
         }
         else if(y == y->parent->left)
            y->parent->left = x;
         else
            y->parent->right = x;
         if(y != z) {
            z->value = y->value;
            z->key = y->key;
         }
         if(y->color == TREE_BLACK)
            deleteFixup(x);
         setSize--;
         delete y;
      }


      virtual bool find(Dyninst::Address key, T *& value) const
      {
         value = NULL;
         if (!precessor(key, value))
            return false;
         // Check to see if the range works
         if (!value->get_size()) {
            if(key > value->get_address())
               return false;
         }
         else if(key >= (value->get_address() + value->get_size())) {
            return false;
         }
         // We can also underflow
         if (key < value->get_address())
            return false;
         return true;
      }


      virtual bool find(Dyninst::Address start, Dyninst::Address end,
                        std::vector<T *> &ranges) const
      {
         entry *cur = nil;
         bool result = precessor_internal(start, cur);
         if (!result || cur == nil)
            result = successor_internal(start, cur);
         if (!result || cur == nil)
            return false;
         assert(cur);
   
         if (cur->key + cur->value->get_size() < start)
            cur = treeSuccessor(cur);
         while (cur != NULL && cur != nil && cur->key < end)
         {
            ranges.push_back(cur->value);
            cur = treeSuccessor(cur);
         }
   
         return (ranges.size() != 0);
      }

      virtual bool precessor(Dyninst::Address key, T *& value) const
      {
         entry *val;
         bool result = precessor_internal(key, val);
         if (!result)
            return false;
         value = val->value;
         return true;
      }


      virtual bool successor(Dyninst::Address key, T *& value) const
      {
         entry *val;
         bool result = successor_internal(key, val);
         if (!result)
            return false;
         value = val->value;
         return true;
      }

      T ** elements(T ** buffer) const
      {
         if(setData == nil) return NULL;
         if(!buffer) return NULL;
         int tmp = 0;
         traverse(buffer,setData,tmp);	
         return buffer;
      }


      bool elements(std::vector<T *> &buffer) const
      {
         if(setData == nil) return false;
         traverse(buffer,setData);	
         return true;
      }

      void clear()
      {
         if (setData == nil) return;
         destroy(setData);
         setData = nil;
         setSize = 0;
      }

};

#endif /* _addrRange_h_ */

