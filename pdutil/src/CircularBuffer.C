/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/* 
 * CircularBuffer.C
 * member functions for the circularBuffer class
 *
 * $Log: CircularBuffer.C,v $
 * Revision 1.1  1996/02/22 17:47:26  karavan
 * Initial version.
 *
 */

#include <iostream.h>
#include "util/h/CircularBuffer.h"

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

