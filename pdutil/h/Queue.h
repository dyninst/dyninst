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
