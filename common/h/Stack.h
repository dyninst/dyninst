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
 * Stack.h: implementation of vector-based stacks.
************************************************************************/





#if !defined(_Stack_h_)
#define _Stack_h_

#if defined(external_templates)
#pragma interface
#endif




/************************************************************************
 * header files.
************************************************************************/

#include "util/h/Vector.h"



#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif

/************************************************************************
 * template<class T> class stack
************************************************************************/

template<class T>
class stack {
public:
    DO_INLINE_F         stack ();
    DO_INLINE_F         stack (const stack<T> &);
    DO_INLINE_F virtual ~stack ();

    DO_INLINE_F stack<T>&  operator= (const stack<T> &);
    DO_INLINE_F bool      operator== (const stack<T> &) const;
    DO_INLINE_F stack<T>& operator+= (const stack<T> &);
    DO_INLINE_F stack<T>   operator+ (const stack<T> &) const;
    DO_INLINE_F stack<T>   operator- ()                 const;
    DO_INLINE_F unsigned        size ()                 const;
    DO_INLINE_F void            push (const T &);
    DO_INLINE_F T                pop ();
    DO_INLINE_F T&               top ()                 const;
    DO_INLINE_F T&            bottom ()                 const;
    DO_INLINE_F void           clear ();
#if defined(notdef)
    DO_INLINE_F void           write (ostream &)        const;

    DO_INLINE_F friend ostream& operator<< (ostream &, const stack<T> &);
#endif
private:
    vector<T> data_;
};

template<class T>
DO_INLINE_F
stack<T>::stack()
    : data_() {
}

template<class T>
DO_INLINE_F
stack<T>::stack(const stack<T>& s)
    : data_(s.data_) {
}

template<class T>
DO_INLINE_F
stack<T>::~stack() {
}

template<class T>
DO_INLINE_F
stack<T>&
stack<T>::operator=(const stack<T>& s) {
    if (this == &s) {
        return *this;
    }
    data_ = s.data_;
    return *this;
}

template<class T>
DO_INLINE_F
bool
stack<T>::operator==(const stack<T>& s) const {
    if (this == &s) {
        return true;
    }
    return (data_ == s.data_);
}

template<class T>
DO_INLINE_F
stack<T>&
stack<T>::operator+=(const stack<T>& s) {
    data_ += s.data_;
    return *this;
}

template<class T>
DO_INLINE_F
stack<T>
stack<T>::operator+(const stack<T>& s) const {
    stack<T> n = *this;
    n += s;
    return n;
}

template<class T>
DO_INLINE_F
stack<T>
stack<T>::operator-() const {
    stack<T> n = *this;
    n.data_ = -n.data_;
    return n;
}

template<class T>
DO_INLINE_F
unsigned
stack<T>::size() const {
    return data_.size();
}

template<class T>
DO_INLINE_F
void
stack<T>::push(const T& t) {
    data_ += t;
}

template<class T>
DO_INLINE_F
T
stack<T>::pop() {
    unsigned sz = data_.size();
    assert(sz != 0);

    T t = data_[sz-1];
    data_.resize(sz-1);
    return t;
}

template<class T>
DO_INLINE_F
T&
stack<T>::top() const {
    assert(data_.size() != 0);
    return data_[data_.size()-1];
}

template<class T>
DO_INLINE_F
T&
stack<T>::bottom() const {
    assert(data_.size() != 0);
    return data_[0];
}

template<class T>
DO_INLINE_F
void
stack<T>::clear() {
    data_.resize(0);
}
#if defined(notdef)
template<class T>
DO_INLINE_F
void
stack<T>::write(ostream& os) const {
    (-data_).write(os);
}

template<class T>
DO_INLINE_F
ostream&
operator<<(ostream& os, const stack<T>& s) {
    os << "[ top: "; s.write(os); return os << " :bottom ]";
}
#endif




#endif /* !defined(_Stack_h_) */
