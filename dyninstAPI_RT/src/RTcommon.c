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

/* $Id: RTcommon.c,v 1.31 2003/02/04 14:59:36 bernat Exp $ */

#if defined(i386_unknown_nt4_0)
#include <process.h>
#define getpid _getpid
#else
#if !defined(mips_unknown_ce2_11) /*ccw 15 may 2000 : 29 mar 2001*/
#include <unistd.h>
#endif

#endif
#if !defined(mips_unknown_ce2_11) /*ccw 15 may 2000 : 29 mar 2001*/
#include <assert.h>
#endif

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

extern void DYNINSTbreakPoint();
extern void DYNINSTos_init(int calledByFork, int calledByAttach);

/* 
   Problem: the tramp guards are in Dyninst, but Paradyn may be multithreaded.
   Solution: make this an array temporarily
*/
unsigned int DYNINSTversion = 1;
unsigned int DYNINSTobsCostLow;
unsigned int DYNINSThasInitialized = 0; /* 0 : has not initialized
					   2 : initialized by Dyninst
					   3 : initialized by Paradyn */

struct DYNINST_bootstrapStruct DYNINST_bootstrap_info ={0,0,0,'\0'} ; /*ccw 10 oct 2000 : 29 mar 2001*/

double DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(double)];
double DYNINSTstaticHeap_32K_lowmemHeap_1[32*1024/sizeof(double)];
double DYNINSTstaticHeap_4M_anyHeap_1[4*1024*1024/sizeof(double)];
int DYNINSTtrampGuard=1;

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

double DYNINSTdummydouble = 4321.71; /* Global so the compiler won't
					optimize away FP code in initFPU */
static void initFPU()
{
       /* Init the FPU.  We've seen bugs with Linux (e.g., Redhat 6.2
	  stock kernel on PIIIs) where processes started by Paradyn
	  started with FPU uninitialized. */
       double x = 17.1234;
       DYNINSTdummydouble *= x;
}

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
int isMutatedExec = 0;
#endif

/*
 * The Dyninst API arranges for this function to be called at the entry to
 * main().
 */
void DYNINSTinit(int cause, int pid)
{
    int calledByFork = 0, calledByAttach = 0;

    initFPU();

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
	/* this checks to see if this is a restart or a
	  	normal attach  ccw 19 nov 2001*/
	
/* 
	isElfFile;
#else
	isXCOFFfile;
#endif
*/
	if(isMutatedExec){
		fflush(stdout);
		cause = 9;
	}
#endif

    DYNINSThasInitialized = 2;

    if (cause == 2) calledByFork = 1;
    else if (cause == 3) calledByAttach = 1;
    
    /* sanity check */
#if !defined(mips_unknown_ce2_11) /*ccw 15 may 2000 : 29 mar 2001*/
    assert(sizeof(int64_t) == 8);
    assert(sizeof(int32_t) == 4);
#endif
    
#ifndef mips_unknown_ce2_11 /*ccw 23 july 2001*/
    RTprintf("%s\n", V_libdyninstAPI_RT);
#endif
    
    DYNINSTos_init(calledByFork, calledByAttach);
    
#if !defined(mips_unknown_ce2_11) /*ccw 16 may 2000 : 29 mar 2001*/
    DYNINST_bootstrap_info.pid = getpid();
#endif
    DYNINST_bootstrap_info.ppid = pid;
    DYNINST_bootstrap_info.event = cause;
    
    DYNINST_mutatorPid = pid;

}

/* These variables are used to pass arguments into DYNINSTinit
   when it is called as an _init function */
int libdyninstAPI_RT_init_localCause=-1;
int libdyninstAPI_RT_init_localPid=-1;

#if defined(i386_unknown_nt4_0)  /*ccw 13 june 2001*/
#include <windows.h>

/* this function is automatically called when windows loads this dll
 if we are launching a mutatee to instrument, dyninst will place
 the correct values in libdyninstAPI_RT_DLL_localPid and
 libdyninstAPI_RT_DLL_localCause and they will be passed to
 DYNINSTinit to correctly initialize the dll.  this keeps us
 from having to instrument two steps from the mutator (load and then 
 the execution of DYNINSTinit()
*/
int DllMainCalledOnce = 0;
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  /* handle to DLL module */
  DWORD fdwReason,     /* reason for calling function */
  LPVOID lpvReserved   /* reserved */
){
	if(DllMainCalledOnce == 0){
		DllMainCalledOnce++;
		if(libdyninstAPI_RT_init_localPid != -1 || libdyninstAPI_RT_init_localCause != -1){
			DYNINSTinit(libdyninstAPI_RT_init_localCause,libdyninstAPI_RT_init_localPid);
		}
	}
	return 1; 
}
 

#else

/* _init table of methods:
   GCC: link with gcc -shared, and use __attribute__((constructor));
   AIX: ld with -binitfini:libdyninstAPI_RT_init
   Solaris: ld with -z initarray=libdyninstAPI_RT_init
   Linux: ld with -init libdyninstAPI_RT_init
          gcc with -Wl,-init -Wl,...
          
*/

/* Convince GCC to run _init when the library is loaded */
#ifdef __GNUC
void libdyninstAPI_RT_init(void) __attribute__ ((constructor));
#endif

/* UNIX-style initialization through _init */
int initCalledOnce = 0;
void libdyninstAPI_RT_init() {
    if (initCalledOnce) return;
    initCalledOnce++;
/* Has its own call-once protection, and so can be called here */
    RTmutatedBinary_init();
    
    if(libdyninstAPI_RT_init_localCause != -1 ||
       libdyninstAPI_RT_init_localPid != -1) {
        DYNINSTinit(libdyninstAPI_RT_init_localCause,
                    libdyninstAPI_RT_init_localPid);
    }
}

#endif

#if !defined(i386_unknown_nt4_0)  && !defined(mips_unknown_ce2_11)/*ccw 2 may 2000 : 29 mar 2001*/
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

/* Does what it's called. Used by the paradyn daemon as a 
   default in certain cases (MT in particular)
*/

int DYNINSTreturnZero()
{
  return 0;
}
