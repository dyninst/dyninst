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
#include "dataflowAPI/h/bitArray.h"



template void bitArray::dispose()
{
  free(data);
  data = NULL;
}

template bitArray::bitArray()
{
  data = NULL;
}


template void bitArray::bitarray_init(int size,bitArray* bitarray)
{
	int bytesize;

	bitarray->size = size;
	if(!(size % 8))
		bytesize = size / 8;
	else
		bytesize = (size / 8) + 1;

	bitarray->data = (char*) malloc (sizeof(char)*bytesize);

	memset(bitarray->data,'\0',bytesize);
}
template void bitArray::bitarray_set(int location,bitArray* bitarray)
{
  int pass,pos;
  char* ptr;
  unsigned char mask = 0x80;
  
  if(location > bitarray->size){
    fprintf(stderr,"trying to set a bit more than the size\n");
    return;
  }
  pass = location / 8;
	pos = location % 8;

	ptr = bitarray->data + pass;
	mask = mask >> pos;

	*ptr = (*ptr) | mask;
}
void bitArray::bitarray_unset(int location,bitArray* bitarray)
{
        int pass,pos;
        char* ptr;
        unsigned char mask = 0x80;
        
        if(location > bitarray->size){
                fprintf(stderr,"trying to unset a bit more than the size\n");
                return;
        }
        pass = location / 8;
        pos = location % 8;
 
        ptr = bitarray->data + pass;
        mask = mask >> pos;
 
        *ptr = (*ptr) & (~mask);
}
int bitArray::bitarray_check(int location,bitArray* bitarray)
{
        int pass,pos;
        char* ptr;
        char unsigned mask = 0x80;
        
	if(location < 0)
		return FALSE;

        if(location > bitarray->size){
                fprintf(stderr,"trying to unset a bit more than the size\n");
                return FALSE;
        }
        pass = location / 8;
        pos = location % 8;

	ptr = bitarray->data + pass;
	mask = mask >> pos;

	if((*ptr) & mask)
		return TRUE;
	return FALSE;
}

void bitArray::bitarray_and(bitArray* s1,bitArray* s2,bitArray* r)
{
	int bytesize,i;

	if(s1->size != s2->size){
		fprintf(stderr,"size do not match to and \n");
		return ;
	}
	if(s1->size % 8)
		bytesize = (s1->size / 8)+1;
	else
		bytesize = s1->size / 8;
	r->size = s1->size;
	for(i=0;i<bytesize;i++)
		*((r->data)+i) = *((s1->data)+i) & *((s2->data)+i);
}
void bitArray::bitarray_or(bitArray* s1,bitArray* s2,bitArray* r)
{
	int bytesize,i;

	if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;
        r->size = s1->size;
        for(i=0;i<bytesize;i++)
                *((r->data)+i) = *((s1->data)+i) | *((s2->data)+i);
}
void bitArray::bitarray_copy(bitArray* s1,bitArray* s2)
{
	int bytesize,i;

        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;

	for(i=0;i<bytesize;i++)
		*((s1->data)+i) = *((s2->data)+i);
}
void bitArray::bitarray_comp(bitArray* s1,bitArray* s2)
{
        int bytesize,i;

        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;

        for(i=0;i<bytesize;i++)
                *((s1->data)+i) = ~(*((s2->data)+i));
}
int bitArray::bitarray_same(bitArray* s1,bitArray* s2)
{
	int bytesize,i;

        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return FALSE;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;
	for(i=0;i<bytesize;i++)
		if(*((s1->data)+i) ^ *((s2->data)+i))
			return FALSE;
	return TRUE;
}
	
void bitArray::bitarray_diff(bitArray* s1,bitArray* s2,bitArray* r)
{
        int bytesize,i;
 
        if(s1->size != s2->size){
                fprintf(stderr,"size do not match to and \n");
                return ;
        }
        if(s1->size % 8)
                bytesize = (s1->size / 8)+1;
        else
                bytesize = s1->size / 8;
        r->size = s1->size;
        for(i=0;i<bytesize;i++)
		*((r->data)+i) = *((s1->data)+i) & (~(*((s2->data)+i)));
}


