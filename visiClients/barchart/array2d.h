// array2d.h
// gives dynamic two-dimensional array capabilities to C++

/* $Log: array2d.h,v $
/* Revision 1.5  1995/08/06 22:09:16  tamches
/* char * --> const char *
/*
 * Revision 1.4  1995/06/23  03:14:24  tamches
 * changed to -fno-implicit-templates
 *
 * Revision 1.3  1994/11/06  10:18:41  tamches
 * more descriptive reporting on assertion failures
 *
 * Revision 1.2  1994/09/29  20:05:29  tamches
 * minor cvs fixes
 *
 * Revision 1.1  1994/09/29  19:47:16  tamches
 * initial implementation
 *
*/

// commentary:
// #pragma flame begin
//             I cannot believe this isn't currently allowed in C++!
//             example: int **myarray = new int[10][20]; // forget it!
//             but      int myarray[10][20]; // okay (statically allocated)
// #pragma flame done

#ifndef _ARRAY2D_H_
#define _ARRAY2D_H_

// This is for g++:
#ifdef external_templates
#pragma interface
#endif

#include <stdlib.h> // exit()
#include <iostream.h>

void panic(const char *);

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
      if (index >= maxNumElems) {
         cerr << "operator[]: index " << index << " too high (valid range=0," << maxNumElems-1 << ")";
         panic("");
      }

      return data[index];
   }
   void reallocate(const int newmaxNumElems) {
      if (newmaxNumElems < 0) panic("negative numElems request");

      delete [] data;
      data = new T [maxNumElems=newmaxNumElems];
      if (data == NULL) panic("out of memory");
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
      if (indexDim1 >= maxNumElems) {
         cerr << "operator[]: index " << indexDim1 << " too high (valid range=0," << maxNumElems-1 << ")" << endl;
         panic("");
      }

      return data[indexDim1];
   }
   void reallocate(const int newmaxDim1, const int newmaxDim2) {
      if (newmaxDim1 < 0) panic("negative numElems request");
      if (newmaxDim2 < 0) panic("negative numElems request");

      delete [] data;
      maxNumElems = newmaxDim1;
      data = new dynamic1dArray<T> [maxNumElems] (newmaxDim2);
      if (data == NULL) panic("out of memory");
   }
};

#endif
