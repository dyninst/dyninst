/************************************************************************
 * Stack.h: implementation of vector-based stacks.
************************************************************************/





#if !defined(_Stack_h_)
#define _Stack_h_





/************************************************************************
 * header files.
************************************************************************/

#include "util/h/Vector.h"





/************************************************************************
 * template<class T> class stack
************************************************************************/

template<class T>
class stack {
public:
             stack ();
             stack (const stack<T> &);
    virtual ~stack ();

    stack<T>&  operator= (const stack<T> &);
    bool      operator== (const stack<T> &) const;
    stack<T>& operator+= (const stack<T> &);
    stack<T>   operator+ (const stack<T> &) const;
    stack<T>   operator- ()                 const;
    unsigned        size ()                 const;
    void            push (const T &);
    T                pop ();
    T&               top ()                 const;
    T&            bottom ()                 const;
    void           clear ();
    void           write (ostream &)        const;

    friend ostream& operator<< (ostream &, const stack<T> &);

private:
    vector<T> data_;
};

template<class T>
inline
stack<T>::stack()
    : data_() {
}

template<class T>
inline
stack<T>::stack(const stack<T>& s)
    : data_(s.data_) {
}

template<class T>
inline
stack<T>::~stack() {
}

template<class T>
inline
stack<T>&
stack<T>::operator=(const stack<T>& s) {
    if (this == &s) {
        return *this;
    }
    data_ = s.data_;
    return *this;
}

template<class T>
inline
bool
stack<T>::operator==(const stack<T>& s) const {
    if (this == &s) {
        return true;
    }
    return (data_ == s.data_);
}

template<class T>
inline
stack<T>&
stack<T>::operator+=(const stack<T>& s) {
    data_ += s.data_;
    return *this;
}

template<class T>
inline
stack<T>
stack<T>::operator+(const stack<T>& s) const {
    stack<T> n = *this;
    n += s;
    return n;
}

template<class T>
inline
stack<T>
stack<T>::operator-() const {
    stack<T> n = *this;
    n.data_ = -n.data_;
    return n;
}

template<class T>
inline
unsigned
stack<T>::size() const {
    return data_.size();
}

template<class T>
inline
void
stack<T>::push(const T& t) {
    data_ += t;
}

template<class T>
inline
T
stack<T>::pop() {
    unsigned sz = data_.size();
    assert(sz != 0);

    T t = data_[sz-1];
    data_.resize(sz-1);
    return t;
}

template<class T>
inline
T&
stack<T>::top() const {
    assert(data_.size() != 0);
    return data_[data_.size()-1];
}

template<class T>
inline
T&
stack<T>::bottom() const {
    assert(data_.size() != 0);
    return data_[0];
}

template<class T>
inline
void
stack<T>::clear() {
    data_.resize(0);
}

template<class T>
inline
void
stack<T>::write(ostream& os) const {
    (-data_).write(os);
}

template<class T>
inline
ostream&
operator<<(ostream& os, const stack<T>& s) {
    os << "[ top: "; s.write(os); return os << " :bottom ]";
}





#endif /* !defined(_Stack_h_) */
