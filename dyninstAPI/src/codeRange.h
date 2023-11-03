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

// $Id: codeRange.h,v 1.18 2008/01/16 22:01:52 legendre Exp $


#ifndef _codeRangeTree_h_
#define _codeRangeTree_h_

/*******************************************************/
/*		header files 			       */
/*******************************************************/

#include <assert.h>
#include <stdlib.h>
#include <vector>
#include "dyntypes.h"
#include "dyninstAPI/src/patch.h"

/** template class for codeRangeTree. The implementation is based on red black
  * tree implementation for efficiency concerns and for getting sorted
  * elements easier.
  * There are two template types, K (key) and V (value).
  */


class func_instance;
class block_instance;
class block_instance;
class image;
class mapped_object;
class parse_func;
class signal_handler_location;
class functionReplacement;
class replacedFunctionCall;
class inferiorRPCinProgress;
class parse_block;

class codeRange : public patchTarget {
  public:
    //These are now inherited from relocTarget
    //virtual Dyninst::Address get_address() const = 0;
    //virtual unsigned get_size() const = 0;

    virtual void *getPtrToInstruction(Dyninst::Address) const { assert(0); return NULL; }

    // This returns a local pointer to the "beginning" of the
    // code range - as opposed to get_address, which returns
    // the "remote" address.
    virtual void *get_local_ptr() const { 
        assert(0); return NULL; }


    // returns NULL if not of type
    // so some people who don't like dynamic_cast don't have to be troubled
    // by it's use
    
    // This is actually a fake; we don't have func_instances as
    // code ranges. However, there are many times we want to know
    // if we're in a function, and this suffices. We actually do a
    // basic block lookup, then transform that into a function.
    func_instance *is_function();
    block_instance *is_basicBlock();
    block_instance *is_basicBlockInstance();

    image *is_image();
    mapped_object *is_mapped_object();
    parse_func *is_parse_func();
    parse_block *is_parse_block();
    signal_handler_location *is_signal_handler_location();
    inferiorRPCinProgress *is_inferior_rpc();

    //Prints codeRange info to stderr.  
    void print_range(Dyninst::Address addr = 0);

    codeRange() = default;
    codeRange(const codeRange&) = default;
    virtual ~codeRange() = default;
};

class codeRangeTree {

   typedef enum {TREE_BLACK, TREE_RED} color_t;

    /** tree implementation structure. Used to implement the RB tree */
    typedef struct entry {
	Dyninst::Address key;
	codeRange *value;
	color_t color;	/* color of the node */
	struct entry* left; /* left child */
	struct entry* right; /* right child */
	struct entry* parent; /* parent of the node */

	/** constructor for structure */
	entry() 
	    : key(0), value(NULL), color(TREE_BLACK),left(NULL),right(NULL),parent(NULL) {}

	/** constructor used for non-nil elements 
	 * @param e nil entry
	 */	  
	entry(entry* e) //constructor with nil entry 
	    : key(0), value(NULL), color(TREE_RED), left(e), right(e), parent(NULL) {}

	/** constructor
	 * @param d data element
	 * @param e nill entry 
	 */
	entry(Dyninst::Address key_, codeRange *value_, entry* e)
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
    entry* treeInsert(Dyninst::Address, codeRange *);

    // finds the elemnts in the tree that will be replaced with the element
    // being deleted in the  deletion. That is the element with the largest
    // smallest value than the element being deleted. 
    entry* treeSuccessor(entry* ) const;

    // method that returns the entry pointer for the element that is searched
    //for. If the entry is not found then it retuns NULL
    entry* find_internal(Dyninst::Address) const;

    // infix traverse of the RB tree. It traverses the tree in ascending order
    void traverse(codeRange **,entry*,int&) const;

    // Vector version of above
    // infix traverse of the RB tree. It traverses the tree in ascending order
    void traverse(std::vector<codeRange *> &all, entry*) const;

    // deletes the tree structure for deconstructor.
    void destroy(entry*);

    /** copy constructor */
    codeRangeTree(const codeRangeTree &/* y */) {}

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
    void remove(Dyninst::Address);

    /** returns true if the argument is member of the codeRangeTree
     * @param e the element that will be searched for
     */
    bool find(Dyninst::Address, codeRange *&) const;

    /** Returns the largest value less than or equal to the
     * key given
     */
    bool precessor(Dyninst::Address, codeRange *&) const;

    /** Returns the smallest value greater than or equal to the
     * key given
     */
    bool successor(Dyninst::Address, codeRange *&) const;

    /** fill an buffer array with the sorted
     * elements of the codeRangeTree in ascending order according to comparison function
     * if the codeRangeTree is empty it retuns NULL, other wise it returns 
     * the input argument.
     */
    codeRange ** elements(codeRange **) const;

    // And vector-style
    bool elements(std::vector<codeRange *> &) const;

    // method that replicates the tree structure of this tree
    entry* replicateTree(entry*,entry*,entry*,entry*);

    // Remove all entries in the tree
    void clear();
};

#endif /* _codeRangeTree_h_ */

