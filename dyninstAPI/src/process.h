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

/* $Id: process.h,v 1.311 2005/02/17 21:10:55 bernat Exp $
 * process.h - interface to manage a process in execution. A process is a kernel
 *   visible unit with a seperate code and data space.  It might not be
 *   the only unit running the code, but it is only one changed when
 *   ptrace updates are applied to the text space.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <assert.h>

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

#include "dyninstAPI/src/Object.h"
#include "common/h/String.h"
#include "common/h/vectorSet.h"
#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/syscalltrap.h"
#include "dyninstAPI/src/libState.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/codeRange.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/imageUpdate.h"

#if (! defined( BPATCH_LIBRARY )) && defined( PAPI )
#include "paradynd/src/papiMgr.h"
#endif

// Annoying... Solaris has two /proc header files, one for the
// multiple-FD /proc and one for an ioctl-based compatibility /proc.
// /usr/include/procfs.h is the one we want, and /usr/include/sys/procfs.h
// is the ioctl one. This makes it impossible to have a single include
// line. 
#if defined(os_solaris)
#include <procfs.h>
#elif defined(os_aix) || defined(os_alpha) || defined(os_irix)
#include <sys/procfs.h>
#endif

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"

#if defined( ia64_unknown_linux2_4 )
#include <libunwind.h>
#include <libunwind-ptrace.h>
#endif

#if defined(SHM_SAMPLING)
extern unsigned SHARED_SEGMENT_SIZE;
#endif

extern unsigned activeProcesses; // number of active processes
   // how about just processVec.size() instead?  At least, this should be made
   // a (static) member vrble of class process

class instPoint;
class trampTemplate;
class miniTrampHandle;
class instReqNode;
class dyn_thread;
// TODO a kludge - to prevent recursive includes
class image;
class instPoint;
class dyn_lwp;
class rpcMgr;
class syscallNotification;

#ifdef BPATCH_LIBRARY
class BPatch_thread;
class BPatch_function;
#endif

typedef enum { HEAPfree, HEAPallocated } heapStatus;
// Bit pattern...
typedef enum { textHeap=0x01,
               dataHeap=0x02,
               uncopiedHeap=0x04, // AIX -- not copied on fork
               anyHeap=0x7, // OR of the previous three
               lowmemHeap=0x1000 }
        inferiorHeapType;
typedef pdvector<Address> addrVecType;
typedef enum { unstarted, begun, initialized, loadingRT, loadedRT, bootstrapped } bootstrapState_t;
typedef enum { terminateFailed, terminateSucceeded, alreadyTerminated } terminateProcStatus_t;

const int LOAD_DYNINST_BUF_SIZE = 256;

typedef struct sym_data {
  pdstring name;
  bool must_find;
} sym_data;

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
    bperr( "error: unused disabledItem ctor\n");
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
  heapDescriptor(const pdstring name,
		 Address addr,
		 unsigned int size,
		 const inferiorHeapType type):
    name_(name),addr_(addr),size_(size), type_(type) {}
  heapDescriptor():
    name_(pdstring("")),addr_(0),size_(0),type_(anyHeap) {}
  ~heapDescriptor() {}
  heapDescriptor &operator=(const heapDescriptor& h)
    {
      name_ = h.name();
      addr_ = h.addr();
      size_ = h.size();
      type_ = h.type();
      return *this;
    }
  const pdstring &name() const {return name_;}
  const Address &addr() const {return addr_;}
  const unsigned &size() const {return size_;}
  const inferiorHeapType &type() const {return type_;}
 private:
  pdstring name_;
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


static inline unsigned miniTrampHandleHash(miniTrampHandle * const &inst) {
   unsigned result = (unsigned)(Address)inst;
   result >>= 2;
   return result; // how about %'ing by a huge prime number?
//  return ((unsigned)inst);
}

typedef void (*continueCallback)(timeStamp timeOfCont);

// Get the file descriptor for the (eventually) 'symbols' image
// Necessary data to load and parse the executable file
fileDescriptor *getExecFileDescriptor(pdstring filename,
				      int &status,
				      bool); // bool for AIX

class process {
 friend class ptraceKludge;
 friend class dyn_thread;
 friend class dyn_lwp;
 friend Address loadDyninstDll(process *, char Buffer[]);

  //  
  //  PUBLIC MEMBERS FUNCTIONS
  //  

 public:

  //removed for output redirection
  process(int iPid, image *iImage, int iTraceLink);
     // this is the "normal" ctor

  process(int iPid, image *iSymbols,
          bool& success);
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

  // Is the current address "after" the given instPoint?
  bool triggeredInStackFrame(Frame &frame,
                             instPoint *point,
                             callWhen when,
                             callOrder order);

  bool isInSignalHandler(Address addr);

  // Notify daemon of threads

  dyn_thread *createThread(
      int tid, 
      unsigned index, 
      unsigned lwp,
      unsigned stackbase, 
      unsigned startpc, 
      void* resumestate_p, 
      bool);
  // For initial thread (assume main is top func)
  void updateThread(dyn_thread *thr, int tid, unsigned index,
                    unsigned lwp,
                    void* resumestate_p) ;
  // For new threads
  void updateThread(
    dyn_thread *thr, 
    int tid, 
    unsigned index,
    unsigned lwp,
    unsigned stackbase,
    unsigned startpc, 
    void* resumestate_p);
  void deleteThread(int tid);

  // Thread index functions
  unsigned getIndexToThread(unsigned index);
  void setIndexToThread(unsigned index, unsigned value);
  void updateThreadIndexAddr(Address addr);
  
  private:
  Address threadIndexAddr;
  public:

  processState status() const { return status_;}

  // update the status on the whole process (ie. process state and all lwp
  // states)
  void set_status(processState st);

  // update the status of the process and one particular lwp in the process
  void set_lwp_status(dyn_lwp *whichLWP, processState st);

  // should only be called by dyn_lwp::continueLWP
  void clearCachedRegister();

  Address previousSignalAddr() const { return previousSignalAddr_; }
  void setPreviousSignalAddr(Address a) { previousSignalAddr_ = a; }
  pdstring getStatusAsString() const; // useful for debug printing etc.

  bool checkContinueAfterStop() {
          if( continueAfterNextStop_ ) {
                  continueAfterNextStop_ = false;
                  return true;
          }
          return false;
  }
                  
  void continueAfterNextStop() { continueAfterNextStop_ = true; }
  static process *findProcess(int pid);
  bool findInternalSymbol(const pdstring &name, bool warn, internalSym &ret_sym)
         const;

  Address findInternalAddress(const pdstring &name, bool warn, bool &err) const;

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
  char* dumpPatchedImage(pdstring outFile);//ccw 28 oct 2001
#else
  char* dumpPatchedImage(pdstring outFile) { return NULL; } 
#endif

  bool dumpImage(pdstring outFile);
  
  bool symbol_info(const pdstring &name, Symbol &ret) {
     assert(symbols);
     return symbols->symbol_info(name, ret);
  }

  // This will find the named symbol in the image or in a shared object
  bool getSymbolInfo( const pdstring &name, Symbol &ret );

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

#ifdef INFERIOR_RPC_DEBUG
  void CheckAppTrapIRPCInfo();
#endif

  bool getCurrPCVector(pdvector <Address> &currPCs);

#if defined(os_solaris) && defined(arch_x86)
  bool changeIntReg(int reg, Address addr);
#endif

  bool instrSideEffect(Frame &frame, instPoint *inst);

  void installInstrRequests(const pdvector<instMapping*> &requests);
  void recognize_threads(pdvector<unsigned> *completed_lwps);
  void determineLWPs(pdvector<unsigned> *pids);

  int getPid() const { return pid;}

  bool heapIsOk(const pdvector<sym_data>&);


  /***************************************************************************
   **** Runtime library initialization code (Dyninst)                     ****
   ***************************************************************************/
  bool loadDyninstLib();
  bool setDyninstLibPtr(shared_object *libobj);
  bool setDyninstLibInitParams();
  static void dyninstLibLoadCallback(process *, pdstring libname, shared_object *libobj, void *data);
  bool finalizeDyninstLib();
  
  bool iRPCDyninstInit();
  static void DYNINSTinitCompletionCallback(process *, unsigned /* rpc_id */,
                                            void *data, void *ret);
  
  // Get the list of inferior heaps from:
  bool getInfHeapList(pdvector<heapDescriptor> &infHeaps); // Whole process
  bool getInfHeapList(const image *theImage,
                      pdvector<heapDescriptor> &infHeaps,
                      Address baseAddr); // Single image

  void addInferiorHeap(const image *theImage, Address baseAddr = 0);

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

#if defined(cap_save_the_world)
  pdvector<imageUpdate*> imageUpdates;//ccw 28 oct 2001
  pdvector<imageUpdate*> highmemUpdates;//ccw 20 nov 2001
  pdvector<dataUpdate*>  dataUpdates;//ccw 26 nov 2001
  pdvector<pdstring> loadLibraryCalls;//ccw 14 may 2002 
  pdvector<pdstring> loadLibraryUpdates;//ccw 14 may 2002

  char* saveWorldFindDirectory();

  unsigned int saveWorldSaveSharedLibs(int &mutatedSharedObjectsSize,
                                   unsigned int &dyninst_SharedLibrariesSize,
                                   char* directoryName, unsigned int &count);
  char* saveWorldCreateSharedLibrariesSection(int dyninst_SharedLibrariesSize);

  void saveWorldCreateHighMemSections(pdvector<imageUpdate*> &compactedHighmemUpdates, 
                                      pdvector<imageUpdate*> &highmemUpdates,
                                      void *newElf);
  void saveWorldCreateDataSections(void* ptr);
  void saveWorldAddSharedLibs(void *ptr);//ccw 14 may 2002
  void saveWorldloadLibrary(pdstring tmp) {
     loadLibraryUpdates.push_back(tmp);
  };
  
#if defined(os_aix)
  void addLib(char *lname);//ccw 30 jul 2002
  int requestTextMiniTramp; //ccw 20 jul 2002
#endif // os_aix

#endif // cap_save_the_world

#if !defined(os_windows) 
  //ccw 3 sep 2002
  //These variables are used by UNIX.C during the loading
  //of the runtime libraries.
  //
  bool finishedDYNINSTinit;
  int RPCafterDYNINSTinit;
#endif // os_windows

  bool writeDataSpace(void *inTracedProcess,
                      u_int amount, const void *inSelf);
  bool readDataSpace(const void *inTracedProcess, u_int amount,
                     void *inSelf, bool displayErrMsg);

  bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);
  bool writeTextWord(caddr_t inTracedProcess, int data);

  bool readTextSpace(const void *inTracedProcess, u_int amount,
                     const void *inSelf);

  static bool IndependentLwpControl() {
#if defined(os_linux)
     return true;
#else
     return false;
#endif
  }
  void independentLwpControlInit();

  enum { NoSignal = -1 };   // matches declaration in dyn_lwp.h
  bool continueProc(int signalToContinueWith = NoSignal);

  bool terminateProc();

  // Detach from a process, deleting data to prep for reattaching
  // NOTE: external callers should use this function, not ::detach
  bool detachProcess(const bool leaveRunning);

  private:
    bool detach(const bool leaveRunning);
  public:
  
  // Clear out all dynamically allocated process data
  void deleteProcess();
  ~process();
  bool pause();

  bool replaceFunctionCall(const instPoint *point,const int_function *newFunc);

  bool dumpCore(const pdstring coreFile);
  bool attach();
  // Set whatever OS-level process flags are needed
  bool setProcessFlags();
  bool unsetProcessFlags(); // Counterpart to above
  
  bool isDynamicCallSite(instPoint *callSite); 
  bool getDynamicCallSiteArgs(instPoint *callSite, 
                              pdvector<AstNode *> &args);
                              
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
  
  BPatch_point *findOrCreateBPPoint(BPatch_function *bpfunc, instPoint *ip,
				    BPatch_procedureLocation pointType);
  BPatch_function *findOrCreateBPFunc(int_function* pdfunc, BPatch_module* bpmod = NULL);

  //  
  //  PUBLIC DATA MEMBERS
  //  

  // The BPatch_thread associated with this process
  BPatch_thread *bpatch_thread; 

  // the following 2 vrbles probably belong in a different class:
  static pdstring programName; // the name of paradynd (specifically, argv[0])
  static pdstring pdFlavor;
  pdstring dyninstRT_name; // the filename of the dyninst runtime library

  // These member vrbles should be made private!
  int traceLink;                /* pipe to transfer traces data over */

  // the following 3 are used in perfStream.C
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;

  //removed for output redirection
  //int ioLink;                   /* pipe to transfer stdout/stderr over */

  Address previousSignalAddr_;
  
  bool continueAfterNextStop_;

#if defined(arch_ia64)
  /* Map an aligned address to its multi-tramp */
  dictionary_hash<Address, Address> multiTrampMap;
#endif

  /* map an inst point to its base tramp */
  dictionary_hash<const instPoint*, trampTemplate *> baseMap;   

  /* map a dyninst internal function back to a BPatch_function(per proc) */
  dictionary_hash <int_function*, BPatch_function*> PDFuncToBPFuncMap;

  /* map an address to an instPoint (that's not at entry, call or exit) */
  dictionary_hash<Address, BPatch_point *> instPointMap;

  // More #if defined(uses_traps_and_can't_mask_signals
  // Trap address to base tramp address (for trap instrumentation)
  dictionary_hash<Address, Address> trampTrapMapping;

 #if defined(i386_unknown_linux2_0)
  public:
  Address getVsyscallStart() { return vsyscall_start_; }
  Address getVsyscallEnd() { return vsyscall_end_; }
  Address getVsyscallText() { return vsyscall_text_; } 
  void setVsyscallRange(Address start, Address end) 
    { vsyscall_start_ = start; vsyscall_end_ = end; }
  void *getVsyscallData() { return vsyscall_data_; }
  void setVsyscallData(void *data) { vsyscall_data_ = data; }
  bool readAuxvInfo();
  
 
  private: 
  Address vsyscall_start_;
  Address vsyscall_end_;
  Address vsyscall_text_;
  void *vsyscall_data_;
#endif
  
  private:
  bootstrapState_t bootstrapState;
  bool suppressCont_;

  public:
  // True if we've reached or past a certain state
  bool reachedBootstrapState(bootstrapState_t state) const { return bootstrapState >= state; }
  bool suppressEventConts() { return suppressCont_; } 
  void setSuppressEventConts(bool s) { suppressCont_ = s; }

  // Strictly increments (we never drop back down)
  void setBootstrapState(bootstrapState_t state) {
      // DEBUG
      if (bootstrapState > state)
          cerr << "Warning: attempt to revert bootstrap state from "
               << bootstrapState << " to " << state << endl;
      else
          bootstrapState = state;
  }  
  pdstring getBootstrapStateAsString() const;

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
#if defined(arch_x86)
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
  // Tracing for whatever mechanism we use to notify us of syscalls made
  // by the process (fork/exec/exit)
  syscallNotification *tracedSyscalls_;

// A list of traps inserted at system calls
  pdvector<syscallTrap *> syscallTraps_;
  
  // Trampoline guard location -- actually an addr in the runtime library.
  Address trampGuardAddr_;

  processState status_;         /* running, stopped, etc. */
  
  //
  //  PRIVATE MEMBER FUNCTIONS
  // 


  bool need_to_wait(void) ;
  // The follwing 5 routines are implemented in an arch-specific .C file

 public:

  Address dyninstlib_brk_addr;
  Address main_brk_addr;

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

  pdstring getProcessStatus() const;

  static pdstring tryToFindExecutable(const pdstring &progpath, int pid);
      // os-specific implementation.  Returns empty string on failure.
      // Otherwise, returns a full-path-name for the file.  Tries every
      // trick to determine the full-path-name, even though "progpath" may
      // be unspecified (empty string)

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
  bool registerLoadLibraryCallback(pdstring libname,
                                   loadLibraryCallbackFunc callback,
                                   void *data);
  // And delete the above
  bool unregisterLoadLibraryCallback(pdstring libname);  

  // Run a callback (if appropriate)
  bool runLibraryCallback(pdstring libname, shared_object *libobj);
    
  private:
  // Hashtable of registered callbacks
  dictionary_hash<pdstring, libraryCallback *> loadLibraryCallbacks_;

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
  bool addASharedObject(shared_object *, Address newBaseAddr = 0);

  // return the list of dynamically linked libs
  pdvector<shared_object *> *sharedObjects() { return shared_objects;  } 

  // getMainFunction: returns the main function for this process
  int_function *getMainFunction() const { return mainFunction; }

#if !defined(BPATCH_LIBRARY)
  // findOneFunction: returns the function associated with function "func"
  // and module "mod".  This routine checks both the a.out image and any
  // shared object images for this function.  
  // mcheyney - should return NULL if function is excluded!!!!
  int_function *findOnlyOneFunction(resource *func,resource *mod);
  int_function *findOnlyOneFunction(pdstring func_name, pdstring mod_name);

  //this routine searches for a function in a module.  Note that res is a vector
  // due to gcc emitting duplicate constructors/destructors
  bool findAllFuncsByName(resource *func, resource *mod, pdvector<int_function *> &res);
#endif

 private:
  enum mt_cache_result { not_cached, cached_mt_true, cached_mt_false };
  enum mt_cache_result cached_result;

 public:

  // If true is passed for ignore_if_mt_not_set, then an error won't be
  // initiated if we're unable to determine if the program is multi-threaded.
  // We are unable to determine this if the daemon hasn't yet figured out
  // what libraries are linked against the application.  Currently, we
  // identify an application as being multi-threaded if it is linked against
  // a thread library (eg. libpthreads.a on AIX).  There are cases where we
  // are querying whether the app is multi-threaded, but it can't be
  // determined yet but it also isn't necessary to know.
  bool multithread_capable(bool ignore_if_mt_not_set = false);

  // Do we have the RT-side multithread functions available
  bool multithread_ready(bool ignore_if_mt_not_set = false) {
    if (!multithread_capable(ignore_if_mt_not_set))
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

  
  dyn_thread *STdyn_thread();

  // findOnlyOneFunction: returns the function associated with function 
  // "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function.  It fails if more than one match is found.
  // findOnlyOnefunctionFromAll:  also checks uninstrumentable functions
  int_function *findOnlyOneFunction(const pdstring &func_name) const;

  // findFuncByName: returns function associated with "func_name"
  // This routine checks both the a.out image and any shared object images 
  // for this function
  //int_function *findFuncByName(const pdstring &func_name);

  // And do it, returning a vector if multiple matches
  bool findAllFuncsByName(const pdstring &func_name, pdvector<int_function *> &res);

  // Most find* routines check pretty names first, then check the mangled
  // hashes when a match cannot be found.  Sometimes, however, we want
  // to go right to the mangled hash.
  bool findFuncsByMangled(const pdstring &func_name, pdvector<int_function *> &res);
  // Find the code sequence containing an address
  // Note: fix the name....
  codeRange *findCodeRangeByAddress(Address addr);
  int_function *findFuncByAddr(Address addr);
  
  bool addCodeRange(Address addr, codeRange *codeobj);
  bool deleteCodeRange(Address addr);
    
  // No function is pushed onto return vector if address can't be resolved
  // to a function
  pdvector<int_function *>pcsToFuncs(pdvector<Frame> stackWalk);

  // findModule: returns the module associated with "mod_name" 
  // this routine checks both the a.out image and any shared object 
  // images for this module
  // if check_excluded is true it checks to see if the module is excluded
  // and if it is it returns 0.  If check_excluded is false it doesn't check

  pdmodule *findModule(const pdstring &mod_name);
  // getSymbolInfo:  get symbol info of symbol associated with name n
  // this routine starts looking a.out for symbol and then in shared objects
  // baseAddr is set to the base address of the object containing the symbol
  bool getSymbolInfo(const pdstring &n, Symbol &info, Address &baseAddr) const;

  // getAllFunctions: returns a vector of all functions defined in the
  // a.out and in the shared objects
  pdvector<int_function *> *getAllFunctions();

  // getAllModules: returns a vector of all modules defined in the
  // a.out and in the shared objects
  pdvector<module *> *getAllModules();

  void triggerNormalExitCallback(int exitCode);
  void triggerSignalExitCallback(int signalnum);  
  
  // triggering normal exit callback and cleanup process happen at different
  // times.  if triggerSignalExitCallback is called, this function should
  // also be called at the same time.

  // this function makes no callback to dyninst but only does cleanup work
  void handleProcessExit();

  // getBaseAddress: sets baseAddress to the base address of the 
  // image corresponding to which.  It returns true  if image is mapped
  // in processes address space, otherwise it returns 0
  bool getBaseAddress(const image *which, Address &baseAddress) const;

  // findCallee: finds the function called by the instruction corresponding
  // to the instPoint "instr". If the function call has been bound to an
  // address, then the callee function is returned in "target" and the
  // instPoint "callee" data member is set to pt to callee's int_function.
  // If the function has not yet been bound, then "target" is set to the
  // int_function associated with the name of the target function (this is
  // obtained by the PLT and relocation entries in the image), and the instPoint
  // callee is not set.  If the callee function cannot be found, (e.g. function
  // pointers, or other indirect calls), it returns false.
  // Returns false on error (ex. process doesn't contain this instPoint).
  bool findCallee(instPoint &instr, int_function *&target);

#ifndef BPATCH_LIBRARY
  bool SearchRelocationEntries(const image *owner, instPoint &instr,
                               int_function *&target,
                               Address target_addr, Address base_addr);
#endif

  // findSignalHandler: if signal_handler is 0, then it checks all images
  // associtated with this process for the signal handler function.
  // Otherwise, the signal handler function has already been found
  void findSignalHandler();

  void handleForkEntry();
#if defined(os_aix)
  // Manually copy inferior heaps past the end of text segments
  void copyDanglingMemory(process *child);
#endif
  void handleForkExit(process *child);
  void handleExecEntry(char *arg0);
  void handleExecExit();

  // Generic handler for anything else waiting on a system call
  // Returns true if handling was done
  bool handleSyscallExit(procSignalWhat_t syscall, dyn_lwp *lwp_with_event);
  
  // For platforms where we can't specifically tell if a signal is due to
  // fork or exec and have to guess
  bool nextTrapIsFork;
  bool nextTrapIsExec;

  // For platforms where we can't get the path string from the PID
  // Argument given to exec
  pdstring execPathArg;
  // Full path info
  pdstring execFilePath;
  
  //  wasExeced: returns true is the process did an exec...this is set
  //  in handleExec()
  bool wasExeced(){ return execed_;}
  private:
  bool execed_;  // true if this process does an exec...set in handleExec

  pdvector<instMapping*> tracingRequests;

  /* If we use instrumentation to catch syscalls, the points are stored here */
  instMapping *preForkInst, *postForkInst, *preExecInst, *postExecInst, *preExitInst;

  public:

  // True if we're in the process of an exec (from exec entry until we load
  // the dyninst RT lib)
  bool inExec;
  
  dyn_thread *getThread(unsigned tid);
  dyn_lwp *getLWP(unsigned lwp_id);

  // return NULL if not found
  dyn_lwp *lookupLWP(unsigned lwp_id);

  // This is an lwp which controls the entire process.  This lwp is a
  // fictional lwp in the sense that it isn't associated with a specific lwp
  // in the system.  For both single-threaded and multi-threaded processes,
  // this lwp represents the process as a whole (ie. all of the individual
  // lwps combined).  This lwp will be NULL if no such lwp exists which is
  // the case for multi-threaded linux processes.
  dyn_lwp *getRepresentativeLWP() const {
     return representativeLWP;
  }

  dyn_thread *getInitialThread() const {
     if(threads.size() == 0)
        return NULL;

     return threads[0];
  }

  dyn_thread *createInitialThread();
  dyn_lwp *createRepresentativeLWP();

  // fictional lwps aren't saved in the real_lwps vector
  dyn_lwp *createFictionalLWP(unsigned lwp_id);
  dyn_lwp *createRealLWP(unsigned lwp_id, int lwp_index);

  void deleteLWP(dyn_lwp *lwp_to_delete);


#if defined(os_alpha)
  int waitforRPC(int *status,bool block = false);
#endif
  process *getParent() const {return parent;}

#if defined(hppa1_1_hp_hpux)
  bool freeNotOK;
#endif

 public:

  void processCost(unsigned obsCostLow, timeStamp wallTime, 
                   timeStamp processTime);

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
  codeRangeTree *codeRangesByAddr_;

  int pid;                      /* id of this process */

#ifndef BPATCH_LIBRARY
  // This is being used to avoid time going backwards in
  // getInferiorProcessCPUtime. We can't use a static variable inside this
  // procedure because there is one previous for each process - naim 5/28/97
  rawTime64 previous; 
#endif

public:

  dynamic_linking *getDyn() { return dyn; }

  Address getObservedCostAddr() const { return costAddr_; }
  void updateObservedCostAddr(Address addr) { costAddr_ = addr;}
  
  
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
   bool uninstallMutations();
   bool reinstallMutations();
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

#if defined(os_windows)
   void set_windows_termination_requested(bool val) {
      windows_termination_requested = val;   
   }
   bool get_windows_termination_requested() {
      return windows_termination_requested;
   }

 private:
   bool windows_termination_requested;
#endif // os_windows

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

  bool flushInstructionCache_(void *baseAddr, size_t size); //ccw 25 june 2001
  void clearProcessEvents();

  bool continueProc_(int sig);
  
  // terminateProcStatus_t is defined at the top of this file.
  terminateProcStatus_t terminateProc_();
  bool dumpCore_(const pdstring coreFile);
  bool osDumpImage(const pdstring &imageFileName,  pid_t pid, Address codeOff);

  dyn_lwp *query_for_stopped_lwp();
  dyn_lwp *stop_an_lwp(bool *wasRunning);

  // stops a process
  bool stop_();
  // wait until the process stops
  bool waitUntilStopped();
  // wait until any lwp in a process stops
  dyn_lwp *waitUntilLWPStops();
  
 public:

  // returns true iff ok to operate on the process (attached)
  bool isAttached() const;

  // returns true if the process is stopped (AKA we can operate on it)
  bool isStopped() const;

  // if the process has exited
  bool hasExited() const;


  // Prolly shouldn't be public... but then we need a stack of 
  // access methods. 
  // This is an lwp which controls the entire process.  This lwp is a
  // fictional lwp in the sense that it isn't associated with a specific lwp
  // in the system.  For both single-threaded and multi-threaded processes,
  // this lwp represents the process as a whole (ie. all of the individual
  // lwps combined).  This lwp will be NULL if no such lwp exists which is
  // the case for multi-threaded linux processes.
  dyn_lwp *representativeLWP;
  dictionary_hash<unsigned, dyn_lwp *> real_lwps;

  pdvector<dyn_thread *> threads;   /* threads belonging to this process */

  // /proc platforms
#if defined(cap_proc)
  bool get_entry_syscalls(sysset_t *entry);
  bool get_exit_syscalls(sysset_t *exit);
  bool set_entry_syscalls(sysset_t *entry);
  bool set_exit_syscalls(sysset_t *exit);
#endif  //cap_proc

#if defined(cap_proc_fd)
    bool get_status(pstatus_t *) const;
#endif // cap_proc_fd

  private:
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
  pdvector<int_function *> *all_functions;
  pdvector<module *> *all_modules;

  // these are a restricted set of functions and modules which are those  
  // from the a.out and shared objects that are instrumentable (not excluded 
  // through the mdl "exclude_lib" or "exclude_func" option) 
  // "excluded" now means never able to instrument
  pdvector<module *> *some_modules;  
  pdvector<int_function *> *some_functions; 

  
#if defined(os_linux) && defined(arch_x86)
public:
  void addSignalHandlerAddr(Address a) { signal_restore.push_back(a); }
  
private:
  pdvector<Address> signal_restore;   // addresses of signal context restore functions
                                    // (for stack walking)
#else // !os_linux
  int_function *signal_handler;  // signal handler function (for stack walking)
#endif

  int_function *mainFunction;  // the main function for this process,
                              // this is usually, but not always, "main"

  unsigned LWPstoppedFromForkExit;                                 

  // hasBeenBound: returns true if the runtime linker has bound the
  // function symbol corresponding to the relocation entry in at the address 
  // specified by entry and base_addr.  If it has been bound, then the callee 
  // function is returned in "target_pdf", else it returns false. 
  bool hasBeenBound(const relocationEntry entry, int_function *&target_pdf, 
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

   int invalid_thr_create_msgs;

  public:
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
   // Easy fix: subclass both base tramp and minitramp off the same
   // kind of object which has certain things in common (base address and size)
   struct instPendingDeletion {
     // Stupid duplication...
     miniTrampHandle *oldMini;
     trampTemplate *oldBase;
   };

   // Closely related: deletion of minitramps
   void deleteMiniTramp(miniTrampHandle *delInst);
   bool checkIfMiniTrampAlreadyDeleted(miniTrampHandle *delInst);
   // And the associated deletion of a base tramp
   void deleteBaseTramp(trampTemplate *baseTramp);

/*Address inferiorMalloc(process *p, unsigned size, inferiorHeapType type=anyHeap,
                       Address near_=0, bool *err=NULL);
void inferiorFree(process *p, Address item, const pdvector<addrVecType> &);
*/
// garbage collect instrumentation
   void gcInstrumentation();
   void gcInstrumentation(pdvector<pdvector<Frame> >&stackWalks);
 private:
   pdvector<instPendingDeletion *> pendingGCInstrumentation;
   
#if defined( ia64_unknown_linux2_4 )
	public:
		 unw_addr_space * unwindAddressSpace;
		 void * unwindProcessArg;
#endif

};


process *ll_createProcess(const pdstring file, pdvector<pdstring> *argv, 
			  pdvector<pdstring> *envp,
                          const pdstring dir, int stdin_fd, int stdout_fd,
                          int stderr_fd);

process *ll_attachProcess(const pdstring &progpath, int pid);

bool  AttachToCreatedProcess(int pid, const pdstring &progpath);

bool isInferiorAllocated(process *p, Address block);

#if !defined(os_linux)
inline void process::independentLwpControlInit() { }
#endif

extern pdvector<process *> processVec;

//
// PARADYND_DEBUG_XXX
//
extern int pd_debug_infrpc;
extern int pd_debug_catchup;


#endif
