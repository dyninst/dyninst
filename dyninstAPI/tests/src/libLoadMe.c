#include <stdio.h>


extern int globalVariable5_1;

/*
   cribbed from RTcommon.c:

   _init table of methods:
   GCC: link with gcc -shared, and use __attribute__((constructor));
   AIX: ld with -binitfini:loadMe_init
   Solaris: ld with -z initarray=loadMe_init
   Linux: ld with -init loadMe_init
          gcc with -Wl,-init -Wl,...
          
*/

/* Convince GCC to run _init when the library is loaded */
//#ifdef __GNUC
void loadMe_init(void) __attribute__ ((constructor));
//#endif

/* UNIX-style initialization through _init */
int initCalledOnce = 0;
void loadMe_init() {

	globalVariable5_1 = 99;

}

