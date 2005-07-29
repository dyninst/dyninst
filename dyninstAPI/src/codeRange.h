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
 * excluded
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

// $Id: codeRange.h,v 1.9 2005/07/29 19:18:24 bernat Exp $


#ifndef _codeRangeTree_h_
#define _codeRangeTree_h_

/*******************************************************/
/*		header files 			       */
/*******************************************************/

#include <assert.h>
#include <stdlib.h>
#include "common/h/Types.h"
#include "common/h/Vector.h"

/** template class for codeRangeTree. The implementation is based on red black
  * tree implementation for efficiency concerns and for getting sorted
  * elements easier.
  * There are two template types, K (key) and V (value).
  */

/* Note: this is a near copy of BPatch_Set. That class didn't do what I needed,
   so... -- bernat, 10OCT03 */

typedef enum { TREE_RED, TREE_BLACK } color_t;

class int_function;
class image;
class mapped_object;
class multiTramp;
class baseTrampInstance;
class miniTrampInstance;
class image_func;
class signal_handler_location;

class codeRange {
  public:
    codeRange() { }
    virtual ~codeRange() { }

    virtual Address get_address_cr() const = 0;
    virtual unsigned get_size_cr() const = 0;

    // returns NULL if not of type
    // so some people who don't like dynamic_cast don't have to be troubled
    // by it's use
    // Don't use this; baseTramps aren't top-level members in the
    //process codeRangeByAddr tree. Instead, use multiTramp and
    //getBaseTrampInstance.
    baseTrampInstance *is_basetramp_multi();
    miniTrampInstance *is_minitramp();
    int_function *is_function();
    image *is_image();
    mapped_object *is_mapped_object();
    multiTramp *is_multitramp();

    image_func *is_image_func();

    signal_handler_location *is_signal_handler_location();
};

class codeRangeTree {

    /** tree implementation structure. Used to implement the RB tree */
    typedef struct entry {
	Address key;
	codeRange *value;
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
	entry(Address key_, codeRange *value_, entry* e) 
	    : key(key_), value(value_), color(TREE_RED), left(e),
	    right(e), parent(NULL) {}

	/** constructor 
	 * @param e the entry structure that will be copied 
	 */
	entry(const entry& e) : key(e.key),value(e.value),color(e.color),
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
    void leftRotate(entry*);

    // method that implements right rotattion used by RB tree for balanced
    // tree construction and keeps the RBtree properties.
    void rightRotate(entry*);

    // method that modifies the tree structure after deletion for keeping
    // the RBtree properties.
    void deleteFixup(entry*);

    // insertion to a binary search tree. It returns the new element pointer
    // that is inserted. If element is already there it returns NULL
    entry* treeInsert(Address, codeRange *);

    // finds the elemnts in the tree that will be replaced with the element
    // being deleted in the  deletion. That is the element with the largest
    // smallest value than the element being deleted. 
    entry* treeSuccessor(entry* ) const;

    // method that returns the entry pointer for the element that is searched
    //for. If the entry is not found then it retuns NULL
    entry* find_internal(Address) const;

    // infix traverse of the RB tree. It traverses the tree in ascending order
    void traverse(codeRange **,entry*,int&) const;

    // Vector version of above
    // infix traverse of the RB tree. It traverses the tree in ascending order
    void traverse(pdvector<codeRange *> &all, entry*) const;

    // deletes the tree structure for deconstructor.
    void destroy(entry*);

    /** copy constructor */
    codeRangeTree(const codeRangeTree &y) {};

  public:

    /** constructor. The default comparison structure is used */
    codeRangeTree() : setSize(0) { 
	nil = new entry;
	setData = nil;
    }

    
    /** destructor which deletes all tree structure and allocated entries */
    ~codeRangeTree() {
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
    void insert(codeRange *);

    /** removes the element in the tree 
     * @param 1 element that will be removed  
     */
    void remove(Address);

    /** returns true if the argument is member of the codeRangeTree
     * @param e the element that will be searched for
     */
    bool find(Address, codeRange *&) const;

    /** Returns the largest value less than or equal to the
     * key given
     */
    bool precessor(Address, codeRange *&) const;

    /** Returns the smallest value greater than or equal to the
     * key given
     */
    bool successor(Address, codeRange *&) const;

    /** fill an buffer array with the sorted
     * elements of the codeRangeTree in ascending order according to comparison function
     * if the codeRangeTree is empty it retuns NULL, other wise it returns 
     * the input argument.
     */
    codeRange ** elements(codeRange **) const;

    // And vector-style
    bool elements(pdvector<codeRange *> &) const;

    // method that replicates the tree structure of this tree
    entry* replicateTree(entry*,entry*,entry*,entry*);

    // Remove all entries in the tree
    void clear();
};

#endif /* _codeRangeTree_h_ */

