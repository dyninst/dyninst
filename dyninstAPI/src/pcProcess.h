/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#ifndef PCPROCESS_H
#define PCPROCESS_H
/*
 * pcProcess.h
 *
 * A class that encapsulates a ProcControlAPI Process for the rest of Dyninst.
 */

#include <string>
#include <map>

#include "addressSpace.h"
#include "pcThread.h"
#include "BPatch_hybridAnalysis.h"
#include "inst.h"
#include "codeRange.h"

#include "Symtab.h"

#define RPC_LEAVE_AS_IS 0
#define RPC_RUN_WHEN_DONE 1
#define RPC_STOP_WHEN_DONE 2

typedef enum { vsys_unknown, vsys_unused, vsys_notfound, vsys_found } syscallStatus_t;
typedef enum { noTracing_ts, libcOpenCall_ts, libcOpenRet_ts, libcClose_ts, instrumentLibc_ts, done_ts } traceState_t;

class PCProcess : public AddressSpace {
public:
    // Process creation and control
    static PCProcess *createProcess(const std::string file, pdvector<std::string> *argv,
                                    BPatch_hybridMode &analysisMode,
                                    pdvector<std::string> *envp,
                                    const std::string dir, int stdin_fd, int stdout_fd,
                                    int stderr_fd);

    static PCProcess *attachProcess(const std::string &progpath, int pid,
                                    void *container_proc_,
                                    BPatch_hybridMode &analysisMode);

    bool continueProcess();
    bool stopProcess();
    bool terminateProcess();
    bool detachProcess(bool cont);
    bool dumpCore(const std::string coreFile);

    // Process status
    bool isBootstrapped() const;
    bool isAttached() const; // true if ok to operate on the process
    bool isStopped() const; // true if the process is stopped
    bool isTerminated() const; // true if the process is terminated
    bool hasExited() const; // true if the process has exited
    bool isExecing() const; // true if the process is in the middle of an exec

    // Memory Management
    bool writeDebugDataSpace(void *inTracedProcess, u_int amount,
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
    bool readTextSpace(const void *inTracedProcess, u_int amount,
                       void *inSelf);
    bool readTextWord(const void *inTracedProcess, u_int amount,
                      void *inSelf);

    // Process properties and fields
    PCThread *getInitialThread() const;
    PCThread *getThread(dynthread_t tid);
    void getThreads(pdvector<PCThread* > &threads);

    int getPid() const;
    unsigned getAddressWidth() const;
    bool isMultithreadCapable() const;
    bool wasRunningWhenAttached() const;
    bool wasCreatedViaAttach() const;
    traceState_t getTraceState() const { return noTracing_ts; }
    void setTraceState(traceState_t) {}
    bool isTracingSysCalls() const { return false; }
    void setTraceSysCalls(bool) {}
    unsigned getMemoryPageSize() const;
    bool isRuntimeHeapAddr(Address) const { return false; }
    bool isExploratoryModeOn() const { return false; }

    // Stackwalking
    bool walkStacks(pdvector<pdvector<Frame> > &stackWalks);

    // Instrumentation support
    void getActiveMultiMap(std::map<Address, multiTramp *> &map);
    void updateActiveMultis();
    void addActiveMulti(multiTramp *multi);

    Address inferiorMalloc(unsigned size, inferiorHeapType type,
                           Address near_, bool *err);

    int_function *findActiveFuncByAddr(Address addr);
    bool mappedObjIsDeleted(mapped_object *obj);

    void installInstrRequests(const pdvector<instMapping*> &requests);
    bool uninstallMutations();
    bool reinstallMutations();

    // iRPC interface
    bool postRPC(AstNodePtr action,
                 void *userData,
                 bool runProcessWhenDone,
                 bool lowmem, PCThread *thread,
                 bool synchronous);

    // Hybrid Mode
    BPatch_hybridMode getHybridMode();
    bool setMemoryAccessRights(Address start, Address size, int rights);
    bool getOverwrittenBlocks(std::map<Address, unsigned char *>& overwrittenPages,//input
                              std::map<Address,Address>& overwrittenRegions,//output
                              std::set<bblInstance *> &writtenBBIs); //output
    void updateMappedFile(std::map<Dyninst::Address,unsigned char*>& owPages,
                          std::map<Address,Address> owRegions);
    bool getDeadCodeFuncs(std::set<bblInstance *> &deadBlocks, // input
                          std::set<int_function*> &affectedFuncs, //output
                          std::set<int_function*> &deadFuncs); //output
    bool hideDebugger();

    // Miscellaneuous
    void debugSuicide();
    void deleteThread(dynthread_t tid);
    bool dumpImage(std::string outFile);
    void cleanupProcess();

    // No function is pushed onto return vector if address can't be resolved
    // to a function
    pdvector<int_function *> pcsToFuncs(pdvector<Frame> stackWalk);
    bool isInSignalHandler(long unsigned int &);

    // OS-specific functions
    bool isRunning() const;
    Address setAOutLoadAddress(fileDescriptor &desc);
    void inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                   inferiorHeapType /* type */);
    bool hasPassedMain();

    // Arch-specific functions
    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &entry, 
			   int_function *&target_pdf, Address base_addr);
    bool getSysCallParameters(dyn_saved_regs *regs, long *params, int numparams);
    int getSysCallNumber(dyn_saved_regs *regs);
    long getSysCallReturnValue(dyn_saved_regs *regs);
    Address getSysCallProgramCounter(dyn_saved_regs *regs);
    bool isMmapSysCall(int callnum);
    Offset getMmapLength(int, dyn_saved_regs *regs);
    Address getLibcStartMainParam(PCThread *thread);

    // Stackwalking internals
    Frame preStackWalkInit(Frame startFrame);
    Address getVsyscallText() { return 0; }
    void setVsyscallText(Address) {}
    Address getVsyscallStart() { return 0; }
    Dyninst::SymtabAPI::Symtab *getVsyscallObject() { return NULL; }
    void setVsyscallObject(Dyninst::SymtabAPI::Symtab *) {}
    void setVsyscallRange(Address, Address) {}
    syscallStatus_t getVsyscallStatus() { return vsys_unknown; }
    void setVsyscallStatus(syscallStatus_t) {} 
    Address getVsyscallEnd() { return 0; }
    void addSignalHandler(Address, unsigned) {}
    bool readAuxvInfo() { return false; }

    // XXX
    static int getStopThreadCB_ID(const Address cb);
    Address getTOCoffsetInfo(Address); // power only -- needs refactoring
    Address getTOCoffsetInfo(int_function *);

};

class inferiorRPCinProgress : public codeRange {
public:
    inferiorRPCinProgress() :
        rpc(ProcControlAPI::IRPC::ptr()),
        rpcStartAddr(0),
        rpcResultAddr(0),
        rpcContPostResultAddr(0),
        rpcCompletionAddr(0),
        resultRegister(REG_NULL),
        resultValue(NULL),
        runProcWhenDone(false),
        thr(ProcControlAPI::Thread::ptr()) {};

    virtual Address get_address() const { return 0; }
    virtual unsigned get_size() const { return 0; }
    virtual void *getPtrToInstruction(Address /*addr*/) const { assert(0); return NULL; }

    ProcControlAPI::IRPC::ptr rpc;
    Address rpcStartAddr;
    Address rpcResultAddr;
    Address rpcContPostResultAddr;
    Address rpcCompletionAddr;

    Register resultRegister; // register that contains the return value
    void *resultValue; // Get the result at rpcResultAddr

    bool runProcWhenDone;
    ProcControlAPI::Thread::ptr thr;
};

#endif
