/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/* $Id: process.h,v 1.253 2003/04/20 01:00:10 schendel Exp $
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
#include "dyninstAPI_RT/h/dyninstAPI_RT.h" // ccw 18 apr 2002 : SPLIT only for (DYNINSTbootstrapStruct

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
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/installed_miniTramps_list.h"
#include "dyninstAPI/src/syscalltrap.h"
#include "dyninstAPI/src/libState.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/rpcMgr.h"

#include "dyninstAPI/src/symtab.h" // internalSym

#include "dyninstAPI/src/imageUpdate.h" //ccw 29 oct 2001

#ifndef BPATCH_LIBRARY
#include "paradynd/src/shmSegment.h"
#include "paradynd/src/shmMgr.h"
#include "paradynd/src/variableMgr.h"
#include "paradynd/src/sharedMetaData.h"
#endif

#ifndef BPATCH_LIBRARY
#ifdef PAPI
#include "paradynd/src/papiMgr.h"
#endif
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include <procfs.h>
#endif

#if defined(HRTIME) && !defined(BPATCH_LIBRARY)
#include "hrtime.h"
#endif

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"

#if !defined(MAX_NUMBER_OF_THREADS) 
#define MAX_NUMBER_OF_THREADS 256
#endif

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
class trampTemplate;
class instReqNode;
class dyn_thread;
// TODO a kludge - to prevent recursive includes
class image;
class instPoint;
class dyn_lwp;
class rpcMgr;

#ifdef BPATCH_LIBRARY
class BPatch_thread;
class BPatch_function;
#endif

typedef enum { neonatal, running, stopped, exited } processState;
typedef enum { HEAPfree, HEAPallocated } heapStatus;
typedef enum { textHeap=0x01, dataHeap=0x02, anyHeap=0x33, lowmemHeap=0x40 }
        inferiorHeapType;
typedef pdvector<Address> addrVecType;
typedef enum { unstarted, begun, initialized, loadingRT, loadedRT, bootstrapped } bootstrapState_t;

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

  disabledItem(heapItem *h, const pdvector<addrVecType> &preds) :
    block(h), pointsToCheck(preds) {}
  disabledItem(const disabledItem &src) :
    block(src.block), pointsToCheck(src.pointsToCheck) {}

  // TODO: unused?
  disabledItem(Address ip, inferiorHeapType iht,
               const pdvector<addrVecType> &ipts) :
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


 ~disabledItem() {}
  
  heapItem block;                    // inferior heap block
  pdvector<addrVecType> pointsToCheck; // list of addresses to check against PCs

  Address getPointer() const {return block.addr;}
  inferiorHeapType getHeapType() const {return block.type;}
  const pdvector<addrVecType> &getPointsToCheck() const {return pointsToCheck;}
  pdvector<addrVecType> &getPointsToCheck() {return pointsToCheck;}
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
  pdvector<heapItem*> heapFree;           // free block of data inferior heap 
  pdvector<disabledItem> disabledList;    // items waiting to be freed.
  int disabledListTotalMem;             // total size of item waiting to free
  int totalFreeMemAvailable;            // total free memory in the heap
  int freed;                            // total reclaimed (over time)

  pdvector<heapItem *> bufferPool;        // distributed heap segments -- csserra
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

typedef void (*continueCallback)(timeStamp timeOfCont);

// Get the file descriptor for the (eventually) 'symbols' image
// Necessary data to load and parse the executable file
fileDescriptor *getExecFileDescriptor(string filename,
				      int &status,
				      bool); // bool for AIX

class process {
 friend class ptraceKludge;
 friend class dyn_thread;
 friend class dyn_lwp;
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
          , key_t theShmSegKey
#endif
          );
     // this is the "normal" ctor

  process(int iPid, image *iSymbols,
          bool& success 
#ifdef SHM_SAMPLING
          , key_t theShmSegKey
#endif
          );
     // this is the "attach" ctor

  process(const process &parentProc, int iPid, int iTrace_fd);
     // this is the "fork" ctor

  protected:  
  bool walkStackFromFrame(Frame currentFrame, // Where to start walking from
			  pdvector<Frame> &stackWalk); // return parameter
  public:
  // Preferred function: returns a stack walk (vector of frames)
  // for each thread in the program
  bool walkStacks(pdvector<pdvector<Frame> > &stackWalks);

  // Get a vector of the active frames of all threads in the process
  bool getAllActiveFrames(pdvector<Frame> &activeFrames);

  bool collectSaveWorldData;//this is set to collect data for
				//save the world

  bool triggeredInStackFrame(instPoint* point, Frame frame,
			     callWhen when, callOrder order);

  bool isInSignalHandler(Address addr);

  // Notify daemon of threads
#if defined(MT_THREAD)
  dyn_thread *createThread(
    int tid, 
    unsigned pos, 
    unsigned stack_addr, 
    unsigned start_pc, 
    void* resumestate_p, 
    bool);
  // For initial thread (assume main is top func)
  void updateThread(dyn_thread *thr, int tid, unsigned pos, void* resumestate_p,
                    resource* rid) ;
  // For new threads
  void updateThread(
    dyn_thread *thr, 
    int tid, 
    unsigned pos, 
    unsigned stack_addr, 
    unsigned start_pc, 
    void* resumestate_p);
  void deleteThread(int tid);
#endif

  processState status() const { return status_;}
  void savePreSignalStatus() { status_before_signal_ = status_; }
  processState preSignalStatus() const { return status_before_signal_; }
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
  static process *findProcess(int pid);
  bool findInternalSymbol(const string &name, bool warn, internalSym &ret_sym)
         const;

  Address findInternalAddress(const string &name, bool warn, bool &err) const;

  bool installSyscallTracing();
  pdvector<instMapping*> tracingRequests;

#ifdef BPATCH_LIBRARY
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
  char* dumpPatchedImage(string outFile);//ccw 28 oct 2001
#else
  char* dumpPatchedImage(string outFile) { return NULL; } 
#endif
#endif

  bool dumpImage(string outFile);
  
  bool symbol_info(const string &name, Symbol &ret) {
     assert(symbols);
     return symbols->symbol_info(name, ret);
  }

  // This will find the named symbol in the image or in a shared object
  bool getSymbolInfo( const string &name, Symbol &ret );

  void setImage( image* img )                   { symbols = img; }

  image *getImage() const {
     assert(symbols);
     return symbols;
  }

  // this is only used on aix so far - naim
  // And should really be defined in a arch-dependent place, not process.h - bernat
  Address getTOCoffsetInfo(Address);

  bool dyninstLibAlreadyLoaded() { return runtime_lib != 0; }

  bool deferredContinueProc;
  void updateActiveCT(bool flag, CTelementType type);

  rpcMgr *theRpcMgr;

  rpcMgr *getRpcMgr() const { return theRpcMgr; }

  void SendAppIRPCInfo(int runningRPC, unsigned begRPC, unsigned endRPC);
  void SendAppIRPCInfo(Address curPC);
#ifdef INFERIOR_RPC_DEBUG
  void CheckAppTrapIRPCInfo();
#endif

  bool getCurrPCVector(pdvector <Address> &currPCs);

#if defined(i386_unknown_solaris2_5)
  bool changeIntReg(int reg, Address addr);
#endif

#if !defined(BPATCH_LIBRARY)
  bool catchupSideEffect(Frame &frame, instReqNode *inst);
#endif

  void installInstrRequests(const pdvector<instMapping*> &requests);
  void recognize_threads(pdvector<unsigned> *completed_lwps);

  int getPid() const { return pid;}

  bool heapIsOk(const pdvector<sym_data>&);


  /***************************************************************************
   **** Runtime library initialization code (Dyninst)                     ****
   ***************************************************************************/
  bool loadDyninstLib();
  bool setDyninstLibPtr(shared_object *libobj);
  bool setDyninstLibInitParams();
  static void dyninstLibLoadCallback(process *, string libname, shared_object *libobj, void *data);
  bool finalizeDyninstLib();
  
  bool iRPCDyninstInit();
  static void DYNINSTinitCompletionCallback(process *, unsigned /* rpc_id */,
                                            void *data, void *ret);
  
  // Get the list of inferior heaps from:
  bool getInfHeapList(pdvector<heapDescriptor> &infHeaps); // Whole process
  bool getInfHeapList(const image *theImage,
		      pdvector<heapDescriptor> &infHeaps); // Single image

  void addInferiorHeap(const image *theImage);

  void initInferiorHeap();

  /* Find the tramp guard addr and set it */
  bool initTrampGuard();

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

void saveWorldData(Address address, int size, const void* src);

#ifdef BPATCH_LIBRARY
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)

  pdvector<imageUpdate*> imageUpdates;//ccw 28 oct 2001
  pdvector<imageUpdate*> highmemUpdates;//ccw 20 nov 2001
  pdvector<dataUpdate*>  dataUpdates;//ccw 26 nov 2001
  pdvector<string> loadLibraryCalls;//ccw 14 may 2002 
  pdvector<string> loadLibraryUpdates;//ccw 14 may 2002

	char* saveWorldFindDirectory();

	unsigned int saveWorldSaveSharedLibs(int &mutatedSharedObjectsSize, unsigned int &dyninst_SharedLibrariesSize,
		 char* directoryName, unsigned int &count);
	char* saveWorldCreateSharedLibrariesSection(int dyninst_SharedLibrariesSize);

	void saveWorldCreateHighMemSections(pdvector<imageUpdate*> &compactedHighmemUpdates, 
		pdvector<imageUpdate*> &highmemUpdates, void *newElf);
	void saveWorldCreateDataSections(void* ptr);
	void saveWorldAddSharedLibs(void *ptr);//ccw 14 may 2002
	void saveWorldloadLibrary(string tmp){ loadLibraryUpdates.push_back(tmp); };

#if defined(rs6000_ibm_aix4_1)
	void addLib(char *lname);//ccw 30 jul 2002
	int requestTextMiniTramp; //ccw 20 jul 2002
#endif

#endif
#endif

#if !defined(i386_unknown_nt4_0) 
	//ccw 3 sep 2002
	//These variables are used by UNIX.C during the loading
	//of the runtime libraries.
	//
	bool finishedDYNINSTinit;
	int RPCafterDYNINSTinit;
#endif
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
	 // Defined lower down
	 //void* getRegisters(unsigned int thrHandle);
	 //bool FlushInstructionCache(); //ccw 29 sep 2000 implemented in pdwinnt.C
#endif

  // Trampoline guard get/set functions
  Address trampGuardAddr(void) { return trampGuardAddr_; }
  void setTrampGuardAddr(Address addr) { trampGuardAddr_ = addr; }
  
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
  timeStamp getCpuTime(int lwp);
  timeStamp units2timeStamp(int64_t rawunits);
  timeLength units2timeLength(int64_t rawunits);
  rawTime64 getRawCpuTime(int lwp);

 private:
  // Platform dependent (ie. define in platform files) process time retrieval
  // function for daemon.  Use process::getCpuTime instead of calling these
  // functions directly.  If platform doesn't implement particular level,
  // still need to define a definition (albeit empty).  Ignores lwp arg if
  // lwp's are irrelevant for platform. The file descriptor argument "fd"
  // is used.

  // NOTE: It is the caller's responsibility to check for rollbacks!

  rawTime64 getRawCpuTime_hw(int lwp);
  rawTime64 getRawCpuTime_sw(int lwp);

  // function always returns true, used when timer level is always available
  bool yesAvail();
  // The process time time mgr.  This handles choosing the best timer level
  // to use.  Call getTime member with a process object and an integer lwp
  // as args.
  typedef timeMgr<process, int> cpuTimeMgr_t;
  cpuTimeMgr_t *cpuTimeMgr;

#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
  bool isLibhrtimeAvail();           // for high resolution cpu timer on linux
  bool isPapiAvail();           
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
  public:
  
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
  private:
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
  static string pdFlavor;
  string dyninstRT_name; // the filename of the dyninst runtime library

  // These member vrbles should be made private!
  int traceLink;                /* pipe to transfer traces data over */

  // the following 3 are used in perfStream.C
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;

  //removed for output redirection
  //int ioLink;                   /* pipe to transfer stdout/stderr over */
  procSignalWhat_t exitCode_;                /* termination status code */
  processState status_;         /* running, stopped, etc. */
  processState status_before_signal_; /* Store the previous proc state */
  
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

  private:
  bootstrapState_t bootstrapState;
  
  public:
  // True if we've reached or past a certain state
  bool reachedBootstrapState(bootstrapState_t state) const { return bootstrapState >= state; }
  // Strictly increments (we never drop back down)
  void setBootstrapState(bootstrapState_t state) {
      // DEBUG
      if (bootstrapState > state)
          cerr << "Warning: attempt to revert bootstrap state from "
               << bootstrapState << " to " << state << endl;
      else
          bootstrapState = state;
  }  
  string getBootstrapStateAsString() const;

  // inferior heap management
 public:
  bool splitHeaps;              /* are there separate code/data heaps? */
  inferiorHeap heap;            /* the heap */


#ifndef BPATCH_LIBRARY
#ifdef PAPI
  papiMgr* papi;
  papiMgr* getPapiMgr() { return papi; }
#endif

#endif

  //
  //  PRIVATE DATA MEMBERS (and structure definitions)....
  //
 private:
  unsigned char savedCodeBuffer[BYTES_TO_SAVE];
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
  unsigned char savedStackFrame[BYTES_TO_SAVE];
#endif


/////////////////////////////////////////////////////////////////
//  System call trap tracking
/////////////////////////////////////////////////////////////////

  public:
  // Overloaded: Address for linux-style, syscall # for /proc
  syscallTrap *trapSyscallExitInternal(Address syscall);
  bool clearSyscallTrapInternal(syscallTrap *trappedSyscall);
  
  // Check all traps entered for a match to the syscall
  bool checkTrappedSyscallsInternal(Address syscall);
    
  private:
  // A list of traps inserted at system calls
  pdvector<syscallTrap *> syscallTraps_;
  
  // Trampoline guard location -- actually an addr in the runtime library.
  Address trampGuardAddr_;
                                               
  
  //
  //  PRIVATE MEMBER FUNCTIONS
  // 


  bool need_to_wait(void) ;
  // The follwing 5 routines are implemented in an arch-specific .C file

 public:

#if !defined(BPATCH_LIBRARY)
  void init_shared_memory(process *parentProc);
#endif

#if !defined(i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
  Address get_dlopen_addr() const;
#endif
  Address dyninstlib_brk_addr;
  Address main_brk_addr;
#if !defined(alpha_dec_osf4_0) && !defined(rs6000_ibm_aix4_1) && !defined(i386_unknown_nt4_0)
  Address rbrkAddr() { assert(dyn); return dyn->get_r_brk_addr(); }
#endif
  bool getDyninstRTLibName();
  bool loadDYNINSTlib();
  bool loadDYNINSTlibCleanup();
  bool trapDueToDyninstLib();

  // trapAddress is not set on non-NT, we discover it inline
  bool trapAtEntryPointOfMain(Address trapAddress = 0);
  bool wasCreatedViaAttach() { return createdViaAttach; }
  bool wasCreatedViaFork() { return createdViaFork; }
  bool wasRunningWhenAttached() { return wasRunningWhenAttached_; }
  bool insertTrapAtEntryPointOfMain();
  bool handleTrapAtEntryPointOfMain();

  
  bool wasCreatedViaAttachToCreated() {return createdViaAttachToCreated; } // Ana

  string getProcessStatus() const;

  static string tryToFindExecutable(const string &progpath, int pid);
      // os-specific implementation.  Returns empty string on failure.
      // Otherwise, returns a full-path-name for the file.  Tries every
      // trick to determine the full-path-name, even though "progpath" may
      // be unspecified (empty string)

  bool continueWithForwardSignal(int sig); // arch-specific implementation
  
  // Used when we get traps for both child and parent of a fork.
  int childPid;
  int parentPid;
  int childHasSignalled() { return childPid; };
  int parentHasSignalled() { return parentPid; };
  void setChildHasSignalled(int c) { childPid = c; };
  void setParentHasSignalled(int c) { parentPid = c; };

  // get and set info. specifying if this is a dynamic executable
  void setDynamicLinking(){ dynamiclinking = true;}
  bool isDynamicallyLinked() { return (dynamiclinking); }

  // handleIfDueToSharedObjectMapping: if a trap instruction was caused by
  // a dlopen or dlclose event then return true
  bool handleIfDueToSharedObjectMapping();

  // Register a callback to be made when a library is detected as loading
  // (but possibly before it is initialized)
  bool registerLoadLibraryCallback(string libname,
                                   loadLibraryCallbackFunc callback,
                                   void *data);
  // And delete the above
  bool unregisterLoadLibraryCallback(string libname);  

  // Run a callback (if appropriate)
  bool runLibraryCallback(string libname, shared_object *libobj);
    
  private:
  // Hashtable of registered callbacks
  dictionary_hash<string, libraryCallback *> loadLibraryCallbacks_;

  public:
  // Insert instrumentation necessary to detect shared objects.
  bool initSharedObjects();
  

#if !defined(BPATCH_LIBRARY)
  bool handleStopDueToExecEntry();
#endif

  // getSharedObjects: This routine is called before main() to get and
  // process all shared objects that have been mapped into the process's
  // address space
  bool getSharedObjects();

  // addASharedObject: This routine is called whenever a new shared object
  // has been loaded by the run-time linker after the process starts running
  // It processes the image, creates new resources
  bool addASharedObject(shared_object &, Address newBaseAddr = 0);

  // return the list of dynamically linked libs
  pdvector<shared_object *> *sharedObjects() { return shared_objects;  } 

  // getMainFunction: returns the main function for this process
  function_base *getMainFunction() const { return mainFunction; }

#if !defined(BPATCH_LIBRARY)
  // findOneFunction: returns the function associated with function "func"
  // and module "mod".  This routine checks both the a.out image and any
  // shared object images for this function.  
  // mcheyney - should return NULL if function is excluded!!!!
  function_base *findOnlyOneFunction(resource *func,resource *mod);
#endif

  typedef struct {
    const instPoint *loc;
    callWhen when;
    installed_miniTramps_list *mtList;
  } mtListInfo;
  void getMiniTrampLists(pdvector<mtListInfo> *vecBuf);

  void newMiniTrampList(const instPoint *loc, callWhen when,
			installed_miniTramps_list **mtList);
  
  void getMiniTrampList(const instPoint *loc, callWhen when, 
			installed_miniTramps_list **mtList);

  void getMiniTrampList(const instPoint *loc, callWhen when, 
			const installed_miniTramps_list **mtList) const;

  void removeMiniTrampList(const instPoint *loc, callWhen when);

  instPoint *findInstPointFromAddress(Address addr, trampTemplate **bt=NULL,
				      instInstance **mt=NULL);

  // findAddressInFuncsAndTramps: returns the function which contains this
  // address.  This checks the a.out image and shared object images for this
  // function, as well as checking base- and mini-tramps which correspond to
  // this function.  If the address was in a tramp, the trampTemplate is
  // returned as well.
  pd_Function *findAddressInFuncsAndTramps(Address addr, instPoint **ip=NULL,
					   trampTemplate **bt=NULL, 
					   instInstance **mt=NULL);

#ifndef BPATCH_LIBRARY
  // returns all the functions in the module "mod" that are not excluded by
  // exclude_lib or exclude_func
  // return 0 on error.
  pdvector<function_base *> *getIncludedFunctions(module *mod); 
#endif

  // Now: multithread daemon/library
  bool multithread_capable() {
#if defined(MT_THREAD)
    return true;
#else
    return false;
#endif
  }
  // Do we have the RT-side multithread functions available
  bool multithread_ready() {
    if (!multithread_capable())
      return false;
#if !defined(BPATCH_LIBRARY)
    if (PARADYNhasBootstrapped)
      return true;
#endif
    return false;
  }
    
  void setLWPStoppedFromForkExit(unsigned lwp_id) {
     LWPstoppedFromForkExit = lwp_id;
  }
  unsigned getLWPStoppedFromForkExit() {
     return LWPstoppedFromForkExit;
  }

  unsigned maxNumberOfThreads() {
#if defined(MT_THREAD)
    return MAX_NUMBER_OF_THREADS;
#else
    return 1;
#endif
  }
  
  dyn_thread *STdyn_thread();

  // findOneFunction: returns the function associated with function "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  function_base *findOnlyOneFunction(const string &func_name) const;

  // findFuncByName: returns function associated with "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  //pd_Function *findFuncByName(const string &func_name);

  // And do it, returning a vector if multiple matches
  bool findAllFuncsByName(const string &func_name, pdvector<function_base *> &res);

  // Check all loaded images for a function containing the given address.
  pd_Function *findFuncByAddr(Address adr);

  // findModule: returns the module associated with "mod_name" 
  // this routine checks both the a.out image and any shared object 
  // images for this module
  // if check_excluded is true it checks to see if the module is excluded
  // and if it is it returns 0.  If check_excluded is false it doesn't check

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
  // Same as pdvector <pd_Function*>convertPCsToFuncs(pdvector<Address> pcs);
  // except that NULL is not used if address cannot be resolved to unique 
  // function. Used in function relocation for x86.
  pdvector<pd_Function *>pcsToFuncs(pdvector<Frame> stackWalk);
#endif

#ifndef BPATCH_LIBRARY
  module *findModule(const string &mod_name,bool check_excluded);
#else
  module *findModule(const string &mod_name);
#endif
  // getSymbolInfo:  get symbol info of symbol associated with name n
  // this routine starts looking a.out for symbol and then in shared objects
  // baseAddr is set to the base address of the object containing the symbol
  bool getSymbolInfo(const string &n, Symbol &info, Address &baseAddr) const;

  // getAllFunctions: returns a vector of all functions defined in the
  // a.out and in the shared objects
  pdvector<function_base *> *getAllFunctions();

  // getAllModules: returns a vector of all modules defined in the
  // a.out and in the shared objects
  pdvector<module *> *getAllModules();

#ifndef BPATCH_LIBRARY
  // getIncludedFunctions: returns a vector of all functions defined in the
  // a.out and in shared objects that are not excluded by an mdl option 
  pdvector<function_base *> *getIncludedFunctions();

  // getIncludedModules: returns a vector of all functions defined in the
  // a.out and in shared objects that are  not excluded by an mdl option
  pdvector<module *> *getIncludedModules();
#endif



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

  /////////////////////////////////////////////////////////////////////
  // Fork, Exec, and Exit callbacks
  /////////////////////////////////////////////////////////////////////

  // The callback functions are public because the signal handling code is
  // not a friend of the process class
  public:
  void registerPreForkCallback(forkEntryCallback_t callback, void *data);
  void registerPostForkCallback(forkExitCallback_t callback, void *data);
  void registerPreExecCallback(execEntryCallback_t callback, void *data);
  void registerPostExecCallback(execExitCallback_t callback, void *data);
  void registerPreExitCallback(exitEntryCallback_t callback, void *data);

  private:
  void *preForkData_;
  void *postForkData_;
  void *preExecData_;
  void *postExecData_;
  void *preExitData_;
  forkEntryCallback_t preForkCallback_;
  forkExitCallback_t postForkCallback_;
  execEntryCallback_t preExecCallback_;
  execExitCallback_t postExecCallback_;
  exitEntryCallback_t preExitCallback_;
  
  public:
  void handleForkEntry();
  void handleForkExit(process *child);
  void handleExecEntry(char *arg0);
  void handleExecExit();
  void handleExitEntry(int code);
  // Generic handler for anything else waiting on a system call
  // Returns true if handling was done
  bool handleSyscallExit(procSignalWhat_t syscall);
  
  // For platforms where we can't specifically tell if a signal is due to
  // fork or exec and have to guess
  bool nextTrapIsFork;
  bool nextTrapIsExec;

  // For platforms where we can't get the path string from the PID
  // Argument given to exec
  string execPathArg;
  // Full path info
  string execFilePath;
  
  //  wasExeced: returns true is the process did an exec...this is set
  //  in handleExec()
  bool wasExeced(){ return execed_;}
  private:
  bool execed_;  // true if this process does an exec...set in handleExec
  public:

  // True if we're in the process of an exec (from exec entry until we load
  // the dyninst RT lib)
  bool inExec;
  
  bool cleanUpInstrumentation(bool wasRunning); // called on exit (also exec?)

  dyn_thread *getThread(unsigned tid);
  dyn_lwp *getLWP(unsigned lwp_id);
  dyn_lwp *getDefaultLWP() const;

  dyn_lwp *createLWP(unsigned lwp_id);
#if defined(mips_unknown_ce2_11) || defined(i386_unknown_nt4_0)
  dyn_lwp *createLWP(unsigned lwp_id, int index, handleT fd);
#endif

  void deleteLWP(dyn_lwp *lwp_to_delete);

#if defined(alpha_dec_osf4_0)
  int waitforRPC(int *status,bool block = false);
#endif
  process *getParent() const {return parent;}

#if defined(hppa1_1_hp_hpux)
  bool freeNotOK;
#endif

#ifdef SHM_SAMPLING
  key_t getShmKeyUsed() const {return theSharedMemMgr->getShmKey();}

  shmMgr *getSharedMemMgr() { return theSharedMemMgr; };

  unsigned getShmHeapTotalNumBytes() {
     return theSharedMemMgr->getHeapTotalNumBytes();
  }

  
  void registerInferiorAttachedSegs(void *inferiorAttachedAtPtr);
  // Where the inferior attached was left undefined in the constructor; this
  // routine fills it in (tells paradynd where, in the inferior proc's addr
  // space, the shm seg was attached.  The attaching was done in DYNINSTinit)

  Address initSharedMetaData();
  sharedMetaData *shmMetaData;
  sharedMetaOffsetData *shMetaOffsetData;   // used to communicate offsets of 
                                            // shmMetaData memory to rtinst
 public:
  virtualTimer *getVirtualTimer(int pos) {
     return shmMetaData->getVirtualTimer(pos);
  }

  void *getObsCostLowAddrInApplicSpace() {
    void *result = theSharedMemMgr->getAddressInApplic(
                              (void *)shmMetaData->getObservedCost());
    return result;
  }
  void *getObsCostLowAddrInParadyndSpace() {
    return (void *)shmMetaData->getObservedCost();
  }

  void processCost(unsigned obsCostLow, timeStamp wallTime, 
		   timeStamp processTime);
#endif /* shm_sampling */

   bool extractBootstrapStruct(DYNINST_bootstrapStruct *);
   bool isBootstrappedYet() const {
      return bootstrapState == bootstrapped;
   }
#if !defined(BPATCH_LIBRARY)
   void setParadynBootstrap() {
       // This should be in paradyn's pd_process object, but is
       // needed for the is_multithreaded check
       PARADYNhasBootstrapped = true;
   }
   bool isParadynBootstrapped() {
       return PARADYNhasBootstrapped;
   }
#endif
   
private:
  // Since we don't define these, 'private' makes sure they're not used:
  process &operator=(const process &); // assign oper

#if !defined(BPATCH_LIBRARY)  //ccw 22 apr 2002 : SPLIT
  bool PARADYNhasBootstrapped;
#endif
  // the next two variables are used when we are loading dyninstlib
  // They are used by the special inferior RPC that makes the call to load the
  // library -- we use a special inferior RPC because the regular RPC assumes
  // that the inferior heap already exists, which is not true if libdyninstRT
  // has not been loaded yet.
  char savedData[LOAD_DYNINST_BUF_SIZE];
  struct dyn_saved_regs *savedRegs;


  process *parent;        /* parent of this process */
  image *symbols;               /* information related to the process */
  shared_object *runtime_lib;           /* shortcut to the runtime library */
  dictionary_hash<const instPoint *, installed_miniTramps_list*> 
                                           installedMiniTramps_beforePt;
  dictionary_hash<const instPoint *, installed_miniTramps_list*> 
                                           installedMiniTramps_afterPt;

  int pid;                      /* id of this process */

#ifndef BPATCH_LIBRARY
  // This is being used to avoid time going backwards in
  // getInferiorProcessCPUtime. We can't use a static variable inside this
  // procedure because there is one previous for each process - naim 5/28/97
  rawTime64 previous; 

  // New components of the conceptual "inferior heap"
  shmMgr *theSharedMemMgr;
#endif

public:

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
  trampTableEntry trampTable[TRAMPTABLESZ];
  unsigned trampTableItems;
#endif

  dynamic_linking *getDyn() { return dyn; }

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

  bool createdViaAttachToCreated;
  // This is set by constructor. True if thisprocess was created by an 
  // extern process and the process has been stopped just after the exec()
  // system call. -- Ana

  // the following 2 are defined only if 'createdViaAttach' is true; action is
  // taken on these vrbles once DYNINSTinit completes.

  bool wasRunningWhenAttached_;
  bool needToContinueAfterDYNINSTinit;

  // for processing observed cost (see method processCost())
  int64_t cumObsCost; // in cycles
  unsigned lastObsCostLow; // in cycles

  Address costAddr_;

  // deal with system differences for ptrace
  bool writeDataSpace_(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readDataSpace_(const void *inTracedProcess, u_int amount, void *inSelf);

  bool flushInstructionCache_(void *baseAddr, size_t size); //ccw 25 june 2001


  bool writeTextWord_(caddr_t inTracedProcess, int data);
  bool writeTextSpace_(void *inTracedProcess, u_int amount, const void *inSelf);
  bool readTextSpace_(void *inTracedProcess, u_int amount, const void *inSelf);

#if defined(i386_unknown_nt4_0)
  public:
#endif
  bool pause_();
  bool continueProc_();
#if defined(i386_unknown_nt4_0)
  private:
#endif
  
#ifdef BPATCH_LIBRARY
  bool terminateProc_();
#endif
  bool dumpCore_(const string coreFile);
  bool osDumpImage(const string &imageFileName,  pid_t pid, Address codeOff);
  bool detach_();
  bool API_detach_(const bool cont); // XXX Should eventually replace detach_()
  bool attach_(); // low-level attach; called by attach() (was OS::osAttach())
  bool stop_(); // formerly OS::osStop

  // stops a process
  bool loopUntilStopped();

  // returns true iff ok to do a ptrace; false (and prints a warning) if not
  bool checkStatus();

  // Prolly shouldn't be public... but then we need a stack of 
  // access methods. 
 public:
  dictionary_hash<unsigned, dyn_lwp *> lwps;

  pdvector<dyn_thread *> threads;   /* threads belonging to this process */
  handleT getProcessHandle() const { return procHandle_; }
  handleT as_fd() const     { return as_fd_; }
  handleT auxv_fd() const    { return auxv_fd_; }
  handleT map_fd() const    { return map_fd_; }
  handleT ps_fd() const     { return ps_fd_; }
  handleT status_fd() const { return status_fd_; }
  handleT usage_fd() const  { return usage_fd_; }

#if defined(sparc_sun_solaris2_4)
  bool get_status(pstatus_t *) const;
  bool set_syscalls (sysset_t *, sysset_t *) const;
  bool process::get_entry_syscalls(pstatus_t *status,
                                   sysset_t *entry);
  bool process::get_exit_syscalls(pstatus_t *status,
                                  sysset_t *exit);
#endif  
  
  private:
  handleT procHandle_; // Process-specific, as opposed to thread-specific,
                       // handle. Currently used by NT
  handleT as_fd_; // Process memory image (/proc)
  handleT auxv_fd_;
  handleT map_fd_;
  handleT status_fd_; // Status (/proc)
  handleT ps_fd_; // ps (/proc)
  handleT usage_fd_;

  // TODO: public access functions. The NT handler needs direct access
  // to this (or a NT-compatible handleIfDueToSharedObjectMapping)
  public:
  dynamic_linking *dyn;   // platform specific dynamic linking routines & data

  bool dynamiclinking;   // if true this a.out has a .dynamic section
  pdvector<shared_object *> *shared_objects;  // list of dynamically linked libs
  private:
  // The set of all functions and modules from the shared objects that show
  // up on the Where axis (both  instrumentable and uninstrumentable due to 
  // exclude_lib or exclude_func),  and all the functions from the a.out that
  // are between DYNINSTStart and DYNINSTend
  // TODO: these lists for a.out functions and modules should be handled the 
  // same way as for shared object functions and modules
  pdvector<function_base *> *all_functions;
  pdvector<module *> *all_modules;

  // these are a restricted set of functions and modules which are those  
  // from the a.out and shared objects that are instrumentable (not excluded 
  // through the mdl "exclude_lib" or "exclude_func" option) 
  // "excluded" now means never able to instrument
  pdvector<module *> *some_modules;  
  pdvector<function_base *> *some_functions; 
  bool waiting_for_resources;  // true if waiting for resourceInfoResponse
#if defined(i386_unknown_linux2_0)
  Address signal_restore;	// address of signal context restore function
				// (for stack walking)
#else
  pd_Function *signal_handler;  // signal handler function (for stack walking)
#endif

  function_base *mainFunction;  // the main function for this process,
                              // this is usually, but not always, "main"

  unsigned LWPstoppedFromForkExit;                                 

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

   // Data gathered via /proc
   string argv0;
   // argv[0] of program, at the time it started up
   string pathenv;
   // path env var of program, at the time it started up
   string cwdenv;
   // curr working directory of program, at the time it started

   int invalid_thr_create_msgs;

  public:
   const string &getArgv0() const { return argv0; }
   const string &getPathEnv() const { return pathenv; }
   const string &getCwdEnv() const { return cwdenv; }
   
   int numInvalidThrCreateMsgs() const { return invalid_thr_create_msgs; }
   void receivedInvalidThrCreateMsg() { invalid_thr_create_msgs++; }

 private:
   static void inferiorMallocCallback(process *proc, unsigned /* rpc_id */,
                                      void *data, void *result);

   bool inInferiorMallocDynamic;
   void inferiorMallocDynamic(int size, Address lo, Address hi);
   void inferiorFreeCompact(inferiorHeap *hp);
   int findFreeIndex(unsigned size, int type, Address lo, Address hi);

 public:
   // Handling of inferior memory management
#if defined(USES_DYNAMIC_INF_HEAP)
   // platform-specific definition of "near" (i.e. close enough for one-insn jump)
   void inferiorMallocConstraints(Address near_, Address &lo, Address &hi, inferiorHeapType type);
   // platform-specific buffer size alignment
   void inferiorMallocAlign(unsigned &size);
#endif /* USES_DYNAMIC_INF_HEAP */

   
   Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
			  Address near_=0, bool *err=NULL);
   void inferiorFree(Address item);

   // Maybe this should be in a different file... instead of crudding up
   // process.h more
   struct instPendingDeletion {
     pdvector<Address> hot;
     Address baseAddr;
     // Stupid duplication...
     instInstance *oldMini;
     trampTemplate *oldBase;
   };

   // Closely related: deletion of minitramps
   void deleteInstInstance(instInstance *delInst);
   bool checkIfInstAlreadyDeleted(instInstance *delInst);
   // And the associated deletion of a base tramp
   void deleteBaseTramp(trampTemplate *baseTramp, instInstance *lastMiniTramp);

Address inferiorMalloc(process *p, unsigned size, inferiorHeapType type=anyHeap,
                       Address near_=0, bool *err=NULL);
void inferiorFree(process *p, Address item, const pdvector<addrVecType> &);

// garbage collect instrumentation
   void gcInstrumentation();
   void gcInstrumentation(pdvector<pdvector<Frame> >&stackWalks);
 private:
   pdvector<instPendingDeletion *> pendingGCInstrumentation;
 
};


process *createProcess(const string file, pdvector<string> argv, 
		       pdvector<string> envp, const string dir,
		       int stdin_fd, int stdout_fd, int stderr_fd);

process *attachProcess(const string &progpath, int pid);

bool  AttachToCreatedProcess(int pid, const string &progpath);

void handleProcessExit(process *p, int exitStatus);

bool isInferiorAllocated(process *p, Address block);

extern resource *machineResource;

extern pdvector<process *> processVec;

//
// PARADYND_DEBUG_XXX
//
extern int pd_debug_infrpc;
extern int pd_debug_catchup;


#endif
