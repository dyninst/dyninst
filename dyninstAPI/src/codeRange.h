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

#include <assert.h>
#include <stdlib.h>
#include <vector>
#include "dyntypes.h"
#include "dyninstAPI/src/patch.h"

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
    //virtual Dyninst::Address get_address() const = 0;
    //virtual unsigned get_size() const = 0;

    virtual void *getPtrToInstruction(Dyninst::Address) const { assert(0); return NULL; }

    virtual void *get_local_ptr() const { 
        assert(0); return NULL; }


    func_instance *is_function();
    block_instance *is_basicBlock();
    block_instance *is_basicBlockInstance();

    image *is_image();
    mapped_object *is_mapped_object();
    parse_func *is_parse_func();
    parse_block *is_parse_block();
    signal_handler_location *is_signal_handler_location();
    inferiorRPCinProgress *is_inferior_rpc();

    void print_range(Dyninst::Address addr = 0);

    codeRange() = default;
    codeRange(const codeRange&) = default;
    virtual ~codeRange() = default;
};

class codeRangeTree {

   typedef enum {TREE_BLACK, TREE_RED} color_t;

    typedef struct entry {
	Dyninst::Address key;
	codeRange *value;
	color_t color;
	struct entry* left;
	struct entry* right;
	struct entry* parent;

	entry() 
	    : key(0), value(NULL), color(TREE_BLACK),left(NULL),right(NULL),parent(NULL) {}

	entry(entry* e)
	    : key(0), value(NULL), color(TREE_RED), left(e), right(e), parent(NULL) {}

	entry(Dyninst::Address key_, codeRange *value_, entry* e)
	    : key(key_), value(value_), color(TREE_RED), left(e),
	    right(e), parent(NULL) {}

	entry(const entry& e) : key(e.key),value(e.value),color(e.color),
	    left(NULL),right(NULL),parent(NULL) {}
    } entry;

    entry* nil;

    int setSize;

    entry* setData;

    void leftRotate(entry*);

    void rightRotate(entry*);

    void deleteFixup(entry*);

    entry* treeInsert(Dyninst::Address, codeRange *);

    entry* treeSuccessor(entry* ) const;

    entry* find_internal(Dyninst::Address) const;

    void traverse(codeRange **,entry*,int&) const;

    void traverse(std::vector<codeRange *> &all, entry*) const;

    void destroy(entry*);

    codeRangeTree(const codeRangeTree &/* y */) {}

  public:

    codeRangeTree() : setSize(0) { 
	nil = new entry;
	setData = nil;
    }

    
    ~codeRangeTree() {
	destroy(setData);
	delete nil;
    }

    int size() const { return setSize; }
    
    bool empty() const { return (setData == nil); }

    void insert(codeRange *);

    void remove(Dyninst::Address);

    bool find(Dyninst::Address, codeRange *&) const;

    bool precessor(Dyninst::Address, codeRange *&) const;

    bool successor(Dyninst::Address, codeRange *&) const;

    codeRange ** elements(codeRange **) const;

    bool elements(std::vector<codeRange *> &) const;

    entry* replicateTree(entry*,entry*,entry*,entry*);

    void clear();
};

#endif /* _codeRangeTree_h_ */

