/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 * process.h - interface to manage a process in execution. A process is a kernel
 *   visible unit with a seperate code and data space.  It might not be
 *   the only unit running the code, but it is only one changed when
 *   ptrace updates are applied to the text space.
 *
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include "rtinst/h/rtinst.h"
#include "dyninstAPI/src/util.h"
#include "util/h/String.h"
#include "util/h/vectorSet.h"
#include "util/h/Dictionary.h"
#include "util/h/Types.h"
#include "util/h/Timer.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/os.h"
// #include "paradynd/src/main.h"
#include "paradynd/src/showerror.h"

#include "dyninstAPI/src/symtab.h" // internalSym

#ifdef SHM_SAMPLING
#include "paradynd/src/fastInferiorHeapMgr.h"
#include "paradynd/src/superTable.h"
#include "paradynd/src/hashTable.h"
#ifdef sparc_sun_sunos4_1_3
#include <kvm.h>
#include <sys/user.h>
#endif
#endif

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"

extern unsigned activeProcesses; // number of active processes
   // how about just processVec.size() instead?  At least, this should be made
   // a (static) member vrble of class process

class resource;
class instPoint;
class instInstance;
class trampTemplate;
class pdThread;

// TODO a kludge - to prevent recursive includes
class image;

typedef enum { neonatal, running, stopped, exited } processState;
typedef enum { HEAPfree, HEAPallocated } heapStatus;
typedef enum { textHeap=0, dataHeap=1 } inferiorHeapType;
typedef vector<unsigned> unsigVecType;

const int LOAD_DYNINST_BUF_SIZE = 64;

class heapItem {
 public:
  heapItem() {
    addr =0; length = 0; status = HEAPfree;
  }
  heapItem(const heapItem *h) {
    addr = h->addr; length = h->length; status = h->status;
  }
  Address addr;
  int length;
  heapStatus status;
};

//
// an item on the heap that we are trying to free.
//
class disabledItem {
 public:
  disabledItem() { pointer = 0; }
  disabledItem(unsigned iPointer, inferiorHeapType iHeapType, const vector<unsigVecType> &iPoints) : pointsToCheck(iPoints) {
     pointer = iPointer;
     whichHeap = iHeapType;
  }
  disabledItem(const disabledItem &src) : pointsToCheck(src.pointsToCheck) {
    pointer = src.pointer; 
    whichHeap = src.whichHeap;
  }
 ~disabledItem() {}
  disabledItem &operator=(const disabledItem &src) {
     if (&src == this) return *this; // the usual check for x=x

     pointer = src.pointer;
     whichHeap = src.whichHeap;
     pointsToCheck = src.pointsToCheck;
     return *this;
  }
  
  unsigned getPointer() const {return pointer;}
  inferiorHeapType getHeapType() const {return whichHeap;}
  const vector<unsigVecType> &getPointsToCheck() const {return pointsToCheck;}
  vector<unsigVecType> &getPointsToCheck() {return pointsToCheck;}

 private:
  unsigned pointer;			// address in heap
  inferiorHeapType whichHeap;		// which heap is it in
  vector<unsigVecType> pointsToCheck;	// range of addrs to check
};


class inferiorHeap {
 public:
  inferiorHeap(): heapActive(addrHash16) {
      freed = 0; disabledListTotalMem = 0; totalFreeMemAvailable = 0;
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
      base = 0; size = 0;
#endif
  }
  inferiorHeap(const inferiorHeap &src);  // create a new heap that is a copy of src.
                                          // used on fork.
  dictionary_hash<unsigned, heapItem*> heapActive; // active part of heap 
  vector<heapItem*> heapFree;  		// free block of data inferior heap 
  vector<disabledItem> disabledList;	// items waiting to be freed.
  int disabledListTotalMem;		// total size of item waiting to free
  int totalFreeMemAvailable;		// total free memory in the heap
  int freed;				// total reclaimed (over time)

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  Address base;				// base of heap
  int size;				// size of heap
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */
};

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
class mutationRecord {
public:
    mutationRecord *next;
    mutationRecord *prev;

    Address	addr;
    int		size;
    void	*data;

    mutationRecord(Address _addr, int _size, const void *_data);
    ~mutationRecord();
};

class mutationList {
private:
    mutationRecord	*head;
    mutationRecord	*tail;
public:
    mutationList() : head(NULL), tail(NULL) {};
    ~mutationList();

    void insertHead(Address addr, int size, const void *data);
    void insertTail(Address addr, int size, const void *data);
    mutationRecord *getHead() { return head; }
    mutationRecord *getTail() { return tail; }
};
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

static inline unsigned ipHash(const instPoint * const &ip)
{
  // assume all addresses are 4-byte aligned
  unsigned result = (unsigned)ip;
  result >>= 2;
  return result;
  // how about %'ing by a huge prime number?  Nah, x % y == x when x < y 
  // so we don't want the number to be huge.
}


static inline unsigned instInstanceHash(instInstance * const &inst) {
   unsigned result = (unsigned)inst;
   result >>= 2;
   return result; // how about %'ing by a huge prime number?
//  return ((unsigned)inst);
}

class Frame;

class process {
 friend class ptraceKludge;
#ifdef BPATCH_LIBRARY
 friend class BPatch_image;
#endif

 public:
  process(int iPid, image *iImage, int iTraceLink, int iIoLink
#ifdef SHM_SAMPLING
	  , key_t theShmSegKey,
	  const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
	  );
     // this is the "normal" ctor

  process(int iPid, image *iSymbols,
          int afterAttach, // 1 --> pause, 2 --> run, 0 --> leave as is
	  bool& success 
#ifdef SHM_SAMPLING
	  , key_t theShmSegKey,
	  const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
	  );
     // this is the "attach" ctor

  process(const process &parentProc, int iPid, int iTrace_fd
#ifdef SHM_SAMPLING
	  , key_t theShmSegKey,
	  void *applShmSegPtr,
	  const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
	  );
     // this is the "fork" ctor

#ifdef SHM_SAMPLING
  void registerInferiorAttachedSegs(void *inferiorAttachedAtPtr);
     // Where the inferior attached was left undefined in the constructor;
     // this routine fills it in (tells paradynd where, in the inferior proc's addr
     // space, the shm seg was attached.  The attaching was done in DYNINSTinit)
#endif

  vector<Address> walkStack(bool noPause=false);

  // 
  // getActiveFrame and readDataFromFrame are platform dependant
  //
  bool getActiveFrame(int *fp, int *pc);
  bool readDataFromFrame(int currentFP, int *previousFP, int *rtn, 
			 bool uppermost=false);


#ifdef SHM_SAMPLING
  time64 getInferiorProcessCPUtime();
     // returns user+sys time from the u or proc area of the inferior process, which in
     // turn is presumably obtained by mmapping it (sunos) or by using a /proc ioctl
     // to obtain it (solaris).  It is hoped that the implementation would not have to
     // pause the inferior process (and then unpause it) in order to obtain the result,
     // since pausing and unpausing are extremely slow (I've seen ~70ms).
#endif

  processState status() const { return status_;}
  string getStatusAsString() const; // useful for debug printing etc.

  void continueAfterNextStop() { continueAfterNextStop_ = true; }
  void Exited();
  void Stopped();

  // the following 2 vrbles probably belong in a different class:
  static string programName; // the name of paradynd (more specifically, its argv[0])
  static vector<string> arg_list; // the arguments of paradynd
  static string pdFlavor ;

  bool findInternalSymbol(const string &name, bool warn, internalSym &ret_sym) const;

  Address findInternalAddress(const string &name, bool warn, bool &err) const;

  bool dumpImage();

  bool symbol_info(const string &name, Symbol &ret) {
     assert(symbols);
     return symbols->symbol_info(name, ret);
  }
  image *getImage() const {
     assert(symbols);
     return symbols;
  }

  // this is only used on aix so far - naim
  vector<int> getTOCoffsetInfo() const;

 private:
  struct inferiorRPCtoDo {
     // This structure keeps track of an inferiorRPC that we will start sometime
     // in the (presumably near) future.  There is a different structure for RPCs
     // which have been launched and which we're waiting to finish.  Don't confuse
     // the two.

     AstNode *action;
     bool noCost; // if true, cost model isn't updated by generated code.
     void (*callbackFunc)(process *, void *userData, unsigned result);
     void *userData;
     int mid;
  };
  vectorSet<inferiorRPCtoDo> RPCsWaitingToStart;
  bool RPCs_waiting_for_syscall_to_complete;
  void *save_exitset_ptr; // platform-specific (for now, just solaris;
			  // it's actually a sysset_t*)
					       
  struct inferiorRPCinProgress {
     // This structure keeps track of an inferiorRPC that has been launched and
     // for which we're waiting to complete.  Don't confuse with 'inferiorRPCtoDo',
     // which is more of a wait queue of RPCs to start launching.
     // Also note: It's only safe for 1 RPC to be in progress at a time.
     // If you _really_ want to launch multiple RPCs at the same time, it's actually
     // easy to do...just do one inferiorRPC with a sequenceNode AST! (neat, eh?)
     // (Admittedly, that does confuse the semantics of callback functions.  So
     // the official line remains: only 1 inferior RPC per process can be ongoing.)
     void (*callbackFunc)(process *, void *userData, unsigned result);
     void *userData;
     
     void *savedRegs; // crucial!

     bool wasRunning; // were we running when we launched the inferiorRPC?

     unsigned firstInstrAddr; // start location of temp tramp

     unsigned stopForResultAddr;
        // location of the TRAP or ILL which marks point where paradynd should grab the
	// result register.  Undefined if no callback fn.
     unsigned justAfter_stopForResultAddr; // undefined if no callback fn.
     reg resultRegister; // undefined if no callback fn.

     unsigned resultValue; // undefined until we stop-for-result, at which time we
                           // fill this in.  The callback fn (which takes in this value)
			   // isn't invoked until breakAddr (the final break)

     unsigned breakAddr;
        // location of the TRAP or ILL insn which marks the end of the inferiorRPC
  };
  vectorSet<inferiorRPCinProgress> currRunningRPCs;
      // see para above for reason why this 'vector' can have at most 1 elem!

 public:
  unsigned numOfActCounters_is;
  unsigned numOfActProcTimers_is;
  unsigned numOfActWallTimers_is; 
  bool deferredContinueProc;
  void updateActiveCT(bool flag, CTelementType type);
  void cleanRPCreadyToLaunch(int mid);
  void postRPCtoDo(AstNode *, bool noCost,
		   void (*)(process *, void *, unsigned), void *, int);
  bool existsRPCreadyToLaunch() const;
  bool existsRPCinProgress() const;
  bool launchRPCifAppropriate(bool wasRunning, bool finishingSysCall);
     // returns true iff anything was launched.
     // asynchronously launches iff RPCsWaitingToStart.size() > 0 AND
     // if currRunningRPCs.size()==0 (the latter for safety)
     // If we're gonna launch, then we'll stop the process (a necessity).
     // Pass wasRunning as true iff you want the process  to continue after
     // receiving the TRAP signifying completion of the RPC.
  bool isRPCwaitingForSysCallToComplete() const {
     return RPCs_waiting_for_syscall_to_complete;
  }
  void setRPCwaitingForSysCallToComplete(bool flag) {
     RPCs_waiting_for_syscall_to_complete = flag;
  }

  bool handleTrapIfDueToRPC();
     // look for curr PC reg value in 'trapInstrAddr' of 'currRunningRPCs'.  Return
     // true iff found.  Also, if true is being returned, then additionally does
     // a 'launchRPCifAppropriate' to fire off the next waiting RPC, if any.
  bool changePC(unsigned addr);

 private:
  // The follwing 5 routines are implemented in an arch-specific .C file
  bool emitInferiorRPCheader(void *, unsigned &base);
  bool emitInferiorRPCtrailer(void *, unsigned &base,
                              unsigned &breakOffset,
			      bool stopForResult,
			      unsigned &stopForResultOffset,
			      unsigned &justAfter_stopForResultOffset);

  unsigned createRPCtempTramp(AstNode *action,
			      bool noCost, bool careAboutResult,
			      unsigned &breakAddr,
			      unsigned &stopForResultAddr,
			      unsigned &justAfter_stopForResultAddr,
			      reg &resultReg);

  void *getRegisters();
     // ptrace-GETREGS and ptrace-GETFPREGS (or /proc PIOCGREG and PIOCGFPREG).
     // Result is returned in an opaque type which is allocated with new[]

  bool changePC(unsigned addr,
                const void *savedRegs // returned by getRegisters()
                );

  bool executingSystemCall();

  bool restoreRegisters(void *buffer);
     // input is the opaque type returned by getRegisters()

  bool set_breakpoint_for_syscall_completion();
  unsigned read_inferiorRPC_result_register(reg);

 public:
  void installBootstrapInst();
  void installInstrRequests(const vector<instMapping*> &requests);
 
  // These member vrbles should be made private!
  int traceLink;		/* pipe to transfer traces data over */
  int ioLink;			/* pipe to transfer stdout/stderr over */
  processState status_;	        /* running, stopped, etc. */
  vector<pdThread *> threads;	/* threads belonging to this process */
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
  hashTable *threadMap;         /* mapping table for threads into superTable */
#endif
  bool continueAfterNextStop_;
  // on some platforms we use one heap for text and data so textHeapFree is not
  // used.
  bool splitHeaps;		/* are the inferior heap split I/D ? */
  inferiorHeap	heaps[2];	/* the heaps text and data */
  resource *rid;		/* handle to resource for this process */

  /* map an inst point to its base tramp */
  dictionary_hash<const instPoint*, trampTemplate *> baseMap;	

  // the following 3 are used in perfStream.C
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;

  time64 wallTimeLastTrampSample;
  time64 timeLastTrampSample;

  bool reachedFirstBreak; // should be renamed 'reachedInitialTRAP'

  int getPid() const { return pid;}

  bool heapIsOk(const vector<sym_data>&);
  bool initDyninstLib();

  void initInferiorHeap(bool textHeap);
     // true --> text heap, else data heap

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  bool isAddrInHeap(Address addr) {
	    if ((addr >= heaps[dataHeap].base &&
		 addr < heaps[dataHeap].base + heaps[dataHeap].size) ||
		(splitHeaps && addr >= heaps[textHeap].base &&
		 addr < heaps[textHeap].base + heaps[textHeap].size))
		return true;
	    else
		return false;
	};
#endif

  bool writeDataSpace(void *inTracedProcess,
		      int amount, const void *inSelf);
  bool readDataSpace(const void *inTracedProcess, int amount,
		     void *inSelf, bool displayErrMsg);

  bool writeTextSpace(void *inTracedProcess, int amount, const void *inSelf);
  bool writeTextWord(caddr_t inTracedProcess, int data);
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  bool readTextSpace(const void *inTracedProcess, int amount,
		     const void *inSelf);
#endif
  bool continueProc();
#ifdef BPATCH_LIBRARY
  bool terminateProc() { return terminateProc_(); }
#endif
  bool pause();

  bool replaceFunctionCall(const instPoint *point,const function_base *newFunc);

  bool dumpCore(const string coreFile);
  bool detach(const bool paused); // why the param?
  bool API_detach(const bool cont); // XXX Should eventually replace detach()
  bool attach();
  string getProcessStatus() const;

  static string tryToFindExecutable(const string &progpath, int pid);
      // os-specific implementation.  Returns empty string on failure.
      // Otherwise, returns a full-path-name for the file.  Tries every
      // trick to determine the full-path-name, even though "progpath" may
      // be unspecified (empty string)

  bool continueWithForwardSignal(int sig); // arch-specific implementation
  
  // forkProcess: this function should be called when a process we are tracing
  // forks a child process.
  // This function returns a new process object associated with the child.
  // It also writes to "map" s.t. for each instInstance in the parent, we have the
  // corresponding instInstance in the child.
  static process *forkProcess(const process *parent, pid_t childPid,
			      dictionary_hash<instInstance*,instInstance*> &map,
			      int iTrace_fd
#ifdef SHM_SAMPLING
                              ,key_t theKey,
                              void *applAttachedAtPtr
#endif
                              );

  // get and set info. specifying if this is a dynamic executable
  void setDynamicLinking(){ dynamiclinking = true;}
  bool isDynamicallyLinked() { return (dynamiclinking); }

  // handleIfDueToSharedObjectMapping: if a trap instruction was caused by
  // a dlopen or dlclose event then return true
  bool handleIfDueToSharedObjectMapping();

  // handleStartProcess: this function is called when an appplication 
  // starts executing.  It is used to insert instrumentation necessary
  // to handle dynamic linking
  static bool handleStartProcess(process *pid);

  bool handleStopDueToExecEntry();

  // getSharedObjects: This routine is called before main() to get and
  // process all shared objects that have been mapped into the process's
  // address space
  bool getSharedObjects();

  // addASharedObject: This routine is called whenever a new shared object
  // has been loaded by the run-time linker after the process starts running
  // It processes the image, creates new resources
  bool addASharedObject(shared_object &);

  // return the list of dynamically linked libs
  vector<shared_object *> *sharedObjects() { return shared_objects;  } 

  // getMainFunction: returns the main function for this process
  function_base *getMainFunction() const { return mainFunction; }

  // findOneFunction: returns the function associated with function "func"
  // and module "mod".  This routine checks both the a.out image and any
  // shared object images for this function
  function_base *findOneFunction(resource *func,resource *mod);

#ifndef BPATCH_LIBRARY
  // returns all the functions in the module "mod" that are not excluded by
  // exclude_lib or exclude_func
  // return 0 on error.
  vector<function_base *> *getIncludedFunctions(module *mod); 
#endif

  // findOneFunction: returns the function associated with function "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  function_base *findOneFunction(const string &func_name);

  // findOneFunctionFromAll: returns the function associated with function "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  function_base *findOneFunctionFromAll(const string &func_name);

  // findFunctionIn: returns the function which contains this address
  // This routine checks both the a.out image and any shared object images 
  // for this function
  function_base *findFunctionIn(Address adr);

  // findModule: returns the module associated with "mod_name" 
  // this routine checks both the a.out image and any shared object 
  // images for this module
  // if check_excluded is true it checks to see if the module is excluded
  // and if it is it returns 0.  If check_excluded is false it doesn't check
  module *findModule(const string &mod_name,bool check_excluded);

  // getSymbolInfo:  get symbol info of symbol associated with name n
  // this routine starts looking a.out for symbol and then in shared objects
  // baseAddr is set to the base address of the object containing the symbol
  bool getSymbolInfo(const string &n, Symbol &info, Address &baseAddr) const;

  // getAllFunctions: returns a vector of all functions defined in the
  // a.out and in the shared objects
  vector<function_base *> *getAllFunctions();

  // getAllModules: returns a vector of all modules defined in the
  // a.out and in the shared objects
  vector<module *> *getAllModules();

#ifndef BPATCH_LIBRARY
  // getIncludedFunctions: returns a vector of all functions defined in the
  // a.out and in shared objects that are not excluded by an mdl option 
  vector<function_base *> *getIncludedFunctions();
#endif

  // getIncludedModules: returns a vector of all functions defined in the
  // a.out and in shared objects that are  not excluded by an mdl option
  vector<module *> *getIncludedModules();

  // getBaseAddress: sets baseAddress to the base address of the 
  // image corresponding to which.  It returns true  if image is mapped
  // in processes address space, otherwise it returns 0
  bool getBaseAddress(const image *which, u_int &baseAddress) const;

  // findCallee: finds the function called by the instruction corresponding
  // to the instPoint "instr". If the function call has been bound to an
  // address, then the callee function is returned in "target" and the
  // instPoint "callee" data member is set to pt to callee's function_base.
  // If the function has not yet been bound, then "target" is set to the
  // function_base associated with the name of the target function (this is
  // obtained by the PLT and relocation entries in the image), and the instPoint
  // callee is not set.  If the callee function cannot be found, (ex. function
  // pointers, or other indirect calls), it returns false.
  // Returns false on error (ex. process doesn't contain this instPoint).
  bool findCallee(instPoint &instr, function_base *&target);

  // these routines are for testing, setting, and clearing the 
  // waiting_for_resources flag, if this flag is true a process is not 
  // started until all outstanding resourceInfoResponses have been received
  void setWaitingForResources(){ waiting_for_resources = true; }
  // called by perfStream.C on SIGSTOP if there are any
  // resource::num_outstanding_creates,
  // and process::handleStartProcess, also if there are any
  // resource::num_outstanding_creates.
  void clearWaitingForResources(){ waiting_for_resources = false; }
  bool isWaitingForResources(){ return(waiting_for_resources); }

  // findSignalHandler: if signal_handler is 0, then it checks all images
  // associtated with this process for the signal handler function.
  // Otherwise, the signal handler function has already been found
  void findSignalHandler();

  // continueProcessIfWaiting: if the waiting_for_resources flag
  // is set then continue the process; in any event, clear that flag
  void continueProcessIfWaiting(){
      if(waiting_for_resources){
          continueProc();
      }
      waiting_for_resources = false;
  }

  //  wasExeced: returns true is the process did an exec...this is set
  //  in handleExec()
  bool wasExeced(){ return execed_;}

  void handleExec();
  bool cleanUpInstrumentation(bool wasRunning); // called on exit (also exec?)
  bool inExec;

  string execFilePath;

  int getProcFileDescriptor(){ return proc_fd;}

  static int waitProcs(int *status);
  const process *getParent() const {return parent;}

#if defined(hppa1_1_hp_hpux)
  bool freeNotOK;
#endif

#ifdef SHM_SAMPLING
  key_t getShmKeyUsed() const {return inferiorHeapMgr.getShmKey();}
  bool doMajorShmSample(time64 currWallTime);
  bool doMinorShmSample();

  const fastInferiorHeapMgr &getShmHeapMgr() const {
     return(inferiorHeapMgr);
  }

  unsigned getShmHeapTotalNumBytes() {
     return inferiorHeapMgr.getHeapTotalNumBytes();
  }

  void *getObsCostLowAddrInApplicSpace() {
     void *result = inferiorHeapMgr.getObsCostAddrInApplicSpace();
     return result;
  }
  unsigned *getObsCostLowAddrInParadyndSpace() {
     unsigned *result = inferiorHeapMgr.getObsCostAddrInParadyndSpace();
     return result;
  }
  void processCost(unsigned obsCostLow,
                   time64 wallTime, time64 processTime);


#ifdef sparc_sun_sunos4_1_3
   static user *tryToMapChildUarea(int pid);
#endif

#endif /* shm_sampling */

   bool isBootstrappedYet() const {
      return hasBootstrapped;
   }
   bool extractBootstrapStruct(DYNINST_bootstrapStruct *);
   int procStopFromDYNINSTinit();
      // returns 0 if not processed, 1 for the usual processed case (process is
      // now paused), or 2 for the processed-but-still-running-inferiorRPC case

   void handleCompletionOfDYNINSTinit(bool fromAttach);
      // called by above routine.  Reads bs_record from applic, takes action.

   static void DYNINSTinitCompletionCallback(process *, void *, unsigned);
      // inferiorRPC callback routine.

private:
  // Since we don't define these, 'private' makes sure they're not used:
  process(const process &); // copy ctor
  process &operator=(const process &); // assign oper

  bool hasBootstrapped;
     // set to true when we get callback from inferiorRPC call to DYNINSTinit

  // the following two variables are used when libdyninstRT is dynamically linked
  // which currently is done only on the Windows NT platform.
  // On other platforms the values are undefined
  bool hasLoadedDyninstLib; // true iff dyninstlib has been loaded already
  bool isLoadingDyninstLib; // true iff we are currently loading dyninst lib
  
  // the next two variables are used when we are loading dyninstlib -- currently
  // Windows NT only
  // They are used by the special inferior RPC that makes the call to load the
  // library -- we use a special inferior RPC because the regular RPC assumes
  // that the inferior heap already exists, which is not true if libdyninstRT
  // has not been loaded yet.
  char savedData[LOAD_DYNINST_BUF_SIZE];
  void *savedRegs;


  const process *parent;	/* parent of this process */
  image *symbols;		/* information related to the process */
  int pid;			/* id of this process */

#ifdef SHM_SAMPLING
  time64 previous; // This is being used to avoid time going backwards in
                   // getInferiorProcessCPUtime. We can't use a static variable
                   // inside this procedure because there is one previous for
                   // each process - naim 5/28/97

  // New components of the conceptual "inferior heap"
  fastInferiorHeapMgr inferiorHeapMgr;

  superTable theSuperTable;

#ifdef sparc_sun_sunos4_1_3
  kvm_t *kvmHandle;
  user *childUareaPtr;
#endif

#endif

public:
#ifdef SHM_SAMPLING
  const superTable &getTable() const {
     return theSuperTable;
  }
  superTable &getTable() {
     return theSuperTable;
  }
#endif
  trampTableEntry trampTable[TRAMPTABLESZ];
  unsigned trampTableItems;

  unsigned currentPC() {
    int pc, fp;
    if (hasNewPC)
      return currentPC_;
    else if (getActiveFrame(&fp, &pc)) {
      currentPC_ = (unsigned) pc;
      return currentPC_;
    }
    else abort();
    return 0;
  }
  void setNewPC(unsigned newPC) {
    currentPC_ = newPC;
    hasNewPC = true;
  }

  inline int costAddr()  const { return costAddr_; }  // why an integer?
  void getObservedCostAddr();   

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
   bool uninstallMutations();
   bool reinstallMutations();
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

private:
  bool createdViaAttach;
     // set in the ctor.  True iff this process was created with an attach,
     // as opposed to being fired up by paradynd.  On fork, has the value of
     // the parent process.  On exec, no change.
     // This vrble is important because it tells us what to do when we exit.
     // If we created via attach, we should (presumably) detach but not fry
     // the application; otherwise, a kill(pid, 9) is called for.
  // the following 2 are defined only if 'createdViaAttach' is true; action is taken
  // on these vrbles once DYNINSTinit completes.

  bool wasRunningWhenAttached;
  bool needToContinueAfterDYNINSTinit;

  unsigned currentPC_;
  bool hasNewPC;

  // for processing observed cost (see method processCost())
  int64 cumObsCost; // in cycles
  unsigned lastObsCostLow; // in cycles

  int costAddr_;  // why an integer?
  bool execed_;  // true if this process does an exec...set in handleExec

  // deal with system differences for ptrace
  bool writeDataSpace_(void *inTracedProcess, int amount, const void *inSelf);
  bool readDataSpace_(const void *inTracedProcess, int amount, void *inSelf);

  bool writeTextWord_(caddr_t inTracedProcess, int data);
  bool writeTextSpace_(void *inTracedProcess, int amount, const void *inSelf);
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  bool readTextSpace_(void *inTracedProcess, int amount, const void *inSelf);
#endif
  bool pause_();
  bool continueProc_();
#ifdef BPATCH_LIBRARY
  bool terminateProc_();
#endif
  bool dumpCore_(const string coreFile);
  bool detach_();
#ifdef BPATCH_LIBRARY
  bool API_detach_(const bool cont); // XXX Should eventually replace detach_()
#endif
  bool attach_(); // low-level attach; called by attach() (formerly OS::osAttach())
  bool stop_(); // formerly OS::osStop

  // stops a process
  bool loopUntilStopped();

  // returns true iff ok to do a ptrace; false (and prints a warning) if not
  bool checkStatus();

  int proc_fd; // file descriptor for platforms that use /proc file system.

  dynamic_linking *dyn;   // platform specific dynamic linking routines & data

  bool dynamiclinking;   // if true this a.out has a .dynamic section
  vector<shared_object *> *shared_objects;  // list of dynamically linked libs

  // The set of all functions and modules from the shared objects that show
  // up on the Where axis (both  instrumentable and uninstrumentable due to 
  // exclude_lib or exclude_func),  and all the functions from the a.out that
  // are between DYNINSTStart and DYNINSTend
  // TODO: these lists for a.out functions and modules should be handled the 
  // same way as for shared object functions and modules
  vector<function_base *> *all_functions;
  vector<module *> *all_modules;

  // these are a restricted set of functions and modules which are those  
  // from the a.out and shared objects that are instrumentable (not excluded 
  // through the mdl "exclude_lib" or "exclude_func" option) 
  // "excluded" now means never able to instrument
  vector<module *> *some_modules;  
  vector<function_base *> *some_functions; 
  bool waiting_for_resources;  // true if waiting for resourceInfoResponse
  pd_Function *signal_handler;  // signal handler function (for stack walking)

  function_base *mainFunction;  // the main function for this process,
                              // this is usually, but not always, main
                                 

  // needToAddALeafFrame: returns true if the between the current frame 
  // and the next frame there is a leaf function (this occurs when the 
  // current frame is the signal handler)
  bool needToAddALeafFrame(Frame current_frame, Address &leaf_pc);

  // hasBeenBound: returns true if the runtime linker has bound the
  // function symbol corresponding to the relocation entry in at the address 
  // specified by entry and base_addr.  If it has been bound, then the callee 
  // function is returned in "target_pdf", else it returns false. 
  bool hasBeenBound(const relocationEntry entry, pd_Function *&target_pdf, 
		    Address base_addr) ;

  // findpdFunctionIn: returns the function which contains this address
  // This routine checks both the a.out image and any shared object images 
  // for this function
  pd_Function *findpdFunctionIn(Address adr);

  bool isRunning_() const;
     // needed to initialize the 'wasRunningWhenAttached' member vrble.  Determines
     // whether the process is running by doing a low-level OS check, not by checking
     // member vrbles like status_.  May assume that process::attach() has already run,
     // but can't assume anything else.  May barf with a not-yet-implemented error on a
     // given platform if the platform doesn't yet support attaching to a running process.
     // But there's no reason why we shouldn't be able to implement this on any platform;
     // after all, the output from the "ps" command can be used (T --> return false,
     // S or R --> return true, other --> return ?)

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
   mutationList	beforeMutationList, afterMutationList;

   bool saveOriginalInstructions(Address addr, int size);
   bool writeMutationList(mutationList &list);
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
   // some very useful items gathered from /proc (initialized in attach() [solaris.C],
   // as soon as /proc fd is opened)
   string argv0; // argv[0] of program, at the time it started up
   string pathenv; // path env var of program, at the time it started up
   string cwdenv; // curr working directory of program, at the time it started up

//   string fullPathToExecutable_;
      // very useful, especially on attach (need this to parse the symbol table)

 public:
   const string &getArgv0() const {return argv0;}
   const string &getPathEnv() const {return pathenv;}
   const string &getCwdEnv() const {return cwdenv;}
#endif
};

process *createProcess(const string file, vector<string> argv, vector<string> envp, const string dir);
bool attachProcess(const string &progpath, int pid, int afterAttach
#ifdef BPATCH_LIBRARY
		   , process *&newProcess
#endif
		  );

void handleProcessExit(process *p, int exitStatus);

unsigned inferiorMalloc(process *proc, int size, inferiorHeapType type);
void inferiorFree(process *proc, unsigned pointer, inferiorHeapType type,
                  const vector<unsigVecType> &pointsToCheck);

extern resource *machineResource;

class Frame {
  private:
    int frame_;
    int pc_;
    bool uppermostFrame;

  public:
    Frame(process *);
       // formerly getActiveStackFrameInfo

    Frame(int theFrame, int thePc, bool theUppermost) {
       frame_ = theFrame; pc_ = thePc;
       uppermostFrame = theUppermost;
    }

    int getPC() const { return pc_; }
    int getFramePtr(){ return frame_;}
    bool isLastFrame() const { 
	if ((pc_ == 0)||(frame_ == 0))
	    return(true);
	else 
	    return(false); 
    }

    Frame getPreviousStackFrameInfo(process *proc) const;
};

extern vector<process *> processVec;

#endif
