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

// simpSeq.h
// Ariel Tamches
// simple sequence-of-anything class.
// Fixed maximum sequence size (currently twenty) for maximum speed
// Very lightweight all-around; no free store operations _ever_

/* $Log: simpSeq.h,v $
/* Revision 1.5  1996/08/16 21:07:24  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.4  1995/11/06 02:33:37  tamches
 * fixed operator= to return *this
 *
 * Revision 1.3  1995/10/17 22:09:48  tamches
 * added operator==
 *
 * Revision 1.2  1995/09/20 01:19:15  tamches
 * int --> unsigned in a lot of places
 *
 * Revision 1.1  1995/07/17  04:59:00  tamches
 * First version of the new where axis
 *
 */

#ifndef _SIMPSEQ_H_
#define _SIMPSEQ_H_

template <class T>
class simpSeq {
 private:
   T data[20];
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
