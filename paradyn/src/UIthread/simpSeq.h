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

// simpSeq.h
// Ariel Tamches
// simple sequence-of-anything class.
// Fixed maximum sequence size (currently twenty) for maximum speed
// Very lightweight all-around; no free store operations _ever_

/* $Id: simpSeq.h,v 1.8 2004/03/23 01:12:30 eli Exp $ */

#ifndef _SIMPSEQ_H_
#define _SIMPSEQ_H_

template <class T>
class simpSeq {
 private:
   T data[50];
   unsigned numitems; // indexes in use are 0 thru numitems-1

   T &getItem(unsigned index);
   const T &getItem(unsigned index) const;

 public:
   simpSeq() {numitems=0;}
   simpSeq(const simpSeq &src) {
      this->numitems = src.numitems;
      for (unsigned item=0; item < numitems; item++)
         this->data[item] = src.data[item]; // T::operator=(const T &)
   }
   simpSeq(const T &item) {
      this->data[0] = item;
      this->numitems = 1;
   }
  ~simpSeq() {}

   simpSeq<T> &operator=(const simpSeq<T> &src) {
      numitems = src.numitems;
      for (unsigned item=0; item < numitems; item++)
         data[item] = src.data[item]; // T::operator=(const T &)
      return *this;
   }

   bool operator==(const simpSeq<T> &other) const;

   unsigned getSize() const {return numitems;}
   void rigSize(unsigned newsize) {numitems = newsize;}

   T &operator[](unsigned index){return getItem(index);}
   const T &operator[] (unsigned index) const {return getItem(index);}

   T &getLastItem();
   const T &getLastItem() const;

   const T *getEntireSeqQuick() const;

   void append(const T &newItem);
   void replaceItem(unsigned index, const T &newItem);
};

#endif
