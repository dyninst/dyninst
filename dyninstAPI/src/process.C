/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/process.C,v 1.6 1994/05/16 22:31:53 hollings Exp $";
#endif

/*
 * process.C - Code to control a process.
 *
 * $Log: process.C,v $
 * Revision 1.6  1994/05/16 22:31:53  hollings
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

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninst.h"
#include "symtab.h"
#include "process.h"
#include "util.h"

List<process*> processList;

extern char *sys_errlist[];

/* root of process resource list */
resource processResource;
resource machineResource;

extern "C" ptrace();

#define INFERRIOR_HEAP_BASE	"DYNINSTdata"
#define GLOBAL_HEAP_BASE	"DYNINSTglobalData"

void initInferiorHeap(process *proc, Boolean globalHeap)
{
    assert(proc->symbols);

    proc->heap = (freeListEntry*) xcalloc(sizeof(freeListEntry), 1);
    if (globalHeap) {
	proc->heap->addr = 
	    findInternalAddress(proc->symbols, GLOBAL_HEAP_BASE, True);
    } else {
	proc->heap->addr = 
	    findInternalAddress(proc->symbols, INFERRIOR_HEAP_BASE, True);
    }
    proc->heap->length = SYN_INST_BUF_SIZE;
    proc->heap->next = NULL;
    proc->status = HEAPfree;
}

void copyInferriorHeap(process *from, process *to)
{
    freeListEntry *curr;
    freeListEntry *newEntry;

    assert(from->symbols);
    assert(to->symbols);

    to->heap = NULL;
    /* copy individual elements */
    for (curr=from->heap; curr; curr=curr->next) {
	newEntry = (freeListEntry*) xcalloc(sizeof(freeListEntry), 1);
	*newEntry = *curr;

	/* setup next pointers */
	newEntry->next = to->heap;
	to->heap = newEntry;
    }
    to->status = HEAPfree;
}

int inferriorMalloc(process *proc, int size)
{
    freeListEntry *newEntry;
    freeListEntry *curr;

    /* round to next cache line size */
    /* 32 bytes on a SPARC */
    size = (size + 0x1f) & ~0x1f; 
    for (curr=proc->heap; curr; curr=curr->next) {
	if ((curr->status == HEAPfree) && (curr->length >= size)) break;
    }

    if (!curr) {
	printf("Inferrior heap overflow\n");
	abort();
    }

    if (curr->length != size) {
	newEntry = (freeListEntry *) xcalloc(sizeof(freeListEntry), 1);
	newEntry->length = curr->length - size;
	newEntry->addr = curr->addr + size;
	newEntry->next = curr->next;
	newEntry->status = HEAPfree;

	/* now split curr */
	curr->status = HEAPallocated;
	curr->length = size;
	curr->next = newEntry;
    }
    return(curr->addr);
}

void inferriorFree(process *proc, int pointer)
{
#ifdef notdef
    freeListEntry *curr;

    /* free is currently disabled because we can't handle the case of an
     *  inst function being deleted while active.  Not freeing the memory means
     *  it stil contains the tramp and will get itself out safely.
     */

    for (curr=proc->heap; curr; curr=curr->next) {
	if (curr->addr == pointer) break;
    }

    if (!curr) {
	printf("unable to find heap entry %x\n", pointer);
	abort();
    }

    if (curr->status != HEAPallocated) {
	printf("attempt to free already free heap entry %x\n", pointer);
	abort();
    }
    curr->status = HEAPfree;
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
	machineRoot = newResource(rootResource, NULL, "Machine", 0.0,FALSE);
	machineResource = newResource(machineRoot, NULL, hostName, 0.0, FALSE);
    }

    ret->pid = pid;
    if (!processResource) {
	processResource = newResource(rootResource, NULL, "Process", 0.0,FALSE);
    }
    ret->rid = newResource(processResource, ret, name, 0.0, TRUE);

    ret->bufEnd = 0;
    return(ret);
}

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(char *file, char *argv[], int nenv, char *envp[])
{
    int pid;
    int r;
    image *i;
    process *ret;
    char name[20];
    int tracePipe[2];

    r = socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
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
#else
    pid = vfork();
#endif
    if (pid > 0) {
	if (errno) {
	    printf("Unable to start %s: %s\n", file, sys_errlist[errno]);
	    return(NULL);
	}

	i = parseImage(file, 0);
	if (!i) return(NULL);

	/* parent */
	sprintf(name, "%s", i->name);
	ret = allocateProcess(pid, name);
	ret->symbols = i;
	initInferiorHeap(ret, False);

	ret->status = neonatal;
	ret->traceLink = tracePipe[0];
	close(tracePipe[1]);
	return(ret);
    } else if (pid == 0) {
#ifdef PARADYND_PVM
	pvmendtask(); 
#endif   
	close(tracePipe[0]);
	if (dup2(tracePipe[1], 3) != 3)
	  {
	    fprintf (stderr, "dup2 failed\n");
	    abort();
	  }

	/* close if higher */
	if (tracePipe[1] > 3) close(tracePipe[1]);

	/* indicate our desire to be trace */
	errno = 0;
	ptrace(0, 0, 0, 0);
	if (errno != 0) {
	  perror("ptrace error\n");
	  _exit(-1);
	}
#ifdef PARADYND_PVM
 	while (nenv-- > 0)
		pvmputenv(envp[nenv]);
#endif
	execv(file, argv);
	perror("exev");
	_exit(-1);
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
