/* $Id: imageUpdate.h,v 1.3 2002/12/20 07:49:56 jaw Exp $ */

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


typedef struct dataUpdate__{


	unsigned int address;
	unsigned int size;
	char* value;	

} dataUpdate;

#endif

#endif
