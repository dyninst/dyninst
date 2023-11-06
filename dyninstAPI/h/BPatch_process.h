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

#ifndef _BPatch_process_h_
#define _BPatch_process_h_

#include "BPatch_snippet.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
// #include "BPatch_image.h"
#include "BPatch_addressSpace.h"
#include "BPatch_enums.h"
#include "dyntypes.h"
#include "BPatch_callbacks.h"
#include "PCProcess.h"

#include <map>
#include <set>
#include <stddef.h>
#include <utility>
#include <vector>

#include <cstdio>
#include <csignal>


class PCProcess;
class AddressSpace;
class PCThread;
class miniTrampHandle;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_point;
class BPatch_funcMap;
class BPatch_instpMap;
class func_instance;
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

typedef Dyninst::THR_ID dynthread_t;


typedef struct {
  BPatch_snippet snip;
  BPatchSnippetHandle *sh;
  BPatch_thread *thread;
} BPatch_catchupInfo;

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
  BPatchOneTimeCodeCallback cb;
  void *returnValue;
  unsigned thrID;
 public:
 OneTimeCodeInfo(bool _synchronous, void *_userData, BPatchOneTimeCodeCallback _cb, unsigned _thrID) :
  synchronous(_synchronous), completed(false), userData(_userData), cb(_cb),
  returnValue(NULL), thrID(_thrID) { }

  bool isSynchronous() { return synchronous; }

  bool isCompleted() const { return completed; }
  void setCompleted(bool _completed) { completed = _completed; }

  void *getUserData() { return userData; }

  void setReturnValue(void *_returnValue) { returnValue = _returnValue; }
  void *getReturnValue() { return returnValue; }

  unsigned getThreadID() { return thrID; }

  BPatchOneTimeCodeCallback getCallback() { return cb;}
   
};


/*
 * Represents a process
 */
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
  friend class AstNode; // AST needs to translate instPoint to
  friend class AstOperatorNode;
  friend class AstMemoryNode;
  friend class PCEventHandler; // to deliver events for callbacks

 protected:
  void getAS(std::vector<AddressSpace *> &as);

 public:
  void PDSEP_updateObservedCostAddr(unsigned long a);
 private:

  //References to lower level objects
  PCProcess *llproc;

  BPatch_Vector<BPatch_thread *> threads;

  int lastSignal;
  int exitCode;
  int exitSignal;
  bool exitedNormally;
  bool exitedViaSignal;
  bool mutationsActive;
  bool createdViaAttach;
  bool detached;

  // BPatch-level; once the callbacks are sent by the llproc, we're terminated
  // Used because callbacks go (and can clean up user code) before the low-level process
  // sets flags.
  bool terminated; 
  bool reportedExit;

  void setExitedNormally();
  void setExitedViaSignal(int signalnumber);

  void setExitCode(int exitcode) { exitCode = exitcode; }
  void setExitSignal(int exitsignal) { exitSignal = exitsignal; }

  bool statusIsTerminated();

  void setLastSignal(int signal) { lastSignal = signal; }

  processType getType();
  bool getTerminated() {return terminated;}
  bool getMutationsActive() {return mutationsActive;}

  HybridAnalysis *hybridAnalysis_;

  static int oneTimeCodeCallbackDispatch(PCProcess *theProc,
					 unsigned /* rpcid */, 
					 void *userData,
					 void *returnValue);

  void *oneTimeCodeInternal(const BPatch_snippet &expr,
			    BPatch_thread *thread, // == NULL if proc-wide
			    void *userData,
			    BPatchOneTimeCodeCallback cb = NULL,
			    bool synchronous = true, bool *err = NULL,
			    bool userRPC = true);

 protected:
  // for creating a process
  BPatch_process(const char *path, const char *argv[], 
		 BPatch_hybridMode mode, const char **envp = NULL, 
		 int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2);
  // for attaching
  BPatch_process(const char *path, int pid, BPatch_hybridMode mode);	

  // for forking
  BPatch_process(PCProcess *proc);

  void triggerThreadCreate(PCThread *thread);
  void triggerInitialThreadEvents();
  void deleteBPThread(BPatch_thread *thrd);

 public:

  // Begin internal functions, DO NOT USE
  //
  // this function should go away as soon as Paradyn links against Dyninst
  PCProcess *lowlevel_process() const { return llproc; }
  // These internal funcs trigger callbacks registered to matching events
  bool triggerStopThread(instPoint *intPoint, func_instance *intFunc, 
			 int cb_ID, void *retVal);
  bool triggerSignalHandlerCB(instPoint *point, func_instance *func, long signum, 
			      BPatch_Vector<Dyninst::Address> *handlers); 
  bool triggerCodeOverwriteCB(instPoint * faultPoint,
			      Dyninst::Address faultTarget); 
  bool setMemoryAccessRights(Dyninst::Address start, size_t size, 
			     Dyninst::ProcControlAPI::Process::mem_perm rights);
  unsigned char *makeShadowPage(Dyninst::Address pageAddress);
  void overwriteAnalysisUpdate
  ( std::map<Dyninst::Address,unsigned char*>& owPages, //input
    std::vector<std::pair<Dyninst::Address,int> >& deadBlocks, //output
    std::vector<BPatch_function*>& owFuncs,     //output
    std::set<BPatch_function *> &monitorFuncs, //output
    bool &changedPages, bool &changedCode ); //output
  HybridAnalysis *getHybridAnalysis() { return hybridAnalysis_; }
  bool protectAnalyzedCode();
  // DO NOT USE
  // This is an internal debugging function
  void  debugSuicide();
  //
  // End internal functions


  
  //  BPatch_process::~BPatch_process
  //
  //  Destructor
  ~BPatch_process();

       
  //  BPatch_process::getPid
  //  
  //  Get  id of mutatee process

  int getPid();

  // BPatch_process::getAddressWidth
  //
  // Get the address width (4 or 8) of the process

  int getAddressWidth();

  //  BPatch_process::stopExecution
  //  
  //  Stop the mutatee process

  bool stopExecution();

  //  BPatch_process::continueExecution
  //  
  //  Resume execution of mutatee process

  bool continueExecution();

  //  BPatch_process::terminateExecution
  //  
  //  Terminate mutatee process

  bool terminateExecution();

  //  BPatch_process::isStopped
  //  
  //  Returns true if mutatee process is currently stopped

  bool isStopped();

  //  BPatch_process::stopSignal
  //  
  //  Returns signal number of signal that stopped mutatee process

  int stopSignal();

  //  BPatch_process::isTerminated
  //  
  //  Returns true if mutatee process is terminated

  bool isTerminated();

  //  BPatch_process::terminationStatus
  //  
  //  Returns information on how mutatee process was terminated

  BPatch_exitType terminationStatus();

  //  BPatch_process::getExitCode
  //  
  //  Returns integer exit code of (exited) mutatee process

  int getExitCode();

  //  BPatch_process::getExitSignal
  //  
  //  Returns integer signal number of signal that caused mutatee to exit

  int getExitSignal();

  //  BPatch_process::detach
  //  
  //  Detach from the mutatee process, optionally leaving it running

  bool wasRunningWhenAttached();

  //  BPatch_process::detach
  //  
  //  Detach from the mutatee process, optionally leaving it running

  bool detach(bool cont);

  //  BPatch_process::isDetached
  //  
  //  Returns true if DyninstAPI is detached from this mutatee

  bool isDetached();

  //  BPatch_process::getThreads
  //
  //  Fills a vector with the BPatch_thread objects that belong to
  //  this process
  void  getThreads(BPatch_Vector<BPatch_thread *> &thrds);

  //  BPatch_prOcess::isMultithreaded
  //
  //  Returns true if this process has more than one thread
  bool  isMultithreaded();

  //  BPatch_prOcess::isMultithreadCapable
  //
  //  Returns true if this process is linked against a thread library
  //  (and thus might be multithreaded)
  bool  isMultithreadCapable();

  //  BPatch_process::getThread
  //
  //  Returns one of this process's threads, given a tid
  BPatch_thread * getThread(dynthread_t tid);

  //  BPatch_process::getThread
  //
  //  Returns one of this process's threads, given an index
  BPatch_thread * getThreadByIndex(unsigned index);

  //  BPatch_process::dumpCore
  //  
  //  Produce a core dump file <file> for the mutatee process

  bool dumpCore(const char *file, bool terminate);

  //  BPatch_process::dumpImage
  //  
  //  Write contents of memory to <file>

  bool dumpImage(const char *file);

  //  BPatch_process::getInheritedVariable
  //  
  //  

  BPatch_variableExpr * getInheritedVariable(BPatch_variableExpr &pVar);

  //  BPatch_process::getInheritedSnippet
  //  
  //  

  BPatchSnippetHandle * getInheritedSnippet(BPatchSnippetHandle &parentSnippet);


  //  BPatch_binaryEdit::beginInsertionSet()
  //
  //  Start the batch insertion of multiple points; all calls to insertSnippet*
  //  after this call will not actually instrument until finalizeInsertionSet is
  //  called

  void  beginInsertionSet();


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

  bool  finalizeInsertionSet(bool atomic, bool *modified = NULL);
                                       
    
  bool  finalizeInsertionSetWithCatchup(bool atomic, bool *modified,
					BPatch_Vector<BPatch_catchupInfo> &catchup_handles);
   
    
  //  BPatch_process::oneTimeCode
  //  
  //  Have the specified code be executed by the mutatee once.  Wait until done.

  void* oneTimeCode(const BPatch_snippet &expr, bool *err = NULL);

  //  BPatch_process::oneTimeCodeAsync
  //  
  //  Have the specified code be executed by the mutatee once.  Dont wait until done.

  bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL,
			BPatchOneTimeCodeCallback cb = NULL);
                           
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
  bool hideDebugger();
#if 0
  void  printDefensiveStats();
#endif

  // BPatch_process::loadLibrary
  //
  //  Load a shared library into the mutatee's address space
  //  Returns true if successful

  virtual BPatch_object * loadLibrary(const char *libname, bool reload = false);

  bool supportsUserThreadEvents();

};

#endif /* BPatch_process_h_ */
