/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "solo_mutatee_boilerplate.h"

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
	if (strcmp(buf, "aaaa")) {};
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

