/*
 * Copyright (c) 1996 Barton P. Miller
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

/************************************************************************
 * Vector.h: resizable vectors.
************************************************************************/





#if !defined(_Vector_h_)
#define _Vector_h_

#if defined(external_templates)
#pragma interface
#endif



/************************************************************************
 * header files.
************************************************************************/

#include <assert.h>
#include <stdlib.h>




/************************************************************************
 * template<class T> class Vector
************************************************************************/

#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif

template<class T>
class vector {
public:
    DO_INLINE_F vector (unsigned =0);
    DO_INLINE_F vector (unsigned, const T &);
    DO_INLINE_F vector (const vector<T> &);
    DO_INLINE_F ~vector ();

    DO_INLINE_F vector<T>&  operator= (const vector<T> &);
    DO_INLINE_F vector<T>& operator+= (const vector<T> &);
    DO_INLINE_F vector<T>& operator+= (const T &);

    DO_INLINE_F T&         operator[] (unsigned)                               const;
    DO_INLINE_F bool       operator== (const vector<T> &)                      const;
    DO_INLINE_F unsigned         size ()                                       const;
    DO_INLINE_F void           resize (unsigned);


    DO_INLINE_F void             sort (int (*)(const void *, const void *));

private:
    DO_INLINE_P void  create (unsigned);
    DO_INLINE_P void    init (const T &);
    DO_INLINE_P void    copy (unsigned, const T *);
    DO_INLINE_P void destroy ();

    T*       data_;
    unsigned sz_;
    unsigned tsz_;
};

template<class T>
DO_INLINE_F
vector<T>::vector(unsigned sz)
    : data_(0), sz_(0), tsz_(0) {
    create(sz);
}

template<class T>
DO_INLINE_F
vector<T>::vector(unsigned sz, const T& v0)
    : data_(0), sz_(0), tsz_(0) {
    create(sz);
    init(v0);
}


template<class T>
DO_INLINE_F
vector<T>::vector(const vector<T>& v)
    : data_(0), sz_(0), tsz_(0) {
    create(v.sz_);
    copy(v.sz_, v.data_);
}

template<class T>
DO_INLINE_F
vector<T>::~vector() {
    destroy();
}

template<class T>
DO_INLINE_F
vector<T>&
vector<T>::operator=(const vector<T>& v) {
    if (this == &v) {
        return *this;
    }
    destroy();
    create(v.sz_);
    copy(v.sz_, v.data_);
    return *this;
}

template<class T>
DO_INLINE_F
vector<T>&
vector<T>::operator+=(const vector<T>& v) {
    unsigned osz = sz_;
    resize(osz + v.sz_);
    for (unsigned i = osz; i < sz_; ++i) {
        data_[i] = v.data_[i-osz];
    }
    return *this;
}

template<class T>
DO_INLINE_F
vector<T>&
vector<T>::operator+=(const T& v0) {
    resize(sz_+1);
    data_[sz_-1] = v0;
    return *this;
}

template<class T>
DO_INLINE_F
T&
vector<T>::operator[](unsigned i) const {
    assert(i < sz_);
    return data_[i];
}

template<class T>
DO_INLINE_F
bool
vector<T>::operator==(const vector<T>& v) const {
    if (sz_ != v.sz_) {
        return false;
    }
// TODO
#if defined(notdef)
    for (unsigned i = 0; i < sz_; i++) {
        if (!(data_[i] == v.data_[i])) {
            return false;
        }
    }
#endif
    return true;
}

template<class T>
DO_INLINE_F
unsigned
vector<T>::size() const {
    return sz_;
}

template<class T>
DO_INLINE_F
void
vector<T>::resize(unsigned sz) {
    if (tsz_ == 0) {
        create(sz);
    }
    else if (tsz_ >= sz) {
        sz_ = sz;
    }
    else {
        T*       odata = data_;
        unsigned osz   = sz_;
        unsigned nsz   = (((2*tsz_)>sz)?(2*tsz_):(sz));

        data_ = new T[nsz];
        tsz_  = nsz;
        sz_   = sz;

        for (unsigned i = 0; i < osz; ++i) {
            data_[i] = odata[i];
        }
        delete[] odata;

        sz_ = sz;
    }
}

template<class T>
DO_INLINE_F
void
vector<T>::sort(int (*cmpfunc)(const void *, const void *)) {
    qsort((void *) data_, sz_, sizeof(T), cmpfunc);
}

template<class T>
DO_INLINE_P
void
vector<T>::create(unsigned sz) {
    if (sz > 0) {
        data_ = new T[sz];
    }
    sz_ = tsz_ = sz;
}

template<class T>
DO_INLINE_P
void
vector<T>::init(const T& v0) {
    for (unsigned i = 0; i < sz_; ++i) {
        data_[i] = v0;
    }
}

template<class T>
DO_INLINE_P
void
vector<T>::copy(unsigned sz, const T* data) {
    for (unsigned i = 0; i < sz; ++i) {
        data_[i] = data[i];
    }
}

template<class T>
DO_INLINE_P
void
vector<T>::destroy() {
    delete[] data_; data_ = 0;
    sz_ = tsz_ = 0;
}

#endif /* !defined(_Vector_h_) */
