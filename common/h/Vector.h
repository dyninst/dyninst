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
    DO_INLINE_F vector<T>&  operator= (const T &);
    DO_INLINE_F vector<T>& operator+= (const vector<T> &);
    DO_INLINE_F vector<T>& operator+= (const T &);
    DO_INLINE_F vector<T>   operator+ (const vector<T> &)                      const;
    DO_INLINE_F vector<T>   operator- ()                                       const;

    DO_INLINE_F T&         operator[] (unsigned)                               const;
    DO_INLINE_F bool       operator== (const vector<T> &)                      const;
    DO_INLINE_F unsigned         size ()                                       const;
    DO_INLINE_F void           resize (unsigned);


    DO_INLINE_F void             sort (int (*)(const void *, const void *));
    DO_INLINE_F bool           sorted (int (*)(const void *, const void *))    const;
    DO_INLINE_F void              app (void (*)(T &));
    DO_INLINE_F void           revapp (void (*)(T &));
    DO_INLINE_F T                fold (T (*)(const T &, const T &), const T &) const;
    DO_INLINE_F T             revfold (T (*)(const T &, const T &), const T &) const;

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
vector<T>::operator=(const T& v0) {
    init(v0);
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
vector<T>
vector<T>::operator+(const vector<T>& v) const {
    vector<T> ret = *this;
    return ret += v;
}

template<class T>
DO_INLINE_F
vector<T>
vector<T>::operator-() const {
    vector<T> ret(sz_);
    for (unsigned i = 0; i < sz_; ++i) {
        ret.data_[sz_-i-1] = data_[i];
    }
    return ret;
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
DO_INLINE_F
bool
vector<T>::sorted(int (*cmpfunc)(const void *, const void *)) const {
    if (sz_ <= 1) {
        return true;
    }
    for (unsigned i = 0; i < (sz_-1); ++i) {
        if (cmpfunc(&data_[i], &data_[i+1]) > 0) {
            return false;
        }
    }
    return true;
}

template<class T>
DO_INLINE_F
void
vector<T>::app(void (*f)(T &)) {
    for (unsigned i = 0; i < sz_; ++i) {
        f(data_[i]);
    }
}

template<class T>
DO_INLINE_F
void
vector<T>::revapp(void (*f)(T &)) {
    for (int i = sz_-1; i >= 0; --i) {
        f(data_[i]);
    }
}

template<class T>
DO_INLINE_F
T
vector<T>::fold(T (*f)(const T &, const T &), const T& arg0) const {
    T ret = arg0;
    for (int i = sz_-1; i >= 0; --i) {
        ret = f(data_[i], ret);
    }
    return ret;
}

template<class T>
DO_INLINE_F
T
vector<T>::revfold(T (*f)(const T &, const T &), const T& arg0) const {
    T ret = arg0;
    for (unsigned i = 0; i < sz_; ++i) {
        ret = f(ret, data_[i]);
    }
    return ret;
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

template<class T, class U>
DO_INLINE_F
vector<U>
map(U (*f)(const T &), const vector<T>& v) {
    vector<U> ret(sz_);
    for (unsigned i = 0; i < sz_; ++i) {
        ret[i] = f(data_[i]);
    }
    return ret;
}

template<class T, class U>
DO_INLINE_F
U
fold(U (*f)(const T &, const U &), const vector<T>& v, const U& arg0) {
    U ret = arg0;
    for (int i = sz_-1; i >= 0; --i) {
        ret = f(data_[i], ret);
    }
    return ret;
}

template<class T, class U>
DO_INLINE_F
U
revfold(U (*f)(const U &, const T &), const vector<T>& v, const U& arg0) {
    U ret = arg0;
    for (unsigned i = 0; i < sz_; ++i) {
        ret = f(ret, data_[i]);
    }
    return ret;
}





#endif /* !defined(_Vector_h_) */
