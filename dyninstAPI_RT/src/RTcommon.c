/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: RTcommon.c,v 1.13 2000/10/17 17:42:28 schendel Exp $ */

#if defined(i386_unknown_nt4_0)
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif
#include <assert.h>

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

extern void DYNINSTbreakPoint();
extern void DYNINSTos_init(int calledByFork, int calledByAttach);

unsigned int DYNINSTversion = 1;
unsigned int DYNINSTobsCostLow;
unsigned int DYNINSThasInitialized = 0; /* 0 : has not initialized
					   2 : initialized by Dyninst
					   3 : initialized by Paradyn */

struct DYNINST_bootstrapStruct DYNINST_bootstrap_info;

double DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(double)];
double DYNINSTstaticHeap_32K_lowmemHeap_1[32*1024/sizeof(double)];
double DYNINSTstaticHeap_4M_anyHeap_1[4*1024*1024/sizeof(double)];

/* Written to by daemon just before launching an inferior RPC */
rpcInfo curRPC = { 0, 0, 0 };
unsigned pcAtLastIRPC;  /* just used to check for errors */
/* 1 = a trap was ignored and needs to be regenerated
   0 = there is not a trap that hasn't been processed */
int trapNotHandled = 0;

#ifdef DEBUG_PRINT_RT
int DYNINSTdebugPrintRT = 1;
#else
int DYNINSTdebugPrintRT = 0;
#endif

int DYNINST_mutatorPid = -1;

extern const char V_libdyninstAPI_RT[];

/*
 * The Dyninst API arranges for this function to be called at the entry to
 * main().
 */
void DYNINSTinit(int cause, int pid)
{
    int calledByFork = 0, calledByAttach = 0;

    DYNINSThasInitialized = 2;
    if (cause == 2) calledByFork = 1;
    else if (cause == 3) calledByAttach = 1;

    /* sanity check */
    assert(sizeof(int64_t) == 8);
    assert(sizeof(int32_t) == 4);

    RTprintf("%s\n", V_libdyninstAPI_RT);

    DYNINSTos_init(calledByFork, calledByAttach);

    DYNINST_bootstrap_info.pid = getpid();
    DYNINST_bootstrap_info.ppid = pid;
    DYNINST_bootstrap_info.event = cause;

    DYNINST_mutatorPid = pid;

    DYNINSTbreakPoint();
}

#if !defined(i386_unknown_nt4_0)
/*
 * handle vfork special case
 */
void DYNINSTvfork(int parent)
{
    /* sanity check */
    assert(sizeof(int64_t)  == 8);
    assert(sizeof(int32_t)  == 4);

    if (parent != getpid()) {
	DYNINST_bootstrap_info.pid = getpid();
	DYNINST_bootstrap_info.ppid = getppid();
	DYNINST_bootstrap_info.event = 3;
    } else {
	DYNINSTbreakPoint();
	DYNINST_bootstrap_info.event = 0;
    }
}
#endif
