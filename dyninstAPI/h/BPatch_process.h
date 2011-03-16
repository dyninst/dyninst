/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#ifndef _BPatch_process_h_
#define _BPatch_process_h_

#include "BPatch_snippet.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_eventLock.h"
#include "BPatch_point.h"
#include "BPatch_addressSpace.h"
#include "BPatch_hybridAnalysis.h"

#include "BPatch_callbacks.h"

#include <vector>

#include <stdio.h>
#include <signal.h>

class process;
class AddressSpace;
class dyn_thread;
class miniTrampHandle;
class miniTramp;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_funcMap;
class BPatch_instpMap;
class int_function;
class rpcMgr;
class HybridAnalysis;
struct batchInsertionRecord;

typedef enum {
  BPatch_nullEvent,
  BPatch_newConnectionEvent,
  BPatch_internalShutDownEvent,
  BPatch_threadCreateEvent,
  BPatch_threadDestroyEvent,
  BPatch_dynamicCallEvent,
  BPatch_userEvent,
  BPatch_errorEvent,
  BPatch_dynLibraryEvent,
  BPatch_preForkEvent,
  BPatch_postForkEvent,
  BPatch_execEvent,
  BPatch_exitEvent,
  BPatch_signalEvent,
  BPatch_oneTimeCodeEvent
} BPatch_asyncEventType;

typedef long dynthread_t;


typedef struct {
  BPatch_snippet snip;
  BPatchSnippetHandle *sh;
  BPatch_thread *thread;
} BPatch_catchupInfo;

class EventRecord;
class OneTimeCodeCallback;
/*
 * class OneTimeCodeInfo
 *
 * This is used by the oneTimeCode (inferiorRPC) mechanism to keep per-RPC
 * information.
 */
class OneTimeCodeInfo {
   bool synchronous;
   bool completed;
   void *userData;
   OneTimeCodeCallback *cb;
   void *returnValue;
   unsigned thrID;
public:
   OneTimeCodeInfo(bool _synchronous, void *_userData, OneTimeCodeCallback *_cb, unsigned _thrID) :
      synchronous(_synchronous), completed(false), userData(_userData), cb(_cb),
      thrID(_thrID) { };

   bool isSynchronous() { return synchronous; }

   bool isCompleted() const { return completed; }
   void setCompleted(bool _completed) { completed = _completed; }

   void *getUserData() { return userData; }

   void setReturnValue(void *_returnValue) { returnValue = _returnValue; }
   void *getReturnValue() { return returnValue; }

   unsigned getThreadID() { return thrID; }

   OneTimeCodeCallback *getCallback() { return cb;}
   
};


/*
 * Represents a process
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_process
class BPATCH_DLL_EXPORT BPatch_process : public BPatch_addressSpace {
    friend class BPatch;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_frame;
    friend class BPatch_thread;
    friend class BPatch_asyncEventHandler;
    friend class BPatch_module;
    friend class BPatch_basicBlock;
    friend class BPatch_flowGraph;
    friend class BPatch_loopTreeNode;
    friend class BPatch_point;
    friend class BPatch_funcCallExpr;
    friend class BPatch_eventMailbox;
    friend class BPatch_instruction;
    friend class BPatch_addressSpace;
    friend class process;
    friend class SignalHandler;
    friend int handleSignal(EventRecord &ev);
    friend void threadDeleteWrapper(BPatch_process *, BPatch_thread *); 
    friend bool pollForStatusChange();
    friend class AsyncThreadEventCallback;
    friend class AstNode; // AST needs to translate instPoint to
    friend class AstOperatorNode;
    friend class AstMemoryNode;
    friend class rpcMgr;
    friend class EventRecord;
    friend bool handleThreadCreate(process *, EventRecord &, unsigned, int, dynthread_t, unsigned long, unsigned long);

    protected:
    void getAS(std::vector<AddressSpace *> &as);

    public:
    void PDSEP_updateObservedCostAddr(unsigned long a);
    private:

    //References to lower level objects
    process *llproc;

    BPatch_Vector<BPatch_thread *> threads;

    // Due to interactions of internal events and signal handling,
    // we need to keep an internal variable of whether the user
    // should observe us as stopped. This is set by internal 
    // code if a "stop" is a real stop. 
    bool isVisiblyStopped;
    bool isAttemptingAStop;

    int lastSignal;
    int exitCode;
    int exitSignal;
    bool exitedNormally;
    bool exitedViaSignal;
    bool mutationsActive;
    bool createdViaAttach;
    bool detached;

    bool unreportedStop;
    bool unreportedTermination;

    // BPatch-level; once the callbacks are sent by the llproc, we're terminated
    // Used because callbacks go (and can clean up user code) before the low-level process
    // sets flags.
    bool terminated; 
    bool reportedExit;

    //When an async RPC is posted on a stopped process we post it, but haven't 
    // yet launched it.  The next process level continue should start the RPC 
    // going.  unstarteRPC is true if we have a posted but not launced RPC.
    bool unstartedRPC;

    void setUnreportedStop(bool new_value) { unreportedStop = new_value; }
    void setUnreportedTermination(bool new_value) {unreportedTermination = new_value;}

    void setExitedNormally();
    void setExitedViaSignal(int signalnumber);

    void setExitCode(int exitcode) { exitCode = exitcode; }
    void setExitSignal(int exitsignal) { exitSignal = exitsignal; }

    bool pendingUnreportedStop() { return unreportedStop;}
    bool pendingUnreportedTermination() { return unreportedTermination; }

    bool statusIsStopped();
    bool statusIsTerminated();

    bool getType();
    bool getTerminated() {return terminated;}
    bool getMutationsActive() {return mutationsActive;}

    int activeOneTimeCodes_;
    bool resumeAfterCompleted_;

    HybridAnalysis *hybridAnalysis_;

    static int oneTimeCodeCallbackDispatch(process *theProc,
                                           unsigned /* rpcid */, 
                                           void *userData,
                                           void *returnValue);

    void *oneTimeCodeInternal(const BPatch_snippet &expr,
                              BPatch_thread *thread, // == NULL if proc-wide
                              void *userData,
                              BPatchOneTimeCodeCallback cb = NULL,
                              bool synchronous = true, bool *err = NULL);

    void oneTimeCodeCompleted(bool isSynchronous);

    protected:
    // for creating a process
    BPatch_process(const char *path, const char *argv[], 
                   BPatch_hybridMode mode, const char **envp = NULL, 
                   int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2);
    // for attaching
    BPatch_process(const char *path, int pid, BPatch_hybridMode mode);	

    // for forking
    BPatch_process(process *proc);

    // Create a new thread in this proc
    BPatch_thread *createOrUpdateBPThread(int lwp, dynthread_t tid, unsigned index, 
                                          unsigned long stack_start, 
                                          unsigned long start_addr);
    BPatch_thread *handleThreadCreate(unsigned index, int lwpid, dynthread_t threadid, 
                            unsigned long stack_top, unsigned long start_pc, process *proc = NULL);
    void deleteBPThread(BPatch_thread *thrd);

    bool updateThreadInfo();

        
    public:

    // Begin internal functions, DO NOT USE
    //
    // this function should go away as soon as Paradyn links against Dyninst
    process *lowlevel_process() { return llproc; }
    // These internal funcs trigger callbacks registered to matching events
    bool triggerStopThread(instPoint *intPoint, int_function *intFunc, 
                            int cb_ID, void *retVal);
    bool triggerSignalHandlerCB(instPoint *point, int_function *func, long signum, 
                               BPatch_Vector<Dyninst::Address> *handlers); 
    bool triggerCodeOverwriteCB(Dyninst::Address fault_instruc, 
                                Dyninst::Address viol_target); 
    bool setMemoryAccessRights(Dyninst::Address start, Dyninst::Address size, 
                               int rights);
    unsigned char *makeShadowPage(Dyninst::Address pageAddress);
    void overwriteAnalysisUpdate
        ( std::map<Dyninst::Address,unsigned char*>& owPages, //input
          std::vector<Dyninst::Address>& deadBlockAddrs, //output
          std::vector<BPatch_function*>& owFuncs,     //output
          bool &changedPages, bool &changedCode ); //output
    // take a function and split off the blocks that correspond to the range
    bool removeFunctionSubRange
        (BPatch_function &curFunc, 
         Dyninst::Address startAddr, 
         Dyninst::Address endAddr, 
         std::vector<Dyninst::Address> &blockAddrs);
    HybridAnalysis *getHybridAnalysis() { return hybridAnalysis_; }
    bool protectAnalyzedCode();
    // DO NOT USE
    // This is an internal debugging function
    API_EXPORT_V(Int, (), 
    void, debugSuicide,());
    //
    // End internal functions


  
    //  BPatch_process::~BPatch_process
    //
    //  Destructor
    API_EXPORT_DTOR(_dtor, (),
    ~,BPatch_process,());

       
    //  BPatch_process::getPid
    //  
    //  Get  id of mutatee process

    API_EXPORT(Int, (),
    int,getPid,());

	// BPatch_process::getAddressWidth
	//
	// Get the address width (4 or 8) of the process

    API_EXPORT(Int, (),
    int,getAddressWidth,());

    //  BPatch_process::stopExecution
    //  
    //  Stop the mutatee process

    API_EXPORT(Int, (),
    bool,stopExecution,());

    //  BPatch_process::continueExecution
    //  
    //  Resume execution of mutatee process

    API_EXPORT(Int, (),
    bool,continueExecution,());

    //  BPatch_process::terminateExecution
    //  
    //  Terminate mutatee process

    API_EXPORT(Int, (),
    bool,terminateExecution,());

    //  BPatch_process::isStopped
    //  
    //  Returns true if mutatee process is currently stopped

    API_EXPORT(Int, (),
    bool,isStopped,());

    //  BPatch_process::stopSignal
    //  
    //  Returns signal number of signal that stopped mutatee process

    API_EXPORT(Int, (),
    int,stopSignal,());

    //  BPatch_process::isTerminated
    //  
    //  Returns true if mutatee process is terminated

    API_EXPORT(Int, (),
    bool,isTerminated,());

    //  BPatch_process::terminationStatus
    //  
    //  Returns information on how mutatee process was terminated

    API_EXPORT(Int, (),
    BPatch_exitType,terminationStatus,());

    //  BPatch_process::getExitCode
    //  
    //  Returns integer exit code of (exited) mutatee process

    API_EXPORT(Int, (),
    int,getExitCode,());

    //  BPatch_process::getExitSignal
    //  
    //  Returns integer signal number of signal that caused mutatee to exit

    API_EXPORT(Int, (),
    int,getExitSignal,());

    //  BPatch_process::detach
    //  
    //  Detach from the mutatee process, optionally leaving it running

    API_EXPORT(Int, (),
    bool,wasRunningWhenAttached,());

    //  BPatch_process::detach
    //  
    //  Detach from the mutatee process, optionally leaving it running

    API_EXPORT(Int, (cont),
    bool,detach,(bool cont));

    //  BPatch_process::isDetached
    //  
    //  Returns true if DyninstAPI is detached from this mutatee

    API_EXPORT(Int, (),
    bool,isDetached,());

    //  BPatch_process::getThreads
    //
    //  Fills a vector with the BPatch_thread objects that belong to
    //  this process
    API_EXPORT_V(Int, (thrds),
    void, getThreads, (BPatch_Vector<BPatch_thread *> &thrds));

    //  BPatch_prOcess::isMultithreaded
    //
    //  Returns true if this process has more than one thread
    API_EXPORT(Int, (),
    bool, isMultithreaded, ());

    //  BPatch_prOcess::isMultithreadCapable
    //
    //  Returns true if this process is linked against a thread library
    //  (and thus might be multithreaded)
    API_EXPORT(Int, (),
    bool, isMultithreadCapable, ());

    //  BPatch_process::getThread
    //
    //  Returns one of this process's threads, given a tid
    API_EXPORT(Int, (tid),
    BPatch_thread *, getThread, (dynthread_t tid));

    //  BPatch_process::getThread
    //
    //  Returns one of this process's threads, given an index
    API_EXPORT(Int, (index),
    BPatch_thread *, getThreadByIndex, (unsigned index));

    //  BPatch_process::dumpCore
    //  
    //  Produce a core dump file <file> for the mutatee process

    API_EXPORT(Int, (file, terminate),
    bool,dumpCore,(const char *file, bool terminate));

    //  BPatch_process::dumpImage
    //  
    //  Write contents of memory to <file>

    API_EXPORT(Int, (file),
    bool,dumpImage,(const char *file));

    //  BPatch_process::dumpPatchedImage
    //  
    //  Write executable image of mutatee, including runtime modifications, to <file>

    API_EXPORT(Int, (file),
    char *,dumpPatchedImage,(const char* file));


    //  BPatch_process::getInheritedVariable
    //  
    //  

    API_EXPORT(Int, (pVar),
    BPatch_variableExpr *,getInheritedVariable,(BPatch_variableExpr &pVar));

    //  BPatch_process::getInheritedSnippet
    //  
    //  

    API_EXPORT(Int, (parentSnippet),
    BPatchSnippetHandle *,getInheritedSnippet,(BPatchSnippetHandle &parentSnippet));


        //  BPatch_binaryEdit::beginInsertionSet()
    //
    //  Start the batch insertion of multiple points; all calls to insertSnippet*
    //  after this call will not actually instrument until finalizeInsertionSet is
    //  called

    API_EXPORT_V(Int, (),
                 void, beginInsertionSet, ());


    //BPatch_process::finalizeInsertionSet()
    
	// Finalizes all instrumentation logically added since a call to beginInsertionSet.
    //  Returns true if all instrumentation was successfully inserted; otherwise, none
    //  was. Individual instrumentation can be manipulated via the BPatchSnippetHandles
    //  returned from individual calls to insertSnippet.
    //
    //  atomic: if true, all instrumentation will be removed if any fails to go in.
    //  modified: if provided, and set to true by finalizeInsertionSet, additional
    //            steps were taken to make the installation work, such as modifying
    //            process state.  Note that such steps will be taken whether or not
    //            a variable is provided.

    API_EXPORT(Int, (atomic, modified),
               bool, finalizeInsertionSet, (bool atomic, bool *modified = NULL));
                                       
    
    API_EXPORT(Int, (atomic, modified, catchup_handles),
               bool, finalizeInsertionSetWithCatchup, (bool atomic, bool *modified,
                                            BPatch_Vector<BPatch_catchupInfo> &catchup_handles));
   
    
    //  BPatch_process::setMutationsActive
    //  
    //  Turn on/off instrumentation

    API_EXPORT(Int, (activate),
    bool,setMutationsActive,(bool activate));

    //  BPatch_process::oneTimeCode
    //  
    //  Have the specified code be executed by the mutatee once.  Wait until done.

    API_EXPORT(Int, (expr, err),
    void *,oneTimeCode,(const BPatch_snippet &expr, bool *err = NULL));

    //  BPatch_process::oneTimeCodeAsync
    //  
    //  Have the specified code be executed by the mutatee once.  Dont wait until done.

    API_EXPORT(Int, (expr, userData, cb),
    bool,oneTimeCodeAsync,(const BPatch_snippet &expr, void *userData = NULL,
                           BPatchOneTimeCodeCallback cb = NULL));
                           
    // BPatch_process::hideDebugger()
    //
    // This is a Windows only function that removes debugging artifacts that
    // are added to user-space datastructures and the heap of the debugged 
    // process, by CreateProcess and DebugActiveProcess.  Removing the artifacts
    // doesn't have any effect on the process, as the kernel still knows that 
    // the process is being debugged.  Three of the artifacts are flags that can
    // be reached through the Process Environment Block of the debuggee (PEB):
    //  1.  BeingDebugged, one byte at offset 2 in the PEB.  
    //  2.  NtGlobalFlags, at offset 0x68 in the PEB
    //  3.  There are two consecutive words of heap flags which are at offset 0x0c
    //      from the beginning of the heap.  The heap base address is at offset
    //      0x18 from the beginning of the PEB.  
    // The other thing this method does is clear the 0xabababababababab value that
    // it CreateProcess adds to the end of the heap section when creating a debugged
    // process, in response to the heap flag: HEAP_TAIL_CHECKING_ENABLED, which it
    // sets to true for debugged processes.  We are clearing that flag, but by the 
    // time we do, the value is already written to disk. 
    // 
    // Various system calls can still be used by the debuggee to recognize that 
    // it is being debugged, so this is not a complete solution.  
    API_EXPORT(Int, (),
    bool,hideDebugger,());

    //  BPatch_process::enableDumpPatchedImage
    //  
    //  
    API_EXPORT_V(Int, (),
    void,enableDumpPatchedImage,());

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Int, (name, loadaddr),
    bool,addSharedObject,(const char *name, const unsigned long loadaddr));
#endif

	// BPatch_process::loadLibrary
    //
    //  Load a shared library into the mutatee's address space
    //  Returns true if successful

    API_EXPORT_VIRT(Int, (libname, reload),
    bool, loadLibrary,(const char *libname, bool reload = false));

};

#endif /* BPatch_process_h_ */
