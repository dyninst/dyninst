// simpSeq.h
// Ariel Tamches
// simple sequence-of-anything class.
// Fixed maximum sequence size (currently twenty) for maximum speed
// Very lightweight all-around; no free store operations _ever_

/* $Log: simpSeq.h,v $
/* Revision 1.1  1995/07/17 04:59:00  tamches
/* First version of the new where axis
/*
 */

#ifndef _SIMPSEQ_H_
#define _SIMPSEQ_H_

template <class T>
class simpSeq {
 private:
   T data[20];
   int numitems; // indexes in use are 0 thru numitems-1

 public:
   simpSeq();
   simpSeq(const simpSeq &src) {
      this->numitems = src.numitems;
      for (int item=0; item < numitems; item++)
         this->data[item] = src.data[item]; // T::operator=(const T &)
   }
   simpSeq(const T &item) {
      this->data[0] = item;
      this->numitems = 1;
   }
  ~simpSeq();

   int getSize() const;
   void rigSize(const int newsize) {numitems = newsize;}

   T &getItem(const int index);
   T &operator[](const int index){return getItem(index);}
   const T &operator[] (const int index) const {return getConstItem(index);}
   T &getLastItem();

   const T &getConstItem(const int index) const;
   const T &getConstLastItem() const;

   const T *getEntireSeqQuick() const;

   void append(const T &newItem);
   void replaceItem(const int index, const T &newItem);
};

#endif
