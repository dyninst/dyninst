// array2d.h
// gives dynamic two-dimensional array capabilities to C++

// $Log: array2d.h,v $
// Revision 1.1  1994/09/29 19:47:16  tamches
// initial implementation
//

// commentary:
// #pragma flame begin
//             I cannot believe this isn't currently allowed in C++!
//             example: int **myarray = new int[10][20]; // forget it!
//             but      int myarray[10][20]; // okay (statically allocated)
// #pragma flame done

#ifndef _ARRAY2D_H_
#define _ARRAY2D_H_

// This is for g++:
#pragma interface

#include <stdlib.h> // exit()
#include <iostream.h>

void panic(char *);

template <class T>
class dynamic1dArray {
 private:
   T *data;
   // what the hell; add some bounds checking:
   int maxNumElems;
 public:
   dynamic1dArray(const int initmaxNumElems) {
      if (initmaxNumElems < 0) panic("negative numElems request");

      data = new T [maxNumElems=initmaxNumElems];
      if (NULL == data) panic("out of memory");
   }
  ~dynamic1dArray() {
      delete [] data;
   }
   T &operator[](const int index) {
      if (index < 0) panic("index < 0");
      if (index >= maxNumElems) panic("index >= maxNumElems");

      return data[index];
   }
   void reallocate(const int newmaxNumElems) {
      if (newmaxNumElems < 0) panic("negative numElems request");

      delete [] data;
      data = new T [newmaxNumElems];
      maxNumElems = newmaxNumElems;
   }
};

template <class T>
class dynamic2dArray {
 private:
   dynamic1dArray<T> *data;
   int maxNumElems;
 public:
   dynamic2dArray(const int initmaxDim1, const int initmaxDim2) {
      if (initmaxDim1 < 0) panic("negative numElems request");
      if (initmaxDim2 < 0) panic("negative numElems request");

      data = new dynamic1dArray<T> [maxNumElems = initmaxDim1] (initmaxDim2);
      if (NULL == data) panic ("out of memory");
   }
  ~dynamic2dArray() {
      delete [] data;
   }
   dynamic1dArray<T> &operator[] (const int indexDim1) {
      if (indexDim1 < 0) panic("index < 0");
      if (indexDim1 >= maxNumElems) panic("index >= maxNumElems");

      return data[indexDim1];
   }
   void reallocate(const int newmaxDim1, const int newmaxDim2) {
      if (newmaxDim1 < 0) panic("negative numElems request");
      if (newmaxDim2 < 0) panic("negative numElems request");

      delete [] data;
      maxNumElems = newmaxDim1;
      data = new dynamic1dArray<T> [maxNumElems] (newmaxDim2);
   }
};

#endif
