/* $Id: imageUpdate.h,v 1.2 2002/03/12 18:40:02 jaw Exp $ */

#if defined(BPATCH_LIBRARY)

//ccw 28 oct 2001
// if we are sure BPATCH_SET_MUTATIONS_ACTIVE is always
//set we can use mutationRecord

#ifndef IMAGEUPDATE__

#define IMAGEUPDATE__



class imageUpdate{

        public:

        unsigned int address;
        unsigned int size;
        int startPage, stopPage;


	imageUpdate(){
		address =0;
		size = 0;
	};	

	imageUpdate(const imageUpdate& obj){
		address = obj.address;
		size = obj.size;
		startPage = obj.startPage;
		stopPage = obj.stopPage;
	};

	const imageUpdate& operator=(const imageUpdate &obj){ 
                address = obj.address;
                size = obj.size;
                startPage = obj.startPage;
                stopPage = obj.stopPage;
		return *this;
	};

	bool operator==(const imageUpdate& right){
		return address == right.address; 
	}; 

	bool operator!=(const imageUpdate& right){
		return address != right.address;
	};

	bool operator<(const imageUpdate &right) const{
		return address< right.address;

	};
	bool operator>(const imageUpdate &right) const{
		return address> right.address;
	};

	static int imageUpdateSort(const void * left, const void *right)
	  {
	    const imageUpdate *leftSym = *(const imageUpdate* const *) left;
	    const imageUpdate *rightSym = *(const imageUpdate* const *) right;

	    if(leftSym->address > rightSym->address)
	      return 1;
	    else 
	      return -1;
	  };
};

#ifdef USE_STL_VECTOR
#include <vector>
#include <algorithm>
#include "imageUpdate.h"
struct imageUpdateOrderingRelation : public binary_function<imageUpdate *, imageUpdate *, bool> {
  bool operator()(imageUpdate *left, imageUpdate *right) {return left->address < right->address;}
};
#endif




typedef struct dataUpdate__{


	unsigned int address;
	unsigned int size;
	char* value;	

} dataUpdate;

#endif

#endif
