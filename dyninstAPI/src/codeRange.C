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

// $Id: codeRange.C,v 1.3 2004/03/23 01:12:02 eli Exp $

#include <stdio.h>
#include "codeRange.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/trampTemplate.h"
#include "dyninstAPI/src/miniTrampHandle.h"
#include "dyninstAPI/src/sharedobject.h"

trampTemplate *codeRange::is_basetramp() {
   return dynamic_cast<trampTemplate *>(this);
}

miniTrampHandle *codeRange::is_minitramp() {
   return dynamic_cast<miniTrampHandle *>(this);
}

pd_Function *codeRange::is_pd_Function() {
   return dynamic_cast<pd_Function *>(this);
}

image *codeRange::is_image() {
   return dynamic_cast<image *>(this);
}

shared_object *codeRange::is_shared_object() {
   return dynamic_cast<shared_object *>(this);
}

relocatedFuncInfo *codeRange::is_relocated_func() {
   return dynamic_cast<relocatedFuncInfo *>(this);
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
	if(!pivot->parent)
		setData = y;
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
	if(!pivot->parent)
		setData = x;
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


codeRangeTree::entry *codeRangeTree::treeInsert(Address key, codeRange *value)
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
	if(!y)
		setData = z;
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


codeRangeTree::entry *codeRangeTree::find_internal(Address element) const{
	entry* x = setData;
	while(x != nil){
        if (element < x->key)
            x = x->left;
        else if (element > x->key)
            x = x->right;
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

//////////////////////////// PUBLIC FUNCTIONS ////////////////////////////////

 void codeRangeTree::insert(Address key, codeRange *value) {
	entry* x = treeInsert(key, value);
	if(!x) return;
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

 void codeRangeTree::remove(Address key){
	entry* z = find_internal(key);
	if(!z)
		return;
	entry* y=((z->left == nil)||(z->right == nil)) ? z : treeSuccessor(z);
	entry* x=(y->left != nil) ? y->left : y->right;
	x->parent = y->parent;
	if(!y->parent)
		setData = x;
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




void codeRangeTree::destroy(entry* node){
	if(!node || (node == nil))
		return;
	if(node->left != nil)
		destroy(node->left);
	if(node->right != nil)
		destroy(node->right);
	delete node;
}

bool codeRangeTree::find(Address key, codeRange *& value) const{
	entry* x = find_internal(key);
   if (!x) return false;
   value = x->value;
   return true;
}

bool codeRangeTree::precessor(Address key, codeRange * &value) const{
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

bool codeRangeTree::successor(Address key, codeRange * &value) const{
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


codeRangeTree::entry *codeRangeTree::replicateTree(entry* node,entry* parent,
                                                   entry* oldNil,entry* newNil)
{
	if(node == oldNil)
		return newNil;	

	entry* newNode = new entry(*node);
	newNode->parent = parent; 
	newNode->left = replicateTree(node->left,newNode,oldNil,newNil);
	newNode->right = replicateTree(node->right,newNode,oldNil,newNil);
	return newNode; 
}




