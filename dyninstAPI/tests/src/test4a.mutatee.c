/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

/* Test application (Mutatee) */

/* $Id: test4a.mutatee.c,v 1.9 2004/04/19 20:12:59 chadd Exp $ */

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
#include <process.h>
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

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

#define TRUE    1
#define FALSE   0

int runAllTests = TRUE;
#define MAX_TEST 5
int runTest[MAX_TEST+1];

unsigned int globalVariable1_1 = 0xdeadbeef;
unsigned int globalVariable2_1 = 0xdeadbeef;

void func1_1()
{
    globalVariable1_1 = 1000001;
    exit((int) getpid());
}

/* should be called by parent */
void func2_3()
{
    dprintf("pid %d in func2_3\n", (int) getpid());
    globalVariable2_1 = 2000002;
}

/* should be called by child */
void func2_4()
{
    dprintf("pid %d in func2_4\n", (int) getpid());
    globalVariable2_1 = 2000003;
}

void func2_2() {
    /* if we get this value at exit, no function was called */
    globalVariable2_1 = 2000001;
}

void func2_1()
{
#ifndef i386_unknown_nt4_0
    int pid;

    pid = fork();
    if (pid >= 0) {
        /* both parent and child exit here */
        func2_2();
        dprintf("at exit of %d, globalVariable2_1 = %d\n", (int) getpid(),
                globalVariable2_1);

#if defined(rs6000_ibm_aix4_1)
	if( pid == 0){
	        /* On AIX the child dies when the parent exits, so wait */
		/* apparently the parent needs to wake up occasionally to keep Dyninst happy */
		dprintf("%d SLEEPING\n",getpid());
        	sleep(5);
		dprintf("%d SLEEP MORE\n",getpid());
		sleep(1);
		dprintf("%d SLEEP MORE\n",getpid());
		sleep(5);
		dprintf("%d DONE SLEEPING\n",getpid());
	}
#endif


        exit(getpid());
    } else if (pid < 0) {
        /* error case */
        exit(pid);
    }
#endif
}

unsigned int globalVariable3_1 = 0xdeadbeef;

void func3_1(int argc, char *argv[])
{
    int i;
    char *ch;
    char **newArgv;

    newArgv = (char **) calloc(sizeof(char *), argc +1);
    for (i = 0; i < argc; i++) newArgv[i] = argv[i];

    /* replace 4a in copy of myName by 4b */
    newArgv[0] = strdup(argv[0]);
    for (ch=newArgv[0]; *ch; ch++) {
	if (!strncmp(ch, "4a", 2)) *(ch+1) = 'b';
    }

    globalVariable3_1 = 3000001;
    dprintf("Starting \"%s\"\n", newArgv[0]);
    execvp(newArgv[0], newArgv);
    perror("execvp");
}

unsigned int globalVariable4_1 = 0xdeadbeef;

void func4_3()
{
    dprintf("in func4_3\n");
    globalVariable4_1 = 4000002;
}

void func4_2()
{
    dprintf("in func4_2\n");
    /* call to func4_3 should be inserted here */
}

void func4_1(int argc, char *argv[])
{
#ifndef i386_unknown_nt4_0
    int i;
    int pid;
    char *ch;
    char **newArgv;

    pid = fork();
    if (pid == 0) {
        newArgv = (char**) calloc(sizeof(char *), argc +1);
        for (i = 0; i < argc; i++) newArgv[i] = argv[i];
        
        /* replace 4a in copy of myName by 4b */
        newArgv[0] = strdup(argv[0]);
        for (ch=newArgv[0]; *ch; ch++) {
            if (!strncmp(ch, "4a", 2)) *(ch+1) = 'b';
        }
        
        globalVariable3_1 = 3000001;
        dprintf("Starting \"%s\"\n", newArgv[0]);
        execvp(newArgv[0], newArgv);
        perror("execvp");
    } else {
        func4_2();
#if defined(rs6000_ibm_aix4_1)
        /* On AIX the child dies when the parent exits, so wait */
	/* and the parent needs to wake up occasionally to keep dyninst happy*/
	dprintf("%d SLEEPING\n",getpid());
        sleep(10);
	dprintf("%d SLEEP MORE\n",getpid());
	sleep(2);
	dprintf("%d SLEEP MORE\n",getpid());
	sleep(5);
	dprintf("%d DONE SLEEPING\n",getpid());

#endif
        exit(getpid());
    }
#endif
}

int main(int argc, char *argv[])
{                                       
    int i, j;

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

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1(argc, argv);
    if (runTest[4]) func4_1(argc, argv);

    dprintf("Mutatee %s terminating.\n", argv[0]);
    return 0;
}
