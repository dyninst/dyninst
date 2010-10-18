/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

/* $Id: process.h,v 1.418 2008/06/19 19:53:35 legendre Exp $
 * process.h - interface to manage a process in execution. A process is a kernel
 *   visible unit with a seperate code and data space.  It might not be
 *   the only unit running the code, but it is only one changed when
 *   ptrace updates are applied to the text space.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <assert.h>
#include <string>

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/inst.h" // callWhen
#include "dyninstAPI/src/frame.h"
#if defined(cap_syscall_trap)
#include "dyninstAPI/src/syscalltrap.h"
#endif
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/imageUpdate.h"
#include "dyninstAPI/src/infHeap.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/h/BPatch_hybridAnalysis.h"
#include "debug.h"

#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "common/h/Timer.h"

#include "parseAPI/h/InstructionSource.h"

// Annoying... Solaris has two /proc header files, one for the
// multiple-FD /proc and one for an ioctl-based compatibility /proc.
// /usr/include/procfs.h is the one we want, and /usr/include/sys/procfs.h
// is the ioctl one. This makes it impossible to have a single include
// line. 
#if defined(os_solaris)
#include <procfs.h>
#elif defined(os_aix) || defined(os_osf) || defined(os_irix)
#include <sys/procfs.h>
#endif


#if defined( cap_unwind )
#include <libunwind.h>
#include <libunwind-ptrace.h>
#endif /* defined( cap_unwind ) */

#if defined(os_linux)
#include "common/h/parseauxv.h"
#endif

#if defined(SHM_SAMPLING)
extern unsigned SHARED_SEGMENT_SIZE;
#endif

#define MAX_THREADS 32 //Should match MAX_THREADS in RTcommon.c
extern unsigned activeProcesses; // number of active processes
   // how about just processVec.size() instead?  At least, this should be made
   // a (static) member vrble of class process

typedef enum { unstarted_bs, 
               attached_bs, 
               begun_bs, 
               libcLoaded_bs, 
               initialized_bs, 
               loadingRT_bs, 
               loadedRT_bs, 
               bootstrapped_bs} bootstrapState_t;

typedef enum { terminateFailed, terminateSucceeded, alreadyTerminated } terminateProcStatus_t;

typedef enum { vsys_unknown, vsys_unused, vsys_notfound, vsys_found } syscallStatus_t;

typedef enum { noTracing_ts, libcOpenCall_ts, libcOpenRet_ts, libcClose_ts, instrumentLibc_ts, done_ts } traceState_t;

const int LOAD_DYNINST_BUF_SIZE = 256;

using namespace Dyninst;
//using namespace Dyninst::SymtabAPI;

class instPoint;
class baseTramp;
class miniTramp;

class dyn_thread;
class dyn_lwp;

class Object;
class fileDescriptor;
class image;
class mapped_object;
class mapped_module;
class dynamic_linking;
class int_variable;
class int_function;

class rpcMgr;
class syscallNotification;

class SignalGenerator;

class BPatch_thread;
class BPatch_function;
class BPatch_point;
namespace Dyninst {
   namespace SymtabAPI {
      class Symtab;
   }
}

typedef void (*continueCallback)(timeStamp timeOfCont);

class process : public AddressSpace {
    friend class ptraceKludge;
    friend class dyn_thread;
    friend class dyn_lwp;
    friend Address loadDyninstDll(process *, char Buffer[]);
    friend class SignalGenerator;
    friend class SignalGeneratorCommon;

    //  
    //  PUBLIC MEMBERS FUNCTIONS
    //  
    
 public:
    
    // Default constructor
    process(SignalGenerator *sh_,BPatch_hybridMode mode);

    // Fork constructor
    process(process *parentProc, SignalGenerator *sg_, int iTrace_fd);

    SignalGenerator *getSG() {return sh;}
    // Creation work
    bool setupCreated(int iTraceLink);
    bool setupAttached();
    // Similar case... process execs, we need to clean everything out
    // and reload the runtime library. This is basically creation,
    // just without someone calling us to make sure it happens...
    bool prepareExec(fileDescriptor &desc);
    // Once we're sure an exec is finishing, there's all sorts of work to do.
    bool finishExec();

    // And fork...
    bool setupFork();

    // Shared creation tasks
    bool setupGeneral();

    // And parse/get the a.out
    // Doing this post-attach allows us to one-pass parse things that need it (e.g., AIX)
    bool setAOut(fileDescriptor &desc);
    Address setAOutLoadAddress(fileDescriptor &desc);
    bool setMainFunction();

 protected:  
    bool walkStackFromFrame(Frame currentFrame, // Where to start walking from
                            pdvector<Frame> &stackWalk); // return parameter
    Frame preStackWalkInit(Frame startFrame); //Let's the OS do any needed initialization
 public:
    // Preferred function: returns a stack walk (vector of frames)
    // for each thread in the program
    bool walkStacks(pdvector<pdvector<Frame> > &stackWalks);
    
    // Get a vector of the active frames of all threads in the process
    bool getAllActiveFrames(pdvector<Frame> &activeFrames);

    
    // Is the current address "after" the given instPoint?
    bool triggeredInStackFrame(Frame &frame,
                               instPoint *point,
                               callWhen when,
                               callOrder order);
    
    bool isInSignalHandler(Address addr);
    
    void deleteThread(dynthread_t tid);
    void deleteThread_(dyn_thread *thr);
    bool removeThreadIndexMapping(dynthread_t tid, unsigned index);
    
 public:
    // Current process state
    processState status() const { return status_;}
    
    // State when we attached;
    bool wasRunningWhenAttached() const { return stateWhenAttached_ == running; }
    
    // Are we in the middle of an exec?
    bool execing() const { return inExec_; }
    
    // update the status on the whole process (ie. process state and all lwp
    // states)
    void set_status(processState st, bool global_st = true, bool overrideState = false);
    
    // update the status of the process and one particular lwp in the process
    void set_lwp_status(dyn_lwp *whichLWP, processState st);
    
    // should only be called by dyn_lwp::continueLWP
    void clearCachedRegister();
    
    Address previousSignalAddr() const { return previousSignalAddr_; }
    void setPreviousSignalAddr(Address a) { previousSignalAddr_ = a; }
    std::string getStatusAsString() const; // useful for debug printing etc.
    
    bool checkContinueAfterStop() {
        if( continueAfterNextStop_ ) {
            continueAfterNextStop_ = false;
            return true;
        }
        return false;
    }
    
    void continueAfterNextStop() { continueAfterNextStop_ = true; }
    static process *findProcess(int pid);

#if defined (cap_save_the_world) 
    
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
    char* dumpPatchedImage(std::string outFile);//ccw 28 oct 2001
    
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
    bool prelinkSharedLibrary(string originalLibNameFullPath, char* dirName, Address baseAddr);
#endif
    
#if defined(sparc_sun_solaris2_4)  //ccw 10 mar 2004
    bool dldumpSharedLibrary(std::string dyninstRT_name, char* directoryname);
#endif
    
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) \
 || defined(sparc_sun_solaris2_4)
    char *saveWorldFindNewSharedLibraryName(string originalLibNameFullPath, char* dirName);
    void setPrelinkCommand(char *command);
#endif
    
#endif

#else
#if 0
    char* dumpPatchedImage(std::string /*outFile*/) { return NULL; } 
#endif
#endif

    bool applyMutationsToTextSection(char *textSection, unsigned textAddr, unsigned textSize);
    
    
    bool dumpImage(std::string outFile);
    
    //  SignalHandler::dumpMemory()
    //
    //  Dumps memory +/- nbytes around target addr, to stderr
    //  Used in place of dumpImage() on platforms that cannot dump core
    //  (windows, notably)
    //  Useful for debug output?  (belongs in process class?)
    bool dumpMemory(void *addr, unsigned nbytes);
    
    // Not at all sure we want to use this anymore...
    void overwriteImage( image* /*img */) {
        assert(0);
    }
   
    /* pure virtual AddressSpace implementations */ 
    unsigned getAddressWidth() const; 
    Address offset() const;
    Address length() const;
    Architecture getArch() const;
    
    // this is only used on aix so far - naim
    // And should really be defined in a arch-dependent place, not process.h - bernat
    Address getTOCoffsetInfo(Address);
    Address getTOCoffsetInfo(int_function *);
    
    bool dyninstLibAlreadyLoaded() { return runtime_lib.size() != 0; }
    
    rpcMgr *getRpcMgr() const { return theRpcMgr; }
    
    bool getCurrPCVector(pdvector <Address> &currPCs);
    
#if defined(os_solaris) && (defined(arch_x86) || defined(arch_x86_64))
    bool changeIntReg(int reg, Address addr);
#endif
    
    void installInstrRequests(const pdvector<instMapping*> &requests);

    // Returns false if process exited while recognizing threads
    bool recognize_threads(process *parent = NULL);
    // Get LWP handles from /proc (or as appropriate)
    
    bool determineLWPs(pdvector<unsigned> &lwp_ids);
    
    int getPid() const;
    
    /***************************************************************************
     **** Runtime library initialization code (Dyninst)                     ****
     ***************************************************************************/
    
    bool loadDyninstLib();
    bool setDyninstLibPtr(mapped_object *libobj);
    bool setDyninstLibInitParams();
    bool finalizeDyninstLib();
    
    bool iRPCDyninstInit();
    static int DYNINSTinitCompletionCallback(process *, unsigned /* rpc_id */,
                                             void *data, void *ret);
    bool initMT();
    
    // Get the list of inferior heaps from:
    
    bool getInfHeapList(pdvector<heapDescriptor> &infHeaps); // Whole process
    bool getInfHeapList(mapped_object *theObj,
                        pdvector<heapDescriptor> &infHeaps); // Single mapped object
    
    void addInferiorHeap(mapped_object *obj);
    
    void initInferiorHeap();
    
    /* Find the tramp guard addr and set it */
    bool initTrampGuard();
    void saveWorldData(Address address, int size, const void* src);
    
#if defined(cap_save_the_world)
    
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
    void saveWorldloadLibrary(std::string tmp, void *brk_ptr) {
        loadLibraryUpdates.push_back(tmp);
        loadLibraryBRKs.push_back(brk_ptr);
    };
    
#if defined(os_aix)
    void addLib(char *lname);//ccw 30 jul 2002
#endif // os_aix
    
#endif // cap_save_the_world
    
    
    void writeDebugDataSpace(void *inTracedProcess, u_int amount, 
                             const void *inSelf);
    bool writeDataSpace(void *inTracedProcess,
                        u_int amount, const void *inSelf);
    bool writeDataWord(void *inTracedProcess,
                       u_int amount, const void *inSelf);
    bool readDataSpace(const void *inTracedProcess, u_int amount,
                       void *inSelf, bool displayErrMsg);
    bool readDataWord(const void *inTracedProcess, u_int amount,
                       void *inSelf, bool displayErrMsg);

    bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);
    bool writeTextWord(void *inTracedProcess, u_int amount, const void *inSelf);
#if 0
    bool writeTextWord(caddr_t inTracedProcess, int data);
#endif
    bool readTextSpace(const void *inTracedProcess, u_int amount,
                       void *inSelf);
    bool readTextWord(const void *inTracedProcess, u_int amount,
                      void *inSelf);
    
    static bool IndependentLwpControl() { return INDEPENDENT_LWP_CONTROL; }
    void independentLwpControlInit();
    
    // Internal calls only; this is an asynchronous call that says "run when everyone
    // is finished". BPatch should use the equivalent SignalGeneratorCommon call.
    bool continueProc(int signalToContinueWith = -1);
    
    bool terminateProc();
    
    // Detach from a process, deleting data to prep for reattaching
    // NOTE: external callers should use this function, not ::detach
    bool detachProcess(const bool leaveRunning);
    bool detachForDebugger(const EventRecord &crash_event);
    bool startDebugger();

    
 private:
    bool detach(const bool leaveRunning);
 public:
    
    // Clear out all dynamically allocated process data
    void deleteProcess();
    ~process();
    bool pause();
    
    bool dumpCore(const std::string coreFile);
    bool attach();
    // Set whatever OS-level process flags are needed
    bool setProcessFlags();
    bool unsetProcessFlags(); // Counterpart to above
    
    
    //  
    //  PUBLIC DATA MEMBERS
    //  
    
  ////////////////////////////////////////////////
  // Address to <X> mappings
  ////////////////////////////////////////////////

 public:
  //interface call to Dyninst pass-through debugging
  Address stepi(bool verbose, int lwp);  
  void stepi(int lwp);  
  void stepi();  

 private:
  void print_instrucs(unsigned char *buffer, unsigned size, 
                 bool leave_files);
 public:
  void disass(Address start, Address end);
  void disass(Address start, Address end, bool leave_files);

#if defined( os_linux )
 public:
  Address getVsyscallStart() { return vsyscall_start_; }
  Address getVsyscallEnd() { return vsyscall_end_; }
  Address getVsyscallText() { return vsyscall_text_; } 
  syscallStatus_t getVsyscallStatus() { return vsys_status_; }
  void setVsyscallStatus(syscallStatus_t s) { vsys_status_ = s; }
  void setVsyscallRange(Address start, Address end) 
    { vsyscall_start_ = start; vsyscall_end_ = end; }
  void setVsyscallText(Address addr) { vsyscall_text_ = addr; }
  Dyninst::SymtabAPI::Symtab *getVsyscallObject() { return vsyscall_obj; }
  void setVsyscallObject(Dyninst::SymtabAPI::Symtab *vo) { vsyscall_obj = vo; }
  bool readAuxvInfo(); 
#endif

#if defined(os_windows)
  bool instrumentThreadInitialFunc(int_function *f);
  pdvector<int_function *> initial_thread_functions;
  bool setBeingDebuggedFlag(bool debuggerPresent);
#endif

  public:
  // True if we've reached or past a certain state
  bool reachedBootstrapState(bootstrapState_t state) const { return bootstrapState >= state; }

  std::string getBootstrapStateAsString() const;
  bootstrapState_t getBootstrapState() {return bootstrapState;}

  // Strictly increments (we never drop back down)
  void setBootstrapState(bootstrapState_t state) {
      // DEBUG
      /*
      if (bootstrapState > state)
          cerr << "Warning: attempt to revert bootstrap state from "
          << bootstrapState << " to " << state << endl;
      else
          bootstrapState = state;
      */
      bootstrapState = (state > bootstrapState) ? state : bootstrapState;
      startup_printf("%s[%d]:  setting bootstrap state for process %d (0x%x) to %s\n",
                     FILE__, __LINE__, getPid(), this, getBootstrapStateAsString().c_str());
  }  

  void resetBootstrapState(bootstrapState_t state) {
      // Every so often we need to force this to a particular value
      bootstrapState = state;
  }

  void suppressBPatchCallbacks(bool state) { 
      suppress_bpatch_callbacks_ = state; 
  }


  void warnOfThreadDelete(dyn_thread *thr);
 
 public:

 //Run the mutatee until exit in single-step mode, printing each instruction
 //as it executes.  
 void debugSuicide();
 private:


/////////////////////////////////////////////////////////////////
//  System call trap tracking
/////////////////////////////////////////////////////////////////

#if defined(cap_syscall_trap)
  public:
  // Overloaded: Address for linux-style, syscall # for /proc
  syscallTrap *trapSyscallExitInternal(Address syscall);
  bool clearSyscallTrapInternal(syscallTrap *trappedSyscall);
  
  // Check all traps entered for a match to the syscall
  bool checkTrappedSyscallsInternal(Address syscall);
#endif
    
  private:
  
  // Trampoline guard location -- actually an addr in the runtime library.

  
  //
  //  PRIVATE MEMBER FUNCTIONS
  // 

 public:


  bool loadDYNINSTlib();
#if defined(os_linux)
  // If dlopen is present, use it. Otherwise, call a libc-internal function
  bool loadDYNINSTlib_exported(const char *dlopen_name = NULL);
  bool loadDYNINSTlib_hidden();

  // Unprotect stack if necessary for runtime library loading
  Address tryUnprotectStack(codeGen &buf, Address codeBase);
#endif
  bool loadDYNINSTlibCleanup(dyn_lwp *trappingLWP);
  bool trapDueToDyninstLib(dyn_lwp *trappingLWP);

  // trapAddress is not set on non-NT, we discover it inline
  bool trapAtEntryPointOfMain(dyn_lwp *checkLWP, Address trapAddress = 0);
  bool wasCreatedViaAttach() { return creationMechanism_ == attached_cm; }
  bool wasCreatedViaAttachToCreated() { return creationMechanism_ == attachedToCreated_cm; }
  bool hasPassedMain();

  // This is special, since it's orthogonal to the others. We're forked if
  // the "parent process" is non-null
  bool wasCreatedViaFork() { return parent != NULL; }


  bool insertTrapAtEntryPointOfMain();
  bool handleTrapAtEntryPointOfMain(dyn_lwp *trappingLWP);

  bool shouldSaveFPState();

  static std::string tryToFindExecutable(const std::string &progpath, int pid);
      // os-specific implementation.  Returns empty string on failure.
      // Otherwise, returns a full-path-name for the file.  Tries every
      // trick to determine the full-path-name, even though "progpath" may
      // be unspecified (empty string)

  bool isDynamicallyLinked() const { return mapped_objects.size() > 1; }

  bool decodeIfDueToSharedObjectMapping(EventRecord &);
  bool handleChangeInSharedObjectMapping(EventRecord &);

  private:

  public:

  // getSharedObjects: This routine is called before main() to get and
  // process all shared objects that have been mapped into the process's
  // address space
  bool processSharedObjects();

  // addASharedObject: This routine is called whenever a new shared object
  // has been loaded by the run-time linker after the process starts running
  // It processes the image, creates new resources
  bool addASharedObject(mapped_object *);

  // removeASharedObject: get rid of a shared object; e.g., dlclose
  bool removeASharedObject(mapped_object *);

  // getMainFunction: returns the main function for this process
  int_function *getMainFunction() const { return main_function; }

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
  bool multithread_ready(bool ignore_if_mt_not_set = false);
    
  dyn_thread *STdyn_thread();

  // No function is pushed onto return vector if address can't be resolved
  // to a function
  pdvector<int_function *>pcsToFuncs(pdvector<Frame> stackWalk);

  bool mappedObjIsDeleted(mapped_object *mobj);

  void triggerNormalExitCallback(int exitCode);
  void triggerSignalExitCallback(int signalnum);  
  
  // triggering normal exit callback and cleanup process happen at different
  // times.  if triggerSignalExitCallback is called, this function should
  // also be called at the same time.

  // this function makes no callback to dyninst but only does cleanup work
  bool handleProcessExit();

  // Checks the mapped object for signal handlers
  void findSignalHandler(mapped_object *obj);

#if defined(os_aix)
  // Manually copy inferior heaps past the end of text segments
  void copyDanglingMemory(process *parent);
#endif
  bool handleForkEntry();
  bool handleForkExit(process *child);
  bool handleExecEntry(char *arg0);
  bool handleExecExit(fileDescriptor &desc);

  bool handleStopThread(EventRecord &ev);
  static int getStopThreadCB_ID(const Address cb);



  //////////////////////////////////////////////
  // Begin Exploratory and Defensive mode stuff
  //////////////////////////////////////////////

  // active instrumentation tracking stuff
  int_function *findActiveFuncByAddr(Address addr);

  typedef std::pair<Address, Address> AddrPair;
  typedef std::set<AddrPair> AddrPairSet;
  typedef std::set<Address> AddrSet;

  struct ActiveDefensivePad {
    Address activePC;
    Address padStart;
    bblInstance *callBlock;
    bblInstance *ftBlock;
  ActiveDefensivePad(Address a, Address b, bblInstance *c, bblInstance *d) : 
    activePC(a), padStart(b), callBlock(c), ftBlock(d) {};
  };
  typedef std::list<ActiveDefensivePad> ADPList;
  
  //void getActivePCs(AddrSet &);
  //void getActiveDefensivePads(AddrSet &, ADPList &);
  bool patchPostCallArea(instPoint *point);
private:
  bool generateRequiredPatches(instPoint *callPt, AddrPairSet &);
  void generatePatchBranches(AddrPairSet &);
public:

  // code overwrites 
  bool getOverwrittenBlocks
      ( std::map<Address, unsigned char *>& overwrittenPages,//input
        std::map<Address,Address>& overwrittenRegions,//output
        std::list<bblInstance *> &writtenBBIs);//output
  bool getDeadCode
    ( const std::list<bblInstance*> &owBlocks, // input
      std::set<bblInstance*> &delBlocks, //output: Del(for all f)
      std::map<int_function*,set<bblInstance*> > &elimMap, //output: elimF
      std::list<int_function*> &deadFuncs, //output: DeadF
      std::list<bblInstance*> &newFuncEntries); //output: newF
  unsigned getMemoryPageSize() const { return memoryPageSize_; }

  // synch modified mapped objects with current memory contents
  mapped_object *createObjectNoFile(Address addr);
  void updateCodeBytes
      ( const std::map<Dyninst::Address,unsigned char*>& owPages,
        const std::map<Address,Address> &owRegions );

  // misc
  bool hideDebugger();
  void flushAddressCache_RT(codeRange *flushRange=NULL);
  BPatch_hybridMode getHybridMode() { return analysisMode_; }
  bool isExploratoryModeOn();
  bool isRuntimeHeapAddr(Address addr);

 private:
  BPatch_hybridMode analysisMode_;
  int memoryPageSize_;

  //stopThread instrumentation
  Address stopThreadCtrlTransfer(instPoint* intPoint, Address target);
  Address resolveJumpIntoRuntimeLib(instPoint* srcPoint, Address target);
  static int stopThread_ID_counter;
  static dictionary_hash< Address, unsigned > stopThread_callbacks;

  //// active instrumentation tracking
  //bool isAMcacheValid;
  //std::map<bblInstance*,Address> activeBBIs;
  //std::map<int_function*,std::set<Address>*> am_funcRelocs;

  // runtime library stuff 
  Address RT_address_cache_addr;
  vector<heapItem*> dyninstRT_heaps;
 public:
  /////////////////////////////////////////////
  // End Exploratory and Defensive mode stuff
  /////////////////////////////////////////////


  dyn_thread *getThread(dynthread_t tid);
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

  dyn_lwp *getInitialLwp() const;
  dyn_thread *getInitialThread() const {
     if(threads.size() == 0)
        return NULL;

     return threads[0];
  }

  void updateThreadIndex(dyn_thread *thread, int index);
  dyn_thread *createInitialThread();
  void addThread(dyn_thread *thread);
  dyn_lwp *createRepresentativeLWP();

  // fictional lwps aren't saved in the real_lwps vector
  dyn_lwp *createFictionalLWP(unsigned lwp_id);
  dyn_lwp *createRealLWP(unsigned lwp_id, int lwp_index);

  void deleteLWP(dyn_lwp *lwp_to_delete);

  int maxNumberOfThreads();
#if defined(os_osf)
  int waitforRPC(int *status,bool block = false);
#endif
  const process *getParent() const {return parent;}

 public:

   bool extractBootstrapStruct(DYNINST_bootstrapStruct *);
   bool isBootstrappedYet() const;
  
private:
  // Since we don't define these, 'private' makes sure they're not used:
  process &operator=(const process &); // assign oper

public:

  dynamic_linking *getDyn() { return dyn; }

  // Add a signal handler that wasdetected
  void addSignalHandler(Address addr, unsigned size);
  
  // Used to be ifdefed, now not because of rework
  // Goes right to the multitramp and installs/uninstalls
  // jumps.
   bool uninstallMutations();
   bool reinstallMutations();

  dyn_lwp *query_for_stopped_lwp();
  dyn_lwp *stop_an_lwp(bool *wasRunning);

private:

  bool flushInstructionCache_(void *baseAddr, size_t size); //ccw 25 june 2001

  bool continueProc_(int sig);
  
  // terminateProcStatus_t is defined at the top of this file.
  terminateProcStatus_t terminateProc_();
  bool dumpCore_(const std::string coreFile);
  bool osDumpImage(const std::string &imageFileName,  pid_t pid, Address codeOff);

  // stops a process
  bool stop_(bool waitForStop = true);
  // wait until the process stops
  bool waitUntilStopped();
#if defined (os_linux)
  bool waitUntilLWPStops();
#endif
  
 public:

  dyn_lwp *query_for_running_lwp();

  // returns true iff ok to operate on the process (attached)
  bool isAttached() const;

  // returns true if the process is stopped (AKA we can operate on it)
  bool isStopped() const;

  // if the process has exited
  bool hasExited() const;

  // true if the process is passed main
  bool hasPassedMain() const;

  // Prolly shouldn't be public... but then we need a stack of 
  // access methods. 
  // This is an lwp which controls the entire process.  This lwp is a
  // fictional lwp in the sense that it isn't associated with a specific lwp
  // in the system.  For both single-threaded and multi-threaded processes,
  // this lwp represents the process as a whole (ie. all of the individual
  // lwps combined).  This lwp will be NULL if no such lwp exists which is
  // the case for multi-threaded linux processes.

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

 public:
    const char *getInterpreterName();
    Address getInterpreterBase();
    void setInterpreterName(const char *name);
    void setInterpreterBase(Address newbase);

 public:
    // Needed by instPoint
    // hasBeenBound: returns true if the runtime linker has bound the
    // function symbol corresponding to the relocation entry in at the address 
    // specified by entry and base_addr.  If it has been bound, then the callee 
    // function is returned in "target_pdf", else it returns false. 
    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &entry, int_function *&target_pdf, 
                              Address base_addr) ;
 private:

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

 private:
  static int inferiorMallocCallback(process *proc, unsigned /* rpc_id */,
                                      void *data, void *result);

   void inferiorMallocDynamic(int size, Address lo, Address hi);


 public:
   // Handling of inferior memory management
#if defined(cap_dynamic_heap)
   // platform-specific definition of "near" (i.e. close enough for one-insn jump)
   void inferiorMallocConstraints(Address near_, Address &lo, Address &hi, inferiorHeapType type);
#endif

   
   Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
			  Address near_=0, bool *err=NULL);
   virtual void inferiorFree(Address item);
   virtual bool inferiorRealloc(Address item, unsigned newSize);

   // garbage collect instrumentation

#if defined(cap_garbage_collection)
   void gcInstrumentation();
   void gcInstrumentation(pdvector<pdvector<Frame> >&stackWalks);
#endif

   virtual bool needsPIC();
  ///////////////////////////////////////////////////
  // Process class members
  ///////////////////////////////////////////////////
  int tmp;
  

  ///////////////////////////////
  // Core process data
  ///////////////////////////////
  process *parent;
  SignalGenerator *sh;

  // And deleted...
  pdvector<mapped_object *> deleted_objects;

  // We have to perform particular steps based on how we were started.
  typedef enum { unknown_cm, 
                 created_cm, 
                 attached_cm, 
                 attachedToCreated_cm } creationMechanism_t;

  // Use accessor method that checks before returning...
  creationMechanism_t creationMechanism_; 
  // Users may want to leave a process in the same state as it was;
  // for example, if attached.
  processState stateWhenAttached_;

  int_function *main_function; // Usually, but not always, "main"
  int_function *thread_index_function;
  dyn_thread *lastForkingThread;

  ///////////////////////////////
  // Shared library handling
  ///////////////////////////////
  dynamic_linking *dyn;

  const char *interpreter_name_;
  Address interpreter_base_;

  
  ///////////////////////////////
  // Process control
  ///////////////////////////////
  dyn_lwp *representativeLWP;
  // LWPs are index by their id
  dictionary_hash<unsigned, dyn_lwp *> real_lwps;
  pdvector<dyn_lwp *> lwps_to_delete;

  // Threads are accessed by index.
  int max_number_of_threads;
  pdvector<dyn_thread *> threads;
  Address thread_structs_base;


  dynthread_t mapIndexToTid(int index);
  bool deferredContinueProc;
  Address previousSignalAddr_;
  bool continueAfterNextStop_;
  // Defined in os.h
  processState status_;         /* running, stopped, etc. */
  // And so we don't run into a _lot_ of problems in process control...
  bool exiting_; // Post-exit callback; "we don't care any more"

  //// Exec
  // For platforms where we have to guess what the next signal is caused by.
  // Currently: Linux/ptrace
  bool nextTrapIsExec;
  // More sure then looking at /proc/pid
  std::string execPathArg;		// Argument given to exec
  std::string execFilePath;	// Full path info
  bool inExec_; // Used to be a status vrble, but is orthogonal to running/stopped

  ///////////////////////////////
  // RPCs
  ///////////////////////////////
  rpcMgr *theRpcMgr;


  ///////////////////////////////
  // Save The World
  ///////////////////////////////
  bool collectSaveWorldData;//this is set to collect data for
				//save the world

  pdvector<imageUpdate*> imageUpdates;//ccw 28 oct 2001
  pdvector<imageUpdate*> highmemUpdates;//ccw 20 nov 2001
  pdvector<dataUpdate*>  dataUpdates;//ccw 26 nov 2001
  pdvector<std::string> loadLibraryCalls;//ccw 14 may 2002 
  pdvector<std::string> loadLibraryUpdates;//ccw 14 may 2002
  pdvector<void*> loadLibraryBRKs;
  int requestTextMiniTramp; //ccw 20 jul 2002
	void setRequestTextMiniTramp(int flag){requestTextMiniTramp=flag;};


  // Pipe between mutator and mutatee
  int traceLink;                /* pipe to transfer traces data over */

  // the following 3 are used in perfStream.C
  // Should migrate to process level when we get the LL tracePipe
  char buffer[2048];
  unsigned bufStart;
  unsigned bufEnd;

  // Signal handling!
  codeRangeTree signalHandlerLocations_;

  //////////////////
  // Startup and initialization
  //////////////////
  bootstrapState_t bootstrapState;
  bool suppress_bpatch_callbacks_;
  unsigned char savedCodeBuffer[BYTES_TO_SAVE];
  Address loadDyninstLibAddr;
  dyn_saved_regs *savedRegs;

  Address dyninstlib_brk_addr;
  Address main_brk_addr;

  char * systemPrelinkCommand;

#if defined(os_windows)
  dictionary_hash<Address, unsigned char> main_breaks;
  pdvector<unsigned> cached_lwps;

  // On windows we need to temporarily keep details of process creation in
  // order to handle their debug mechanism. We create the process, then get a message
  // about it (instead of our "pull" mechanism on other platforms). This gives us
  // space to stash the info.
  // Instead of tracking what we need to peel out, we keep the whole thing for later.

  handleT processHandle_;
  handleT mainFileHandle_;
  Address mainFileBase_;

  pdvector<int> continueHandles;
  pdvector<int> continueTypes;

#endif

  ////////////////////
  // Inferior heap
  ////////////////////
  bool inInferiorMallocDynamic; // The oddest recursion problem...

  /////////////////////
  // System call and signal handling
  /////////////////////
  // Tracing for whatever mechanism we use to notify us of syscalls made
  // by the process (fork/exec/exit)
  syscallNotification *tracedSyscalls_;

  // A list of traps inserted at system calls
  pdvector<syscallTrap *> syscallTraps_;

  /////////////////////
  // Instrumentation
  /////////////////////
  pdvector<instMapping *> tracingRequests;
  pdvector<generatedCodeObject *> pendingGCInstrumentation;

  ////////////////////////////////////////////
  // __libc_start_main instrumentation stuff 
  ////////////////////////////////////////////
 public:
  bool decodeStartupSysCalls(EventRecord &ev);
  bool handleTrapAtLibcStartMain(dyn_lwp *trappingLWP);
  bool instrumentLibcStartMain();
  void setTraceSysCalls(bool traceSys);
  void setTraceState(traceState_t state);
  bool getTraceSysCalls() { return traceSysCalls_; }
  traceState_t getTraceState() { return traceState_; }
  Address getlibcstartmain_brk_addr() { return libcstartmain_brk_addr; }

 private:
  bool getSysCallParameters(dyn_saved_regs *regs, long *params, int numparams);
  int getSysCallNumber(dyn_saved_regs *regs);
  long getSysCallReturnValue(dyn_saved_regs *regs);
  Address getSysCallProgramCounter(dyn_saved_regs *regs);
  bool isMmapSysCall(int callnum);
  Offset getMmapLength(int, dyn_saved_regs *regs);
  Address getLibcStartMainParam(dyn_lwp *trappingLWP);
  // regions that are added during syscall tracking phase
  pdvector<Address> mappedRegionStart;
  pdvector<Address> mappedRegionEnd;
  // start addrs of regions munmapped before call to findLibcStartMain
  pdvector<Address> munmappedRegions;
  bool traceSysCalls_;
  int libcHandle_;
  traceState_t traceState_;
  Address libcstartmain_brk_addr;
 public:  
  Address last_single_step;


  ///////////////////////////////
  // Platform-specific
  ///////////////////////////////

#if defined(os_linux)
  //////////////////
  // Linux vsyscall stuff
  //////////////////
  syscallStatus_t vsys_status_; 
  Address vsyscall_start_;
  Address vsyscall_end_;
  Address vsyscall_text_;
  AuxvParser *auxv_parser;
  Dyninst::SymtabAPI::Symtab *vsyscall_obj;

  bool started_stopped;
#endif

  ///////////////////
  // 64-32 bit mapping functions
  //////////////////
  bool readThreadStruct(Address baseAddr, dyninst_thread_t &struc);

};// end class process


process *ll_createProcess(const std::string file, pdvector<std::string> *argv, 
                          BPatch_hybridMode &analysisMode, 
                          pdvector<std::string> *envp,
                          const std::string dir, int stdin_fd, int stdout_fd,
                          int stderr_fd);

process *ll_attachProcess(const std::string &progpath, int pid, 
                          void *container_proc_, 
                          BPatch_hybridMode &analysisMode);

bool isInferiorAllocated(process *p, Address block);

#if !defined(os_linux)
inline void process::independentLwpControlInit() { }
#endif

extern pdvector<process *> processVec;

#define NUMBER_OF_MAIN_POSSIBILITIES 7
extern char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20];

#endif
