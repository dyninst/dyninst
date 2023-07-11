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

// $Id$

#ifndef _IBSTree_h_
#define _IBSTree_h_

/*******************************************************/
/*		header files 			       */
/*******************************************************/

#include <assert.h>
#include "dyntypes.h"
#include "concurrent.h"

#include <stddef.h>
#include <vector>
#include <set>
#include <limits>
#include <iostream>

/** Template class for Interval Binary Search Tree. The implementation is
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
    [a,b) **/

// windows.h defines min(), max() macros that interfere with numeric_limits
#undef min
#undef max

namespace Dyninst {

namespace IBS {
typedef enum { TREE_RED, TREE_BLACK } color_t;
}


template <typename T = int, typename U = void*>
class SimpleInterval
{
  public:
    typedef T type;
    SimpleInterval(T low, T high, U id) {
        low_ = low;
        high_ = high;
        id_ = id;
    }
    SimpleInterval() {}
    virtual ~SimpleInterval() {}

    virtual T low() const { return low_; }
    virtual T high() const { return high_; }
    virtual U id() const { return id_; }
  protected:
    T low_{};
    T high_{};
    U id_{}; // some arbitrary unique identifier
};

template<class ITYPE>
class IBSTree;

template<class ITYPE = SimpleInterval<> >
class IBSNode {
    friend class IBSTree<ITYPE>;
    typedef typename ITYPE::type interval_type;
  public:
    IBSNode() : 
        val_(0),
        color(IBS::TREE_BLACK),
        left(NULL),
        right(NULL),
        parent(NULL) { }

    /** constructor for non-nil elements **/
    IBSNode(interval_type value, IBSNode *n) :
        val_(value),
        color(IBS::TREE_RED),
        left(n),
        right(n),
        parent(NULL) { }

    ~IBSNode() { }

    interval_type value() const { return val_; }
    interval_type operator*() const { return value; }
    friend std::ostream& operator<<(std::ostream& stream, const IBSNode<ITYPE>& node)
    {
        if(node.left) stream << *(node.left);
        stream << node.val_ << std::endl;
        if(node.right) stream << *(node.right);
        return stream;

    }


  private: 
    /* The endpoint of an interval range */
    interval_type val_;

    /* Intervals indexed by this node */
    std::set<ITYPE *> less;
    std::set<ITYPE *> greater;
    std::set<ITYPE *> equal;

    IBS::color_t color;

    IBSNode<ITYPE> *left;
    IBSNode<ITYPE> *right;
    IBSNode<ITYPE> *parent;
};

 
template<class ITYPE = SimpleInterval<> >
 std::ostream &operator<<(std::ostream &os, std::set<ITYPE *> &s) {
  for (auto i = s.begin(); i != s.end(); i++) {
    std::cerr << "[0x" << std::hex << (*i)->low() 
              << ", 0x" << (*i)->high() << std::dec << ")  ";
  }
  return os;
}

template<class ITYPE = SimpleInterval<> >
class IBSTree {
public:
    typedef typename ITYPE::type interval_type;
    typedef IBSNode<ITYPE>* iterator;
    typedef const IBSNode<ITYPE>* const_iterator;
    typedef ITYPE value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t difference_type;
    typedef size_t size_type;

    IBSNode<ITYPE> *nil;

private:
    /** size of tree **/
    boost::atomic<int> treeSize;

    /** pointer to the tree root **/
    IBSNode<ITYPE> *root;

    /** reader-writer lock to coordinate concurrent operations **/
    mutable dyn_rwlock rwlock;

    /** RB-tree left rotation with modification to enforce IBS invariants **/
    void leftRotate(IBSNode<ITYPE> *);

    /** RB-tree right rotation with modification to enforce IBS invariants **/
    void rightRotate(IBSNode<ITYPE> *);


    void removeInterval(IBSNode<ITYPE> *R, ITYPE *range);

    /** Insertion operations: insert the left or right endpoint of
        an interval into the tree (may or may not add a node **/
    IBSNode<ITYPE>* addLeft(ITYPE *I, IBSNode<ITYPE> *R);
    IBSNode<ITYPE>* addRight(ITYPE *I, IBSNode<ITYPE> *R);

    /** Find the lowest valued ancestor of node R that has R in its
        left subtree -- used in addLeft to determine whether all of
        the values in R's right subtree are covered by an interval **/
    interval_type rightUp(IBSNode<ITYPE> *R);

    /** Symmetric to rightUp **/
    interval_type leftUp(IBSNode<ITYPE> *R);

    /** Tree-balancing algorithm on insertion **/
    void insertFixup(IBSNode<ITYPE> *x);

    /** Finds the precessor of the node; this node will have its value
        copied to the target node of a deletion and will itself be deleted **/
    //IBSNode* treePredecessor(IBSNode *);

    /** Find a node with the provided value (interval endpoint) **/
    //IBSNode* findNode(int) const;

    /** Delete all nodes in the subtree rooted at the parameter **/
    void destroy(IBSNode<ITYPE> *);

    void findIntervals(interval_type X, IBSNode<ITYPE> *R, std::set<ITYPE *> &S) const;
    void findIntervals(ITYPE *I, IBSNode<ITYPE> *R, std::set<ITYPE *> &S) const;

    void PrintPreorder(IBSNode<ITYPE> *n, int indent);

    std::ostream& doIndent(int n)
    {
      std::cerr.width(n);
      std::cerr << "";
      return std::cerr;
    }

    int height(IBSNode<ITYPE> *n);
    int CountMarks(IBSNode<ITYPE> *R) const;

public:
    friend std::ostream& operator<<(std::ostream& stream, const IBSTree<ITYPE>& tree)
    {
        return stream << *(tree.root);
    }

  public:

    /** public for debugging purposes **/
    //StatContainer stats_;

    IBSTree() :
        nil(new IBSNode<ITYPE>),
        treeSize(0),
        root(nil)
    {
        //stats_.add("insert",TimerStat);
        //stats_.add("remove",TimerStat);
    }     

    ~IBSTree() {
        destroy(root);
        delete nil;
    }

    size_type size() const {
        return treeSize.load();
    }
    const_iterator begin() const {
        dyn_rwlock::shared_lock l(rwlock);
        iterator b = root;
        while(b->left) b = b->left;
        return b;
    }
    const_iterator end() const {
        dyn_rwlock::shared_lock l(rwlock);
        iterator e = root;
        while(e->right) e = e->right;
        return e;
    }
    int CountMarks() const;
    
    bool empty() const {
        return (root == nil);
    }

    void insert(ITYPE *);

    void remove(ITYPE *);

    /** Find all intervals that overlap the provided point. Returns
        the number of intervals found **/  
    int find(interval_type, std::set<ITYPE *> &) const;
    int find(ITYPE *I, std::set<ITYPE *> &) const;

    /** Finds the very next interval(s) with left endpoint
        = supremum(X) **/
    void successor(interval_type X, std::set<ITYPE *> &) const; 
    /** Use only when no two intervals share the same lower bound **/
    ITYPE * successor(interval_type X) const;

    /** Delete all entries in the tree **/
    void clear();

    void PrintPreorder() {
        dyn_rwlock::shared_lock l(rwlock);
        PrintPreorder(root, 0);
    }
};

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

    typename std::set< ITYPE * >::iterator it = y->greater.begin();
    while( it != y->greater.end() )
    {
        ITYPE *tmp = *it;
    
        typename std::set< ITYPE *>::iterator pit = pivot->greater.find( tmp );
        if(pit == pivot->greater.end()) {
            // Case 2
            pivot->less.insert( tmp );
            // remove from y's >. This invalidates the iterator, so
            // update first
            typename std::set< ITYPE * >::iterator del = it;
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

    typename std::set< ITYPE * >::iterator it = y->less.begin();
    while( it != y->less.end() )
    {
        ITYPE *tmp = *it;
    
        typename std::set< ITYPE *>::iterator pit = pivot->less.find( tmp );
        if(pit == pivot->less.end()) {
            // Case 2
            pivot->greater.insert( tmp );
            // remove from y's <. This invalidates the iterator, so
            // update first
            typename std::set< ITYPE * >::iterator del = it;
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
IBSTree<ITYPE>::addLeft(ITYPE *I, IBSNode<ITYPE> *R)
{
    IBSNode<ITYPE> *parent = NULL;

    // these calls can't be inlined as they're to virtuals
    interval_type ilow = I->low();
    interval_type ihigh = I->high();

    while(1) {
        bool created = false;
        if(R == nil) {
            // create a new node
            IBSNode<ITYPE> *tmp = new IBSNode<ITYPE>( ilow, nil );
            treeSize.fetch_add(1);
            created = true;
           
            if(parent == NULL)  // must be the root
                root = tmp;
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

        interval_type rval = R->value();
    
        if(rval == ilow) {
            if( rightUp(R) <= ihigh ) {
                R->greater.insert( I );
            }
            R->equal.insert( I );   // assumes closed lower bound
    
            if(created)
                return R;
            else
                return NULL;
        }
        else if(rval < ilow) {
            parent = R;
            R = R->right;
            continue;
        }
        else if(rval > ilow) {
            if(rval < ihigh) {
                R->equal.insert( I );
            }
            if( rightUp(R) <= ihigh ) {
                R->greater.insert( I );
            }
            parent = R;
            R = R->left;
            continue;
        } else {
            assert(0);  // can't get here, but gcc whines
            return NULL;
        }
    }

    assert(0);
    return NULL;    // make GCC happy
}

template<class ITYPE>
IBSNode<ITYPE> *
IBSTree<ITYPE>::addRight(ITYPE *I, IBSNode<ITYPE> *R)
{
    IBSNode<ITYPE> *parent = NULL;

    // these calls can't be inlined as they're to virtuals
    interval_type ilow = I->low();
    interval_type ihigh = I->high();

    while(1)
    {
        bool created = false;
        if(R == nil) {
            IBSNode<ITYPE> *tmp = new IBSNode<ITYPE>(ihigh,nil);
            treeSize.fetch_add(1);
            created = true;
            if(parent == NULL) // must be the root
                root = tmp;
            else {
                tmp->parent = parent;
                if(tmp->value() < parent->value())
                    parent->left = tmp;
                else if(tmp->value() > parent->value())
                    parent->right = tmp;
                else 
                    assert(0); // can't get here
            }
            R = tmp;
        }

        interval_type rval = R->value();

        if(rval == ihigh) {
            // Case 1. Everything in R's left subtree will be
            // within the interval (that is, the nearest ancestor
            // node that has R in its right subtree [leftUp(R)]
            // has a value equal to or exceeding the low bound of I
            //
            // NB the upper bound of the interval is open, so don't 
            // add to r.equals here. Compare addLeft
            if(leftUp(R) >= ilow)
                R->less.insert(I);

            if(created)
                return R; 
            else
                return NULL;
        }
        else if(rval < ihigh) {
            if(rval > ilow) {
                // R is in the interval
                R->equal.insert( I ); 
            }
            if( leftUp(R) >= ilow ) {
                // Everything to the left of R is within the interval
                R->less.insert( I );
            }

            parent = R;
            R = R->right;
            continue;
        }
        else if(rval > ihigh) {
            // R not in the interval
            parent = R;
            R = R->left;
            continue;
        }
        else {
            assert(0);
            return NULL;
        }
    }

    assert(0);
    return NULL; // make GCC happy
}

/* Traverse upward in the tree, looking for the nearest ancestor
   that has R in its left subtree and return that value. Since this
   routine is used to compute an upper bound on an interval, failure
   to find a node should return +infinity */
template<class ITYPE>
typename ITYPE::type
IBSTree<ITYPE>::rightUp(IBSNode<ITYPE> *R)
{
    while(NULL != R->parent) {
        if(R->parent->left == R)
            return R->parent->value();
        R = R->parent; 
    }
    return std::numeric_limits<interval_type>::max();
}

/* Same as rightUp, only looking for the nearest ancestor node that
   has R in its RIGHT subtree, returning NEGATIVE infinity upon failure */
template<class ITYPE>
typename ITYPE::type
IBSTree<ITYPE>::leftUp(IBSNode<ITYPE> *R)
{
    while(NULL != R->parent) {
        if(R->parent->right == R)
            return R->parent->value();
        R = R->parent;
    }
    // XXX is this right? for unsigned values, min() is a possible value
    return std::numeric_limits<interval_type>::min();
}

/* Restore RB-tree invariants after node insertion */
template<class ITYPE>
void IBSTree<ITYPE>::insertFixup(IBSNode<ITYPE> *x)
{
    x->color = IBS::TREE_RED;
    while((x != root) && (x->parent->color == IBS::TREE_RED)) {
        if(x->parent == x->parent->parent->left) {
            IBSNode<ITYPE>* y = x->parent->parent->right;
            if(y->color == IBS::TREE_RED) {
                x->parent->color = IBS::TREE_BLACK;
                y->color = IBS::TREE_BLACK;
                x->parent->parent->color = IBS::TREE_RED;
                x = x->parent->parent;
            }
            else {
                if(x == x->parent->right) {
                    x = x->parent;
                    leftRotate(x);
                }
                x->parent->color = IBS::TREE_BLACK;
                x->parent->parent->color = IBS::TREE_RED;
                rightRotate(x->parent->parent);
            }
        }
        else {
            IBSNode<ITYPE> *y = x->parent->parent->left;
            if(y->color == IBS::TREE_RED) {
                x->parent->color = IBS::TREE_BLACK;
                y->color = IBS::TREE_BLACK;
                x->parent->parent->color = IBS::TREE_RED;
                x = x->parent->parent;
            }
            else {
                if(x == x->parent->left) {
                    x = x->parent;
                    rightRotate(x);
                }
                x->parent->color = IBS::TREE_BLACK;
                x->parent->parent->color = IBS::TREE_RED;
                leftRotate(x->parent->parent);
            }
        }
    }
    root->color = IBS::TREE_BLACK;
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
void IBSTree<ITYPE>::findIntervals(interval_type X, IBSNode<ITYPE> *R, std::set<ITYPE *> &S) const
{
    while(R != nil) {
        if(X == R->value()) {
            S.insert(R->equal.begin(),R->equal.end());
            return;
        }
        else if(X < R->value()) {
            S.insert(R->less.begin(),R->less.end());
            R = R->left;
        } else {
            S.insert(R->greater.begin(),R->greater.end());
            R = R->right;
        }
    }
}

/* Find all intervals that intersect an interval:

   If low is < a node, take the < set (any interval in < contains low)
   If low or high are > a node, take the > set
   If low <= a node and high > a node, take the = set

   NB Because this traversal may go both directions in the tree,
      it remains a recursive operation and is less efficient
      than a pointwise stabbing query.
*/
template<class ITYPE>
void IBSTree<ITYPE>::findIntervals(ITYPE * I, IBSNode<ITYPE> *R, std::set<ITYPE *> &S) const
{
    if(R == nil) return;

    interval_type low = I->low();
    interval_type high = I->high();    

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
    else if(low == R->value() && high == R->value()) {
        // XXX explicitly allow zero-length intervals
        //     to `match' the starting value
        S.insert(R->equal.begin(),R->equal.end());
    }
}

template<class ITYPE>
void IBSTree<ITYPE>::removeInterval(IBSNode<ITYPE> *R, ITYPE *range)
{
    if(R == nil) return;

    interval_type low = range->low();
    interval_type high = range->high();    

    // FIXME this doesn't use maximality and containment to minimize the
    // number of set::erase calls
    if(low < R->value()) {
        R->less.erase(range);
        removeInterval(R->left,range);
    }
    if(low > R->value() || high > R->value()) {
        R->greater.erase(range);
        removeInterval(R->right,range);
    }
    if(low <= R->value() && high > R->value()) {
        R->equal.erase(range);
    }
    else if(low == R->value() && high == R->value()) {
        // XXX explicitly allow zero-length intervals
        //     to `match' the starting value
        R->equal.erase(range);
    }
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
    //stats_.startTimer("insert");

    dyn_rwlock::unique_lock l(rwlock);

    // Insert the endpoints of the range, rebalancing if new
    // nodes were created
    IBSNode<ITYPE> *x = addLeft(range,root);
    if(x) {
        insertFixup(x);
    }
    x = addRight(range,root);
    if(x) {
        insertFixup(x);
    }

    //stats_.stopTimer("insert");
}

template<class ITYPE>
void IBSTree<ITYPE>::remove(ITYPE * range)
{
    //stats_.startTimer("remove");

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

    dyn_rwlock::unique_lock l(rwlock);

    removeInterval(root,range);

    //stats_.stopTimer("remove");
}

template<class ITYPE>
int IBSTree<ITYPE>::find(interval_type X, std::set<ITYPE *> &out) const
{
    unsigned size = out.size();
    {
        dyn_rwlock::shared_lock l(rwlock);
        findIntervals(X,root,out);
    }
    return out.size() - size;
}

template<class ITYPE>
int IBSTree<ITYPE>::find(ITYPE * I, std::set<ITYPE *> &out) const
{
    unsigned size = out.size();
    {
        dyn_rwlock::shared_lock l(rwlock);
        findIntervals(I,root,out);
    }
    return out.size() - size;
}

template<class ITYPE>
void IBSTree<ITYPE>::successor(interval_type X, std::set<ITYPE *> &out) const
{
    IBSNode<ITYPE> *n = root;
    IBSNode<ITYPE> *last = nil;

    std::vector< IBSNode<ITYPE>* > stack;

    dyn_rwlock::shared_lock l(rwlock);

    /* last will hold the node immediately greater than X */
    while(1) {
        if(n == nil) {
            if(last != nil) {
                typename std::set<ITYPE *>::iterator sit = last->equal.begin();
                for( ; sit != last->equal.end(); ++sit) {
                   if((*sit)->low() == last->value()) out.insert(*sit);
                }
                if(!out.empty())
                    break;
                else {
                    // have missed out. pop back up to the last node where
                    // we went left and then advance down its right path
                    n = last->right;
                    if(!stack.empty()) {
                        last = stack.back();
                        stack.pop_back();
                    } else {
                        last = nil;
                    }
                    continue;
                } 
            } else 
                break;
        }

        if(X >= n->value()) {
            n = n->right;
        } else {
            if(last != nil)
                stack.push_back(last);
            last = n;
            n = n->left;
        }
    }
}

template<class ITYPE>
ITYPE * IBSTree<ITYPE>::successor(interval_type X) const
{
    std::set<ITYPE *> out;
    {
        dyn_rwlock::shared_lock l(rwlock);
        successor(X,out);
    }

    assert( out.size() <= 1 );
    if(!out.empty())
        return *out.begin();
    else
        return NULL;
}

template<class ITYPE>
void IBSTree<ITYPE>::clear() {
    if(root == nil) return;

    dyn_rwlock::unique_lock l(rwlock);

    destroy(root);
    root = nil;
    treeSize.store(0);
}

template<class ITYPE>
int IBSTree<ITYPE>::height(IBSNode<ITYPE> *n)
{
    if(!n)
        return 0;

    int leftHeight, rightHeight;
    {
        dyn_rwlock::shared_lock l(rwlock);
        leftHeight = 1 + height(n->left);
        rightHeight = 1 + height(n->right);
    }
    if(leftHeight > rightHeight)
        return leftHeight;
    else
        return rightHeight;
}

template<class ITYPE>
void IBSTree<ITYPE>::PrintPreorder(IBSNode<ITYPE> *n, int indent)
{
    if(n == nil) return;

    // print self
    doIndent(indent) << "node: 0x" << std::hex << n->value() << std::dec << " (" << n->value() << ")" << std::endl;
    if (!n->less.empty())
      doIndent(indent) << "  <: " << n->less << std::endl;
    if (!n->equal.empty())
      doIndent(indent) << "  =: " << n->equal << std::endl;
    if (!n->greater.empty())
      doIndent(indent) << "  >: " << n->greater << std::endl;

    // print children
    PrintPreorder(n->left, indent + 1);
    PrintPreorder(n->right, indent + 1);

    if(n == root) {
        std::cerr << "tree height: " << height(root) << std::endl;
    }
}

template<class ITYPE>
int IBSTree<ITYPE>::CountMarks() const
{
    dyn_rwlock::shared_lock l(rwlock);
    return CountMarks(root);
}
}/* Dyninst */


#endif

