/* -*- Mode: C; indent-tabs-mode: true -*- */
// Since the author of this file chose to use tabs instead of spaces
// for the indentation mode, the above line switches users into tabs
// mode with emacs when editing this file.

/* $Id: imageUpdate.h,v 1.5 2003/01/31 18:55:42 chadd Exp $ */

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
        unsigned int startPage, stopPage; //ccw 13 july 2002 : fix for AIX 

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

	const imageUpdate& operator=(const imageUpdate &obj) { 
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
};

extern "C" {
static inline int imageUpdateSort(const void * left, 
											 const void *right)
{
	const imageUpdate *leftSym = *(const imageUpdate* const *) left;
	const imageUpdate *rightSym = *(const imageUpdate* const *) right;
   
	if(leftSym->address > rightSym->address)
		return 1;
	else 
		return -1;
};
}


typedef struct dataUpdate__ {
	unsigned int address;
	unsigned int size;
	char* value;	
} dataUpdate;

#endif

#endif
