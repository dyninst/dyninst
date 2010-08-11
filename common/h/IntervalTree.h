/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#if !defined(INTERVAL_TREE_H)
#define INTERVAL_TREE_H

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <map>

template <class K, class V>
class IntervalTree {
  typedef typename std::map<K, std::pair<K, V> > Tree;
  typedef typename Tree::const_iterator c_iter;

 public:
  typedef typename std::pair<K, K> Range;
  typedef typename std::pair<Range, V> Entry;

  IntervalTree() {};
  ~IntervalTree() {};
  int size() const { return tree_.size(); }
  bool empty() const { return tree_.empty(); }
  void insert(K lb, K ub, V v) {
    tree_[lb] = std::make_pair<K, V>(ub, v);
  }

  void remove(K lb) {
    tree_.erase(lb);
  }

  bool find(K key, V &value) const {
    K lb = 0;
    K ub = 0;
    V val;
    if (!precessor(key, lb, ub, val))
      return false;
    if (key < lb) return false;
    if (key >= ub) return false;
    value = val;
    return true;
  }

  bool find(K key, K &l, K &u, V &value) const {
    if (!precessor(key, l, u, value))
      return false;
    if (key < l) return false;
    if (key >= u) return false;
    return true;
  }

  bool precessor(K key, K &l, K &u, V &v) const {
    Entry e;
    if (!precessor(key, e)) {
      return false;
    }
    l = lb(e);
    u = ub(e);
    v = value(e);
    return true;
  }

  bool precessor(K key, Entry &e) const {
    if (tree_.empty()) return false;
    c_iter iter = tree_.lower_bound(key);
    if ((iter == tree_.end()) ||
	(iter->first != key)) {
      if (iter == tree_.begin()) {
	return false;
      }
      --iter;
    }
    if (iter->first > key) return false;

    lb(e) = iter->first;
    ub(e) = iter->second.first;
    value(e) = iter->second.second;

    assert(lb(e) <= key);
    if (ub(e) <= key) return false;
    return true;
  }
  void elements(std::vector<Entry> &buffer) const {
    buffer.clear();
    for (c_iter iter = tree_.begin();
	 iter != tree_.end(); ++iter) {
      Entry e;
      lb(e) = iter->first;
      ub(e) = iter->second.first;
      value(e) = iter->second.second;
      buffer.push_back(e);
    }
  }

  void clear() { tree_.clear(); }

 private:
  
  static V &value(Entry &e) { return e.second; }
  static K &lb(Entry &e) { return e.first.first; }
  static K &ub(Entry &e) { return e.first.second; }

  Tree tree_;
};
  
#if 0
 private:
    typedef enum { TREE_RED, TREE_BLACK } color_t;
    typedef std::pair<K, K> Range;

    /** tree implementation structure. Used to implement the RB tree */
    typedef struct entry {
        // Stores a range [lB...uB)
        K lB; // Lower bound
        K uB; // Upper bound (soft)
	V value;
	color_t color;	/* color of the node */
	struct entry* left; /* left child */
	struct entry* right; /* right child */
	struct entry* parent; /* parent of the node */

	/** constructor for structure */
	entry() 
	    : color(TREE_BLACK),left(NULL),right(NULL),parent(NULL) {}

	/** constructor used for non-nil elements 
	 * @param e nil entry
	 */	  
	entry(entry* e) //constructor with nil entry 
	    : color(TREE_RED), left(e), right(e), parent(NULL) {}

	/** constructor
	 * @param d data element
	 * @param e nill entry 
	 */
	entry(K lB_, K uB_, V value_, entry* e) 
	    : lB(lB_), uB(uB_), value(value_), color(TREE_RED), left(e),
	  right(e), parent(NULL) {}

	/** constructor 
	 * @param e the entry structure that will be copied 
	 */
	entry(const entry& e) : lB(e.lB), uB(e.uB),value(e.value),color(e.color),
	    left(NULL),right(NULL),parent(NULL) {}
    } entry;

    /** pointer to define the nil element of the tree NULL is not used
     * since some operations need sentinel nil which may have non-nil
     * parent.
     */
    entry* nil;

    /** size of the tree */
    int setSize;

    /** pointer to the tree structure */
    entry* setData;

    // method that implements left rotation used by RB tree for balanced
    // tree construction and keeps the RBtree properties.
    void leftRotate(entry* pivot) {
	if(!pivot || (pivot == nil)) return;
	entry* y = pivot->right;
	if(y == nil) return;
	pivot->right = y->left;
	if(y->left != nil) y->left->parent = pivot;
	y->parent = pivot->parent;
	if(!pivot->parent) setData = y;
	else if(pivot == pivot->parent->left)
            pivot->parent->left = y;
	else
            pivot->parent->right = y;
	y->left = pivot;
	pivot->parent = y;
    }

    // method that implements right rotattion used by RB tree for balanced
    // tree construction and keeps the RBtree properties.
    void rightRotate(entry* pivot) {
	if(!pivot || (pivot == nil)) return;
	entry* x = pivot->left;
	if(x == nil) return;
	pivot->left = x->right;
	if(x->right != nil) x->right->parent = pivot;
	x->parent = pivot->parent;
	if(!pivot->parent) setData = x;
	else if(pivot == pivot->parent->left)
            pivot->parent->left = x;
	else
            pivot->parent->right = x;
	x->right = pivot;
	pivot->parent = x;
    }

    // method that modifies the tree structure after deletion for keeping
    // the RBtree properties.
    void deleteFixup(entry* x) {
	while((x != setData) && 
	      (x->color == TREE_BLACK)) {            
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
    // insertion to a binary search tree. It returns the new element pointer
    // that is inserted. If element is already there it returns NULL
    entry* treeInsert(K lB, K uB, V value) {
	entry* y = NULL;
	entry* x = setData;
	while(x != nil){
            y = x;
            if (lB < x->lB) 
                x = x->left;
            else if(lB > x->lB)
                x = x->right;
            else 
                // Found it...
                return NULL;
	}	
	entry* z = new entry(lB, uB, value, nil);
	z->parent = y;
	if(!y) {
            setData = z;
        }
	else {
            if (lB < y->lB)
                y->left = z;
            else if (lB > y->lB)
                y->right = z;
	}
	setSize++;
	return z;
    }
    
    // finds the elemnts in the tree that will be replaced with the element
    // being deleted in the  deletion. That is the element with the largest
    // smallest value than the element being deleted. 
    entry* treeSuccessor(entry* x) const {
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

    // method that returns the entry pointer for the element that is searched
    //for. If the entry is not found then it retuns NULL
    entry* find_internal(K key) const {
	entry* x = setData;
	while(x != nil){
            if (key < x->lB) {
                x = x->left;
            }
            else if (key > x->lB) {
                x = x->right;
            }
            else
                return x;
	}	
	return NULL;
    }

    // Vector version of above
    // infix traverse of the RB tree. It traverses the tree in ascending order
    void traverse(std::vector<std::pair<Range, V> > &all, entry*node) const {
	if(node == nil)
            return;
	if(node->left != nil)
            traverse(all,node->left);
        all.push_back(std::make_pair(std::make_pair(node->lB, node->uB), node->value));
	if(node->right != nil)
            traverse(all,node->right);
    }

    // deletes the tree structure for deconstructor.
    void destroy(entry* node) {
	if(!node || (node == nil))
		return;
	if(node->left != nil)
		destroy(node->left);
	if(node->right != nil)
		destroy(node->right);
	delete node;
    }

    entry *precessor_int(K key) const {
        entry *x = setData;
        entry *last = nil;
        while (x != nil) {
            assert(x != NULL);
            if (x->lB == key) {
                return x;
            }
            else if (key < x->lB) {
                x = x->left;
            }
            else { // key > x->lB
                last = x;
                x = x->right;
            }
        }
        if (x == nil) {
            // Ran out of tree to search... get the parent
            assert(last != NULL);
            if (last != nil) {
                return last;
            }
            else return NULL;
        }
        // Should never hit here
        assert(0);
        return NULL;
    }


  public:

    /** constructor. The default comparison structure is used */
    IntervalTree() : setSize(0) { 
	nil = new entry;
	setData = nil;
    }

    
    /** destructor which deletes all tree structure and allocated entries */
    ~IntervalTree() {
	destroy(setData);
	delete nil;
    }

    /** returns the cardinality of the tree , number of elements */
    int size() const { return setSize; }
    
    /** returns true if tree is empty */
    bool empty() const { return (setData == nil); }

    /** inserts the element in the tree 
     * @param 1 element that will be inserted
     */
    void insert(K lB, K uB, V value) {
        // See if we can simply expand an existing range
        entry *q = precessor_int(lB);
        if (q &&
            (q->value == value) &&
            (q->uB >= lB) &&
            (q->uB <= uB)) {
            // Expand q. By definition (non-overlapping
            // intervals) this will not cause the
            // tree to unbalance.
            q->uB = uB;
            return;
        }

 	entry* x = treeInsert(lB, uB, value);
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
    
    /** removes the element in the tree 
     * @param 1 element that will be removed  
     */
    void remove(K lB) {
	entry* z = find_internal(lB);
	if(!z)
            return;
        if (z->lB != lB)
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
            z->lB = y->lB;
        }
	if(y->color == TREE_BLACK)
            deleteFixup(x);
	setSize--;
	delete y;
    }
    
    /** returns true if the argument is member of the IntervalTree
     * @param e the element that will be searched for
     */
    bool find(K key, V &value) const {
        V val;
        K lowerBound;
        K upperBound;
        if (!precessor(key, lowerBound, upperBound, val))
            return false;
        // Check to see if the range works
        if (key < lowerBound) {
            return false;
        }
        else if (key >= upperBound) {
            return false;
        }
        value = val;
        return true;
    }

    /** Like above, but returns the lb/ub range **/
    bool find(K key, K &lb, K &ub, V &value) const {
        V val;
        K lowerBound;
        K upperBound;
        if (!precessor(key, lowerBound, upperBound, val))
            return false;
        // Check to see if the range works
        if (key < lowerBound) {
            return false;
        }
        else if (key >= upperBound) {
            return false;
        }
        value = val;
	lb = lowerBound;
	ub = upperBound;
        return true;
    }

    /** Returns the largest value less than or equal to the
     * lB given
     */
    bool precessor(K key, K &lB, K &uB, V &value) const {   
        entry *x = precessor_int(key);
        if (x) {
            lB = x->lB;
            uB = x->uB;
            value = x->value;
            return true;
        }
        return false;
    }

    // And vector-style
    bool elements(std::vector<std::pair<Range, V> > &buffer) const {
	if(setData == nil) return false;
	traverse(buffer,setData);	
        return true;
    }

    // Remove all entries in the tree
    void clear() {
        if (setData == nil) return;
        destroy(setData);
        setData = nil;
        setSize = 0;
    }
};
#endif

#endif
