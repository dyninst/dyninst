/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

#ifndef _BPatch_process_h_
#define _BPatch_process_h_

#include "BPatch_snippet.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_eventLock.h"
#include "BPatch_point.h"
#include "BPatch_addressSpace.h"

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
    friend class process;
    friend class SignalHandler;
    friend int handleSignal(EventRecord &ev);
    friend void threadDeleteWrapper(BPatch_process *, BPatch_thread *); 
    friend bool pollForStatusChange();
    friend class AsyncThreadEventCallback;
    friend class AstNode; // AST needs to translate instPoint to
		      // BPatch_point via instp_map
    friend class AstOperatorNode;
    friend class AstMemoryNode;
    friend class rpcMgr;

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
    AddressSpace * getAS();
    bool getTerminated() {return terminated;}
    bool getMutationsActive() {return mutationsActive;}

    int activeOneTimeCodes_;
    bool resumeAfterCompleted_;

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
    BPatch_process(const char *path, const char *argv[], const char **envp = NULL, 
                  int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2);
    // for attaching
    BPatch_process(const char *path, int pid);	

    // for forking
    BPatch_process(process *proc);

    // Create a new thread in this proc
    BPatch_thread *createOrUpdateBPThread(int lwp, dynthread_t tid, unsigned index, 
                                          unsigned long stack_start, 
                                          unsigned long start_addr);
    BPatch_thread *handleThreadCreate(unsigned index, int lwpid, dynthread_t threadid, 
                            unsigned long stack_top, unsigned long start_pc, process *proc = NULL);
    void deleteBPThread(BPatch_thread *thrd);

    // These callbacks are triggered by lower-level code and forward
    // calls up to the findOrCreate functions.
    static BPatch_function *createBPFuncCB(AddressSpace *p, int_function *f);
    static BPatch_point *createBPPointCB(AddressSpace *p, int_function *f,
                                         instPoint *ip, int type);
    void updateThreadInfo();

        
    public:

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *lowlevel_process() { return llproc; }

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    BPatch_function *get_function(int_function *f);

    // DO NOT USE
    // This is an internal debugging function
    API_EXPORT_V(Int, (), 
    void, debugSuicide,());

  
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

    //  BPatch_process::insertSnippet
    //  
    //  Insert new code into the mutatee

    API_EXPORT(Int, (expr, point, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                           BPatch_snippetOrder order = BPatch_firstSnippet));

    //BPatch_process::insertSnippet
      
    //Insert new code into the mutatee, specifying "when" (before/after point)

    API_EXPORT(When, (expr, point, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                     BPatch_callWhen when,
                                       BPatch_snippetOrder order = BPatch_firstSnippet));

    //BPatch_process::insertSnippet
      
    //Insert new code into the mutatee at multiple points

    API_EXPORT(AtPoints, (expr, points, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                   const BPatch_Vector<BPatch_point *> &points,
                                       BPatch_snippetOrder order = BPatch_firstSnippet));

      // BPatch_process::insertSnippet
      
      //Insert new code into the mutatee at multiple points, specifying "when"

    API_EXPORT(AtPointsWhen, (expr, points, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));



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
                           

    //  BPatch_process::loadLibrary
    //  
    //  Load a shared library into the mutatee's address space
    //
    //  the reload argument is used by save the world to determine
    //  if this library should be reloaded by the mutated binary
    //  when it starts up. this is up to the user because loading
    //  an extra shared library could hide access to the 'correct'
    //  function by redefining a function  

    API_EXPORT(Int, (libname, reload),
    bool,loadLibrary,(const char *libname, bool reload = false));

    //  BPatch_process::enableDumpPatchedImage
    //  
    //  
    API_EXPORT_V(Int, (),
    void,enableDumpPatchedImage,());

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Int, (name, loadaddr),
    bool,addSharedObject,(const char *name, const unsigned long loadaddr));
#endif

};

#endif /* BPatch_process_h_ */
