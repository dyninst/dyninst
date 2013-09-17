/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if !defined(_Vector_h_)
#define _Vector_h_

#if !defined (cap_use_pdvector)
#include <vector>
#include <algorithm>
#include <functional>

#define pdvector std::vector
#define VECTOR_SORT(l1, f)    std::sort(l1.begin(), l1.end(), ptr_fun(f));
//  Note on erase:  in STL, the end_pos iterator points to one element beyond the desired
//  erasure range, thus we increment it here.
#define VECTOR_ERASE(v, start_pos, end_pos) v.erase(v.begin() + start_pos, v.begin() + (end_pos + 1))
#define VECTOR_APPEND(v1, v2) v1.insert(v1.end(), v2.begin(), v2.end())
#define GET_ITER(v, pos) (v.begin() + pos)

#else

#if defined(external_templates)
#pragma interface
#endif

#include "common/src/language.h"

#if defined(os_windows)
//turn off 255 char identifier truncation message
#pragma warning (disable: 4786)
#endif

#define VECTOR_APPEND(l1, l2) 	{ for (unsigned int _i=0; _i < (l2).size(); _i++) (l1).push_back((l2)[_i]); }
#define VECTOR_SORT(l1, f) 	l1.sort((qsort_cmpfunc_t)f);
#define VECTOR_ERASE(v, start_pos, end_pos) v.erase(start_pos, end_pos)
#define GET_ITER(v, pos) v.getIter(pos)


#ifdef _KERNEL
#include <sys/types.h>
#include <sys/kmem.h>
#include <sys/debug.h> // ASSERT()
#define assert ASSERT
#else
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#endif

extern "C" {
   typedef int (*qsort_cmpfunc_t)(const void *, const void *);
}

#ifndef _KERNEL
#include <new> // placement (void*) new to coax copy-ctor
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
      if (!result) {
         unsigned long lnbytes = nbytes;
         fprintf(stderr, "%s[%d]:  FATAL:  malloc (%lu) failed\n", __FILE__, __LINE__, lnbytes);
         assert(result);
      }
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
class pdvector {
 public:
   // A few typedefs to be compatible with stl algorithms.
   typedef T        value_type;
   typedef T*       pointer;
   typedef const T* const_pointer;
   typedef T*       iterator;
   typedef const T* const_iterator;
   typedef T&       reference;
   typedef const T& const_reference;

   pdvector() {
      data_ = NULL;
      sz_ = tsz_ = 0;
   }
   
#ifndef _KERNEL
   explicit
#endif
   DO_INLINE_F
   pdvector(unsigned sz) {
      // explicit: don't allow this ctor to be used as an implicit conversion from
      // unsigned to pdvector<T>.  So, this is disallowed:
      // pdvector<T> mypdvector = 3;
      // But this is still allowed, of course:
      // pdvector<T> mypdvector(3);

      // note: if you make use of this method, class T must have a default ctor
      initialize_1(sz, T());
   }
   
   DO_INLINE_F
   pdvector(unsigned sz, const T &t) {
      initialize_1(sz, t);
   }

   DO_INLINE_F
   pdvector(const pdvector<T, A> &src) {
      initialize_copy(src.sz_, src.begin(), src.end());
   }

   DO_INLINE_F
   pdvector(const pdvector<T, A> &src1, const T &src2) {
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
   pdvector(const pdvector<T,A> &src1, const pdvector<T,A> &src2) {
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

   DO_INLINE_F 
   ~pdvector() {
      zap();
   }
   DO_INLINE_F bool operator==(const pdvector<T, A> &) const;

   DO_INLINE_F pdvector<T, A>&  operator=(const pdvector<T, A> &);
   DO_INLINE_F pdvector<T, A>& operator+=(const pdvector<T, A> &);
   DO_INLINE_F pdvector<T, A>& operator+=(const T &);
   DO_INLINE_F 
   pdvector<T, A> operator+(const T &item) const {
      // A horribly slow routine, so use only when convenience
      // outshines the performance loss.
      pdvector<T, A> result(*this);
      result += item;
      return result;
   }

   DO_INLINE_F
   reference operator[] (unsigned i) {
      assert((i < sz_) && data_);
      return *(data_ + i);
   }

   DO_INLINE_F
   const_reference operator[] (unsigned i) const {
      assert((i < sz_) && data_);
      return *(data_ + i);
   }

   DO_INLINE_F
   void pop_back() { // fry the last item in the pdvector
      shrink(sz_ - 1);
   }

   DO_INLINE_F pdvector<T, A>& push_back(const T &);

   // Careful, you'll have errors if you attempt to erase an element while
   // iterating forward over the pdvector.  This is because the erase method
   // copies down later elements down to fill the hole.  Iterate from the end
   // back to the beginning of the pdvector if you intend to erase an element.
   DO_INLINE_F void erase(unsigned start, unsigned end);
   DO_INLINE_F void erase(iterator itr);
   DO_INLINE_F void sort(qsort_cmpfunc_t cmpfunc);

   DO_INLINE_F
   void swap(pdvector<T, A> &other) {
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
      // to the pdvector.

   DO_INLINE_F void clear() {if (sz_ > 0) shrink(0);}
      // prefer clear() to resize(0) because former doesn't require a copy-ctor
      // Doesn't free up any memory (expects you to add to the pdvector later).
      // If you're pretty sure that you're done with pdvector, then prefer zap()
      // instead.

   DO_INLINE_F
   void zap() {
      // like clear() but truly frees up the pdvector's memory; 
      // call when you don't expect to add to the pdvector any more
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
      // reallocation takes place; thus, pointers into the pdvector remain
      // valid until then.
      return tsz_;
   }

   DO_INLINE_F iterator reserve_for_inplace_construction(unsigned nelems);
      // allocates uninitialized memory, sets size(), returns iterator to first item.
      // Use placement void* operator new to initialize in place, eliminating the
      // need for a ctor,copy-ctor sequence.  For expert users only!
   
   DO_INLINE_F iterator append_with_inplace_construction();
      // Appends a single, totally uninitialized member to the pdvector.  Bumps up sz_;
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
pdvector<T, A>&
pdvector<T, A>::operator=(const pdvector<T, A>& src) {
   if (this == &src)
      return *this;

   bool fryAndRecreate = (src.sz_ > tsz_); // or if alloc/free fns differ...

   if (fryAndRecreate) {
      // deconstruct the old pdvector and deallocate it
      destroy();
       
      // allocate the new pdvector and initialize it
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
pdvector<T, A>&
pdvector<T, A>::operator+=(const pdvector<T,A>& src) {
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
pdvector<T, A>&
pdvector<T, A>::operator+=(const T& t) {
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
pdvector<T, A>&
pdvector<T, A>::push_back(const T& t) {
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
void pdvector<T, A>::sort(qsort_cmpfunc_t cmpfunc) {
   if (sz_)
      qsort((void *) data_, sz_, sizeof(T), cmpfunc);
}

template<class T, class A>
DO_INLINE_F
void pdvector<T, A>::shrink(unsigned newsize) {
   if (newsize == sz_) return;
   if (newsize > sz_) {
     fprintf(stderr, "%s[%d]:  FAILING:  cannot shrink %d to %d\n", __FILE__, __LINE__, sz_, newsize);
   }
   assert(newsize < sz_);
   deconstruct_items(begin() + newsize, end());
   sz_ = newsize;
}

template<class T, class A>
DO_INLINE_F
void pdvector<T, A>::grow(unsigned newsize, bool exact) {
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
void pdvector<T, A>::resize(unsigned newsize, bool exact) {
   if (newsize == sz_)
      ;
   else if (newsize < sz_)
      shrink(newsize);
   else
      grow(newsize, exact);
}

template<class T, class A>
DO_INLINE_F
void pdvector<T, A>::erase(unsigned start, unsigned end) {
    int origSz = sz_;
    int emptyIndex = start;
    int copyIndex = end + 1;
    while(copyIndex<origSz) {
        data_[emptyIndex++] = data_[copyIndex++];
    }
    /*resize(origSz - (end - start + 1)); */
    shrink(origSz - (end - start + 1));
}

template<class T, class A>
DO_INLINE_F
void pdvector<T, A>::erase(iterator itr) { 
   unsigned index = itr - data_;
   erase(index, index);
}

template<class T, class A>
DO_INLINE_P
void pdvector<T, A>::initialize_1(unsigned sz, const T &t) {
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
void pdvector<T, A>::initialize_copy(unsigned sz, const T *srcfirst, const T *srclast) {
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
void pdvector<T, A>::destroy() {
   if (data_) {
      deconstruct_items(begin(), end());
      assert(tsz_ > 0);
      A::free(data_, tsz_); // tsz_, not sz_!
      data_ = NULL; // help purify find mem leaks
   }
   else {
      if(sz_ == 0) assert(tsz_==0);
   }
}

template<class T, class A>
DO_INLINE_F
void pdvector<T, A>::reserve_roundup(unsigned nelems) {
   // reserve *at least* nelems elements in the pdvector.
   assert(nelems >= sz_); // don't call us to shrink the pdvector!

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
TYPENAME pdvector<T,A>::iterator
pdvector<T, A>::append_with_inplace_construction() {
   reserve_roundup(sz_ + 1);
      // used to be reserve_exact(), but with huge performance penalty!
   
   ++sz_;
   
   return end()-1;
}


template<class T, class A>
DO_INLINE_F
TYPENAME pdvector<T, A>::iterator
pdvector<T, A>::reserve_for_inplace_construction(unsigned nelems) {
   // NOTE: while I hunt down a certain mem leak, I'm declaring that this
   // routine must only be called on a default-constructor pdvector or one that
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
void pdvector<T, A>::reserve_exact(unsigned nelems) {
   // exact because we're not gonna round up nelems to a power of 2
   // note: sz_ doesn't change

   assert(nelems >= sz_); // don't use to shrink the pdvector!

   // Don't do a 0-size allocation, weirdness results
   if (nelems == 0) 
       return; 

   //if (tsz_ >= nelems)
   //return; // nothing needed

   // Alloc new pdvector
   T *newdata = A::alloc(nelems);

   // Copy items over, if any
   if (data_) {
      assert(tsz_ > 0); // it would be too strict to also assert that sz_ > 0
      copy_into_uninitialized_space(newdata, // dest
                                    begin(), end());
   }
   else
      assert(tsz_ == 0 && sz_ == 0);

   // Fry the old pdvector
   destroy();
   
   // Finalize
   data_ = newdata;
   tsz_ = nelems;
   // note: sz_ doesn't change (on purpose)
}

template<class T, class A>
DO_INLINE_P
bool
find(const pdvector<T, A> &v, const T &v0, unsigned &l) {
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
bool pdvector<T, A>::operator== (const pdvector<T,A> &) const
{
  return false;
}
#endif /* cap_use_pdvector */
#endif /* !defined(_Pdvector_h_) */
