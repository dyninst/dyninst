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
 * Revision 1.15  1995/02/16 08:34:37  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
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
#include "os.h"
#include <stdio.h>

class resource;
class instPoint;

// TODO a kludge - to prevent recursive includes
class image;

typedef enum { neonatal, running, stopped, exited } processState;
typedef enum { HEAPfree, HEAPallocated } heapStatus;

class heapItem {
 public:
  heap() {
    addr =0; length = 0; status = HEAPfree;
  }
  Address addr;
  int length;
  heapStatus status;
}; 

#define HIST_LIMIT      6

class costC {
 public:
    costC() {
      currentHist=0; lastObservedCost = 0.0; totalPredictedCost=0.0;
      currentPredictedCost=0.0; timeLastTrampSample = 0; hybrid = 0.0;
      wallTimeLastTrampSample = 0;
      int i;
      for (i=0; i<HIST_LIMIT; ++i)
	past[i] = 0.0;
    }
    float hybrid;
    int currentHist;
    float lastObservedCost;
    float past[HIST_LIMIT];
    float totalPredictedCost;
    float currentPredictedCost;
    time64 timeLastTrampSample;
    time64 wallTimeLastTrampSample;
};

inline unsigned ipHash(const instPoint *&ip) {
  return ((unsigned)ip);
}

class process {
public:
friend class ptraceKludge;

  process() : heapActive(uiHash), baseMap(ipHash), firstRecordTime(0) {
    symbols = NULL; traceLink = 0; ioLink = 0;
    status_ = neonatal; pid = 0; thread = 0;
    aggregate = false; rid = 0; parent = NULL;
    bufStart = 0; bufEnd = 0; pauseTime = 0.0;
    freed = 0; reachedFirstBreak = 0; 
  }

  static string programName;
  static vector<string> arg_list;

  processState status() const { return status_;}

  image *symbols;		/* information related to the process */
  int traceLink;		/* pipe to transfer traces data over */
  int ioLink;			/* pipe to transfer stdout/stderr over */
  processState status_;	/* running, stopped, etc. */
  int pid;			/* id of this process */
  int thread;			/* thread id for thread */
  bool aggregate;		/* is this process a pseudo process ??? */
  dictionary_hash<unsigned, heapItem*> heapActive; // active part of inferior heap 
  vector<heapItem*> heapFree;  /* free block of inferrior heap */
  resource *rid;		/* handle to resource for this process */
  process *parent;		/* parent of this proces */
  dictionary_hash<instPoint*, unsigned> baseMap;	/* map and inst point to its base tramp */
  char buffer[2048];
  int bufStart;
  int bufEnd;
  costC theCost;
  float pauseTime;		/* only used on the CM-5 version for now 
				   jkh 7/21/94 */
  int freed;
  int reachedFirstBreak;

  time64 getFirstRecordTime() const { return firstRecordTime;}
  void setFirstRecordTime(time64 to) { firstRecordTime = to;}
  int getPid() const { return pid;}

  inline bool writeDataSpace(caddr_t inTracedProcess,
			     int amount, caddr_t inSelf);
  inline bool readDataSpace(caddr_t inTracedProcess, int amount,
			    caddr_t inSelf);
  inline bool writeTextSpace(caddr_t inTracedProcess, int amount, caddr_t inSelf);
  inline bool writeTextWord(caddr_t inTracedProcess, int data);
  inline bool continueProc();
  inline bool pause();
  inline bool dumpCore(const string coreFile);
  inline bool detach(const bool paused);
  string getProcessStatus() const;

  // this is required because of the ugly implementation of PCptrace for the cm5
  // when that is cleaned up, this will go away
  void kludgeStatus(const processState stat) { status_ = stat;}

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
};

extern dictionary_hash<int, process*> processMap;

bool process::detach(const bool paused) {
  bool res;
  assert (res = detach_());
  // PCptrace(PTRACE_DETACH, proc, (char*) 1, SIGCONT, NULL);
  if (paused) {
    OS::osStop(pid);
    sprintf(errorLine, "detaching process %d leaving it paused\n", pid);
    logLine(errorLine);
  }
  return res;
}

bool process::checkStatus() {
  if (status_ == exited) {
    sprintf(errorLine, "attempt to ptrace exited process %d\n", pid);
    logLine(errorLine);
    return(false);
  } else
    return true;
}

bool process::pause() {
  if (status_ == running && reachedFirstBreak) {
    bool res;
    assert (res = pause_());
    // (void) PCptrace(PTRACE_INTERRUPT, proc, (char*)1, 0, (char*)NULL);
    status_ = stopped;
    return res;
  }
  return true;
}

bool process::continueProc() {
  if (status_ == stopped) {
    bool res;
    assert (res = continueProc_());
    // (void) PCptrace(PTRACE_CONT, proc, (char*)1, 0, (char*)NULL);
    status_ = running;
    return res;
  }
  return true;
}

bool process::dumpCore(const string fileOut) {
  bool res;
  assert (res = dumpCore_(fileOut));
  return res;
}

/*
 * Copy data from controller process to the named process.
 */
bool process::writeDataSpace(caddr_t inTracedProcess, int size, caddr_t inSelf) {
  bool res;
  assert (res = writeDataSpace_(inTracedProcess, size, inSelf));
  return res;
}

bool process::readDataSpace(caddr_t inTracedProcess, int size, caddr_t inSelf) {
  bool res;
  assert (res = readDataSpace_(inTracedProcess, size, inSelf));
  return res;
}

bool process::writeTextWord(caddr_t inTracedProcess, int data) {
  bool res;
  assert (res = writeTextWord_(inTracedProcess, data));
  return res;
}

bool process::writeTextSpace(caddr_t inTracedProcess, int amount, caddr_t inSelf) {
  bool res;
  assert (res = writeTextSpace_(inTracedProcess, amount, inSelf));
  return res;
}


process *createProcess(const string file, vector<string> argv, vector<string> envp);
process *allocateProcess(int pid, const string name);

void initInferiorHeap(process *proc, bool globalHeap);
void copyInferiorHeap(process *from, process *to);

unsigned inferiorMalloc(process *proc, int size);
void inferiorFree(process *proc, unsigned pointer);

extern resource *machineResource;

#endif
