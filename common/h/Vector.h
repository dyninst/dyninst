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

#if !defined(_Vector_h_)
#define _Vector_h_

#if defined(external_templates)
#pragma interface
#endif

//ifdef this to nothing if it bothers old broken compilers
#define TYPENAME typename

#if ! defined( TYPENAME31 )
#if ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ == 1 )
#define TYPENAME31 typename
#else
#define TYPENAME31
#endif
#endif

#if defined(i386_unknown_nt4_0)
//turn off 255 char identifier truncation message
#pragma warning (disable: 4786)
#endif

#ifdef USE_STL_VECTOR
#include <stdlib.h>
#include <stl.h>
extern "C" {
typedef int (*qsort_cmpfunc_t)(const void *, const void *);
}
#define VECTOR_APPEND(l1, l2) 	{ for (unsigned int _i=0; _i < (l2).size(); _i++) (l1).push_back((l2)[_i]); }
#define VECTOR_SORT(l1, f) 	qsort((l1).begin(),(l1).size(),sizeof((l1).front()), (qsort_cmpfunc_t) f);
/* #define VECTOR_SORT(l1, f) 	stable_sort((l1).begin(),(l1).end(),f); */

#else

#define VECTOR_APPEND(l1, l2) 	{ for (unsigned int _i=0; _i < (l2).size(); _i++) (l1).push_back((l2)[_i]); }
#define VECTOR_SORT(l1, f) 	l1.sort((qsort_cmpfunc_t)f);

#ifdef _KERNEL
#include <sys/types.h>
#include <sys/kmem.h>
#include <sys/debug.h> // ASSERT()
#define assert ASSERT
#else
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#endif

#ifndef _KERNEL
extern "C" {
typedef int (*qsort_cmpfunc_t)(const void *, const void *);
}
#endif


#ifndef _KERNEL
#include <new.h> // placement (void*) new to coax copy-ctor
#elif defined(_LP64)
inline void *operator new(unsigned long /* size_t */, void *place) { 
   return place; 
}
#else
inline void *operator new(unsigned /* size_t does not work */, void *place) { 
   return place; 
}
#endif

#ifndef NULL
#define NULL 0
#endif

#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif

template <class T>
class vec_stdalloc {
 public:
   static T *alloc(unsigned nelems) {
      const size_t nbytes = nelems * sizeof(T);
      
#ifdef _KERNEL
      T* result = (T*)kmem_alloc(nbytes, KM_SLEEP);
      // no need to assert(result) for the kernel due to KM_SLEEP flag
#else
      T* result = (T*)malloc(nbytes);
      assert(result);
#endif

      return result;
   }

#ifdef _KERNEL
   static void free(T *vec, unsigned nelems) {
      kmem_free(vec, sizeof(T)*nelems);
   }
#else   
   static void free(T *vec, unsigned /*nelems*/) {
      ::free(vec);
   }
#endif
};

template<class T, class A=vec_stdalloc<T> >
class vector {
   enum { REMOVEONEELEM = 99999999 };
 public:
   // A few typedefs to be compatible with stl algorithms.
   typedef T        value_type;
   typedef T*       pointer;
   typedef const T* const_pointer;
   typedef T*       iterator;
   typedef const T* const_iterator;
   typedef T&       reference;
   typedef const T& const_reference;

   vector() {
      data_ = NULL;
      sz_ = tsz_ = 0;
   }
   
#ifndef _KERNEL
   explicit
#endif
   DO_INLINE_F
   vector(unsigned sz) {
      // explicit: don't allow this ctor to be used as an implicit conversion from
      // unsigned to vector<T>.  So, this is disallowed:
      // vector<T> myvector = 3;
      // But this is still allowed, of course:
      // vector<T> myvector(3);

      // note: if you make use of this method, class T must have a default ctor
      initialize_1(sz, T());
   }
   
   DO_INLINE_F
   vector(unsigned sz, const T &t) {
      initialize_1(sz, t);
   }

   DO_INLINE_F
   vector(const vector<T, A> &src) {
      initialize_copy(src.sz_, src.begin(), src.end());
   }

   DO_INLINE_F
   vector(const vector<T, A> &src1, const T &src2) {
      // initialize as: this = src1; this += src2;
      sz_ = tsz_ = src1.size() + 1;
      data_ = A::alloc(sz_);
      assert(data_);

      // copy all of 'src1':
      copy_into_uninitialized_space(data_, // dest
                                    src1.begin(), src1.end());

      // copy all of 'src2':
      (void)new((void*)(data_ + sz_ - 1))T(src2);
   }

   DO_INLINE_F
   vector(const vector<T,A> &src1, const vector<T,A> &src2) {
      // initialize as: this = src1; this += src2;
      sz_ = tsz_ = src1.size() + src2.size();
      data_ = A::alloc(sz_);
      assert(data_);
      
      // copy all of 'src1':
      copy_into_uninitialized_space(data_, // dest
                                    src1.begin(), src1.end());
      
      // copy all of 'src2':
      copy_into_uninitialized_space(data_ + src1.size(), // dest
                                    src2.begin(), src2.end());
   }      

//  // Sun's version 4.2 compilers can't handle templated methods, but egcs/g++ can
//  #ifdef __GNUC__
//     template <class OTHERALLOC>
//     explicit vector(const vector<T, OTHERALLOC> &src) {
//        initialize_copy(src.size(), src.begin(), src.end());
//     }
//  #endif
   
   DO_INLINE_F 
   ~vector() {
      destroy();
   }
   DO_INLINE_F bool operator==(const vector<T, A> &) const;

   DO_INLINE_F vector<T, A>&  operator=(const vector<T, A> &);
   DO_INLINE_F vector<T, A>& operator+=(const vector<T, A> &);
   DO_INLINE_F vector<T, A>& operator+=(const T &);
   DO_INLINE_F 
   vector<T, A> operator+(const T &item) const {
      // A horribly slow routine, so use only when convenience
      // outshines the performance loss.
      vector<T, A> result(*this);
      result += item;
      return result;
   }

   DO_INLINE_F
   reference operator[] (unsigned i) {
      assert(i < sz_);
      return *(data_ + i);
   }

   DO_INLINE_F
   const_reference operator[] (unsigned i) const {
      assert(i < sz_);
      return *(data_ + i);
   }

   DO_INLINE_F
   void pop_back() { // fry the last item in the vector
      shrink(sz_ - 1);
   }

   DO_INLINE_F vector<T, A>& push_back(const T &);
   DO_INLINE_F void erase (unsigned start, unsigned end = REMOVEONEELEM);
   DO_INLINE_F void sort (int (*)(const void *, const void *));

   DO_INLINE_F
   void swap(vector<T, A> &other) {
      T *mydata = data_;
      const unsigned mysz = sz_;
      const unsigned mytsz = tsz_;
      
      data_ = other.data_;
      sz_ = other.sz_;
      tsz_ = other.tsz_;

      other.data_ = mydata;
      other.sz_ = mysz;
      other.tsz_ = mytsz;
   }

   DO_INLINE_F reference       front()       { return *begin(); }
   DO_INLINE_F const_reference front() const { return *begin(); }
   DO_INLINE_F reference       back()       { return *(end()-1); }
   DO_INLINE_F const_reference back() const { return *(end()-1); }

   DO_INLINE_F unsigned size() const {return sz_;}
   
   DO_INLINE_F void resize(unsigned, bool exact=true);
      // note: if you use this method, class T must have a default ctor
      // this method simply calls shrink() or grow(), as appropriate.

   DO_INLINE_F void shrink(unsigned newsize);
      // doesn't free up any memory, so well suited if you plan to later re-add
      // to the vector.

   DO_INLINE_F void clear() {if (sz_ > 0) shrink(0);}
      // prefer clear() to resize(0) because former doesn't require a copy-ctor
      // Doesn't free up any memory (expects you to add to the vector later).
      // If you're pretty sure that you're done with vector, then prefer zap()
      // instead.

   DO_INLINE_F
   void zap() {
      // like clear() but truly frees up the vector's memory; 
      // call when you don't expect to add to the vector any more
      destroy();
      sz_ = tsz_ = 0;
   }
      
   DO_INLINE_F void grow(unsigned newsize, bool exact=false);
      // note: if you use this method, class T must have a default ctor

   DO_INLINE_F void reserve(unsigned nelems) { reserve_exact(nelems); }
      // for STL compatibility
   DO_INLINE_F void reserve_exact(unsigned nelems);
   DO_INLINE_F void reserve_roundup(unsigned nelems);
   
   DO_INLINE_F
   unsigned capacity() const {
      // capacity()-size() is the # of elems that can be inserted before
      // reallocation takes place; thus, pointers into the vector remain
      // valid until then.
      return tsz_;
   }

   DO_INLINE_F iterator reserve_for_inplace_construction(unsigned nelems);
      // allocates uninitialized memory, sets size(), returns iterator to first item.
      // Use placement void* operator new to initialize in place, eliminating the
      // need for a ctor,copy-ctor sequence.  For expert users only!
   
   DO_INLINE_F iterator append_with_inplace_construction();
      // Appends a single, totally uninitialized member to the vector.  Bumps up sz_;
      // reserves as needed.
   
   // iteration, stl style:
   DO_INLINE_F iterator       begin()       { return data_; }
   DO_INLINE_F const_iterator begin() const { return data_; }
   DO_INLINE_F iterator       end()       { return begin() + sz_; }
   DO_INLINE_F const_iterator end() const { return begin() + sz_; }

   DO_INLINE_F iterator       getIter(unsigned index)       { 
     assert(index < sz_);
     return data_ + index; 
   }
   DO_INLINE_F const_iterator getIter(unsigned index) const { 
     assert(index < sz_);
     return data_ + index;
   }

 private:
   DO_INLINE_P
   static void deconstruct_items(T *first, T *last) {
      for (; first != last; ++first) 
         first->~T();
   }

   DO_INLINE_P
   static void copy_into_uninitialized_space(T *dest, const T *src_first,
                                             const T *src_last) {
      // manually call copy-ctor to copy all the items from src to dest, assuming
      // that dest is uninitialized memory but src is properly initialized.
                                                
      while (src_first != src_last)
         new((void*)dest++) T(*src_first++);
   }

   DO_INLINE_P
   static void copy_1item_into_uninitialized_space(T *dest, const T &src,
                                                   unsigned n) {
      while (n--)
         new((void*)dest++) T(src);
   }

   DO_INLINE_P void initialize_1(unsigned sz, const T &t);
   DO_INLINE_P void initialize_copy(unsigned sz, const T *srcfirst, const T *srcend);

   DO_INLINE_P void destroy();

   T*       data_;
   unsigned sz_;
   unsigned tsz_;
};

template<class T, class A>
DO_INLINE_F
vector<T, A>&
vector<T, A>::operator=(const vector<T, A>& src) {
   if (this == &src)
      return *this;

   bool fryAndRecreate = (src.sz_ > tsz_); // or if alloc/free fns differ...

   if (fryAndRecreate) {
      // deconstruct the old vector and deallocate it
      destroy();
       
      // allocate the new vector and initialize it
      initialize_copy(src.sz_, src.begin(), src.end());
   }
   else {
      // deconstruct existing items
      deconstruct_items(begin(), end());
       
      sz_ = src.sz_; // tsz_ left unchanged
      copy_into_uninitialized_space(data_, // dest
                                    src.begin(), src.end());
   }

   return *this;
}

template<class T, class A>
DO_INLINE_F
vector<T, A>&
vector<T, A>::operator+=(const vector<T,A>& src) {
   const unsigned newsize = sz_ + src.sz_;
   if (newsize > tsz_)
      reserve_roundup(newsize);
   
   copy_into_uninitialized_space(data_ + sz_, // dest
                                 src.begin(), src.end());

   sz_ += src.sz_;
   assert(tsz_ >= sz_); // reserve_roundup() made this so

   return *this;
}

template<class T, class A>
DO_INLINE_F
vector<T, A>&
vector<T, A>::operator+=(const T& t) {
   const unsigned newsize = sz_ + 1;
   if (newsize > tsz_)
      reserve_roundup(newsize);
   
   copy_1item_into_uninitialized_space(data_ + sz_, // dest
                                       t, // src item
                                       1);
   sz_++;
   assert(tsz_ >= sz_); // reserve_roundup() made this so

   return *this;
}

template<class T, class A>
DO_INLINE_F
vector<T, A>&
vector<T, A>::push_back(const T& t) {
   const unsigned newsize = sz_ + 1;
   if (newsize > tsz_)
      reserve_roundup(newsize);
   
   copy_1item_into_uninitialized_space(data_ + sz_, // dest
                                       t, // src item
                                       1);
   sz_++;
   assert(tsz_ >= sz_); // reserve_roundup() made this so

   return *this;
}

template<class T, class A>
DO_INLINE_F
void vector<T, A>::sort(int (*cmpfunc)(const void *, const void *)) {
   qsort((void *) data_, sz_, sizeof(T), (qsort_cmpfunc_t)cmpfunc);
}

template<class T, class A>
DO_INLINE_F
void vector<T, A>::shrink(unsigned newsize) {
   assert(newsize < sz_);
   deconstruct_items(begin() + newsize, end());
   sz_ = newsize;
}

template<class T, class A>
DO_INLINE_F
void vector<T, A>::grow(unsigned newsize, bool exact) {
   // reallocate if tsz_ isn't big enough
   if (exact)
      reserve_exact(newsize);
   else
      reserve_roundup(newsize);
      
   copy_1item_into_uninitialized_space(data_ + sz_, // dest
                                       T(), // src item
                                       newsize - sz_);
   sz_ = newsize;
   assert(tsz_ >= sz_);
}


template<class T, class A>
DO_INLINE_F
void vector<T, A>::resize(unsigned newsize, bool exact) {
   if (newsize == sz_)
      ;
   else if (newsize < sz_)
      shrink(newsize);
   else
      grow(newsize, exact);
}

template<class T, class A>
DO_INLINE_F
void vector<T, A>::erase(unsigned start, unsigned end) {
    int origSz = sz_;
    int emptyIndex = start;
    if(end == REMOVEONEELEM)  
        end = start;
    int copyIndex = end + 1;
    while(copyIndex<origSz) {
        data_[emptyIndex++] = data_[copyIndex++];
    }
    resize(origSz - (end - start + 1));
}

template<class T, class A>
DO_INLINE_P
void vector<T, A>::initialize_1(unsigned sz, const T &t) {
   sz_ = tsz_ = sz;
   if (sz_ > 0) {
      data_ = A::alloc(sz_);
      assert(data_);
      copy_1item_into_uninitialized_space(data_, t, sz_);
   }
   else
      data_ = NULL;
}

template<class T, class A>
DO_INLINE_P
void vector<T, A>::initialize_copy(unsigned sz, const T *srcfirst, const T *srclast) {
   sz_ = tsz_ = sz;
   if (sz_ > 0) {
      data_ = A::alloc(sz_);
      assert(data_ && srcfirst && srclast);
      copy_into_uninitialized_space(data_, // dest
                                    srcfirst, srclast);
   }
   else
      data_ = NULL;
}

template<class T, class A>
DO_INLINE_P
void vector<T, A>::destroy() {
   if (data_) {
      deconstruct_items(begin(), end());
      assert(tsz_ > 0); // it would be too strict to also assert that sz_ > 0
      A::free(data_, tsz_); // tsz_, not sz_!
      data_ = NULL; // help purify find mem leaks
   }
   else {
      if(sz_ == 0) assert(tsz_==0);
   }
}

template<class T, class A>
DO_INLINE_F
void vector<T, A>::reserve_roundup(unsigned nelems) {
   // reserve *at least* nelems elements in the vector.
   assert(nelems >= sz_); // don't call us to shrink the vector!

   if (tsz_ > nelems)
      return; // nothing needed...(neat optimization)

   // round up 'nelems' to a power of two
   unsigned roundup_nelems = 1;
   while (roundup_nelems < nelems)
      roundup_nelems <<= 1; // multiply by 2

   assert(roundup_nelems >= nelems);
   
   reserve_exact(roundup_nelems);
}

template<class T, class A>
DO_INLINE_F
TYPENAME vector<T, A>::iterator
vector<T, A>::append_with_inplace_construction() {
   reserve_roundup(sz_ + 1);
      // used to be reserve_exact(), but with huge performance penalty!
   
   ++sz_;
   
   return end()-1;
}


template<class T, class A>
DO_INLINE_F
TYPENAME vector<T, A>::iterator
vector<T, A>::reserve_for_inplace_construction(unsigned nelems) {
   // NOTE: while I hunt down a certain mem leak, I'm declaring that this
   // routine must only be called on a default-constructor vector or one that
   // has been zap()'d (clear(0) or shrink(0) is not sufficient).

   assert(sz_ == 0);
   assert(tsz_ == 0); // too strong an assert? [see above]
   assert(data_ == NULL); // too strong an assert? [see above]

   if (nelems > 0) {
      //   reserve_exact(nelems);

      // we use the following "optimized reserve_exact()" to avoid the need for a
      // copy-ctor.  If we remove the above asserts then we may not have the luxury
      // of assuming such a trivial reserve_exact().
      data_ = A::alloc(nelems); // part 1 of our optimized reserve_exact()
      tsz_ = nelems; // part 2 of our optimized reserve_exact()
   }
   
   assert(sz_ == 0);
   assert(tsz_ >= nelems);
   if (nelems > 0)
      assert(data_ != NULL);

   sz_ = nelems;
   
   return begin();
}

template<class T, class A>
DO_INLINE_F
void vector<T, A>::reserve_exact(unsigned nelems) {
   // exact because we're not gonna round up nelems to a power of 2
   // note: sz_ doesn't change

   assert(nelems >= sz_); // don't use to shrink the vector!

   if (tsz_ >= nelems)
      return; // nothing needed

   // Alloc new vector
   T *newdata = A::alloc(nelems);

   // Copy items over, if any
   if (data_) {
      assert(tsz_ > 0); // it would be too strict to also assert that sz_ > 0
      copy_into_uninitialized_space(newdata, // dest
                                    begin(), end());
   }
   else
      assert(tsz_ == 0 && sz_ == 0);

   // Fry the old vector
   destroy();
   
   // Finalize
   data_ = newdata;
   tsz_ = nelems;
   // note: sz_ doesn't change (on purpose)
}

template<class T, class A>
DO_INLINE_P
bool
find(const vector<T, A> &v, const T &v0, unsigned &l) {
	unsigned i, sz;
	sz = v.size();
	for( i = 0; i < sz; ++i ) {
		if( v[ i ] == v0 ) {
			l = i;
			return true;
		}
	}
	return false;
}

template<class T, class A>
DO_INLINE_P
bool vector<T, A>::operator== (const vector<T,A> &) const
{
  return false;
}

#endif /* ifdef USE_STL_VECTOR */

#endif /* !defined(_Vector_h_) */








