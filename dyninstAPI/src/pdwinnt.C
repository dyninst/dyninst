/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "common/h/std_namesp.h"
#include <iomanip>
#include <string>
#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include <psapi.h>
#include <windows.h>
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emit-x86.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "symtab.h"
#include "MemoryEmulator/memEmulator.h"
#include <boost/tuple/tuple.hpp>

#include "dyninstAPI/src/ast.h"

#include "dyninstAPI/src/function.h"

/* XXX This is only needed for emulating signals. */
#include "BPatch_thread.h"
#include "BPatch_process.h"
#include "nt_signal_emul.h"

#include "dyninstAPI/src/rpcMgr.h"

// prototypes of functions used in this file

void InitSymbolHandler( HANDLE hProcess );
void ReleaseSymbolHandler( HANDLE hProcess );
extern bool isValidAddress(AddressSpace *proc, Address where);

void printSysError(unsigned errNo) {
    char buf[1000];
    bool result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);
    if (!result) {
        fprintf(stderr, "Couldn't print error message\n");
        printSysError(GetLastError());
    }
    fprintf(stderr, "*** System error [%d]: %s\n", errNo, buf);
    fflush(stderr);
}


// check if a file handle is for kernel32.dll
static bool kludge_isKernel32Dll(HANDLE fileHandle, std::string &kernel32Name) {
    static DWORD IndxHigh, IndxLow;
    static bool firstTime = true;
    BY_HANDLE_FILE_INFORMATION info;
    static std::string kernel32Name_;

    if (firstTime) {
       HANDLE kernel32H;
       firstTime = false;
       char sysRootDir[MAX_PATH+1];
       if (GetSystemDirectory(sysRootDir, MAX_PATH) == 0)
          assert(0);
       kernel32Name_ = std::string(sysRootDir) + "\\kernel32.dll";
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

bool process::dumpImage(std::string outFile)
{
  fprintf(stderr, "%s[%d]:  Sorry, dumpImage() not implemented for windows yet\n", FILE__, __LINE__);
  fprintf(stderr, "\t cannot create '%s' as requested\n", outFile.c_str());
  return false;
}

dyn_lwp *process::createRepresentativeLWP() {
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

static void hasIndex(process *, unsigned, void *data, void *result) 
{
    *((int *) data) = (int) result;
}

// Thread creation
bool SignalHandler::handleThreadCreate(EventRecord &ev, bool &continueHint)
{
   process *proc = ev.proc;
   CONTEXT cont;
   Address initial_func = 0, stack_top = 0;
   BPatch_process *bproc = (BPatch_process *) ev.proc->up_ptr();
   HANDLE lwpid = ev.info.u.CreateThread.hThread;
   func_instance *func = NULL;
   int tid = ev.info.dwThreadId;
   
   //Create the lwp early on Windows
   dyn_lwp *lwp = proc->createRealLWP((int) lwpid, (int) lwpid);
   lwp->setFileHandle(lwpid);
   lwp->setProcessHandle(proc->processHandle_);
   lwp->attach();
   ev.lwp = lwp;
   proc->set_lwp_status(lwp, stopped);

   continueHint = true;
   if (proc->reachedBootstrapState(bootstrapped_bs)) 
   {
        //The process is already running when this thread was created.  It's at
        //its initial entry point where we can read the initial function out of EAX
        cont.ContextFlags = CONTEXT_FULL;
        if (GetThreadContext(lwpid, &cont))
        {
            initial_func = cont.Eax;
            stack_top = cont.Esp;           
        }
   }

   if (initial_func) {
     func = proc->findJumpTargetFuncByAddr(initial_func);
     if (!func)
        return false;
     if (!func) {
        mapped_object *obj = proc->findObject(initial_func);
        if (obj) {
           vector<Address> faddrs;
           faddrs.push_back(initial_func);
           obj->parseNewFunctions(faddrs);
           func = proc->findOneFuncByAddr(initial_func);
        }
     }
   }

   //Create the dyn_thread early as well.
   dyn_thread *thr = new dyn_thread(proc, -1, lwp);
   thr->update_tid(tid);
   thr->update_start_pc(initial_func);
   thr->update_start_func(func);
   thr->update_stack_addr(stack_top);

   if (func) {
       proc->instrumentThreadInitialFunc(func);
   }

   return true;
}

bool SignalHandler::handleExecEntry(EventRecord &, bool &)
{
  assert(0);
  return false;
}

// Process creation
bool SignalHandler::handleProcessCreate(EventRecord &ev, bool &continueHint) 
{
    process *proc = ev.proc;
    
    if(! proc)
        return true;
  
    dyn_lwp *rep_lwp = proc->getRepresentativeLWP();
    assert(rep_lwp);  // the process based lwp should already be set

    //We're starting up, convert the representative lwp to a real one.
    rep_lwp->set_lwp_id((int) rep_lwp->get_fd());
    proc->real_lwps[rep_lwp->get_lwp_id()] = rep_lwp;
    proc->representativeLWP = NULL;
    if (proc->theRpcMgr)
       proc->theRpcMgr->addLWP(rep_lwp);
    
    if (proc->threads.size() == 0) {
        dyn_thread *t = new dyn_thread(proc, 
                                       0, // POS (main thread is always 0)
                                       rep_lwp);
    }
    else {
        proc->threads[0]->update_tid(ev.info.dwThreadId);
        proc->threads[0]->update_lwp(rep_lwp);
    }

    proc->set_status(stopped);
   proc->setBootstrapState(begun_bs);
   if (proc->insertTrapAtEntryPointOfMain()) {
     startup_printf("%s[%d]:  attached to process, stepping to main\n", FILE__, __LINE__);
   }
   else {
     proc->handleProcessExit();
   }
   continueHint = true;
   return true;
}


bool CALLBACK printMods(PCSTR name, DWORD64 addr, PVOID unused) {
    fprintf(stderr, " %s @ %llx\n", name, addr);
    return true;
}

//Returns true if we need to 
bool SignalGenerator::SuspendThreadFromEvent(LPDEBUG_EVENT ev, dyn_lwp *lwp) {
    HANDLE hlwp;
    if (ev->dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
        hlwp = ev->u.CreateThread.hThread;
    }
    else if (lwp) {
        hlwp = lwp->get_fd();
    }
    else if (proc->getRepresentativeLWP()) {
        hlwp = proc->getRepresentativeLWP()->get_fd();
    }
    else {
		return false;
    }

    int result = SuspendThread(hlwp);
    if (result == -1) {
        //Happens for thread exit events.
        return false;        
    }
    return true;
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
bool SignalGenerator::waitForEventsInternal(pdvector<EventRecord> &events) 
{
  static bool first_signal = true;
  DWORD milliseconds = INFINITE;
  EventRecord ev;

  waitingForOS_ = true;
  __UNLOCK;
  bool result = WaitForDebugEvent(&ev.info, milliseconds);
  __LOCK;
  waitingForOS_ = false;
  if (!result)
  {
    DWORD err = GetLastError();
    if ((WAIT_TIMEOUT == err) || (ERROR_SEM_TIMEOUT == err)) {
      //  apparently INFINITE milliseconds returns with SEM_TIMEOUT
      //  This may be a problem, but it doesn't seem to break anything.
      ev.type = evtTimeout;
      events.push_back(ev);
      return true;
    }else {
      printSysError(err);
      fprintf(stderr, "%s[%d]:  Unexpected error from WaitForDebugEvent: %d\n",
              __FILE__, __LINE__, err);
    }
    stopThreadNextIter();
    return false;
  }

  process *proc = process::findProcess(ev.info.dwProcessId);
  if (proc == NULL) {
     /* this case can happen when we create a process, but then are unable
        to parse the symbol table, and so don't complete the creation of the
        process. We just ignore the event here.  */
     ContinueDebugEvent(ev.info.dwProcessId, ev.info.dwThreadId, DBG_CONTINUE);
     ev.type = evtNullEvent;
     events.push_back(ev);
     return true;
   }

   ev.proc = proc;
   dyn_thread *thr = proc->getThread(ev.info.dwThreadId);
   ev.lwp = NULL;
   if (thr) {
       ev.lwp = thr->get_lwp();
       proc->set_lwp_status(ev.lwp, stopped);
   }
   if (!ev.lwp && ev.proc->getRepresentativeLWP() &&
       ev.info.dwDebugEventCode != CREATE_THREAD_DEBUG_EVENT) 
   {
       //Happens during process startup
       ev.lwp = ev.proc->getRepresentativeLWP();
       proc->set_lwp_status(ev.lwp, stopped);
   }
   if (!ev.lwp) {
       //Happens during thread creation events
       // the status will be set to stopped when we create
       // the new lwp later.
       ev.lwp = ev.proc->getInitialLwp();
   }

   signal_printf("[%s:%u] - Got event %d on %d (%d)\n", __FILE__, __LINE__, 
           ev.info.dwDebugEventCode, ev.lwp->get_fd(), ev.info.dwThreadId);

   Frame af = ev.lwp->getActiveFrame();
   ev.address = (Address) af.getPC();

   events.push_back(ev);
   return true;
}

bool SignalGenerator::decodeEvents(pdvector<EventRecord> &events) {
    bool result = true;
    for (unsigned i=0; i<events.size(); i++) {
        if (!decodeEvent(events[i]))
            result = false;
    }
    return result;
}

bool SignalGenerator::decodeEvent(EventRecord &ev)
{
	if (ev.address == 0x1052ed19) {
		int i = 3;
	}
	if (ev.info.dwDebugEventCode == 0x1) {
		int i = 3;
	}
   bool ret = false;
   switch (ev.info.dwDebugEventCode) {
     case EXCEPTION_DEBUG_EVENT:
        //ev.type = evtException;
        ev.what = ev.info.u.Exception.ExceptionRecord.ExceptionCode;
        ret = decodeException(ev);
		assert(ev.type != evtUndefined);
        break;
     case CREATE_THREAD_DEBUG_EVENT:
        ev.type = evtThreadCreate;
        ret = true;
        break;
     case CREATE_PROCESS_DEBUG_EVENT:
        ev.type = evtProcessCreate;
        ret = true;
        break;
     case EXIT_THREAD_DEBUG_EVENT:
        ev.type = evtThreadExit;
        requested_wait_until_active = true;
        ret = true;
        break;
     case EXIT_PROCESS_DEBUG_EVENT:
        ev.type = evtProcessExit;
        ev.what = ev.info.u.ExitProcess.dwExitCode;
        ev.status = statusNormal;
        requested_wait_until_active = true;
        ret = true;
        break;
     case LOAD_DLL_DEBUG_EVENT:
        ev.type = evtLoadLibrary;
        ev.what = SHAREDOBJECT_ADDED;
        ret = true;
        break;
     case UNLOAD_DLL_DEBUG_EVENT:
         signal_printf("WaitForDebugEvent returned UNLOAD_DLL_DEBUG_EVENT\n");
         ev.type = evtUnloadLibrary;
         ev.what = SHAREDOBJECT_REMOVED;
         ret = true;
         break;
   case OUTPUT_DEBUG_STRING_EVENT:
       ev.type = evtNullEvent;
       if (ev.info.u.DebugString.fUnicode == false && ev.info.u.DebugString.nDebugStringLength > 0) {
           int buflen = (ev.info.u.DebugString.nDebugStringLength < 512) ? 
               ev.info.u.DebugString.nDebugStringLength : 512;
           char *buf = (char*) malloc(buflen);
           if (proc->readDataSpace(ev.info.u.DebugString.lpDebugStringData, 
                                   buflen, buf, true)) {
               signal_printf("Captured OUTPUT_DEBUG_STRING_EVENT, debug string = %s\n", buf);
           }
           free (buf);
       }
       break;
     default: // RIP_EVENT or unknown event
        fprintf(stderr, "%s[%d]:  WARN:  unknown debug event=0x%x\n", FILE__, __LINE__, ev.info.dwDebugEventCode);
        ev.type = evtNullEvent;
        ret = true;
        break;
   };

   // Due to NT's odd method, we have to call pause_
   // directly (a call to pause returns without doing anything
   // pre-initialization)
   if (!requested_wait_until_active) {
      bool success = SuspendThreadFromEvent(&(ev.info), ev.lwp);
      if (success) {
         if (!ContinueDebugEvent(ev.info.dwProcessId, ev.info.dwThreadId, DBG_CONTINUE)) {
           printf("ContinueDebugEvent failed\n");
           printSysError(GetLastError());
         }
      }
   }
   assert(ev.type != evtUndefined);
  return ret;
}

static void decodeHandlerCallback(EventRecord &ev)
{
    ev.address = (eventAddress_t) 
       ev.info.u.Exception.ExceptionRecord.ExceptionAddress;

    // see if a signalhandler callback is registered
    pdvector<CallbackBase *> callbacks;
    SignalHandlerCallback *sigHandlerCB = NULL;
    if (getCBManager()->dispenseCallbacksMatching(evtSignalHandlerCB, callbacks)
       && ev.address != ((SignalHandlerCallback*)callbacks[0])->getLastSigAddr()
       && ((SignalHandlerCallback*)callbacks[0])->handlesSignal(ev.what)) 
    {
       ev.type = evtSignalHandlerCB;
    }
    else {// no handler is registered, return to signal to program 

       if ( EXCEPTION_ILLEGAL_INSTRUCTION == ev.what || 
            EXCEPTION_ACCESS_VIOLATION    == ev.what    ) 
       {
           Frame af = ev.lwp->getActiveFrame();
           signal_printf
               ("DECODE CRITICAL -- ILLEGAL INSN OR ACCESS VIOLATION\n");
           ev.type = evtCritical;
       }
       else {
           ev.type = evtSignalled;
       }
    }
}

extern std::set<Address> suicideAddrs;

bool SignalGenerator::decodeBreakpoint(EventRecord &ev) 
{
  char buf[128];
  bool ret = false;
  process *proc = ev.proc;
  if (decodeIfDueToProcessStartup(ev)) {
	  ret = true;
  }
  else if (proc->getRpcMgr()->decodeEventIfDueToIRPC(ev)) {
     signal_printf("%s[%d]:  BREAKPOINT due to RPC\n", FILE__, __LINE__);
	 ret = true;
  }
  else if (proc->trapMapping.definesTrapMapping(ev.address)) {
     ev.type = evtInstPointTrap;
     Frame activeFrame = ev.lwp->getActiveFrame();
#if 0
	 cerr << "SPRINGBOARD FRAME: " << hex << activeFrame.getPC() << " / " <<activeFrame.getSP() 
                 << " (DEBUG:" 
                 << "EAX: " << activeFrame.eax
                 << ", ECX: " << activeFrame.ecx
                 << ", EDX: " << activeFrame.edx
                 << ", EBX: " << activeFrame.ebx
                 << ", ESP: " << activeFrame.esp
                 << ", EBP: " << activeFrame.ebp
                 << ", ESI: " << activeFrame.esi 
                 << ", EDI " << activeFrame.edi
				 << ", EFLAGS: " << activeFrame.eflags << ")" << dec << endl;
	 for (unsigned i = 0; i < 10; ++i) {
			Address stackTOPVAL =0;
		    ev.proc->readDataSpace((void *) (activeFrame.esp + 4*i), sizeof(ev.proc->getAddressWidth()), &stackTOPVAL, false);
			cerr << "\tSTACK TOP VALUE=" << hex << stackTOPVAL << dec << endl;
	 }
#endif
	 ret = true;
  }
  else if (decodeRTSignal(ev)) {
	  ret = true;
  }
  else if (BPatch_defensiveMode == ev.proc->getHybridMode()) {
     Frame activeFrame = ev.lwp->getActiveFrame();
     if (!ev.proc->inEmulatedCode(activeFrame.getPC() - 1)) {
        requested_wait_until_active = true;//i.e., return exception to mutatee
        decodeHandlerCallback(ev);
     }
     else {
	    requested_wait_until_active = false;
        ret = true;
		ev.type = evtIgnore;
		static bool debug1 = true;
		if (debug1)
		{
			cerr << "BREAKPOINT FRAME: " << hex <<  activeFrame.getUninstAddr() << " / " << activeFrame.getPC() << " / " <<activeFrame.getSP() 
				<< " (DEBUG:" 
				<< "EAX: " << activeFrame.eax
				<< ", ECX: " << activeFrame.ecx
				<< ", EDX: " << activeFrame.edx
				<< ", EBX: " << activeFrame.ebx
				<< ", ESP: " << activeFrame.esp
				<< ", EBP: " << activeFrame.ebp
				<< ", ESI: " << activeFrame.esi 
				<< ", EDI: " << activeFrame.edi
				<< ", EFLAGS: " << activeFrame.eflags << ")" << dec << endl;
			Address stackTOPVAL[200];
            ev.proc->readDataSpace((void *) activeFrame.esp, sizeof(ev.proc->getAddressWidth())*200, stackTOPVAL, false);
            for (int i = 0; i < 200; ++i) 
            {
                Address remapped = 0;
                vector<func_instance *> funcs;
                baseTrampInstance *bti;
				ev.proc->getAddrInfo(stackTOPVAL[i], remapped, funcs, bti);
				cerr  << hex << activeFrame.esp + 4*i << ": "  << stackTOPVAL[i] << ", orig @ " << remapped << " in " << funcs.size() << "functions" << dec << endl;
			}
		}
	 }
  }
  else {
	  ev.type = evtCritical;
     ret = true;
  }

  signal_printf("%s[%d]:  decodeSigTrap for %s, state: %s\n",
                FILE__, __LINE__, ev.sprint_event(buf),
                proc->getBootstrapStateAsString().c_str());

  return ret;
}

static bool decodeAccessViolation_defensive(EventRecord &ev, bool &wait_until_active)
{
    bool ret = false;
    wait_until_active = true;
    ev.address = (eventAddress_t) ev.info.u.Exception.ExceptionRecord.ExceptionAddress;
    Address violationAddr = 
        ev.info.u.Exception.ExceptionRecord.ExceptionInformation[1];
    mapped_object *obj = NULL;

    switch(ev.info.u.Exception.ExceptionRecord.ExceptionInformation[0]) {
    case 0: // bad read
        if (dyn_debug_malware) {
            Address origAddr = ev.address;
            vector<func_instance *> funcs;
            baseTrampInstance *bti = NULL;
            ev.proc->getAddrInfo(ev.address, origAddr, funcs, bti);
            mal_printf("bad read in pdwinnt.C %lx[%lx]=>%lx [%d]\n",
                       ev.address, origAddr, violationAddr,__LINE__);
            // detach so we can see what's going on 
            //ev.proc->detachProcess(true);
            pdvector<pdvector<Frame> >  stacks;
            if (!ev.proc->walkStacks(stacks)) {
                mal_printf("%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
                return false;
            }
            for (unsigned i = 0; i < stacks.size(); ++i) {
                pdvector<Frame> &stack = stacks[i];
                for (unsigned int j = 0; j < stack.size(); ++j) {
                    Address origPC = 0;
                    vector<func_instance*> dontcare1;
                    baseTrampInstance *dontcare2 = NULL;
                    ev.proc->getAddrInfo(stack[j].getPC(), origPC, dontcare1, dontcare2);
                    mal_printf("frame %d: %lx[%lx]\n", j, stack[j].getPC(), origPC);
                }
            }
            dyn_saved_regs regs;
            ev.lwp->getRegisters(&regs,false);
            printf("REGISTER STATE:\neax=%lx \necx=%lx \nedx=%lx \nebx=%lx \nesp=%lx \nebp=%lx \nesi=%lx "
                   "\nedi=%lx\n",regs.cont.Eax, regs.cont.Ecx, regs.cont.Edx, 
                   regs.cont.Ebx, regs.cont.Esp, regs.cont.Ebp, 
                   regs.cont.Esi, regs.cont.Edi);
        }
        break;

    case 1: {// bad write 
        Address origAddr = ev.address;
        vector<func_instance *> writefuncs;
        baseTrampInstance *bti = NULL;
        bool success = ev.proc->getAddrInfo(ev.address, origAddr, writefuncs, bti);
        if (dyn_debug_malware) {
            Address origAddr = ev.address;
			Address shadowAddr = 0;
			bool valid = false;
			boost::tie(valid, shadowAddr) = ev.proc->getMemEm()->translateBackwards(violationAddr);

			cerr << "Overwrite insn @ " << hex << origAddr << endl;
            vector<func_instance *> writefuncs;
            baseTrampInstance *bti = NULL;
            bool success = ev.proc->getAddrInfo(ev.address, origAddr, writefuncs, bti);
            if (success) {
                fprintf(stderr,"---%s[%d] overwrite insn at %lx[%lx] in "
                        "function\"%s\" [%lx], writing to %lx (%lx) \n",
                        FILE__,__LINE__, ev.address, origAddr,
						writefuncs.empty() ? "<NO FUNC>" : writefuncs[0]->get_name().c_str(), 
						writefuncs.empty() ? 0 : writefuncs[0]->get_address(), 
                        violationAddr, shadowAddr);
            } else { 
                fprintf(stderr,"---%s[%d] overwrite insn at %lx, not "
                        "contained in any range, writing to %lx \n",
                        __FILE__,__LINE__, ev.address, violationAddr);
            }
            dyn_saved_regs regs;
            ev.lwp->getRegisters(&regs,false);
            printf("REGISTER STATE:\neax=%lx \necx=%lx \nedx=%lx \nebx=%lx \nesp=%lx \nebp=%lx \nesi=%lx "
                   "\nedi=%lx\n",regs.cont.Eax, regs.cont.Ecx, regs.cont.Edx, 
                   regs.cont.Ebx, regs.cont.Esp, regs.cont.Ebp, 
                   regs.cont.Esi, regs.cont.Edi);
        }

        // ignore memory access violations originating in kernel32.dll 
        // (if not originating from an instrumented instruction)
        mapped_object *obj = ev.proc->findObject(origAddr);
        assert(obj);
        if ( BPatch_defensiveMode != obj->hybridMode() ) 
        {
            wait_until_active = false;
            ret = true;
            ev.type = evtIgnore;
            ev.lwp->changeMemoryProtections(
                violationAddr - (violationAddr % ev.proc->getMemoryPageSize()), 
                ev.proc->getMemoryPageSize(), 
                PAGE_EXECUTE_READWRITE, 
                false);
            break;
        }
        // it's a write to a page containing write-protected code if region
        // permissions don't match the current permissions of the written page
        obj = ev.proc->findObject(violationAddr);
        if (!obj && ev.proc->isMemoryEmulated()) {
            bool valid=false;
            Address orig=0;
            boost::tie(valid,orig) = ev.proc->getMemEm()->translateBackwards(violationAddr);
            if (valid) {
                violationAddr = orig;
                obj = ev.proc->findObject(violationAddr);
            }
        }
        if (obj) {
            using namespace SymtabAPI;
            Region *reg = obj->parse_img()->getObject()->
                findEnclosingRegion(violationAddr - obj->codeBase());
            pdvector<CallbackBase *> callbacks;
            if (reg && (reg->getRegionPermissions() == Region::RP_RW ||
                        reg->getRegionPermissions() == Region::RP_RWX  ) &&
                getCBManager()->dispenseCallbacksMatching
                    (evtCodeOverwrite, callbacks)) //checks for CBs, doesn't call them
                {
                    ev.info2 = reg;
                    ev.type = evtCodeOverwrite;
                    ret = true;
                    wait_until_active = false;
                }
            callbacks.clear();
        }
        else {
            fprintf(stderr,"%s[%d] WARNING, possible bug, write insn at "
                    "%lx wrote to %lx\n",
                    __FILE__,__LINE__,ev.address, violationAddr);
            // detach so we can see what's going on 
            //ev.proc->detachProcess(true);
        }
        break;
    }
    case 8: // no execute permissions
        fprintf(stderr, "ERROR: executing code that lacks executable "
                "permissions in pdwinnt.C at %lx, evt.addr=%lx [%d]\n",
                ev.address, violationAddr,__LINE__);
        ev.proc->detachProcess(true);
        assert(0);
        break;
    default:
        if (dyn_debug_malware) {
            Address origAddr = ev.address;
            vector<func_instance *> funcs;
            baseTrampInstance *bti = NULL;
            ev.proc->getAddrInfo(ev.address, origAddr, funcs, bti);
            mal_printf("weird exception in pdwinnt.C illegal instruction or "
                       "access violation w/ code (%lx) %lx[%lx]=>%lx [%d]\n",
                       ev.info.u.Exception.ExceptionRecord.ExceptionInformation[0],
                       ev.address, origAddr, violationAddr,__LINE__);
        }
        ev.proc->detachProcess(true);
        assert(0);
    }
    if (evtCodeOverwrite != ev.type && ev.proc->isMemoryEmulated()) {
        // see if we were executing in defensive code whose memory access 
        // would have been emulated
        Address origAddr = ev.address;
        vector<func_instance *> writefuncs;
        baseTrampInstance *bti = NULL;
        bool success = ev.proc->getAddrInfo(ev.address, origAddr, writefuncs, bti);
        mapped_object *faultObj = NULL;
        if (success) {
            faultObj = ev.proc->findObject(origAddr);
        }
        if (!faultObj || BPatch_defensiveMode == faultObj->hybridMode()) {
            // KEVINTODO: we're emulating the instruction, pop saved regs off of the stack and into the appropriate registers, 
            // KEVINTODO: signalHandlerEntry will have to fix up the saved context information on the stack 
            assert(1 || "stack imbalance and bad reg values resulting from incomplete memory emulation of instruction that caused a fault");
        }
    }
    return ret;
}

bool SignalGenerator::decodeException(EventRecord &ev) 
{
   bool ret = false;
   switch (ev.what) {
     case EXCEPTION_BREAKPOINT: 
        signal_printf("DECODE BREAKPOINT\n");
        ret = decodeBreakpoint(ev);
        break;
     case EXCEPTION_ILLEGAL_INSTRUCTION:
        signal_printf("ILLEGAL INSTRUCTION\n");
        mal_printf("ILLEGAL INSTRUCTION\n");
     case EXCEPTION_ACCESS_VIOLATION:
     {
         requested_wait_until_active = true;
         if ( BPatch_defensiveMode == ev.proc->getHybridMode() ) {
             ret = decodeAccessViolation_defensive
                        (ev,
                         requested_wait_until_active);
         } 
         break;
     }
     case EXCEPTION_SINGLE_STEP:
         signal_printf("SINGLE STEP\n");
         ev.type = evtDebugStep;
         ev.address = (eventAddress_t) ev.info.u.Exception.ExceptionRecord.ExceptionAddress;
         ret = true;
         break;
     default:
         break;
   }

   // trigger callback if we haven't resolved the signal and a 
   // signalHandlerCallback is registered
   if (!ret) {
       requested_wait_until_active = true;//i.e., return exception to mutatee
       decodeHandlerCallback(ev);
       ret = true;
   }

   return ret;
}

bool SignalGeneratorCommon::decodeRTSignal_NP(EventRecord &ev, 
                                              Address rt_arg, int status)
{
    // windows uses ev.info for the DEBUG_EVENT struct, so we
    // shanghai the fd field instead
    ev.fd = (eventFileDesc_t) rt_arg;

    switch(status) {
    case DSE_snippetBreakpoint:
        ev.type = evtProcessStop;
        return true;
    case DSE_stopThread: 
        ev.type = evtStopThread;
        return true; 
    default:
        assert(0);
        return false;
    }
}

bool SignalGenerator::decodeSyscall(EventRecord &) 
{
    return false;
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
}

bool process::setProcessFlags() {
    return true;
}

bool process::unsetProcessFlags() {
    return true;
}


/* continue a process that is stopped */
bool dyn_lwp::continueLWP_(int /*signalToContinueWith*/) {
   unsigned count;
   signal_printf("[%s:%u] - continuing %d\n", __FILE__, __LINE__, get_fd());
   count = ResumeThread((HANDLE)get_fd());
   if (count == (unsigned) -1) {
      fprintf(stderr, "[%s:%u] - Couldn't resume thread\n", __FILE__, __LINE__);
      printSysError(GetLastError());
      return false;
   } else
      return true;
}


/*
   terminate execution of a process
 */
terminateProcStatus_t process::terminateProc_()
{
    OS::osKill(getPid());
    return terminateSucceeded;
}

/*
   pause a process that is running
*/
bool dyn_lwp::stop_() {
   unsigned count = 0;
   count = SuspendThread((HANDLE)get_fd());
   if (count == (unsigned) -1)
      return false;
   else
      return true;
}

bool process::dumpCore_(const std::string) {
    return false;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   return writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
   return writeDataSpace(inTraced, amount, inSelf);
}

bool process::flushInstructionCache_(void *baseAddr, size_t size){ //ccw 25 june 2001
   dyn_lwp *replwp = getInitialLwp();
   return FlushInstructionCache((HANDLE)replwp->getProcessHandle(), baseAddr, size);
}

bool dyn_lwp::readTextSpace(const void *inTraced, u_int amount, void *inSelf) {
   return readDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int amount, const void *inSelf)
{
    DWORD nbytes;
    handleT procHandle = getProcessHandle();
    bool res = WriteProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
				  (LPVOID)inSelf, (DWORD)amount, &nbytes);
    if (BPatch_defensiveMode == proc()->getHybridMode() && 
        !res || nbytes != amount) 
    {
        // the write may have failed because we've removed write permissions
        // from the page, remove them and try again

        int oldRights = changeMemoryProtections((Address)inTraced, amount, 
                                                PAGE_EXECUTE_READWRITE,true);
        if (oldRights == PAGE_EXECUTE_READ || oldRights == PAGE_READONLY) {
            res = WriteProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
                                     (LPVOID)inSelf, (DWORD)amount, &nbytes);
        }
        if (oldRights != -1) { // set the rights back to what they were
            changeMemoryProtections((Address)inTraced, amount, oldRights,true);
        }
    }

    return res && (nbytes == amount);
}


bool dyn_lwp::readDataSpace(const void *inTraced, u_int amount, void *inSelf) {
    if ((Address)inTraced <= 0xc30000 && ((Address)inTraced + amount) >= 0xc30000) {
        cerr << "readDataSpace [" << hex << (Address) inTraced << "," << (Address) inTraced + amount << "]" << dec << endl;
    }
    DWORD nbytes;
    handleT procHandle = getProcessHandle();
    bool res = ReadProcessMemory((HANDLE)procHandle, (LPVOID)inTraced, 
				 (LPVOID)inSelf, (DWORD)amount, &nbytes);
	if (!res && (GetLastError() == 299)) // Partial read success...
	{
		// Loop and copy piecewise
		Address start = (Address) inTraced;
		Address cur = start;
		Address end = start + amount;
		Address bufStart = (Address) inSelf;
		Address bufCur = bufStart;
		Address bufEnd = bufStart + amount;

		cerr << "Starting piecewise copy [" << hex << start << "," << end << dec << "]" << endl;

		MEMORY_BASIC_INFORMATION meminfo;
		memset(&meminfo, 0, sizeof(MEMORY_BASIC_INFORMATION));
		do {
			VirtualQueryEx(procHandle,
				(LPCVOID) cur, 
				&meminfo,
				sizeof(MEMORY_BASIC_INFORMATION));
			cerr << "\t VirtualQuery returns base " << hex
				<< (Address) meminfo.AllocationBase << " and pages range [" 
				<< (Address) meminfo.BaseAddress << "," << ((Address)meminfo.BaseAddress) + meminfo.RegionSize 
				<< dec << "]" << endl;
			unsigned remaining = end - cur;
			assert(remaining == (bufEnd - bufCur));
			unsigned toCopy = (remaining < meminfo.RegionSize) ? remaining : meminfo.RegionSize;
			if (meminfo.State == MEM_COMMIT) {
				cerr << "\t Copying range [" << hex << cur << "," << cur + toCopy << "]" << dec << endl;
				bool res = ReadProcessMemory(procHandle, (LPVOID) cur, (LPVOID) bufCur, (DWORD) toCopy, &nbytes);
				assert(res);
			}
			else {
				cerr << "\t Zeroing range [" << hex << cur << "," << cur + toCopy << dec << "]" << endl;
				memset((void *)bufCur, 0, toCopy);
			}
			cur += toCopy;
			bufCur += toCopy;
		} while (bufCur < bufEnd);
		return true;
	}
    return res && (nbytes == amount);
}

bool process::setMemoryAccessRights
(Address start, Address size, int rights)
{
    mal_printf("setMemoryAccessRights to %x [%lx %lx]\n", rights, start, start+size);
    // get lwp from which we can call changeMemoryProtections
    dyn_lwp *stoppedlwp = query_for_stopped_lwp();
    if ( ! stoppedlwp ) {
        assert(0); //KEVINTODO: I don't think this code is right, it doesn't resume the stopped lwp
        bool wasRunning = true;
        stoppedlwp = stop_an_lwp(&wasRunning);
        if ( ! stoppedlwp ) {
        return false;
        }
    }
    if (PAGE_EXECUTE_READWRITE == rights || PAGE_READWRITE == rights) {
        mapped_object *obj = findObject(start);
        int page_size = getMemoryPageSize();
        for (Address cur = start; cur < (start + size); cur += page_size) {
            obj->removeProtectedPage(start -(start % page_size));
        }
    }
    stoppedlwp->changeMemoryProtections(start, size, rights, true);
    return true;
}

bool process::getMemoryAccessRights(Address start, Address size, int rights)
{
   assert(0 && "Unimplemented!");
}

int dyn_lwp::changeMemoryProtections
(Address addr, Offset size, unsigned rights, bool setShadow)
{
    unsigned oldRights=0;
    unsigned pageSize = proc()->getMemoryPageSize();

	Address pageBase = addr - (addr % pageSize);
	size += (addr % pageSize);

	// Temporary: set on a page-by-page basis to work around problems
	// with memory deallocation
	for (Address idx = pageBase; idx < pageBase + size; idx += pageSize) {
        //mal_printf("setting rights to %lx for [%lx %lx)\n", 
          //         rights, idx , idx + pageSize);
		if (!VirtualProtectEx((HANDLE)getProcessHandle(), (LPVOID)(idx), 
			(SIZE_T)pageSize, (DWORD)rights, (PDWORD)&oldRights)) 
		{
			fprintf(stderr, "ERROR: failed to set access rights for page %lx, error code %d "
				"%s[%d]\n", addr, GetLastError(), FILE__, __LINE__);
			MEMORY_BASIC_INFORMATION meminfo;
			SIZE_T size = VirtualQueryEx(getProcessHandle(), (LPCVOID) (addr), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
			fprintf(stderr, "ERROR DUMP: baseAddr 0x%lx, AllocationBase 0x%lx, AllocationProtect 0x%lx, RegionSize 0x%lx, State 0x%lx, Protect 0x%lx, Type 0x%lx\n",
				meminfo.BaseAddress, meminfo.AllocationBase, meminfo.AllocationProtect, meminfo.RegionSize, meminfo.State, meminfo.Protect, meminfo.Type);
		}
		else if (proc()->isMemoryEmulated() && setShadow) {
			Address shadowAddr = 0;
			unsigned shadowRights=0;
			bool valid = false;
			boost::tie(valid, shadowAddr) = proc()->getMemEm()->translate(idx);
			if (!valid) {
				fprintf(stderr, "WARNING: set access rights on page %lx that has "
					"no shadow %s[%d]\n",addr,FILE__,__LINE__);
			}
			else 
			{
				if (!VirtualProtectEx((HANDLE)getProcessHandle(), (LPVOID)(shadowAddr), 
					(SIZE_T)pageSize, (DWORD)rights, (PDWORD)&shadowRights)) 
				{
					fprintf(stderr, "ERROR: set access rights found shadow page %lx "
						"for page %lx but failed to set its rights %s[%d]\n",
						shadowAddr, addr, FILE__, __LINE__);
				}

				if (shadowRights != oldRights) {
					//mal_printf("WARNING: shadow page[%lx] rights %x did not match orig-page"
					//           "[%lx] rights %x\n",shadowAddr,shadowRights, addr, oldRights);
				}
			}
		}
	}
	return oldRights;
}


bool dyn_lwp::waitUntilStopped() {
   return true;
}

bool process::waitUntilStopped() {
   return true;
}

Frame dyn_lwp::getActiveFrame()
{
	w32CONTEXT cont; //ccw 27 july 2000 : 29 mar 2001

	Address pc = 0, fp = 0, sp = 0;

	// we must set ContextFlags to indicate the registers we want returned,
	// in this case, the control registers.
	// The values for ContextFlags are defined in winnt.h
	cont.ContextFlags = CONTEXT_FULL;
	if (GetThreadContext((HANDLE)get_fd(), &cont))
	{
		fp = cont.Ebp;
		pc = cont.Eip;
		sp = cont.Esp;
		Frame frame(pc, fp, sp, proc_->getPid(), proc_, NULL, this, true);
                frame.eax = cont.Eax;
                frame.ebx = cont.Ebx;
                frame.ecx = cont.Ecx;
                frame.edx = cont.Edx;
                frame.esp = cont.Esp;
                frame.ebp = cont.Ebp;
		frame.esi = cont.Esi;
		frame.edi = cont.Edi;
		frame.eflags = cont.EFlags;

		return frame;
	}
	printSysError(GetLastError());
	return Frame();
}

// sets PC for stack frames other than the active stack frame
bool Frame::setPC(Address newpc) {

	if (!pcAddr_) {
		// if pcAddr isn't set it's because the stackwalk isn't getting the 
		// frames right
		fprintf(stderr,"WARNING: unable to change stack frame PC from %lx to %lx "
			"because we don't know where the PC is on the stack %s[%d]\n",
			pc_,newpc,FILE__,__LINE__);
		return false;
	}

	if (getProc()->writeDataSpace( (void*)pcAddr_, 
		getProc()->getAddressWidth(), 
		&newpc) ) 
	{
		this->pc_ = newpc;
		return true;
	}

	return false;
}

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs, bool includeFP) {
   // we must set ContextFlags to indicate the registers we want returned,
   // in this case, the control registers.
   // The values for ContextFlags are defined in winnt.h
   regs->cont.ContextFlags = w32CONTEXT_FULL;//ccw 27 july 2000 : 29 mar 2001
   handleT handle = get_fd();
   if (!GetThreadContext((HANDLE)handle, &(regs->cont)))
   {
      return false;
   }
   return true;
}

void dyn_lwp::dumpRegisters()
{
   dyn_saved_regs regs;
   if (!getRegisters(&regs)) {
     fprintf(stderr, "%s[%d]:  registers unavailable\n", FILE__, __LINE__);
     return;
   }
}

bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *regs)
{    
  if (dyn_debug_malware) {
      std::set<func_instance *> funcs;
      proc()->findFuncsByAddr(addr, funcs, true);
      cerr << "CHANGEPC to addr " << hex << addr;
      cerr << " to func " << (funcs.empty() ? "<UNKNOWN>" :
                              ((funcs.size() == 1) ? (*(funcs.begin()))->symTabName() : "<MULTIPLE>"));
      cerr << dec << endl;
      cerr << "Currently at: " << getActiveFrame();
  }
  w32CONTEXT cont;//ccw 27 july 2000
  if (!regs) {
      cont.ContextFlags = w32CONTEXT_FULL;//ccw 27 july 2000 : 29 mar 2001
      if (!GetThreadContext((HANDLE)get_fd(), &cont))
      {
          printf("GetThreadContext failed\n");
          return false;
      }
  }
  else {
      memcpy(&cont, &(regs->cont), sizeof(w32CONTEXT));
  }
  cont.Eip = addr;
  if (!SetThreadContext((HANDLE)get_fd(), &cont))
  {
    printf("SethreadContext failed\n");
    return false;
  }
  return true;
}

bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs, bool includeFP) {
  if (!SetThreadContext((HANDLE)get_fd(), &(regs.cont)))
  {
    //printf("SetThreadContext failed\n");
    return false;
  }
  return true;
}

bool process::isRunning_() const {
    // TODO
    return true;
}


std::string 
process::tryToFindExecutable(const std::string& iprogpath, int pid)
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

Address dyn_lwp::readRegister(Register reg)
{
   w32CONTEXT *cont = new w32CONTEXT;//ccw 27 july 2000 : 29 mar 2001
    if (!cont)
	return NULL;
    // we must set ContextFlags to indicate the registers we want returned,
    // in this case, the control registers.
    // The values for ContextFlags are defined in winnt.h
    cont->ContextFlags = w32CONTEXT_FULL;//ccw 27 july 2000
    if (!GetThreadContext((HANDLE)get_fd(), cont)) {
      delete cont;
	  return NULL;
    }
    return cont->Eax;
}


void InitSymbolHandler( HANDLE hProcess )
{
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

    CloseHandle(hProcess);
}

bool SignalGenerator::waitForStopInline()
{
   return true;
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
 *   ioLink: handle or file descriptor of io link (read only)
 *   pid: process id of new process
 *   tid: thread id for main thread (needed by WindowsNT)
 *   procHandle: handle for new process (needed by WindowsNT)
 *   thrHandle: handle for main thread (needed by WindowsNT)
 ****************************************************************************/
bool SignalGenerator::forkNewProcess()
{
    // create the child process    
    std::string args;
    for (unsigned ai=0; ai<argv_->size(); ai++) {
       args += (*argv_)[ai];
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
    if (CreateProcess(file_.c_str(), (char *)args.c_str(), 
		      NULL, NULL, TRUE,
		      DEBUG_PROCESS /* | CREATE_NEW_CONSOLE */ | CREATE_SUSPENDED | DEBUG_ONLY_THIS_PROCESS ,
		      NULL, dir_ == "" ? NULL : dir_.c_str(), 
		      &stinfo, &procInfo)) 
    {
                  procHandle = (Word)procInfo.hProcess;
                  thrHandle = (Word)procInfo.hThread;
                  pid_ = (Word)procInfo.dwProcessId;
                  tid = (Word)procInfo.dwThreadId;
                  traceLink_ = -1;
                  return true;    
    }
   
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
                     ) > 0) 
    {
      char *errorLine = (char *)malloc(strlen((char *)lpMsgBuf) +
                                       file_.length() + 64);
      if (errorLine != NULL) {
         sprintf(errorLine, "Unable to start %s: %s\n", file_.c_str(),
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
              file_.c_str());
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
           buf[i-1]=c[i];
       buf[i-1]='\0';
       stripAtSuffix(buf);
       if (buf[0] == '\0') return 0; // avoid null names which seem to annoy Paradyn
       return P_strdup(buf);
    } else {
       if (includeTypes) {
          if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE| UNDNAME_NO_ACCESS_SPECIFIERS|UNDNAME_NO_MEMBER_TYPE|UNDNAME_NO_MS_KEYWORDS)) {
            //   printf("Undecorate with types: %s = %s\n", c, buf);
            stripAtSuffix(buf);
            return P_strdup(buf);
          }
       }  else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_NAME_ONLY)) {
         //     else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE|UNDNAME_32_BIT_DECODE)) {
         //	printf("Undecorate: %s = %s\n", c, buf);
         stripAtSuffix(buf);          
         return P_strdup(buf);
       }
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

bool SignalGeneratorCommon::getExecFileDescriptor(std::string filename,
                                    int pid,
                                    bool,
                                    int &status,
                                    fileDescriptor &desc)
{
    assert(proc);
    dyn_lwp *rep_lwp = proc->getRepresentativeLWP();
    assert(rep_lwp);  // the process based lwp should already be set

    if (proc->processHandle_ == INVALID_HANDLE_VALUE) {

        if (!proc->wasCreatedViaAttach()) {
           int res = ResumeThread(proc->sh->getThreadHandle());
           if (res == -1) {
             fprintf(stderr, "%s[%d]:  could not resume thread here\n", FILE__, __LINE__);
             printSysError(GetLastError());
           }
        }

       //  need to snarf up the next debug event, at which point we can get 
       //  a handle to the debugged process.

       DEBUG_EVENT snarf_event;
       timed_out_retry:

       if (!WaitForDebugEvent(&snarf_event, INFINITE))
       {
         fprintf(stderr, "%s[%d][%s]:  WaitForDebugEvent returned\n", 
               FILE__, __LINE__, getThreadStr(getExecThreadID()));
         DWORD err = GetLastError();
         if ((WAIT_TIMEOUT == err) || (ERROR_SEM_TIMEOUT == err)) {
           //  apparently INFINITE milliseconds returns with SEM_TIMEOUT
           //  This may be a problem, but it doesn't seem to break anything.
           goto timed_out_retry;
         }else {
           printSysError(err);
           fprintf(stderr, "%s[%d]:  Unexpected error from WaitForDebugEvent: %d\n",
                   __FILE__, __LINE__, err);
         }
         return false;
       }

       //  Now snarf_event should have the right handles set...

       proc->processHandle_ = snarf_event.u.CreateProcessInfo.hProcess;
       proc->mainFileHandle_ = snarf_event.u.CreateProcessInfo.hFile;
       proc->mainFileBase_ = (Address)snarf_event.u.CreateProcessInfo.lpBaseOfImage;
       proc->sh->thrHandle = (int) snarf_event.u.CreateProcessInfo.hThread;
       proc->sh->procHandle = (int) snarf_event.u.CreateProcessInfo.hProcess;
       char *imageName = (char *) snarf_event.u.CreateProcessInfo.lpImageName;

       rep_lwp->setFileHandle(snarf_event.u.CreateProcessInfo.hThread);
       if (NULL == snarf_event.u.CreateProcessInfo.hThread)
         assert(0);
       rep_lwp->setProcessHandle(snarf_event.u.CreateProcessInfo.hProcess);

       if (proc->threads.size() == 0) {
           dyn_thread *t = new dyn_thread(proc, 
                                          0, // POS (main thread is always 0)
                                          rep_lwp);
           t->update_tid(snarf_event.dwThreadId);
       }
    
       //This must be called on each process in order to use the 
       // symbol/line-info reading API
       bool result = SymInitialize(proc->processHandle_, NULL, FALSE);
       if (!result) {
           fprintf(stderr, "Couldn't SymInitialize\n");
           printSysError(GetLastError());
       } 
       DWORD64 iresult = SymLoadModule64(proc->processHandle_, proc->mainFileHandle_,
                                    imageName, NULL,
                                    (DWORD64) proc->mainFileBase_, 0);
       /*
       int res = ResumeThread((HANDLE) proc->sh->thrHandle);
       if (res == -1) {
          fprintf(stderr, "%s[%d]:  could not resume thread here\n", FILE__, __LINE__);
          printSysError(GetLastError());
       }
*/
       if (!ContinueDebugEvent(snarf_event.dwProcessId, 
                               snarf_event.dwThreadId, DBG_CONTINUE))
       {
         DebugBreak();
         printf("ContinueDebugEvent failed\n");
         printSysError(GetLastError());
         return false;
       }

       proc->set_status(running);
    }

    desc = fileDescriptor(filename.c_str(), 
                        (Address) 0,
                        (HANDLE) proc->processHandle_,
                        (HANDLE) proc->mainFileHandle_, 
                        false,
                        (Address) proc->mainFileBase_);
    return true;
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
std::string GetLoadedDllImageName( process* p, const DEBUG_EVENT& ev )
{
    char *msgText = NULL;
	std::string ret;
	void* pImageName = NULL;

	if( ev.u.LoadDll.lpImageName != NULL )
	{
        msgText = new char[1024];	// buffer for error messages
	    // On non-CE flavors of Windows, the address given in the debug
        // event struct is the address of a pointer to the DLL name string.

        if( !p->readDataSpace( ev.u.LoadDll.lpImageName, 4, &pImageName, false ) )
        {
            sprintf( msgText, "Failed to read DLL image name pointer: %d\n",
            GetLastError() );
            logLine( msgText );
	    }
    }
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
		if (kludge_isKernel32Dll(ev.u.LoadDll.hFile, ret))
            return ret;

        //I'm embarassed to be writing this.  We didn't get a name for the image, 
        // but we did get a file handle.  According to MSDN, the best way to turn
        // a file handle into a file name is to map the file into the address space
        // (using the handle), then ask the OS what file we have mapped at that location.
        // I'm sad now.
        
        void *pmap = NULL;
        HANDLE fmap = CreateFileMapping(ev.u.LoadDll.hFile, NULL, 
                                        PAGE_READONLY, 0, 1, NULL);
        if (fmap) {
            pmap = MapViewOfFile(fmap, FILE_MAP_READ, 0, 0, 1);
            if (pmap) {   
                char filename[MAX_PATH+1];
                int result = GetMappedFileName(GetCurrentProcess(), pmap, filename, MAX_PATH);
                if (result)
                    ret = std::string(filename);
                UnmapViewOfFile(pmap);
            }
            CloseHandle(fmap);
        }
	}

	if (ret.substr(0,7) == "\\Device") {
      HANDLE currentProcess = p->processHandle_;
      DWORD num_modules_needed;
      int errorCheck = EnumProcessModules(currentProcess,
                                          NULL,
                                          0,
                                          &num_modules_needed);
	  num_modules_needed /= sizeof(HMODULE);
      HMODULE* loadedModules = new HMODULE[num_modules_needed];
      errorCheck = EnumProcessModules(currentProcess,
                                          loadedModules,
                                          sizeof(HMODULE)*num_modules_needed,
                                          &num_modules_needed);
      HMODULE* candidateModule = loadedModules; 
      while(candidateModule < loadedModules + num_modules_needed)
      {
         MODULEINFO candidateInfo;
         GetModuleInformation(currentProcess, *candidateModule, &candidateInfo,
                              sizeof(candidateInfo));
         if(ev.u.LoadDll.lpBaseOfDll == candidateInfo.lpBaseOfDll)
            break;
         candidateModule++;
      }
      if(candidateModule != loadedModules + num_modules_needed) 
      {
         TCHAR filename[MAX_PATH];
         if(GetModuleFileNameEx(currentProcess, *candidateModule, filename, MAX_PATH))
         {
            ret = filename;
         }
      }
      delete[] loadedModules;

	}
	// cleanup
    if (msgText)
        delete[] msgText;

	return ret;
}

bool dyn_lwp::realLWP_attach_() {
   return true;
}

bool dyn_lwp::representativeLWP_attach_() {
    if(proc_->wasCreatedViaAttach()) {
        if (!DebugActiveProcess(getPid())) {
            //printf("Error: DebugActiveProcess failed\n");
            return false;
        }
    }
    
    // We either created this process, or we have just attached it.
    // In either case, our descriptor already has a valid process handle.
    setProcessHandle(proc()->processHandle_);
    proc()->set_lwp_status(this, stopped);

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
  mapped_object *aout = getAOut();
  SymtabAPI::Symtab *aout_obj = aout->parse_img()->getObject();
  pdvector<func_instance *> funcs;
  Address min_addr = 0xffffffff;
  Address max_addr = 0x0;
  bool result;
  unsigned char oldbyte;
  const unsigned char trapInsn = 0xcc;
  startup_printf("[%s:%u] - Asked to insert bp at entry point of main\n", 
      __FILE__, __LINE__);
  
  if (main_function) {
	  //Address addr = main_function->getAddress() - aout_obj->getBaseAddress()+ aout->getFileDesc().loadAddr();
     Address addr = main_function->getAddress();
     startup_printf("[%s:%u] - insertTrapAtEntryPointOfMain found main at %x\n",
                    __FILE__, __LINE__, addr);
     result = readDataSpace((void *) addr, sizeof(trapInsn), &oldbyte, false);
     if (!result) {
         fprintf(stderr, "Internal Error - Couldn't write breakpoint at top of main\n");
         return false;
     }
     assert (oldbyte != trapInsn);
     writeDataSpace((void *) addr, sizeof(trapInsn), (void *) &trapInsn);
     main_breaks[addr] = oldbyte;
     flushInstructionCache_((void *) addr, 1);
     return true;
  }


  if (max_addr >= min_addr)
    flushInstructionCache_( (void*)min_addr, max_addr - min_addr + 1 );
  return true;
}

// True if we hit the trap at the entry of main
bool process::trapAtEntryPointOfMain(dyn_lwp *lwp, Address trapAddr) {
    if (getBootstrapState() < begun_bs || getBootstrapState() > loadingRT_bs) return false;
    if (!main_breaks.defines(trapAddr)) return false;

    startup_printf("[%s:%u] - Hit possible main breakpoint at %x:\n", __FILE__, __LINE__, trapAddr);

    //Set the last function we hit as a possible main
    /*if (!main_function) {
       main_function = this->findFuncByAddr(trapAddr);
    }*/
    main_brk_addr = trapAddr;

    return true;
}

// Clean up after breakpoint in main() is hit
bool process::handleTrapAtEntryPointOfMain(dyn_lwp *lwp)
{
    //Remove this trap
    dictionary_hash<Address, unsigned char>::iterator iter = main_breaks.begin();
    Address min_addr = 0xffffffff;
    Address max_addr = 0x0;
    for (; iter != main_breaks.end(); iter++) {
        Address addr = iter.currkey();
        unsigned char value = *(iter);

        bool result = writeDataSpace((void *) addr, sizeof(unsigned char), &value);
        if (!result) {
            fprintf(stderr, "Unexpected Error.  Couldn't remove breakpoint from "
                    "potential main at %x\n", addr);
            continue;   
        }
        if (max_addr < addr)
            max_addr = addr;
        if (min_addr > addr)
            min_addr = addr;
    }
    main_breaks.clear();

    //Restore PC and flush instruction cache
    flushInstructionCache_((void *)min_addr, max_addr - min_addr + 1);
    lwp->changePC(main_brk_addr, NULL);

    setBootstrapState(initialized_bs);
    return true;
}

bool process::handleTrapAtLibcStartMain(dyn_lwp *)  { assert(0); return false; }
bool process::instrumentLibcStartMain() { assert(0); return false; }
bool process::decodeStartupSysCalls(EventRecord &) { assert(0); return false; }
void process::setTraceSysCalls(bool) { assert(0); }
void process::setTraceState(traceState_t) { assert(0); }
bool process::getSysCallParameters(dyn_saved_regs *, long *, int) { assert(0); return false; }
int process::getSysCallNumber(dyn_saved_regs *) { assert(0); return -1; }
long process::getSysCallReturnValue(dyn_saved_regs *) { assert(0); return -1; }
Address process::getSysCallProgramCounter(dyn_saved_regs *) { assert(0); return 0; }
bool process::isMmapSysCall(int) { assert(0); return false; }
Offset process::getMmapLength(int, dyn_saved_regs *) { assert(0); return 0; }
Address process::getLibcStartMainParam(dyn_lwp *) { assert(0); return 0; }

bool AddressSpace::getDyninstRTLibName() {
    // Set the name of the dyninst RT lib
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            std::string msg = std::string("Environment variable ") +
               std::string("DYNINSTAPI_RT_LIB") +
               std::string(" has not been defined");
            showErrorCallback(101, msg);
            return false;
        }
    }
    //Canonicalize name
    char *sptr = P_strdup(dyninstRT_name.c_str());
    for (unsigned i=0; i<strlen(sptr); i++)
       if (sptr[i] == '/') sptr[i] = '\\';
    dyninstRT_name = sptr;
    free(sptr);
           
    if (_access(dyninstRT_name.c_str(), 04)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name +
                       std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }

    return true;
}


// Load the dyninst library
bool process::loadDYNINSTlib()
{
    loadDyninstLibAddr = getAOut()->parse_img()->getObject()->getEntryOffset() + getAOut()->getBaseAddress();
    Address LoadLibAddr;
    int_symbol sym;
    
    dyn_lwp *lwp;
    lwp = getInitialLwp();
 /*   if (lwp->status() == running) {
       lwp->pauseLWP();
    }*/

    if (!getSymbolInfo("_LoadLibraryA@4", sym) &&
        !getSymbolInfo("_LoadLibraryA", sym) &&
        !getSymbolInfo("LoadLibraryA", sym))
        {
            printf("unable to find function LoadLibrary\n");
            assert(0);
        }
    LoadLibAddr = sym.getAddr();
    assert(LoadLibAddr);

    char ibuf[BYTES_TO_SAVE];
    memset(ibuf, '\0', BYTES_TO_SAVE);//ccw 25 aug 2000
    char *iptr = ibuf;
    strcpy(iptr, dyninstRT_name.c_str());
    
    // Code overview:
    // Dynininst library name
    //    Executable code begins here:
    // Push (address of dyninst lib name)
    // Call LoadLibrary
    // Pop (cancel push)
    // Trap
    
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
    *(int *)iptr = loadDyninstLibAddr; // string at codeBase
    iptr += sizeof(int);
    
    int offsetFromBufferStart = (int)iptr - (int)ibuf;
    offsetFromBufferStart += 5; // Skip next instruction as well.
    // call LoadLibrary ; 5 bytes
    *iptr++ = (char)0xe8;
    
    // Jump offset is relative
    *(int *)iptr = LoadLibAddr - (loadDyninstLibAddr + 
                                  offsetFromBufferStart); // End of next instruction
    iptr += sizeof(int);
    
    
    // add sp, 4 (Pop)
    *iptr++ = (char)0x83; *iptr++ = (char)0xc4; *iptr++ = (char)0x04;
    
    // int3
    *iptr = (char)0xcc;
    
    int offsetToTrap = (int) iptr - (int) ibuf;

    readDataSpace((void *)loadDyninstLibAddr, BYTES_TO_SAVE, savedCodeBuffer, false);
    writeDataSpace((void *)loadDyninstLibAddr, BYTES_TO_SAVE, ibuf);
    
    flushInstructionCache_((void *)loadDyninstLibAddr, BYTES_TO_SAVE);
    
    dyninstlib_brk_addr = loadDyninstLibAddr + offsetToTrap;
    
    savedRegs = new dyn_saved_regs;

    bool status = lwp->getRegisters(savedRegs);
    assert(status == true);    
    lwp->changePC(loadDyninstLibAddr + instructionOffset, NULL);
    
    setBootstrapState(loadingRT_bs);
    return true;
}



// Not used on NT. We'd have to rewrite the
// prototype to take a PC. Handled inline.
// True if trap is from dyninst load finishing
bool process::trapDueToDyninstLib(dyn_lwp *lwp) 
{
    if (!dyninstlib_brk_addr)
       return false;
    assert(lwp);
    Frame active = lwp->getActiveFrame();
    if (active.getPC() == dyninstlib_brk_addr ||
        (active.getPC()-1) == dyninstlib_brk_addr)
        return true;
    return false;
}



// Cleanup after dyninst lib loaded
bool process::loadDYNINSTlibCleanup(dyn_lwp *)
{
    // First things first: 
    assert(savedRegs != NULL);
    getInitialLwp()->restoreRegisters(*savedRegs);
    delete savedRegs;
    savedRegs = NULL;

    writeDataSpace((void *) loadDyninstLibAddr,
                   BYTES_TO_SAVE,
                   (void *)savedCodeBuffer);

    flushInstructionCache_((void *)getAOut()->codeAbs(), BYTES_TO_SAVE);

    dyninstlib_brk_addr = 0;

    return true;
}

void loadNativeDemangler() 
{
    // ensure we load line number information when we load
    // modules, and give us mangled names
    DWORD dwOpts = SymGetOptions();
    dwOpts &= ~(SYMOPT_UNDNAME);
    dwOpts |= SYMOPT_LOAD_LINES;
    dwOpts &= ~(SYMOPT_DEFERRED_LOADS);
    SymSetOptions(dwOpts);
}


Frame dyn_thread::getActiveFrameMT() {
   return get_lwp()->getActiveFrame();
}

bool process::determineLWPs(pdvector<unsigned> &lwp_ids)
{
  dyn_lwp *lwp;
  unsigned index;

  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  while (lwp_iter.next(index, lwp)) {
	  if (!lwp->isDebuggerLWP()) {
      lwp_ids.push_back(lwp->get_lwp_id());
  }
  }
  return true;
}

bool process::initMT()
{
   return true;
}

void dyninst_yield()
{
    SwitchToThread();
}

void OS::make_tempfile(char *name) {
}

bool OS::execute_file(char *file) {
   STARTUPINFO s;
   PROCESS_INFORMATION proc;
   BOOL result;

   ZeroMemory(&s, sizeof(s));
   ZeroMemory(&proc, sizeof(proc));
   s.cb = sizeof(s);

   result = CreateProcess(NULL, file, NULL, NULL, FALSE, 0, NULL, NULL, 
                          &s, &proc);
   if (!result) {
      fprintf(stderr, "Couldn't create %s - Error %d\n", file, GetLastError());
      return false;
   }

   WaitForSingleObject(proc.hProcess, INFINITE);
   CloseHandle(proc.hProcess);
   CloseHandle(proc.hThread);
   return true;
}

void OS::unlink(char *file) {
   DeleteFile(file);
}

#if !defined(TF_BIT)
#define TF_BIT 0x100
#endif

Address dyn_lwp::step_next_insn() {
   CONTEXT context;
   BOOL result;

   singleStepping = true;
   context.ContextFlags = CONTEXT_FULL;
   result = GetThreadContext((HANDLE)get_fd(), &context);
   if(!result) {
      fprintf(stderr, "[%s:%u - step_next_insn] - Couldn't get thread context ", 
              __FILE__, __LINE__);
      return (Address) -1;
   }

   context.ContextFlags = CONTEXT_FULL;
   context.EFlags |= TF_BIT ;
   if(!SetThreadContext((HANDLE)get_fd(), &context))
   if(!result) {
      fprintf(stderr, "[%s:%u - step_next_insn] - Couldn't set thread context ", 
              __FILE__, __LINE__);
      return (Address) -1;
   }
   
   continueLWP();

   do {
      if(proc()->hasExited()) 
         return (Address) -1;
      proc()->sh->waitForEvent(evtDebugStep);
   } while (singleStepping);

   return getActiveFrame().getPC();
}

#if defined (cap_dynamic_heap)
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                        inferiorHeapType /* type */ ) 
{
}
#endif

/**
 stdcall:
   * C Naming - Name prefixed by a '_', followed by the name, then an '@',
     followed by number of bytes in arguments.  
     i.e. foo(int a, double b) = _foo@12
   * C++ Naming - __stdcall
   * Args - Arguments are passed on the stack.
   * Cleanup - Callee cleans up the stack before returning
 cdecl:
   * C Naming - Name prefixed by a '_'
   * C++ Naming - __cdecl in demangled name
   * Args - Arguments are passed on the stack.
   * Cleanup - Caller cleans up the stack after the return
 fastcall:
   * C Naming - Name prefixed by a '@', followed by the func name, then 
     another '@', followed by the number of bytes in the arguments.  i.e.
     foo(double a, int b, int c, int d) = @foo@20
   * C++ Naming - __fastcall in the mangled name
   * Args - First two arguments that are less than DWORD size are passed in ECX & EDX
   * Cleanup - Callee cleans up the stack before returning
 thiscall:
   * C Naming - NA
   * C++ Naming - __thiscall in the demangled name
   * 'this' parameter is passed in ECX, others are passed in the stack
   * Cleanup Callee cleans up the stack before returning
 **/
callType func_instance::getCallingConvention() {
    const char *name = symTabName().c_str();
    const int buffer_size = 1024;
    char buffer[buffer_size];
    const char *pos;

    if (callingConv != unknown_call)
        return callingConv;

    if (!name) {
        //Umm...
        return unknown_call;
    }

    switch(name[0]) {
        case '?':
            //C++ Encoded symbol. Everything is stored in the C++ name 
            // mangling scheme
            UnDecorateSymbolName(name, buffer, buffer_size, 
                UNDNAME_NO_ARGUMENTS | UNDNAME_NO_FUNCTION_RETURNS);
            if (strstr(buffer, "__thiscall")) {
                callingConv = thiscall_call;
                return callingConv;
            }
            if (strstr(buffer, "__fastcall")) {
                callingConv = fastcall_call;
                return callingConv;
            }
            if (strstr(buffer, "__stdcall")) {
                callingConv = stdcall_call;
                return callingConv;
            }
            if (strstr(buffer, "__cdecl")) {
                callingConv = cdecl_call;
                return callingConv;
            }
            break;
        case '_':
          //Check for stdcall or cdecl
          pos = strrchr(name, '@');
          if (pos) {
            callingConv = stdcall_call;
            return callingConv;
          }
          else {
            callingConv = cdecl_call;
            return callingConv;
          }
          break;
        case '@':
          //Should be a fast call
          pos = strrchr(name, '@');
          if (pos) {
             callingConv = fastcall_call;
             return callingConv;
          }
          break;
    }

    //We have no idea what this call is.  We probably got an undecorated
    // name.  If the function doesn't clean up it's own stack (doesn't 
    // have a ret #) instruction, then it must be a cdecl call, as that's
    // the only type that doesn't clean its own stack.
    //If the function is part of a class, then it's most likely a thiscall,
    // although that could be incorrect for a static function.  
    //Otherwise let's guess that it's a stdcall.
    if (!ifunc()->cleansOwnStack()) {
        callingConv = cdecl_call;
    }
    else if (strstr(name, "::")) {
        callingConv = thiscall_call;
    }
    else {
        callingConv = stdcall_call;
    }
    return callingConv;
}

static void emitNeededCallSaves(codeGen &gen, Register reg, pdvector<Register> &extra_saves);
static void emitNeededCallRestores(codeGen &gen, pdvector<Register> &saves);

int EmitterIA32::emitCallParams(codeGen &gen, 
                              const pdvector<AstNodePtr> &operands,
                              func_instance *target, 
                              pdvector<Register> &extra_saves, 
                              bool noCost)
{
    callType call_conven = target->getCallingConvention();
    int estimatedFrameSize = 0;
    pdvector <Register> srcs;
    Register ecx_target = REG_NULL, edx_target = REG_NULL;
    Address unused = ADDR_NULL;
    const int num_operands = operands.size();

    switch (call_conven) {
        case unknown_call:
        case cdecl_call:
        case stdcall_call:
          //Push all registers onto stack
          for (unsigned u = 0; u < operands.size(); u++) {
              Register src = REG_NULL;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != REG_NULL);
              srcs.push_back(src);
          }
          break;
    case thiscall_call:
        //Allocate the ecx register for the 'this' parameter
        if (num_operands) {
            //result = gen.rs()->allocateSpecificRegister(gen, REGNUM_ECX, false);
            //if (!result) {
            //    emitNeededCallSaves(gen, REGNUM_ECX, extra_saves);
            //}
            if (!operands[0]->generateCode_phase2(gen, 
                                                  noCost, 
                                                  unused, ecx_target)) assert(0);
        }
        srcs.push_back(Null_Register);
        //Push other registers onto the stack
        for (unsigned u = 1; u < operands.size(); u++) {
              Register src = REG_NULL;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != REG_NULL);
              srcs.push_back(src);
        }     
        break;
    case fastcall_call:
        if (num_operands) {
            //Allocate the ecx register for the first parameter
            //ecx_target = gen.rs()->allocateSpecificRegister(gen, REGNUM_ECX, false);
            //if (!ecx_target) {
            //    emitNeededCallSaves(gen, REGNUM_ECX, extra_saves);
            //}
        }
        if (num_operands > 1) {
            //Allocate the edx register for the second parameter
            //edx_target = gen.rs()->allocateSpecificRegister(gen, REGNUM_EDX, false);
            //if (!edx_target) {
            //    emitNeededCallSaves(gen, REGNUM_EDX, extra_saves);
            //}
        }
        if (num_operands) {
            if (!operands[0]->generateCode_phase2(gen, 
                                                  noCost, 
                                                  unused, ecx_target)) assert(0);
        }
        if (num_operands > 1) {
            if (!operands[1]->generateCode_phase2(gen, 
                                                  noCost, unused, edx_target)) assert(0);
        }
        srcs.push_back(Null_Register);
        srcs.push_back(Null_Register);

        //Push other registers onto the stack
        for (unsigned u = 2; u < operands.size(); u++) {
              Register src = REG_NULL;
              Address unused = ADDR_NULL;
              if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
              assert(src != REG_NULL);
              srcs.push_back(src);
        }
        break;
    default:
        fprintf(stderr, "Internal error.  Unknown calling convention\n");
        assert(0);
    }

    // push arguments in reverse order, last argument first
    // must use int instead of unsigned to avoid nasty underflow problem:
    for (int i=srcs.size() - 1; i >= 0; i--) {
       if (srcs[i] == Null_Register) continue;
	   RealRegister r = gen.rs()->loadVirtual(srcs[i], gen);
	   ::emitPush(r, gen);
       estimatedFrameSize += 4;
       if (operands[i]->decRefCount())
          gen.rs()->freeRegister(srcs[i]);
    }

    if (ecx_target != REG_NULL) {
        //Store the parameter in ecx
		gen.rs()->loadVirtualToSpecific(ecx_target, RealRegister(REGNUM_ECX), gen);
    }

    if (edx_target != REG_NULL) {
		gen.rs()->loadVirtualToSpecific(edx_target, RealRegister(REGNUM_EDX), gen);
    }
    return estimatedFrameSize;
}

bool EmitterIA32::emitCallCleanup(codeGen &gen, func_instance *target, 
                     int frame_size, pdvector<Register> &extra_saves)
{
    callType call_conv = target->getCallingConvention();
    if ((call_conv == unknown_call || call_conv == cdecl_call) && frame_size)
    {
        //Caller clean-up
        emitOpRegImm(0, RealRegister(REGNUM_ESP), frame_size, gen); // add esp, frame_size        
    }
    gen.rs()->incStack(-1 * frame_size);

    //Restore extra registers we may have saved when storing parameters in
    // specific registers
    //emitNeededCallRestores(gen, extra_saves);
    return 0;
}

static void emitNeededCallSaves(codeGen &gen, Register regi, 
                           pdvector<Register> &extra_saves)
{
    extra_saves.push_back(regi);
    switch (regi) {
        case REGNUM_EAX:
            emitSimpleInsn(PUSHEAX, gen);
            break;
        case REGNUM_EBX:
            emitSimpleInsn(PUSHEBX, gen);
            break;
        case REGNUM_ECX:
            emitSimpleInsn(PUSHECX, gen);
            break;
        case REGNUM_EDX:
            emitSimpleInsn(PUSHEDX, gen);
            break;
        case REGNUM_EDI:
            emitSimpleInsn(PUSHEDI, gen);
            break;
    }
}

static void emitNeededCallRestores(codeGen &gen, pdvector<Register> &saves)
{
    for (unsigned i=0; i<saves.size(); i++) {
      switch (saves[i]) {
          case REGNUM_EAX:
              emitSimpleInsn(POP_EAX, gen);
              break;
          case REGNUM_EBX:
              emitSimpleInsn(POP_EBX, gen);
              break;
          case REGNUM_ECX:
              emitSimpleInsn(POP_ECX, gen);
              break;
          case REGNUM_EDX:
              emitSimpleInsn(POP_EDX, gen);
              break;
          case REGNUM_EDI:
              emitSimpleInsn(POP_EDI, gen);
              break;
      }
    }
    saves.clear();
}

bool SignalHandler::handleProcessExitPlat(EventRecord &ev, bool &continueHint) 
{
    ReleaseSymbolHandler(ev.proc->processHandle_);
    continueHint = false;
    ev.proc->continueHandles.push_back(ev.info.dwThreadId);
    ev.proc->continueTypes.push_back(DBG_CONTINUE);
    return true;
}

bool process::continueProc_(int sig) {
    unsigned index;
    dyn_lwp *lwp;
    if (representativeLWP) {
        representativeLWP->continueLWP(true);
    }
    dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
    while (lwp_iter.next(index, lwp)) {
        lwp->continueLWP(true);
    }
    return true;
}

bool process::stop_(bool waitUntilStop) {
   unsigned index;
   dyn_lwp *lwp;
   if (representativeLWP) {
       representativeLWP->pauseLWP(true);
   }
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   while (lwp_iter.next(index, lwp)) {
       lwp->pauseLWP(true);
   }
   return true;
}

void process::deleteThread_(dyn_thread *thr) {
    int hand = thr->get_tid();
    int contType = DBG_CONTINUE;

    continueHandles.push_back(hand);
    continueTypes.push_back(contType);
}

bool SignalGeneratorCommon::postSignalHandler() {
    for (unsigned i=0; i<proc->continueHandles.size(); i++) {
        ContinueDebugEvent(proc->getPid(), proc->continueHandles[i], proc->continueTypes[i]);
    }
    proc->continueHandles.clear();
    proc->continueTypes.clear();
    return true;
}

bool SignalHandler::forwardSigToProcess(EventRecord &ev, bool &continueHint) 
{
   process *proc = ev.proc;
   int hand = (int) ev.info.dwThreadId;

   proc->continueHandles.push_back(hand);
   proc->continueTypes.push_back(DBG_EXCEPTION_NOT_HANDLED);
   
   if (getExecThreadID() != sg->getThreadID()) {
      signal_printf("%s[%d][%s]:  signalling active process\n", 
                    FILE__, __LINE__, getThreadStr(getExecThreadID()));
      sg->requested_wait_until_active = false;
      sg->signalActiveProcess();
   }
   return true;
}

/* 1. Gather the list of Structured Exception Handlers by walking the linked
 * list whose head is in the TIB.  
 * 2. If the fault occurred at an emulated memory instruction, we saved a
 *    register before stomping its effective address computation
 * 3. Create an instPoint at the faulting instruction, If the exception-raising
 *    instruction is in a relocated block or multiTramp, save it as an active 
 *    tramp, we can't get rid of it until the handler returns
 * 4. Invoke the registered callback
 * 5. mark parsed handlers as such, store fault addr info in the handlers
 */
bool SignalHandler::handleSignalHandlerCallback(EventRecord &ev)
{
    process *proc = ev.proc;
    pdvector<CallbackBase *> cbs;
    if (!getCBManager()->dispenseCallbacksMatching(evtSignalHandlerCB, cbs)) {
        return false;
    }
    mal_printf("Handling exception, excCode=0x%X raised by %lx %s[%d]\n",
            ev.what, ev.address, FILE__, __LINE__);

    Address origAddr = ev.address;
    vector<func_instance*> faultFuncs;
    baseTrampInstance *bti = NULL;
    ev.proc->getAddrInfo(ev.address, origAddr, faultFuncs, bti);
    cerr << "Address " << hex << ev.address << " maps to address " << origAddr << dec << endl;


/* begin debugging output */
	cerr << "Frame info dump" << endl;
	Frame activeFrame = ev.lwp->getActiveFrame();
	cerr << "EXCEPTION FRAME: " << hex << activeFrame.getPC() << " / " <<activeFrame.getSP() 
                 << " (DEBUG:" 
                 << "EAX: " << activeFrame.eax
                 << ", ECX: " << activeFrame.ecx
                 << ", EDX: " << activeFrame.edx
                 << ", EBX: " << activeFrame.ebx
                 << ", ESP: " << activeFrame.esp
                 << ", EBP: " << activeFrame.ebp
                 << ", ESI: " << activeFrame.esi 
                 << ", EDI " << activeFrame.edi << ")" << dec << endl;

    // decode the executed instructions
    using namespace InstructionAPI;
    cerr << "Disassembling faulting insns" << endl;
    Address base = ev.address - 64;
    const int BUF_SIZE=256;
    unsigned char buf[BUF_SIZE];
    ev.proc->readDataSpace((void *)base, BUF_SIZE, buf, false);
    InstructionDecoder deco(buf,BUF_SIZE,ev.proc->getArch());
    Instruction::Ptr insn = deco.decode();
    while(insn) {
        cerr << "\t" << hex << base << ": " << insn->format(base) << endl;
        base += insn->size();
        insn = deco.decode();
    }
    cerr << "raw bytes: ";
    for(int idx=0; idx < BUF_SIZE; idx++) {
        cerr << (unsigned int)buf[idx] << " ";
    }
    cerr << endl << dec << "Stack" << endl;
	for (int i = -10; i < 10; ++i) {
		Address stackTOPVAL =0;
	    ev.proc->readDataSpace((void *) (activeFrame.esp + 4*i), sizeof(ev.proc->getAddressWidth()), &stackTOPVAL, false);
		cerr << "\t" << hex <<activeFrame.esp + 4*i << ": " << stackTOPVAL << dec << endl;
	 }
/* end debugging output */

    // 1. gather the list of handlers by walking the SEH datastructure in the TEB
    Address tibPtr = ev.lwp->getThreadInfoBlockAddr();
    struct EXCEPTION_REGISTRATION handler;
    EXCEPTION_REGISTRATION *prevEvtReg=NULL;
    if (!proc->readDataSpace((void*)tibPtr,sizeof(Address),
                             (void*)&prevEvtReg,false)) {
        fprintf(stderr, "%s[%d]Error reading from TIB at 0x%x\n", 
                FILE__, __LINE__,tibPtr);
        return false;
    }
    vector<Address> handlers;
    while(((long)prevEvtReg) != -1 && prevEvtReg != NULL) {
        if (!proc->readDataSpace((void*)prevEvtReg,sizeof(handler),
                                 &handler,false)) {
            fprintf(stderr, "%s[%d]Error reading from SEH chain at 0x%lx\n", 
                    FILE__, __LINE__,(long)prevEvtReg);
            return false;
        }
        prevEvtReg = handler.prev;
        if (!proc->findOneFuncByAddr((Address)prevEvtReg)) {
            mal_printf("EUREKA! Found handler at 0x%x while handling "
                   "exceptionCode=0x%X for exception at %lx %s[%d]\n",
                   handler.handler, ev.what, ev.address, FILE__,__LINE__);
            handlers.push_back(handler.handler);
        }
    }
    if (0 == handlers.size()) {
        return true;
    }

    // 2.  If the fault occurred at an emulated memory instruction, we saved a
    //     register before stomping its effective address computation, 
    //     restore the original register value

    block_instance *faultBBI = NULL;
    switch( faultFuncs.size() ) {
    case 0: 
        fprintf(stderr,"ERROR: Failed to find a valid instruction for fault "
            "at %lx %s[%d] \n", ev.address, FILE__,__LINE__);
         return false;
    case 1:
        faultBBI = faultFuncs[0]->findOneBlockByAddr(origAddr);
        break;
    default: 
        faultBBI = ev.proc->findActiveFuncByAddr(ev.address)->
                findOneBlockByAddr(origAddr);
        break;
    }
    if (ev.proc->isMemoryEmulated() && 
        BPatch_defensiveMode == faultFuncs[0]->obj()->hybridMode())
    {
        if (faultFuncs[0]->obj()->isEmulInsn(origAddr)) {
            void * val =0;
            assert( sizeof(void*) == ev.proc->getAddressWidth() );
            ev.proc->readDataSpace((void*)(activeFrame.getSP() + MemoryEmulator::STACK_SHIFT_VAL), 
                                   ev.proc->getAddressWidth(), 
                                   &val, false);

            CONTEXT context;
            context.ContextFlags = CONTEXT_FULL;
            if (!GetThreadContext(ev.lwp->get_fd(), (LPCONTEXT) & context)) {
                malware_cerr << "ERROR: Failed call to GetThreadContext(" << hex << ev.lwp->get_fd() 
                    << ") getLastError: " << endl;
                printSysError(GetLastError());
            }
            Register reg = faultFuncs[0]->obj()->getEmulInsnReg(origAddr);
            switch(reg) {
                case REGNUM_ECX:
                    context.Ecx = (DWORD) val;
                    break;
                case REGNUM_EDX:
                    context.Edx = (DWORD) val;
                    break;
                case REGNUM_EAX:
                    context.Eax = (DWORD) val;
                    break;
                case REGNUM_EBX:
                    context.Ebx = (DWORD) val;
                    break;
                case REGNUM_ESI:
                    context.Esi = (DWORD) val;
                    break;
                case REGNUM_EDI:
                    context.Edi = (DWORD) val;
                    break;
                case REGNUM_EBP:
                    context.Ebp = (DWORD) val;
                    break;
                default:
                    assert(0);
            }
            SetThreadContext(ev.lwp->get_fd(), (LPCONTEXT) & context);
        }
    }

    // 3. create instPoint at faulting instruction & trigger callback

    instPoint *point = faultBBI->func()->findInstPByAddr(origAddr);
    if (!point) {
        point = instPoint::createArbitraryInstPoint
                    (origAddr, proc, faultBBI->func());                
    }
    if (!point) {
        fprintf(stderr,"Failed to create an instPoint for faulting "
            "instruction at %lx[%lx] in function at %lx %s[%d]\n",
            ev.address,origAddr,faultBBI->func()->getAddress(),FILE__,__LINE__);
        return false;
    }

    //4. cause callbacks registered for this event to be triggered, if any.
    ((BPatch_process*)proc->up_ptr())->triggerSignalHandlerCB
            (point, faultBBI->func(), ev.what, &handlers);

    //5. mark parsed handlers as such, store fault addr info in the handlers
    for (vector<Address>::iterator hIter=handlers.begin(); 
         hIter != handlers.end(); 
         hIter++) 
    {
        func_instance *hfunc = ev.proc->findOneFuncByAddr(*hIter);
        if (hfunc) {
            using namespace ParseAPI;
            hfunc->setHandlerFaultAddr(point->addr());
            Address base = hfunc->getAddress() - hfunc->ifunc()->addr();
            const vector<FuncExtent*> &exts = hfunc->ifunc()->extents();
            for (unsigned eix=0; eix < exts.size(); eix++) {
                ev.proc->addSignalHandler(base + exts[eix]->start(),
                                          exts[eix]->end()-exts[eix]->start());
            }
        } else {
            fprintf(stderr, "WARNING: failed to parse handler at %lx for "
                    "exception at %lx %s[%d]\n", *hIter, point->addr(), 
                    FILE__,__LINE__);
        }
    }

    return true;
}


/* An access violation occurred to a memory page that contains code
 * and was originally write-protected was protected 
 * 1. Get violation address
 * 2. Flush the runtime cache if we overwrote any code, else return
 * 3. Find the instruction that caused the violation and determine 
 *    its address in unrelocated code
 * 4. Create an instPoint for the write
 * 5. Trigger user-mode callback to respond to the overwrite
 */
bool SignalHandler::handleCodeOverwrite(EventRecord &ev)
{
    //1. Get violation address
    Address writtenAddr = 
        ev.info.u.Exception.ExceptionRecord.ExceptionInformation[1];
    SymtabAPI::Region *reg = (SymtabAPI::Region*) ev.info2;
	mal_printf("handleCodeOverwrite: 0x%lx\n", writtenAddr);

    if (ev.proc->isMemoryEmulated()) {
        Address shadowAddr = writtenAddr;
        int shadowRights=0;
        bool valid = false;
        boost::tie(valid, shadowAddr) = ev.proc->getMemEm()->translateBackwards(writtenAddr);
		if (!valid) {
			cerr << "WARNING: writing to original memory directly, should only happen in uninstrumented code!" << endl;
		}
		else {
			assert(valid && shadowAddr != writtenAddr);
			writtenAddr = shadowAddr;
		}
	}

    // 2. Flush the runtime cache if we overwrote any code
    // Produce warning message if we've overwritten weird types of code: 
    Address origWritten = writtenAddr;
    vector<func_instance *> writtenFuncs;
    baseTrampInstance *bti = NULL;
    bool success = ev.proc->getAddrInfo(writtenAddr, 
                                        origWritten, 
                                        writtenFuncs, 
                                        bti);
    if (writtenFuncs.size() == 0) {
        mapped_object *writtenObj = ev.proc->findObject(writtenAddr);
        assert(writtenObj);
        mal_printf("%s[%d] Insn at %lx wrote to %lx on a page containing "
                "code, but no code was overwritten\n",
                FILE__,__LINE__,ev.address,writtenAddr);
    }
    else {
        // flush all addresses matching the mapped object and the
        // runtime library heaps
        ev.proc->flushAddressCache_RT(writtenFuncs[0]->obj());

        if (writtenFuncs.size() > 1) {
            fprintf(stderr, "WARNING: overwrote shared code, we may not "
                    "handle this correctly %lx->%lx[%lx] %s[%d]\n",
                    ev.address, writtenAddr, origWritten, FILE__,__LINE__);
            //assert(0 && "overwrote shared code"); //KEVINTODO: test this case
        }
    }

    // 3. Find the instruction that caused the violation and determine 
    //    its address in unrelocated code
    Address origWrite = ev.address;
    vector<func_instance *> writeFuncs;
    success = ev.proc->getAddrInfo(ev.address, origWrite, writeFuncs, bti);
    if (!success) {
        // this is an error case, meaning that we're executing 
        // uninstrumented code. It has been a sign that:
        //  - we invalidated relocated code that we were executing in
        //  - we removed code-discovery instrumentation, because of an 
        //    overwrite in a block that ends with an indirect ctrl 
        //    transfer that should be instrumented, and are executing
        // sometimes arises as a race condition
        fprintf(stderr, "ERROR: found no code to match instruction at %lx,"
                " which writes to %lx on page containing analyzed code\n",
                ev.address, writtenAddr);
        assert(0 && "couldn't find the overwrite instruction"); 
    }

    // 4. Create an instPoint for the write
    func_instance *writeFunc;
    if (writeFuncs.size() == 1) {
        writeFunc = writeFuncs[0];
    } else { 
        writeFunc = ev.proc->findActiveFuncByAddr(ev.address);
    }
    instPoint *writePoint = writeFunc->findInstPByAddr(origWrite);
    if (!writePoint) {
        // it can't be a call or exit point, if it exists it's an 
        // entryPoint, or abruptEnd point (or an arbitrary point, but
        // those aren't created lazily
        if (origWrite == writeFunc->getAddress()) {
            writeFunc->funcEntries();
            writePoint = writeFunc->findInstPByAddr(origWrite);
        } else {
            writeFunc->funcAbruptEnds();
            writePoint = writeFunc->findInstPByAddr(origWrite);
        }
    }
    if (!writePoint) {
        writePoint = instPoint::createArbitraryInstPoint(
            origWrite, ev.proc, writeFunc);
    }
    assert(writePoint);

    // 5. Trigger user-mode callback to respond to the overwrite
    success = (((BPatch_process*)ev.proc->up_ptr())->
        triggerCodeOverwriteCB(writePoint, writtenAddr));
    assert(success);

    return true;
}

bool process::hideDebugger() 
{
    dyn_lwp *lwp = getInitialLwp();
    if (!lwp) {
        return false;
    }
    Address tibPtr = lwp->getThreadInfoBlockAddr();
    if (!tibPtr) {
        return false;
    }

    // read in address of PEB
    unsigned int pebPtr;
    if (!readDataSpace((void*)(tibPtr+48), getAddressWidth(),(void*)&pebPtr, false)) {
        fprintf(stderr, "%s[%d] Failed to read address of Process Environment "
            "Block at 0x%x, which is TIB + 0x30\n", FILE__,__LINE__,tibPtr+48);
        return false;
    }

    // patch up the processBeingDebugged flag in the PEB
    unsigned char flag;
    if (!readDataSpace((void*)(pebPtr+2), 1, (void*)&flag, true)) 
        return false;
    if (flag) {
        flag = 0;
        if (!writeDataSpace((void*)(pebPtr+2), 1, (void*)&flag)) 
            return false;
    }

    //while we're at it, clear the NtGlobalFlag
    if (!readDataSpace((void*)(pebPtr+0x68), 1, (void*)&flag, true)) 
        return false;
    if (flag) {
        flag = flag & 0x8f;
        if (!writeDataSpace((void*)(pebPtr+0x68), 1, (void*)&flag)) 
            return false;
    }

    // clear the heap flags in the PEB
    unsigned int heapBase;
    unsigned int flagWord;
    if (!readDataSpace((void*)(pebPtr+0x18), 4, (void*)&heapBase, true)) 
        return false;

    // clear the flags in the heap itself
    if (!readDataSpace((void*)(heapBase+0x0c), 4, (void*)&flagWord, true)) 
        return false;
    flagWord = flagWord & (~0x50000062);
    if (!writeDataSpace((void*)(heapBase+0x0c), 4, (void*)&flagWord)) 
        return false;
    if (!readDataSpace((void*)(heapBase+0x10), 4, (void*)&flagWord, true)) 
        return false;
    flagWord = flagWord & (~0x40000060);
    if (!writeDataSpace((void*)(heapBase+0x10), 4, (void*)&flagWord)) 
        return false;

    return true;
}


mapped_object *process::createObjectNoFile(Address addr)
{
	cerr << "createObjectNoFile " << hex << addr << dec << endl;
    Address closestObjEnd = 0;
    for (unsigned i=0; i<mapped_objects.size(); i++)
    {
        if (addr >= mapped_objects[i]->codeAbs() &&
            addr <   mapped_objects[i]->codeAbs() 
                   + mapped_objects[i]->imageSize())
        {
            fprintf(stderr,"createObjectNoFile called for addr %lx, "
                    "matching existing mapped_object %s %s[%d]\n", addr,
                    mapped_objects[i]->fullName().c_str(), FILE__,__LINE__);
            return mapped_objects[i];
        }
        if (  addr >= ( mapped_objects[i]->codeAbs() + 
                        mapped_objects[i]->imageSize() ) &&  
            closestObjEnd < ( mapped_objects[i]->codeAbs() + 
                               mapped_objects[i]->imageSize() ) ) 
        {
            closestObjEnd = mapped_objects[i]->codeAbs() + 
                            mapped_objects[i]->imageSize();
        }
    }

    Address testRead = 0;

    // VirtualQueryEx rounds down to pages size, so we need to round up first.
    if (proc()->proc() && closestObjEnd % proc()->proc()->getMemoryPageSize())
    {
        closestObjEnd = closestObjEnd 
            - (closestObjEnd % proc()->proc()->getMemoryPageSize()) 
            + proc()->proc()->getMemoryPageSize();
    }
    if (proc()->proc() && readDataSpace((void*)addr, proc()->getAddressWidth(),
                                        &testRead, false)) 
    {
		// create a module for the region enclosing this address
        MEMORY_BASIC_INFORMATION meminfo;
        memset(&meminfo,0, sizeof(MEMORY_BASIC_INFORMATION) );
        SIZE_T size = VirtualQueryEx(proc()->processHandle_,
                                     (LPCVOID)addr, &meminfo, 
                                     sizeof(MEMORY_BASIC_INFORMATION));
        assert(meminfo.State == MEM_COMMIT);
		cerr << "VirtualQuery reports baseAddr " << hex << meminfo.BaseAddress << ", allocBase " << meminfo.AllocationBase << ", size " << meminfo.RegionSize << ", state " << meminfo.State << dec << endl;

        Address objStart = (Address) meminfo.AllocationBase;
        Address probeAddr = (Address) meminfo.BaseAddress +  (Address) meminfo.RegionSize;
        Address objEnd = probeAddr;
        MEMORY_BASIC_INFORMATION probe;
        memset(&probe, 0, sizeof(MEMORY_BASIC_INFORMATION));
        do {
            objEnd = probeAddr;
            SIZE_T size2 = VirtualQueryEx(proc()->processHandle_,
                                          (LPCVOID) ((Address)meminfo.BaseAddress + meminfo.RegionSize),
                                          &probe,
                                          sizeof(MEMORY_BASIC_INFORMATION));
			cerr << "VirtualQuery reports baseAddr " << hex << probe.BaseAddress << ", allocBase " << probe.AllocationBase << ", size " << probe.RegionSize << ", state " << probe.State << dec << endl;

			probeAddr = (Address) probe.BaseAddress + (Address) probe.RegionSize;
        } while ((probe.AllocationBase == meminfo.AllocationBase) && // we're in the same allocation unit...
			(objEnd != probeAddr)); // we're making forward progress


        // The size of the region returned by VirtualQueryEx is from BaseAddress
        // to the end, NOT from meminfo.AllocationBase, which is what we want.
        // BaseAddress is the start address of the page of the address parameter
        // that is sent to VirtualQueryEx as a parameter
        Address regionSize = objEnd - objStart;
        mal_printf("[%lx %lx] is valid region containing %lx and corresponding "
               "to no object, closest is object ending at %lx %s[%d]\n", 
               objStart, 
               objEnd,
               addr, closestObjEnd, FILE__,__LINE__);
        // read region into this process
        unsigned char* rawRegion = (unsigned char*) 
            ::LocalAlloc(LMEM_FIXED, regionSize);
		if (!proc()->readDataSpace((void *)objStart,
								   regionSize,
								   rawRegion, true))
		{
			cerr << "Error: failed to read memory region [" << hex << objStart << "," << objStart + regionSize << "]" << dec << endl;
			printSysError(GetLastError());
			assert(0);
		}
		// set up file descriptor
        char regname[64];
        snprintf(regname,63,"mmap_buffer_%lx_%lx",
                    objStart, objEnd);
        fileDescriptor desc(string(regname), 
                            0, 
                            (HANDLE)0, 
                            (HANDLE)0, 
                            true, 
                            (Address)objStart,
                            (Address)regionSize,
                            rawRegion);
        mapped_object *obj = mapped_object::createMappedObject
            (desc,this,proc()->getHybridMode(),false);
        if (obj != NULL) {
            obj->setMemoryImg();
            mapped_objects.push_back(obj);

            obj->parse_img()->getOrCreateModule(
                obj->parse_img()->getObject()->getDefaultModule());
            return obj;
        }
    }
    return NULL;
}


SignalGenerator::SignalGenerator(char *idstr, std::string file, int pid)
    : SignalGeneratorCommon(idstr)
{
    setupAttached(file, pid);
} 

void EventRecord::clear() {
    proc = NULL;
    lwp = NULL;
    type = evtUndefined;
    what = 0;
    status = statusUnknown;
    info.dwDebugEventCode = 0;
    info.dwProcessId = 0;
    info.dwThreadId = 0;
    address = 0;
    fd = 0;
}

// Unix functions that aren't needed on Windows
void DBICallbackBase::dbi_signalCompletion(CallbackBase *cbb) {}
bool DBICallbackBase::execute() { return false; }
bool DBICallbackBase::waitForCompletion() { return false; }
bool PtraceCallback::execute_real() {return false;}
bool ReadDataSpaceCallback::execute_real() {return false;}
bool WaitPidNoBlockCallback::execute_real() {return false;}
bool WriteDataSpaceCallback::execute_real() {return false;}

bool OS::executableExists(const std::string &file) {
   struct stat file_stat;
   int stat_result;

   stat_result = stat(file.c_str(), &file_stat);
   if (stat_result == -1)
       stat_result = stat((file + std::string(".exe")).c_str(), &file_stat);
   return (stat_result != -1);
}

func_instance *dyn_thread::map_initial_func(func_instance *ifunc) {
    if (!ifunc || strcmp(ifunc->prettyName().c_str(), "mainCRTStartup"))
        return ifunc;

    //mainCRTStartup is not a real initial function.  Use main, if it exists.
    const pdvector<func_instance *> *mains = proc->getAOut()->findFuncVectorByPretty("main");
    if (!mains || !mains->size())
        return ifunc;
    return (*mains)[0];
}

bool process::instrumentThreadInitialFunc(func_instance *f) {
    if (!f)
        return false;

    for (unsigned i=0; i<initial_thread_functions.size(); i++) {
		if (initial_thread_functions[i] == f) {
            return true;
    }
    }
    func_instance *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
    if (!dummy_create)
    {
      return false;
    } 

    pdvector<AstNodePtr> args;
    AstNodePtr call_dummy_create = AstNode::funcCallNode(dummy_create, args);
    const pdvector<instPoint *> &ips = f->funcEntries();
    for (unsigned j=0; j<ips.size(); j++)
    {
       miniTramp *mt;
       mt = ips[j]->instrument(call_dummy_create, callPreInsn, orderFirstAtPoint, false, 
                               false);
       if (!mt)
       {
          fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                  __FILE__, __LINE__);
       }
    }
    initial_thread_functions.push_back(f);
    return true;
}

bool SignalHandler::handleProcessAttach(EventRecord &ev, bool &continueHint) {
    process *proc = ev.proc;
    proc->setBootstrapState(initialized_bs);
    
    dyn_lwp *rep_lwp = proc->getRepresentativeLWP();
    assert(rep_lwp);

    //We're starting up, convert the representative lwp to a real one.
    rep_lwp->set_lwp_id((int) rep_lwp->get_fd());
    proc->real_lwps[rep_lwp->get_lwp_id()] = rep_lwp;
    proc->representativeLWP = NULL;
    if (proc->theRpcMgr)
       proc->theRpcMgr->addLWP(rep_lwp);
    continueHint = true;

	ev.lwp->setDebuggerLWP(true);
    return true;
}

bool process::hasPassedMain() 
{
   return true;
}

Address dyn_lwp::getThreadInfoBlockAddr()
{
    if (threadInfoBlockAddr_) {
        return threadInfoBlockAddr_;
    }
    // use getRegisters to get value of the FS segment register
    dyn_saved_regs regs;
    if (!getRegisters(&regs)) {
        return 0;
    }
    // use the FS segment selector to look up the segment descriptor in the local descriptor table
    LDT_ENTRY segDesc;
	if (!GetThreadSelectorEntry(fd_, (DWORD)regs.cont.SegFs, &segDesc)) {
		fprintf(stderr, "%s[%d] Failed to read segment register FS for thread 0x%x with FS index of 0x%x\n", 
			FILE__,__LINE__,fd_,regs.cont.SegFs);
		return 0;
	}
    // calculate the address of the TIB
    threadInfoBlockAddr_ = (Address) segDesc.BaseLow;
    Address tmp = (Address) segDesc.HighWord.Bytes.BaseMid;
    threadInfoBlockAddr_ = threadInfoBlockAddr_ | (tmp << (sizeof(WORD)*8));
    tmp = segDesc.HighWord.Bytes.BaseHi;
    threadInfoBlockAddr_ = threadInfoBlockAddr_ | (tmp << (sizeof(WORD)*8+8));
    return threadInfoBlockAddr_;
}

bool process::startDebugger()
{
   return false;
}

// Temporary remote debugger interface.
// I assume these will be removed when procControlAPI is complete.
bool OS_isConnected(void)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_connect(BPatch_remoteHost &remote)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_getPidList(BPatch_remoteHost &remote,
                   BPatch_Vector<unsigned int> &tlist)
{
    return false;  // Not implemented.
}

bool OS_getPidInfo(BPatch_remoteHost &remote,
                   unsigned int pid, std::string &pidStr)
{
    return false;  // Not implemented.
}

bool OS_disconnect(BPatch_remoteHost &remote)
{
    return true;
}
