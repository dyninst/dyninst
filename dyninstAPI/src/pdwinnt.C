/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: pdwinnt.C,v 1.111 2003/10/22 16:00:52 schendel Exp $

#include "common/h/std_namesp.h"
#include <iomanip>
#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/signalhandler.h"
#include <psapi.h>

#ifndef BPATCH_LIBRARY
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h" //SPLIT ccw 4 jun 2002
#endif

#ifdef BPATCH_LIBRARY
/* XXX This is only needed for emulating signals. */
#include "BPatch_thread.h"
#include "nt_signal_emul.h"
#endif

#include "dyninstAPI/src/rpcMgr.h"

// prototypes of functions used in this file
static pdstring GetLoadedDllImageName( process* p, const DEBUG_EVENT& ev );


void InitSymbolHandler( HANDLE hProcess );
void ReleaseSymbolHandler( HANDLE hProcess );

//ccw  27 july 2000 : dummy methods to get the thing to compile before i add
//the remoteDevice : 29 mar 2001
#ifdef mips_unknown_ce2_11
#include <stdio.h> //for wprintf

void kludgeWCHAR(const char *str8, WCHAR *str16){
	int k;

	k=0;
	while(str8[k]!='\0'){
		str16[k] = str8[k];
		k++;
	}
	str16[k] = '\0';
	str16[k+1]='\0';

}

//int GetThreadContext(HANDLE hThread, w32CONTEXT *lpContext){
//	return 0;
//}

//int SetThreadContext(HANDLE hThread, const w32CONTEXT *lpContext){
//	return 0;
//}
#endif

extern bool isValidAddress(process *proc, Address where);

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
static bool kludge_isKernel32Dll(HANDLE fileHandle, pdstring &kernel32Name) {
    static DWORD IndxHigh, IndxLow;
    static bool firstTime = true;
    BY_HANDLE_FILE_INFORMATION info;
    static pdstring kernel32Name_;

    if (firstTime) {
       HANDLE kernel32H;
       firstTime = false;
       char sysRootDir[MAX_PATH+1];
       if (GetSystemDirectory(sysRootDir, MAX_PATH) == 0)
          assert(0);
       kernel32Name_ = pdstring(sysRootDir) + "\\kernel32.dll";
       kernel32H = CreateFile(kernel32Name_.c_str(), GENERIC_READ, 
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

    if (f = p->findFuncByAddr((Address)addr))
    {
        printf("Function %s, addr=0x%lx, sz=%d\n", 
                f->prettyName().c_str(),
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



//
// walkStackFrame
//
// Try to walk one more frame in the stack of the indicated process.
// Assumes that the STACKFRAME argument is either the result
// from a previous StackWalk call, or has been initialized 
// to the context from a GetThreadContext call.
//
inline
BOOL
walkStackFrame( HANDLE hProc, HANDLE hThread, STACKFRAME* psf )
{
	return StackWalk( IMAGE_FILE_MACHINE_I386,
                        hProc,
                        hThread,
		                psf,
                        NULL,
		                NULL,
                        SymFunctionTableAccess,
                        SymGetModuleBase,
		                NULL);
}


#ifdef i386_unknown_nt4_0 //ccw 27 july 2000 : 29 mar 2001
//
// process::walkStackFromFrame
//
// 8OCT02: this should now pay proper attention to threads,
//         though a better method than handing in a thread to
//         kickstart the stackwalk is probably a good idea -- Bernat
//
// Note that we have *not* been able to find a mechanism that 
// can perform stack walks reliably for apps compiled with the FPO 
// (frame pointer omission) optimization.  Even if the FPO data is
// available, the Win32 StackWalk call either skips functions or bottoms
// out the stack too early.  Unfortunately, this means that we require
// that the app under study *not* use the FPO optimization.  (Specify /Oy- 
// for the VC++ compiler to turn this optimization off.)
//

bool process::walkStackFromFrame(Frame currentFrame, pdvector<Frame> &stackWalk)
{
#ifndef BPATCH_LIBRARY
    startTimingStackwalk();
#endif
    
#ifdef DEBUG_STACKWALK
    cout << "\n<stack>" << endl;
#endif // DEBUG_STACKWALK
    
    if (status_ == running) {
        cerr << "Error: stackwalk attempted on running process" << endl;
#ifndef BPATCH_LIBRARY
      stopTimingStackwalk();
#endif
      return false;
    }
    
    // establish the current execution context
    CONTEXT cont;
    HANDLE hThread = (HANDLE)(currentFrame.getLWP()->get_fd());
    
    cont.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(hThread, &cont))
    {
        cerr << "walkStack: GetThreadContext failed:" << GetLastError()
             << endl;

#ifndef BPATCH_LIBRARY
	stopTimingStackwalk();
#endif
        return false;
    }

    STACKFRAME sf;
    ZeroMemory( &sf, sizeof(STACKFRAME) );
    sf.AddrPC.Offset = cont.Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = cont.Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Offset = cont.Esp;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrReturn.Mode = AddrModeFlat;

    // walk the stack, frame by frame
    // we use the Win32 StackWalk function to automatically
    // handle compiler optimizations, especially FPO optimizations
    bool done = false;

    while( !done ) {
       STACKFRAME saved_sf = sf;
       BOOL walked;
       ADDRESS patchedAddrReturn;
       ADDRESS patchedAddrPC;
       instPoint* ip = NULL;
       function_base* fp = NULL;
       dyn_lwp *replwp = getRepresentativeLWP();
       
       // set defaults for return address and PC
       patchedAddrReturn = saved_sf.AddrReturn;
       patchedAddrPC = saved_sf.AddrPC;
       
       handleT procHandle = replwp->getProcessHandle();
       
       // try to step through the stack using the current information
       walked = walkStackFrame((HANDLE)procHandle, hThread, &sf);
       
       if( !walked && (GetLastError() == ERROR_INVALID_ADDRESS) ) {
          
          // try to patch the return address, in case it is outside
          // of the original text of the process.  It might be outside
          // the original text if the function has been relocated, or
          // if it represents a return from a call instruction that
          // has been relocated to a base tramp.
          // we first try to patch the return address only, because it
          // is most likely that the return address only is out of the
          // original text space.  Once we process the first stack
          // frame, it appears that the StackWalk function transfers 
          // the return address from the STACKFRAME struct to the PC,
          // so once we've gotten a valid return address in a 
          // STACKFRAME, the PC will be within the original text after
          // the StackWalk
          sf = saved_sf;
          fp = (function_base *)findFuncByAddr(sf.AddrReturn.Offset);
          
          if( fp != NULL ) {
      	    // because StackWalk seems to support it, we simply use 
       	    // the address of the function itself rather than
       	    // trying to do the much more difficult task of finding
       	    // the original address of the relocated instruction
       	    patchedAddrReturn.Offset = fp->addr();
       	    sf.AddrReturn = patchedAddrReturn;
          }
          
          // retry the stack step
          walked = walkStackFrame((HANDLE)replwp->getProcessHandle(),
                                  hThread, &sf);
       }
       
       if( !walked && (GetLastError() == ERROR_INVALID_ADDRESS) ) {
          
          // patching the return address alone didn't work.
          // try patching the return address and the PC
          sf = saved_sf;
          fp = (function_base *)findFuncByAddr(sf.AddrPC.Offset);
          
          if( fp != NULL ) {
             
       	    // because StackWalk seems to support it, we simply use 
       	    // the address of the function itself rather than
       	    // trying to do the much more difficult task of finding
       	    // the original address of the relocated instruction
       	    patchedAddrPC.Offset = fp->addr();
       	    sf.AddrPC = patchedAddrPC;
          }
          
          // use the patched return address we calculated above
          sf.AddrReturn = patchedAddrReturn;

          // retry the stack step
          walked = walkStackFrame((HANDLE)getRepresentativeLWP()->getProcessHandle(),
                                  hThread, &sf);
       }
       
       if( !walked && (GetLastError() == ERROR_INVALID_ADDRESS) )
       {
          
          // patching both addresses didn't work
          //
          // try patching the PC only
          sf = saved_sf;
          sf.AddrPC = patchedAddrPC;
          
          // retry the stack step
          walked = walkStackFrame((HANDLE)replwp->getProcessHandle(),
                                  hThread, &sf);
       }
       

       // by now we've tried all of our tricks to handle the stack 
       // frame if we haven't succeeded by now, we're not going to be
       // able to handle this frame
       if( walked ) {
          
          Address pc = NULL;
	  
          // save the PC for this stack frame
          // make sure we use the original address in case
          // it was outside the original text of the process
          if( saved_sf.AddrReturn.Offset == 0 ) {
             
             // this was the first stack frame, so we had better
             // use the original PC
             pc = saved_sf.AddrPC.Offset;
             
          } else {
             
             // this was not the first stack frame
             // use the original return address
             pc = saved_sf.AddrReturn.Offset;
          }
          
          stackWalk.push_back(Frame(pc, 0, getPid(), 
                                    currentFrame.getThread(), 
                                    currentFrame.getLWP(), 
                                    false));
          
#ifdef DEBUG_STACKWALK
          cout << "0x" << setw(8) << setfill('0') << std::hex << pc << ": ";
          
          if( fp != NULL ) {
             cout << fp->prettyName();
          } else {
             cout << "<unknown>";
          }
          
          if( sf.AddrPC.Offset != pc ) {
             cout << " (originally 0x" << setw(8) << setfill('0') 
                  << std::hex << sf.AddrPC.Offset << ")";
          }
          cout << endl;
#endif
          
       } else {
          // we tried everything we know to recover - we'll have 
          // to fail
          done = true;
       }
       
    }

    // Terminate stack walk with an empty frame
    stackWalk.push_back(Frame());
#ifndef BPATCH_LIBRARY
    stopTimingStackwalk();
#endif

    return true;
}
#endif

#if defined(mips_unknown_ce2_11) //ccw 6 feb 2001 : 29 mar 2001
//ccw 6 feb 2001 : windows CE does not have the NT walkStack function
//so we use this one.

bool process::walkStackFromFrame(Frame currentFrame, pdvector<Frame> &stackWalk)
{
    if (status_ == running) {
        cerr << "Error: stackwalk attempeded on running process" << endl;
        return false;
    }  
    Address spOld = 0xffffffff;
    
    while (!currentFrame.isLastFrame(this)) {
        Address spNew = currentFrame.getSP(); // ccw 6 feb 2001 : should get SP?
        
        // successive frame pointers might be the same (e.g. leaf functions)
        if (spOld < spNew) {
            return false;            
        }
        
        spOld = spNew;
        
        Address next_pc = currentFrame.getPC();
        stackWalk.push_back(currentFrame);
        
        //ccw 6 feb 2001 : at this point, i need to use the 
        //list of functions parsed from the debug symbols to
        //determine the frame size for each function and find the 
        //return value for each (which is the previous fir value)
        currentFrame = currentFrame.getCallerFrame(this); 
        
    }    
    stackWalk.push_back(currentFrame);
    return true;    
}
#endif


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
    return 0;
}

// osTraceMe is not needed in Windows NT
void OS::osTraceMe(void) {}

// Default continue type (modified by handlers)
DWORD continueType = DBG_CONTINUE; //ccw 25 oct 2000 : 28 mar 2001

// Breakpoint handler
DWORD handleBreakpoint(process *proc,
                       procSignalInfo_t info) {
    Address addr = (Address) info.u.Exception.ExceptionRecord.ExceptionAddress;
/*    
      printf("Debug breakpoint exception, %d, addr = %x\n", 
      info.u.Exception.ExceptionRecord.ExceptionFlags,
      info.u.Exception.ExceptionRecord.ExceptionAddress);
*/
              
    if (!proc->reachedBootstrapState(bootstrapped)) {
        // If we're attaching, the OS sends a stream of debug events
        // to the mutator informing it of the status of the process.
        // This finishes with a breakpoint that basically says
        // "We're done, have fun". Check for that here.
        if (proc->wasCreatedViaAttach() &&
            !proc->reachedBootstrapState(initialized)) {
            proc->setBootstrapState(initialized);
            return DBG_CONTINUE;
        }
        
        // Wait for the process to hit main before we start
        // doing things to it (creation only)
        if(!proc->reachedBootstrapState(begun)) {
            proc->setBootstrapState(begun);
            proc->insertTrapAtEntryPointOfMain();
            proc->continueProc();
            return DBG_CONTINUE;
        }
    
        // When we hit main, cleanup and set init state
        if (proc->trapAtEntryPointOfMain(addr)) {
            proc->handleTrapAtEntryPointOfMain(); // cleanup
            proc->setBootstrapState(initialized);
            return DBG_CONTINUE;
    }
    
        
        // When dyninst lib load finishes, cleanup
        if (proc->dyninstlib_brk_addr &&
            (proc->dyninstlib_brk_addr == addr)) {
            proc->dyninstlib_brk_addr = 0;
            proc->loadDYNINSTlibCleanup();
            proc->setBootstrapState(loadedRT);
            return DBG_CONTINUE;
        }
    }

    // Hitting a base tramp?
    
#if !defined( mips_unknown_ce2_11 )//ccw 26 july 2000 : 29 mar 2001
    Address trampAddr = 0;
    unsigned u; unsigned k;
    unsigned key = addr;
    for (u = HASH1(key); 1; u = (u + HASH2(key)) % TRAMPTABLESZ) {
        k = proc->trampTable[u].key;
        if (k == 0)
            break;
        else if (k == key) {
            trampAddr = proc->trampTable[u].val;
            break;
        }
    }
    if (trampAddr) {
        // this is a trap from an instrumentation point
        
        // find the current thread
        dyn_thread* currThread = NULL;
        for(unsigned int i = 0; i < proc->threads.size(); i++)
        {
            if ((unsigned)proc->threads[i]->get_tid() == info.dwThreadId)
            {
                currThread = proc->threads[i];
                break;
            }
        }
        assert( currThread != NULL );
        
        // Due to a race between the processing of trap debug events
        // and the desire to run an inferior RPC, it is possible that
        // we hit the trap, set ourselves up to run an inferior RPC,
        // and then processed the trap notification.  If this happens,
        // we don't end up running *any* of the inferior RPC code.
        //
        // We can tell that this is what's happened based on the
        // thread's Eip - if it doesn't match the ExceptionAddress, we
        // know that we had tried to reset the Eip to execute an inferior
        // RPC.  In that case, we leave the Eip alone here, which means
        // the inferior RPC code will execute when we continue the 
        // thread.
        // We have to remember that we need to execute this 
        // instrumentation once the inferior RPC is done, however.
        
        CONTEXT cont;
        cont.ContextFlags = CONTEXT_FULL;
        if( !GetThreadContext( (HANDLE)currThread->get_lwp()->get_fd(), &cont ) )
            assert(0 && "Failed to get thread context");
        if( addr == Address(cont.Eip - 1) )
        {
            // The Eip indicates we've just executed a trap instruction
            // reset the Eip to the address of the base tramp
            cont.Eip = trampAddr;
            if( !SetThreadContext( (HANDLE)currThread->get_lwp()->get_fd(), &cont ) )
                assert(0 && "Failed to set thread context");
        }
        else {
            // We leave the Eip alone, since we've set it to execute 
            // at inferior RPC code.  However, we need to remember that
            // we should execute the base tramp once the inferiorRPC is
            // completed.
            currThread->set_pending_tramp_addr( trampAddr );
        }
        // We're actually in instrumentation, so rerun the 
        // process
        proc->continueProc();
        return DBG_CONTINUE;
    } // if trampAddr         
#endif // mips_unknown_ce2_11
    
    // If it is not from an instrumentation point,
    // it could be from a call to DYNINSTbreakPoint,
    // and so we leave paused

    return DBG_CONTINUE;
}

DWORD handleIllegal(process *proc, procSignalInfo_t info) {

    Address addr = (Address) info.u.Exception.ExceptionRecord.ExceptionAddress;
    
    if( proc->getRpcMgr()->handleSignalIfDueToIRPC() )
    {
        // handleTrapIfDueToRPC calls continueproc()
        // however, under Windows NT, it doesn't actually 
        // continue the thread until the ContinueDbgEvent call is made
        
        // We take advantage of this fact to ensure that any
        // pending instrumentation is executed.  (I.e., instrumentation
        // put off so we could be sure to execute inferior RPC code is
        // now executed.)
        if( !proc->getRpcMgr()->existsRunningIRPC() )	{
            // we finished the inferior RPC, so we can now execute
            // any pending instrumentation
            
            // find the current thread
            dyn_thread* currThread = NULL;
            for( unsigned int i = 0; i < proc->threads.size(); i++ ) {
                if ((unsigned)proc->threads[i]->get_tid() == 
                    info.dwThreadId) {
                    currThread = proc->threads[i];
                    break;
                }
            }
            assert( currThread != NULL );
            
            Address pendingTrampAddr = currThread->get_pending_tramp_addr();
            if( pendingTrampAddr) {
                // reset the Eip to the pending instrumentation
                CONTEXT ctxt;
                ctxt.ContextFlags = CONTEXT_FULL;
                if( !GetThreadContext( (HANDLE)currThread->get_lwp()->get_fd(), &ctxt ) )
                    assert(0 && "Failed to get thread context");
                ctxt.Eip = pendingTrampAddr;
                if( !SetThreadContext( (HANDLE)currThread->get_lwp()->get_fd(), &ctxt ) )
                    assert(0 && "Failed to set thread context");
                currThread->set_pending_tramp_addr(0);
            }
        }
        // Inferior RPC states whether to leave running or paused
        return DBG_CONTINUE;
    }
    proc->continueProc();
    return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD handleViolation(process *proc, procSignalInfo_t info) {
    /*
	  printf("Access violation exception, %d, addr = %08x\n", 
      info.u.Exception.ExceptionRecord.ExceptionFlags,
      info.u.Exception.ExceptionRecord.ExceptionAddress);
    */
    dumpMem(proc, info.u.Exception.ExceptionRecord.ExceptionAddress, 32);
    
    {
        
#ifndef mips_unknown_ce2_11 //ccw 6 feb 2001 : 29 mar 2001
        // Should walk stacks for other threads as well
        pdvector<pdvector<Frame> > stackWalks;
        proc->walkStacks(stackWalks);
        for (unsigned walk_iter = 0; walk_iter < stackWalks.size(); walk_iter++)
            for( unsigned i = 0; i < stackWalks[walk_iter].size(); i++ )
            {
                function_base* f = proc->findFuncByAddr( stackWalks[walk_iter][i].getPC() );
                const char* szFuncName = (f != NULL) ? f->prettyName().c_str() : "<unknown>";
                fprintf( stderr, "%08x: %s\n", stackWalks[walk_iter][i].getPC(), szFuncName );
            }
#endif
        
    }
    proc->continueProc();
    return DBG_EXCEPTION_NOT_HANDLED;
}


// Exception dispatcher
DWORD handleException(process *proc, procSignalWhat_t what, procSignalInfo_t info) {
    DWORD ret = DBG_EXCEPTION_NOT_HANDLED;

    switch (what) {
  case EXCEPTION_BREAKPOINT: 
      ret = handleBreakpoint(proc, info);
      break;
  case EXCEPTION_ILLEGAL_INSTRUCTION:
      ret = handleIllegal(proc, info);
      break;
  case EXCEPTION_ACCESS_VIOLATION:
      ret = handleViolation(proc, info);
      break;
  default:
      break;
    }
    return ret;
}

dyn_lwp *process::createRepresentativeLWP() {
   // I'm not sure if the initial lwp should be in the real_lwps array.  Is
   // this lwp more like a representative lwp or more like the linux initial
   // lwp?

   // don't register the representativeLWP in the real_lwps since it's not a
   // true lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

// Thread creation
DWORD handleThreadCreate(process *proc, procSignalInfo_t info) {
   dyn_lwp *l = proc->createFictionalLWP(info.dwThreadId);
   l->setFileHandle(info.u.CreateThread.hThread);
   l->attach();
   dyn_thread *t = new dyn_thread(proc, info.dwThreadId, // thread ID
                                  proc->threads.size(), // POS in threads array (and rpcMgr thrs_ array?)
                                  l); // dyn_lwp object for thread handle
   proc->threads.push_back(t);
   proc->continueProc();
   return DBG_CONTINUE;
}

// Process creation
DWORD handleProcessCreate(process *proc, procSignalInfo_t info) {
   if(! proc)
      return DBG_CONTINUE;

   dyn_lwp *rep_lwp = proc->getRepresentativeLWP();
   assert(rep_lwp);  // the process based lwp should already be set

   if(! rep_lwp->isFileHandleSet()) {
      rep_lwp->setFileHandle(info.u.CreateProcessInfo.hThread);
      // the real lwp id is at info.dwThreadId if want to save around
   }

   if (proc->threads.size() == 0) {
      dyn_thread *t = new dyn_thread(proc, info.dwThreadId, // thread ID,
                                     0, // POS (main thread is always 0)
                                     rep_lwp);
      // define the main thread
      proc->threads.push_back(t);
   }
   else {
      proc->threads[0]->update_tid(info.dwThreadId);
      proc->threads[0]->update_lwp(rep_lwp);
   }

   
   if( proc->getImage()->getObject().have_deferred_parsing() )
   {
      fileDescriptor_Win* oldDesc = 
         proc->getImage()->getObject().GetDescriptor();
      
      // now we have an process to work with -
      // build a new descriptor with the new information
      fileDescriptor_Win* desc = new fileDescriptor_Win( *oldDesc );
      
      // update the descriptor with the new information
      desc->SetAddr( (Address)info.u.CreateProcessInfo.lpBaseOfImage );
      desc->SetFileHandle( info.u.CreateProcessInfo.hFile );
      // 7APR -- when we attach we get a process handle after oldDesc was created
      if(! rep_lwp->isProcessHandleSet()) {
         desc->SetProcessHandle( info.u.CreateProcessInfo.hProcess );
         rep_lwp->setProcessHandle( info.u.CreateProcessInfo.hProcess );
      } else {
         desc->SetProcessHandle(rep_lwp->getProcessHandle());
      }
      
      fileDescriptor_Win *ptr = dynamic_cast<fileDescriptor_Win*>(desc);

      // reparse the image with the updated descriptor
      image* img = image::parseImage( desc );
      proc->setImage( img );
      // We had originally inserted it at 0 -- move it over
      proc->deleteCodeRange(0);
      proc->addCodeRange(img->codeOffset(), img);
      
   }

   proc->continueProc();

   return DBG_CONTINUE;
}

DWORD handleThreadExit(process *proc, procSignalInfo_t info) {
    printf("exit thread, tid = %d\n", info.dwThreadId);
    unsigned nThreads = proc->threads.size();
    // start from one to skip main thread
    for (unsigned u = 1; u < nThreads; u++) {
        if ((unsigned)proc->threads[u]->get_tid() == info.dwThreadId) {
            delete proc->threads[u];
            proc->threads[u] = proc->threads[nThreads-1];
            proc->threads.resize(nThreads-1);
            break;
        }
    }
    proc->continueProc();
    return DBG_CONTINUE;
}

DWORD handleProcessExit(process *proc, procSignalInfo_t info) {
    if (proc) {
        char errorLine[1024];
        sprintf(errorLine, "Process %d has terminated with code 0x%x\n", 
                proc->getPid(), info.u.ExitProcess.dwExitCode);
          statusLine(errorLine);
          logLine(errorLine);
          proc->handleProcessExit(info.u.ExitProcess.dwExitCode);
    }
    proc->continueProc();
    return DBG_CONTINUE;
}

DWORD handleDllLoad(process *proc, procSignalInfo_t info) {
/*
      printf("load dll: hFile=%x, base=%x, debugOff=%x, debugSz=%d lpname=%x, %d\n",
      info.u.LoadDll.hFile, info.u.LoadDll.lpBaseOfDll,
      info.u.LoadDll.dwDebugInfoFileOffset,
      info.u.LoadDll.nDebugInfoSize,
      info.u.LoadDll.lpImageName,
             info.u.LoadDll.fUnicode,
      GetFileSize(info.u.LoadDll.hFile,NULL));
*/
    // This is NT's version of handleIfDueToSharedObjectMapping
    
    // Hacky hacky: after the Paradyn RT lib is loaded, skip further
    // parsings. 
    
    // obtain the name of the DLL
    pdstring imageName = GetLoadedDllImageName( proc, info );
    
    handleT procHandle = proc->getRepresentativeLWP()->getProcessHandle();

    fileDescriptor* desc =
       new fileDescriptor_Win(imageName, (HANDLE)procHandle,
                              (Address)info.u.LoadDll.lpBaseOfDll,
                              info.u.LoadDll.hFile );
    
    // discover structure of new DLL, and incorporate into our
    // list of known DLLs
    if (imageName.length() > 0) {
        shared_object *so = new shared_object( desc,
                                                false,
                                                true,
                                                true,
                                                0 );
        assert(proc->dyn);
        proc->dyn->sharedObjects.push_back(so);
        if (!proc->shared_objects) {
            proc->shared_objects = new pdvector<shared_object *>;
        }
        (*(proc->shared_objects)).push_back(so);
        proc->addCodeRange((Address) info.u.LoadDll.lpBaseOfDll,
                           so);
        
#ifndef BPATCH_LIBRARY
        tp->resourceBatchMode(true);
#endif 
        proc->addASharedObject(so,(Address) info.u.LoadDll.lpBaseOfDll); //ccw 20 jun 2002
#ifndef BPATCH_LIBRARY
        tp->resourceBatchMode(false);
#endif 
        proc->setDynamicLinking();
        
        char dllFilename[_MAX_FNAME];
        _splitpath(imageName.c_str(),
                   NULL, NULL, dllFilename, NULL);
        
        // See if there is a callback registered for this library
        proc->runLibraryCallback(pdstring(dllFilename), so);
        
    }
    
    // WinCE used to check for "coredll.dll" here to see if the process
    // was initialized, this should have been fixed by inserting a trap
    // at the entry of main -- bernat, JAN03
    proc->continueProc();
    return DBG_CONTINUE;
}

// ccw 2 may 2001 win2k fixes
// when you launch a process to be debugged with win2k (as in createProcess)
// the system sends you back at least two debug events before starting the
// process.  a debug event is also sent back for every dll that is loaded
// prior to starting main(), these include ntdll.dll and kernel32.dll and any
// other dlls the process needs that are not loaded with an explicit call
// to LoadLibrary().
//
// dyninst catches the first debug event (CREATE_PROCESS) and initializes
// various process specific data structures.  dyninst catches the second
// debug event (an EXCEPTION_DEBUG_EVENT) and used this event as a trigger to
// put in the bit of code that forced the mutatee to load
// libdyninstAPI_RT.dll.  In win2k, this does not work.  The bit of code is
// run and the trailing DebugBreak is caught and handled but the Dll will not
// be loaded.  The EXCEPTION_DEBUG_EVENT must be handled and continued from
// before LoadLibrary will perform correctly.
//
// the fix for this is to handle this EXCEPTION_DEBUG_EVENT, and put a
// DebugBreak (0xcc) at the beginning of main() in the mutatee.  catching
// that DebugBreak allows dyninst to write in the bit of code used to load
// the libdyninstAPI_RT.dll.
//
// after this, dyninst previously instrumented the mutatee to force the
// execution of DYNINSTinit() in the dll.  in order to take out this bit of
// complexity, the DllMain() function in the dll, which is run upon loading
// the dll, is used to automatically call DYNINSTinit().
//
// DYNINSTinit() takes two parameters, a flag denoting how dyninst attached
// to this process and the pid of the mutator.  These are passed from the
// mutator to the mutatee by finding a variable in the dll and writing the
// correct values into the mutatee's address space.  When a Dll is loaded, a
// LOAD_DLL debug event is thrown before the execution of DllMain(), so
// dyninst catches this event, writes the necessary values into the mutatee
// memory, then lets DllMain() call DYNINSTinit().  the DebugBreak() at the
// end of DYNINSTinit() is now removed for NT/win2K
//
// the bit of code inserted to load the dll fires a DebugBreak() to signal
// that it is done. dyninst catches this, patches up the code that was used
// to load the dll, replaces what was overwritten in main() and resets the
// instruction pointer (EIP) to the beginning of main().

#ifdef mips_unknown_ce2_11
int secondDLL = 0; //ccw 24 oct 2000 : 28 mar 2001
#endif

int handleProcessEvent(process *proc,
                       procSignalWhy_t why,
                       procSignalWhat_t what,
                       procSignalInfo_t info) {
    DWORD ret = DBG_EXCEPTION_NOT_HANDLED;
    // Process is paused
    // Make sure pause does something...
    proc->savePreSignalStatus();
    // Due to NT's odd method, we have to call pause_
    // directly (a call to pause returns without doing anything
    // pre-initialization)
    proc->getRepresentativeLWP()->stop_();
    proc->set_status(stopped);

    switch(why) {
  case procException:
      ret = handleException(proc, what, info);
      break;
  case procThreadCreate:
      ret = handleThreadCreate(proc, info);
      break;
  case procProcessCreate:
      ret = handleProcessCreate(proc, info);
      break;
  case procThreadExit:
      ret = handleThreadExit(proc, info);
      break;
  case procProcessExit:
      ret = handleProcessExit(proc, info);
      break;
  case procDllLoad:
      
      ret = handleDllLoad(proc, info);
      break;
  default:
      break;
    }
    // Continue the process after the debug event
    
#if defined( mips_unknown_ce2_11 )//ccw 28 july 2000 : 29 mar 2001
    if (!BPatch::bpatch->rDevice->RemoteContinueDebugEvent(info.dwProcessId, info.dwThreadId, 
                                                           ret))
#else
    if (!ContinueDebugEvent(info.dwProcessId, info.dwThreadId, ret))
#endif
    {
        
        DebugBreak();
        printf("ContinueDebugEvent failed\n");
        printSysError(GetLastError());
    }
    return (ret == DBG_CONTINUE);
}

// the pertinantLWP and wait_options are ignored on Solaris, AIX

process *decodeProcessEvent(dyn_lwp **pertinantLWP, int wait_arg, 
                            procSignalWhy_t &why, procSignalWhat_t &what,
                            procSignalInfo_t &info, bool block,
                            int wait_options)
{
    process *proc;
    
    // We wait for 1 millisecond here. On the Unix platforms, the wait
    // happens on the select in controllerMainLoop. But on NT, because
    // we have to handle traps, we set the timeout on the select as 0,
    // so that we can check for traps quickly

    DWORD milliseconds;
    if (block) milliseconds = INFINITE;
    else milliseconds = 1;

#if defined(mips_unknown_ce2_11) //ccw 28 july 2000 : 29 mar 2001
    if (!BPatch::bpatch->rDevice->RemoteWaitForDebugEvent(&info, milliseconds))
#else
    if (!WaitForDebugEvent(&info, milliseconds))
#endif    
        return NULL;
   
    proc = process::findProcess(info.dwProcessId);
    if (proc == NULL) {
        /* 
           this case can happen when we create a process, but then are
           unable to parse the symbol table, and so don't complete the
           creation of the process. We just ignore the event here.  
        */
        
#if defined(mips_unknown_ce2_11) //ccw 28 july 2000 : 29 mar 2001
        BPatch::bpatch->rDevice->RemoteContinueDebugEvent(info.dwProcessId, info.dwThreadId, 
                                                          DBG_CONTINUE);
#else
		ContinueDebugEvent(info.dwProcessId, info.dwThreadId, 
                           DBG_CONTINUE);
        
#endif
        return NULL; 
    }
    
    switch (info.dwDebugEventCode) {
  case EXCEPTION_DEBUG_EVENT:
      why = procException;
      what = info.u.Exception.ExceptionRecord.ExceptionCode;
      break;
  case CREATE_THREAD_DEBUG_EVENT:
      
      why = procThreadCreate;
      break;
  case CREATE_PROCESS_DEBUG_EVENT:
      
      why = procProcessCreate;
      break;
  case EXIT_THREAD_DEBUG_EVENT:
      why = procThreadExit;
      break;
  case EXIT_PROCESS_DEBUG_EVENT:
      why = procProcessExit;
      what = info.u.ExitProcess.dwExitCode;
      break;
  case LOAD_DLL_DEBUG_EVENT:
      why = procDllLoad;
      break;
  default:
      procUndefined;
      break;
    }
    
    return proc;
    
}    

// Global interface
void decodeAndHandleProcessEvent(bool block) {
    procSignalWhy_t why;
    procSignalWhat_t what;
    procSignalInfo_t info;
    process *proc;
    dyn_lwp *selectedLWP;

    proc = decodeProcessEvent(&selectedLWP, -1, why, what, info, block, 0);
    if (!proc) return;
    handleProcessEvent(proc, why, what, info);
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

bool process::installSyscallTracing()
{
    return true;
}

/* continue a process that is stopped */
bool dyn_lwp::continueLWP_() {
   unsigned count = 0;
#ifdef mips_unknown_ce2_11 //ccw 10 feb 2001 : 29 mar 2001
   count = BPatch::bpatch->rDevice->RemoteResumeThread((HANDLE)get_fd());
#else
   count = ResumeThread((HANDLE)get_fd());
#endif
      
   if (count == 0xFFFFFFFF) {
      printSysError(GetLastError());
      return false;
   } else
      return true;
}


/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    OS::osKill(pid);
    this->handleProcessExit(-1);
    return true;
}


/*
   pause a process that is running
*/
bool dyn_lwp::stop_() {
   unsigned count = 0;
#ifdef mips_unknown_ce2_11 //ccw 10 feb 2001 : 29 mar 2001
   count = BPatch::bpatch->rDevice->RemoteSuspendThread((HANDLE)get_fd());
#else            
   count = SuspendThread((HANDLE)get_fd());
#endif
   if (count == 0xFFFFFFFF) {
      // printf("pause_: %d\n", threads[u]->get_tid());
      // printSysError(GetLastError());
      return false;
   }  else
      return true;
}


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


bool process::dumpCore_(const pdstring) {
    return false;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
    return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
    return writeDataSpace_(inTraced, amount, inSelf);
}

bool process::flushInstructionCache_(void *baseAddr, size_t size){ //ccw 25 june 2001
   dyn_lwp *replwp = getRepresentativeLWP();
	return FlushInstructionCache((HANDLE)replwp->getProcessHandle(),
                                baseAddr, size);
}

//#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, (void *)inSelf);
}
//#endif

bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
    DWORD nbytes;

    handleT procHandle = getRepresentativeLWP()->getProcessHandle();
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
    bool res = BPatch::bpatch->rDevice->RemoteWriteProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
				  (LPVOID)inSelf, (DWORD)amount, &nbytes);
#else
    bool res = WriteProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
				  (LPVOID)inSelf, (DWORD)amount, &nbytes);
#endif
    
    return res && (nbytes == amount);
}


bool process::readDataSpace_(const void *inTraced, u_int amount, void *inSelf) {
    DWORD nbytes;
    handleT procHandle = getRepresentativeLWP()->getProcessHandle();
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
    bool res = BPatch::bpatch->rDevice->RemoteReadProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
				 (LPVOID)inSelf, (DWORD)amount, &nbytes);
#else
    bool res = ReadProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
				 (LPVOID)inSelf, (DWORD)amount, &nbytes);
	if( !res )
	{
		fprintf( stderr, "ReadProcessMemory failed! %x\n", GetLastError() );
	}
#endif
    return res && (nbytes == amount);
}

bool dyn_lwp::waitUntilStopped() {
   return true;
}

bool process::waitUntilStopped() {
   return true;
}

int getNumberOfCPUs() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}  


Frame dyn_lwp::getActiveFrame()
{
  w32CONTEXT cont; //ccw 27 july 2000 : 29 mar 2001
  
  Address pc = 0, fp = 0, sp = 0;
  
  // we must set ContextFlags to indicate the registers we want returned,
  // in this case, the control registers.
  // The values for ContextFlags are defined in winnt.h
  cont.ContextFlags = CONTEXT_CONTROL;
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
  if (BPatch::bpatch->rDevice->RemoteGetThreadContext((HANDLE)get_fd(), &cont))
#else
  if (GetThreadContext((HANDLE)get_fd(), &cont))
#endif
    {
#ifdef i386_unknown_nt4_0 //ccw 27 july 2000 : 29 mar 2001
      fp = cont.Ebp;
      pc = cont.Eip;
#elif mips_unknown_ce2_11
      pc = cont.Fir;
      sp = cont.IntSp;
      fp = cont.IntS8;
#endif
      return Frame(pc, fp, sp, proc_->getPid(), NULL, this, true);
    }
    printSysError(GetLastError());
    return Frame();
}

#if defined(i386_unknown_nt4_0) //ccw 29 mar 2001
 

Frame Frame::getCallerFrame(process *p) const
{
    // For x86, the frame-pointer (EBP) points to the previous 
    // frame-pointer, and the saved return address is in EBP-4.

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

#elif mips_unknown_ce2_11 //ccw 6 feb 2001

Frame Frame::getCallerFrame(process *p) const
{
//	DebugBreak();//ccw 6 feb 2001

	Frame ret;
	Address prevPC;

	ret.sp_ = sp_ - callee->frame_size;

	Address tmpSp = sp_ + 20;
	p->readDataSpace((caddr_t)(tmpSp), sizeof(int),
			 &prevPC, true);

	ret.pc_ = prevPC;
	return ret;
}
#endif

struct dyn_saved_regs *dyn_lwp::getRegisters() {
    struct dyn_saved_regs *regs = new dyn_saved_regs();
    
  // we must set ContextFlags to indicate the registers we want returned,
  // in this case, the control registers.
  // The values for ContextFlags are defined in winnt.h
  regs->cont.ContextFlags = w32CONTEXT_FULL;//ccw 27 july 2000 : 29 mar 2001
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
  if (!BPatch::bpatch->rDevice->RemoteGetThreadContext((HANDLE)get_fd(), &(regs->cont)))
#else
  handleT handle = get_fd();
  if (!GetThreadContext((HANDLE)handle, &(regs->cont)))
#endif
  {
      return NULL;
  }
  return regs;
}

bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *regs)
{
    
  w32CONTEXT cont;//ccw 27 july 2000
  if (!regs) {
      cont.ContextFlags = w32CONTEXT_FULL;//ccw 27 july 2000 : 29 mar 2001
      //	DebugBreak();
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
      if (!BPatch::bpatch->rDevice->RemoteGetThreadContext((HANDLE)get_fd(), &cont)) 
#else
          if (!GetThreadContext((HANDLE)get_fd(), &cont))
#endif
          {
              printf("GetThreadContext failed\n");
              return false;
          }
  }
  else {
      memcpy(&cont, &(regs->cont), sizeof(w32CONTEXT));
  }
#ifdef i386_unknown_nt4_0 //ccw 27 july 2000 : 29 mar 2001 
  cont.Eip = addr;
#elif  mips_unknown_ce2_11 
  cont.Fir = addr;
#endif
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
  if (!BPatch::bpatch->rDevice->RemoteSetThreadContext((HANDLE)get_fd(), &cont))
#else
  if (!SetThreadContext((HANDLE)get_fd(), &cont))
#endif
    {
      printf("SethreadContext failed\n");
      return false;
    }
  return true;
}

bool dyn_lwp::restoreRegisters(struct dyn_saved_regs *regs) {
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
  if (!BPatch::bpatch->rDevice->RemoteSetThreadContext((HANDLE)get_fd(),
                                                       &(regs->cont)))
#else
  if (!SetThreadContext((HANDLE)get_fd(), 
                        &(regs->cont)))
#endif
    {
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


pdstring 
process::tryToFindExecutable(const pdstring& iprogpath, int pid)
{
    if( iprogpath.length() == 0 )
    {
        HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                    FALSE,
                                    pid );
        if( hProc != NULL )
        {
            // look at the process' modules to see if we can get at an EXE
            DWORD nMods = 32;
            DWORD cb = nMods * sizeof(HMODULE);
            DWORD cbNeeded = 0;
            HMODULE* hMods = new HMODULE[cb];
            BOOL epmRet = EnumProcessModules( hProc,
                                                hMods,
                                                cb,
                                                &cbNeeded );
            if( !epmRet && (cbNeeded > cb) )
            {
                // we didn't pass a large enough array in
                delete[] hMods;
                nMods = (cbNeeded / sizeof(HMODULE));
                cb = cbNeeded;
                hMods = new HMODULE[cb];

                epmRet = EnumProcessModules( hProc,
                                                hMods,
                                                cb,
                                                &cbNeeded );
            }

            if( epmRet )
            {
                // we got modules
                // look for the EXE (always first item?)
                nMods = cbNeeded / sizeof(HMODULE);
                for( unsigned int i = 0; i < nMods; i++ )
                {
                    char modName[MAX_PATH];

                    BOOL gmfnRet = GetModuleFileNameEx( hProc,
                                                        hMods[i],
                                                        modName,
                                                        MAX_PATH );
                    if( gmfnRet )
                    {
                        // check if this is the EXE
                        // TODO is this sufficient?
                        // should we instead be recognizing the
                        // "program" by some other criteria?
                        unsigned int slen = strlen( modName );
                        if( (modName[slen-4] == '.') &&
                            ((modName[slen-3]=='E')||(modName[slen-3]=='e')) &&
                            ((modName[slen-2]=='X')||(modName[slen-2]=='x')) &&
                            ((modName[slen-1]=='E')||(modName[slen-1]=='e')) )
                        {
                            return modName;
                            break;
                        }
                    }
                }
            }

            CloseHandle( hProc );
        }
    }
    return iprogpath;
}


syscallTrap *process::trapSyscallExitInternal(Address syscall) {
    // Don't support trapping syscalls here yet, sorry
    return NULL;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    assert(0 && "Unimplemented");
    return true;
}

Address dyn_lwp::getCurrentSyscall(Address /*ignored*/) {
    return 0;
}

bool dyn_lwp::stepPastSyscallTrap() {
    return false;
}

int dyn_lwp::hasReachedSyscallTrap() {
    return 0;
}

Address dyn_lwp::readRegister(Register reg)
{
   w32CONTEXT *cont = new w32CONTEXT;//ccw 27 july 2000 : 29 mar 2001
    if (!cont)
	return NULL;
    // we must set ContextFlags to indicate the registers we want returned,
    // in this case, the control registers.
    // The values for ContextFlags are defined in winnt.h
    cont->ContextFlags = w32CONTEXT_FULL;//ccw 27 july 2000
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
    if (!BPatch::bpatch->rDevice->RemoteGetThreadContext((HANDLE)get_fd(), cont)) {
#else
    if (!GetThreadContext((HANDLE)get_fd(), cont)) {
#endif
	delete cont;
	return NULL;
    }
#ifdef i386_unknown_nt4_0 //ccw 27 july 2000 : 29 mar 2001
    return cont->Eax;
#elif mips_unknown_ce2_11 
	return (&cont->IntZero)[reg]; 
	//the registers in the MIPS context returned by
	//winCE return general registers 0..31 in the struct
	//starting at IntZero and going to IntRa, each being a DWORD
#endif
}

bool dyn_lwp::executingSystemCall() {
   // TODO
   return false;
}


void
InitSymbolHandler( HANDLE hProcess )
{
#if READY
    fprintf( stderr, "Calling SymInitialize with handle %x\n", hProcess );
    if( !SymInitialize( hProcess, NULL, FALSE ) )
    {
        // TODO how to report error?
        fprintf( stderr, "failed to initialize symbol handler: %x\n",
            GetLastError() );
    }
#endif // READY
}

void
ReleaseSymbolHandler( HANDLE hProcess )
{
    if( !SymCleanup( hProcess ) )
    {
        // TODO how to report error?
        fprintf( stderr, "failed to release symbol handler: %x\n",
            GetLastError() );
    }
}

void process::recognize_threads(pdvector<unsigned> *completed_lwps) {
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
 *   inputFile: where to redirect standard input
 *   outputFile: where to redirect standard output
 *   traceLink: handle or file descriptor of trace link (read only)
 //removed all ioLink related code for output redirection
 *   ioLink: handle or file descriptor of io link (read only)
 *   pid: process id of new process
 *   tid: thread id for main thread (needed by WindowsNT)
 *   procHandle: handle for new process (needed by WindowsNT)
 *   thrHandle: handle for main thread (needed by WindowsNT)
 ****************************************************************************/
bool forkNewProcess(pdstring &file, pdstring dir, pdvector<pdstring> argv, 
                    pdstring inputFile, pdstring outputFile,
                    int &traceLink, int &pid, int &tid, 
                    int &procHandle, int &thrHandle, int /* stdin_fd */, 
                    int stdout_fd, int /* stderr_fd */) {
#ifdef mips_unknown_ce2_11 //ccw 8 aug 2000 : 29 mar 2001
	WCHAR appName[256];
	WCHAR dirName[256];
	WCHAR commLine[256];
#endif

#ifndef BPATCH_LIBRARY
#ifdef notdef_Pipes     // isn't this all obsolete???
    HANDLE rTracePipe;
    HANDLE wTracePipe;
    // security attributes to make handles inherited
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = true;
    sa.lpSecurityDescriptor = NULL;
    
    // create trace pipe
    if (!CreatePipe(&rTracePipe, &wTracePipe, &sa, 0)) {
       pdstring msg = pdstring("Unable to create trace pipe for program '") +
                      File + pdstring("': ") + pdstring(sys_errlist[errno]);
       showErrorCallback(68, msg);
       return(NULL);
    }
    
    /* removed for output redirection
    HANDLE rIoPipe;
    HANDLE wIoPipe;
    // create IO pipe
    // ioPipe is used to redirect the child's stdout & stderr to a pipe which
    // is in turn read by the parent via the process->ioLink socket.
    if (!CreatePipe(&rIoPipe, &wIoPipe, &sa, 0)) {
       pdstring msg = pdstring("Unable to create IO pipe for program '") +
       File + pdstring("': ") + pdstring(sys_errlist[errno]);
       showErrorCallback(68, msg);
       return(NULL);
    }
    SetEnvironmentVariable("PARADYN_IO_PIPE",
                           pdstring((unsigned)wIoPipe).c_str());
    */

    printf("tracepipe = %d\n", (unsigned)wTracePipe);
    // enter trace and IO pipes in child's environment
    SetEnvironmentVariable("PARADYN_TRACE_PIPE", 
			   pdstring((unsigned)wTracePipe).c_str());
#endif
    //  extern int traceSocket;
    //  SetEnvironmentVariable("PARADYND_TRACE_SOCKET", pdstring((unsigned)traceSocket).c_str());
#endif /* BPATCH_LIBRARY */
    
    // create the child process
    
    pdstring args;
    for (unsigned ai=0; ai<argv.size(); ai++) {
       args += argv[ai];
       args += " ";
    }

    
    STARTUPINFO stinfo;
    memset(&stinfo, 0, sizeof(STARTUPINFO));
    stinfo.cb = sizeof(STARTUPINFO);

    /*to do: output redirection
    //stinfo.hStdOutput = (HANDLE)ioLink;
    stinfo.hStdOutput = (HANDLE)stdout_fd;
    stinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    stinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    stinfo.dwFlags |= STARTF_USESTDHANDLES;
    */
    PROCESS_INFORMATION procInfo;
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
	kludgeWCHAR(file.c_str(),appName);
	kludgeWCHAR(args.c_str(),commLine);
	if(dir == ""){
		dirName[0] = (unsigned short) 0;
	}else{
		kludgeWCHAR(dir.c_str(),dirName);
	}

	/* ccw 8 aug 2000  : 29 mar 2001
	 WARNING:  
		If compiler optimization are turned on (-Ox) for remoteDevice.C the following
		line of code bugs. An extra parameter is passed to the function before 
		appName and consequently each argument is shifted one to the right, 
		completely destroying the code.
		
		DO NOT TURN ON -Ox for remoteDevice.C!

		if need be, pdwinnt.C can be compiled with -Ox
		less optimization may be safe, it has not yet been tried. if someone is bored
		they should try different optimization flags to see if any work correctly here.
		Until then the mips/ce version of dyninst will be compiled w/o ANY optimizations!
	*/
	//DebugBreak();
    if (BPatch::bpatch->rDevice->RemoteCreateProcess(appName , commLine,
		      NULL, NULL, false,
		      (DWORD) DEBUG_PROCESS  /* | CREATE_NEW_CONSOLE /* | CREATE_SUSPENDED */,
		      NULL, dirName, 
		      &stinfo, &procInfo)) {

#else
    if (CreateProcess(file.c_str(), (char *)args.c_str(), 
		      NULL, NULL, TRUE,
		      DEBUG_PROCESS /* | CREATE_NEW_CONSOLE /* | CREATE_SUSPENDED */,
		      NULL, dir == "" ? NULL : dir.c_str(), 
		      &stinfo, &procInfo)) {
#endif

	procHandle = (Word)procInfo.hProcess;
	thrHandle = (Word)procInfo.hThread;
	pid = (Word)procInfo.dwProcessId;
	tid = (Word)procInfo.dwThreadId;
#ifdef notdef_Pipes
        traceLink = (Word)rTracePipe;
	CloseHandle(wTracePipe);
	/ *removed for output redirection
        //ioLink = (Word)rIoPipe;
        CloseHandle(wIoPipe);
        */
#else
        traceLink = -1;
	/* removed for output redirection
        ioLink = -1;
	*/
#endif
	return true;
    }
   
#ifndef BPATCH_LIBRARY
#ifdef notdef_Pipes
    CloseHandle(rTracePipe);
    CloseHandle(wTracePipe);
    / *removed for output redirection
    CloseHandle(rIoPipe);
    CloseHandle(wIoPipe);
    */
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
	    sprintf(errorLine, "Unable to start %s: %s\n", file.c_str(),
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
		file.c_str());
	logLine(errorLine);
	showErrorCallback(68, (const char *) errorLine);
    }

    return false;
}

/*
 * stripAtSuffix
 *
 * Strips off of a string any suffix that consists of an @ sign followed by
 * decimal digits.
 *
 * str	The string to strip the suffix from.  The string is altered in place.
 */
static void stripAtSuffix(char *str)
{
    // many symbols have a name like foo@4, we must remove the @4
    // just searching for an @ is not enough,
    // as it may occur on other positions. We search for the last one
    // and check that it is followed only by digits.
    char *p = strrchr(str, '@');
    if (p) {
      char *q = p+1;
      strtoul(p+1, &q, 10);
      if (q > p+1 && *q == '\0') {
	*p = '\0';
      }
    }
}

char *cplus_demangle(char *c, int, bool includeTypes) { 
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
        buf[i-1]='\0';
	stripAtSuffix(buf);
        if (buf[0] == '\0') return 0; // avoid null names which seem to annoy Paradyn
        return strdup(buf);
      } else {
        if (includeTypes) {
          if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE| UNDNAME_NO_ACCESS_SPECIFIERS|UNDNAME_NO_MEMBER_TYPE|UNDNAME_NO_MS_KEYWORDS)) {
            //   printf("Undecorate with types: %s = %s\n", c, buf);
            stripAtSuffix(buf);
            return strdup(buf);
          }
        }  else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_NAME_ONLY)) {
          //     else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE|UNDNAME_32_BIT_DECODE)) {
          //	printf("Undecorate: %s = %s\n", c, buf);
          stripAtSuffix(buf);
          return strdup(buf);
        }
      }
    return 0;
}

bool OS::osKill(int pid) {
    bool res;
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
    HANDLE h = BPatch::bpatch->rDevice->RemoteOpenProcess(PROCESS_ALL_ACCESS, false, pid);
#else
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
#endif
    if (h == NULL) {
	return false;
    }
#ifdef mips_unknown_ce2_11 //ccw 31 aug 2000 : 29 mar 2001
    res = BPatch::bpatch->rDevice->RemoteTerminateProcess(h,0);
    BPatch::bpatch->rDevice->RemoteCloseHandle(h);
#else
    res = TerminateProcess(h,0);
    CloseHandle(h);
#endif
    return res;
}

#if !defined(BPATCH_LIBRARY)
rawTime64 dyn_lwp::getRawCpuTime_hw() {
  fprintf(stderr, "dyn_lwp::getRawCpuTime_hw: not implemented\n");
  return 0;
}

rawTime64
FILETIME2rawTime64( FILETIME& ft )
{
    return (((rawTime64)(ft).dwHighDateTime<<32) | 
	    ((rawTime64)(ft).dwLowDateTime));
}

/* return unit: nsecs */
rawTime64 dyn_lwp::getRawCpuTime_sw() 
{
  FILETIME kernelT, userT, creatT, exitT;
  rawTime64 now;
  if(GetThreadTimes( (HANDLE)fd_,
      &creatT, &exitT, &kernelT,&userT)==0) {
    return 0;
  }

  // GetProcessTimes returns values in 100-ns units
  now = (FILETIME2rawTime64(userT)+FILETIME2rawTime64(kernelT));

  // time shouldn't go backwards, but we'd better handle it if it does
//  printf(" %I64d %I64d \n", now, previous);
  if (now < sw_previous_) {
	  //DebugBreak();
     logLine("********* time going backwards in paradynd **********\n");
     now = sw_previous_;
  }
  else {
     sw_previous_ = now;
  }

  return now;
}

bool process::catchupSideEffect(Frame &frame, instReqNode *inst)
{
  return true;
}

#endif

#ifdef mips_unknown_ce2_11 //ccw 2 aug 2000 : 29 mar 2001

#include "inst-mips.h"
//this comes from irix.C
bool process::heapIsOk(const pdvector<sym_data>&findUs)
{ 
	
  //ccw 12 oct 2000 NEED TO FIND WinMain here or _WinMain then try to find main or _main
//	DebugBreak();


  if (!(mainFunction = findOnlyOneFunction("WinMain")) &&
	  !(mainFunction = findOnlyOneFunction("_WinMain"))) {
		fprintf(stderr, "process::heapIsOk(): failed to find \"WinMain\"\n");

		if (!(mainFunction = findOnlyOneFunction("main")) &&
		!(mainFunction = findOnlyOneFunction("_main"))) {

			if (!(mainFunction = findOnlyOneFunction("wWinMain")) &&
				!(mainFunction = findOnlyOneFunction("_wWinMain"))) {

				fprintf(stderr, "process::heapIsOk(): failed to find \"main\"\n");
				return false;
			}
	  }	
  }

  for (unsigned i = 0; i < findUs.size(); i++) {
    const pdstring &name = findUs[i].name;
    /*
    Address addr = lookup_fn(this, name);
    if (!addr && findUs[i].must_find) {
      fprintf(stderr, "process::heapIsOk(): failed to find \"%s\"\n", name.c_str());
      return false;
    }
    */
  }

  return true;
}
#endif


fileDescriptor*
getExecFileDescriptor(pdstring filename, int& status, bool)
{
    // "status" holds the process handle
    return new fileDescriptor_Win( filename, (HANDLE)status );
}

bool getLWPIDs(pdvector <unsigned> &LWPids)
{
  assert (0 && "Not implemented");
  return false;
}




//
// This function retrieves the name of a DLL that has been
// loaded in an inferior process.  On the desktop flavors
// of Windows, the debug event that we get for loaded DLLs
// includes the location of a pointer to the DLL's image name.
// (Note the indirection.)  On Windows CE, the event contains
// the location of the image name string, with no indirection.
//
// There are several complications to overcome when reading this string:
//
// 1.  There is no guarantee that the image name is available.
//     In this case, the location in the debug event may be NULL,
//     or the pointer in the inferior process' address space may be NULL.
// 2.  The image name string may be either ASCII or Unicode.  Most of
//     the Windows system DLLs have Unicode strings, but many user-built
//     DLLs use single-byte strings.  If the string is Unicode, we need
//     to copy it to our address space and convert it to a single-byte
//     string because the rest of Paradyn/Dyninst has no clue what to
//     do with Unicode strings.
// 3.  We don't know how long the string is.  We have a loose upper
//     bound in that we know it is not more than MAX_PATH characters.
//     Unfortunately, the call we use to read from the inferior
//     process' address space will return failure if we ask for more
//     bytes than it can actually read (so we can't just issue a read
//     for MAX_PATH characters).  Given this limitation, we have to
//     try a read and check whether the read succeeded *and* whether
//     we read the entire image name string.  If not, we have to adjust
//     the amount we read and try again.
//
static pdstring GetLoadedDllImageName( process* p, const DEBUG_EVENT& ev )
{
	pdstring ret;


	if( ev.u.LoadDll.lpImageName == NULL )
	{
		// there is no image name string for us to read
		return ret;
	}

	char* msgText = new char[1024];	// buffer for error messages

#if defined(mips_unknown_ce2_11)

	// On Windows CE, the address given in the debug event struct
	// is the address of the DLL name string.
	void* pImageName = ev.u.LoadDll.lpImageName;
#else // defined(ce)
	// On non-CE flavors of Windows, the address given in the debug
	// event struct is the address of a pointer to the DLL name string.
	void* pImageName = NULL;
	if( !p->readDataSpace( ev.u.LoadDll.lpImageName, 4, &pImageName, false ) )
	{
		sprintf( msgText, "Failed to read DLL image name pointer: %d\n",
			GetLastError() );
		logLine( msgText );
	}
#endif // defined(ce)

	if( pImageName != NULL )
	{
		// we have the pointer to the DLL image name -
		// now read the name

		// allocate a conservatively-sized buffer
		char* buf = new char[(MAX_PATH + 1) * sizeof(WCHAR)];
		WCHAR* wbuf = (WCHAR*)buf;

		// We don't know how long the image name actually is, but
		// we do know that they tend not to be very long.
		// Therefore, we use a scheme to try to minimize the number
		// of reads needed to get the image name.
		// We do reads within ranges, starting with [1,128] bytes,
		// then [129,256] bytes, etc. up to MAX_PATH if necessary.
		// Within each range, we do reads following a binary search
		// algorithm.  For example, for the [1,128] range, we start
		// by trying to read 128 bytes.  If that read fails, we
		// try to half the number of bytes (i.e., 64).  If that
		// read also fails, we continue to halve the read requests 
		// until we find one that succeeds.
		//
		// When a read succeeds, we still may not have gotten the
		// entire string.  So when reads start succeeding, we have to
		// check the data we got for a null-terimated string.  If we didn't
		// get the full string, we change the byte count to either
		// move into the next higher range (if we were already reading
		// the max within the current range) or we set it to a factor
		// of 1.5 of the current byte count to try a value between the
		// current succeeding read and one that had failed.
		unsigned int loRead = 1;		// range boundaries
		unsigned int hiRead = 128;
		unsigned int cbRead = 128;		// number of bytes to read
		unsigned int chunkRead = 64;	// amount to change cbRead if we fail
										// we will not halve this before we read
		bool gotString = false;
		bool doneReading = false;
		while( !doneReading )
		{
			// try the read with the current byte count
			if( p->readDataSpace( pImageName, cbRead, buf, false ) )
			{
				// the read succeeded - 
				// did we get the full string?
				if( ev.u.LoadDll.fUnicode )
				{
					unsigned int cbReadIdx = cbRead / sizeof(WCHAR);
					wbuf[cbReadIdx] = L'\0';
					WCHAR* nulp = wcschr( wbuf, L'\0' );
					assert( nulp != NULL );			// because we just NULL-terminated the string
					gotString = (nulp != &(wbuf[cbReadIdx]));
				}
				else
				{
					buf[cbRead] = '\0';
					char* nulp = strchr( buf, '\0' );
					assert( nulp != NULL );			// because we just NULL-terminated the string
					gotString = (nulp != &(buf[cbRead]));
				}

				if( gotString )
				{
					doneReading = true;
				}
				else
				{
					// we didn't get the full string
					// we need to try a larger read
					if( cbRead == hiRead )
					{
						// we were at the high end of the current range -
						// move to the next range
						loRead = hiRead + 1;
						hiRead = loRead + 128 - 1;
						chunkRead = 128;				// we will halve this before we read again
						if( loRead > (MAX_PATH * sizeof(WCHAR)) )
						{
							// we've tried every range but still failed
							doneReading = true;
						}
						else
						{
							cbRead = hiRead;
						}
					}
					else
					{
						// we were within the current range -
						// try something higher but still within the range
						cbRead = cbRead + chunkRead;
					}
				}
			}
			else
			{
				// the read failed -
				// we need to try a smaller read
				if( cbRead > loRead )
				{
					unsigned int nextRead = cbRead - chunkRead;
					if( nextRead == cbRead )
					{
						// we can't subdivide any further
						doneReading = true;
					}
					else
					{
						cbRead = nextRead;
					}
				}
				else
				{
					// there are no smaller reads to try in this range,
					// and by induction, in any range.
					doneReading = true;
				}
			}

			// update the amount that we use to change the read request
			chunkRead /= 2;
		}

		if( !gotString )
		{
			// this is a serious problem because some read 
			// should've succeeded
			sprintf( msgText, "Failed to read DLL image name - no read succeeded\n" );
			logLine( msgText );
		}
		else
		{
			if( ev.u.LoadDll.fUnicode )
			{
				// the DLL path is a Unicode string
				// we have to convert it to single-byte characters
				char* tmpbuf = new char[MAX_PATH];

				WideCharToMultiByte(CP_ACP,		// code page to use (ANSI)
									0,			// flags
									wbuf,		// Unicode string
									-1,			// length of Unicode string (-1 => null-terminated)
									tmpbuf,		// destination buffer
									MAX_PATH,	// size of destionation buffer
									NULL,		// default for unmappable chars
									NULL);		// var to set when defaulting a char

				// swap buffers so that buf points to the single-byte string
				// when we're out of this code block
				delete[] buf;
				buf = tmpbuf;
			}
			ret = buf;
		}

		delete[] buf;
	}
	else
	{
		// we were given an image name pointer, but it was NULL
		// this happens for some system DLLs, and if we attach to
		// the process instead of creating it ourselves.
		// However, it is very important for us to know about kernel32.dll,
		// so we check for it specially.
		//
		// This call only changes the string parameter if the indicated file is
		// actually kernel32.dll.
		kludge_isKernel32Dll( ev.u.LoadDll.hFile, ret );
	}

	// cleanup
	delete[] msgText;

	return ret;
}

bool dyn_lwp::realLWP_attach_() {
   //assert( false && "threads not yet supported on Windows");
   return false;
}

bool dyn_lwp::representativeLWP_attach_() {
   if(proc_->createdViaAttach) {
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000 : 29 mar 2001
      if (!BPatch::bpatch->rDevice->RemoteDebugActiveProcess(getPid()))
      {
         return false;
      }
      procHandle_ = 
         BPatch::bpatch->rDevice->RemoteOpenProcess(PROCESS_ALL_ACCESS,
                                                    false, getPid());
      assert( procHandle_ != NULL );
      return true;
#else
      if (!DebugActiveProcess(getPid()))
      {
         //printf("Error: DebugActiveProcess failed\n");
         return false;
      }
#endif
   }

   // We either created this process, or we have just attached it.
   // In either case, our descriptor already has a valid process handle.
   fileDescriptor_Win* fdw = (fileDescriptor_Win*)(proc_->getImage()->desc());
   assert( fdw != NULL );

   if(fdw->GetProcessHandle() != INVALID_HANDLE_VALUE)
      setProcessHandle(fdw->GetProcessHandle());
   
   return true;
}

void dyn_lwp::realLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
   return;
}

void dyn_lwp::representativeLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
   return;
}


// Insert a breakpoint at the entry of main()

bool process::insertTrapAtEntryPointOfMain() {
    // if this was created via attach then
    // we are not at main() so no reason to
    // put a breakpoint there, we are already
    // at a state that will allow loadLibrary() to
    // succeed. 
    
    function_base *mainFunc;
    //DebugBreak();//ccw 14 may 2001  
    if (!((mainFunc = findOnlyOneFunction("main")))){
        if(!(mainFunc = findOnlyOneFunction("_main"))){
            if(!(mainFunc = findOnlyOneFunction("WinMain"))){
                if(!(mainFunc = findOnlyOneFunction("_WinMain"))){
                    if(!(mainFunc = findOnlyOneFunction("wWinMain"))){
                        if(!(mainFunc = findOnlyOneFunction("_wWinMain"))){
                            fprintf(stderr, "Couldn't find main!\n");
                            
                            return false;
                        }
                    }
                }
            }
        }
        
    }
    main_brk_addr = mainFunc->addr();
    
    readDataSpace((void*) (main_brk_addr), BYTES_TO_SAVE, (void*)savedCodeBuffer, false);
    
    // A trap instruction. 
    unsigned char trapInsn = 0xcc;
    
    // write the modified code sequence back
    writeDataSpace((void*) (main_brk_addr), sizeof(trapInsn), 
                   (void*)&trapInsn);
    flushInstructionCache_( (void*)main_brk_addr, sizeof(trapInsn) );
    return true;
}

// True if we hit the trap at the entry of main

bool process::trapAtEntryPointOfMain(Address trapAddr) {
    if (!main_brk_addr) return false;
    if (trapAddr == main_brk_addr) {
        return true;
    }
    return false;
}


// Clean up after breakpoint in main() is hit
bool process::handleTrapAtEntryPointOfMain()
{
    // Rewrite saved registers and code buffer
    assert(main_brk_addr);

    writeDataSpace((void *)main_brk_addr,
                   sizeof(unsigned char), (void *)savedCodeBuffer);
    flushInstructionCache_((void *)main_brk_addr, sizeof(unsigned char));

    // And bump the PC back
    getRepresentativeLWP()->changePC(main_brk_addr, NULL);

    // Don't trap here accidentally
    main_brk_addr = 0;
    return true;
}

bool process::getDyninstRTLibName() {
    // Set the name of the dyninst RT lib
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            pdstring msg = pdstring("Environment variable " +
                           pdstring("DYNINSTAPI_RT_LIB") +
                           " has not been defined for process ") +
                           pdstring(pid);
            showErrorCallback(101, msg);
            return false;
        }
    }
    if (_access(dyninstRT_name.c_str(), 04)) {
        pdstring msg = pdstring("Runtime library ") + dyninstRT_name +
                       pdstring(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }

    return true;
}


// Load the dyninst library
bool process::loadDYNINSTlib()
{
    Address codeBase = getImage()->codeOffset();
    Address LoadLibBase;
    Address LoadLibAddr;
    Symbol sym;

#ifdef mips_unknown_ce2_11 //ccw 14 aug 2000 : 29 mar 2001

	Address hackLoadLibAddr = 0x01f9ac30; //ccw 2 feb 2001 : HACK
    
	//ccw 2 feb 2001
	//this is a terrible hack.  i know that LoadLibrary is 0xac30 bytes from
	//the base address of coredll.dll from emprical evidence.  i have no
	//debug symbols for coredll.dll to prove this dynamically though....
	
	for(int i=0; i< (p->sharedObjects())->size();i++){
		
		if((*p->sharedObjects())[i]->getName() == "coredll.dll" ){
			hackLoadLibAddr = (*p->sharedObjects())[i]->getBaseAddress() + 0x0000bf1c;
			//0x0000ac30; //HACK 24 apr 2001
		}
	}
#else

	if (!getSymbolInfo("_LoadLibraryA@4", sym, LoadLibBase) &&
		!getSymbolInfo("_LoadLibraryA", sym, LoadLibBase ) &&
		!getSymbolInfo("LoadLibraryA", sym, LoadLibBase ) )
    {
	    printf("unable to find function LoadLibrary\n");
	    assert(0);
    }
    LoadLibAddr = sym.addr() + LoadLibBase;
    assert(LoadLibAddr);
#endif
    char ibuf[BYTES_TO_SAVE];
    char *iptr = ibuf;
	memset(ibuf, '\0', BYTES_TO_SAVE);//ccw 25 aug 2000

    // Code overview:
    // Dynininst library name
    //    Executable code begins here:
    // Push (address of dyninst lib name)
    // Call LoadLibrary
    // Pop (cancel push)
    // Trap

    process::dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");

    if (!process::dyninstRT_name.length())
        // if environment variable unset, use the default name/strategy
        process::dyninstRT_name = "libdyninstAPI_RT.dll";
	
    // make sure that directory separators are what LoadLibrary expects
    strcpy(iptr, process::dyninstRT_name.c_str());
    for (unsigned int i=0; i<strlen(iptr); i++)
        if (iptr[i]=='/') iptr[i]='\\';

    // 4: give us plenty of room after the string to start instructions
    int instructionOffset = strlen(iptr) + 4;
    // Regenerate the pointer
    iptr = &(ibuf[instructionOffset]);
    
    // At this point, the buffer contains the name of the dyninst
    // RT lib. We now generate code to load this string into memory
    // via a call to LoadLibrary

    // push nameAddr ; 5 bytes
    *iptr++ = (char)0x68; 
    // Argument for push
    *(int *)iptr = codeBase; // string at codeBase
    iptr += sizeof(int);

    int offsetFromBufferStart = (int)iptr - (int)ibuf;
    offsetFromBufferStart += 5; // Skip next instruction as well.
    // call LoadLibrary ; 5 bytes
    *iptr++ = (char)0xe8;
   
    // Jump offset is relative
    *(int *)iptr = LoadLibAddr - (codeBase + 
                                  offsetFromBufferStart); // End of next instruction
    iptr += sizeof(int);


    // add sp, 4 (Pop)
    *iptr++ = (char)0x83; *iptr++ = (char)0xc4; *iptr++ = (char)0x04;

    // int3
    *iptr = (char)0xcc;

    int offsetToTrap = (int) iptr - (int) ibuf;
    readDataSpace((void *)codeBase, BYTES_TO_SAVE, savedCodeBuffer, false);
    writeDataSpace((void *)codeBase, BYTES_TO_SAVE, ibuf);
    
    flushInstructionCache_((void *)codeBase, BYTES_TO_SAVE);

#ifdef mips_unknown_ce2_11 //ccw 22 aug 2000
    assert(0 && "Need to update dyninst lib load instructions");
#else
    dyninstlib_brk_addr = codeBase + offsetToTrap;
#endif

    savedRegs = getRepresentativeLWP()->getRegisters();
    getRepresentativeLWP()->changePC(codeBase + instructionOffset, NULL);
    
    setBootstrapState(loadingRT);
    return true;
}



// Not used on NT. We'd have to rewrite the
// prototype to take a PC. Handled inline.
// True if trap is from dyninst load finishing
bool process::trapDueToDyninstLib()
{
    assert(0);
    return true;
}



// Cleanup after dyninst lib loaded
bool process::loadDYNINSTlibCleanup()
{
    // First things first: 
    getRepresentativeLWP()->restoreRegisters(savedRegs);
    delete savedRegs;

    writeDataSpace((void *)getImage()->codeOffset(),
				      BYTES_TO_SAVE,
				      (void *)savedCodeBuffer);

    flushInstructionCache_((void *)getImage()->codeOffset(), BYTES_TO_SAVE);

    dyninstlib_brk_addr = 0;

    return true;
}

//MIPS code from loadDyninstLib. I'm sticking it here
//to assist in debugging NT. Copy it back in when needed

#if 0

	//the following are the instructions to load libdyninstRT_API.dll
	
	//ccw 2 oct 2000
	//the instructions we need to generate are:
	// lui $a0,hi16(codeBase)  # codeBase contains libBuf
	// ori $a0,lo16(codeBase)
	// lui $t0, hi16(hackLoadLibAddr)
	// ori $t0, lo16(hackLoadLibAddr)
	// jalr $t0
	// nop
	// nop
	// break
	// nop
	// nop
	// nop
	// where libBuf is the location of the string containing the name of the dll
	// in the remote process's memory

	WCHAR libBuf[] = L"\\libdyninstAPI_RT.dll";
	BYTE instBuff[4]; //buffer for the instruction to write to memory;

	//lay the name of the DLL to load into memory, UNICODE!
	assert(memcpy(iptr, (char*)libBuf, 42));
	iptr +=42;
	memset(iptr, '\0', 2);
	iptr +=2;

	//LUI //ccw 2 oct 2000
	instBuff[0]=((char*) &codeBase)[2];
	instBuff[1]=((char*) &codeBase)[3];
	instBuff[2]=0x04;
	instBuff[3]=0x3C;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

	//ORI //ccw 2 oct 2000
	instBuff[0]=((char*) &codeBase)[0];
	instBuff[1]=((char*) &codeBase)[1];
	instBuff[2]=0x84;
	instBuff[3]=0x34;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

	//LUI //ccw 2 feb 2001
	instBuff[0]=((char*) &hackLoadLibAddr)[2];
	instBuff[1]=((char*) &hackLoadLibAddr)[3];
	instBuff[2]=0x08;
	instBuff[3]=0x3C;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

	//ORI //ccw 2 feb 2001
	instBuff[0]=((char*) &hackLoadLibAddr)[0];
	instBuff[1]=((char*) &hackLoadLibAddr)[1];
	instBuff[2]=0x08;
	instBuff[3]=0x35;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

	//JALR
	instBuff[0]=0x09;
	instBuff[1]=0xf8;
	instBuff[2]=0x00;
	instBuff[3]=0x01;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;


	//NOP
	instBuff[0]=0x00;
	instBuff[1]=0x00;
	instBuff[2]=0x00;
	instBuff[3]=0x00;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

	//NOP
	instBuff[0]=0x00;
	instBuff[1]=0x00;
	instBuff[2]=0x00;
	instBuff[3]=0x00;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;


	//DebugBreak();
	instBuff[0]=0x0D;
	instBuff[1]=0x00;
	instBuff[2]=0x00;
	instBuff[3]=0x00;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;


	//ccw 25 aug 2000 : throw some NOPs after the debugBreak just in case!
	//NOP
	instBuff[0]=0x00;
	instBuff[1]=0x00;
	instBuff[2]=0x00;
	instBuff[3]=0x00;
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;
	
	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

	memcpy(iptr, (char*) &instBuff, 4);
	iptr +=4;

#endif

void loadNativeDemangler() {}


Frame dyn_thread::getActiveFrameMT() {
   return Frame();
}  // not used until MT supported

