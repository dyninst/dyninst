/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/* 
 * CircularBuffer.C
 * member functions for the circularBuffer class
 *
 * $Log: CircularBuffer.C,v $
 * Revision 1.5  2004/03/23 01:12:41  eli
 * Updated copyright string
 *
 * Revision 1.4  2003/07/18 15:45:13  schendel
 * fix obsolete header file warnings by updating to new C++ header files;
 *
 * Revision 1.3  2000/07/28 17:22:35  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.2  1996/08/16 21:31:44  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.1  1996/02/22 17:47:26  karavan
 * Initial version.
 *
 */

#include <iostream>
#include "pdutil/h/CircularBuffer.h"

template <class Type, int Size>
circularBuffer<Type, Size>::circularBuffer ()  
{
  first = 0;
  last = 0;
  count = 0;
}

template <class Type, int Size>
circularBuffer<Type, Size>::~circularBuffer ()
{
  ;
}

template <class Type, int Size>
void circularBuffer<Type, Size>::print()
{
  if (isEmpty()) {
    cout << "\tCircular Buffer is empty" << endl;
    return;
  }
  cout << "\tCircular Buffer Contents:" << endl;
  int index = first;
  int total = 0;
  while ((total < count) && (index < Size)) {
    cout << bufData[index] ; 
    index++;
    total++;
  }
  index = 0;
  while (total < count) {
    cout << bufData[index];
    index++;
    total++;
  }
  cout << endl;
}

template <class Type, int Size>
bool circularBuffer<Type, Size>::add (Type *t)
{
  bufData[last] = *t;
  if (last == (Size - 1))
    last = 0;
  else
    last++;
  if (count == Size) {
    // overflow
    if (first == (Size -1)) first = 0; else first++;
    return true;
  } else {
    count++;
    return false;
  }
}

// assumes nonempty
template <class Type, int Size>
Type circularBuffer<Type, Size>::remove ()
{
  int retIndex = first;
  if (first == (Size - 1))
    first = 0;
  else
    first++;
  count--;
  return bufData[retIndex];
}

template <class Type, int Size>
const Type *circularBuffer<Type, Size>::peek ()
{
  if (isEmpty())
    return (Type*)NULL;
  return &(bufData[first]);
}

