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
  friend class AstNode;
  friend class AstOperatorNode;
  friend class AstMemoryNode;
  friend class PCEventHandler;

 protected:
  void getAS(std::vector<AddressSpace *> &as);

 public:
  void PDSEP_updateObservedCostAddr(unsigned long a);
 private:

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
			    BPatch_thread *thread,
			    void *userData,
			    BPatchOneTimeCodeCallback cb = NULL,
			    bool synchronous = true, bool *err = NULL,
			    bool userRPC = true);

 protected:

  BPatch_process(const char *path, const char *argv[], 
		 BPatch_hybridMode mode, const char **envp = NULL, 
		 int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2);

  BPatch_process(const char *path, int pid, BPatch_hybridMode mode);	

  BPatch_process(PCProcess *proc);

  void triggerThreadCreate(PCThread *thread);
  void triggerInitialThreadEvents();
  void deleteBPThread(BPatch_thread *thrd);

 public:

  PCProcess *lowlevel_process() const { return llproc; }

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
  ( std::map<Dyninst::Address,unsigned char*>& owPages,
    std::vector<std::pair<Dyninst::Address,int> >& deadBlocks,
    std::vector<BPatch_function*>& owFuncs,
    std::set<BPatch_function *> &monitorFuncs,
    bool &changedPages, bool &changedCode );
  HybridAnalysis *getHybridAnalysis() { return hybridAnalysis_; }
  bool protectAnalyzedCode();

  void  debugSuicide();

  ~BPatch_process();
       
  int getPid();

  int getAddressWidth();

  bool stopExecution();

  bool continueExecution();

  bool terminateExecution();

  bool isStopped();

  int stopSignal();

  bool isTerminated();

  BPatch_exitType terminationStatus();

  int getExitCode();

  int getExitSignal();

  bool wasRunningWhenAttached();

  bool detach(bool cont);

  bool isDetached();

  void  getThreads(BPatch_Vector<BPatch_thread *> &thrds);

  bool  isMultithreaded();

  bool  isMultithreadCapable();

  BPatch_thread * getThread(dynthread_t tid);

  BPatch_thread * getThreadByIndex(unsigned index);

  bool dumpCore(const char *file, bool terminate);

  bool dumpImage(const char *file);

  BPatch_variableExpr * getInheritedVariable(BPatch_variableExpr &pVar);

  BPatchSnippetHandle * getInheritedSnippet(BPatchSnippetHandle &parentSnippet);

  void  beginInsertionSet();

  bool  finalizeInsertionSet(bool atomic, bool *modified = NULL);
                                       
  bool  finalizeInsertionSetWithCatchup(bool atomic, bool *modified,
					BPatch_Vector<BPatch_catchupInfo> &catchup_handles);
   
  void* oneTimeCode(const BPatch_snippet &expr, bool *err = NULL);

  bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL,
			BPatchOneTimeCodeCallback cb = NULL);
                           
  bool hideDebugger();
#if 0
  void  printDefensiveStats();
#endif

  virtual BPatch_object * loadLibrary(const char *libname, bool reload = false);

  bool supportsUserThreadEvents();

};

#endif /* BPatch_process_h_ */
