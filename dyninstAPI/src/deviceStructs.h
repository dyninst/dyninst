/*
 * Copyright (c) 1998 Barton P. Miller
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


/* 
 * $Id: deviceStructs.h,v 1.1 2001/08/01 15:39:55 chadd Exp $
 */

#include "w32CONTEXT.h"
#define RKill 0
#define RWriteProcessMemory 1
#define RCreateProcess 2
#define ROpenProcess 3
#define RReadProcessMemory 4
#define RDebugActiveProcess 5
#define RGetThreadContext 6
#define RSetThreadContext 7
#define RWaitForDebugEvent 8
#define RContinueDebugEvent 9
#define RGetTrampTemplate 10
#define RResumeThread 11
#define RSuspendThread 12
#define RTerminateProcess 13
#define RCloseHandle 14
#define RFlushInstructionCache 15

struct iWriteProcessMemory{
 
	DWORD  hProcessSize;
	HANDLE hProcess;

	DWORD  lpBaseAddressSize;
	LPVOID lpBaseAddress;

	DWORD  nSizeSize;
	DWORD  nSize;
	//LPDWORD lpNumberOfBytesWritten // return parameter

	DWORD  lpBufferSize;
	BYTE lpBuffer;
};

struct oWriteProcessMemory{
	DWORD retValSize;
	BOOL  retVal;

	DWORD lpNumberOfBytesWrittenSize;
	DWORD lpNumberOfBytesWritten;
};

struct iCreateProcess{
	DWORD lpApplicationNameSize;
	DWORD lpApplicationNameOffset;

	DWORD lpCommandLineSize;
	DWORD lpCommandLineOffset;

	DWORD dwCreationFlagsSize;
	DWORD dwCreationFlags;
};

 
struct oCreateProcess{
	DWORD retValSize;
	BOOL retVal;

	DWORD lpProcessInformationSize;
	PROCESS_INFORMATION lpProcessInformation;
};

struct iOpenProcess{
	DWORD IDProcess;
};

struct oOpenProcess{
	HANDLE retVal;
	DWORD lastError;
};


struct iReadProcessMemory{
	DWORD  hProcessSize;
	HANDLE hProcess;

	DWORD  lpBaseAddressSize;
	LPVOID lpBaseAddress;

	DWORD  nSizeSize;
	DWORD  nSize;

};

struct oReadProcessMemory{
	DWORD retValSize;
	BOOL  retVal;

	DWORD lpNumberOfBytesReadSize;
	DWORD lpNumberOfBytesRead;

	DWORD  lpBufferSize;
	BYTE   lpBuffer;

};

struct iDebugActiveProcess {

	DWORD dwProcessIdSize;
	DWORD dwProcessId;

};

struct oDebugActiveProcess {

	DWORD retValSize;
	DWORD retVal;
};

struct iGetThreadContext {
	DWORD hThreadSize;
	HANDLE hThread;

};

struct oGetThreadContext {
	DWORD retValSize;
	DWORD retVal;

	DWORD lpContextSize;
	w32CONTEXT lpContext;
};

struct iSetThreadContext {
	DWORD  hThreadSize;
	HANDLE hThread;

	DWORD   lpContextSize;
	w32CONTEXT lpContext;

};

struct oSetThreadContext {
	DWORD retValSize;
	DWORD retVal;
};

struct iWaitForDebugEvent{
	DWORD dwMillisecondsSize;
	DWORD dwMilliseconds;
};

struct oWaitForDebugEvent{

	DWORD retValSize;
	DWORD retVal;

	DWORD lpDebugEventSize;
	DEBUG_EVENT lpDebugEvent;

};

struct iContinueDebugEvent{
	DWORD dwProcessIdSize;
	DWORD dwProcessId;

	DWORD dwThreadIdSize;
	DWORD dwThreadId;

	DWORD dwContinueStatusSize;
	DWORD dwContinueStatus;

};

struct oContinueDebugEvent{
	DWORD retValSize;
	DWORD retVal;

};

struct iGetTrampTemplate{

};

struct oGetTrampTemplate{
	DWORD bufferSize;
	DWORD bufferSizeSize;
	DWORD bufferNRSize;
	DWORD bufferNRSizeSize;
	// offsets for different functions

	DWORD baseTramp;
	DWORD baseTramp_savePreInsOffset;
	DWORD baseTramp_skipPreInsOffset;
	DWORD baseTramp_globalPreOffset;
	DWORD baseTramp_localPreOffset;
	DWORD baseTramp_localPreReturnOffset;
	DWORD baseTramp_updateCostOffset;
	DWORD baseTramp_restorePreInsOffset;
	DWORD baseTramp_emulateInsOffset;
	DWORD baseTramp_skipPostInsOffset;
	DWORD baseTramp_savePostInsOffset;
	DWORD baseTramp_globalPostOffset;
	DWORD baseTramp_localPostOffset;
	DWORD baseTramp_localPostReturnOffset;
	DWORD baseTramp_restorePostInsOffset;
	DWORD baseTramp_returnInsOffset;
	DWORD baseTramp_endTrampOffset;

	DWORD baseTrampSize;
	DWORD baseTrampCost;           // cost if both pre- and post- skipped
	DWORD baseTrampPrevBaseCost ; // cost of [global_pre_branch, update_cost)
	DWORD baseTrampPostBaseCost ; // cost of [global_post_branch, return_insn)
	DWORD baseTrampPrevInstru ;
	DWORD baseTrampPostInstru ;

	DWORD baseNonRecursiveTramp;
	DWORD baseNonRecursiveTramp_savePreInsOffset;
	DWORD baseNonRecursiveTramp_skipPreInsOffset;
	DWORD baseNonRecursiveTramp_globalPreOffset;
	DWORD baseNonRecursiveTramp_localPreOffset;
	DWORD baseNonRecursiveTramp_localPreReturnOffset;
	DWORD baseNonRecursiveTramp_updateCostOffset;
	DWORD baseNonRecursiveTramp_restorePreInsOffset;
	DWORD baseNonRecursiveTramp_emulateInsOffset;
	DWORD baseNonRecursiveTramp_skipPostInsOffset;
	DWORD baseNonRecursiveTramp_savePostInsOffset;
	DWORD baseNonRecursiveTramp_globalPostOffset;
	DWORD baseNonRecursiveTramp_localPostOffset;
	DWORD baseNonRecursiveTramp_localPostReturnOffset;
	DWORD baseNonRecursiveTramp_restorePostInsOffset;
	DWORD baseNonRecursiveTramp_returnInsOffset;
	DWORD baseNonRecursiveTramp_guardOnPre_beginOffset;
	DWORD baseNonRecursiveTramp_guardOffPre_beginOffset;
	DWORD baseNonRecursiveTramp_guardOnPost_beginOffset;
	DWORD baseNonRecursiveTramp_guardOffPost_beginOffset;
	DWORD baseNonRecursiveTramp_guardOnPre_endOffset;
	DWORD baseNonRecursiveTramp_guardOffPre_endOffset;
	DWORD baseNonRecursiveTramp_guardOnPost_endOffset;
	DWORD baseNonRecursiveTramp_guardOffPost_endOffset;
	DWORD baseNonRecursiveTramp_endTrampOffset;

	DWORD baseNonRecursiveTrampSize;
	DWORD baseNonRecursiveTrampCost;           // cost if both pre- and post- skipped
	DWORD baseNonRecursiveTrampPrevBaseCost ; // cost of [global_pre_branch, update_cost)
	DWORD baseNonRecursiveTrampPostBaseCost ; // cost of [global_post_branch, return_insn)
	DWORD baseNonRecursiveTrampPrevInstru ;
	DWORD baseNonRecursiveTrampPostInstru ;

	DWORD retValSize;
	DWORD retVal;

	BYTE buffer;//this holds the Buffer and the bufferNR, the first
				//bufferSize bytes are the BUffer and the next bufferNRSize bytes is
				//the bufferNR
};

struct iResumeThread {
	DWORD hThreadSize;
	HANDLE hThread;

};

struct oResumeThread {
	DWORD retValSize;
	bool retVal;

};


struct iSuspendThread {
	DWORD hThreadSize;
	HANDLE hThread;

};

struct oSuspendThread {
	DWORD retValSize;
	bool retVal;

};

struct iTerminateProcess {
	DWORD hThreadSize;
	HANDLE hThread;

	DWORD uExitCodeSize;
	UINT uExitCode;
};

struct oTerminateProcess {
	DWORD retValSize;
	bool retVal;
};

struct iCloseHandle {
	DWORD hThreadSize;
	HANDLE hThread;

};

struct oCloseHandle{
	DWORD retValSize;
	bool retVal;

};

//ccw 29 sep 2000
struct iFlushInstructionCache {
	DWORD hProcessSize;
	HANDLE hProcess;

};

//ccw 29 sep 2000
struct oFlushInstructionCache {
	DWORD retValSize;
	bool retVal;
};
