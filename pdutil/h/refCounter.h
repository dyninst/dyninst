// refCounter.h
// Ariel Tamches

#ifndef _REF_COUNTER_H_
#define _REF_COUNTER_H_

#pragma interface

#include <assert.h>

template <class T>
class refCounter {
 private:
   class actualData {
    private:
      mutable unsigned refCount;
      T data;
    public:
      actualData(const T &src) : data(src) {refCount=0;}
     ~actualData() {}
      void reference() const {refCount++;}
      bool dereference() const {
	 assert(refCount > 0);
	 return (--refCount == 0);
      }
      T &getData() {return data;}
      const T &getData() const {return data;}
   };
   actualData *theData;
      // allocated with new, but not necessarily by us.  _Never_ NULL.

 private:
   void reference() const {
      assert(theData);
      theData->reference();
   }
   void dereference() const {
      assert(theData);
      if (theData->dereference())
         delete theData;
   }
   refCounter(); // explicitly disallowed

 public:
   refCounter(const T &src) {
      // examples:
      // T y; (y is initialized somehow...)
      // refCounter<T> x = y; or
      // refCounter<T> x(y);
      theData = new actualData(src);
      assert(theData);
      reference();
   }
   refCounter(const refCounter &src) {
      // This constructor is what this whole class revolves around.  It's fast.
      // examples:
      // refCounter<T> y; (y is initialized somehow...)
      // refCounter<T> x = y; or
      // refCounter<T> x(y);
      src.reference(); // just bumps up a ref count --> fast
      theData = src.theData;  // just a ptr assignment --> fast
   }
  ~refCounter() {
      dereference();
   }
   refCounter &operator=(const refCounter &src) {
      if (this == &src)
         return *this; // protect against x=x

      if (theData) {
        // dereference what we were using...
        dereference();
      }

      // ...and attach to the new stuff efficiently
      theData = src.theData; // just a ptr assignment --> fast
      reference();           // just bumps a ref cnt  --> fast
      return *this;
   }
   refCounter &operator=(const T &src) {
      dereference();
      theData = new actualData(src);
      reference();
      return *this;
   }
   T &getData() {
      assert(theData);
      return theData->getData();
   }
   const T &getData() const {
      assert(theData);
      return theData->getData();
   }
};

#endif
