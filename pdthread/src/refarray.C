#ifndef __libthread_refarray_c__
#define __libthread_refarray_c__

/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
 * excluded.
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

/***************************************************
 * refarray.C:  implementation of reference array class
 * 
 * A refarray is a templated array of references.  It
 * does bounds-checking, is resizable and is optionally 
 * protectable by a newly-allocated or preexisting rwlock.
 *
 ***************************************************/

#include <stdio.h>
#include "refarray.h"
#include "rwlock.h"
#include <assert.h>

/* This operator will return NULL if presented with an out-of-bounds
   index */
template<class REFOBJ, int USE_RWLOCK>
REFOBJ *refarray<REFOBJ,USE_RWLOCK>::operator[](unsigned index) {
    REFOBJ *rv;
    
        /* instead of providing seperate classes for refarray<REFOBJ,0> 
           and refarray<REFOBJ,1>, we "specialize" by using an if block,
           which should be optimized out as dead in the <REFOBJ,0> case */
    
    if(USE_RWLOCK)
        lock->acquire(rwlock::read);
    
    if(index >= _size) {
#if LIBTHREAD_DEBUG
        fprintf(stderr, "%d out of bounds for refarray of size %d\n", index, _size);
#endif
        rv = NULL;
    } else {  
        rv = items[index];
    }
   
    if(USE_RWLOCK)
        lock->release(rwlock::read);
    
    return rv;
}

template<class REFOBJ, int USE_RWLOCK>
void refarray<REFOBJ,USE_RWLOCK>::resize() {
    resize(NULL);
}

template<class REFOBJ, int USE_RWLOCK>
void refarray<REFOBJ,USE_RWLOCK>::resize(REFOBJ* default_value) {
    unsigned new_size;
    unsigned old_size;
    REFOBJ *new_items[];
    int i; /* unfortunately, VC++ does not scope variables declared in
              for loop invariant blocks properly */

    if(USE_RWLOCK)
        lock->acquire(rwlock::read);

    new_size = (_size * 2) + 1;
    old_size = _size;

    if(USE_RWLOCK)
        lock->release(rwlock::read);

    new_items = new (REFOBJ*)[new_size];

    for(i = 0; i < old_size; i++)
        new_items[i] = (*this)[i];
    
    for(i = old_size; i < new_size; i++)
        new_items[i] = default_value;
    
    if(USE_RWLOCK)
        lock->acquire(rwlock::write);
    
    if (new_size > _size) {
        delete [] this->items;
        this->items = new_items;
    } else {
            /* someone has usurped this resize request from us; we
               don't need to copy over our pointers */
        for(i = old_size; i < new_size; i++)
            delete new_items[i];
        delete [] new_items;        
    }

    if(USE_RWLOCK)
        lock->release(rwlock::write);

    return;
    
}

template<class REFOBJ, int USE_RWLOCK>
void refarray<REFOBJ,USE_RWLOCK>::set_elem_at(unsigned index, REFOBJ* value) {
    if(USE_RWLOCK) {
        assert(lock);
        lock->acquire(rwlock::write);
    }
    
    if(index <= _last) {
        _last = index + 1;
    }

    items[index] = value;

    if(USE_RWLOCK) {
        assert(lock);
        lock->release(rwlock::write);
    }
}


template<class REFOBJ, int USE_RWLOCK>
unsigned refarray<REFOBJ,USE_RWLOCK>::push_back(REFOBJ* value) {
    unsigned retval;
    
    if(USE_RWLOCK)
        lock->acquire(rwlock::write);        

    if(_last >= _size) _resize(NULL);

    retval = _last;
    items[_last] = value;

#if LIBTHREAD_DEBUG
    fprintf(stderr, "inserted %p into items[%d]; items[%d] == %p\n", value, _last, _last, items[_last]);
#endif

    _last++;

    if(USE_RWLOCK)
        lock->release(rwlock::write);

    return retval;
}


template<class REFOBJ, int USE_RWLOCK>
void refarray<REFOBJ,USE_RWLOCK>::_resize(REFOBJ* default_value) {
    unsigned new_size;
    unsigned old_size;
    REFOBJ **new_items;

    int i; /* unfortunately, VC++ does not scope variables declared in
              for loop invariant blocks properly */

    new_size = (_size * 2) + 1;
    old_size = _size;

    new_items = new (REFOBJ*)[new_size];

    for(i = 0; i < old_size; i++)
        new_items[i] = (*this)[i];
    
    for(i = old_size; i < new_size; i++)
        new_items[i] = default_value;
    
    if (new_size > _size) {
        delete [] this->items;
        this->items = new_items;
    } else {
            /* someone has usurped this resize request from us; we
               don't need to copy over our pointers */
        for(i = old_size; i < new_size; i++)
            delete new_items[i];
        delete [] new_items;        
    }

    return;    
}
#endif
