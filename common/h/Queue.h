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
 * Queue.h: implementation of a stack-based queue.
************************************************************************/





#if !defined(_Queue_h_)
#define _Queue_h_

#if defined(external_templates)
#pragma interface
#endif




/************************************************************************
 * header files.
************************************************************************/

#include "util/h/Stack.h"




#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif


/************************************************************************
 * template<class T> class queue
************************************************************************/

template<class T>
class queue {
public:
    DO_INLINE_F         queue ();
    DO_INLINE_F         queue (const queue<T> &);
    DO_INLINE_F virtual ~queue ();

    DO_INLINE_F queue<T>&  operator= (const queue<T> &);
    DO_INLINE_F bool      operator== (const queue<T> &) const;
    DO_INLINE_F queue<T>& operator+= (const queue<T> &);
    DO_INLINE_F queue<T>   operator+ (const queue<T> &) const;
    DO_INLINE_F queue<T>   operator- ()                 const;
    DO_INLINE_F unsigned        size ()                 const;
    DO_INLINE_F void         enqueue (const T &);
    DO_INLINE_F T            dequeue ();
    DO_INLINE_F T&              head ()                 const;
    DO_INLINE_F T&              tail ()                 const;
    DO_INLINE_F void           clear ();
#if defined(notdef)
    DO_INLINE_F void           write (ostream &)        const;

    DO_INLINE_F friend ostream& operator<< (ostream &, const queue<T> &);
#endif
private:
    DO_INLINE_P void         reverse ();

    stack<T> in_;
    stack<T> out_;
};

template<class T>
DO_INLINE_F
queue<T>::queue()
    : in_(), out_() {
}

template<class T>
DO_INLINE_F
queue<T>::queue(const queue<T>& q)
    : in_(q.in_), out_(q.out_) {
}

template<class T>
DO_INLINE_F
queue<T>::~queue() {
}

template<class T>
DO_INLINE_F
queue<T>&
queue<T>::operator=(const queue<T>& q) {
    if (this == &q) {
        return *this;
    }
    in_  = q.in_;
    out_ = q.out_;
    return *this;
}

template<class T>
DO_INLINE_F
bool
queue<T>::operator==(const queue<T>& q) const {
    if (this == &q) {
        return true;
    }

    if ((in_ == q.in_) && (out_ == q.out_)) {
        return true;;
    }

    if (size() != q.size()) {
        return false;
    }

    stack<T> t1 = out_   + (-in_);
    stack<T> t2 = q.out_ + (-q.in_);
    return (t1 == t2);
}

template<class T>
DO_INLINE_F
queue<T>&
queue<T>::operator+=(const queue<T>& q) {
    in_ += (-q.out_ + q.in_);
    return *this;
}

template<class T>
DO_INLINE_F
queue<T>
queue<T>::operator+(const queue<T>& q) const {
    queue<T> n = *this;
    n += q;
    return n;
}

template<class T>
DO_INLINE_F
queue<T>
queue<T>::operator-() const {
    queue<T> t;
    t.in_  = out_;
    t.out_ = in_;
    return t;
}

template<class T>
DO_INLINE_F
unsigned
queue<T>::size() const {
    return (in_.size() + out_.size());
}

template<class T>
DO_INLINE_F
void
queue<T>::enqueue(const T& t) {
    in_.push(t);
}

template<class T>
DO_INLINE_P
T
queue<T>::dequeue() {
    reverse();
    assert(out_.size() != 0);
    return out_.pop();
}

template<class T>
DO_INLINE_F
T&
queue<T>::head() const {
    assert(size() != 0);
    return ((out_.size() > 0) ? out_.top() : in_.bottom());
}

template<class T>
DO_INLINE_F
T&
queue<T>::tail() const {
    assert(size() != 0);
    return ((in_.size() > 0) ? in_.top() : out_.bottom());
}

template<class T>
DO_INLINE_F
void
queue<T>::clear() {
    in_.clear();
    out_.clear();
}
#if defined(notdef)
template<class T>
DO_INLINE_F
void
queue<T>::write(ostream& os) const {
    stack<T> t = out_ + (-in_);
    t.write(os);
}

template<class T>
DO_INLINE_F
ostream&
operator<<(ostream& os, const queue<T>& q) {
    return os << "[ head: "; q.write(os); return os << " :tail ]";
}
#endif
template<class T>
DO_INLINE_F
void
queue<T>::reverse() {
    if (out_.size() != 0) {
        return;
    }
    out_ += -in_;
    in_.clear();
}





#endif /* !defined(_Queue_h_) */
