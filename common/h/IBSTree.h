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

// $Id$

#ifndef _IBSTree_h_
#define _IBSTree_h_

/*******************************************************/
/*		header files 			       */
/*******************************************************/

#include <assert.h>
#include <stdlib.h>
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/std_namesp.h"
#include "stats.h"

#include <set>
#include <limits.h>

using namespace std;

/** Templape class for Interval Binary Search Tree. The implementation is
  * based on a red-black tree (derived from our codeRange implementation)
  * to control the tree height and thus insertion and search cost.
  *
  * Unlike our codeRangeTree, this data structure can represent overlapping
  * intervals. It is useful for executing stabbing queries and more
  * generally for finding invervals that overlap a particular interval.
  * A stabbing query requires O(log(N) + L) time in the worst case, where
  * L is the number of overlapping intervals, and insertion requires
  * O(log^2(N)) time (compare to O(log(N)) for a standard RB-tree).
  *
  * This class requires a worst case storage O(N log(N))
  *
  * For more information:
  *
  * @TECHREPORT{Hanson90theibs-tree:,
  *     author = {Eric N. Hanson and Moez Chaabouni},
  *     title = {The IBS-tree: A data structure for finding all intervals that overlap a point},
  *     institution = {},
  *     year = {1990}
  * }
  **/

/** EXTREMELY IMPORTANT XXX: Assuming that all intervals have lower bound
    predicate <= and upper bound predicate > ; that is, intervals are 
    (a,b] **/

typedef enum { TREE_RED, TREE_BLACK } color_t;

class interval {
  public:
    interval() { }
    virtual ~interval() { }

    virtual int low() const = 0;
    virtual int high() const = 0;
};

class SimpleInterval : interval {
  public:
    SimpleInterval( interval & i, void * id ) {
        low_ = i.low();
        high_ = i.high();
        id_ = id;
    }
    SimpleInterval(int low, int high, void * id) {
        low_ = low;
        high_ = high;
        id_ = id;
    }
        
  private:
    int low_;
    int high_;
    void * id_; // some arbitrary unique identifier
};

template<class ITYPE>
class IBSTree;

template<class ITYPE = interval>
class IBSNode {
    friend class IBSTree<ITYPE>;
  public:
    IBSNode() : 
        color(TREE_BLACK),
        left(NULL),
        right(NULL),
        parent(NULL) { }

    /** constructor for non-nil elements **/
    IBSNode(int value, IBSNode *n) :
        val_(value),
        left(n),
        right(n),
        parent(NULL) { }

    ~IBSNode() { }

    int value() const { return val_; };

  private: 
    /* The endpoint of an interval range */
    int val_;

    /* Intervals indexed by this node */
    set<ITYPE *> less;
    set<ITYPE *> greater;
    set<ITYPE *> equal;

    color_t color;

    IBSNode<ITYPE> *left;
    IBSNode<ITYPE> *right;
    IBSNode<ITYPE> *parent;
};

template<class ITYPE = interval>
class IBSTree {

    IBSNode<ITYPE> *nil;

    /** size of tree **/
    int treeSize;

    /** pointer to the tree root **/
    IBSNode<ITYPE> *root;

    /** RB-tree left rotation with modification to enforce IBS invariants **/
    void leftRotate(IBSNode<ITYPE> *);

    /** RB-tree right rotation with modification to enforce IBS invariants **/
    void rightRotate(IBSNode<ITYPE> *);

    /** Node deletion **/
    void deleteFixup(IBSNode<ITYPE> *);

    void removeInterval(IBSNode<ITYPE> *R, ITYPE *range);

    /** Insertion operations: insert the left or right endpoint of
        an interval into the tree (may or may not add a node **/
    IBSNode<ITYPE>* addLeft(ITYPE *I, IBSNode<ITYPE> *R, IBSNode<ITYPE> *parent);
    IBSNode<ITYPE>* addRight(ITYPE *I, IBSNode<ITYPE> *R, IBSNode<ITYPE> *parent);

    /** Find the lowest valued ancestor of node R that has R in its
        left subtree -- used in addLeft to determine whether all of
        the values in R's right subtree are covered by an interval **/
    int rightUp(IBSNode<ITYPE> *R);
    int rightUpAux(IBSNode<ITYPE> *parent, IBSNode<ITYPE> *child, int cur);

    /** Symmetric to rightUp **/
    int leftUp(IBSNode<ITYPE> *R);
    int leftUpAux(IBSNode<ITYPE> *parent, IBSNode<ITYPE> *child, int cur);

    /** Tree-balancing algorithm on insertion **/
    void insertFixup(IBSNode<ITYPE> *x);

    /** Finds the precessor of the node; this node will have its value
        copied to the target node of a deletion and will itself be deleted **/
    //IBSNode* treePredecessor(IBSNode *);

    /** Find a node with the provided value (interval endpoint) **/
    //IBSNode* findNode(int) const;

    /** Delete all nodes in the subtree rooted at the parameter **/
    void destroy(IBSNode<ITYPE> *);

    void findIntervals(int X, IBSNode<ITYPE> *R, set<ITYPE *> &S) const;
    void findIntervals(ITYPE *I, IBSNode<ITYPE> *R, set<ITYPE *> &S) const;

    void PrintPreorder(IBSNode<ITYPE> *n);

    int height(IBSNode<ITYPE> *n);
    int CountMarks(IBSNode<ITYPE> *R) const;

    const unsigned MemUse() const;

  public:

    /** public for debugging purposes **/
    StatContainer stats_;

    IBSTree() : treeSize(0) {
        nil = new IBSNode<ITYPE>;
        root = nil;
        stats_.add("insert",TimerStat);
        stats_.add("remove",TimerStat);
    }     

    ~IBSTree() {
        destroy(root);
        delete nil;
    }

    int size() const { return treeSize; }
    int CountMarks() const;
    
    bool empty() const { return (root == nil); }

    void insert(ITYPE *);

    void remove(ITYPE *);

    /** Find all intervals that overlap the provided point. Returns
        the number of intervals found **/  
    int find(int, set<ITYPE *> &) const;
    int find(ITYPE *I, set<ITYPE *> &) const;

    /** Delete all entries in the tree **/
    void clear();

    void PrintPreorder() { PrintPreorder(root); }
};

// XXX XXX HOW I HATE YOU GCC! XXX XXX

template<class ITYPE>
void IBSTree<ITYPE>::rightRotate(IBSNode<ITYPE> *pivot)
{
    if(!pivot || (pivot == nil))
        return;

    IBSNode<ITYPE> *y = pivot->left;
    if(y == nil)
        return;
    pivot->left = y->right;
    if(y->right != nil)
        y->right->parent = pivot;
    y->parent = pivot->parent;
    if(!pivot->parent) {
        root = y;
    }
    else if(pivot == pivot->parent->left)
        pivot->parent->left = y;
    else
        pivot->parent->right = y;
    y->right = pivot;
    pivot->parent = y;

    /* Maintain the IBS annotation invariants */
    
    // 1. Copy all marks from the < slot of the pivot (old subtree root)
    //    to the < and = slots of y (new subtree root). Maintains containment.
    y->less.insert(pivot->less.begin(), pivot->less.end());
    y->equal.insert(pivot->less.begin(), pivot->less.end());

    // 2. For each mark in y's > slot, if it was not in pivot's >,
    //    *move* it to pivot's <. Maintains containment and maximality,
    //    because it ensures that nodes under y->right covered by
    //    the mark before rotation are now (which are now under pivot->left)
    //    are still covered by the mark (if y > can't cover them still)
    
    // 3. Simultaneously, if the mark in in y's > slot AND pivot's > slot 
    //    (before rotation), remove the mark from pivot's > and = slots
    //    (preserving maximality).

    typename set< ITYPE * >::iterator it = y->greater.begin();
    while( it != y->greater.end() )
    {
        ITYPE *tmp = *it;
    
        typename set< ITYPE *>::iterator pit = pivot->greater.find( tmp );
        if(pit == pivot->greater.end()) {
            // Case 2
            pivot->less.insert( tmp );
            // remove from y's >. This invalidates the iterator, so
            // update first
            typename set< ITYPE * >::iterator del = it;
            ++it;
            y->greater.erase( del );
        } else {
            // Case 3
            // remove from pivot's >
            pivot->greater.erase( pit );
            pit = pivot->equal.find( tmp );
            if(pit != pivot->equal.end())
                pivot->equal.erase( pit );
    
            ++it;
        }
    }
}

template<class ITYPE>
void IBSTree<ITYPE>::leftRotate(IBSNode<ITYPE> *pivot)
{
    if(!pivot || (pivot == nil))
        return;

    IBSNode<ITYPE> *y = pivot->right;

    if(y == nil)
        return;

    pivot->right = y->left;
    if(y->left != nil)
        y->left->parent = pivot;
    y->parent = pivot->parent;
    if(!pivot->parent) {
        root = y;
    }
    else if(pivot == pivot->parent->left)
        pivot->parent->left = y;
    else
        pivot->parent->right = y;
    y->left = pivot;
    pivot->parent = y;

    /* Fix up the IBS annotations. These are exactly opposeite of the
       rules for right rotation */
    
    y->greater.insert(pivot->greater.begin(),pivot->greater.end());
    y->equal.insert(pivot->greater.begin(),pivot->greater.end());

    typename set< ITYPE * >::iterator it = y->less.begin();
    while( it != y->less.end() )
    {
        ITYPE *tmp = *it;
    
        typename set< ITYPE *>::iterator pit = pivot->less.find( tmp );
        if(pit == pivot->less.end()) {
            // Case 2
            pivot->greater.insert( tmp );
            // remove from y's <. This invalidates the iterator, so
            // update first
            typename set< ITYPE * >::iterator del = it;
            ++it;
            y->less.erase( del );
        } else {
            // Case 3
            // remove from pivot's < and =
            pivot->less.erase( pit );
            pit = pivot->equal.find( tmp );
            if(pit != pivot->equal.end())
                pivot->equal.erase( pit );
    
            ++it;
        }
    }
}

template<class ITYPE>
IBSNode<ITYPE>* 
IBSTree<ITYPE>::addLeft(ITYPE *I, IBSNode<ITYPE> *R, IBSNode<ITYPE> *parent)
{
    bool created = false;
    if(R == nil) {
        // create a new node
        IBSNode<ITYPE> *tmp = new IBSNode<ITYPE>( I->low(), nil );
        treeSize++;
        created = true;
       
        if(parent == NULL)  // must be the root
        {
            root = tmp;
        } 
        else {
            tmp->parent = parent;
            if(tmp->value() < parent->value()) {
                parent->left = tmp;
            } else if(tmp->value() > parent->value()) {
                parent->right = tmp;
            } else {
                assert(0); // can't get here
            }
        }

        R = tmp;
    }

    if(R->value() == I->low()) {
        if( rightUp(R) <= I->high() ) {
            R->greater.insert( I );
        }
        R->equal.insert( I );   // assumes closed lower bound

        if(created)
            return R;
        else
            return NULL;
    }
    else if(R->value() < I->low()) {
        return addLeft(I,R->right, R);
    }
    else if(R->value() > I->low()) {
        if(R->value() < I->high()) {
            R->equal.insert( I );
        }
        if( rightUp(R) <= I->high() ) {
            R->greater.insert( I );
        }
        return addLeft(I, R->left, R);
    } else {
        assert(0);  // can't get here, but gcc whines
        return NULL;
    }
}

template<class ITYPE>
IBSNode<ITYPE> * 
IBSTree<ITYPE>::addRight(ITYPE *I, IBSNode<ITYPE> *R, IBSNode<ITYPE> *parent)
{
    bool created = false;
    if(R == nil) {
        IBSNode<ITYPE> *tmp = new IBSNode<ITYPE>(I->high(), nil);
        treeSize++;
        created = true;
        if(parent == NULL)  // must be the root
        {
            root = tmp;
        } 
        else {
            tmp->parent = parent;
            if(tmp->value() < parent->value()) {
                parent->left = tmp;
            } else if(tmp->value() > parent->value()) {
                parent->right = tmp;
            } else {
                assert(0); // can't get here
            }
        }

        R = tmp;
    }

    if(R->value() == I->high()) {
        // if everything in R's left subtree will be
        // within the interval (that is, if the nearest ancestor
        // node that has R in its right subtree (that R is greater than)
        // is equal to or exceeds the low bound of I
        if( leftUp(R) >= I->low() ) {
            R->less.insert( I );
        }
        // the upper bound of the interval is open, so we don't add to r.equals
    
        if(created)
            return R;
        else
            return NULL;
    }
    else if(R->value() < I->high()) {
        if(R->value() > I->low()) {
            // R is in the interval
            R->equal.insert( I ); 
        }
        if( leftUp(R) >= I->low() ) {
            // Everything to the left of R is within the interval
            R->less.insert( I );
        }
        return addRight(I, R->right, R);
    }
    else if(R->value() > I->high()) {
        // R not in the interval
        return addRight(I, R->left, R);
    }
    else {
        assert(0);
        return NULL;
    }
}

/* Traverse upward in the tree, looking for the nearest ancestor
   that has R in its left subtree and return that value. Since this
   routine is used to compute an upper bound on an interval, failure
   to find a node should return +infinity */
template<class ITYPE>
int IBSTree<ITYPE>::rightUp(IBSNode<ITYPE> *R)
{
    return rightUpAux(R->parent, R, INT_MAX);
}
// FIXME don't need cur; maintain the defaults within the code

template<class ITYPE>
int IBSTree<ITYPE>::rightUpAux(IBSNode<ITYPE> *parent, IBSNode<ITYPE> *child, int cur)
{
    if(parent == NULL) {
        return cur;
    }

    // Return value only if our traverse is up the left
    // path of the parent's children
    if(parent->left == child)
        return parent->value();
    else
        return rightUpAux(parent->parent,parent,cur);
}

/* Same as rightUp, only looking for the nearest ancestor node that
   has R in its RIGHT subtree, returning NEGATIVE infinity upon failure */
template<class ITYPE>
int IBSTree<ITYPE>::leftUp(IBSNode<ITYPE> *R)
{
    return leftUpAux(R->parent,R,-INT_MAX - 1);    
}

template<class ITYPE>
int IBSTree<ITYPE>::leftUpAux(IBSNode<ITYPE> *parent, IBSNode<ITYPE> *child, int cur)
{
    if(parent == NULL)
        return cur;

    if(parent->right == child)
        return parent->value();
    else 
        return leftUpAux(parent->parent,parent,cur);
}

/* Restore RB-tree invariants after node insertion */
template<class ITYPE>
void IBSTree<ITYPE>::insertFixup(IBSNode<ITYPE> *x)
{
    x->color = TREE_RED;
    while((x != root) && (x->parent->color == TREE_RED)) {
        if(x->parent == x->parent->parent->left) {
            IBSNode<ITYPE>* y = x->parent->parent->right;
            if(y->color == TREE_RED) {
                x->parent->color = TREE_BLACK;
                y->color = TREE_BLACK;
                x->parent->parent->color = TREE_RED;
                x = x->parent->parent;
            }
            else {
                if(x == x->parent->right) {
                    x = x->parent;
                    leftRotate(x);
                }
                x->parent->color = TREE_BLACK;
                x->parent->parent->color = TREE_RED;
                rightRotate(x->parent->parent);
            }
        }
        else {
            IBSNode<ITYPE> *y = x->parent->parent->left;
            if(y->color == TREE_RED) {
                x->parent->color = TREE_BLACK;
                y->color = TREE_BLACK;
                x->parent->parent->color = TREE_RED;
                x = x->parent->parent;
            }
            else {
                if(x == x->parent->left) {
                    x = x->parent;
                    rightRotate(x);
                }
                x->parent->color = TREE_BLACK;
                x->parent->parent->color = TREE_RED;
                leftRotate(x->parent->parent);
            }
        }
    }
    root->color = TREE_BLACK;
}

template<class ITYPE>
void IBSTree<ITYPE>::destroy(IBSNode<ITYPE> *n)
{
    if(!n || (n == nil))
        return;
    if(n->left != nil)
        destroy(n->left);
    if(n->right != nil)
        destroy(n->right);
    delete n;
}


/* void deleteFixup(IBSNode<ITYPE> *)
{
    // XXX not implemented
    assert(0);
}*/

template<class ITYPE>
void IBSTree<ITYPE>::findIntervals(int X, IBSNode<ITYPE> *R, set<ITYPE *> &S) const
{
    if(R == nil) return;
    
    if(X == R->value()) {
        S.insert(R->equal.begin(),R->equal.end());
        return;
    }
    else if(X < R->value()) {
        S.insert(R->less.begin(),R->less.end());
        findIntervals(X,R->left,S);
    } else {
        S.insert(R->greater.begin(),R->greater.end());
        findIntervals(X,R->right,S);
    }
}

/* Find all intervals that intersect an interval:

   If low is < a node, take the < set (any interval in < contains low)
   If low or high are > a node, take the > set
   If low <= a node and high > a node, take the = set
*/
template<class ITYPE>
void IBSTree<ITYPE>::findIntervals(ITYPE * I, IBSNode<ITYPE> *R, set<ITYPE *> &S) const
{
    if(R == nil) return;

    int low = I->low();
    int high = I->high();    

    if(low < R->value()) {
        S.insert(R->less.begin(),R->less.end());
        findIntervals(I,R->left,S);
    }
    if(low > R->value() || high > R->value()) {
        S.insert(R->greater.begin(),R->greater.end());
        findIntervals(I,R->right,S);
    }
    if(low <= R->value() && high > R->value()) {
        S.insert(R->equal.begin(),R->equal.end());
    }
}

template<class ITYPE>
void IBSTree<ITYPE>::removeInterval(IBSNode<ITYPE> *R, ITYPE *range)
{
    if(R == nil) return;

    R->less.erase(range);
    R->greater.erase(range);
    R->equal.erase(range);

    removeInterval(R->left,range);
    removeInterval(R->right,range);
}

template<class ITYPE>
int IBSTree<ITYPE>::CountMarks(IBSNode<ITYPE> *R) const
{
    if(R == nil) return 0;

    return (R->less.size() + R->greater.size() + R->equal.size()) +
        CountMarks(R->left) + CountMarks(R->right);
}

/***************** Public methods *****************/

template<class ITYPE>
void IBSTree<ITYPE>::insert(ITYPE *range)
{
    stats_.startTimer("insert");

    // Insert the endpoints of the range, rebalancing if new
    // nodes were created
    IBSNode<ITYPE> *x = addLeft(range,root,NULL);
    if(x) {
        insertFixup(x);
    }
    x = addRight(range,root,NULL);
    if(x) {
        insertFixup(x);
    }

    stats_.stopTimer("insert");
}

template<class ITYPE>
void IBSTree<ITYPE>::remove(ITYPE * range)
{
    stats_.startTimer("remove");

    // 1. Remove all interval markers corresponding to range from the tree,
    //    using the reverse of the insertion procedure.

    // 2. If no other intervals use the endpoints of this interval
    //    (find this how?), remove their endpoints (complex) and fix
    //    up the tree.

    // XXX FIXME XXX
    // Currently being very lazy and inefficient: only removing interval
    // markers (not end nodes even if they end up unused), and also removing
    // elements from each of the <, >, and = sets of each node (following
    // the tests of the insertion procedures would avoid many of these
    // O(log n) lookups

    removeInterval(root,range);
   
    stats_.startTimer("remove"); 
}

template<class ITYPE>
int IBSTree<ITYPE>::find(int X, set<ITYPE *> &out) const
{
    set<ITYPE *> tmp; 
    findIntervals(X,root,tmp);

    out.insert(tmp.begin(),tmp.end());

    return tmp.size();
}

template<class ITYPE>
int IBSTree<ITYPE>::find(ITYPE * I, set<ITYPE *> &out) const
{
    set<ITYPE *> tmp;
    findIntervals(I,root,tmp);

    out.insert(tmp.begin(),tmp.end());
    
    return tmp.size();
}

template<class ITYPE>
void IBSTree<ITYPE>::clear() {
    if(root == nil) return;
    destroy(root);
    root = nil;
    treeSize = 0;
}

template<class ITYPE>
int IBSTree<ITYPE>::height(IBSNode<ITYPE> *n)
{
    if(!n)
        return 0;
    
    int leftHeight = 1 + height(n->left);
    int rightHeight = 1 + height(n->right);

    if(leftHeight > rightHeight)
        return leftHeight;
    else
        return rightHeight;
}

template<class ITYPE>
void IBSTree<ITYPE>::PrintPreorder(IBSNode<ITYPE> *n)
{
    if(n == nil) return;

    PrintPreorder(n->left);
    printf(" %d\n",n->value());
    PrintPreorder(n->right);

    if(n == root) {
        int h = height(root);
        printf(" tree height: %d\n", h);
    }
}

template<class ITYPE>
int IBSTree<ITYPE>::CountMarks() const
{
    return CountMarks(root);
}
#endif

