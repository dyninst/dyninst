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

/* Test application (Mutatee) */

/* $Id: test4b.mutatee.c,v 1.8 2006/02/08 23:41:34 bernat Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(i386_unknown_nt4_0) && !defined(__GNUC__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

int debugPrint = 0;

/* control debug printf statements */
void dprintf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
     
	if( debugPrint ) {
		vfprintf(stderr, fmt, args);
		}
	va_end(args);
	fflush(stderr);
	}
	
#define TRUE    1
#define FALSE   0

int runAllTests = TRUE;
#define MAX_TEST 5
int runTest[MAX_TEST+1];

int globalVariable3_1;

void func3_2() 
{
    dprintf("in func3_2\n");
    globalVariable3_1 = 3000002;
}

void func3_1()
{
    dprintf("in func3_1\n");
}

int globalVariable4_1;

void func4_4()
{
    dprintf("in test4b func4_4\n");

    globalVariable4_1 = 4000003;
}

void func4_2()
{
    dprintf("in test4b func4_2\n");

    /* a call to func4_4 gets hooked onto the exit of this */
}

void func4_1()
{
#ifndef i386_unknown_nt4_0
    dprintf("in test4b func4_1\n");

    func4_2();

    exit(getpid());
#endif
}

int main(int argc, char *argv[])
{                                       
    int i, j;

    dprintf("test4b.mutatee started...\n");

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-run")) {
            runAllTests = FALSE;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = FALSE;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = TRUE;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    /* end of test list */
                    break;
                }
            }
            i=j-1;
        } else {
            fprintf(stderr, "Usage: %s [-verbose] -run <num> ..\n", argv[0]);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();

    dprintf("Mutatee %s terminating.\n", argv[0]);
    return 0;
}
