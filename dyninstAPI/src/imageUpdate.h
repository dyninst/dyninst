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

/* -*- Mode: C; indent-tabs-mode: true -*- */
// Since the author of this file chose to use tabs instead of spaces
// for the indentation mode, the above line switches users into tabs
// mode with emacs when editing this file.

/* $Id: imageUpdate.h,v 1.7 2004/03/23 19:10:57 eli Exp $ */


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

