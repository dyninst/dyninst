/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/process.C,v 1.17 1994/08/17 18:17:43 markc Exp $";
#endif

/*
 * process.C - Code to control a process.
 *
 * $Log: process.C,v $
 * Revision 1.17  1994/08/17 18:17:43  markc
 * Changed execv to execvp.
 *
 * Revision 1.16  1994/07/26  20:01:41  hollings
 * fixed heap allocation to use hash tables.
 *
 * Revision 1.15  1994/07/20  23:23:39  hollings
 * added insn generated metric.
 *
 * Revision 1.14  1994/07/14  23:29:03  hollings
 * Corrected file mask on io redirection.
 *
 * Revision 1.13  1994/06/29  02:52:47  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.12  1994/06/27  21:28:18  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.11  1994/06/27  18:57:07  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.10  1994/06/22  03:46:32  markc
 * Removed compiler warnings.
 *
 * Revision 1.9  1994/06/22  01:43:18  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.8  1994/05/31  17:59:05  markc
 * Closed iopipe fd that had been dup'd if the fd was greater than 2.
 *
 * Revision 1.7  1994/05/18  00:52:31  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.6  1994/05/16  22:31:53  hollings
 * added way to request unique resource name.
 *
 * Revision 1.5  1994/03/31  02:00:35  markc
 * Changed to fork for paradyndPVM since client calls pvmendtask which writes
 * to the address space.
 *
 * Revision 1.4  1994/03/22  21:03:15  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.3  1994/03/20  01:53:11  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.2  1994/02/05  23:09:56  hollings
 * Added extern for sys_errlist[] (g++ version 2.5.7).
 *
 * Revision 1.1  1994/01/27  20:31:38  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.8  1993/10/04  21:38:41  hollings
 * round inferrior mallocs to cache line size.
 *
 * Revision 1.7  1993/08/23  23:15:25  hollings
 * added code to third parameter to findInternalAddress calls.
 *
 * Revision 1.6  1993/08/11  01:47:09  hollings
 * added copyInferrior heap for UNIX fork.
 *
 * Revision 1.5  1993/07/13  18:29:38  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/28  23:13:18  hollings
 * fixed process stopping.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <fcntl.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninst.h"
#include "symtab.h"
#include "process.h"
#include "util.h"
#include "inst.h"

List<process*> processList;

extern char *sys_errlist[];

/* root of process resource list */
resource processResource;
resource machineResource;

// <sts/ptrace.h> should really define this. 
extern "C" {
int gethostname(char*, int);
int socketpair(int, int, int, int sv[2]);
int vfork();
int ptrace(enum ptracereq request, 
		      int pid, 
		      int *addr, 
		      int data, 
		      char *addr2);

#ifdef PARADYND_PVM
int pvmputenv (char *);
int pvmendtask();
#endif
}

void initInferiorHeap(process *proc, Boolean globalHeap)
{
    heapItem *np;

    assert(proc->symbols);

    np = (heapItem*) xcalloc(sizeof(heapItem), 1);
    if (globalHeap) {
	np->addr = (int)
	    findInternalAddress(proc->symbols, GLOBAL_HEAP_BASE, True);
    } else {
	np->addr = (int)
	    findInternalAddress(proc->symbols, INFERRIOR_HEAP_BASE, True);
    }
    np->length = SYN_INST_BUF_SIZE;
    np->status = HEAPfree;

    proc->heapFree.add(np);
}

void copyInferriorHeap(process *from, process *to)
{
    abort();

#ifdef notdef
    heapItem *curr;
    heapItem *newEntry;

    // not done jkh 7/22/94

    assert(from->symbols);
    assert(to->symbols);

    to->heapActive = NULL;
    /* copy individual elements */
    for (curr=from->heap; curr; curr=curr->next) {
	newEntry = (heapItem*) xcalloc(sizeof(heapItem), 1);
	*newEntry = *curr;

	/* setup next pointers */
	newEntry->next = to->heap;
	to->heap = newEntry;
    }
    to->heap->status = HEAPfree;
#endif
}

int inferriorMalloc(process *proc, int size)
{
    heapItem *np;
    heapItem *newEntry;
    List <heapItem *> curr;

    /* round to next cache line size */
    /* 32 bytes on a SPARC */
    size = (size + 0x1f) & ~0x1f; 

    for (curr = proc->heapFree; np = *curr; curr++) {
	if (np->length >= size) break;
    }

    if (!np) {
	logLine("Inferrior heap overflow\n");
	sprintf(errorLine, "%d bytes freed\n", proc->freed);
	logLine(errorLine);
	abort();
    }

    if (np->length != size) {
	// divide it up.
	newEntry = (heapItem *) xcalloc(sizeof(heapItem), 1);
	newEntry->length = np->length - size;
	newEntry->addr = np->addr + size;

	proc->heapFree.add(newEntry);

	/* now split curr */
	np->length = size;
    }

    // mark it used
    np->status = HEAPallocated;

    // remove from active list.
    proc->heapFree.remove(np);

    // onto in use list
    proc->heapActive.add(np, (void *) np->addr);
    return(np->addr);
}

void inferriorFree(process *proc, int pointer)
{
    heapItem *np;

    np = proc->heapActive.find((void*) pointer);
    if (!np) abort();
    proc->freed += np->length;
#ifdef notdef
    heapItem *curr;

    /* free is currently disabled because we can't handle the case of an
     *  inst function being deleted while active.  Not freeing the memory means
     *  it stil contains the tramp and will get itself out safely.
     */

    curr = proc->heapActive.find((heapItem *) pointer);

    if (!curr) {
	logLine("unable to find heap entry %x\n", pointer);
	abort();
    }

    if (curr->status != HEAPallocated) {
	logLine("attempt to free already free heap entry %x\n", pointer);
	abort();
    }
    curr->status = HEAPfree;

    // remove from active list.
    proc->heapActive.remove(pointer);

    // back onto free list.
    proc->heapFree.add(pointer);
#endif
}

process *allocateProcess(int pid, char *name)
{
    process *ret;

    ret = (process *) xcalloc(sizeof(process), 1);
    processList.add(ret, (void *) pid);

    if (!machineResource) {
	char hostName[80];

	gethostname(hostName, sizeof(hostName));
	resource machineRoot;
	machineRoot = newResource(rootResource, NULL, NULL, "Machine", 0.0,FALSE);
	machineResource = newResource(machineRoot, NULL, NULL, hostName, 0.0, FALSE);
    }

    ret->pid = pid;
    if (!processResource) {
	processResource = newResource(rootResource, NULL, NULL, "Process", 0.0,FALSE);
    }
    ret->rid = newResource(processResource, ret, NULL, name, 0.0, TRUE);

    ret->bufEnd = 0;

    // this process won't be paused until this flag is set
    ret->reachedFirstBreak = 0;
    return(ret);
}

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(char *file, char *argv[], int nenv, char *envp[])
{
    int r;
    int fd;
    int pid;
    image *img;
    int i, j, k;
    process *ret;
    char name[20];
    int ioPipe[2];
    int tracePipe[2];
    FILE *childError;
    char *inputFile = NULL;
    char *outputFile = NULL;

    // check for I/O redirection in arg list.
    for (i=0; argv[i]; i++) {
	if (argv[i] && !strcmp("<", argv[i])) {
	    inputFile = argv[i+1];
	    for (j=i+2, k=i; argv[j]; j++, k++) {
		argv[k] = argv[j];
	    }
	    argv[k] = NULL;
	}
	if (argv[i] && !strcmp(">", argv[i])) {
	    outputFile = argv[++i];
	    for (j=i+2, k=i; argv[j]; j++, k++) {
		argv[k] = argv[j];
	    }
	    argv[k] = NULL;
	}
    }

    r = socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
    if (r) {
	perror("socketpair");
	return(NULL);
    }

    r = socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, ioPipe);
    if (r) {
	perror("socketpair");
	return(NULL);
    }
    //
    // WARNING This code assumes that vfork is used, and a failed exec will
    //   corectly change failed in the parent process.
    //
    errno = 0;
#ifdef PARADYND_PVM
// must use fork, since pvmendtask will do some writing in the address space
    pid = fork();
    fprintf(stderr, "FORK: pid=%d\n", pid);
#else
    pid = vfork();
#endif
    if (pid > 0) {
	if (errno) {
	    sprintf(errorLine, "Unable to start %s: %s\n", file, sys_errlist[errno]);
	    logLine(errorLine);
	    return(NULL);
	}

	img = parseImage(file, 0);
	if (!img) {
	    // destory child process
	    kill(pid, 9);

	    return(NULL);
	}

	/* parent */
	sprintf(name, "%s", img->name);
	ret = allocateProcess(pid, name);
	ret->symbols = img;
	initInferiorHeap(ret, False);

	ret->status = neonatal;
	ret->traceLink = tracePipe[0];
	ret->ioLink = ioPipe[0];
	close(tracePipe[1]);
	close(ioPipe[1]);
	return(ret);
    } else if (pid == 0) {
#ifdef PARADYND_PVM
	pvmendtask(); 
#endif   
	// handle stdio.
	close(ioPipe[0]);
	dup2(ioPipe[1], 1);
	dup2(ioPipe[1], 2);
	if (ioPipe[1] > 2) close (ioPipe[1]);

	// setup stderr for rest of exec try.
	childError = fdopen(2, "w");

	close(tracePipe[0]);
	if (dup2(tracePipe[1], 3) != 3) {
	    fprintf(childError, "dup2 failed\n");
	    fflush(childError);
	    _exit(-1);
	}

	/* close if higher */
	if (tracePipe[1] > 3) close(tracePipe[1]);

	/* see if I/O needs to be redirected */
	if (inputFile) {
	    fd = open(inputFile, O_RDONLY, 0);
	    if (fd < 0) {
		fprintf(childError, "stdin open of %s failed\n", inputFile);
		fflush(childError);
		_exit(-1);
	    } else {
		dup2(fd, 0);
		close(fd);
	    }
	}

	if (outputFile) {
	    fd = open(outputFile, O_WRONLY|O_CREAT, 0444);
	    if (fd < 0) {
		fprintf(childError, "stdout open of %s failed\n", inputFile);
		fflush(childError);
		_exit(-1);
	    } else {
		dup2(fd, 1);
		close(fd);
	    }
	}

	/* indicate our desire to be trace */
	errno = 0;
	ptrace(PTRACE_TRACEME, 0, 0, 0, 0);
	if (errno != 0) {
	  perror("ptrace error\n");
	  _exit(-1);
	}
#ifdef PARADYND_PVM
 	while (nenv-- > 0)
		pvmputenv(envp[nenv]);
#endif
	execvp(file, argv);
	perror("exev");
	_exit(-1);
	return(NULL);
    } else {
	perror("vfork");
	free(ret);
	return(NULL);
    }
}

process *findProcess(int pid)
{
   return(processList.find((void *) pid));
   return(NULL);
}
