/* $Id: RTmutatedBinary.c,v 1.5 2003/01/31 18:55:43 chadd Exp $ */

/* this file contains the code to restore the necessary
   data for a mutated binary 
 */


#include <unistd.h>

extern int isMutatedExec;
char *buffer;

/* checkMutatedFile() is defined in RTmutatedBinary_<fileformat>.c */

extern int checkMutatedFile();

/* with solaris, the mutatee has a jump from
 * main() to a trampoline that calls DYNINSTinit() the
 * trampoline resides in the area that was previously
 * the heap, this trampoline is loaded as part of the
 * data segment
 * UPDATE: now the heap tramps are not loaded by the loader
 * but rather this file so _init is necessary
 * UPDATE: gcc handles the _init fine, but
 * cc chokes on it.  There seems to be no compiler
 * independent way to have my code called correctly
 * at load time so i have defined _NATIVESO_ in
 * the sparc Makefile for cc only.  The #pragma
 * forces the my_init function to be called 
 * correctly upon load of the library.
 * Building with gcc is the same as before. 
 * THIS NEEDS TO BE FIXED 
 * 
 * with linux the trampolines are ALL in the big
 * array at the top of this file and so are not loaded
 * by the loader as part of the data segment. this
 * needs to be called to map in everything before
 * main() jumps to the big array
 */ 

#if defined(_NATIVESO_)

#pragma init (my_init) 
void my_init(){
#else 
void _init(){

#endif

/* this buffer is allocated to clear
   the first page on the heap. This is necessary
   because loading the heap tramps uses mmap, which
   is going to eat the heap if the heap begins on 
   the same page the heap tramps end on (almost certain)
*/
	buffer = (char*) malloc(getpagesize());
	isMutatedExec =checkMutatedFile();
	free(buffer);
}
