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
 * refarray.h:  declaration of reference array class
 * 
 * A refarray is a templated array of references.  It
 * does bounds-checking, is resizable and is optionally 
 * protectable by a newly-allocated or preexisting lock.
 *
 ***************************************************/

#ifndef __libthread_refarray_h__
#define __libthread_refarray_h__

#include "xplat/Mutex.h"

namespace pdthr
{

template <class REFOBJ>
class refarray {
  private:
    REFOBJ **items;
    XPlat::Mutex *lock;
    unsigned _size;
    unsigned _last;
    void _resize(REFOBJ* default_value);
    bool destroy_constituents;
    bool use_lock;

  public:

    refarray( bool _use_lock=true )
      : _size( 1024 ),
        destroy_constituents( false ),
        lock( new XPlat::Mutex ),
        items( NULL ),
        _last( 0 ),
        use_lock( _use_lock )
    {
        items = new REFOBJ*[_size];
        for(unsigned int i = 0; i < this->_size; i++)
        {
            items[i] = NULL;
        }
    }
    

    refarray(int sz,
                XPlat::Mutex *lk,
                bool _use_lock = true,
                REFOBJ* def_value = NULL)
      : _size( sz ),
        lock( lk ),
        destroy_constituents( false ),
        items( NULL ),
        _last( 0 ),
        use_lock( _use_lock )
    {
        if(!lk)
        {
            lock = new XPlat::Mutex;
        }
        
        items = new REFOBJ*[_size];
        for(unsigned int i = 0; i < sz; i++)
        {
            items[i] = def_value;
        }
    }



    ~refarray() {
        if(destroy_constituents)
        {
            for(unsigned int i = 0; i < _size; i++)
            {
                delete items[i];
            }
        }
        delete [] items;
    }
    
    inline REFOBJ *operator[](unsigned index);

    inline void set_destroy_constituents() {
        destroy_constituents = true;
    }
    
    inline unsigned capacity() {
        unsigned sz;

        if( use_lock )
        {
            lock->Lock();
        }

        sz = _size;

        if( use_lock )
        {
            lock->Unlock();
        }

        return sz;
    }

    void set_elem_at(unsigned index, REFOBJ* value);
    unsigned push_back(REFOBJ* value);

    void resize();
    void resize(REFOBJ* default_value);

};

} // namespace pdthr

#endif /*  __libthread_refarray_h__ */
