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

#ifndef _BPatch_thread_h_
#define _BPatch_thread_h_

#include <stdio.h>

/*
 * The following is a kludge so that the functions that refer to signals (such
 * as BPatch_thread::stopSignal) can emulate the Unix behavior on Windows NT.
 */
#include <signal.h>
#ifndef SIGTRAP
#define SIGTRAP		5
#endif

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_snippet.h"
#include "BPatch_eventLock.h"

class process;
class miniTrampHandle;
class BPatch;
class BPatch_thread;
typedef enum {
  BPatch_nullEvent,
  BPatch_newConnectionEvent,
  BPatch_internalShutDownEvent,
  BPatch_threadCreateEvent,
  BPatch_threadStartEvent,
  BPatch_threadStopEvent,
  BPatch_threadDestroyEvent,
  BPatch_dynamicCallEvent
} BPatch_asyncEventType;



// BPatch_callWhen is defined in BPatch_point.h

/*
 * Used to specify whether a snippet should be installed before other snippets
 * that have previously been inserted at the same point, or after.
 */
typedef enum {
    BPatch_firstSnippet,
    BPatch_lastSnippet
} BPatch_snippetOrder;


/*
 * Contains information about the code that was inserted by an earlier call to
 * Bpatch_thread::insertSnippet.
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatchSnippetHandle

class BPATCH_DLL_EXPORT BPatchSnippetHandle : public BPatch_eventLock {
    friend class BPatch_thread;
    friend class BPatch_point;
private:
    BPatch_Vector<miniTrampHandle *> mtHandles;
      
    process *proc;

    BPatchSnippetHandle(process *_proc) : proc(_proc) {};

    void add(miniTrampHandle *pointInstance);
    void getMiniTrampHandles(BPatch_Vector<miniTrampHandle *> *save_mtHandles) {
      for(unsigned i=0; i<mtHandles.size(); i++)
	(*save_mtHandles).push_back(mtHandles[i]);
    }

public:
    API_EXPORT_DTOR(_dtor, (),
    ~,BPatchSnippetHandle,());
};

/*
 * The following types are used by BPatch_thread::getCallStack()
 */

// Stack frame types
typedef enum {
    BPatch_frameNormal,
    BPatch_frameSignal,
    BPatch_frameTrampoline
} BPatch_frameType;

typedef enum {
   NoExit,
   ExitedNormally,
   ExitedViaSignal
} BPatch_exitType;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_frame

// Contains information about a stack frame (used by
class BPATCH_DLL_EXPORT BPatch_frame : public BPatch_eventLock{
    friend class BPatch_thread;
    friend class BPatch_Vector<BPatch_frame>;
    BPatch_thread *thread;

    void *pc;
    void *fp;
    bool isSignalFrame;
    bool isTrampoline;
    BPatch_frame() : thread(NULL), pc(NULL), fp(NULL), 
                     isSignalFrame(false), isTrampoline(false) {};
    BPatch_frame(BPatch_thread *_thread, void *_pc, void *_fp, 
                 bool isf = false, bool istr = false ) :
	thread(_thread), pc(_pc), fp(_fp), isSignalFrame(isf), isTrampoline(istr) {};

public:
    //  BPatch_frame::getFrameType
    //  Returns type of frame: BPatch_frameNormal for a stack frame for a function,
    //  BPatch_frameSignal for the stack frame created when a signal is delivered,
    //  or BPatch_frameTrampoline for a stack frame created by internal Dyninst 
    //  instrumentation.
    API_EXPORT(Int, (),

    BPatch_frameType,getFrameType,());

    //  BPatch_frame::getPC
    //  Returns:  value of program counter
    API_EXPORT(Int, (),

    void *,getPC,()); 

    //  BPatch_frame::getFP
    API_EXPORT(Int, (),

    void *,getFP,()); 

    //  BPatch_frame::findFunction
    //  Returns:  the function corresponding to this stack frame, NULL if there is none
    API_EXPORT(Int, (),

    BPatch_function *,findFunction,());
   
    // The following are planned but no yet implemented:
    // int getSignalNumber();
    // BPatch_point *findPoint();
};

typedef void (*BPatchThreadEventCallback)(BPatch_thread *thr, unsigned long thread_id);

/*
 * Represents a thread of execution.
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_thread

class BPATCH_DLL_EXPORT BPatch_thread : public BPatch_eventLock {
    friend class BPatch;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_frame;
    friend class process;
    friend bool pollForStatusChange();
    friend class BPatch_asyncEventHandler;
    friend class ThreadLibrary;

    process *proc;
    BPatch_image *image;

    int	lastSignal;
    int exitCode;
    int exitSignal;
    bool exitedNormally;
    bool exitedViaSignal;
    bool mutationsActive;
    bool createdViaAttach;
    bool detached;

    bool unreportedStop;
    bool unreportedTermination;

    //pdvector<pdpair<BPatchDynamicCallSiteCallback, Address> > dynsiteCallbacks;
    //bool registerDynamicCallSiteCB(BPatchDynamicCallSiteCallback,
    //                               BPatch_point *);
    //BPatchDynamicCallSiteCallback getCallback(Address);

    void setUnreportedStop(bool new_value) {unreportedStop = new_value;}
    void setUnreportedTermination(bool new_value) {unreportedTermination = new_value;}

    void setExitedNormally() {exitedNormally = true;}
    void setExitedViaSignal(int signalnumber) {
           exitedViaSignal = true;
           lastSignal = signalnumber;
    }

    void setExitCode(int exitcode) { exitCode = exitcode; }
    void setExitSignal(int exitsignal) { exitSignal = exitsignal; }

    bool pendingUnreportedStop() { return unreportedStop;}
    bool pendingUnreportedTermination() { return unreportedTermination; }

    bool statusIsStopped();
    bool statusIsTerminated();

    static void	oneTimeCodeCallbackDispatch(process *theProc,
                                            unsigned /* rpcid */, 
                                            void *userData,
                                            void *returnValue);

    void *oneTimeCodeInternal(const BPatch_snippet &expr,
			      void *userData,
			      bool synchronous);

    protected:
    // for creating a process
    BPatch_thread(const char *path, char *argv[], char *envp[] = NULL, 
                  int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2);
    // for attaching
    BPatch_thread(const char *path, int pid);	

    // for forking
    BPatch_thread(int childPid, process *proc);

    public:

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *lowlevel_process() { return proc; }

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *PDSEP_process() { return proc; }

  
    //  BPatch_thread::~BPatch_thread
    //
    //  Destructor
    API_EXPORT_DTOR(_dtor, (),
    ~,BPatch_thread,());

    //  BPatch_thread::getImage
    //
    //  Obtain BPatch_image associated with this BPatch_thread

    API_EXPORT(Int, (),
    BPatch_image *,getImage,());
    
    //  BPatch_thread::getPid
    //  
    //  Get  id of mutatee process

    API_EXPORT(Int, (),
    int,getPid,());

    //  BPatch_thread::stopExecution
    //  
    //  Stop the mutatee process

    API_EXPORT(Int, (),
    bool,stopExecution,());

    //  BPatch_thread::continueExecution
    //  
    //  Resume execution of mutatee process

    API_EXPORT(Int, (),
    bool,continueExecution,());

    //  BPatch_thread::terminateExecution
    //  
    //  Terminate mutatee process

    API_EXPORT(Int, (),
    bool,terminateExecution,());

    //  BPatch_thread::isStopped
    //  
    //  Returns true if mutatee process is currently stopped

    API_EXPORT(Int, (),
    bool,isStopped,());

    //  BPatch_thread::stopSignal
    //  
    //  Returns signal number of signal that stopped mutatee process

    API_EXPORT(Int, (),
    int,stopSignal,());

    //  BPatch_thread::isTerminated
    //  
    //  Returns true if mutatee process is terminated

    API_EXPORT(Int, (),
    bool,isTerminated,());

    //  BPatch_thread::terminationStatus
    //  
    //  Returns information on how mutatee process was terminated

    API_EXPORT(Int, (),
    BPatch_exitType,terminationStatus,());

    //  BPatch_thread::getExitCode
    //  
    //  Returns integer exit code of (exited) mutatee process

    API_EXPORT(Int, (),
    int,getExitCode,());

    //  BPatch_thread::getExitSignal
    //  
    //  Returns integer signal number of signal that caused mutatee to exit

    API_EXPORT(Int, (),
    int,getExitSignal,());

    //  BPatch_thread::detach
    //  
    //  Detach from the mutatee process, optionally leaving it running

    API_EXPORT(Int, (cont),
    bool,detach,(bool cont));

    //  BPatch_thread::isDetached
    //  
    //  Returns true if DyninstAPI is detached from this mutatee

    API_EXPORT(Int, (),
    bool,isDetached,());

    //  BPatch_thread::dumpCore
    //  
    //  Produce a core dump file <file> for the mutatee process

    API_EXPORT(Int, (file, terminate),
    bool,dumpCore,(const char *file, bool terminate));

    //  BPatch_thread::dumpImage
    //  
    //  Write contents of memory to <file>

    API_EXPORT(Int, (file),
    bool,dumpImage,(const char *file));

    //  BPatch_thread::dumpPatchedImage
    //  
    //  Write executable image of mutatee, including runtime modifications, to <file>

    API_EXPORT(Int, (file),
    char *,dumpPatchedImage,(const char* file));

    //  BPatch_thread::malloc
    //  
    //  Allocate memory for a new variable in the mutatee process

    API_EXPORT(Int, (n),
    BPatch_variableExpr *,malloc,(int n));

    //  BPatch_thread::malloc
    //  
    //  Allocate memory for a new variable in the mutatee process

    API_EXPORT(ByType, (type),
    BPatch_variableExpr *,malloc,(const BPatch_type &type));

    //  BPatch_thread::free
    //  
    //  Free memory allocated by Dyninst in the mutatee process

    API_EXPORT(Int, (ptr),
    bool,free,(BPatch_variableExpr &ptr));

    //  BPatch_thread::getInheritedVariable
    //  
    //  

    API_EXPORT(Int, (pVar),
    BPatch_variableExpr *,getInheritedVariable,(BPatch_variableExpr &pVar));

    //  BPatch_thread::getInheritedSnippet
    //  
    //  

    API_EXPORT(Int, (parentSnippet),
    BPatchSnippetHandle *,getInheritedSnippet,(BPatchSnippetHandle &parentSnippet));

    //  BPatch_thread::insertSnippet
    //  
    //  Insert new code into the mutatee

    API_EXPORT(Int, (expr, point, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_thread::insertSnippet
    //  
    //  Insert new code into the mutatee, specifying "when" (before/after point)

    API_EXPORT(When, (expr, point, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, BPatch_point &point,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_thread::insertSnippet
    //  
    //  Insert new code into the mutatee at multiple points

    API_EXPORT(AtPoints, (expr, points, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));

    //  BPatch_thread::insertSnippet
    //  
    //  Insert new code into the mutatee at multiple points, specifying "when"

    API_EXPORT(AtPointsWhen, (expr, points, when, order),
    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                         const BPatch_Vector<BPatch_point *> &points,
                                         BPatch_callWhen when,
                                         BPatch_snippetOrder order = BPatch_firstSnippet));
    
    //  BPatch_thread::deleteSnippet
    //  
    //  Remove instrumentation from the mutatee process

    API_EXPORT(Int, (handle),
    bool,deleteSnippet,(BPatchSnippetHandle *handle));

    //  BPatch_thread::setMutationsActive
    //  
    //  Turn on/off instrumentation

    API_EXPORT(Int, (activate),
    bool,setMutationsActive,(bool activate));

    //  BPatch_thread::replaceFunctionCall
    //  
    //  Replace function call at one point with another

    API_EXPORT(Int, (point, newFunc),
    bool,replaceFunctionCall,(BPatch_point &point, BPatch_function &newFunc));

    //  BPatch_thread::removeFunctionCall
    //  
    //  Remove function call at one point 

    API_EXPORT(Int, (point),
    bool,removeFunctionCall,(BPatch_point &point));

    //  BPatch_thread::replaceFunction
    //  
    //  Replace all calls to a function with calls to another

    API_EXPORT(Int, (oldFunc, newFunc),
    bool,replaceFunction,(BPatch_function &oldFunc, BPatch_function &newFunc));

    //  BPatch_thread::oneTimeCode
    //  
    //  Have the specified code be executed by the mutatee once.  Wait until done.

    API_EXPORT(Int, (expr),
    void *,oneTimeCode,(const BPatch_snippet &expr));

    //  BPatch_thread::oneTimeCodeAsync
    //  
    //  Have the specified code be executed by the mutatee once.  Dont wait until done.

    API_EXPORT(Int, (expr, userData),
    bool,oneTimeCodeAsync,(const BPatch_snippet &expr, void *userData = NULL));

    //  BPatch_thread::loadLibrary
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

    //  BPatch_thread::getLineAndFile
    //  
    //  Method that retrieves the line number and file name corresponding 
    //  to an address

    API_EXPORT(Int, (addr, lineNo, fileName, length),
    bool,getLineAndFile,(unsigned long addr, unsigned short& lineNo,
                         char *fileName, int length));

    //  BPatch_thread::findFunctionByAddr
    //  
    //  Returns the function containing an address

    API_EXPORT(Int, (addr),
    BPatch_function *,findFunctionByAddr,(void *addr));

    //  BPatch_thread::getCallStack
    //  
    //  Returns a vector of BPatch_frame, representing the current call stack

    API_EXPORT(Int, (stack),
    bool,getCallStack,(BPatch_Vector<BPatch_frame>& stack));

    //  BPatch_thread::enableDumpPatchedImage
    //  
    //  

    API_EXPORT_V(Int, (),
    void,enableDumpPatchedImage,());

    //  BPatch_thread::registerThreadEventCallback
    //  
    //  Specifies a user defined function to call when a thread event of
    //  <type> occurs
    //  
    //  relevant event types:  BPatch_threadCreateEvent, BPatch_threadStartEvent
    //                         BPatch_threadStopEvent, BPatch_threadDestroyEvent 
    //  BPatchThreadEventCallback is:
    //  void (*BPatchThreadEventCallback)(BPatch_thread *thr, int thread_id);

    API_EXPORT(Int, (type,cb),
    bool,registerThreadEventCallback,(BPatch_asyncEventType type, 
                                      BPatchThreadEventCallback cb));

    API_EXPORT(Int, (type,cb),
    bool,removeThreadEventCallback,(BPatch_asyncEventType type,
                                    BPatchThreadEventCallback cb));

    //  BPatch_thread::registerThreadEventCallback
    //
    //  Specifies a user defined function to execute in the mutatee when a 
    //  thread event of <type> occurs.
    //  Typically function will be loaded into mutatee in a user defined 
    //  shared library (see loadLibrary()).
    //  The function <cb> _must_ have the following prototype:
    //    void user_cb(int thread_id);
    //  XXX  This will change.

    API_EXPORT(MutateeSide, (type,cb),
    bool,registerThreadEventCallback,(BPatch_asyncEventType type,
                                      BPatch_function *cb));

    API_EXPORT(MutateeSide, (type,cb),
    bool,removeThreadEventCallback,(BPatch_asyncEventType type,
                                    BPatch_function *cb));


#ifdef IBM_BPATCH_COMPAT

    API_EXPORT(Int, (),
    bool,isThreaded,());

    API_EXPORT(Int, (name, loadaddr),
    bool,addSharedObject,(const char *name, const unsigned long loadaddr));
#endif

};

#endif /* BPatch_thread_h_ */
