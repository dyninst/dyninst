/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * process.h - interface to manage a process in execution. A process is a kernel
 *   visible unit with a seperate code and data space.  It might not be
 *   the only unit running the code, but it is only one changed when
 *   ptrace updates are applied to the text space.
 *
 * $Log: process.h,v $
 * Revision 1.38  1996/07/09 04:10:32  lzheng
 * add variable freeNotOK for the stack walking on hpux
 *
 * Revision 1.37  1996/05/31 23:59:22  tamches
 * inferiorHeap now uses addrHash16 instead of addrHash
 *
 * Revision 1.36  1996/05/17 16:49:34  mjrg
 * added a test to continueProc to catch a case where we might try to continue
 * an exited process.
 *
 * Revision 1.35  1996/05/11 23:14:54  tamches
 * inferiorHeap uses addrHash instead of uiHash; performs better.
 *
 * Revision 1.34  1996/05/10 06:54:16  tamches
 * disabledItem is now a class w/ its data private; added
 * proper operator= and a constructor
 * inferiorFree's last 2 args are now references; saves needless
 * calls to vector ctor --> new/delete
 *
 * Revision 1.33  1996/05/08 23:55:05  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.32  1996/04/18 22:06:10  naim
 * Adding parameters that control and delay (when necessary) the deletion
 * of instrumentation. Also, some minor misspelling fixes - naim
 *
 * Revision 1.31  1996/04/06  21:25:31  hollings
 * Fixed inst free to work on AIX (really any platform with split I/D heaps).
 * Removed the Line class.
 * Removed a debugging printf for multiple function returns.
 *
 * Revision 1.30  1996/04/03  14:27:54  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.29  1996/03/12  20:48:37  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.28  1996/03/01 22:37:20  mjrg
 * Added a type to resources.
 * Added function handleProcessExit to handle exiting processes.
 *
 * Revision 1.27  1996/02/13 06:17:36  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.26  1995/12/15  14:40:58  naim
 * Changing "hybrid_cost" by "smooth_obs_cost" - naim
 *
 * Revision 1.25  1995/11/29  18:45:24  krisna
 * added inlines for compiler. added templates
 *
 * Revision 1.24  1995/10/26 21:07:09  tamches
 * corrected constructor for heapItem(), which had been mysteriously
 * named heap() by mistake
 *
 * Revision 1.23  1995/10/19 22:36:46  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
 *
 * Revision 1.22  1995/09/18  22:41:38  mjrg
 * added directory command.
 *
 * Revision 1.21  1995/09/05  23:11:36  mjrg
 * Initialize splitHeaps.
 *
 * Revision 1.20  1995/08/29  21:47:24  mjrg
 * added third argument to declaration of initInferiorHeap.
 *
 * Revision 1.19  1995/08/24  15:04:31  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.18  1995/08/05  17:16:47  krisna
 * (const T *) vs (T * const)
 *
 * Revision 1.17  1995/05/18  10:41:11  markc
 * Changed process dict to process map
 *
 * Revision 1.16  1995/02/16  08:54:04  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.15  1995/02/16  08:34:37  markc
 * Changed igen interfaces to use strings/vectors rather than charigen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.14  1994/11/10  18:58:17  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.13  1994/11/09  18:40:35  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.12  1994/11/02  11:15:38  markc
 * Added prototypes.
 *
 * Revision 1.11  1994/09/22  02:23:44  markc
 * Changed structs to classes
 *
 * Revision 1.10  1994/08/17  18:18:07  markc
 * Added reachedFirstBreak variable.
 *
 * Revision 1.9  1994/07/26  20:02:08  hollings
 * fixed heap allocation to use hash tables.
 *
 * Revision 1.8  1994/07/22  19:20:40  hollings
 * added pauseTime and wallTime.
 *
 * Revision 1.7  1994/07/20  23:23:40  hollings
 * added insn generated metric.
 *
 * Revision 1.6  1994/07/14  23:30:32  hollings
 * Hybrid cost model added.
 *
 * Revision 1.5  1994/07/12  19:43:18  jcargill
 * Changed order of processState defn, so that initial (==0) state is neonatal.
 * Otherwise there is a small time-window at startup when it looks like it's
 * running, but hasn't been initialized.
 *
 * Revision 1.4  1994/05/18  00:52:32  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.3  1994/03/31  01:58:33  markc
 * Extended arguments to createProcess.
 *
 * Revision 1.2  1994/03/20  01:53:11  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.1  1994/01/27  20:31:39  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.6  1993/08/11  01:46:24  hollings
 * added defs for baseMap.
 *
 * Revision 1.5  1993/07/13  18:29:49  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/25  22:23:28  hollings
 * added parent field to process.h
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
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

extern unsigned activeProcesses; // number of active processes

class resource;
class instPoint;
class instInstance;
class baseTrampoline;

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
    stopAtFirstBreak = false;
    splitHeaps = false;
    waitingForNodeDaemon = false;
    inExec = false;
    proc_fd = -1;
  }

  static string programName;
  static vector<string> arg_list;

  vector<Address> walkStack();
  //
  // getActiveFrame and readDataFromFrame are platform dependant
  //
  bool getActiveFrame(int *fp, int *pc);
  bool readDataFromFrame(int currentFP, int *previousFP, int *rtn);

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
  dictionary_hash<instPoint*, unsigned> baseMap;	/* map and inst point to its base tramp */
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;
  time64 wallTimeLastTrampSample;
  time64 timeLastTrampSample;
  float pauseTime;		/* only used on the CM-5 version for now 
				   jkh 7/21/94 */
  int reachedFirstBreak;
  bool stopAtFirstBreak;      // true if the process must stop when it reaches the
                              // ptrace trap at the start of the program.

  bool waitingForNodeDaemon;  // CM5 process only: if true, this process is stopped,
			      // waiting for a node daemon (paradyndCM5) to start.

  time64 getFirstRecordTime() const { return firstRecordTime;}
  void setFirstRecordTime(time64 to) { firstRecordTime = to;}
  int getPid() const { return pid;}

  inline bool writeDataSpace(caddr_t inTracedProcess,
			     int amount, caddr_t inSelf);
  inline bool readDataSpace(caddr_t inTracedProcess, int amount,
			    caddr_t inSelf, bool displayErrMsg);
  inline bool writeTextSpace(caddr_t inTracedProcess, int amount, caddr_t inSelf);
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

  void handleExec();
  bool inExec;
  string execFilePath;

  static int waitProcs(int *status);

#if defined(hppa1_1_hp_hpux)
  bool freeNotOK;
#endif

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
  if (waitingForNodeDaemon) return true; // CM5 kludge

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
  if (waitingForNodeDaemon) return true; // CM5 kludge

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

/*
 * Copy data from controller process to the named process.
 */
inline bool process::writeDataSpace(caddr_t inTracedProcess, int size, caddr_t inSelf) {
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
    string msg = string("System error: unable to write to process data space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;
}

inline bool process::readDataSpace(caddr_t inTracedProcess, int size, caddr_t inSelf, bool displayErrMsg) {
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
      msg=string("System error: unable to read from process data space:")
          + string(sys_errlist[errno]);
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
    string msg = string("System error: unable to write to process data space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;

}

inline bool process::writeTextSpace(caddr_t inTracedProcess, int amount, caddr_t inSelf) {
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
    string msg = string("System error: unable to write to process data space:")
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
    }
    Frame getPreviousStackFrameInfo(process *proc)
    {
      int fp;
      int rtn;
      Frame frame;

      if (frame_ != 0) {
        if (proc->readDataFromFrame(frame_, &fp, &rtn))
        {
          frame.frame_ = fp;
          frame.pc_ = rtn;
        }
      }
      return(frame);
    }
    int getPC() { return pc_; }
    bool isLastFrame() { if (frame_ == 0) return(true);
                         else return(false); }
  private:
    int frame_;
    int pc_;
};

#endif

