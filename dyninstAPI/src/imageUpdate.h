/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* -*- Mode: C; indent-tabs-mode: true -*- */
// Since the author of this file chose to use tabs instead of spaces
// for the indentation mode, the above line switches users into tabs
// mode with emacs when editing this file.

/* $Id: imageUpdate.h,v 1.9 2008/09/03 06:08:44 jaw Exp $ */


//ccw 28 oct 2001
// if we are sure BPATCH_SET_MUTATIONS_ACTIVE is always
//set we can use mutationRecord

#ifndef IMAGEUPDATE__

#define IMAGEUPDATE__



class imageUpdate{

        public:

        unsigned long address;
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
#if defined (cap_use_pdvector)
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
#else
static inline bool imageUpdateSort(const void * left, 
											 const void *right)
{
	const imageUpdate *leftSym = *(const imageUpdate* const *) left;
	const imageUpdate *rightSym = *(const imageUpdate* const *) right;
   
	if(leftSym->address < rightSym->address)
		return true;
   return false;
};
#endif
}


typedef struct dataUpdate__ {
	unsigned long address;
	unsigned int size;
	char* value;	
} dataUpdate;

#endif

