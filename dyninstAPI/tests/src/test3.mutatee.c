
/* Test application (Mutatee) */

/* $Id: test3.mutatee.c,v 1.2 1999/06/18 23:28:59 wylie Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include <unistd.h>
#endif
#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif

#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
#include <dlfcn.h> // For replaceFunction test
#endif

// Mutatee for multi-process tests.
//

// 
// Test #1 - just run in a busy wait loop and then exit.
//
void test1()
{
     int i;

     for (i=0; i < 2000000; i++);
     exit(0);
}

int test2ret = 0xdeadbeef;

volatile int dummy = 1;

int func2_1()
{
     return (dummy * 2);
}

//
// Test #2 - call a function which should be instrumented to set the 
//     global variable test2ret to a value (by the mutator).
//
void test2()
{
     FILE *fp;
     char filename[80];

     func2_1();

#ifndef i386_unknown_nt4_0
     sprintf(filename, "test3.out.%d", getpid());
#else
     sprintf(filename, "test3.out.%d", _getpid());
#endif
     fp = fopen(filename, "w");
     assert(fp);
     fprintf(fp, "%d\n", test2ret);
     fclose(fp);
}

// 
// Test #4 - terminate with an abort
//
void test4()
{
     abort();
}

int main(int argc, char *argv[])
{
#ifndef i386_unknown_nt4_0
    int pfd;
#endif
 
    int testNum;
    if ((argc!=2) || (testNum=atoi(argv[1]))==0) {
	printf("usage: %s <num>\n", argv[0]);
	exit(-1);
    }

    switch (testNum) {
	case 1:
		test1();
		break;
	
	case 2:
		test2();
		break;

	case 4:
		test4();
		break;

	default:
		printf("invalid test number %d in mutatee\n", testNum);
		break;
    }
    return(0);
}
