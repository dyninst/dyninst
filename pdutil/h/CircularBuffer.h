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
 * CircularBuffer.h
 * 
 * This circular buffer wraps around after Size values, erasing 
 * previous values
 *
 * $Log: CircularBuffer.h,v $
 * Revision 1.1  1996/02/22 17:47:41  karavan
 * initial version.
 *
 */

#if !defined(_CircularBuffer_h)
#define _CircularBuffer_h

#if defined(external_templates)
#pragma interface
#endif

template <class Type, int Size>
class circularBuffer {
public:
  circularBuffer ();
  ~circularBuffer();
  // add a new value to the buffer, trashing oldest value if no more 
  // empty slots available
  bool add (Type *t);
  // careful!! this one assumes buffer is not empty!
  Type remove (); 
  // look but don't touch...
  const Type *peek ();
  // debug printing, least to most recently added elements
  void print();
  int getSize () {return Size;}
  bool isEmpty() {return (count == 0);}
private:
  Type bufData[Size];
  int first;
  int last;
  int count;    // how many slots currently filled?
};

#endif   /* !defined(_CircularBuffer_h) */
