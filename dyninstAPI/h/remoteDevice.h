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
 * $Id: remoteDevice.h,v 1.1 2001/08/01 15:39:52 chadd Exp $
 */


#include "common/h/ntHeaders.h"
#include "w32CONTEXT.h"
#include <rapi.h> //ccw 2 aug 2000
#include "baseTrampTemplate.h" //ccw 2 aug 2000


class remoteDevice {

	SOCKET deviceSocket;
	bool success;
public:
	//constructor
	remoteDevice();

	bool test(LPCWSTR st1r,LPCWSTR s2tr, int y, LPCWSTR str);

bool RemoteGetTrampTemplate();

bool RemoteWriteProcessMemory( HANDLE hProcess, LPVOID lpBaseAddress, 
	LPVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesWritten);

bool RemoteCreateProcess(LPCWSTR lpApplicationName, LPCWSTR lpCommandLine, 
LPSECURITY_ATTRIBUTES lpProcessAttributes,
LPSECURITY_ATTRIBUTES lpThreadAttributes, 
bool bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, 
LPCWSTR lpCurrentDirectory, LPSTARTUPINFO
lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation ); 

HANDLE RemoteOpenProcess( DWORD fdwAccess, bool fInherit, 
DWORD IDProcess);

bool RemoteReadProcessMemory( HANDLE hProcess, LPCVOID
lpBaseAddress, 
LPVOID lpBuffer, DWORD nSize, LPDWORD
lpNumberOfBytesRead);

bool RemoteDebugActiveProcess( DWORD dwProcessId );


bool RemoteGetThreadContext( HANDLE hThread, w32CONTEXT *
lpContext);

bool RemoteSetThreadContext( HANDLE hThread, 
CONST w32CONTEXT * lpContext);

bool RemoteWaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent, 
DWORD dwMilliseconds);

bool RemoteContinueDebugEvent( DWORD dwProcessId, DWORD
dwThreadId, 
DWORD dwContinueStatus );

bool RemoteResumeThread(HANDLE hThread);

bool RemoteSuspendThread(HANDLE hThread);

bool RemoteTerminateProcess(HANDLE hThread, UINT uExitCode);
bool RemoteCloseHandle(HANDLE hObject);

//ccw 29 sep 2000
bool RemoteFlushInstructionCache(HANDLE hProcess, LPCVOID lpBaseAddress, DWORD dwSize);

};
