/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

// $Id: pdwinnt.C,v 1.27 2001/02/01 01:11:26 schendel Exp $

#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#endif

#ifdef BPATCH_LIBRARY
/* XXX This is only needed for emulating signals. */
#include "BPatch_thread.h"
#include "nt_signal_emul.h"
#endif

extern bool isValidAddress(process *proc, Address where);
extern process *findProcess(int);

//HANDLE kludgeProcHandle;

void printSysError(unsigned errNo) {
    char buf[1000];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);
    fprintf(stderr, "*** System error [%d]: %s\n", errNo, buf);
    fflush(stderr);
}


// check if a file handle is for kernel32.dll
static bool kludge_isKernel32Dll(HANDLE fileHandle, string &kernel32Name) {
    static DWORD IndxHigh, IndxLow;
    static bool firstTime = true;
    BY_HANDLE_FILE_INFORMATION info;
    static string kernel32Name_;

    if (firstTime) {
	HANDLE kernel32H;
	firstTime = false;
	char sysRootDir[MAX_PATH+1];
	if (GetSystemDirectory(sysRootDir, MAX_PATH) == 0)
	  assert(0);
	kernel32Name_ = string(sysRootDir) + "\\kernel32.dll";
	kernel32H = CreateFile(kernel32Name_.string_of(), GENERIC_READ, 
			       FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	assert(kernel32H);
	if (!GetFileInformationByHandle(kernel32H, &info)) {
	    printSysError(GetLastError());
	    assert(0);
	}
	IndxHigh = info.nFileIndexHigh;
	IndxLow = info.nFileIndexLow;
	CloseHandle(kernel32H);
    }

    if (!GetFileInformationByHandle(fileHandle, &info))
	return false;

    if (info.nFileIndexHigh==IndxHigh && info.nFileIndexLow==IndxLow) {
      kernel32Name = kernel32Name_;
      return true;
    }
    return false;
}


void dumpMem(process *p, void * addr, unsigned nbytes) {
    unsigned char *buf = new unsigned char[nbytes];
    memset(buf, 0, nbytes);
    function_base *f;
    assert(buf);

    if (f = p->findFunctionIn((Address)addr))
    {
        printf("Function %s, addr=0x%lx, sz=%d\n", 
                f->prettyName().string_of(),
                f->getAddress(p),
                f->size());
    }
    p->readDataSpace((void *)((unsigned)addr-32), nbytes, buf, true);
    printf("## 0x%lx:\n", (unsigned)addr-32);
    for (unsigned u = 0; u < nbytes; u++)
    {
	    printf(" %x", buf[u]);
    }
    printf( "\n" );
    p->readDataSpace(addr, nbytes, buf, true);
    printf("## 0x%lx:\n", addr);
    for (unsigned u1 = 0; u1 < nbytes; u1++)
    {
	    printf(" %x", buf[u1]);
    }
    printf( "\n" );
}



/***
  process::walkStack - get a stack trace using the imagehlp function StackWalk
  Because the compiler can do optimizations that omit the frame pointer,
  it may not be possible to get a complete stack trace in some cases.
  We assume that a trace is complete if we get to the main function (main or WinMain).
  If we can't get a complete trace, we return an empty vector.
***/
vector<Address> process::walkStack(bool noPause) {
  vector<Address> pcs;
  bool needToCont = noPause ? false : (status() == running);

#ifndef BPATCH_LIBRARY
  BEGIN_STACKWALK;
#endif

  if (!noPause && !pause()) {
     // pause failed...give up
     cerr << "walkStack: pause failed" << endl;

#ifndef BPATCH_LIBRARY
     END_STACKWALK;
#endif

     return pcs;
  }

  static STACKFRAME zero;
  STACKFRAME sf = zero;
  CONTEXT cont;

  cont.ContextFlags = CONTEXT_FULL;
  if (!GetThreadContext((HANDLE)threads[0]->get_handle(), &cont))
     cerr << "walkStack: GetThreadContext failed\n";
  //fprintf(stderr, "Context: EIP = %x, ESP = %x, EBP = %x\n",
  //	 cont.Eip, cont.Esp, cont.Ebp);
  //fflush(stderr);

  sf.AddrPC.Offset = cont.Eip;
  sf.AddrPC.Mode = AddrModeFlat;
  sf.AddrFrame.Offset = cont.Ebp;
  sf.AddrFrame.Mode = AddrModeFlat;
  sf.AddrStack.Offset = cont.Esp;
  sf.AddrStack.Mode = AddrModeFlat;

  sf.AddrReturn.Mode = AddrModeFlat;

  bool res;
  bool reachedMain = false; // set to true if we reach the main function
  while (1) {
    res = StackWalk(IMAGE_FILE_MACHINE_I386, (HANDLE)getProcFileDescriptor(), NULL,
		    &sf, NULL,
		    ReadProcessMemory, SymFunctionTableAccess, SymGetModuleBase,
		    NULL);

    if (!res && GetLastError() == 0) {
      // reached end of stack
      break;
    } else if (!res && GetLastError() != 0) {
      // error - can't complete stack walk
      //fprintf(stderr, "error walking stack\n");
      //printSysError(GetLastError());
      break;
    }
    else {
      function_base *f = findFunctionIn(sf.AddrPC.Offset);
      // We might reach here before the program has reached its main()
      // function, so we need to check for a function higher up in the
      // calling sequence -- mainCRTStartup
      if (f && (f ==  getMainFunction() || f->prettyName() == "mainCRTStartup"))
        reachedMain = true;

      pcs += sf.AddrPC.Offset;
    }

    if (sf.AddrReturn.Offset == 0) {
      pcs += 0;
      break;
    }

  }

  //for (unsigned u = 0; u < pcs.size(); u++) {
  //  function_base *pdf = findFunctionIn(pcs[u]);
  //  fprintf(stderr,"> %x (%s)\n", pcs[u], pdf ? pdf->prettyName().string_of() : "");
  //  fflush(stderr);
  //}

  if (!reachedMain) {
    // error - incomplete trace, return an empty vector
    pcs.resize(0);
    //fprintf(stderr, "****** walkStack failed\n"); fflush(stderr);
  }

  if (!noPause && needToCont) {
     if (!continueProc()){
        cerr << "walkStack: continueProc failed" << endl;
     }
  }  

#ifndef BPATCH_LIBRARY
  END_STACKWALK;
#endif

  return(pcs);
}



/* 
   Loading libDyninstRT.dll

   We load libDyninstRT.dll dynamically, by inserting code into the
   application to call LoadLibraryA. We don't use the inferior RPC
   mechanism from class process because it already assumes that
   libdyninst is loaded (it uses the inferior heap).
   Instead, we use a simple inferior call mechanism defined below
   to insert the code to call LoadLibraryA("libdyninstRT.dll").
 */

Address loadDyninstDll(process *p, char Buffer[LOAD_DYNINST_BUF_SIZE]) {
    Address codeBase = p->getImage()->codeOffset();

    Address LoadLibBase;
    Address LoadLibAddr;
    Symbol sym;
    if (!p->getSymbolInfo("_LoadLibraryA@4", sym, LoadLibBase)) {
        if( !p->getSymbolInfo( "_LoadLibraryA", sym, LoadLibBase ))
        {
	        printf("unable to find function LoadLibrary\n");
	        assert(0);
        }
    }
    LoadLibAddr = sym.addr() + LoadLibBase;
    assert(LoadLibAddr);

    char ibuf[LOAD_DYNINST_BUF_SIZE];
    char *iptr = ibuf;

    // push nameAddr ; 5 bytes
    *iptr++ = (char)0x68; 
    *(int *)iptr = codeBase + 14; 
    iptr += sizeof(int);

    // call LoadLibrary ; 5 bytes
    *iptr++ = (char)0xe8;
    // offset relative to next instruction (address codeBase+10)
    *(int *)iptr = LoadLibAddr - (codeBase + 10);
    iptr += sizeof(int);

    // add sp, 4
    *iptr++ = (char)0x83; *iptr++ = (char)0xc4; *iptr++ = (char)0x04;

    // int3
    *iptr++ = (char)0xcc;

    if (!process::dyninstName.length())
        // check for an environment variable
#ifdef BPATCH_LIBRARY  // dyninstAPI loads a different run-time library
        process::dyninstName = getenv("DYNINSTAPI_RT_LIB");
#else
        process::dyninstName = getenv("PARADYN_LIB");
#endif

    if (!process::dyninstName.length())
        // if environment variable unset, use the default name/strategy
#ifdef BPATCH_LIBRARY
        process::dyninstName = "libdyninstAPI_RT.dll";
#else
        process::dyninstName = "libdyninstRT.dll";
#endif
	
    // make sure that directory separators are what LoadLibrary expects
    strcpy(iptr, process::dyninstName.string_of());
    for (unsigned int i=0; i<strlen(iptr); i++)
        if (iptr[i]=='/') iptr[i]='\\';

    p->readDataSpace((void *)codeBase, LOAD_DYNINST_BUF_SIZE, Buffer, false);
    p->writeDataSpace((void *)codeBase, LOAD_DYNINST_BUF_SIZE, ibuf);

    return codeBase;
}



// osTraceMe is not needed in Windows NT
void OS::osTraceMe(void) {}


#ifndef BPATCH_LIBRARY
void checkProcStatus() {
    int wait_status;
    extern void doDeferredRPCs();

    process::waitProcs(&wait_status);
    doDeferredRPCs();
}
#endif

/*
   wait for inferior processes to terminate or stop.
*/
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block) {
#else
int process::waitProcs(int *status) {
#endif
    DEBUG_EVENT debugEv;
    process *p;
#ifdef BPATCH_LIBRARY
    *status = 0;
#endif

    // We wait for 1 millisecond here. On the Unix platforms, the wait
    // happens on the select in controllerMainLoop. But on NT, because
    // we have to handle traps, we set the timeout on the select as 0,
    // so that we can check for traps quickly
#ifdef BPATCH_LIBRARY
    DWORD milliseconds;
    if (block) milliseconds = INFINITE;
    else milliseconds = 1;
    if (!WaitForDebugEvent(&debugEv, milliseconds))
	return 0;
#else

       if (!WaitForDebugEvent(&debugEv, 1))
	return 0;
#endif

    //printf("Debug event from process %d, tid %d\n", debugEv.dwProcessId,
    //	 debugEv.dwThreadId);

    p = findProcess(debugEv.dwProcessId);
    if (p == NULL) {
	/* 
	   this case can happen when we create a process, but then are
	   unable to parse the symbol table, and so don't complete the
	   creation of the process. We just ignore the event here.  
	*/
	ContinueDebugEvent(debugEv.dwProcessId, debugEv.dwThreadId, 
			   DBG_CONTINUE);
	return 0; 
    }

    switch (debugEv.dwDebugEventCode) {
    case EXCEPTION_DEBUG_EVENT: {
	DWORD exnCode = debugEv.u.Exception.ExceptionRecord.ExceptionCode;
	switch(exnCode) {
	case EXCEPTION_BREAKPOINT: {
	    //printf("Debug breakpoint exception, %d, addr = %x\n", 
	    //       debugEv.u.Exception.ExceptionRecord.ExceptionFlags,
	    //       debugEv.u.Exception.ExceptionRecord.ExceptionAddress);

	    if (!p->reachedFirstBreak) {

		if (!p->hasLoadedDyninstLib && !p->isLoadingDyninstLib) {
		    Address addr = loadDyninstDll(p, p->savedData);
		    p->savedRegs = p->getRegisters();
		    p->changePC(addr);
		    //p->LoadDyninstTrapAddr = addr + 0xd;
		    p->isLoadingDyninstLib = true;
		    break;
		} else if (p->isLoadingDyninstLib) {
		    p->isLoadingDyninstLib = false;
		    p->hasLoadedDyninstLib = true;
		    p->restoreRegisters(p->savedRegs);
		    delete p->savedRegs;
		    p->writeDataSpace((void *)p->getImage()->codeOffset(),
				      LOAD_DYNINST_BUF_SIZE,
				      (void *)p->savedData);
		}

		p->reachedFirstBreak = true;
      
		if (!p->createdViaAttach) {
		    int pid = p->getPid();
		    p->status_ = stopped;

		    if (!p->initDyninstLib()) {
			OS::osKill(pid);
			handleProcessExit(p, -1);
#ifdef BPATCH_LIBRARY
			*status = SIGEM_SIGNALED | SIGTERM;
			return p->getPid();
#else
			return 0;
#endif
		    }

		    string buffer = string("PID=") + string(pid);
		    buffer += string(", passed trap at start of program");
		    statusLine(buffer.string_of());
      
		    buffer=string("PID=") + string(pid) 
			 + ", installing call to DYNINSTinit()";
		    statusLine(buffer.string_of());
		    p->installBootstrapInst();
      
		    // now, let main() and then DYNINSTinit() get invoked.  As it
		    // completes, DYNINSTinit() does a DYNINSTbreakPoint, at which time
		    // we read bootstrap information "sent back" (in a global vrble),
		    // propagate metric instances, do tp->newProgramCallbackFunc(), etc.
		    break;
		} else {
		    //printf("First breakpoint after attach\n");
		    p->pause_();
		    break;
		}
	    }

	    // the process has already been initialized
	    // First lookup in the tramp table, to see if the breakpoint
	    // is from an instrumentation point.

	    // lookup in tramp table
	    Address trampAddr = 0;
	    unsigned u; unsigned k;
	    unsigned key = 
		(unsigned)debugEv.u.Exception.ExceptionRecord.ExceptionAddress;
	    for (u = HASH1(key); 1; u = (u + HASH2(key)) % TRAMPTABLESZ) {
		k = p->trampTable[u].key;
		if (k == 0)
		    break;
		else if (k == key) {
		    trampAddr = p->trampTable[u].val;
		    break;
		}
	    }
	    if (trampAddr) {
		// this is a trap from an instrumentation point
		// change the PC to the address of the base tramp

		for (unsigned i = 0; i < p->threads.size(); i++) {
		    if ((unsigned)p->threads[i]->get_tid()==debugEv.dwThreadId) {
			HANDLE thrH = (HANDLE)p->threads[i]->get_handle();
			CONTEXT cont;
			cont.ContextFlags = CONTEXT_FULL;
			if (!GetThreadContext(thrH, &cont))
			    assert(0);
			cont.Eip = trampAddr;
			if (!SetThreadContext(thrH, &cont))
			    assert(0);
		    }
		}

		break;
	    }

	    // If it is not from an instrumentation point,
	    // it could be from a call to DYNINSTbreakPoint
	    // or an inferior procedure call

	    p->status_ = stopped;
	    p->pause_();
	    int result = p->procStopFromDYNINSTinit();
	    assert(result >= 0 && result <= 2);
	    if (result != 0) {
		if (result == 1) {
		    //p->pause_();
		} else {
		    // procStopFromDYNINSTinit called continueProc()
		    // What should we do here?
		    break;
		}
	    } else if (p->handleTrapIfDueToRPC()) {
		//p->pause_();
		break;
	    } else {
#ifdef BPATCH_LIBRARY /* Don't ignore unknown bkpt in library; leave stopped. */
		*status = SIGEM_STOPPED | SIGTRAP;
#else
		// Unknown breakpoint: we just ignore it
		printf("Debug breakpoint exception, %d, addr = %x\n", 
		       debugEv.u.Exception.ExceptionRecord.ExceptionFlags,
		       debugEv.u.Exception.ExceptionRecord.ExceptionAddress);
		p->status_ = running;
		p->continueProc_();
#endif
	    }
	    break;
	}
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	    //printf("Illegal instruction\n");
	    p->pause_();
	    p->status_ = stopped;
	    if (p->handleTrapIfDueToRPC()) {
		// handleTrapIfDueToRPC calls continueProc()
		break;
	    }
	    p->status_ = running;
	    p->continueProc_();
	    ContinueDebugEvent(debugEv.dwProcessId, debugEv.dwThreadId, 
			       DBG_EXCEPTION_NOT_HANDLED);
	    return 0;
	case EXCEPTION_ACCESS_VIOLATION:
	    printf("Access violation exception, %d, addr = %x\n", 
		   debugEv.u.Exception.ExceptionRecord.ExceptionFlags,
		   debugEv.u.Exception.ExceptionRecord.ExceptionAddress);
	    dumpMem(p, debugEv.u.Exception.ExceptionRecord.ExceptionAddress, 32);

        {
            vector<Address> pcs = p->walkStack( false );
            for( unsigned i = 0; i < pcs.size(); i++ )
            {
                function_base* f = p->findFunctionIn( pcs[i] );
                const char* szFuncName = (f != NULL) ? f->prettyName().string_of() : "<unknown>";
                fprintf( stderr, "%08x: %s\n", pcs[i], szFuncName );
            }
        }
	    //	ContinueDebugEvent(debugEv.dwProcessId, debugEv.dwThreadId, 
	    //			   DBG_EXCEPTION_NOT_HANDLED);
	    //	break;
	default:
	    //printf("exeption %x, addr %x\n", exnCode, 
	    //   debugEv.u.Exception.ExceptionRecord.ExceptionAddress);
	    ContinueDebugEvent(debugEv.dwProcessId, debugEv.dwThreadId, 
			       DBG_EXCEPTION_NOT_HANDLED);
	    return 0;
	}
    } break;

    case CREATE_THREAD_DEBUG_EVENT: {
	//printf("create thread, tid = %d\n", debugEv.dwThreadId);
	assert(p->threads.size() > 0); // main thread should be already defined
	pdThread *t = new pdThread(p, debugEv.dwThreadId,
				   debugEv.u.CreateThread.hThread);
	p->threads += t;
    } break;

    case CREATE_PROCESS_DEBUG_EVENT: {
	//CREATE_PROCESS_DEBUG_INFO info = debugEv.u.CreateProcessInfo;
	//printf("CREATE_PROCESS event: %d\n", debugEv.dwProcessId);
	p = findProcess(debugEv.dwProcessId);
	if (p) {
	    //fprintf(stderr,"create process: base = %x\n", info.lpBaseOfImage);
	    if (p->threads.size() == 0) {
		// define the main thread
		p->threads += new pdThread(p);
	    }
	    p->threads[0]->update_handle(debugEv.dwThreadId, 
					 debugEv.u.CreateProcessInfo.hThread);
	    
	}
    } break;

    case EXIT_THREAD_DEBUG_EVENT: {
	//printf("exit thread, tid = %d\n", debugEv.dwThreadId);
	unsigned nThreads = p->threads.size();
	// start from one to skip main thread
	for (unsigned u = 1; u < nThreads; u++) {
	    if ((unsigned)p->threads[u]->get_tid() == debugEv.dwThreadId) {
		delete p->threads[u];
		p->threads[u] = p->threads[nThreads-1];
		p->threads.resize(nThreads-1);
		break;
	    }
	}
    } break;

    case EXIT_PROCESS_DEBUG_EVENT:
	p = findProcess(debugEv.dwProcessId);
	if (p) {
            char errorLine[1024];
            sprintf(errorLine, "Process %d has terminated with code 0x%x\n", 
                p->getPid(), debugEv.u.ExitProcess.dwExitCode);
            statusLine(errorLine);
            logLine(errorLine);
	    handleProcessExit(p, debugEv.u.ExitProcess.dwExitCode);
	}
#ifdef BPATCH_LIBRARY
	*status = SIGEM_EXITED;
#else
	break;
#endif

    case LOAD_DLL_DEBUG_EVENT: {
	//printf("load dll: hFile=%x, base=%x, debugOff=%x, debugSz=%d lpname=%x, %d\n",
	//       debugEv.u.LoadDll.hFile, debugEv.u.LoadDll.lpBaseOfDll,
	//       debugEv.u.LoadDll.dwDebugInfoFileOffset,
	//       debugEv.u.LoadDll.nDebugInfoSize,
	//       debugEv.u.LoadDll.lpImageName,
	//       /*debugEv.u.LoadDll.fUnicode*/
	//       GetFileSize(debugEv.u.LoadDll.hFile,NULL));

	// We are currently not handling dynamic libraries loaded after the
	// application starts.
	if (p->isBootstrappedYet())
	  break;

	// set the proc handle for the process that is loading the library
	// This is need when we use the imagehelp library to get the
	// symbols for the dll.
	// kludgeProcHandle = (HANDLE)p->getProcFileDescriptor();

        void *addr = debugEv.u.LoadDll.lpImageName;
        int ptr = 0;
	char nameBuffer[MAX_PATH];
	if (addr) {
	    /***
	      addr is a pointer to the image name (in the application address space)
	      There is a problem here, because this pointer can be null and it is
	      null in some cases (in particular, if we attach to a running process
	      it is always null). This means that it can be difficult to find
	      the name of an image. 

	      Currently, if we can't find the name, we just ignore the
	      image.  However, we need to at least be able to parse
	      kernel32.dll, because this library contains some very
	      important functions, including LoadLibrary, which we
	      need to call to load libdyninstRT.dll. We use the following
	      kludge to find if a library is kernel32.dll: the function
	      klugde_isKernel32Dll open a handle to kernel32.dll. We then
	      find if the handle we get from the LOAD_DLL_DEBUG_EVENT
	      is for the same file.
	    ***/

	    p->readDataSpace_(addr, 4, &ptr);
	    //printf("ptr = %x\n", ptr);
	    string kernel32Path;
	    if (ptr) {
		if (debugEv.u.LoadDll.fUnicode) {
		    unsigned short wbuffer[128];
		    p->readDataSpace_((void *)ptr, 128*sizeof(short), wbuffer);
		    WideCharToMultiByte(CP_ACP, 0, wbuffer, 128,
					nameBuffer, 128, NULL, NULL);
		} else {
		    p->readDataSpace_((void *)ptr, 128, nameBuffer);
		}
		//printf("Dll name: %s, base = %x\n", nameBuffer, debugEv.u.LoadDll.lpBaseOfDll);
	    }
	    else if (kludge_isKernel32Dll(debugEv.u.LoadDll.hFile, kernel32Path)) {
	        assert(kernel32Path.length() > 0);
	        assert(kernel32Path.length() < sizeof(nameBuffer));
		strcpy(nameBuffer, kernel32Path.string_of());
	    } else
	        break;
	}

	// try to load symbols for the DLL
        if (!SymLoadModule((HANDLE)p->getProcFileDescriptor(),
			   debugEv.u.LoadDll.hFile,
			   NULL, NULL, 0, 0)) {

		char msgText[1024];

		sprintf( msgText, "SymLoadModule failed for %s: 0x%x\n",
			nameBuffer, GetLastError() );

		logLine(msgText);
	}

	// discover structure of new DLL, and incorporate into our
	// list of known DLLs
	if (addr) {

	    shared_object *so = 
	      new shared_object(string(nameBuffer), 0, false, true, true, 0);
	    assert(p->dyn);
	    p->dyn->sharedObjects += so;
	    if (!p->shared_objects) {
	      p->shared_objects = new vector<shared_object *>;
	    }
	    *(p->shared_objects) += so;
#ifndef BPATCH_LIBRARY
	    tp->resourceBatchMode(true);
#endif 
	    p->addASharedObject(*so);
#ifndef BPATCH_LIBRARY
	    tp->resourceBatchMode(false);
#endif 
	    p->setDynamicLinking();
	  }
      }
    break;
    case UNLOAD_DLL_DEBUG_EVENT:
	// TODO
	//printf("unload dll\n");
	break;
    case OUTPUT_DEBUG_STRING_EVENT:
	//printf("output debug string\n");
	break;
    case RIP_EVENT:
	//printf("rip event\n");
	break;
    default:
	;
	//printf("unknown debug event\n");
	
    }
    
    if (!ContinueDebugEvent(debugEv.dwProcessId, debugEv.dwThreadId, 
			    DBG_CONTINUE)) {
	printf("ContinueDebugEvent failed\n");
	printSysError(GetLastError());
    }

#ifdef BPATCH_LIBRARY
    if (*status)
	return debugEv.dwProcessId;
    else
	return 0;
#else
    return 0;
#endif
}


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
#ifdef notdef
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
#endif
}


bool process::attach() {
    if (createdViaAttach) {
	if (!DebugActiveProcess(getPid())) {
	    //printf("Error: DebugActiveProcess failed\n");
	    return false;
	}
    }
    proc_fd = (int)OpenProcess(PROCESS_ALL_ACCESS, false, getPid());
    if (proc_fd == NULL) {
	//printf("Error: OpenProcess failed\n");
	assert(0);
    }

    void initSymbols(HANDLE procH, const string file, const string dir);
    initSymbols((HANDLE)proc_fd, symbols->file(), "");
    if (createdViaAttach) {
	//
	//void initSymbols(HANDLE procH, const string file, const string dir);
	//initSymbols((HANDLE)proc_fd, symbols->file(), "");
    }
    return true;
}

/* continue a process that is stopped */
bool process::continueProc_() {
    if (hasNewPC) {
      changePC(currentPC_);
      hasNewPC = false;
    }
    for (unsigned u = 0; u < threads.size(); u++) {
	unsigned count = ResumeThread((HANDLE)threads[u]->get_handle());
	if (count == 0xFFFFFFFF) {
	    printSysError(GetLastError());
	    return false;
	}
    }
    return true;
}


#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    OS::osKill(pid);
    handleProcessExit(this, -1);
    return true;
}
#endif


/*
   pause a process that is running
*/
bool process::pause_() {
    for (unsigned u = 0; u < threads.size(); u++) {
	unsigned count = SuspendThread((HANDLE)threads[u]->get_handle());
	if (count == 0xFFFFFFFF) {
	    // printf("pause_: %d\n", threads[u]->get_tid());
	    // printSysError(GetLastError());
	    return false;
	} 
    }
    return true;
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {
    return false;
}


#ifdef BPATCH_LIBRARY
/*
   detach from thr process, continuing its execution if the parameter "cont"
   is true.
 */
bool process::API_detach_(const bool cont)
{
    // XXX Not yet implemented
    assert(false);
    return false;
}
#endif


bool process::dumpCore_(const string) {
    return false;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
    return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
    return writeDataSpace_(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, (void *)inSelf);
}
#endif

bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
    DWORD nbytes;

    //printf("write %d bytes, %x\n", amount, inTraced);

    bool res = WriteProcessMemory((HANDLE)proc_fd, (LPVOID)inTraced, 
				  (LPVOID)inSelf, (DWORD)amount, &nbytes);
    if (!res) {
	char buf[1000];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      buf, 1000, NULL);
	printf(">>> %d %s\n", GetLastError(), buf);
	printf("WriteProcessMem: %d bytes, addr = 0x%lx, %d\n", 
	       amount, inTraced, (int)res);
    }
    return res && (nbytes == amount);
}


bool process::readDataSpace_(const void *inTraced, u_int amount, void *inSelf) {
    DWORD nbytes;
    bool res = ReadProcessMemory((HANDLE)proc_fd, (LPVOID)inTraced, 
				 (LPVOID)inSelf, (DWORD)amount, &nbytes);
    if (!res) {
	char buf[1000];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      buf, 1000, NULL);
	printf(">>> %d %s\n", GetLastError(), buf);
	printf("ReadProcessMem: %d bytes, addr = 0x%lx, %d\n", 
	       amount, inTraced, (int)res);
    }
    return res && (nbytes == amount);
}


bool process::loopUntilStopped() { assert(0); return false; }

float OS::compute_rusage_cpu() { return 0.0; }
float OS::compute_rusage_sys() { return 0.0; }
float OS::compute_rusage_min() { return 0.0; }
float OS::compute_rusage_maj() { return 0.0; }
float OS::compute_rusage_swap() { return 0.0; }
float OS::compute_rusage_io_in() { return 0.0; }
float OS::compute_rusage_io_out() { return 0.0; }
float OS::compute_rusage_msg_send() { return 0.0; }
float OS::compute_rusage_msg_recv() { return 0.0; }
float OS::compute_rusage_sigs() { return 0.0; }
float OS::compute_rusage_vol_cs() { return 0.0; }
float OS::compute_rusage_inv_cs() { return 0.0; }


int getNumberOfCPUs() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}  


void Frame::getActiveFrame(process *p)
{
    CONTEXT cont;

    // we must set ContextFlags to indicate the registers we want returned,
    // in this case, the control registers.
    // The values for ContextFlags are defined in winnt.h
    cont.ContextFlags = CONTEXT_CONTROL;
    if (GetThreadContext((HANDLE)p->threads[0]->get_handle(), &cont)) {
        fp_ = cont.Ebp;
	pc_ = cont.Eip;
	return;
    }
    printSysError(GetLastError());
}


Frame Frame::getCallerFrameNormal(process *p) const
{
    //
    // for the x86, the frame-pointer (EBP) points to the previous frame-pointer,
    // and the saved return address is in EBP-4.
    //
    struct {
	Address fp;
	Address rtn;
    } addrs;

    if (p->readDataSpace((caddr_t)(fp_), sizeof(int)*2,
			 (caddr_t)&addrs, true))
    {
        Frame ret;
        ret.fp_ = addrs.fp;
	ret.pc_ = addrs.rtn;
	return ret;
    }

    return Frame(); // zero frame
}


void *process::getRegisters() {
    CONTEXT *cont = new CONTEXT;
    if (!cont)
	return NULL;
    // we must set ContextFlags to indicate the registers we want returned,
    // in this case, the control registers.
    // The values for ContextFlags are defined in winnt.h
    cont->ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext((HANDLE)threads[0]->get_handle(), cont)) {
	delete cont;
	return NULL;
    }
    return (void *)cont;
}

bool process::changePC(Address addr, const void *savedRegs) {
    assert(status_ == stopped);

    CONTEXT cont = *(CONTEXT *)savedRegs;
    cont.Eip = addr;
    if (!SetThreadContext((HANDLE)threads[0]->get_handle(), &cont)) {
	printf("SethreadContext failed\n");
	return false;
    }
    return true;
}

bool process::changePC(Address addr) {
    assert(status_ == stopped || status_ == neonatal); 
    CONTEXT cont;
    cont.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext((HANDLE)threads[0]->get_handle(), &cont)) {
	printf("GetThreadContext failed\n");
	return false;
    }
    cont.Eip = addr;
    if (!SetThreadContext((HANDLE)threads[0]->get_handle(), &cont)) {
	printf("SethreadContext failed\n");
	return false;
    }
    return true;
}

bool process::restoreRegisters(void *buffer) {
    if (!SetThreadContext((HANDLE)threads[0]->get_handle(), (CONTEXT *)buffer)) {
	//printf("SetThreadContext failed\n");
	return false;
    }
    return true;
}

bool process::isRunning_() const {
    // TODO
    printf("process::isRunning_() returning true\n");
    return true;
}


string process::tryToFindExecutable(const string& iprogpath, int pid) {
  return iprogpath;
}

bool process::set_breakpoint_for_syscall_completion() {
   /* Can assume: (1) process is paused and (2) in a system call.
      We want to set a TRAP for the syscall exit, and do the
      inferiorRPC at that time.  We'll use /proc PIOCSEXIT.
      Returns true iff breakpoint was successfully set. */

    // This is never called on Windows NT
    assert(false);
    return false;
}

void process::clear_breakpoint_for_syscall_completion() { return; }

Address process::read_inferiorRPC_result_register(Register) {
    CONTEXT *cont = new CONTEXT;
    if (!cont)
	return NULL;
    // we must set ContextFlags to indicate the registers we want returned,
    // in this case, the control registers.
    // The values for ContextFlags are defined in winnt.h
    cont->ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext((HANDLE)threads[0]->get_handle(), cont)) {
	//printf("GetThreadContext failed\n");
	delete cont;
	return NULL;
    }
    return cont->Eax;
}

bool process::executingSystemCall() {
   // TODO
   return false;
}

// TODO
bool process::needToAddALeafFrame(Frame , Address &) {
  return false;
}


void initSymbols(HANDLE procH, const string file, const string dir) {
  string searchPath;
  char sysRootDir[MAX_PATH+1];
  if (GetSystemDirectory(sysRootDir, MAX_PATH) == 0)
     assert(0);
  string sysSymsDir = string(sysRootDir) + "\\..\\symbols";
  if (dir.length())
    searchPath = dir + ";";
  searchPath = searchPath + sysSymsDir + ";" + sysSymsDir + "\\dll";
  if (!SymInitialize(procH, (char *)searchPath.string_of(), 0)) {
    fprintf(stderr,"SymInitialize failed, %x\n", GetLastError()); fflush(stderr);
    return;
  }
  if (!SymLoadModule(procH, NULL, (char *)file.string_of(), NULL, 0, 0)) {
    printf("SymLoadModule failed for \"%s\", %x\n",
	    file.string_of(), GetLastError());
    return;
  }
}





/*****************************************************************************
 * forkNewProcess: starts a new process, setting up trace and io links between
 *                the new process and the daemon
 * Returns true if succesfull.
 * 
 * Arguments:
 *   file: file to execute
 *   dir: working directory for the new process
 *   argv: arguments to new process
 *   envp: environment **** not in use
 *   inputFile: where to redirect standard input
 *   outputFile: where to redirect standard output
 *   traceLink: handle or file descriptor of trace link (read only)
 *   ioLink: handle or file descriptor of io link (read only)
 *   pid: process id of new process
 *   tid: thread id for main thread (needed by WindowsNT)
 *   procHandle: handle for new process (needed by WindowsNT)
 *   thrHandle: handle for main thread (needed by WindowsNT)
 ****************************************************************************/
bool forkNewProcess(string &file, string dir, vector<string> argv, 
		    vector<string>envp, string inputFile, string outputFile,
		    int &traceLink, int &ioLink, 
		    int &pid, int &tid, 
		    int &procHandle, int &thrHandle, int /* stdin_fd */, 
		    int /* stdout_fd */, int /* stderr_fd */) {
#ifndef BPATCH_LIBRARY
#ifdef notdef_Pipes     // isn't this all obsolete???
    HANDLE rTracePipe;
    HANDLE wTracePipe;
    HANDLE rIoPipe;
    HANDLE wIoPipe;
    // security attributes to make handles inherited
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = true;
    sa.lpSecurityDescriptor = NULL;
    
    // create trace pipe
    if (!CreatePipe(&rTracePipe, &wTracePipe, &sa, 0)) {
	string msg = string("Unable to create trace pipe for program '") 
	    + File + string("': ") + string(sys_errlist[errno]);
	showErrorCallback(68, msg);
	return(NULL);
    }
    
    // create IO pipe
    // ioPipe is used to redirect the child's stdout & stderr to a pipe which
    // is in turn read by the parent via the process->ioLink socket.
    if (!CreatePipe(&rIoPipe, &wIoPipe, &sa, 0)) {
	string msg = string("Unable to create IO pipe for program '") + File +
	    string("': ") + string(sys_errlist[errno]);
	showErrorCallback(68, msg);
	return(NULL);
    }
    printf("tracepipe = %d\n", (unsigned)wTracePipe);
    // enter trace and IO pipes in child's environment
    SetEnvironmentVariable("PARADYN_TRACE_PIPE", 
			   string((unsigned)wTracePipe).string_of());
    SetEnvironmentVariable("PARADYN_IO_PIPE",
			   string((unsigned)wIoPipe).string_of());
#endif
    //  extern int traceSocket;
    //  SetEnvironmentVariable("PARADYND_TRACE_SOCKET", string((unsigned)traceSocket).string_of());
#endif /* BPATCH_LIBRARY */
    
    // create the child process
    
    string args;
    for (unsigned ai=0; ai<argv.size(); ai++) {
	args += argv[ai];
	args += " ";
    }
    
    STARTUPINFO stinfo;
    memset(&stinfo, 0, sizeof(STARTUPINFO));
    stinfo.cb = sizeof(STARTUPINFO);
    
    PROCESS_INFORMATION procInfo;
    if (CreateProcess(file.string_of(), (char *)args.string_of(), 
		      NULL, NULL, false,
		      DEBUG_PROCESS /* | CREATE_NEW_CONSOLE /* | CREATE_SUSPENDED */,
		      NULL, dir == "" ? NULL : dir.string_of(), 
		      &stinfo, &procInfo)) {
	procHandle = (Word)procInfo.hProcess;
	thrHandle = (Word)procInfo.hThread;
	pid = (Word)procInfo.dwProcessId;
	tid = (Word)procInfo.dwThreadId;
#ifdef notdef_Pipes
        traceLink = (Word)rTracePipe;
        ioLink = (Word)rIoPipe;
        CloseHandle(wTracePipe);
        CloseHandle(wIoPipe);
        //initSymbols((HANDLE)procHandle, file, dir);
#else
        traceLink = -1;
        ioLink = -1;
#endif
	return true;
    }
   
#ifndef BPATCH_LIBRARY
#ifdef notdef_Pipes
    CloseHandle(rTracePipe);
    CloseHandle(wTracePipe);
    CloseHandle(rIoPipe);
    CloseHandle(wIoPipe);
#endif
#endif /* BPATCH_LIBRARY */

    // Output failure message
    LPVOID lpMsgBuf;

    if (FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    GetLastError(),
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	    (LPTSTR) &lpMsgBuf,
	    0,
	    NULL 
	) > 0) {
	char *errorLine = (char *)malloc(strlen((char *)lpMsgBuf) +
					 file.length() + 64);
	if (errorLine != NULL) {
	    sprintf(errorLine, "Unable to start %s: %s\n", file.string_of(),
		    (char *)lpMsgBuf);
	    logLine(errorLine);
	    showErrorCallback(68, (const char *) errorLine);

	    free(errorLine);
	}

	// Free the buffer returned by FormatMsg
	LocalFree(lpMsgBuf);
    } else {
	char errorLine[512];
	sprintf(errorLine, "Unable to start %s: unknown error\n",
		file.string_of());
	logLine(errorLine);
	showErrorCallback(68, (const char *) errorLine);
    }

    return false;
}

char *cplus_demangle(char *c, int) { 
    char buf[1000];
    if (c[0]=='_') {
        // VC++ 5.0 seems to decorate C symbols differently to C++ symbols
        // and the UnDecorateSymbolName() function provided by imagehlp.lib
        // doesn't manage (or want) to undecorate them, so it has to be done
        // manually, removing a leading underscore from functions & variables
        // and the trailing "$stuff" from variables (actually "$Sstuff")
        unsigned i;
        for (i=1; i<sizeof(buf) && c[i]!='$' && c[i]!='\0'; i++)
           { buf[i-1]=c[i]; }
        if (i==1) return 0; // avoid null names which seem to annoy Paradyn
        buf[i-1]='\0';
        return strdup(buf);
      }
    else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_NAME_ONLY)) {
	//printf("Undecorate: %s = %s\n", c, buf);

        // many symbols have a name like foo@4, we must remove the @4
        // just searching for an @ is not enough,
        // as it may occur on other positions. We search for the last one
        // and check that it is followed only by digits.
        char *p = strrchr(buf, '@');
	if (p) {
	  char *q = p+1;
	  strtoul(p+1, &q, 10);
	  if (q > p+1 && *q == '\0') {
	    *p = '\0';
	  }
	}
	return strdup(buf);
    }
    return 0;
}

bool OS::osKill(int pid) {
    bool res;
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (h == NULL) {
	return false;
    }
    res = TerminateProcess(h,0);
    CloseHandle(h);
    return res;
}

#ifdef SHM_SAMPLING
rawTime64 process::getRawCpuTime_hw(int lwp_id) {
  lwp_id = 0;  // to turn off warning for now
  return 0;
}

rawTime64
FILETIME2rawTime64( FILETIME& ft )
{
    return (((rawTime64)(ft).dwHighDateTime<<32) | 
	    ((rawTime64)(ft).dwLowDateTime));
}

/* return unit: nsecs */
rawTime64 process::getRawCpuTime_sw(int lwp_id) {
  FILETIME kernelT, userT, creatT, exitT;
  rawTime64 now;

  if(GetProcessTimes( (HANDLE)getProcFileDescriptor(),
      &creatT, &exitT, &kernelT,&userT)==0) {
    abort();
    return 0;
  }

  // GetProcessTimes returns values in 100-ns units
  now = (FILETIME2rawTime64(userT)+FILETIME2rawTime64(kernelT));

  // time shouldn't go backwards, but we'd better handle it if it does
  if (now < previous) {
     logLine("********* time going backwards in paradynd **********\n");
     now = previous;
  }
  else {
     previous = now;
  }

  return now;
}
#endif // SHM_SAMPLING

fileDescriptor *getExecFileDescriptor(string filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

#ifndef BPATCH_LIBRARY
void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   timeUnit(fraction(100)), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "DYNINSTgetCPUtime_sw");
}
#endif
