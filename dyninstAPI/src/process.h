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
 * $Log: process.h,v $
 * Revision 1.43  1996/09/13 21:54:01  mjrg
 * Changed code to allow base tramps of variable size.
 *
 * Revision 1.42  1996/08/20 21:34:54  lzheng
 * Minor fix for the procedure readDataFromFrame
 *
 * Revision 1.41  1996/08/20 19:19:39  lzheng
 * Implementation of moving multiple instructions sequence and
 * splitting the instrumentation into two phases
 *
 * Revision 1.40  1996/08/16 21:19:41  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.39  1996/08/12 16:32:43  mjrg
 * Code cleanup: removed cm5 kludges and unused code
 *
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
  void cleanUpInstrumentation(); //XXXX 
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

