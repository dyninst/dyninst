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
