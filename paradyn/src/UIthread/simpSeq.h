// simpSeq.h
// Ariel Tamches
// simple sequence-of-anything class.
// Fixed maximum sequence size (currently twenty) for maximum speed
// Very lightweight all-around; no free store operations _ever_

/* $Log: simpSeq.h,v $
/* Revision 1.3  1995/10/17 22:09:48  tamches
/* added operator==
/*
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
