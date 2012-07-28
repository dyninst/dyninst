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

// $Id: BPatch_Vector.h,v 1.22 2006/08/24 11:19:24 jaw Exp $

#ifndef _BPatch_Vector_h_
#define _BPatch_Vector_h_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if !defined(USE_DEPRECATED_BPATCH_VECTOR)
#include <vector>
#include <algorithm>
#define BPatch_Vector std::vector

#else
#include "BPatch_dll.h"
#ifdef external_templates
#pragma interface
#endif /* external_templates */

template<class T>
class BPATCH_DLL_EXPORT BPatch_Vector {
    int		reserved;	// Number of objects for which space is reserved
    int		len;		// The current number of objects in the vector
    T*		data;		// Points to the array of objects

    void	reserve(int n);
    inline void	copy_from(const BPatch_Vector &);
public:
    BPatch_Vector();
    BPatch_Vector(int n);
    BPatch_Vector(const BPatch_Vector<T> &);
    ~BPatch_Vector();

    BPatch_Vector<T>& operator=(const BPatch_Vector<T> &);
    BPatch_Vector<T>& operator+=(const BPatch_Vector<T> &);

    unsigned int size() const { return len; }
    void	push_back(const T& x);
    void	push_front(const T& x);
    void        clear();
    void        resize(int sz);
    void erase(int n);

    T&		operator[](int n) const;
};

// VG(06/15/02): VC.NET doesn't like definitions for dll imports
#ifndef BPATCH_DLL_IMPORT

// Reserve space for at least n entries in the vector
template<class T>
void BPatch_Vector<T>::reserve(int n)
{
   if (n > reserved) { // If we haven't already reserved enough space
      // Generally we double the size each time we reserve memory, so
      // that we're not doing it for every insertion.
      if (reserved*2 > n) n = reserved*2;

      // Create a new array with enough space
      T* new_data = new T[n];
      
      // Copy the entries from the old array to the new one
      for (int i = 0; i < len; i++)
         new_data[i] = data[i];

      // Get rid of the old array and set up to use the new one
      if( data != NULL ) delete [] data;
      data = new_data;
      reserved = n;
   }
   assert(data != NULL || n == 0);
}

// Copy the contents of another vector into this one.
template<class T>
inline void BPatch_Vector<T>::copy_from(const BPatch_Vector &src)
{
    reserved = src.reserved;
    len      = src.len;
	if( reserved == 0 )
		data = NULL;
	else {
		data     = new T[reserved];

		for (int i = 0; i < src.len; i++)
			data[i] = src.data[i];
	}
}

// Contructor.  Takes one optional parameter, the number of entries for which
// to reserve space.
template<class T>
BPatch_Vector<T>::BPatch_Vector(int n) : reserved(0), len(0), data(NULL)
{
    assert(n >= 0);
    if (n > 0) reserve(n);
}

// Default constructor
template<class T>
BPatch_Vector<T>::BPatch_Vector() : reserved(0), len(0), data(NULL) {}

// Copy constructor.
template<class T>
BPatch_Vector<T>::BPatch_Vector(const BPatch_Vector<T> &src)
{
    copy_from(src);
}

// Destructor.  Frees allocated memory.
template<class T>
BPatch_Vector<T>::~BPatch_Vector()
{
	if( data != NULL ) delete [] data;
}

// Assignment operator.  Delete the contents of this vector and copy the
// contents of the other vector into it.
template<class T>
BPatch_Vector<T>& BPatch_Vector<T>::operator=(const BPatch_Vector<T> &src)
{
	if( data != NULL ) delete [] data;

    copy_from(src);

    return *this;
}

// Append operator.  Keep data and append the
// contents of the other vector into it.
template<class T>
BPatch_Vector<T>& BPatch_Vector<T>::operator+=(const BPatch_Vector<T> &src)
{
    reserve(len + src.size());
    for (unsigned int i = 0; i < src.size(); ++i) {
      data[i+len] = src[i];
    }
    len = len + src.size();
    return *this;
}

// Add an element to the end of the vector.
template<class T>
void BPatch_Vector<T>::push_back(const T& x)
{
    reserve(len+1);
    data[len] = x;
    len++;
}

// Add an element to the end of the vector.
template<class T>
void BPatch_Vector<T>::push_front(const T& x)
{
    int i;

    reserve(len+1);
    for (i=len; i > 0; i--) data[i] = data[i-1];
    data[0] = x;
    len++;
}

// Resize vector (pare elements from the end)
// This is skeletal and doesn't do much but adjust indices
// If vector needs to grow, it will do so automatically on insert
template<class T>
void BPatch_Vector<T>::resize(int sz)
{
    if (sz >= len) return;
    if (sz < 0) return;
    len = sz;
}

// Clear vector 
template<class T>
void BPatch_Vector<T>::clear()
{
  if( data != NULL ) delete [] data;
  len = 0;
  reserved = 0;
  data = NULL;
}

// Reference the nth element of the vector.
template<class T>
T& BPatch_Vector<T>::operator[](int n) const
{
    assert(data != NULL && n >= 0 && n < len);
    return data[n];
}

// delete the nth element
template<class T>
void BPatch_Vector<T>::erase(int n) 
{
   assert(data != NULL && n >= 0 && n < len);
   for (int i=n; i<len-1; i++)
   {
      data[i] = data[i+1];
   }
   len--;
}

#endif /* BPATCH_DLL_IMPORT */
#endif /* USE_STL_VECTOR */
#endif /* _BPatch_Vector_h_ */
