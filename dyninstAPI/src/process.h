/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/* $Id: process.h,v 1.174 2002/01/25 00:05:46 schendel Exp $
 * process.h - interface to manage a process in execution. A process is a kernel
 *   visible unit with a seperate code and data space.  It might not be
 *   the only unit running the code, but it is only one changed when
 *   ptrace updates are applied to the text space.
 */

#ifndef PROCESS_H
#define PROCESS_H
#include <stdio.h>
#include <assert.h>
#ifdef BPATCH_LIBRARY
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#else
#include "rtinst/h/rtinst.h"
#include "paradynd/src/timeMgr.h"
#endif
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/util.h"
#include "common/h/String.h"
#include "common/h/vectorSet.h"
#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/os.h"
// #include "paradynd/src/main.h"
#include "dyninstAPI/src/showerror.h"
#if defined(MT_THREAD) && defined(rs6000_ibm_aix4_1)
#include <sys/pthdebug.h>
#endif

#include "dyninstAPI/src/symtab.h" // internalSym

#include "dyninstAPI/src/imageUpdate.h" //ccw 29 oct 2001

#ifdef SHM_SAMPLING
#include "paradynd/src/shmSegment.h"
#include "paradynd/src/fastInferiorHeapMgr.h"
#include "paradynd/src/superTable.h"
#include "paradynd/src/hashTable.h"
#ifdef sparc_sun_sunos4_1_3
#include <kvm.h>
#include <sys/user.h>
#endif
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif

#if defined(HRTIME) && !defined(BPATCH_LIBRARY)
#include "hrtime.h"
#endif

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"

#ifdef SHM_SAMPLING
/* these maxima selections are still rather arbitrary; can we do better? */
#ifdef MT_THREAD
static const unsigned maxNumMetrics=MAX_NUMBER_OF_THREADS*100;
/* Correct handling of per-thread metrics requires this to be a factor */
#else
static const unsigned maxNumMetrics=12800;
/* Currently set so that the size of shared-memory segments are close to, 
   but not exceeding, the default maximum size [shmmax] (typically 1MB). */
/* While this will be inefficient and constraining on most modern machines
   as yet we don't have the infrastructure for managing multiple segments
   nor dynamically determining the optimal size based on the current [shmmax].
   Expect shmget() failures accordingly! */
#endif

static const unsigned numIntCounters=maxNumMetrics;
static const unsigned numWallTimers =maxNumMetrics;
static const unsigned numProcTimers =maxNumMetrics;
#endif

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

#ifdef BPATCH_LIBRARY
class BPatch_thread;
class BPatch_function;
#endif

typedef enum { neonatal, running, stopped, exited } processState;
typedef enum { HEAPfree, HEAPallocated } heapStatus;
typedef enum { textHeap=0x01, dataHeap=0x02, anyHeap=0x33, lowmemHeap=0x40 }
        inferiorHeapType;
typedef vector<Address> addrVecType;

const int LOAD_DYNINST_BUF_SIZE = 256;

class heapItem {
 public:
  heapItem() : 
    addr(0), length(0), type(anyHeap), dynamic(true), status(HEAPfree) {}
  heapItem(Address a, int n, inferiorHeapType t, 
           bool d = true, heapStatus s = HEAPfree) :
    addr(a), length(n), type(t), dynamic(d), status(s) {}
  heapItem(const heapItem *h) :
    addr(h->addr), length(h->length), type(h->type), 
    dynamic(h->dynamic), status(h->status) {}
  heapItem(const heapItem &h) :
    addr(h.addr), length(h.length), type(h.type), 
    dynamic(h.dynamic), status(h.status) {}
  heapItem &operator=(const heapItem &src) {
    addr = src.addr;
    length = src.length;
    type = src.type;
    dynamic = src.dynamic;
    status = src.status;
    return *this;
  }

  Address addr;
  unsigned length;
  inferiorHeapType type;
  bool dynamic; // part of a dynamically allocated segment?
  heapStatus status;
};


// disabledItem: an item on the heap that we are trying to free.
// "pointsToCheck" corresponds to predecessor code blocks
// (i.e. prior minitramp/basetramp code)
class disabledItem {
 public:
  disabledItem() : block() {}

#if defined(USE_STL_VECTOR)

  disabledItem(heapItem *h, const vector<addrVecType> &preds);
  disabledItem(const disabledItem &src);
  
  disabledItem &operator=(const disabledItem &src) {
    if (&src == this) return *this; // check for x=x    
    block = src.block;
    
    // is this a mem leak???
    pointsToCheck.clear();
    for (unsigned int i = 0; i < src.pointsToCheck.size(); ++i) {
      pointsToCheck.push_back(src.pointsToCheck[i]);
    }
    return *this;
  }

#else
  disabledItem(heapItem *h, const vector<addrVecType> &preds) :
    block(h), pointsToCheck(preds) {}
  disabledItem(const disabledItem &src) :
    block(src.block), pointsToCheck(src.pointsToCheck) {}

  // TODO: unused?
  disabledItem(Address ip, inferiorHeapType iht,
               const vector<addrVecType> &ipts) :
    block(ip, 0, iht), pointsToCheck(ipts) { 
    fprintf(stderr, "error: unused disabledItem ctor\n");
    assert(0);
  }
  disabledItem &operator=(const disabledItem &src) {
    if (&src == this) return *this; // check for x=x    
    block = src.block;
    pointsToCheck = src.pointsToCheck;
    return *this;
  }
#endif

 ~disabledItem() {}
  
  heapItem block;                    // inferior heap block
  vector<addrVecType> pointsToCheck; // list of addresses to check against PCs

  Address getPointer() const {return block.addr;}
  inferiorHeapType getHeapType() const {return block.type;}
  const vector<addrVecType> &getPointsToCheck() const {return pointsToCheck;}
  vector<addrVecType> &getPointsToCheck() {return pointsToCheck;}
};

/* Dyninst heap class */
/*
  This needs a better name. Contains a name, and address, and a size.
  Any ideas?
*/
class heapDescriptor {
 public:
  heapDescriptor(const string name,
		 Address addr,
		 unsigned int size,
		 const inferiorHeapType type):
    name_(name),addr_(addr),size_(size), type_(type) {}
  heapDescriptor():
    name_(string("")),addr_(0),size_(0),type_(anyHeap) {}
  ~heapDescriptor() {}
  heapDescriptor &operator=(const heapDescriptor& h)
    {
      name_ = h.name();
      addr_ = h.addr();
      size_ = h.size();
      type_ = h.type();
      return *this;
    }
  const string &name() const {return name_;}
  const Address &addr() const {return addr_;}
  const unsigned &size() const {return size_;}
  const inferiorHeapType &type() const {return type_;}
 private:
  string name_;
  Address addr_;
  unsigned size_;
  inferiorHeapType type_;
};


class inferiorHeap {
 public:
  inferiorHeap(): heapActive(addrHash16) {
      freed = 0; disabledListTotalMem = 0; totalFreeMemAvailable = 0;
  }
  inferiorHeap(const inferiorHeap &src);  // create a new heap that is a copy
                                          // of src (used on fork)
  dictionary_hash<Address, heapItem*> heapActive; // active part of heap 
  vector<heapItem*> heapFree;           // free block of data inferior heap 
  vector<disabledItem> disabledList;    // items waiting to be freed.
  int disabledListTotalMem;             // total size of item waiting to free
  int totalFreeMemAvailable;            // total free memory in the heap
  int freed;                            // total reclaimed (over time)

  vector<heapItem *> bufferPool;        // distributed heap segments -- csserra
};


 
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
class mutationRecord {
public:
    mutationRecord *next;
    mutationRecord *prev;

    Address     addr;
    int         size;
    void        *data;

    mutationRecord(Address _addr, int _size, const void *_data);
    ~mutationRecord();
};

class mutationList {
private:
    mutationRecord      *head;
    mutationRecord      *tail;
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
  unsigned result = (unsigned)(Address)ip;
  result >>= 2;
  return result;
  // how about %'ing by a huge prime number?  Nah, x % y == x when x < y 
  // so we don't want the number to be huge.
}


static inline unsigned instInstanceHash(instInstance * const &inst) {
   unsigned result = (unsigned)(Address)inst;
   result >>= 2;
   return result; // how about %'ing by a huge prime number?
//  return ((unsigned)inst);
}


class Frame {
  private:
    bool      uppermost_;
    Address   pc_;
    Address   fp_;
    Address   sp_;     // NOTE: this is not always populated
    int       lwp_id_; // kernel-level thread (LWP)
#if defined(mips_sgi_irix6_4)
    Address   saved_fp;
#endif
#if defined(MT_THREAD)
    pdThread *thread_; // user-level thread
#endif

  public:

    /* platform-independent methods */

    // process ctor (toplevel frame)
    Frame(process *);
    // process ctor (toplevel frame, particular LWP)
    Frame(process *, unsigned);
    // default ctor (zero frame)
    Frame() : uppermost_(false), pc_(0), fp_(0), lwp_id_(0)
#if defined(MT_THREAD)
      ,thread_(NULL) 
#endif
      {}

    Address getPC() const { return pc_; }
    Address getFP() const { return fp_; }
#ifdef mips_unknown_ce2_11 //ccw 6 feb 2001 : 29 mar 2001
    Address getSP() const { return sp_; }
#endif
    int getLWP() const { return lwp_id_;}

    // check for zero frame
    bool isLastFrame() const { 
      if (pc_ == 0) return true;
      if (fp_ == 0) return true;
      return false;
    }

    // get stack frame of caller
    Frame getCallerFrame(process *proc) const;


    /* platform-dependent methods */

#if defined(MT_THREAD)
    // thread-specific ctors (see solaris.C, aix.C)
    Frame(pdThread *);
    Frame(int lwpid, Address fp, Address pc, bool uppermost)
      : uppermost_(uppermost), pc_(pc), fp_(fp), 
      lwp_id_(lwpid), thread_(NULL)
      {}
#endif

 private:

    // platform-dependent component of Frame::Frame(process *)
    void getActiveFrame(process *);

    // platform-dependent components of getCallerFrame()
    Frame getCallerFrameNormal(process *) const;
#if defined(MT_THREAD)
    Frame getCallerFrameLWP(process *) const;
    Frame getCallerFrameThread(process *) const;
#endif
};

typedef void (*continueCallback)(timeStamp timeOfCont);

// inferior RPC callback function type
typedef void(*inferiorRPCcallbackFunc)(process *p, void *data, void *result);

// Get the file descriptor for the (eventually) 'symbols' image
// Necessary data to load and parse the executable file
fileDescriptor *getExecFileDescriptor(string filename,
				      int &status,
				      bool); // bool for AIX

class process {
 friend class ptraceKludge;
#ifdef BPATCH_LIBRARY
 friend class BPatch_image;
#endif
 friend Address loadDyninstDll(process *, char Buffer[]);

  //  
  //  PUBLIC MEMBERS FUNCTIONS
  //  

 public:

  //removed for output redirection
  process(int iPid, image *iImage, int iTraceLink
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
     // Where the inferior attached was left undefined in the constructor; this
     // routine fills it in (tells paradynd where, in the inferior proc's addr
     // space, the shm seg was attached.  The attaching was done in DYNINSTinit)
#endif

  vector<Address> walkStack(bool noPause=false);
  bool triggeredInStackFrame(instPoint* point, pd_Function* stack_fn,
                             Address pc, callWhen when, callOrder order);
#if defined(MT_THREAD)  
#if defined(rs6000_ibm_aix4_1)
  // We have the pthread debug library to deal with
  pthdb_session_t *get_pthdb_session() { return &pthdb_session_; }
  pthdb_session_t pthdb_session_;
  bool init_pthdb_library();
#endif

  // Walk threads stacks
  vector<vector<Address> > walkAllStack(bool noPause=false);
  void walkAStack(int, Frame, Address sig_addr, u_int sig_size,
                  vector<Address>&pcs, vector<Address>&fps);
  bool getLWPIDs(int **IDs_p); //caller should do a "delete [] *IDs_p"
  bool getLWPFrame(int lwp_id, Address *fp, Address *pc);
  bool readDataFromLWPFrame(int lwp_id, 
                         Address currentFP, 
                         Address *previousFP, 
                         Address *rtn, 
                         bool uppermost=false);

  bool readDataFromThreadFrame(Address currentFP, 
                         Address *previousFP, 
                         Address *rtn, 
                         bool uppermost=false);
  bool getActiveFrame(Address *fp, Address *pc, int *lwpid);

  // Notify daemon of threads
  pdThread *createThread(
    int tid, 
    unsigned pos, 
    unsigned stack_addr, 
    unsigned start_pc, 
    void* resumestate_p, 
    bool);
  void updateThread(pdThread *thr, int tid, unsigned pos, void* resumestate_p,
                    resource* rid) ;
  void updateThread(
    pdThread *thr, 
    int tid, 
    unsigned pos, 
    unsigned stack_addr, 
    unsigned start_pc, 
    void* resumestate_p);
  void deleteThread(int tid);

  // SAFE inferiorRPC
  bool     handleDoneSAFEinferiorRPC(void);
#endif

  processState status() const { return status_;}
  int exitCode() const { return exitCode_; }
  string getStatusAsString() const; // useful for debug printing etc.

  bool checkContinueAfterStop() {
          if( continueAfterNextStop_ ) {
                  continueAfterNextStop_ = false;
                  return true;
          }
          return false;
  }
                  
  void continueAfterNextStop() { continueAfterNextStop_ = true; }
  void Exited();
  bool hasExited() { return (status_ == exited); }
  void Stopped();

#ifdef DETACH_ON_THE_FLY
  bool haveDetached;
  bool juststopped;
  bool needsDetach;
  int pendingSig;
  int detach();
  int reattach();
  int reattachAndPause();
  int detachAndContinue();
#endif /* DETACH_ON_THE_FLY */

  bool findInternalSymbol(const string &name, bool warn, internalSym &ret_sym)
         const;

  Address findInternalAddress(const string &name, bool warn, bool &err) const;

#ifdef BPATCH_LIBRARY
  bool setProcfsFlags();
  bool dumpImage(string outFile);

#ifdef sparc_sun_solaris2_4
  bool dumpPatchedImage(string outFile);//ccw 28 oct 2001
#else
  bool dumpPatchedImage(string outFile) { return false; } 
#endif
  string execPathArg;	// path exec is trying - used when process calls exec
#else
  bool dumpImage();
#endif

  bool symbol_info(const string &name, Symbol &ret) {
     assert(symbols);
     return symbols->symbol_info(name, ret);
  }

  // This will find the named symbol in the image or in a shared object
  bool getSymbolInfo( const string &name, Symbol &ret );

  image *getImage() const {
     assert(symbols);
     return symbols;
  }

  // this is only used on aix so far - naim
  // And should really be defined in a arch-dependent place, not process.h - bernat
  Address getTOCoffsetInfo(Address);

  bool dyninstLibAlreadyLoaded() { return hasLoadedDyninstLib; }
  bool dyninstLibIsBeingLoaded() { return isLoadingDyninstLib; }
  void clearDyninstLibLoadFlags() {
        hasLoadedDyninstLib = isLoadingDyninstLib = false; }
  unsigned numOfActCounters_is;
  unsigned numOfActProcTimers_is;
  unsigned numOfActWallTimers_is;
#if defined(MT_THREAD)
  unsigned numOfCurrentLevels_is;
  unsigned numOfCurrentThreads_is; 
#endif
  bool deferredContinueProc;
  void updateActiveCT(bool flag, CTelementType type);
  void cleanRPCreadyToLaunch(int mid);
#if defined(MT_THREAD)
  void postRPCtoDo(AstNode *, bool noCost,
                   inferiorRPCcallbackFunc, void *data, int, 
                   int thrId=(-1), 
		   bool isSAFE=false,
		   bool lowmem=false);
#else
  void postRPCtoDo(AstNode *, bool noCost,
                   inferiorRPCcallbackFunc, void *data, int, bool lowmem=false);
#endif
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

  void SendAppIRPCInfo(int runningRPC, unsigned begRPC, unsigned endRPC);
  void SendAppIRPCInfo(Address curPC);
#ifdef INFERIOR_RPC_DEBUG
  void CheckAppTrapIRPCInfo();
#endif
  bool handleTrapIfDueToRPC();
     // look for curr PC reg value in 'trapInstrAddr' of 'currRunningRPCs'.
     // Return true iff found.  Also, if true is being returned, then
     // additionally does a 'launchRPCifAppropriate' to fire off the next
     // waiting RPC, if any.
  bool changePC(Address addr);

  bool getCurrPCVector(vector <Address> &currPCs);

#if defined(i386_unknown_solaris2_5)
  bool changeIntReg(int reg, Address addr);
#endif

  void installBootstrapInst();
  void installInstrRequests(const vector<instMapping*> &requests);

  int getPid() const { return pid;}

  bool heapIsOk(const vector<sym_data>&);
  bool initDyninstLib();

  // Get the list of inferior heaps from:
  bool getInfHeapList(vector<heapDescriptor> &infHeaps); // Whole process
  bool getInfHeapList(const image *theImage,
		      vector<heapDescriptor> &infHeaps); // Single image

  void addInferiorHeap(const image *theImage);

  void initInferiorHeap();

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  bool isAddrInHeap(Address addr) 
    {
      for (unsigned i = 0; i < heap.bufferPool.size(); i++) {
        heapItem *seg = heap.bufferPool[i];
        if (addr >= seg->addr && addr < seg->addr + seg->length)
          return true;
      }
      return false;
    }
#endif

#ifdef BPATCH_LIBRARY
#ifdef sparc_sun_solaris2_4
  vector<imageUpdate*> imageUpdates;//ccw 28 oct 2001
  vector<imageUpdate*> highmemUpdates;//ccw 20 nov 2001
  vector<dataUpdate*>  dataUpdates;//ccw 26 nov 2001
#endif
#endif
	void saveWorldData(Address address, int size, const void* src);

  bool writeDataSpace(void *inTracedProcess,
                      u_int amount, const void *inSelf);
  bool readDataSpace(const void *inTracedProcess, u_int amount,
                     void *inSelf, bool displayErrMsg);

  bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);
  bool writeTextWord(caddr_t inTracedProcess, int data);
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  bool readTextSpace(const void *inTracedProcess, u_int amount,
                     const void *inSelf);
#endif
  bool continueProc();
#ifdef BPATCH_LIBRARY
  bool terminateProc() { return terminateProc_(); }
#endif
  ~process();
  bool pause();

  bool replaceFunctionCall(const instPoint *point,const function_base *newFunc);

  bool dumpCore(const string coreFile);
  bool detach(const bool paused); // why the param?
  bool API_detach(const bool cont); // XXX Should eventually replace detach()
  bool attach();
#ifndef BPATCH_LIBRARY
  void FillInCallGraphStatic();
  void MonitorDynamicCallSites(string function_name);
  bool MonitorCallSite(instPoint *callSite);
  bool isDynamicCallSite(instPoint *callSite); 
#endif

#ifdef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001
	 //void* GetRegisters() { return getRegisters(); }
	 //ccw 10 aug 2000
	 void* GetRegisters(unsigned int thrHandle) { return getRegisters(thrHandle); }
	 void* getRegisters(unsigned int thrHandle);
	 //bool FlushInstructionCache(); //ccw 29 sep 2000 implemented in pdwinnt.C
#endif

  // Trampoline guard get/set functions
  unsigned long getTrampGuardFlagAddr(void) { return trampGuardFlagAddr; }
  void setTrampGuardFlagAddr(unsigned long t) { trampGuardFlagAddr = t;  }

  // Cpu time related functions and members
#ifndef BPATCH_LIBRARY
 public:
  // called by process object constructor
  void initCpuTimeMgr();
  // called by initCpuTimeMgr, sets up platform specific aspects of cpuTimeMgr
  void initCpuTimeMgrPlt();

  // Call getCpuTime to get the current cpu time of process. Time conversion
  // from raw to primitive time units is done in relevant functions by using
  // the units ratio as defined in the cpuTimeMgr.  getCpuTime and getRawTime
  // use the best level as determined by the cpuTimeMgr.
  timeStamp getCpuTime(int lwp_id = -1);
  timeStamp units2timeStamp(int64_t rawunits);
  timeLength units2timeLength(int64_t rawunits);
  rawTime64 getRawCpuTime(int lwp_id = -1);

 private:
  // Platform dependent (ie. define in platform files) process time retrieval
  // function for daemon.  Use process::getCpuTime instead of calling these
  // functions directly.  If platform doesn't implement particular level,
  // still need to define a definition (albeit empty).  Ignores lwp_id arg if
  // lwp's are irrelevant for platform.
  rawTime64 getRawCpuTime_hw(int lwp_id);
  rawTime64 getRawCpuTime_sw(int lwp_id);

  // function always returns true, used when timer level is always available
  bool yesAvail();
  // The process time time mgr.  This handles choosing the best timer level
  // to use.  Call getTime member with a process object and an integer lwp_id
  // as args.
  typedef timeMgr<process, int> cpuTimeMgr_t;
  cpuTimeMgr_t *cpuTimeMgr;

#ifdef i386_unknown_linux2_0
  bool isLibhrtimeAvail();           // for high resolution cpu timer on linux
  void free_hrtime_link();
#ifdef HRTIME
  struct hrtime_struct *hr_cpu_link;
#endif
#endif
#ifdef mips_sgi_irix6_4
  bool isR10kCntrAvail();
#endif
#ifdef rs6000_ibm_aix4_1
  bool isPmapiAvail();
#endif

  // Verifies that the wall and cpu timer levels chosen by the daemon are
  // also available within the rtinst library.  This is an issue because the
  // daemon chooses the wall and cpu timer levels to use at daemon startup
  // and process object initialization respectively.  There is an outside
  // chance that the level would be determined unavailable by the rtinst
  // library upon application startup.  Asserts if there is a mismatch.
  void verifyTimerLevels();
  // Sets the wall and cpu time retrieval functions to use in the the rtinst
  // library by setting a function ptr in the rtinst library to the address
  // of the chosen function.
  void writeTimerLevels();
  // helper routines for writeTimerLevels
  void writeTimerFuncAddr(const char *rtinstVar, const char *rtinstHelperFPtr);
  bool writeTimerFuncAddr_(const char *rtinstVar,const char *rtinstHelperFPtr);

  // returns the address to assign to a function pointer that will
  // allow the time querying function to be called (in the rtinst library)
  // on AIX, this address returned will be the address of a structure which 
  //   has a field that points to the proper querying function (function 
  //   pointers are handled differently on AIX)
  // on other platforms, this address will be the address of the time
  // querying function in the rtinst library
  Address getTimerQueryFuncTransferAddress(const char *helperFPtr);

  // handles setting time retrieval functions for the case of a 64bit daemon
  // and 32bit application
  // see process.C definition for why being disabled
  //bool writeTimerFuncAddr_Force32(const char *rtinstVar, 
  //			  const char *rtinstFunc);
 public:
#endif

#ifdef BPATCH_LIBRARY
  BPatch_point *findOrCreateBPPoint(BPatch_function *bpfunc, instPoint *ip,
				    BPatch_procedureLocation pointType);
  BPatch_function *findOrCreateBPFunc(pd_Function* pdfunc, BPatch_module* bpmod = NULL);
#endif

  //  
  //  PUBLIC DATA MEMBERS
  //  

#if defined(BPATCH_LIBRARY)
  BPatch_thread *thread; // The BPatch_thread associated with this process
#endif

  // the following 2 vrbles probably belong in a different class:
  static string programName; // the name of paradynd (specifically, argv[0])
  static vector<string> arg_list; // the arguments of paradynd
  static string pdFlavor;
  static string dyninstName; // the filename of the runtime library


  // These member vrbles should be made private!
  int traceLink;                /* pipe to transfer traces data over */
  //removed for output redirection
  //int ioLink;                   /* pipe to transfer stdout/stderr over */
  int exitCode_;                /* termination status code */
  processState status_;         /* running, stopped, etc. */
  vector<pdThread *> threads;   /* threads belonging to this process */
#if defined(MT_THREAD) 
  hashTable *threadMap;         /* mapping table for tid->superTable */
  Address DYNINST_allthreads_p ;  /* in libRTinst  */
  Address allthreads ;            /* in libthread  */
  vector<metricDefinitionNode*> allMIComponentsWithThreads;
  bool preambleForDYNINSTinit;
  bool inThreadCreation;

  // SAFE inferiorRPC
  rpcToDo* DYNINSTthreadRPC; 
  mutex_t* DYNINSTthreadRPC_mp;
  cond_t*  DYNINSTthreadRPC_cvp;
  int*     DYNINSTthreadRPC_pending_p;
#endif//MT_THREAD 
  bool continueAfterNextStop_;

  resource *rid;                /* handle to resource for this process */

  /* map an inst point to its base tramp */
  dictionary_hash<const instPoint*, trampTemplate *> baseMap;   

#ifdef BPATCH_LIBRARY
  /* map a dyninst internal function back to a BPatch_function(per proc) */
  dictionary_hash <function_base*, BPatch_function*> PDFuncToBPFuncMap;

  /* map an address to an instPoint (that's not at entry, call or exit) */
  dictionary_hash<Address, BPatch_point *> instPointMap;
#endif

  // the following 3 are used in perfStream.C
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;

  bool reachedFirstBreak; // should be renamed 'reachedInitialTRAP'
  bool reachedVeryFirstTrap; 

  // inferior heap management
 public:
  bool splitHeaps;              /* are there separate code/data heaps? */
  inferiorHeap heap;            /* the heap */

  //
  //  PRIVATE DATA MEMBERS (and structure definitions)....
  //
 private:
#if !defined(i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
  unsigned char savedCodeBuffer[BYTES_TO_SAVE];
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
  unsigned char savedStackFrame[BYTES_TO_SAVE];
#endif
#endif
  struct inferiorRPCtoDo {
     // This structure keeps track of an inferiorRPC that we will start sometime
     // in the (presumably near) future.  There's a different structure for RPCs
     // which have been launched and which we're waiting to finish.
     // Don't confuse the two!

     AstNode *action;
     bool noCost; // if true, cost model isn't updated by generated code.
     inferiorRPCcallbackFunc callbackFunc;
     void *userData;
     int mid;
     bool lowmem; /* set to true when the inferior is low on memory */
#if defined(MT_THREAD)
     int thrId;
     bool isSafeRPC; // launch it as safe RPC or regular RPC
                     // if DYNINSTinit, we launch it as regular RPC
                     // otherwise launch it as MT RPC
#endif
  };

  int abortSyscall();
  vectorSet<inferiorRPCtoDo> RPCsWaitingToStart;
  bool RPCs_waiting_for_syscall_to_complete;
  bool was_running_before_RPC_syscall_complete;
  void *save_exitset_ptr; // platform-specific (for now, just solaris;
                          // it's actually a sysset_t*)
 
  // Trampoline guard location
  unsigned long trampGuardFlagAddr;
                                               
  struct inferiorRPCinProgress {
     // This structure keeps track of a launched inferiorRPC which we're
     // waiting to complete.  Don't confuse with 'inferiorRPCtoDo', 
     // which is more of a wait queue of RPCs to start launching.
     // Also note: It's only safe for 1 (one) RPC to be in progress at a time.
     // If you _really_ want to launch multiple RPCs at the same time, it's
     // actually easy to do...just do one inferiorRPC with a sequenceNode AST!
     // (Admittedly, that confuses the semantics of callback functions.  So the
     // official line remains: only 1 inferior RPC per process can be ongoing.)
     inferiorRPCcallbackFunc callbackFunc;
     void *userData;
     
     void *savedRegs; // crucial!

     bool wasRunning; // were we running when we launched the inferiorRPC?

     Address firstInstrAddr; // start location of temp tramp

     Address stopForResultAddr;
        // location of TRAP or ILL which marks point where paradynd should grab
        // the result register.  Undefined if no callback fn.
     Address justAfter_stopForResultAddr; // undefined if no callback fn.
     Register resultRegister; // undefined if no callback fn.

     void *resultValue; // undefined until we stop-for-result, at which time we
                        // fill this in.  callback fn (which takes this value)
                        // isn't invoked until breakAddr (the final break)

     Address breakAddr;
        // location of TRAP or ILL insn which marks the end of the inferiorRPC
#if defined(MT_THREAD)
     bool isSafeRPC ;
#endif
    unsigned lwp; // Target the RPC to a specific kernel thread?
  };
  vectorSet<inferiorRPCinProgress> currRunningRPCs;
      // see para above for reason why this 'vector' can have at most 1 elem!


  //
  //  PRIVATE MEMBER FUNCTIONS
  // 

  bool need_to_wait(void) ;
  // The follwing 5 routines are implemented in an arch-specific .C file
  bool emitInferiorRPCheader(void *, Address &baseBytes);
#if defined(MT_THREAD)
  bool emitInferiorRPCtrailer(void *, Address &baseBytes,
                              unsigned &breakOffset,
                              bool stopForResult,
                              unsigned &stopForResultOffset,
                              unsigned &justAfter_stopForResultOffset, 
                              bool isMT=false);
#else
  bool emitInferiorRPCtrailer(void *, Address &baseBytes,
                              unsigned &breakOffset,
                              bool stopForResult,
                              unsigned &stopForResultOffset,
                              unsigned &justAfter_stopForResultOffset);
#endif
#if defined(MT_THREAD)
  Address createRPCtempTramp(AstNode *action,
                              bool noCost, bool careAboutResult,
                              Address &breakAddr,
                              Address &stopForResultAddr,
                              Address &justAfter_stopForResultAddr,
                              Register &resultReg, bool lowmem=false,
                              int thrId=(-1), bool isMT=false);
#else
  Address createRPCtempTramp(AstNode *action,
                              bool noCost, bool careAboutResult,
                              Address &breakAddr,
                              Address &stopForResultAddr,
                              Address &justAfter_stopForResultAddr,
                              Register &resultReg, bool lowmem=false);
#endif
  void *getRegisters();
     // ptrace-GETREGS and ptrace-GETFPREGS (or /proc PIOCGREG and PIOCGFPREG).
     // Result is returned in an opaque type which is allocated with new[]

  bool changePC(Address addr,
                const void *savedRegs // returned by getRegisters()
                );
#if defined(MT_THREAD)
#if defined(rs6000_ibm_aix4_1)
 public:
  int findLWPbyPthread(int tid);
 private:
  bool changePC(Address addr, const void *ignored,
		int thrId);
#else
 public:
  int findLWPbyPthread(int tid) { return tid; }
 private:
#endif
#endif

  bool executingSystemCall();

  bool restoreRegisters(void *buffer);
     // input is the opaque type returned by getRegisters()

  bool set_breakpoint_for_syscall_completion();
  void clear_breakpoint_for_syscall_completion();
  Address read_inferiorRPC_result_register(Register);

 public:

#if !defined(i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
  Address get_dlopen_addr() const;
  Address dyninstlib_brk_addr;
  Address main_brk_addr;
#if !defined(alpha_dec_osf4_0) && !defined(rs6000_ibm_aix4_1)
  Address rbrkAddr() { assert(dyn); return dyn->get_r_brk_addr(); }
#endif
  bool dlopenDYNINSTlib();
  bool trapDueToDyninstLib();
  bool trapAtEntryPointOfMain();
//  bool wasCreatedViaAttach() { return createdViaAttach; }
  bool wasCreatedViaFork() { return createdViaFork; }
  void handleIfDueToDyninstLib();  
  void insertTrapAtEntryPointOfMain();
  void handleTrapAtEntryPointOfMain();
#endif
  bool wasCreatedViaAttach() { return createdViaAttach; } //ccw 28 june 2001 : was above

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
  // It also writes to "map" s.t. for each instInstance in the parent, we have
  // the corresponding instInstance in the child.
#ifdef SHM_SAMPLING
  static process *forkProcess(const process *parent, pid_t childPid,
                              dictionary_hash<instInstance*,instInstance*> &map,
                              int iTrace_fd,
                              key_t theKey,
                              void *applAttachedAtPtr);
#else
  static process *forkProcess(const process *parent, pid_t childPid,
                              dictionary_hash<instInstance*,instInstance*> &map,
                              int iTrace_fd);
#endif

  // get and set info. specifying if this is a dynamic executable
  void setDynamicLinking(){ dynamiclinking = true;}
  bool isDynamicallyLinked() { return (dynamiclinking); }

  // handleIfDueToSharedObjectMapping: if a trap instruction was caused by
  // a dlopen or dlclose event then return true
  bool handleIfDueToSharedObjectMapping();

  // handleStartProcess: this function is called when an appplication 
  // starts executing.  It is used to insert instrumentation necessary
  // to handle dynamic linking
  bool handleStartProcess();

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
  // shared object images for this function.  
  // mcheyney - should return NULL if function is excluded!!!!
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
  function_base *findOneFunction(const string &func_name) const;

  // findFuncByName: returns function associated with "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  pd_Function *findFuncByName(const string &func_name);

  // Check all loaded images for a function containing the given address.
  pd_Function *findFuncByAddr(Address adr);


  // Correct a vector of PCs for the cases where any PC points to an
  // address in one of our tramps.  The PC should point to the address
  // to which the tramp returns
  void correctStackFuncsForTramps(vector<Address> &, vector<pd_Function *> &);

  // Convert a vector of PCs (program counters) to vector of
  //  functions which contain them.  Should return vector
  //  with exactly 1 entry (function) per element of <pcs>
  //  NULL is used if address cannot be resolved to unique function.
  // Used to convert vector of pcs returned by walkStack() to
  //  functions....
  vector <pd_Function*>convertPCsToFuncs(vector<Address> pcs);

  // findModule: returns the module associated with "mod_name" 
  // this routine checks both the a.out image and any shared object 
  // images for this module
  // if check_excluded is true it checks to see if the module is excluded
  // and if it is it returns 0.  If check_excluded is false it doesn't check

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4)
  // Same as vector <pd_Function*>convertPCsToFuncs(vector<Address> pcs);
  // except that NULL is not used if address cannot be resolved to unique 
  // function. Used in function relocation for x86.
  vector<pd_Function *>pcsToFuncs(vector<Address> pcs);
#endif


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
  bool getBaseAddress(const image *which, Address &baseAddress) const;

  // findCallee: finds the function called by the instruction corresponding
  // to the instPoint "instr". If the function call has been bound to an
  // address, then the callee function is returned in "target" and the
  // instPoint "callee" data member is set to pt to callee's function_base.
  // If the function has not yet been bound, then "target" is set to the
  // function_base associated with the name of the target function (this is
  // obtained by the PLT and relocation entries in the image), and the instPoint
  // callee is not set.  If the callee function cannot be found, (e.g. function
  // pointers, or other indirect calls), it returns false.
  // Returns false on error (ex. process doesn't contain this instPoint).
  bool findCallee(instPoint &instr, function_base *&target);

#ifndef BPATCH_LIBRARY
  bool SearchRelocationEntries(const image *owner, instPoint &instr,
                               function_base *&target,
                               Address target_addr, Address base_addr);
#endif
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

  string execFilePath;		// full path of process

  int getProcFileDescriptor(){ return proc_fd;}

#ifdef BPATCH_LIBRARY
  static int waitProcs(int *status, bool block = false);
#else
  static int waitProcs(int *status);
#endif

#if defined(alpha_dec_osf4_0)
  int waitforRPC(int *status,bool block = false);
  Address changedPCvalue;
#endif
  const process *getParent() const {return parent;}

#if defined(hppa1_1_hp_hpux)
  bool freeNotOK;
#endif

#ifdef SHM_SAMPLING
  key_t getShmKeyUsed() const {return inferiorHeapMgr.getShmKey();}
  bool doMajorShmSample();
  bool doMinorShmSample();

  const fastInferiorHeapMgr &getShmHeapMgr() const {
     return(inferiorHeapMgr);
  }

  unsigned getShmHeapTotalNumBytes() {
     return inferiorHeapMgr.getHeapTotalNumBytes();
  }

#if defined(MT_THREAD)
  void *getRTsharedDataInApplicSpace() {
     void *result = inferiorHeapMgr.getRTsharedDataInApplicSpace();
     return result;
  }
  void *getRTsharedDataInParadyndSpace() {
     void *result = inferiorHeapMgr.getRTsharedDataInParadyndSpace();
     return result;
  }
#endif
  void *getObsCostLowAddrInApplicSpace() {
     void *result = inferiorHeapMgr.getObsCostAddrInApplicSpace();
     return result;
  }
  void *getObsCostLowAddrInParadyndSpace() {
     void *result = inferiorHeapMgr.getObsCostAddrInParadyndSpace();
     return result;
  }
  void processCost(unsigned obsCostLow, timeStamp wallTime, 
		   timeStamp processTime);

   bool extractBootstrapStruct(PARADYN_bootstrapStruct *);
#endif /* shm_sampling */

   bool extractBootstrapStruct(DYNINST_bootstrapStruct *);
   bool isBootstrappedYet() const {
      return hasBootstrapped;
   }
   int procStopFromDYNINSTinit();
      // returns 0 if not processed, 1 for the usual processed case (process is
      // now paused), or 2 for the processed-but-still-running-inferiorRPC case

   void handleCompletionOfDYNINSTinit(bool fromAttach);
      // called by above routine.  Reads bs_record from applic, takes action.

   static void DYNINSTinitCompletionCallback(process *, void *data, void *ret);
      // inferiorRPC callback routine.

#ifdef DETACH_ON_THE_FLY
   // Golly, you'd think this would have been needed long ago.
   bool isRunningRPC() { return !(currRunningRPCs.empty()); }
#endif


private:
  // Since we don't define these, 'private' makes sure they're not used:
  process(const process &); // copy ctor
  process &operator=(const process &); // assign oper

  bool hasBootstrapped;
     // set to true when we get callback from inferiorRPC call to DYNINSTinit

  // following two variables are used when libdyninstRT is dynamically linked
  // On other platforms the values are undefined
  bool hasLoadedDyninstLib; // true iff dyninstlib has been loaded already
  bool isLoadingDyninstLib; // true iff we are currently loading dyninst lib
  
  // the next two variables are used when we are loading dyninstlib
  // They are used by the special inferior RPC that makes the call to load the
  // library -- we use a special inferior RPC because the regular RPC assumes
  // that the inferior heap already exists, which is not true if libdyninstRT
  // has not been loaded yet.
  char savedData[LOAD_DYNINST_BUF_SIZE];
  void *savedRegs;


  const process *parent;        /* parent of this process */
  image *symbols;               /* information related to the process */
  int pid;                      /* id of this process */

#ifdef SHM_SAMPLING
  // This is being used to avoid time going backwards in
  // getInferiorProcessCPUtime. We can't use a static variable inside this
  // procedure because there is one previous for each process - naim 5/28/97
  rawTime64 previous; 

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

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
  trampTableEntry trampTable[TRAMPTABLESZ];
  unsigned trampTableItems;
#endif

  dynamic_linking *getDyn() { return dyn; }

  Address currentPC() {
    Address pc;
    if (hasNewPC) {
      return currentPC_;
    } else if ((pc = Frame(this).getPC())) {
      currentPC_ = pc;
      return currentPC_;
    } else {
      abort();
    }
    return 0;
  }
  void setNewPC(Address newPC) {
    currentPC_ = newPC;
    hasNewPC = true;
  }

  void setCallbackBeforeContinue(continueCallback func) {
    callBeforeContinue = func;
  }

  inline Address costAddr()  const { return costAddr_; }
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

  // This is set by the constructor to the obvious value, and is rarely used.
  // Specifically, I made it so that the Linux version could still do a full
  // ptrace_attach, even though the rest of paradyn assumes we are Solaris 
  // and have an inherit_tracing_state flag -- DAN
  bool createdViaFork;

  // the following 2 are defined only if 'createdViaAttach' is true; action is
  // taken on these vrbles once DYNINSTinit completes.

  bool wasRunningWhenAttached;
  bool needToContinueAfterDYNINSTinit;

  Address currentPC_;
  bool hasNewPC;

  // for processing observed cost (see method processCost())
  int64_t cumObsCost; // in cycles
  unsigned lastObsCostLow; // in cycles

  continueCallback callBeforeContinue;

  Address costAddr_;
  bool execed_;  // true if this process does an exec...set in handleExec

  // deal with system differences for ptrace
  bool writeDataSpace_(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readDataSpace_(const void *inTracedProcess, u_int amount, void *inSelf);

  bool flushInstructionCache_(void *baseAddr, size_t size); //ccw 25 june 2001


  bool writeTextWord_(caddr_t inTracedProcess, int data);
  bool writeTextSpace_(void *inTracedProcess, u_int amount, const void *inSelf);
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  bool readTextSpace_(void *inTracedProcess, u_int amount, const void *inSelf);
#endif
  bool pause_();
  bool continueProc_();
#ifdef BPATCH_LIBRARY
  bool terminateProc_();
#endif
  bool dumpCore_(const string coreFile);
  bool osDumpImage(const string &imageFileName,  pid_t pid, Address codeOff);
  bool detach_();
#ifdef BPATCH_LIBRARY
  bool API_detach_(const bool cont); // XXX Should eventually replace detach_()
#endif
  bool attach_(); // low-level attach; called by attach() (was OS::osAttach())
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
                              // this is usually, but not always, "main"
                                 

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

  bool isRunning_() const;
     // needed to initialize the 'wasRunningWhenAttached' member vrble. 
     // Determines whether the process is running by doing a low-level OS
     // check, not by checking member vrbles like status_.  May assume that
     // process::attach() has already run, but can't assume anything else.
     // May barf with a not-yet-implemented error on a given platform if the
     // platform doesn't yet support attaching to a running process. But
     // there's no reason why we shouldn't be able to implement this on any
     // platform; after all, the output from the "ps" command can be used
     // (T --> return false, S or R --> return true, other --> return ?)

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
   mutationList beforeMutationList, afterMutationList;

   bool saveOriginalInstructions(Address addr, int size);
   bool writeMutationList(mutationList &list);
#endif

   // System call interruption, currently for Solaris, only.  If the
   // process is sleeping in a system call during an inferior RPC
   // attempt, we interrupt the system call, perform the RPC, and
   // restart the system call.  (This var is defined on all platforms
   // to avoid platform-dependent initialization in process ctor.)
   bool stoppedInSyscall;  
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
   // These variables are meaningful only when `stoppedInSyscall' is true.
   int stoppedSyscall;     // The number of the interrupted syscall
   prgregset_t syscallreg; // Registers during sleeping syscall
                           // (note we do not save FP registers)
   sigset_t sighold;       // Blocked signals during sleeping syscall
   Address postsyscallpc;  // PC after the syscall is interrupted
#endif

   // MT_AIX stuff
   // Keep the current "best guess" of the active kernel thread around
   unsigned curr_lwp;

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(mips_sgi_irix6_4)
   // some very useful items gathered from /proc as soon as /proc fd is opened)
   // (initialized in attach() [solaris.C],
   string argv0; // argv[0] of program, at the time it started up
   string pathenv; // path env var of program, at the time it started up
   string cwdenv; // curr working directory of program, at the time it started

//   string fullPathToExecutable_;
      // very useful, especially on attach (need this to parse the symbol table)


 public:
   const string &getArgv0() const {return argv0;}
   const string &getPathEnv() const {return pathenv;}
   const string &getCwdEnv() const {return cwdenv;}
#endif
};

#if defined(USES_DYNAMIC_INF_HEAP)
// platform-specific definition of "near" (i.e. close enough for one-insn jump)
void inferiorMallocConstraints(Address near_, Address &lo, Address &hi, inferiorHeapType type);
// platform-specific buffer size alignment
void inferiorMallocAlign(unsigned &size);
#endif /* USES_DYNAMIC_INF_HEAP */

Address inferiorMalloc(process *p, unsigned size, inferiorHeapType type=anyHeap,
                       Address near_=0, bool *err=NULL);
void inferiorFree(process *p, Address item, const vector<addrVecType> &);
process *createProcess(const string file, vector<string> argv, 
		       vector<string> envp, const string dir,
		       int stdin_fd, int stdout_fd, int stderr_fd);
#ifdef BPATCH_LIBRARY
bool attachProcess(const string &progpath, int pid, int afterAttach,
                   process *&newProcess);
#else
bool attachProcess(const string &progpath, int pid, int afterAttach);
#endif

void handleProcessExit(process *p, int exitStatus);

extern resource *machineResource;

extern vector<process *> processVec;

//
// PARADYND_DEBUG_XXX
//
extern int pd_debug_infrpc;
extern int pd_debug_catchup;


#endif
