/* $Id: RTmutatedBinary.c,v 1.6 2003/02/04 15:19:04 bernat Exp $ */

/* this file contains the code to restore the necessary
   data for a mutated binary 
 */


#include <unistd.h>

extern int isMutatedExec;
char *buffer;

/* checkMutatedFile() is defined in RTmutatedBinary_<fileformat>.c */

extern int checkMutatedFile();

/*
 * This function sets up pre-initialization
 * data structures for SaveTheWorld. Ensure
 * it is called _before_ DYNINSTinit
 */ 
void RTmutatedBinary_init(){

/* this buffer is allocated to clear
   the first page on the heap. This is necessary
   because loading the heap tramps uses mmap, which
   is going to eat the heap if the heap begins on 
   the same page the heap tramps end on (almost certain)
*/
    /* Call-once protection */
    static int init = 0;
    
    if (!init) {
        buffer = (char*) malloc(getpagesize());
        isMutatedExec =checkMutatedFile();
        free(buffer);
        init++;
    }
    
}

