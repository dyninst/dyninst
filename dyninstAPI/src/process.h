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

#include "rtinst/h/rtinst.h"
#include "util.h"
#include "util/h/String.h"
#include "util/h/Dictionary.h"
#include "util/h/Types.h"
#include "util/h/Timer.h"
#include "os.h"
#include "main.h"
#include "dyninstRPC.xdr.h"
#include <stdio.h>
#include "showerror.h"
#include "sharedobject.h"
#include "dynamiclinking.h"

extern unsigned activeProcesses; // number of active processes

class resource;
class instPoint;
class instInstance;
class trampTemplate;

// TODO a kludge - to prevent recursive includes
class image;

typedef enum { neonatal, running, stopped, exited } processState;
typedef enum { HEAPfree, HEAPallocated } heapStatus;
typedef enum { textHeap=0, dataHeap=1 } inferiorHeapType;
typedef vector<unsigned> unsigVecType;

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
  }
  inferiorHeap(const inferiorHeap &src);  // create a new heap that is a copy of src.
                                          // used on fork.
  dictionary_hash<unsigned, heapItem*> heapActive; // active part of heap 
  vector<heapItem*> heapFree;  		// free block of data inferior heap 
  vector<disabledItem> disabledList;	// items waiting to be freed.
  int disabledListTotalMem;		// total size of item waiting to free
  int totalFreeMemAvailable;		// total free memory in the heap
  int freed;				// total reclaimed (over time)
};

//
// read a C/C++ book to find out the difference
// between:
//
// (const T *) and (T * const)
//
static inline unsigned ipHash(instPoint * const &ip) {
  return ((unsigned)ip);
}

static inline unsigned instInstanceHash(instInstance * const &inst) {
  return ((unsigned)inst);
}

class process {
public:
friend class ptraceKludge;

  process() : baseMap(ipHash), 
              instInstanceMapping(instInstanceHash),
              firstRecordTime(0) {
    symbols = NULL; traceLink = 0; ioLink = 0;
    status_ = neonatal; pid = 0; thread = 0;
    aggregate = false; rid = 0; parent = NULL;
    bufStart = 0; bufEnd = 0; pauseTime = 0.0;
    reachedFirstBreak = 0; 
    splitHeaps = false;
    inExec = false;
    proc_fd = -1;

    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
    currentPC_ = 0;
    hasNewPC = false;

    inhandlestart = false;
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
  }

  static string programName;
  static vector<string> arg_list;

  vector<Address> walkStack();
  //
  // getActiveFrame and readDataFromFrame are platform dependant
  //
  bool getActiveFrame(int *fp, int *pc);
  bool readDataFromFrame(int currentFP, int *previousFP, int *rtn, bool uppermost=false);

  processState status() const { return status_;}
  inline void Exited();
  inline void Stopped();

  image *symbols;		/* information related to the process */
  int traceLink;		/* pipe to transfer traces data over */
  int ioLink;			/* pipe to transfer stdout/stderr over */
  processState status_;	/* running, stopped, etc. */
  int pid;			/* id of this process */
  int thread;			/* thread id for thread */
  bool aggregate;		/* is this process a pseudo process ??? */
  // on some platforms we use one heap for text and data so textHeapFree is not
  // used.
  bool splitHeaps;		/* are the inferior heap split I/D ? */
  inferiorHeap	heaps[2];	// the heaps text and data
  resource *rid;		/* handle to resource for this process */
  process *parent;		/* parent of this proces */
  dictionary_hash<instPoint*, trampTemplate *> baseMap;	/* map and inst point to its base tramp */
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;
  time64 wallTimeLastTrampSample;
  time64 timeLastTrampSample;
  float pauseTime;		/* only used on the CM-5 version for now 
				   jkh 7/21/94 */
  int reachedFirstBreak;


  time64 getFirstRecordTime() const { return firstRecordTime;}
  void setFirstRecordTime(time64 to) { firstRecordTime = to;}
  int getPid() const { return pid;}

  inline bool writeDataSpace(caddr_t inTracedProcess,int amount,caddr_t inSelf);
  inline bool readDataSpace(caddr_t inTracedProcess, int amount,
			     caddr_t inSelf, bool displayErrMsg);
  inline bool writeTextSpace(caddr_t inTracedProcess,int amount,caddr_t inSelf);
  inline bool writeTextWord(caddr_t inTracedProcess, int data);
  inline bool continueProc();
  inline bool pause();
  inline bool dumpCore(const string coreFile);
  inline bool detach(const bool paused);
  bool attach();
  string getProcessStatus() const;
  
  // this is required because of the ugly implementation of PCptrace for the cm5
  // when that is cleaned up, this will go away
  void kludgeStatus(const processState stat) { status_ = stat;}

  // instInstanceMapping is used when a process we are tracing forks a child 
  // process.
  // The child will have a copy of all instrumentation in the parent.
  // instInstanceMapping is a mapping of each instInstance of the parent to the
  // corresponding instInstance of the child.
  dictionary_hash<instInstance *, instInstance *>instInstanceMapping;

  // forkProcess: this function should be called when a process we are tracing
  // forks a child process.
  // This function returns a new process object associated with the child.
  static process *forkProcess(process *parent, pid_t childPid);

  // get and set info. specifying if this is a dynamic executable
  void setDynamicLinking(){ dynamiclinking = true;}
  bool isDynamicallyLinked() { return (dynamiclinking); }

  bool isInHandleStart() { return (inhandlestart); }
  void setInHandleStart() { inhandlestart = true; }
  void clearInHandleStart() { inhandlestart = false; }

  // handleStartProcess: this function is called when an appplication 
  // starts executing.  It is used to insert instrumentation necessary
  // to handle dynamic linking
  static bool handleStartProcess(process *pid);

  // findDynamicLinkingInfo: This routine is called on exit point of 
  // of the exec system call. It checks if the a.out is dynamically linked,
  // and if so, it inserts any initial instrumentation that is necessary
  // for collecting run-time linking info.
  bool findDynamicLinkingInfo();

  // getSharedObjects: This routine is called before main() to get and
  // process all shared objects that have been mapped into the process's
  // address space
  bool getSharedObjects();

  // addASharedObject: This routine is called whenever a new shared object
  // has been loaded by the run-time linker after the process starts running
  // It processes the image, creates new resources
  bool addASharedObject(shared_object &);

  // findOneFunction: returns the function associated with function "func"
  // and module "mod".  This routine checks both the a.out image and any
  // shared object images for this function
  pdFunction *findOneFunction(resource *func,resource *mod);

  // findOneFunction: returns the function associated with function "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  pdFunction *findOneFunction(const string &func_name);

  // findModule: returns the module associated with "mod_name" 
  // this routine checks both the a.out image and any shared object 
  // images for this module
  module *findModule(const string &mod_name);

  // getSymbolInfo:  get symbol info of symbol associated with name n
  // this routine starts looking a.out for symbol and then in shared objects
  bool getSymbolInfo(string &n, Symbol &info);

  // getAllFunctions: returns a vector of all functions defined in the
  // a.out and in the shared objects
  vector<pdFunction *> *getAllFunctions();

  // getAllModules: returns a vector of all modules defined in the
  // a.out and in the shared objects
  vector<module *> *getAllModules();

  // getIncludedFunctions: returns a vector of all functions defined in the
  // a.out and in shared objects that are not excluded by an mdl option 
  vector<pdFunction *> *getIncludedFunctions();

  // getIncludedModules: returns a vector of all functions defined in the
  // a.out and in shared objects that are  not excluded by an mdl option
  vector<module *> *getIncludedModules();

  // getBaseAddress: sets baseAddress to the base address of the 
  // image corresponding to which.  It returns true  if image is mapped
  // in processes address space, otherwise it returns 0
  bool getBaseAddress(const image *which, u_int &baseAddress);

  // these routines are for testing, setting, and clearing the 
  // waiting_for_resources flag, if this flag is true a process is not 
  // started until all outstanding resourceInfoResponses have been received
  void setWaitingForResources(){ waiting_for_resources = true; }
  void clearWaitingForResources(){ waiting_for_resources = false; }
  bool isWaitingForResources(){ return(waiting_for_resources); }

  // continueProcessIfWaiting: if the waiting_for_resources flag
  // is set then continue the process
  void continueProcessIfWaiting(){
      if(waiting_for_resources){
          continueProc();
      }
      waiting_for_resources = false;
  }

  void handleExec();
  void cleanUpInstrumentation(); //XXXX 
  bool inExec;
  string execFilePath;

  static int waitProcs(int *status);

#if defined(hppa1_1_hp_hpux)
  bool freeNotOK;
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

private:
  unsigned currentPC_;
  bool hasNewPC;


private:

  time64 firstRecordTime;

  // deal with system differences for ptrace
  bool writeDataSpace_(caddr_t inTracedProcess, int amount, caddr_t inSelf);
  bool readDataSpace_(caddr_t inTracedProcess, int amount, caddr_t inSelf);
  bool writeTextWord_(caddr_t inTracedProcess, int data);
  bool writeTextSpace_(caddr_t inTracedProcess, int amount, caddr_t inSelf);
  bool pause_();
  bool continueProc_();
  bool dumpCore_(const string coreFile);
  bool detach_();

  // stops a process
  bool loopUntilStopped();

  // is it ok to attempt a ptrace operation
  inline bool checkStatus();

  int proc_fd; // file descriptor for platforms that use /proc file system.

  dynamic_linking *dyn;   // platform specific dynamic linking routines & data
  bool inhandlestart;     // true if the executable is dynamic & initial 
			  // libraries have not yet been processed 
  bool dynamiclinking;   // if true this a.out has a .dynamic section
  vector<shared_object *> *shared_objects;  // list of dynamically linked libs

  // The set of all functions and modules in the a.out and in the shared objects
  vector<pdFunction *> *all_functions;
  vector<module *> *all_modules;
  // these are a restricted set of functions and modules which are those  
  // from shared objects that are not excluded through an mdl "exclude_library"
  // option: these are used to satisfy foci that are not refined on the
  // Code heirarchy
  vector<module *> *some_modules;  
  vector<pdFunction *> *some_functions; 
  bool waiting_for_resources;  // true if waiting for resourceInfoResponse
};

extern vector<process*> processVec;
inline process *findProcess(int pid) {
  unsigned size=processVec.size();
  for (unsigned u=0; u<size; u++)
    if (processVec[u]->getPid() == pid)
      return processVec[u];
  return NULL;
}

inline bool process::detach(const bool paused) {
  if (paused) {
    logLine("detach: pause not implemented\n");
  }
  bool res = detach_();
  if (!res) {
    // process may have exited
    return false;
  }
  return true;
}

inline bool process::checkStatus() {
  if (status_ == exited) {
    sprintf(errorLine, "attempt to ptrace exited process %d\n", pid);
    logLine(errorLine);
    return(false);
  } else
    return true;
}

inline bool process::pause() {
  if (status_ == stopped || status_ == neonatal)
    return true;
  else if (status_ == running && reachedFirstBreak) {
    bool res = pause_();
    if (!res) {
      return false;
    }
    status_ = stopped;
  } else if (status_ == exited)
    return false;
  return true;
}

inline bool process::continueProc() {
  if (status_ == exited) return false;

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::continueProc");
    return false;
  }

  bool res = continueProc_();
  if (!res) {
    showErrorCallback(38, "System error: can't continue process");
    return false;
  }
  status_ = running;
  return true;
}

inline bool process::dumpCore(const string fileOut) {
  bool res = dumpCore_(fileOut);
  if (!res) {
    return false;
  }
  return true;
}

// getBaseAddress: sets baseAddress to the base address of the 
// image corresponding to which.  It returns true  if image is mapped
// in processes address space, otherwise it returns 0
inline bool process::getBaseAddress(const image *which,u_int &baseAddress){

  if((u_int)(symbols) == (u_int)(which)){
      baseAddress = 0; 
      return true;
  }
  else if (shared_objects) {  
      // find shared object corr. to this image and compute correct address
      for(u_int i=0; i <  shared_objects->size(); i++){ 
	  if(((*shared_objects)[i])->isMapped()){
            if(((*shared_objects)[i])->getImageId() == (u_int)which) { 
	      baseAddress = ((*shared_objects)[i])->getBaseAddress();
	      return true;
	  } }
      }
  }
  return false;
}

/*
 * Copy data from controller process to the named process.
 */
inline bool process::writeDataSpace(caddr_t inTracedProcess, int size,
				    caddr_t inSelf) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::writeDataSpace");
    return false;
  }

  bool res = writeDataSpace_(inTracedProcess, size, inSelf); 
  if (!res) {
    string msg = string("System error1: unable to write to process data space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;
}

inline bool process::readDataSpace(caddr_t inTracedProcess, int size, 
				   caddr_t inSelf, bool displayErrMsg) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::readDataSpace");
    return false;
  }

  bool res = readDataSpace_(inTracedProcess, size, inSelf); 
  if (!res) {
    if (displayErrMsg) {
      string msg;
      msg=string("System error2: unable to read from process data space:")
          + string(sys_errlist[errno]);
      msg += string(" address ");
      msg += string((u_int)inTracedProcess);
      msg += string(" size ");
      msg += string((u_int)size);
      showErrorCallback(38, msg);
    }
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;

}

inline bool process::writeTextWord(caddr_t inTracedProcess, int data) {

  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    string msg = string("Internal paradynd error in process::writeTextWord")
               + string((int)status_);
    showErrorCallback(38, msg);
    //showErrorCallback(38, "Internal paradynd error in process::writeTextWord");
    return false;
  }

  bool res = writeTextWord_(inTracedProcess, data); 
  if (!res) {
    string msg = string("System error3: unable to write to process data space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;

}

inline bool process::writeTextSpace(caddr_t inTracedProcess, int amount, 
				    caddr_t inSelf) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    string msg = string("Internal paradynd error in process::writeTextSpace")
               + string((int)status_);
    showErrorCallback(38, msg);
    //showErrorCallback(38, "Internal paradynd error in process::writeTextSpace");
    return false;
  }

  bool res = writeTextSpace_(inTracedProcess, amount, inSelf); 
  if (!res) {
    string msg = string("System error4: unable to write to process data space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;
}


process *createProcess(const string file, vector<string> argv, vector<string> envp, const string dir);
process *allocateProcess(int pid, const string name);
void handleProcessExit(process *p, int exitStatus);

void initInferiorHeap(process *proc, bool globalHeap, bool textHeap);
void copyInferiorHeap(process *from, process *to);

unsigned inferiorMalloc(process *proc, int size, inferiorHeapType type);
void inferiorFree(process *proc, unsigned pointer, inferiorHeapType type,
                  vector<unsigVecType> &pointsToCheck);

extern resource *machineResource;

/*
 *  The process has exited. Update its status and notify Paradyn.
 */
inline void process::Exited(void) {
  if (status_ != exited) {
    status_ = exited;
    tp->processStatus(pid, procExited);
  }
}

/*
 * The process was stopped by a signal. Update its status and notify Paradyn.
 */
inline void process::Stopped(void) {
  if (status_ != stopped) {
    status_ = stopped;
    tp->processStatus(pid, procPaused);
  }
}

class Frame {
  public:
    Frame() {
	frame_=0;
	pc_=0;
    }

    void getActiveStackFrameInfo(process *proc)
    {
	int fp, pc;
	if (proc->getActiveFrame(&fp, &pc)) {
	    frame_ = fp;
	    pc_ = pc;
	}
	else {
	    frame_ = 0;
	    pc_ = 0;
	}
	uppermostFrame = true;
    }

    Frame getPreviousStackFrameInfo(process *proc)
    {
	int fp = frame_;
	int rtn = pc_;
	Frame frame;
	
	if (frame_ != 0) {
	    if (proc->readDataFromFrame(frame_, &fp, &rtn, uppermostFrame))
	    {
		frame.frame_ = fp;
		frame.pc_ = rtn;
	    }
	    frame.uppermostFrame = false; 
	}
	return(frame);
    }

    int getPC() { return pc_; }
    bool isLastFrame() { 
	if (frame_ == 0) return(true);
	else return(false); 
    }
  private:
    int frame_;
    int pc_;
    bool uppermostFrame;
};

#endif

