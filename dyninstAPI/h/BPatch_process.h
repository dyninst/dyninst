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

#include <vector>

#include <stdio.h>
#include <signal.h>

class process;
class miniTramp;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_funcMap;
class BPatch_instpMap;
class int_function;

typedef enum {
  BPatch_nullEvent,
  BPatch_newConnectionEvent,
  BPatch_internalShutDownEvent,
  BPatch_threadCreateEvent,
  BPatch_threadStartEvent,
  BPatch_threadStopEvent,
  BPatch_threadDestroyEvent,
  BPatch_dynamicCallEvent,
  BPatch_errorEvent,
  BPatch_dynLibraryEvent,
  BPatch_preForkEvent,
  BPatch_postForkEvent,
  BPatch_execEvent,
  BPatch_exitEvent,
  BPatch_signalEvent,
  BPatch_userEvent,
  BPatch_oneTimeCodeEvent
} BPatch_asyncEventType;


/*
 * Contains information about the code that was inserted by an earlier call to
 * Bpatch_thread::insertSnippet.
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatchSnippetHandle

class BPATCH_DLL_EXPORT BPatchSnippetHandle : public BPatch_eventLock {
    friend class BPatch_point;
    friend class BPatch_image;
    friend class BPatch_process;
protected:
    process *proc;
private:
    BPatch_Vector<miniTramp *> mtHandles;
      
    BPatchSnippetHandle(process *_proc) : proc(_proc) {};

    void add(miniTramp *pointInstance);
    void getMiniTrampHandles(BPatch_Vector<miniTramp*> *save_mtHandles) {
      for(unsigned i=0; i<mtHandles.size(); i++)
	(*save_mtHandles).push_back(mtHandles[i]);
    }

public:
    API_EXPORT_DTOR(_dtor, (),
    ~,BPatchSnippetHandle,());
};

typedef void (*BPatchAsyncThreadEventCallback)(BPatch_thread *thr, unsigned long thread_id);
typedef void (*BPatchUserEventCallback)(void *buf, unsigned int bufsize);

typedef enum {
   NoExit,
   ExitedNormally,
   ExitedViaSignal
} BPatch_exitType;

/*
 * Represents a process
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_process

class BPATCH_DLL_EXPORT BPatch_process : public BPatch_eventLock {
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
    friend class ThreadLibrary;
    friend class BPatch_point;
    friend class BPatch_funcCallExpr;
    friend class BPatch_eventMailbox;
    friend class process;
    friend bool pollForStatusChange();
    friend class AstNode; // AST needs to translate instPoint to
		      // BPatch_point via instp_map
    
    //References to lower level objects
    process *llproc;
    BPatch_funcMap *func_map;
    BPatch_instpMap *instp_map;


    BPatch_image *image;
    BPatch_Vector<BPatch_thread *> threads;

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

    static void oneTimeCodeCallbackDispatch(process *theProc,
                                            unsigned /* rpcid */, 
                                            void *userData,
                                            void *returnValue);

    void *oneTimeCodeInternal(const BPatch_snippet &expr,
                              void *userData,
                              bool synchronous);

    protected:
    // for creating a process
    BPatch_process(const char *path, char *argv[], char *envp[] = NULL, 
                  int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2);
    // for attaching
    BPatch_process(const char *path, int pid);	

    // for forking
    BPatch_process(int childPid, process *proc);

    BPatch_function *findOrCreateBPFunc(int_function *ifunc, 
                                        BPatch_module *bpmod);
    BPatch_point *findOrCreateBPPoint(BPatch_function *bpfunc, instPoint *ip,
                                      BPatch_procedureLocation pointType);

    // These callbacks are triggered by lower-level code and forward
    // calls up to the findOrCreate functions.
    static BPatch_function *createBPFuncCB(process *p, int_function *f);
    static BPatch_point *createBPPointCB(process *p, int_function *f,
                                         instPoint *ip, int type);
    public:

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *lowlevel_process() { return llproc; }

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *PDSEP_process() { return llproc; }

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    BPatch_function *get_function(int_function *f);

  
    //  BPatch_process::~BPatch_process
    //
    //  Destructor
    API_EXPORT_DTOR(_dtor, (),
    ~,BPatch_process,());

    //  BPatch_process::getImage
    //
    //  Obtain BPatch_image associated with this BPatch_process

    API_EXPORT(Int, (),
    BPatch_image *,getImage,());
    
    //  BPatch_process::getPid
    //  
    //  Get  id of mutatee process

    API_EXPORT(Int, (),
    int,getPid,());

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

    //  BPatch_process::getThread
    //
    //  Returns one of this process's threads, given a tid
    API_EXPORT(Int, (tid),
    BPatch_thread *, getThread, (unsigned tid));

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

    //  BPatch_process::malloc
    //  
    //  Allocate memory for a new variable in the mutatee process

    API_EXPORT(Int, (n),
    BPatch_variableExpr *,malloc,(int n));

    //  BPatch_process::malloc
    //  
    //  Allocate memory for a new variable in the mutatee process

    API_EXPORT(ByType, (type),
    BPatch_variableExpr *,malloc,(const BPatch_type &type));

    //  BPatch_process::free
    //  
    //  Free memory allocated by Dyninst in the mutatee process

    API_EXPORT(Int, (ptr),
    bool,free,(BPatch_variableExpr &ptr));

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

    //  BPatch_process::insertSnippet
    //  
    //  Insert new code into the mutatee, specifying "when" (before/after point)

    API_EXPORT(When, (expr, point, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_process::insertSnippet
    //  
    //  Insert new code into the mutatee at multiple points

    API_EXPORT(AtPoints, (expr, points, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_process::insertSnippet
    //  
    //  Insert new code into the mutatee at multiple points, specifying "when"

    API_EXPORT(AtPointsWhen, (expr, points, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));
    
    //  BPatch_process::deleteSnippet
    //  
    //  Remove instrumentation from the mutatee process

    API_EXPORT(Int, (handle),
    bool,deleteSnippet,(BPatchSnippetHandle *handle));

    // Occasionally users need to insert a set of instrumentation as an atomic action.
    // We provide this capability by "wrapping" a set of instrumentation with an explicit
    // start/end pair.
    // Of course, for this to work correctly Dyninst will need to honor the "checkInst"
    // section of an instPoint, which currently does not happen. Fun.

    API_EXPORT_V(Int, (), 
    void, startInsertionRange, ());


    API_EXPORT_V(Int, (), 
    void, endInsertionRange, ());

    //  BPatch_process::setMutationsActive
    //  
    //  Turn on/off instrumentation

    API_EXPORT(Int, (activate),
    bool,setMutationsActive,(bool activate));

    //  BPatch_process::replaceFunctionCall
    //  
    //  Replace function call at one point with another

    API_EXPORT(Int, (point, newFunc),
    bool,replaceFunctionCall,(BPatch_point &point, BPatch_function &newFunc));

    //  BPatch_process::removeFunctionCall
    //  
    //  Remove function call at one point 

    API_EXPORT(Int, (point),
    bool,removeFunctionCall,(BPatch_point &point));

    //  BPatch_process::replaceFunction
    //  
    //  Replace all calls to a function with calls to another

    API_EXPORT(Int, (oldFunc, newFunc),
    bool,replaceFunction,(BPatch_function &oldFunc, BPatch_function &newFunc));

    //  BPatch_process::oneTimeCode
    //  
    //  Have the specified code be executed by the mutatee once.  Wait until done.

    API_EXPORT(Int, (expr),
    void *,oneTimeCode,(const BPatch_snippet &expr));

    //  BPatch_process::oneTimeCodeAsync
    //  
    //  Have the specified code be executed by the mutatee once.  Dont wait until done.

    API_EXPORT(Int, (expr, userData),
    bool,oneTimeCodeAsync,(const BPatch_snippet &expr, void *userData = NULL));

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

    //  BPatch_process::getAllCallStacks
    //  
    //  Returns a vector of (vector of) BPatch_frame, representing the current call stack
    API_EXPORT(Int, (stack),
    bool,getAllCallStacks,(BPatch_Vector<BPatch_Vector<BPatch_frame> >& stack));

    //  BPatch_process::getLineAndFile
    //  
    //  Method that retrieves the line number and file name corresponding 
    //  to an address

    API_EXPORT(Int, (addr, lines),
    bool,getSourceLines,(unsigned long addr, std::vector< std::pair< const char *, unsigned int > > & lines ));
	
    //  BPatch_process::findFunctionByAddr
    //  
    //  Returns the function containing an address

    API_EXPORT(Int, (addr),
    BPatch_function *,findFunctionByAddr,(void *addr));

    //  BPatch_process::enableDumpPatchedImage
    //  
    //  
    API_EXPORT_V(Int, (),
    void,enableDumpPatchedImage,());

    //  BPatch_process::registerAsyncThreadEventCallback
    //  
    //  Specifies a user defined function to call when a thread event of
    //  <type> occurs
    //  
    //  relevant event types:  BPatch_threadCreateEvent, BPatch_threadStartEvent
    //                         BPatch_threadStopEvent, BPatch_threadDestroyEvent 
    //  BPatchAsyncThreadEventCallback is:
    //  void (*BPatchAsyncThreadEventCallback)(BPatch_thread *thr, int thread_id);

    API_EXPORT(Int, (type,cb),
    bool,registerAsyncThreadEventCallback,(BPatch_asyncEventType type, 
                                           BPatchAsyncThreadEventCallback cb));

    API_EXPORT(Int, (type,cb),
    bool,removeAsyncThreadEventCallback,(BPatch_asyncEventType type,
                                         BPatchAsyncThreadEventCallback cb));

    //  BPatch_thread::registerAsyncThreadEventCallback
    //
    //  Specifies a user defined function to execute in the mutatee when a 
    //  thread event of <type> occurs.
    //  Typically function will be loaded into mutatee in a user defined 
    //  shared library (see loadLibrary()).
    //  The function <cb> _must_ have the following prototype:
    //    void user_cb(int thread_id);
    //  XXX  This will change.

    API_EXPORT(MutateeSide, (type,cb),
    bool,registerAsyncThreadEventCallback,(BPatch_asyncEventType type,
                                           BPatch_function *cb));

    API_EXPORT(MutateeSide, (type,cb),
    bool,removeAsyncThreadEventCallback,(BPatch_asyncEventType type,
                                         BPatch_function *cb));



    //  BPatch_process::registerUserEventCallback
    //  
    //  Specifies a user defined function to call when a "user event" 
    //  occurs, user events are trigger by calls to the function DYNINSTuserMessage(void *, int)
    //  in the runtime library.
    //  
    //  BPatchUserEventCallback is:
    //  void (*BPatchUserEventCallback)(void *msg, unsigned int msg_size);

    API_EXPORT(Int, (cb),
    bool,registerUserEventCallback,(BPatchUserEventCallback cb)); 

    API_EXPORT(Int, (cb),
    bool,removeUserEventCallback,(BPatchUserEventCallback cb));

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Int, (name, loadaddr),
    bool,addSharedObject,(const char *name, const unsigned long loadaddr));
#endif

};

#endif /* BPatch_process_h_ */
