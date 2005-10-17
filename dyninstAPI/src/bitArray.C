#include "dyninstAPI/src/inst.h"


#ifndef BITARRAY_HDR
#define BITARRAY_HDR


void bitArray::dispose()
{
  free(data);
  data = NULL;
}

bitArray::bitArray()
{
  data = NULL;
}


void bitArray::bitarray_init(int size,bitArray* bitarray)
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
void bitArray::bitarray_set(int location,bitArray* bitarray)
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


#endif
