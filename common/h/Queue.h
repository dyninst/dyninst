/************************************************************************
 * Queue.h: implementation of a stack-based queue.
************************************************************************/





#if !defined(_Queue_h_)
#define _Queue_h_





/************************************************************************
 * header files.
************************************************************************/

#include "Stack.h"





/************************************************************************
 * template<class T> class queue
************************************************************************/

template<class T>
class queue {
public:
             queue ();
             queue (const queue<T> &);
    virtual ~queue ();

    queue<T>&  operator= (const queue<T> &);
    bool      operator== (const queue<T> &) const;
    queue<T>& operator+= (const queue<T> &);
    queue<T>   operator+ (const queue<T> &) const;
    queue<T>   operator- ()                 const;
    unsigned        size ()                 const;
    void         enqueue (const T &);
    T            dequeue ();
    T&              head ()                 const;
    T&              tail ()                 const;
    void           clear ();
    void           write (ostream &)        const;

    friend ostream& operator<< (ostream &, const queue<T> &);

private:
    void         reverse ();

    stack<T> in_;
    stack<T> out_;
};

template<class T>
inline
queue<T>::queue()
    : in_(), out_() {
}

template<class T>
inline
queue<T>::queue(const queue<T>& q)
    : in_(q.in_), out_(q.out_) {
}

template<class T>
inline
queue<T>::~queue() {
}

template<class T>
inline
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
inline
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
inline
queue<T>&
queue<T>::operator+=(const queue<T>& q) {
    in_ += (-q.out_ + q.in_);
    return *this;
}

template<class T>
inline
queue<T>
queue<T>::operator+(const queue<T>& q) const {
    queue<T> n = *this;
    n += q;
    return n;
}

template<class T>
inline
queue<T>
queue<T>::operator-() const {
    queue<T> t;
    t.in_  = out_;
    t.out_ = in_;
    return t;
}

template<class T>
inline
unsigned
queue<T>::size() const {
    return (in_.size() + out_.size());
}

template<class T>
inline
void
queue<T>::enqueue(const T& t) {
    in_.push(t);
}

template<class T>
inline
T
queue<T>::dequeue() {
    reverse();
    assert(out_.size() != 0);
    return out_.pop();
}

template<class T>
inline
T&
queue<T>::head() const {
    assert(size() != 0);
    return ((out_.size() > 0) ? out_.top() : in_.bottom());
}

template<class T>
inline
T&
queue<T>::tail() const {
    assert(size() != 0);
    return ((in_.size() > 0) ? in_.top() : out_.bottom());
}

template<class T>
inline
void
queue<T>::clear() {
    in_.clear();
    out_.clear();
}

template<class T>
inline
void
queue<T>::write(ostream& os) const {
    stack<T> t = out_ + (-in_);
    t.write(os);
}

template<class T>
inline
ostream&
operator<<(ostream& os, const queue<T>& q) {
    return os << "[ head: "; q.write(os); return os << " :tail ]";
}

template<class T>
inline
void
queue<T>::reverse() {
    if (out_.size() != 0) {
        return;
    }
    out_ += -in_;
    in_.clear();
}





#endif /* !defined(_Queue_h_) */
