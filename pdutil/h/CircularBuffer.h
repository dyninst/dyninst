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
 * CircularBuffer.h
 * 
 * This circular buffer wraps around after Size values, erasing 
 * previous values
 *
 * $Log: CircularBuffer.h,v $
 * Revision 1.3  2004/03/23 01:12:40  eli
 * Updated copyright string
 *
 * Revision 1.2  1996/08/16 21:29:56  tamches
 * updated copyright for release 1.1
 *
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
