/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if !defined(os_windows_test)
#if defined (__cplusplus)
extern "C" int relocation_test_variable1;
extern "C" int relocation_test_variable2;
extern "C" int relocation_test_function1(int);
extern "C" int relocation_test_function2(int);
#else
extern int relocation_test_variable1;
extern int relocation_test_variable2;
extern int relocation_test_function1(int);
extern int relocation_test_function2(int);
#endif
#endif
void func_relocations_mutatee()
{
#if !defined(os_windows_test)
	int r1, r2;
	char buf[10];
   FILE *f;
	struct stat statbuf;

	/*  some junk system calls to trigger entries in th relocation table */
	printf("a");
	fprintf(stderr,"a");
	sprintf(buf,"a");
	snprintf(buf,2, "%s", "aaaaaa");
	memcpy(buf, "aaaa", 4*sizeof(char));
	strcmp(buf, "aaaa");
	memset(buf, 0, 4*sizeof(char));
   f = fopen("/blaarch", "rw");
	fwrite(buf, sizeof(char), 4, f);
	fread(buf, sizeof(char), 4, f);
	fclose(f);
	stat("/blaarch", &statbuf);
	lstat("/blaarch", &statbuf);
	fstat(7, &statbuf);

	printf("%d, %d\n", relocation_test_variable1, relocation_test_variable2);
	r1 = relocation_test_function1(1);
	r2 = relocation_test_function2(2);
	printf("%d, %d\n", r1, r2);

#endif
}


int test_relocations_mutatee() 
{
	fprintf(stderr, "welcome to test_relocations_mutatee\n");

   /*If mutatee should run, things go here.*/
   return 0;
}

