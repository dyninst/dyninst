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

// $Id: codeRange.C,v 1.21 2008/01/16 22:01:51 legendre Exp $

#include <stdio.h>
#include "codeRange.h"

#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/dynProcess.h"

inferiorRPCinProgress * codeRange::is_inferior_rpc() {
	return dynamic_cast< inferiorRPCinProgress * >( this );
	}

// This is a special case... the multitramp is the thing in the
// codeRange tree, but people think of baseTramps.
// So this is dangerous to use, actually.
block_instance *codeRange::is_basicBlockInstance() {
    return dynamic_cast<block_instance *>(this);
}

func_instance *codeRange::is_function() {
   return NULL;
}

parse_func *codeRange::is_parse_func() {
   return dynamic_cast<parse_func *>(this);
}

parse_block *codeRange::is_parse_block() {
    return dynamic_cast<parse_block *>(this);
}

image *codeRange::is_image() {
   return dynamic_cast<image *>(this);
}

mapped_object *codeRange::is_mapped_object() {
   return dynamic_cast<mapped_object *>(this);
}

void codeRangeTree::leftRotate(entry* pivot){
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


void codeRangeTree::rightRotate(entry* pivot){
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


void codeRangeTree::deleteFixup(entry* x){
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


// fails if the key value is already in the tree (happens for shared code)
codeRangeTree::entry *codeRangeTree::treeInsert(Dyninst::Address key, codeRange *value)
{
	entry* y = NULL;
	entry* x = setData;
	while(x != nil){
		y = x;
                if (key < x->key) 
                    x = x->left;
                else if(key > x->key)
                    x = x->right;
                else {
                    x->value = value;
                    return NULL; // node is already in tree
                }
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

/** finds the minimum value node when x is being deleted */

codeRangeTree::entry *codeRangeTree::treeSuccessor(entry* x) const{
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


codeRangeTree::entry *codeRangeTree::find_internal(Dyninst::Address element) const{
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


void codeRangeTree::traverse(codeRange ** all, entry* node, int& n) const{
	if(node == nil)
		return;
	if(node->left != nil)
		traverse(all,node->left,n);
	if(all)
		all[n++] = node->value;
	if(node->right != nil)
		traverse(all,node->right,n);
}


void codeRangeTree::traverse(std::vector<codeRange *> &all, entry* node) const{
	if(node == nil)
		return;
	if(node->left != nil)
		traverse(all,node->left);
        all.push_back(node->value);
	if(node->right != nil)
		traverse(all,node->right);
}

//////////////////////////// PUBLIC FUNCTIONS ////////////////////////////////

void codeRangeTree::insert(codeRange *value) {
    //assert(value->get_size());
 	entry* x = treeInsert(value->get_address(), value);
	if(!x) {
         x = find_internal(value->get_address());
         assert(value->get_size() == x->value->get_size());
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

 void codeRangeTree::remove(Dyninst::Address key){
	entry* z = find_internal(key);
    if(!z) { return; }
    if(z->key != key) { return; }

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

    if(y->color == TREE_BLACK) {
		deleteFixup(x);
    }
	setSize--;
	delete y;
}




void codeRangeTree::destroy(entry* node){
	if(!node || (node == nil))
		return;
	if(node->left != nil)
		destroy(node->left);
	if(node->right != nil)
		destroy(node->right);
	delete node;
}

bool codeRangeTree::find(Dyninst::Address key, codeRange *& value) const{
    value = NULL;
    if (!precessor(key, value))
        return false;
    // Check to see if the range works
    if (!value->get_size()) {
        // XXX do we really need this warning?
        //fprintf(stderr, "%s[%d]:  Warning:  size was 0...\n", FILE__, __LINE__);
        if(key > value->get_address())
            return false;
    }
    else if(key >= (value->get_address() + value->get_size())) {
        return false;
    }
    // We can also underflow
    if (key < value->get_address()) {
      return false;
    }
    
    return true;
#if 0
    fprintf(stderr, "codeRangeTree::find for 0x%x\n", key);
    entry* x = find_internal(key);
    fprintf(stderr, "find_internal returned %p\n", x);
    if (!x) return false;
    value = x->value;
    assert(value->get_address() <= key); // Otherwise it wouldn't have been returned.

    if (key >= (value->get_address() + value->get_size())) {
        fprintf(stderr, "... ret false\n");
        return false;
    }
    fprintf(stderr, "... ret true\n");
    return true;
#endif
}

bool codeRangeTree::precessor(Dyninst::Address key, codeRange * &value) const{
    entry *x = setData;
    entry *last = nil;
    while (x != nil) {
        assert(x != NULL);
        if (x->key == key) {
            value = x->value;
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
            value = last->value;
            return true;
        }
        else return false;
    }
    // Should never hit here
    assert(0);
    return false;
}

bool codeRangeTree::successor(Dyninst::Address key, codeRange * &value) const{
    entry *x = setData;
    entry *last = nil;
    while (x != nil) {
        if (x->key == key) {
            value = x->value;
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
            value = last->value;
            return true;
        }
        else return false;
    }
    // Should never reach this point
    assert(0);
    return false;
}

codeRange ** codeRangeTree::elements(codeRange ** buffer) const{
	if(setData == nil) return NULL;
	if(!buffer) return NULL;
	int tmp = 0;
	traverse(buffer,setData,tmp);	
	return buffer;
}

bool codeRangeTree::elements(std::vector<codeRange *> &buffer) const{
	if(setData == nil) return false;
	traverse(buffer,setData);	
        return true;
}

void codeRangeTree::clear() {
    if (setData == nil) return;
    destroy(setData);
    setData = nil;
    setSize = 0;
}

#define PRINT_COMMA if (print_comma) fprintf(stderr, ", "); print_comma = true
void codeRange::print_range(Dyninst::Address) {
   bool print_comma = false;
   image *img_ptr = is_image();
   mapped_object *mapped_ptr = is_mapped_object();
	func_instance *func_ptr = is_function();
   baseTramp *base_ptr = NULL;
   inferiorRPCinProgress *rpc_ptr = is_inferior_rpc();

   /**
    * The is_* functions above won't give us mulitple layers of objects
    * (i.e the fact we have a function pointer, doesn't mean we have a 
    * mapped_object pointer).  Build up more information from what we have
    **/
   //if (base_ptr && !func_ptr) 
   //   func_ptr = base_ptr->func();
   if (func_ptr && !mapped_ptr)
      mapped_ptr = func_ptr->obj();
   if (mapped_ptr && !img_ptr)
      img_ptr = mapped_ptr->parse_img();

   fprintf(stderr, "[");

   if (img_ptr) {
      PRINT_COMMA;
      fprintf(stderr, "img:%s", img_ptr->name().c_str());
   }
   if (mapped_ptr) {
      PRINT_COMMA;
      fprintf(stderr, "map_obj:%s", mapped_ptr->fullName().c_str());
   }
   if (func_ptr) {
      PRINT_COMMA;
      fprintf(stderr, "func:%s", func_ptr->prettyName().c_str());
   }
   if (base_ptr) {
      PRINT_COMMA;
      fprintf(stderr, "base"); 
   }
   if (rpc_ptr) {
      PRINT_COMMA;
      fprintf(stderr, "rpc:%lx", rpc_ptr->get_address());
   }
   if (!print_comma)
   {
      fprintf(stderr, "Nothing");
   }
   fprintf(stderr, "]\n");
}

