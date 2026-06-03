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

/************************************************************************
 * $Id: RTwinnt.c,v 1.22 2006/06/09 03:50:49 jodom Exp $
 * RTwinnt.c: runtime instrumentation functions for Windows NT
 ************************************************************************/
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTcommon.h"
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Dbghelp.h>
#include <Psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <errno.h>
#include <limits.h>
#include <process.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <assert.h>

extern unsigned long dyninstTrapTableUsed;
extern unsigned long dyninstTrapTableVersion;
extern trapMapping_t *dyninstTrapTable;
extern unsigned long dyninstTrapTableIsSorted;
extern void DYNINSTBaseInit();
extern double DYNINSTstaticHeap_512K_lowmemHeap_1[];
extern double DYNINSTstaticHeap_16M_anyHeap_1[];
extern unsigned long sizeOfLowMemHeap1;
extern unsigned long sizeOfAnyHeap1;

/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void DYNINSTbreakPoint(void) {
  /* TODO: how do we stop all threads? */
    DYNINST_break_point_event = 1;
    DebugBreak();
	DYNINST_break_point_event = 0;
}

void DYNINSTsafeBreakPoint() {
    DYNINSTbreakPoint();
}

static dyntid_t initial_thread_tid;

/* this function is automatically called when windows loads this dll
 if we are launching a mutatee to instrument, dyninst will place
 the correct values in libdyninstAPI_RT_DLL_localPid and
 libdyninstAPI_RT_DLL_localCause and they will be passed to
 DYNINSTinit to correctly initialize the dll.  this keeps us
 from having to instrument two steps from the mutator (load and then 
 the execution of DYNINSTinit()
*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   static int DllMainCalledOnce = 0;
   //fprintf(stderr,"RTLIB: In DllMain staticmode=%d %s[%d]\n", DYNINSTstaticMode, __FILE__,__LINE__);

   if(DllMainCalledOnce)
      return 1;
   DllMainCalledOnce++;

   DYNINSTinit();

#if defined(cap_mutatee_traps)
   if (DYNINSTstaticMode) {
      DYNINSTinitializeTrapHandler();
   }
#endif

   return 1; 
}
 


char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
    HMODULE res;
    gLoadLibraryErrorString[0] = '\0';
    res = LoadLibrary(libname);
    if (res == NULL) {
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  gLoadLibraryErrorString, ERROR_STRING_LENGTH, NULL);
        return 0;
    }
    return 1;
}


int dyn_pid_self()
{
   return _getpid();
}

int dyn_lwp_self()
{
#ifdef _WIN64
    return GetCurrentThreadId();
#else
    /* return GetCurrentThreadId(); */
    /* getCurrentThreadId() is conflicting with SD-Dyninst instrumentation.
     *  So I'm doing the massively unportable thing here and hard-coding the assembly
     *  FOR GREAT JUSTICE!
     */
    /* This will do stack frame setup, but that seems harmless in this context... */
    __asm
    {
      mov     EAX,FS:[0x18]
      mov     EAX,DS:[EAX+0x24]
    }
#endif
}

dyntid_t dyn_pthread_self()
{
   return (dyntid_t) dyn_lwp_self();
}

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev)
{
    return 1;
}

/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread(dyntid_t tid) {
    return (tid == initial_thread_tid);
}

// Check that the address is backed by a file,
// get the binary's load address,
// get the PE header, assuming there is one,
// see if the last section has been tagged with "DYNINST_REWRITE"
// get trap-table header from last binary section's end - label - size
// sets allocBase to the binary's load address
static struct trap_mapping_header *getStaticTrapMap(unsigned long addr, unsigned long *allocBase)
{
   struct trap_mapping_header *header = NULL;
   char fileName[ERROR_STRING_LENGTH];
   DWORD actualNameLen = 0;
   MEMORY_BASIC_INFORMATION memInfo;
   int numSections = 0;
   PIMAGE_NT_HEADERS peHdr = NULL;
   IMAGE_SECTION_HEADER curSecn;
   int sidx=0;
   char *str=NULL;

   //check that the address is backed by a file
   actualNameLen = GetMappedFileName(GetCurrentProcess(), 
                                     (LPVOID)addr, 
                                     fileName, 
                                     ERROR_STRING_LENGTH);
   if (!actualNameLen) {
      fileName[0] = '\0';
      goto done; // no file mapped at trap address
   }
   fileName[ERROR_STRING_LENGTH-1] = '\0';

   // get the binary's load address, size
   if (!VirtualQuery((LPCVOID)addr, &memInfo, sizeof(memInfo)) 
       || MEM_COMMIT != memInfo.State) 
   {
      fprintf(stderr, "ERROR IN RTLIB: getStaticTrapMap %s[%d]\n", __FILE__,__LINE__);
      goto done; // shouldn't be possible given previous query, but hey
   }
   *allocBase = (unsigned long) memInfo.AllocationBase;

   rtdebug_printf("RTLIB: getStaticTrapMap addr=%lx meminfo.BaseAddress=%lx "
                  "meminfo.AllocationBase = %lx, memInfo.RegionSize = %lx, "
                  "%s[%d]\n", addr, memInfo.BaseAddress, 
                  memInfo.AllocationBase, memInfo.RegionSize, 
                  __FILE__,__LINE__);

   // get the PE header, assuming there is one
   peHdr = ImageNtHeader( memInfo.AllocationBase );
   if (!peHdr) {
      fprintf(stderr, "ERROR IN RTLIB: getStaticTrapMap %s[%d]\n", __FILE__,__LINE__);
      goto done; // no pe header
   }

   // see if the last section has been tagged with "DYNINST_REWRITE"
   numSections = peHdr->FileHeader.NumberOfSections;
   curSecn = *(PIMAGE_SECTION_HEADER)
            (((unsigned char*)peHdr) 
            + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) 
            + peHdr->FileHeader.SizeOfOptionalHeader
            + sizeof(IMAGE_SECTION_HEADER)*(numSections-1));

   //fprintf(stderr, "RTLIB: PE section header address = %lx\n", curSecn);
   //fprintf(stderr, "curSecn.chars = %lx %s[%d]\n",curSecn.Characteristics, __FILE__,__LINE__);
   if ((sizeof(void*) + 16) > curSecn.SizeOfRawData) {
      fprintf(stderr, "ERROR IN RTLIB: getStaticTrapMap %s[%d]\n", __FILE__,__LINE__);
      goto done; // last section is uninitialized, doesn't have trap table
   }

   //fprintf(stderr, "RTLIB %s[%d]\n", __FILE__,__LINE__);
   //fprintf(stderr, "RTLIB mi.ab =%lx cs.va =%lx cs.srd=%lx %s[%d]\n", memInfo.AllocationBase, curSecn.VirtualAddress, curSecn.SizeOfRawData, __FILE__,__LINE__);
   str = (char*)((long)memInfo.AllocationBase 
                 + curSecn.VirtualAddress 
                 + curSecn.SizeOfRawData 
                 - 16);
   if (0 != strncmp("DYNINST_REWRITE", str, 15)) {
      fprintf(stderr, "ERROR IN RTLIB: getStaticTrapMap found bad string [%s] at %p (%s[%d])\n",
              str, str, __FILE__,__LINE__);
      goto done; // doesn't have DYNINST_REWRITE label
   }

   // get trap-table header
   header = (struct trap_mapping_header*) 
       ( (unsigned long)memInfo.AllocationBase + *((unsigned long*)(str - sizeof(void*))) );

done: 
   if (header) {
       rtdebug_printf( "RTLIB: found trap map header at %lx: [%lx %lx]\n", 
              (unsigned long) header, header->low_entry, header->high_entry);
   } else {
      rtdebug_printf( "ERROR: didn't find trap table\n");
   }
   return header;
}

// Find the target IP and substitute. Leave everything else untouched.
LONG dyn_trapHandler(PEXCEPTION_POINTERS e)
{
   void *trap_to=0;
   void *trap_addr = (void*) ((unsigned char*)e->ExceptionRecord->ExceptionAddress);
   unsigned long zero = 0;
   unsigned long one = 1;
   unsigned long loadAddr = 0;
   struct trap_mapping_header *hdr = NULL;
   trapMapping_t *mapping = NULL;
   rtdebug_printf("RTLIB: In dyn_trapHandler for exception type 0x%lx at 0x%lx\n",
           e->ExceptionRecord->ExceptionCode, trap_addr);
 
   assert(DYNINSTstaticMode && "detach on the fly not implemented on Windows");

   if (EXCEPTION_BREAKPOINT != e->ExceptionRecord->ExceptionCode) {
      fprintf(stderr,"RTLIB: dyn_trapHandler exiting early, exception "
              "type = 0x%lx triggered at %p is not breakpoint %s[%d]\n",
              e->ExceptionRecord->ExceptionCode, trap_addr, __FILE__,__LINE__);
      return EXCEPTION_CONTINUE_SEARCH;
   }

   hdr = getStaticTrapMap((unsigned long) trap_addr, &loadAddr);
   assert(hdr);
   mapping = &(hdr->traps[0]);

   rtdebug_printf("RTLIB: calling dyninstTrapTranslate(\n\t0x%lx, \n\t"
           "0x%lx, \n\t0x%lx, \n\t0x%lx, \n\t0x%lx)\n", 
           (unsigned long)trap_addr - loadAddr + 1, 
           hdr->num_entries, zero, mapping, one);

   trap_to = dyninstTrapTranslate((void*)((unsigned long)trap_addr - loadAddr + 1),
                                  (unsigned long *) &hdr->num_entries,
                                  &zero, 
                                  (volatile trapMapping_t **) &mapping,
                                  &one);

#ifdef _WIN64
    rtdebug_printf("RTLIB: changing Rip from trap at 0x%lx to 0x%lx\n",
	   e->ContextRecord->Rip, (long)trap_to + loadAddr);
   e->ContextRecord->Rip = (long)trap_to + loadAddr;
#else
   rtdebug_printf("RTLIB: changing Eip from trap at 0x%lx to 0x%lx\n", 
           e->ContextRecord->Eip, (long)trap_to + loadAddr);
   e->ContextRecord->Eip = (long) trap_to + loadAddr;
#endif
   return EXCEPTION_CONTINUE_EXECUTION;
}

PVOID fake_AVEH_handle;
/* registers the trap handler by calling AddVectoredExceptionHandler
 */
int DYNINSTinitializeTrapHandler()
{
   fake_AVEH_handle = AddVectoredExceptionHandler
      (RT_TRUE, (PVECTORED_EXCEPTION_HANDLER)dyn_trapHandler);
   rtdebug_printf("RTLIB: added vectored trap handler\n");
   return fake_AVEH_handle != 0;
}

PVOID dyn_AddVectoredExceptionHandler
(ULONG isFirst, PVECTORED_EXCEPTION_HANDLER handler)
{
   PVOID handlerHandle;
   if (isFirst) {
      RemoveVectoredExceptionHandler(fake_AVEH_handle);
      handlerHandle = AddVectoredExceptionHandler(isFirst,handler);
      fake_AVEH_handle = AddVectoredExceptionHandler
         (isFirst,(PVECTORED_EXCEPTION_HANDLER)dyn_trapHandler);
   }
   else {
      handlerHandle = AddVectoredExceptionHandler(isFirst,handler);
   }
   return handlerHandle;
}

extern int fakeTickCount;
extern FILE *stOut;
DWORD __stdcall DYNINST_FakeTickCount()
{
    DWORD tmp = 0x12345678;
    if (0 == fakeTickCount) {
        fakeTickCount = tmp;
    } else {
        fakeTickCount = fakeTickCount + 2;
//        fakeTickCount = fakeTickCount + (tmp - fakeTickCount)/1000 + 1;
    }
    fprintf(stOut,"0x%x = DYNINST_FakeTickCount()\n",fakeTickCount);
    return (DWORD) fakeTickCount;
}

BOOL __stdcall DYNINST_FakeBlockInput(BOOL blockit)
{
    BOOL ret = RT_TRUE;
    fprintf(stOut,"0x%x = DYNINST_FakeBlockInput(%d)\n",ret,blockit);
    return ret;
}

DWORD __stdcall DYNINST_FakeSuspendThread(HANDLE hThread)
{
    DWORD suspendCnt = 0;
    fprintf(stOut,"%d = DYNINST_FakeSuspendThread(%p)\n",suspendCnt,hThread);
    return suspendCnt;
}

BOOL __stdcall DYNINST_FakeCheckRemoteDebuggerPresent(HANDLE hProcess, PBOOL bpDebuggerPresent)
{
    BOOL ret = RT_FALSE;
    fprintf(stOut,"%d = DYNINST_FakeCheckRemoteDebuggerPresent(%p,%p)\n",
            ret, hProcess, bpDebuggerPresent);
    (*bpDebuggerPresent) = ret;
    return ret;
}

VOID __stdcall DYNINST_FakeGetSystemTime(LPSYSTEMTIME lpSystemTime)
{
    lpSystemTime->wYear = 2009;
    lpSystemTime->wMonth = 5;
    lpSystemTime->wDayOfWeek = 0;
    lpSystemTime->wDay = 3;
    lpSystemTime->wHour = 10;
    lpSystemTime->wMinute = 1;
    lpSystemTime->wSecond = 33;
    lpSystemTime->wMilliseconds = 855;
    fprintf(stOut,"called DYNINST_FakeGetSystemTime()\n");
    fflush(stOut);
}

void mark_heaps_exec() 
{
	int OK;
	DWORD old_permissions;
	OK = VirtualProtect(DYNINSTstaticHeap_16M_anyHeap_1, sizeOfAnyHeap1, 
			PAGE_EXECUTE_READWRITE, &old_permissions);
	if(!OK) {
		fprintf(stderr, "ERROR: 16M/any heap not usable in RTlib: %d\n", GetLastError());
	}
	OK = VirtualProtect(DYNINSTstaticHeap_512K_lowmemHeap_1, sizeOfLowMemHeap1, 
			PAGE_EXECUTE_READWRITE, &old_permissions);
	if(!OK) {
		fprintf(stderr, "ERROR: 512k/lowmem heap not usable in RTlib: %d\n", GetLastError());
	}
}
