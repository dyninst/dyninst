/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * process.h - interface to manage a process in execution. A process is a kernel
 *   visible unit with a seperate code and data space.  It might not be
 *   the only unit running the code, but it is only one changed when
 *   ptrace updates are applied to the text space.
 *
 * $Log: process.h,v $
 * Revision 1.6  1994/07/14 23:30:32  hollings
 * Hybrid cost model added.
 *
 * Revision 1.5  1994/07/12  19:43:18  jcargill
 * Changed order of processState defn, so that initial (==0) state is neonatal.
 * Otherwise there is a small time-window at startup when it looks like it's
 * running, but hasn't been initialized.
 *
 * Revision 1.4  1994/05/18  00:52:32  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.3  1994/03/31  01:58:33  markc
 * Extended arguments to createProcess.
 *
 * Revision 1.2  1994/03/20  01:53:11  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.1  1994/01/27  20:31:39  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.6  1993/08/11  01:46:24  hollings
 * added defs for baseMap.
 *
 * Revision 1.5  1993/07/13  18:29:49  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/25  22:23:28  hollings
 * added parent field to process.h
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "util/h/list.h"
#include "rtinst/h/rtinst.h"
#include "dyninst.h"

typedef struct processRec process;

typedef enum { neonatal, running, stopped, exited } processState;

typedef struct freeListRec freeListEntry;

typedef enum { HEAPfree, HEAPallocated } heapStatus;

struct freeListRec {
    int addr;
    int length;
    heapStatus status;
    freeListEntry *next;
}; 

#define HIST_LIMIT      6

struct costRec {
    float hybrid;
    int currentHist;
    float lastObservedCost;
    float past[HIST_LIMIT];
    float totalPredictedCost;
    float currentPredictedCost;
    time64 timeLastTrampSample;
};

struct processRec {
    image *symbols;		/* information related to the process */
    int traceLink;		/* pipe to transfer traces data over */
    int ioLink;			/* pipe to transfer stdout/stderr over */
    processState status;	/* running, stopped, etc. */
    int pid;			/* id of this process */
    int thread;			/* thread id for thread */
    Boolean aggregate;		/* is this process a pseudo process ??? */
    freeListEntry *heap;	/* inferior heap */
    resource rid;		/* handle to resource for this process */
    process *parent;		/* parent of this proces */
    List<void *> baseMap;	/* map and inst point to its base tramp */
    char buffer[2048];
    int bufStart;
    int bufEnd;
    struct costRec cost;
};

extern List<process*> processList;

process *createProcess(char *file, char *argv[], int nenv, char *envp[]);
process *allocateProcess(int pid, char *name);
process *findProcess(int pid);
void initInferiorHeap(process *proc, Boolean globalHeap);
void copyInferriorHeap(process *from, process *to);

int inferriorMalloc(process *proc, int size);
void inferriorFree(process *proc, int pointer);

#endif
