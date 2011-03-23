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

#include "pcProcess.h"
#include "pcThread.h"
#include "pcEventHandler.h"
#include "function.h"
#include "os.h"
#include "debug.h"
#include "multiTramp.h"
#include "instPoint.h"
#include "BPatch.h"
#include "mapped_module.h"
#include "baseTramp.h"
#include "registerSpace.h"

#include "common/h/pathName.h"

#include "proccontrol/h/PCErrors.h"

#include "symtabAPI/h/SymtabReader.h"

#include <sstream>

using namespace Dyninst::ProcControlAPI;
using std::map;
using std::vector;
using std::string;
using std::stringstream;

PCProcess *PCProcess::createProcess(const string file, pdvector<string> &argv,
                                    BPatch_hybridMode analysisMode,
                                    pdvector<string> &envp,
                                    const string dir, int stdin_fd, int stdout_fd,
                                    int stderr_fd, PCEventHandler *eventHandler)
{
    // Debugging information
    startup_cerr << "Creating process " << file << " in directory " << dir << endl;

    startup_cerr << "Arguments: (" << argv.size() << ")" << endl;
    for (unsigned a = 0; a < argv.size(); a++)
        startup_cerr << "   " << a << ": " << argv[a] << endl;

    startup_cerr << "Environment: (" << envp.size() << ")" << endl;
    for (unsigned e = 0; e < envp.size(); e++)
        startup_cerr << "   " << e << ": " << envp[e] << endl;

    startup_printf("%s[%d]: stdin: %d, stdout: %d, stderr: %d\n", FILE__, __LINE__,
            stdin_fd, stdout_fd, stderr_fd);

    // Create a full path to the executable
    string path = createExecPath(file, dir);

    std::map<int, int> fdMap;
    redirectFds(stdin_fd, stdout_fd, stderr_fd, fdMap);

    if( !setEnvPreload(envp, path) ) {
        startup_cerr << "Failed to set environment var to preload RT library" << endl;
        return NULL;
    }

    // Create the ProcControl process
    Process::ptr tmpPcProc = Process::createProcess(path, argv, envp, fdMap);

    if( !tmpPcProc ) {
        const char *lastErrMsg = getLastErrorMsg();
        startup_printf("%s[%d]: failed to create process for %s: %s\n", __FILE__,
                __LINE__, file.c_str(), lastErrMsg);
        string msg = string("Failed to create process for ") + file +
           string(": ") + lastErrMsg;
        showErrorCallback(68, msg.c_str());
        return NULL;
    }

    startup_cerr << "Created process " << tmpPcProc->getPid() << endl;

    PCProcess *ret = new PCProcess(tmpPcProc, file, analysisMode, eventHandler);
    assert(ret);
    tmpPcProc->setData(ret);

    if( !ret->bootstrapProcess() ) {
        startup_cerr << "Failed to bootstrap process " << ret->getPid()
                     << ": terminating..." << endl;
        ret->terminateProcess();

        delete ret;
        return NULL;
    }

    return ret;
}

PCProcess *PCProcess::attachProcess(const string &progpath, int pid,
                                    BPatch_hybridMode analysisMode,
                                    PCEventHandler *eventHandler)
{
    startup_cerr << "Attaching to process " << pid << endl;

    Process::ptr tmpPcProc = Process::attachProcess(pid, progpath);

    if( !tmpPcProc ) {
        const char *lastErrMsg = getLastErrorMsg();
        startup_printf("%s[%d]: Failed to attach process %d: %s\n",
                __FILE__, __LINE__, pid, lastErrMsg);
        stringstream msg;
        msg << "Failed to attach to process " << pid << ": " << lastErrMsg;
        showErrorCallback(26, msg.str());
        return NULL;
    }

    startup_cerr << "Attached to process " << tmpPcProc->getPid() << endl;

    PCProcess *ret = new PCProcess(tmpPcProc, analysisMode, eventHandler);
    assert(ret);

    tmpPcProc->setData(ret);

    ret->runningWhenAttached_ = tmpPcProc->allThreadsRunningWhenAttached();
    ret->file_ = tmpPcProc->libraries().getExecutable()->getName();

    if( !ret->bootstrapProcess() ) {
        startup_cerr << "Failed to bootstrap process " << pid 
                     << ": terminating..." << endl;
        ret->terminateProcess();

        delete ret;
        return NULL;
    }

    return ret;
}

PCProcess *PCProcess::setupForkedProcess(PCProcess *parent, Process::ptr pcProc) {
    startup_printf("%s[%d]: setting up forked process %d\n",
            FILE__, __LINE__, pcProc->getPid());

    PCProcess *ret = new PCProcess(parent, pcProc);
    assert(ret);

    pcProc->setData(ret);

    ret->copyAddressSpace(parent);

    // This requires the AddressSpace be copied from the parent
    ret->tracedSyscalls_ = new syscallNotification(parent->tracedSyscalls_, ret);
    ret->irpcTramp_ = new baseTramp(parent->irpcTramp_, ret);

    // Check if RT library exists in child
    if( ret->runtime_lib.size() == 0 ) {
        // Set the RT library name
        if( !ret->getDyninstRTLibName() ) {
            startup_printf("%s[%d]: failed to get Dyninst RT lib name\n",
                    FILE__, __LINE__);
            return false;
        }
        startup_printf("%s[%d]: Got Dyninst RT libname: %s\n", FILE__, __LINE__,
                       ret->dyninstRT_name.c_str());

        for(unsigned i = 0; i < ret->mapped_objects.size(); ++i) {
            const fileDescriptor &desc = ret->mapped_objects[i]->getFileDesc();
            fileDescriptor tmpDesc(ret->dyninstRT_name,
                    desc.code(), desc.data(), true);
            if( desc == tmpDesc ) {
                ret->runtime_lib.insert(ret->mapped_objects[i]);
                break;
            }
        }
    }

    // TODO hybrid mode stuff, pendingGCInstrumentation

    // Copy signal handlers
    pdvector<codeRange *> sigHandlers;
    parent->signalHandlerLocations_.elements(sigHandlers);
    for(unsigned i = 0; i < sigHandlers.size(); ++i) {
        signal_handler_location *oldSig = dynamic_cast<signal_handler_location *>(sigHandlers[i]);
        assert(oldSig);
        signal_handler_location *newSig = new signal_handler_location(*oldSig);
        ret->signalHandlerLocations_.insert(newSig);
    }

    // If required
    if( !ret->copyDanglingMemory(parent) ) {
        startup_printf("%s[%d]: failed to copy dangling memory from parent %d to child %d\n",
                FILE__, __LINE__, parent->getPid(), ret->getPid());
        ret->terminateProcess();

        delete ret;
        return NULL;
    }

    if( !ret->bootstrapProcess() ) {
        startup_cerr << "Failed to bootstrap process " << ret->getPid()
                     << ": terminating..." << endl;
        ret->terminateProcess();

        delete ret;
        return NULL;
    }

    ret->setInEventHandling(true);

    ret->setDesiredProcessState(parent->getDesiredProcessState());

    return ret;
}

PCProcess *PCProcess::setupExecedProcess(PCProcess *oldProc, std::string execPath) {
    BPatch::bpatch->registerExecCleanup(oldProc, NULL);

    PCProcess *newProc = new PCProcess(oldProc->pcProc_, execPath, oldProc->analysisMode_,
            oldProc->eventHandler_);

    oldProc->pcProc_->setData(newProc);
    newProc->setExecing(true);

    if( !newProc->bootstrapProcess() ) {
        proccontrol_printf("%s[%d]: failed to bootstrap execed process %d\n",
                FILE__, __LINE__, newProc->getPid());
        delete newProc;
        return NULL;
    }

    delete oldProc;
    oldProc = NULL;

    newProc->setInEventHandling(true);
    newProc->incPendingEvents();

    BPatch::bpatch->registerExecExit(newProc);

    newProc->setExecing(false);
    newProc->setDesiredProcessState(ps_running);

    return newProc;
}

PCProcess::~PCProcess() {
    if( tracedSyscalls_ ) delete tracedSyscalls_;
    tracedSyscalls_ = NULL;

    if( irpcTramp_ ) delete irpcTramp_;
    irpcTramp_ = NULL;

    signalHandlerLocations_.clear();

    trapMapping.clearTrapMappings();

    for(unsigned i = 0; i < pendingGCInstrumentation_.size(); ++i) {
        if( pendingGCInstrumentation_[i] != NULL ) {
            delete pendingGCInstrumentation_[i];
            pendingGCInstrumentation_[i] = NULL;
        }
    }
    pendingGCInstrumentation_.clear();
}

/***************************************************************************
 **** Runtime library initialization code (Dyninst)                     ****
 ***************************************************************************/

/*
 *
 * Gratuitously large comment. This diagrams the startup flow of
 * messages between the mutator and mutatee. Entry points
 * for create and attach process are both given.
 *     Mutator           Signal              Mutatee
 * Create:
 *     Fork/Exec
 *                     <-- Trap              Halted in exec (handled by ProcControlAPI)
 *     Install trap in main
 *                     <-- Trap              Halted in main
 *  Attach: (also paused, not in main)
 *     Install call to dlopen/
 *     LoadLibrary
 *                     <-- Trap              In library load
 *     Set parameters in library
 *                     <-- Trap              Finished loading
 *     Restore code and leave paused
 *     Finalize library
 *       If finalizing fails, init via iRPC
 */

/*
 * In all cases, the process is left paused at the entry of main
 * (create) or where it was (attach). No permanent instrumentation
 * is inserted.
 */

bool PCProcess::hasReachedBootstrapState(bootstrapState_t state) const {
    return state <= bootstrapState_;
}

void PCProcess::setBootstrapState(bootstrapState_t newState) {
    bootstrapState_ = newState;
}

bool PCProcess::bootstrapProcess() {
    assert( pcProc_->allThreadsStopped() );

    startup_printf("%s[%d]: attempting to bootstrap process %d\n", 
            FILE__, __LINE__, getPid());

    if( !wasCreatedViaFork() ) {
        // Initialize the inferior heaps
        initializeHeap();

        for(unsigned i = 0; i < mapped_objects.size(); ++i) {
            addInferiorHeap(mapped_objects[i]);
        }

        // Create the mapped_objects for the executable and shared libraries
        if( !createInitialMappedObjects() ) {
            startup_printf("%s[%d]: bootstrap failed while creating mapped objects\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    // Create the initial threads
    createInitialThreads();

    // Initialize StackwalkerAPI
    if ( !createStackwalker() )
    {
      startup_printf("Bootstrap failed while initializing Stackwalker\n");
      return false;
    }

    // Insert a breakpoint at the entry point of main (and possibly __libc_start_main)
    if( !hasPassedMain() ) {
        if( !insertBreakpointAtMain() ) {
            startup_printf("%s[%d]: bootstrap failed while setting a breakpoint at main\n",
                    FILE__, __LINE__);
            return false;
        }

        if( !continueProcess() ) {
            startup_printf("%s[%d]: bootstrap failed while continuing the process\n",
                    FILE__, __LINE__);
            return false;
        }

        while( !hasReachedBootstrapState(bs_readyToLoadRTLib) ) {
            if( isStopped() ) {
                if( !continueProcess() ) {
                    startup_printf("%s[%d]: bootstrap failed while continuing the process\n",
                            FILE__, __LINE__);
                    return false;
                }
            }

            if( isTerminated() ) {
                bperr("The process exited during startup.  This is likely due to one "
                      "of two reasons:\n"
                      "A). The application is mis-built and unable to load.  Try "
                      "running the application outside of Dyninst and see if it "
                      "loads properly.\n"
                      "B). libdyninstAPI_RT is mis-built.  Try loading the library "
                      "into another application and see if it reports any errors.  "
                      "Ubuntu users - You may need to rebuild the RT library "
                      "with the DISABLE_STACK_PROT line enabled in "
                      "core/make.config.local");
                startup_printf("%s[%d]: program exited early, never reached "
                               "initialized state\n", FILE__, __LINE__);
                startup_printf("Error is likely due to the application or RT "
                               "library having missing symbols or dependencies\n");
                return false;
            }

            startup_printf("%s[%d]: bootstrap waiting for process to initialize\n",
                    FILE__, __LINE__);
            if( eventHandler_->waitForEvents(true) != PCEventHandler::EventsReceived ) {
                startup_printf("%s[%d]: bootstrap failed to wait for events\n",
                        FILE__, __LINE__);
                return false;
            }
        }
    }else{
        bootstrapState_ = bs_readyToLoadRTLib;
    }
    startup_printf("%s[%d]: process initialized, loading the RT library\n",
            FILE__, __LINE__);

    // Load the RT library
    if( !loadRTLib() ) {
        bperr("Dyninst was unable to load the dyninst runtime library "
              "into the application.  This may be caused by statically "
              "linked executables, or by having dyninst linked against a "
              "different version of libelf than it was built with.");
        startup_printf("%s[%d]: bootstrap failed to load RT library\n",
                FILE__, __LINE__);
        return false;
    }

    pdvector<int_variable *> obsCostVec;
    if( !findVarsByAll("DYNINSTobsCostLow", obsCostVec) ) {
        startup_printf("%s[%d]: failed to find DYNINSTobsCostLow\n",
                FILE__, __LINE__);
        return false;
    }

    costAddr_ = obsCostVec[0]->getAddress();
    assert(costAddr_);

    if( !wasCreatedViaFork() ) {
        // Install system call tracing
        startup_printf("%s[%d]: installing default Dyninst instrumentation into process %d\n", 
            FILE__, __LINE__, getPid());

        tracedSyscalls_ = new syscallNotification(this);

        // TODO 
        // pre-fork and pre-exit should depend on whether a callback is defined
        // 
        // This will require checking whether BPatch holds a defined callback and also
        // adding a way for BPatch enable this instrumentation in all processes when
        // a callback is registered

        if (!tracedSyscalls_->installPreFork()) {
            startup_printf("%s[%d]: failed pre-fork notification setup\n",
                    FILE__, __LINE__);
            return false;
        }

        if (!tracedSyscalls_->installPostFork()) {
            startup_printf("%s[%d]: failed post-fork notification setup\n",
                    FILE__, __LINE__);
            return false;
        }

        if (!tracedSyscalls_->installPreExec()) {
            startup_printf("%s[%d]: failed pre-exec notification setup\n",
                    FILE__, __LINE__);
            return false;
        }

        if (!tracedSyscalls_->installPostExec()) {
            startup_printf("%s[%d]: failed post-exec notification setup\n",
                    FILE__, __LINE__);
            return false;
        }

        if (!tracedSyscalls_->installPreExit()) {
            startup_printf("%s[%d]: failed pre-exit notification setup\n",
                    FILE__, __LINE__);
            return false;
        }

        if (!tracedSyscalls_->installPreLwpExit()) {
            startup_printf("%s[%d]: failed pre-lwp-exit notification setup\n",
                    FILE__, __LINE__);
            return false;
        }

        // Initialize the tramp guard
        startup_printf("%s[%d]: initializing tramp guard\n", FILE__, __LINE__);
        if( !initTrampGuard() ) {
            startup_printf("%s[%d]: failed to initalize tramp guards\n", FILE__, __LINE__);
            return false;
        }

        // Initialize the MT stuff
        if (multithread_capable()) {
            if( !instrumentMTFuncs() ) {
                startup_printf("%s[%d]: Failed to instrument MT funcs\n",
                        FILE__, __LINE__);
                return false;
            }
        }
    }

    // Register the initial threads
    startup_printf("%s[%d]: registering initial threads with RT library\n",
            FILE__, __LINE__);
    vector<pair<dynthread_t, PCThread *> > toUpdate;
    for(map<dynthread_t, PCThread *>::iterator i = threadsByTid_.begin();
            i != threadsByTid_.end(); ++i)
    {
        if( !registerThread(i->second) ) {
            startup_printf("%s[%d]: bootstrap failed while registering threads with RT library\n",
                    FILE__, __LINE__);
            return false;
        }

        // If the information available has improved, update the mapping to reflect this
        if( i->first != i->second->getTid() ) toUpdate.push_back(*i);
    }

    for(vector<pair<dynthread_t, PCThread *> >::iterator i = toUpdate.begin();
            i != toUpdate.end(); ++i)
    {
        threadsByTid_.erase(i->first);
        threadsByTid_.insert(make_pair(i->second->getTid(), i->second));
    }

    startup_printf("%s[%d]: finished registering initial threads with RT library\n",
            FILE__, __LINE__);

    // use heuristics to set hybrid analysis mode
    if (BPatch_heuristicMode == analysisMode_) {
        if (getAOut()->parse_img()->codeObject()->defensiveMode()) {
            analysisMode_ = BPatch_defensiveMode;
        } else {
            analysisMode_ = BPatch_normalMode;
        }
    }

    bootstrapState_ = bs_initialized;
    startup_printf("%s[%d]: finished bootstrapping process %d\n", FILE__, __LINE__, getPid());

    return true;
}

bool PCProcess::initTrampGuard() {
    const std::string vrbleName = "DYNINST_tramp_guards";
    pdvector<int_variable *> vars;
    if (!findVarsByAll(vrbleName, vars)) {
        return false;
    }
    assert(vars.size() == 1);

    Address allocedTrampAddr = 0;

    if (getAddressWidth() == 4) {
        // Don't write directly into trampGuardBase_ as a buffer,
        //   in case we're on a big endian architechture.
        unsigned int value;
        readDataWord((void *)vars[0]->getAddress(), 4, &value, true);
        allocedTrampAddr = value;

    } else if (getAddressWidth() == 8) {
        readDataWord((void *)vars[0]->getAddress(), 8, &allocedTrampAddr, true);
    } else assert(0 && "Incompatible mutatee address width");

    trampGuardBase_ = getAOut()->getDefaultModule()->createVariable("DYNINST_tramp_guard", 
            allocedTrampAddr, getAddressWidth());

    return true;
}

bool PCProcess::createStackwalker()
{
  using namespace Stackwalker;
  ProcDebug *procDebug = NULL;
  StackwalkSymLookup *symLookup = NULL;

  //Set SymbolReaderFactory in Stackwalker
  Walker::setSymbolReader(Dyninst::SymtabAPI::getSymtabReaderFactory());

  // Create ProcessState
  if (NULL == (procDebug = ProcDebug::newProcDebug(pcProc_)))
  {
    startup_printf("Could not create Stackwalker process state\n");
    return false;
  }

  // Create SymbolLookup
  symLookup = new StackwalkSymLookup(this);

  // Create Walker without default steppers
  if (NULL == (stackwalker_ = Walker::newWalker(procDebug,
                                                NULL,
                                                symLookup,
                                                false)))
  {
    startup_printf("Could not create Stackwalker\n");
    return false;
  }

  return createStackwalkerSteppers();
}

void PCProcess::createInitialThreads() {
    ThreadPool &pcThreads = pcProc_->threads();
    initialThread_ = PCThread::createPCThread(this, pcThreads.getInitialThread());
    addThread(initialThread_);

    for(ThreadPool::iterator i = pcThreads.begin(); i != pcThreads.end(); ++i) {
        if( *i == pcThreads.getInitialThread() ) continue;

        // Wait to create threads until they have user thread information available
        if( !(*i)->haveUserThreadInfo() ) continue;

        PCThread *newThr = PCThread::createPCThread(this, *i);
        addThread(newThr);
    }
}

bool PCProcess::createInitialMappedObjects() {
    if( file_.empty() ) {
        startup_printf("%s[%d]: failed to determine executable for process %d\n",
                FILE__, __LINE__, getPid());
        return false;
    }

    // Create the executable mapped object
    fileDescriptor desc;
    startup_printf("%s[%d]: about to getExecFileDescriptor\n", FILE__, __LINE__);
    if( !getExecFileDescriptor(file_, true, desc) ) {
        startup_printf("%s[%d]: failed to find exec descriptor\n", FILE__, __LINE__);
        return false;
    }

    if( !setAOut(desc) ) {
        startup_printf("%s[%d]: failed to setAOut\n", FILE__, __LINE__);
        return false;
    }

    // Set the RT library name
    if( !getDyninstRTLibName() ) {
        startup_printf("%s[%d]: failed to get Dyninst RT lib name\n",
                FILE__, __LINE__);
        return false;
    }
    startup_printf("%s[%d]: Got Dyninst RT libname: %s\n", FILE__, __LINE__,
                   dyninstRT_name.c_str());


    int objCount = 0;
    startup_printf("Processing initial shared objects\n");
    startup_printf("----\n");
    startup_printf("%d: %s (exec)\n", objCount, getAOut()->debugString().c_str());

    // Create mapped objects for any loaded shared libraries
    const LibraryPool &libraries = pcProc_->libraries();
    for(LibraryPool::const_iterator i = libraries.begin(); i != libraries.end(); ++i) {
       // Some platforms don't use the data load address field
       Address dataAddress = (*i)->getLoadAddress();
       if( usesDataLoadAddress() ) dataAddress = (*i)->getDataLoadAddress();
       fileDescriptor tmpDesc((*i)->getName(), (*i)->getLoadAddress(), dataAddress, true);

       dataAddress = (*i)->getLoadAddress();
       if( usesDataLoadAddress() ) dataAddress = (*i)->getDataLoadAddress();
       fileDescriptor rtLibDesc(dyninstRT_name, (*i)->getLoadAddress(),
                           dataAddress, true);

       // Skip the executable
       if( tmpDesc == desc ) continue;


       mapped_object *newObj = mapped_object::createMappedObject(tmpDesc, 
               this, analysisMode_);


       if( newObj == NULL ) {
           startup_printf("%s[%d]: failed to create mapped object for library %s\n",
                   FILE__, __LINE__, (*i)->getName().c_str());
           return false;
       }

       if( rtLibDesc == tmpDesc ) {
           startup_printf("%s[%d]: RT library already loaded, manual loading not necessary\n",
                   FILE__, __LINE__);
           runtime_lib.insert(newObj);
       }

       objCount++;
       startup_printf("%d: %s\n", objCount, newObj->debugString().c_str());

       addASharedObject(newObj);
    }
    startup_printf("----\n");

    return true;
}

// creates an image, creates new resources for a new shared object
// adds it to the collection of mapped_objects
void PCProcess::addASharedObject(mapped_object *newObj) {
    assert(newObj);

    mapped_objects.push_back(newObj);
    addOrigRange(newObj);

    findSignalHandler(newObj);

    startup_printf("%s[%d]: adding shared object %s, addr range 0x%x to 0x%x\n",
            FILE__, __LINE__,
            newObj->fileName().c_str(),
            newObj->getBaseAddress(),
            newObj->getBaseAddress() + newObj->get_size());
    parsing_printf("Adding shared object %s, addr range 0x%x to 0x%x\n",
            newObj->fileName().c_str(),
            newObj->getBaseAddress(),
            newObj->getBaseAddress() + newObj->get_size());

    if( heapInitialized_ ) {
        addInferiorHeap(newObj);
    }else{
        startup_printf("%s[%d]: skipping check for new inferior heaps, heap uninitialized\n",
                                       FILE__, __LINE__);
    }
}

void PCProcess::removeASharedObject(mapped_object *obj) {
    // Remove from mapped_objects list
    for (unsigned j = 0; j < mapped_objects.size(); j++) {
        if (obj == mapped_objects[j]) {
            mapped_objects[j] = mapped_objects.back();
            mapped_objects.pop_back();
            deletedObjects_.push_back(obj);
            break;
        }
    }

    if (runtime_lib.end() != runtime_lib.find(obj)) {
        runtime_lib.erase( runtime_lib.find(obj) );
    }
    proccontrol_printf("Removing shared object %s, addr range 0x%x to 0x%x\n",
                  obj->fileName().c_str(),
                  obj->getBaseAddress(),
                  obj->get_size());

    removeOrigRange(obj);

    // Signal handler...
    // TODO
}

bool PCProcess::setAOut(fileDescriptor &desc) {
    startup_printf("%s[%d]:  enter setAOut\n", FILE__, __LINE__);

    assert(mapped_objects.size() == 0);

    mapped_object *aout = mapped_object::createMappedObject
                          (desc, this, getHybridMode());
    if (!aout) {
        startup_printf("%s[%d]:  fail setAOut\n", FILE__, __LINE__);
        return false;
    }

    mapped_objects.push_back(aout);
    startup_printf("%s[%d]:  setAOut: adding range\n", FILE__, __LINE__);
    addOrigRange(aout);

    startup_printf("%s[%d]:  setAOut: finding signal handler\n", FILE__, __LINE__);
    findSignalHandler(aout);

    // Find main
    startup_printf("%s[%d]:  leave setAOut/setting main\n", FILE__, __LINE__);
    setMainFunction();

    return true;
}

// We keep a vector of all signal handler locations
void PCProcess::findSignalHandler(mapped_object *obj) {
    startup_printf("%s[%d]: findSignalhandler(%p)\n", FILE__, __LINE__, obj);
    assert(obj);

    int_symbol sigSym;
    string signame(SIGNAL_HANDLER);

    startup_printf("%s[%d]: findSignalhandler(%p): gettingSymbolInfo\n", FILE__, __LINE__, obj);
    if (obj->getSymbolInfo(signame, sigSym)) {
        // Symbols often have a size of 0. This b0rks the codeRange code,
        // so override to 1 if this is true...
        unsigned size_to_use = sigSym.getSize();
        if (!size_to_use) size_to_use = 1;

        startup_printf("%s[%d]: findSignalhandler(%p): addingSignalHandler(%p, %d)\n", FILE__, __LINE__, obj, (void *) sigSym.getAddr(), size_to_use);
        addSignalHandler(sigSym.getAddr(), size_to_use);
    }

    startup_printf("%s[%d]: leaving findSignalhandler(%p)\n", FILE__, __LINE__, obj);
}

// Here's the list of functions to look for:
#define NUMBER_OF_MAIN_POSSIBILITIES 7
char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20] = {
    "main",
    "DYNINST_pltMain",
    "_main",
    "WinMain",
    "_WinMain",
    "wWinMain",
    "_wWinMain"
};

void PCProcess::setMainFunction() {
    assert(!main_function_);

    for (unsigned i = 0; i < NUMBER_OF_MAIN_POSSIBILITIES; i++) {
        main_function_ = findOnlyOneFunction(main_function_names[i]);
        if (main_function_) break;
    }
}
 
/*
 * Given an image, add all static heaps inside it
 * (DYNINSTstaticHeap...) to the buffer pool.
 */
void PCProcess::addInferiorHeap(mapped_object *obj) {
    pdvector<heapDescriptor> infHeaps;
    /* Get a list of inferior heaps in the new image */
    if (obj->getInfHeapList(infHeaps)) {
        /* Add the vector to the inferior heap structure */
        for (u_int j=0; j < infHeaps.size(); j++) {
            infmalloc_printf("%s[%d]: adding heap at 0x%lx to 0x%lx, name %s\n",
                             FILE__, __LINE__,
                             infHeaps[j].addr(),
                             infHeaps[j].addr() + infHeaps[j].size(),
                             infHeaps[j].name().c_str());

            // platform-specific check to ignore this heap
            if( skipHeap(infHeaps[j]) ) continue;

            heapItem *h = new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
                                        infHeaps[j].type(), false);

            infmalloc_printf("%s[%d]: Adding heap from 0x%lx - 0x%lx (%d bytes, type %d) from mapped object %s\n",
                             FILE__, __LINE__,
                             infHeaps[j].addr(),
                             infHeaps[j].addr() + infHeaps[j].size(),
                             infHeaps[j].size(),
                             infHeaps[j].type(),
                             obj->fileName().c_str());

            addHeap(h);

            // set rtlib heaps (runtime_lib hasn't been set yet)
            if ( ! obj->fullName().compare( dyninstRT_name ) ) {
                dyninstRT_heaps_.push_back(h);
            }
        }
    }
}

static const unsigned MAX_THREADS = 32; // Should match MAX_THREADS in RTcommon.c

bool PCProcess::loadRTLib() {
    // Check if the RT library has already been loaded
    if( runtime_lib.size() != 0 ) {
        if( !wasCreatedViaFork() ) {
            // Need to still initialize the library
            if( !iRPCDyninstInit() ) return false;
        }

        return true;
    }

    // If not, load it using a iRPC

    // First, generate the code to load the RT lib
    AstNodePtr loadRTAst = createLoadRTAST();
    if( loadRTAst == AstNodePtr() ) {
        startup_printf("%s[%d]: failed to generate code to load RT lib\n", FILE__,
                __LINE__);
        return false;
    }

    // on some platforms, this RPC needs to be run from a specific address range
    Address execAddress = findFunctionToHijack();
    if( !postIRPC(loadRTAst, 
                NULL,  // no user data
                false, // don't run after it is done
                NULL,  // doesn't matter which thread
                true,  // wait for completion
                NULL,  // don't need to check result directly
                false, // internal RPC
                false, // is not a memory allocation RPC
                execAddress) ) 
    {
        startup_printf("%s[%d]: rpc failed to load RT lib\n", FILE__,
                __LINE__);
        return false;
    }

    if( runtime_lib.size() == 0 ) {
        startup_printf("%s[%d]: failed to load RT lib\n", FILE__,
                __LINE__);
        return false;
    }

    startup_printf("%s[%d]: finished running RPC to load RT library\n", FILE__, __LINE__);

    if( !postRTLoadCleanup() ) {
        startup_printf("%s[%d]: failed to perform cleanup after RT library loaded\n",
                FILE__, __LINE__);
        return false;
    }

    // Initialize some variables in the RT lib
    DYNINST_bootstrapStruct bs_record;

    if( !extractBootstrapStruct(&bs_record) || bs_record.event == 0 ) {
        startup_printf("%s[%d]: RT library not initialized, using RPC to initialize\n",
                FILE__, __LINE__);
        if( !iRPCDyninstInit() ) return false;
    }

    return setRTLibInitParams();
}

// Set up the parameters for DYNINSTinit in the RT lib
bool PCProcess::setRTLibInitParams() {
    startup_printf("%s[%d]: welcome to PCProcess::setRTLibInitParams\n",
            FILE__, __LINE__);

    int pid = P_getpid();

    // Cause:
    // 1 = created
    // 2 = forked
    // 3 = attached

    int cause;
    if( createdViaAttach_ ) {
        cause = 3;
    }else{
        cause = 1;
    }

    // Now we write these variables into the following global vrbles
    // in the dyninst library:
    // libdyninstAPI_RT_init_localCause
    // libdyninstAPI_RT_init_localPid

    pdvector<int_variable *> vars;

    if (!findVarsByAll("libdyninstAPI_RT_init_localCause",vars, dyninstRT_name)) {
        if (!findVarsByAll("_libdyninstAPI_RT_init_localCause", vars)) {
            if (!findVarsByAll("libdyninstAPI_RT_init_localCause",vars)) {
                startup_printf("%s[%d]: could not find necessary internal variable\n",
                        FILE__, __LINE__);
                return false;
            }
        }
    }

    assert(vars.size() == 1);
    if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *)&cause)) {
        startup_printf("%s[%d]: writeDataWord failed\n", FILE__, __LINE__);
        return false;
    }
    vars.clear();

    if (!findVarsByAll("libdyninstAPI_RT_init_localPid", vars)) {
        if (!findVarsByAll("_libdyninstAPI_RT_init_localPid", vars)) {
            startup_printf("%s[%d]: could not find necessary internal variable\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    assert(vars.size() == 1);
    if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *)&pid)) {
        startup_printf("%s[%d]: writeDataWord failed\n", FILE__, __LINE__);
        return false;
    }
    vars.clear();

    if (!findVarsByAll("libdyninstAPI_RT_init_maxthreads", vars)) {
        if (!findVarsByAll("_libdyninstAPI_RT_init_maxthreads", vars)) {
            startup_printf("%s[%d]: could not find necessary internal variable\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    unsigned numThreads = MAX_THREADS;
    if( !multithread_capable() ) numThreads = 1;

    assert(vars.size() == 1);
    if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *) &numThreads)) {
        startup_printf("%s[%d]: writeDataWord failed\n", FILE__, __LINE__);
        return false;
    }
    vars.clear();

    if (!findVarsByAll("libdyninstAPI_RT_init_debug_flag", vars)) {
        if (!findVarsByAll("_libdyninstAPI_RT_init_debug_flag", vars)) {
            startup_printf("%s[%d]: could not find necessary internal variable\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    assert(vars.size() == 1);
    if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *) &dyn_debug_rtlib)) {
        startup_printf("%s[%d]: writeDataWord failed\n", FILE__, __LINE__);
        return false;
    }
    vars.clear();
    if (dyn_debug_rtlib) {
        fprintf(stderr, "%s[%d]:  set var in RTlib for debug...\n", FILE__, __LINE__);
    }

    return true;
}

#if !defined(os_vxworks)
bool PCProcess::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record) {
    const std::string vrbleName("DYNINST_bootstrap_info");

    pdvector<int_variable *> bootstrapInfoVec;
    if( !findVarsByAll(vrbleName, bootstrapInfoVec) ) {
        startup_printf("%s[%d]: failed to find bootstrap variable %s\n",
                FILE__, __LINE__, vrbleName.c_str());
        return false;
    }

    if( bootstrapInfoVec.size() > 1 ) {
        startup_printf("%s[%d]: found more than 1 bootstrap struct var, choosing first one\n",
                FILE__, __LINE__);
        return false;
    }

    Address symAddr = bootstrapInfoVec[0]->getAddress();
    if( !readDataSpace((const void *)symAddr, sizeof(*bs_record), bs_record, true) ) {
        startup_printf("%s[%d]: failed to read bootstrap struct in RT library\n",
                FILE__, __LINE__);
        return false;
    }

    return true;
}
#endif

bool PCProcess::iRPCDyninstInit() {
    startup_printf("%s[%d]: running DYNINSTinit via iRPC\n", FILE__, __LINE__);

    extern int dyn_debug_rtlib;
    int pid = P_getpid();
    unsigned maxthreads = MAX_THREADS;
    if( !multithread_capable() ) maxthreads = 1;

    int cause;
    if( createdViaAttach_ ) {
        cause = 3;
    }else{
        cause = 1;
    }

    pdvector<AstNodePtr> the_args(4);
    the_args[0] = AstNode::operandNode(AstNode::Constant, (void*)(Address)cause);
    the_args[1] = AstNode::operandNode(AstNode::Constant, (void*)(Address)pid);
    the_args[2] = AstNode::operandNode(AstNode::Constant, (void*)(Address)maxthreads);
    the_args[3] = AstNode::operandNode(AstNode::Constant, (void*)(Address)dyn_debug_rtlib);
    AstNodePtr dynInit = AstNode::funcCallNode("DYNINSTinit", the_args);

    if( !postIRPC(dynInit, 
                NULL,  // no user data
                false, // don't run after it is done
                NULL,  // doesn't matter which thread
                true,  // wait for completion
                NULL,  // don't need to check result directly
                false) ) // don't deliver callbacks 
    {
        startup_printf("%s[%d]: failed to run DYNINSTinit via iRPC\n", FILE__, __LINE__);
        return false;
    }

    startup_printf("%s[%d]: finished running DYNINSTinit via RPC\n", FILE__, __LINE__);

    return true;
}

#if defined(os_vxworks)
bool PCProcess::insertBreakpointAtMain() {
    // We don't need any extra processing of the RTlib.
    return true;
}
#else
bool PCProcess::insertBreakpointAtMain() {
    if( main_function_ == NULL ) {
        startup_printf("%s[%d]: main function not yet found, cannot insert breakpoint\n",
                FILE__, __LINE__);
    }
    Address addr = main_function_->getAddress();

    // Create the breakpoint
    mainBrkPt_ = Breakpoint::newBreakpoint();
    if( !pcProc_->addBreakpoint(addr, mainBrkPt_) ) {
        startup_printf("%s[%d]: failed to insert a breakpoint at main entry: 0x%x\n",
                FILE__, __LINE__, addr);
        return false;
    }

    startup_printf("%s[%d]: added trap to entry of main, address 0x%x\n", 
            FILE__, __LINE__, addr);

    return true;
}
#endif

bool PCProcess::removeBreakpointAtMain() {
    if( main_function_ == NULL || mainBrkPt_ == Breakpoint::ptr() ) {
        startup_printf("%s[%d]: no breakpoint set at main function, not removing\n",
                FILE__, __LINE__);
        return true;
    }

    Address addr = main_function_->getAddress();

    if( !pcProc_->rmBreakpoint(addr, mainBrkPt_) ) {
        startup_printf("%s[%d]: failed to remove breakpoint at main entry: 0x%x\n",
                FILE__, __LINE__, addr);
        return false;
    }
    mainBrkPt_ = Breakpoint::ptr();

    return true;
}

Breakpoint::ptr PCProcess::getBreakpointAtMain() const {
    return mainBrkPt_;
}

// End Runtime library initialization code

bool PCProcess::continueProcess() {
    proccontrol_printf("%s[%d]: Continuing process %d\n", FILE__, __LINE__, getPid());

    if( !isAttached() || isTerminated() ) {
        bpwarn("Warning: continue attempted on non-attached process\n");
        return false;
    }

    // If the process is in event handling, the process should not be continued, 
    // the processState_t value will be used after event handling to determine the
    // state of the process
    if( isInEventHandling() ) {
        proccontrol_printf("%s[%d]: process currently in event handling, not continuing\n",
                FILE__, __LINE__);
        return true;
    }

    invalidateActiveMultis();

    for(map<dynthread_t, PCThread *>::iterator i = threadsByTid_.begin();
            i != threadsByTid_.end(); ++i)
    {
        i->second->clearStackwalk();
    }

    return pcProc_->continueProc();
}

bool PCProcess::stopProcess() {
    proccontrol_printf("%s[%d]: Stopping process %d\n", FILE__, __LINE__, getPid());

    if( !isAttached() || isTerminated() ) {
        bpwarn("Warning: stop attempted on non-attached process\n");
        return false;
    }

    // See comment in continueProcess about this
    if( isInEventHandling() ) {
        proccontrol_printf("%s[%d]: process currently in event handling, not stopping\n",
                FILE__, __LINE__);
        return true;
    }

    return pcProc_->stopProc();
}

bool PCProcess::terminateProcess() {
    if( isTerminated() ) return true;

    if( !isAttached() ) return false;

    proccontrol_printf("%s[%d]: Terminating process %d\n", FILE__, __LINE__, getPid());
    if( !pcProc_->terminate() ) {
        proccontrol_printf("%s[%d]: Failed to terminate process %d\n", FILE__, __LINE__, 
                getPid());
        return false;
    }
    proccontrol_printf("%s[%d]: finished terminating process %d\n", FILE__, __LINE__, getPid());

    // Handle the event generated by terminate execution
    while( !isExiting() ) {
        if( eventHandler_->waitForEvents(true) != PCEventHandler::EventsReceived ) {
            proccontrol_printf("%s[%d]: failed to wait for completion of terminate event\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    return true;
}

bool PCProcess::detachProcess(bool /*cont*/) {
    if( isTerminated() ) return true;

    if( !isAttached() ) return false;

    // TODO figure out if ProcControl should care about continuing a process
    // after detach
    
    if( pcProc_->detach() ) {
        attached_ = false;
        return true;
    }

    return false;
}

bool PCProcess::isBootstrapped() const {
    return bootstrapState_ == bs_initialized;
}

bool PCProcess::isAttached() const {
    return attached_;
}

bool PCProcess::isStopped() const {
    if( pcProc_ == Process::ptr() ) return true;
    return pcProc_->allThreadsStopped();
}

bool PCProcess::isTerminated() const {
    if( pcProc_ == Process::ptr() ) return true;
    return pcProc_->isTerminated();
}

bool PCProcess::hasExitedNormally() const {
    if( pcProc_ == Process::ptr() ) return true;
    return pcProc_->isExited();
}

bool PCProcess::isExecing() const {
    return execing_;
}

void PCProcess::setExecing(bool b) {
    execing_ = b;
}

bool PCProcess::isExiting() const {
    return exiting_;
}

void PCProcess::setExiting(bool b) {
    exiting_ = b;
}

bool PCProcess::isInEventHandling() const {
    return inEventHandling_;
}

void PCProcess::setInEventHandling(bool b) {
    inEventHandling_ = b;
}

bool PCProcess::hasReportedEvent() const {
    return reportedEvent_;
}

void PCProcess::setReportingEvent(bool b) {
    reportedEvent_ = b;
}

void PCProcess::markExited() {
    pcProc_ = Process::ptr();
}

void PCProcess::writeDebugDataSpace(void *inTracedProcess, u_int amount,
        const void *inSelf)
{
    static unsigned write_no = 0;

    if( !dyn_debug_write ) return;

    write_printf("const unsigned char ");
    switch(getArch()) {
        case Arch_x86:
            write_printf("x86_");
            break;
        case Arch_x86_64:
            write_printf("amd64_");
            break;
        case Arch_ppc32:
        case Arch_ppc64:
            write_printf("power_");
            break;
        default:
            write_printf("unknown_");
            break;
    }
    write_printf("%lx_%d_%u[] = {", inTracedProcess, getPid(), write_no++);

    const unsigned char *buffer = (const unsigned char *)inSelf;
    for(unsigned i = 0; i < amount-1; ++i) {
        if( amount && (i % 10 == 0) ) write_printf("\n");
        write_printf("0x%02hhx, ", buffer[i]);
    }
    if(amount) write_printf("0x%02hhx", buffer[amount-1]);
    write_printf("\n};\n");
}

bool PCProcess::writeDataSpace(void *inTracedProcess,
                    u_int amount, const void *inSelf)
{
    if( isTerminated() ) return false;
    bool result = pcProc_->writeMemory((Address)inTracedProcess, inSelf, amount);

    if( BPatch_defensiveMode == proc()->getHybridMode() && !result ) {
        // the write may have failed because we've removed write permissions
        // from the page, remove them and try again

        int oldRights = setMemoryAccessRights((Address)inTracedProcess,
                amount, PAGE_EXECUTE_READWRITE);
        if( oldRights == PAGE_EXECUTE_READ || oldRights == PAGE_READONLY ) {
            result = pcProc_->writeMemory((Address)inTracedProcess, inSelf, amount);
            if( setMemoryAccessRights((Address)inTracedProcess, amount, oldRights) == -1 ) {
                result = false;
            }
        }else{
            result = false;
        }
    }

    if( result && dyn_debug_write ) writeDebugDataSpace(inTracedProcess, amount, inSelf);

    return result;
}

bool PCProcess::writeDataWord(void *inTracedProcess,
                   u_int amount, const void *inSelf) 
{
    if( isTerminated() ) return false;

    // XXX ProcControlAPI should support word writes in the future
    return pcProc_->writeMemory((Address)inTracedProcess, inSelf, amount);
}

bool PCProcess::readDataSpace(const void *inTracedProcess, u_int amount,
                   void *inSelf, bool displayErrMsg)
{
    if( isTerminated() ) return false;

    bool result = pcProc_->readMemory(inSelf, (Address)inTracedProcess, amount);
    if( !result && displayErrMsg ) {
        stringstream msg;
        msg << "System error: unable to read " << amount << "@" 
            << Address_str((Address)inTracedProcess) << " from process data space: "
            << getLastErrorMsg() << "(pid = " << getPid() << ")";
       showErrorCallback(38, msg.str()); 
    }
    return result;
}

bool PCProcess::readDataWord(const void *inTracedProcess, u_int amount,
                  void *inSelf, bool displayErrMsg)
{
    if( isTerminated() ) return false;

    // XXX see writeDataWord above
    bool result = pcProc_->readMemory(inSelf, (Address)inTracedProcess, amount);
    if( !result && displayErrMsg ) {
        stringstream msg;
        msg << "System error: unable to read " << amount << "@" 
            << Address_str((Address)inTracedProcess) << " from process data space: "
            << getLastErrorMsg() << "(pid = " << getPid() << ")";
       showErrorCallback(38, msg.str());
    }

    return result;
}

bool PCProcess::writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf)
{
    if( isTerminated() ) return false;
    bool result = pcProc_->writeMemory((Address)inTracedProcess, inSelf, amount);

    if( result && dyn_debug_write ) writeDebugDataSpace(inTracedProcess, amount, inSelf);

    return result;
}

bool PCProcess::writeTextWord(void *inTracedProcess, u_int amount, const void *inSelf)
{
    if( isTerminated() ) return false;

    // XXX see writeDataWord above
    bool result = pcProc_->writeMemory((Address)inTracedProcess, inSelf, amount);

    if( result && dyn_debug_write ) writeDebugDataSpace(inTracedProcess, amount, inSelf);

    return result;
}

bool PCProcess::readTextSpace(const void *inTracedProcess, u_int amount,
                   void *inSelf)
{
    if( isTerminated() ) return false;

    return pcProc_->readMemory(inSelf, (Address)inTracedProcess, amount);
}

bool PCProcess::readTextWord(const void *inTracedProcess, u_int amount,
                  void *inSelf)
{
    if( isTerminated() ) return false;

    // XXX see writeDataWord above
    return pcProc_->readMemory(inSelf, (Address)inTracedProcess, amount);
}

PCThread *PCProcess::getInitialThread() const {
    return initialThread_;
}

PCThread *PCProcess::getThread(dynthread_t tid) const {
    map<dynthread_t, PCThread *>::const_iterator findIter;
    findIter = threadsByTid_.find(tid);
    if( findIter == threadsByTid_.end() ) {
        return NULL;
    }

    return findIter->second;
}

bool PCProcess::removeThread(dynthread_t tid) {
    // First, tell the RT library about the removal
    if( !unregisterThread(tid) ) return false;

    map<dynthread_t, PCThread *>::iterator result;
    result = threadsByTid_.find(tid);

    if( result == threadsByTid_.end() ) return false;

    PCThread *toDelete = result->second;

    threadsByTid_.erase(result);

    if( toDelete == initialThread_ ) {
        initialThread_ = NULL;
    }

    toDelete->markExited();

    // Note: don't delete the thread here, the BPatch_thread takes care of it
    proccontrol_printf("%s[%d]: removed thread %lu from process %d\n",
            FILE__, __LINE__, toDelete->getLWP(), getPid());
    return true;
}

bool PCProcess::unregisterThread(dynthread_t tid) {
    pdvector<AstNodePtr> the_args(1);
    the_args[0] = AstNode::operandNode(AstNode::Constant, (void*)(Address)tid);
    AstNodePtr unregisterAST = AstNode::funcCallNode("DYNINSTunregisterThread", the_args);

    Address retval = 0;
    if( !postIRPC(unregisterAST, 
                NULL,  // no user data
                (getDesiredProcessState() == ps_running), 
                NULL,  // doesn't matter which thread
                true,  // wait for completion
                (void **)&retval,
                false) ) // don't deliver callbacks 
    {
        proccontrol_printf("%s[%d]: failed to run DYNINSTunregisterThread via iRPC\n", FILE__, __LINE__);
        return false;
    }

    if( !retval ) {
        proccontrol_printf("%s[%d]: failed to unregister thread with RT library\n", FILE__, __LINE__);
        return false;
    }

    return true;
}

bool PCProcess::registerThread(PCThread *thread) {
    pdvector<AstNodePtr> the_args(2);
    the_args[0] = AstNode::operandNode(AstNode::Constant, (void*)(Address)thread->getTid());
    the_args[1] = AstNode::operandNode(AstNode::Constant, (void*)(Address)thread->getIndex());
    AstNodePtr registerAST = AstNode::funcCallNode("DYNINSTregisterThread", the_args);

    Address retval = 0;
    if( !postIRPC(registerAST, 
                NULL,  // no user data
                (getDesiredProcessState() == ps_running), // don't run after it is done
                NULL,  // doesn't matter which thread
                true,  // wait for completion
                (void **)&retval,
                false) ) // don't deliver callbacks 
    {
        proccontrol_printf("%s[%d]: failed to run DYNINSTregisterThread via iRPC\n", FILE__, __LINE__);
        return false;
    }

    if( !retval ) {
        proccontrol_printf("%s[%d]: failed to register thread with RT library\n", FILE__, __LINE__);
        return false;
    }

    thread->setTid((dynthread_t)retval);

    return true;
}

void PCProcess::addThread(PCThread *thread) {
    pair<map<dynthread_t, PCThread *>::iterator, bool> result;
    result = threadsByTid_.insert(make_pair(thread->getTid(), thread));

    assert( result.second && "Thread already in collection of threads" );
    proccontrol_printf("%s[%d]: added thread %lu to process %d\n",
            FILE__, __LINE__, thread->getLWP(), getPid());
}

void PCProcess::getThreads(vector<PCThread* > &threads) const {
    for(map<dynthread_t, PCThread *>::const_iterator i = threadsByTid_.begin();
            i != threadsByTid_.end(); ++i)
    {
        threads.push_back(i->second);
    }
}

bool PCProcess::wasRunningWhenAttached() const {
    return runningWhenAttached_;
}

bool PCProcess::wasCreatedViaAttach() const {
    return createdViaAttach_;
}

bool PCProcess::wasCreatedViaFork() const {
    return parent_ != NULL;
}

unsigned PCProcess::getMemoryPageSize() const {
    return memoryPageSize_;
}

int PCProcess::getPid() const {
    return savedPid_;
}

int PCProcess::incrementThreadIndex() {
    int ret = curThreadIndex_;
    curThreadIndex_++;
    return ret;
}

unsigned PCProcess::getAddressWidth() const {
    if( mapped_objects.size() > 0 ) {
        return mapped_objects[0]->parse_img()->codeObject()->cs()->getAddressWidth();
    }

    // We can call this before we've attached...best effort guess
    return sizeof(Address);
}

PCEventHandler * PCProcess::getPCEventHandler() const {
    return eventHandler_;
}

bool PCProcess::walkStacks(pdvector<pdvector<Frame> > &stackWalks) {
    bool needToContinue = false;
    bool retval = true;

    // sanity check
    if( stackwalker_ == NULL ) return false;

    // Process needs to be stopped before doing a stackwalk
    if( !isStopped() ) {
        needToContinue = true;
        if( !stopProcess() ) {
            proccontrol_printf("%s[%d]: failed to stop process for stackwalking\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    for(map<dynthread_t, PCThread *>::iterator i = threadsByTid_.begin();
           i != threadsByTid_.end(); ++i)
    {
        PCThread *curThr = i->second;

        pdvector<Frame> stackWalk;
        if( !curThr->walkStack(stackWalk) ) {
            retval = false;
            proccontrol_printf("%s[%d]: failed to walk stack for thread 0x%lx(%d)\n",
                    FILE__, __LINE__,
                    curThr->getTid(), curThr->getLWP());
        }else{
            stackWalks.push_back(stackWalk);
        }
    }

    if( needToContinue ) {
        if( !continueProcess() ) {
            proccontrol_printf("%s[%d]: failed to continue process after performing stackwalking\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    return retval;
}

// Return a vector (possibly with one object) of active frames in the process
bool PCProcess::getAllActiveFrames(pdvector<Frame> &activeFrames) {
    Frame active;
    if( threadsByTid_.size() == 0 ) return false;

    for(map<dynthread_t, PCThread *>::iterator i = threadsByTid_.begin();
            i != threadsByTid_.end(); ++i)
    {
        Frame active = i->second->getActiveFrame();
        if( active == Frame() ) return false;
        activeFrames.push_back(active);
    }

    return true;
}

//
// dynamic inferior heap stuff
//

#if defined(os_vxworks)
#include "vxworks.h"
#define HEAP_DYN_BUF_SIZE (0x4000)
#else
#define HEAP_DYN_BUF_SIZE (0x100000)
#endif

static const Address ADDRESS_LO = ((Address)0);
static const Address ADDRESS_HI = ((Address)~((Address)0));

Address PCProcess::inferiorMalloc(unsigned size, inferiorHeapType type,
                                  Address near_, bool *err) 
{
    enum MallocAttempt {
        AsIs = 0,
        DeferredFree = 1, // compact free blocks
        NewSegment1MBConstrained = 2, // allocate new segment (1 MB, constrained)
        NewSegmentSizedConstrained = 3, // allocate new segment (sized, constrained)
        RemoveRangeConstraints = 4,
        NewSegment1MBUnconstrained = 5,
        NewSegmentSizedUnconstrained = 6,
        DeferredFreeAgain = 7 // why again?
    };

    Address ret = 0;
    if (err) *err = false;

    if( size <= 0 ) {
        infmalloc_printf("%s[%d]: inferior malloc cannot be <= 0\n",
                FILE__, __LINE__);
        if( err ) *err = true;
        return 0;
    }

    // allocation range
    Address lo = ADDRESS_LO; // Should get reset to a more reasonable value
    Address hi = ADDRESS_HI; // Should get reset to a more reasonable value

#if defined(cap_dynamic_heap)
    inferiorMallocAlign(size); // align size
    // Set the lo/hi constraints (if necessary)
    inferiorMallocConstraints(near_, lo, hi, type);
#else
    /* align to cache line size (32 bytes on SPARC) */
    size = (size + 0x1f) & ~0x1f;
#endif

    infmalloc_printf("%s[%d]: inferiorMalloc entered; size %d, type %d, near 0x%lx (0x%lx to 0x%lx)\n",
                     FILE__, __LINE__, size, type, near_, lo, hi);

    // find free memory block (multiple attempts)
    int freeIndex = -1;
    int ntry = 0;
    for (ntry = 0; freeIndex == -1; ntry++) {
        switch(ntry) {
        case AsIs: 
            break;
#if defined(cap_dynamic_heap)
        case DeferredFree: 
            infmalloc_printf("%s[%d]: garbage collecting and compacting\n",
                             FILE__, __LINE__);
            gcInstrumentation();
            inferiorFreeCompact();
            break;
        case NewSegment1MBConstrained: 
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, HEAP_DYN_BUF_SIZE, HEAP_DYN_BUF_SIZE, lo, hi);
            inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
            break;
        case NewSegmentSizedConstrained: 
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, size, size, lo, hi);
            inferiorMallocDynamic(size, lo, hi);
            break;
        case RemoveRangeConstraints: 
            infmalloc_printf("%s[%d]: inferiorMalloc: removing range constraints\n",
                             FILE__, __LINE__);
            lo = ADDRESS_LO;
            hi = ADDRESS_HI;
            if (err) {
                infmalloc_printf("%s[%d]: error in inferiorMalloc\n", FILE__, __LINE__);
                *err = true;
            }
            break;
        case NewSegment1MBUnconstrained: 
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, HEAP_DYN_BUF_SIZE, HEAP_DYN_BUF_SIZE, lo, hi);
            inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
            break;
        case NewSegmentSizedUnconstrained: 
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, size, size, lo, hi);
            inferiorMallocDynamic(size, lo, hi);
            break;
        case DeferredFreeAgain: 
            infmalloc_printf("%s[%d]: inferiorMalloc: recompacting\n", FILE__, __LINE__);
            inferiorFreeCompact();
            break;
#else /* !(cap_dynamic_heap) */
        case DeferredFree: // deferred free, compact free blocks
            gcInstrumentation();
            inferiorFreeCompact();
            break;
#endif /* cap_dynamic_heap */

        default: // error - out of memory
            infmalloc_printf("%s[%d]: failed to allocate memory\n", FILE__, __LINE__);
            if( err ) *err = true;
            return 0;
        }

        ret = inferiorMallocInternal(size, lo, hi, type);
        if (ret) break;
    }
    infmalloc_printf("%s[%d]: inferiorMalloc, returning address 0x%lx\n", FILE__, __LINE__, ret);
    return ret;
}

void PCProcess::inferiorFree(Dyninst::Address item) {
    inferiorFreeInternal(item);
}

bool PCProcess::inferiorRealloc(Dyninst::Address item, unsigned int newSize) {
    return inferiorReallocInternal(item, newSize);
}

static
void alignUp(int &val, int align) {
    assert(val >= 0);
    assert(align >= 0);

    if (val % align != 0) {
        val = ((val / align) + 1) * align;
    }
}

bool PCProcess::inferiorMallocDynamic(int size, Address lo, Address hi) {
    const int MallocFailed = 0;
    const int UnalignedBuffer = -1;

    infmalloc_printf("%s[%d]: entering inferiorMallocDynamic\n", FILE__, __LINE__);

    // word-align buffer size
    // (see "DYNINSTheap_align" in rtinst/src/RTheap-<os>.c)
    alignUp(size, 4);
    // build AstNode for "DYNINSTos_malloc" call
    std::string callee = "DYNINSTos_malloc";
    pdvector<AstNodePtr> args(3);
    args[0] = AstNode::operandNode(AstNode::Constant, (void *)(Address)size);
    args[1] = AstNode::operandNode(AstNode::Constant, (void *)lo);
    args[2] = AstNode::operandNode(AstNode::Constant, (void *)hi);
    AstNodePtr code = AstNode::funcCallNode(callee, args);

    // issue RPC and wait for result
    bool wasRunning = !isStopped();

    proccontrol_printf("%s[%d]: running inferiorMalloc via iRPC on process %d\n",
            FILE__, __LINE__, getPid());

    Address result = 0;
    if( !postIRPC(code,
                  NULL, // only care about the result
                  wasRunning, // run when finished?
                  NULL, // no specific thread
                  true, // wait for completion
                  (void **)&result,
                  false, // internal iRPC
                  true) ) // is a memory allocation RPC
    {
        infmalloc_printf("%s[%d]: failed to post iRPC for inferior malloc\n",
                FILE__, __LINE__);
        return false;
    }
    proccontrol_printf("%s[%d]: inferiorMalloc via iRPC returned 0x%lx\n",
            FILE__, __LINE__, result);

    switch ((int)result) {
        case MallocFailed:
            infmalloc_printf("%s[%d]: DYNINSTos_malloc() failed\n",
                               FILE__, __LINE__);
            return false;
        case UnalignedBuffer:
            infmalloc_printf("%s[%d]: DYNINSTos_malloc(): unaligned buffer size\n",
                               FILE__, __LINE__);
            return false;
        default:
            // add new segment to buffer pool
            heapItem *h = new heapItem(result, size, getDynamicHeapType(),
                    true, HEAPfree);
            addHeap(h);
            break;
    }

    return true;
}

void PCProcess::gcInstrumentation() {
    // The without-a-passed-in-stackwalk version. Walk the stack
    // and pass it down.
    // First, idiot check...
    if (isTerminated()) return;

    if (pendingGCInstrumentation_.size() == 0) return;

    // We need to pause the process. Otherwise we could have an incorrect
    // stack walk
    bool wasPaused = true;
    if (!isStopped()) wasPaused = false;

    if (!wasPaused && !stopProcess()) {
        proccontrol_printf("%s[%d]: failed to stop process to garbage collection instrumentation.\n",
                FILE__, __LINE__);
        return;
    }

    pdvector< pdvector<Frame> > stackWalks;
    if (!walkStacks(stackWalks)) return;

    gcInstrumentation(stackWalks);
    if(!wasPaused) {
        if( !continueProcess() ) {
            proccontrol_printf("%s[%d]: failed to continue process after garbage collecting instrumentation.\n",
                    FILE__, __LINE__);
        }
    }
}

// garbage collect instrumentation
void PCProcess::gcInstrumentation(pdvector<pdvector<Frame> > &stackWalks) {
    // Go through the list and try to clear out any
    // instInstances that are freeable.
    if (isTerminated()) return;

    // This is seriously optimizable -- go by the stack walks first,
    // and label each item as to whether it is deletable or not,
    // then handle them all at once.

    if (pendingGCInstrumentation_.size() == 0) return;

    for (unsigned deletedIter = 0;
            deletedIter < pendingGCInstrumentation_.size();
            deletedIter++) {

        generatedCodeObject *deletedInst = pendingGCInstrumentation_[deletedIter];
        bool safeToDelete = true;

        for (unsigned threadIter = 0;
                threadIter < stackWalks.size();
                threadIter++) {
            pdvector<Frame> stackWalk = stackWalks[threadIter];
            for (unsigned walkIter = 0;
                    walkIter < stackWalk.size();
                    walkIter++) {

                Frame frame = stackWalk[walkIter];
                codeRange *range = frame.getRange();

                if (!range) {
                    // Odd... couldn't find a match at this PC
                    // Do we want to skip GCing in this case? Problem
                    // is, we often see garbage at the end of stack walks.
                    continue;
                }
                safeToDelete = deletedInst->safeToFree(range);

                // If we can't delete, don't bother to continue checking
                if (!safeToDelete)
                    break;
            }
            // Same as above... pop out.
            if (!safeToDelete)
                break;
        }
        if (safeToDelete) {
            // Delete from list of GCs
            // Vector deletion is slow... so copy the last item in the list to
            // the current position. We could also set this one to NULL, but that
            // means the GC vector could get very, very large.

            if (deletedInst->is_multitramp()) {
                mal_printf("garbage collecting multi %p at %lx[%lx %lx] %s[%d]\n",
                           deletedInst, ((multiTramp*)deletedInst)->instAddr(),
                           deletedInst->get_address(),
                           deletedInst->get_address() + deletedInst->get_size(),
                           FILE__,__LINE__);
            } else {
                mal_printf("garbage collecting object %p at [%lx %lx] %s[%d]\n",
                           deletedInst, deletedInst->get_address(),
                           deletedInst->get_address() + deletedInst->get_size(),
                           FILE__,__LINE__);
            }

            pendingGCInstrumentation_[deletedIter] =
                pendingGCInstrumentation_.back();
            // Lop off the last one
            pendingGCInstrumentation_.pop_back();
            // Back up iterator to cover the fresh one
            deletedIter--;
            delete deletedInst;
        }
    }
}

void PCProcess::deleteGeneratedCode(generatedCodeObject *delInst) {
    // Add to the list and deal with it later.
    // The question is then, when to GC. I'd suggest
    // when we try to allocate memory, and leave
    // it a public member that can be called when
    // necessary

    // Make sure we don't double-add
    for (unsigned i = 0; i < pendingGCInstrumentation_.size(); i++)
        if (pendingGCInstrumentation_[i] == delInst)
            return;

    pendingGCInstrumentation_.push_back(delInst);
}

bool PCProcess::uninstallMutations() {
    pdvector<codeRange *> modifiedRanges;
    if (!getModifiedRanges(modifiedRanges)) return false;

    for (unsigned i = 0; i < modifiedRanges.size(); i++) {
        instArea *tmp = dynamic_cast<instArea *>(modifiedRanges[i]);
        if (tmp) {
            multiTramp *multi = tmp->multi;
            multi->disable();
            continue;
        }

        replacedFunctionCall *rfc = dynamic_cast<replacedFunctionCall *>(modifiedRanges[i]);
        if (rfc) {
            if (!writeDataSpace((void *)rfc->callAddr,
                                rfc->oldCall.used(),
                                rfc->oldCall.start_ptr())) 
            {
                proccontrol_printf("%s[%d]: writing memory to uninstall mutations failed\n",
                        FILE__, __LINE__);
            }
            continue;
        }

        functionReplacement *fr = dynamic_cast<functionReplacement *>(modifiedRanges[i]);
        if (fr) {
            // don't handle this yet...
            continue;
        }

        assert(0 && "Unhandled type of modified code in uninstallMutations!");
    }

    return true;
}

bool PCProcess::reinstallMutations() {
    pdvector<codeRange *> modifiedRanges;
    if (!getModifiedRanges(modifiedRanges))
        return false;

    for (unsigned i = 0; i < modifiedRanges.size(); i++) {
        instArea *tmp = dynamic_cast<instArea *>(modifiedRanges[i]);
        if (tmp) {
            multiTramp *multi = tmp->multi;
            multi->enable();
            continue;
        }

        replacedFunctionCall *rfc = dynamic_cast<replacedFunctionCall *>(modifiedRanges[i]);
        if (rfc) {
            if (!writeDataSpace((void *)rfc->callAddr,
                                rfc->newCall.used(),
                                rfc->newCall.start_ptr())) 
            {
                proccontrol_printf("%s[%d]: writing memory to reinstallMutations failed\n",
                        FILE__, __LINE__);
            }
            continue;
        }

        functionReplacement *fr = dynamic_cast<functionReplacement *>(modifiedRanges[i]);
        if (fr) {
            // don't handle this yet...
            continue;
        }

        assert(0 && "Unhandled type of modified code in uninstallMutations!");
    }

    return true;
}

// A copy of the BPatch-level instrumentation installer
void PCProcess::installInstrRequests(const pdvector<instMapping*> &requests) {
    if (requests.size() == 0) {
        return;
    }

    // Instrumentation is now generated on a per-function basis, while
    // the requests are per-inst, not per-function. So
    // accumulate functions, then generate afterwards.

    vector<int_function *> instrumentedFuncs;

    for (unsigned lcv=0; lcv < requests.size(); lcv++) {

        instMapping *req = requests[lcv];
        pdvector<miniTramp *> minis;

        if(!multithread_capable() && req->is_MTonly())
            continue;

        pdvector<int_function *> matchingFuncs;

        if (!findFuncsByAll(req->func, matchingFuncs, req->lib)) {
            inst_printf("%s[%d]: failed to find any functions matching %s (lib %s), returning failure from installInstrRequests\n", 
                    FILE__, __LINE__, req->func.c_str(), req->lib.c_str());
            return;
        }
        else {
            inst_printf("%s[%d]: found %d functions matching %s (lib %s), instrumenting...\n",
                        FILE__, __LINE__, matchingFuncs.size(), req->func.c_str(), req->lib.c_str());
        }

        for (unsigned funcIter = 0; funcIter < matchingFuncs.size(); funcIter++) {
            int_function *func = matchingFuncs[funcIter];
            if (!func) {
                inst_printf("%s[%d]: null int_func detected\n",
                    FILE__,__LINE__);
                continue;  // probably should have a flag telling us whether errors
            }

            // should be silently handled or not
            AstNodePtr ast;
            if ((req->where & FUNC_ARG) && req->args.size()>0) {
                ast = AstNode::funcCallNode(req->inst,
                                            req->args,
                                            this);
            }
            else {
                pdvector<AstNodePtr> def_args;
                def_args.push_back(AstNode::operandNode(AstNode::Constant,
                                                        (void *)0));
                ast = AstNode::funcCallNode(req->inst,
                                            def_args);
            }
            // We mask to strip off the FUNC_ARG bit...
            switch ( ( req->where & 0x7) ) {
            case FUNC_EXIT:
                {
                    const pdvector<instPoint*> func_rets = func->funcExits();
                    for (unsigned j=0; j < func_rets.size(); j++) {
                        miniTramp *mt = func_rets[j]->addInst(ast,
                                                              req->when,
                                                              req->order,
                                                              (!req->useTrampGuard),
                                                              false);
                        if (mt)
                            minis.push_back(mt);
                        else {
                           fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
                        }
                    }
                }
                break;
            case FUNC_ENTRY:
                {
                    const pdvector<instPoint *> func_entries = func->funcEntries();
                    for (unsigned k=0; k < func_entries.size(); k++) {
                        miniTramp *mt = func_entries[k]->addInst(ast,
                                                                 req->when,
                                                                 req->order,
                                                                 (!req->useTrampGuard),
                                                                 false);
                        if (mt)
                            minis.push_back(mt);
                        else {
                           fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
                        }
                    }
                }
                break;
            case FUNC_CALL:
                {
                    pdvector<instPoint*> func_calls = func->funcCalls();
                    for (unsigned l=0; l < func_calls.size(); l++) {
                        miniTramp *mt = func_calls[l]->addInst(ast,
                                                               req->when,
                                                               req->order,
                                                               (!req->useTrampGuard),
                                                               false);
                        if (mt)
                            minis.push_back(mt);
                        else {
                           fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
                        }
                    }
                }
                break;
            default:
                fprintf(stderr, "Unknown where: %d\n",
                        req->where);
            } // switch
            pdvector<instPoint *> failedPoints;
            if (func->performInstrumentation(false, failedPoints)) {
                for (unsigned i = 0; i < minis.size(); i++) {
                    req->miniTramps.push_back(minis[i]);
                }
            }
            minis.clear();
        } // matchingFuncs

    } // requests
    return;
}

static const unsigned MAX_IRPC_SIZE = 0x100000;

bool PCProcess::postIRPC(AstNodePtr action, void *userData, 
        bool runProcessWhenDone, PCThread *thread, bool synchronous,
        void **result, bool userRPC, bool isMemAlloc, Address addr)
{
    if( isTerminated() ) {
        proccontrol_printf("%s[%d]: cannot post RPC to exited or terminated process %d\n",
                FILE__, __LINE__, getpid());
        return false;
    }

    if( thread && !thread->isLive() ) {
        proccontrol_printf("%s[%d]: attempted to post RPC to dead thread %d\n",
                FILE__, __LINE__, thread->getLWP());
        return false;
    }

    // The process needs to be stopped for code gen because memory may be
    // read or written
    bool tempStop = false;
    if( !isStopped() ) {
        tempStop = true;
        if( !stopProcess() ) {
            proccontrol_printf("%s[%d]: failed to stop process for code gen\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    // Default to initial thread
    if( thread == NULL ) thread = initialThread_;

    inferiorRPCinProgress *newRPC = new inferiorRPCinProgress;
    newRPC->runProcWhenDone = runProcessWhenDone;
    newRPC->deliverCallbacks = userRPC;
    newRPC->userData = userData;
    newRPC->synchronous = synchronous;

    // Generate the code for the iRPC
    codeGen irpcBuf(MAX_IRPC_SIZE);
    irpcBuf.setAddrSpace(this);
    irpcBuf.setRegisterSpace(registerSpace::irpcRegSpace(proc()));
    irpcBuf.beginTrackRegDefs();
    irpcBuf.setThread(thread);

    // Emit the header for the iRPC, if necessary

#if defined(bug_syscall_changepc_rewind)
    // Reported by SGI, during attach to a process in a system call:

    // Insert eight NOP instructions before the actual call to dlopen(). Loading
    // the runtime library when the mutatee was in a system call will sometimes
    // cause the process to (on IA32 anyway) execute the instruction four bytes
    // PREVIOUS to the PC we actually set here. No idea why. Prepending the
    // actual dlopen() call with eight NOP instructions insures this doesn't
    // really matter. Eight was selected rather than four because I don't know
    // if x86-64 does the same thing (and jumps eight bytes instead of four).

    // We will put in <addr width> rather than always 8; this will be 4 on x86 and
    // 32-bit AMD64, and 8 on 64-bit AMD64.
    irpcBuf.fill(proc()->getAddressWidth(), codeGen::cgNOP);
#endif

    // Create a stack frame for the RPC
    if( !irpcTramp_->generateSaves(irpcBuf, irpcBuf.rs(), NULL) ) {
        proccontrol_printf("%s[%d]: failed to generate saves via baseTramp\n",
                FILE__, __LINE__);
        delete newRPC;
        return false;
    }

    newRPC->resultRegister = REG_NULL;
    if( !action->generateCode(irpcBuf, false, newRPC->resultRegister) ) {
        proccontrol_printf("%s[%d]: failed to generate code from AST\n",
                FILE__, __LINE__);
        delete newRPC;
        return false;
    }

    // Note: we should not do a corresponding baseTramp restore here:
    // 1) It isn't necessary because ProcControl will restore the
    //    registers
    // 2) We need to be able to read registers to get the result of the iRPC
    //    If we restore, we can't do that

    // Emit the trailer for the iRPC

    // breakOffset: where the irpc ends
    unsigned breakOffset = irpcBuf.used();
    insnCodeGen::generateTrap(irpcBuf);
    insnCodeGen::generateTrap(irpcBuf);

    irpcBuf.endTrackRegDefs();

    // Create the iRPC at the ProcControl level
    if( addr == 0 ) {
        bool err = false;
        if( isMemAlloc ) {
            // This assumes that there will always be space
            addr = inferiorMalloc(irpcBuf.used(), lowmemHeap, 0, &err);
        }else{
            // recursive RPCs are okay when this isn't an inferiorMalloc RPC
            addr = inferiorMalloc(irpcBuf.used(), anyHeap, 0, &err);
        }

        if( err ) {
            proccontrol_printf("%s[%d]: failed to allocate memory for RPC\n",
                    FILE__, __LINE__);
            delete newRPC;
            return false;
        }
        newRPC->memoryAllocated = true;
    }
    newRPC->rpc = IRPC::createIRPC(irpcBuf.start_ptr(), irpcBuf.used(), addr, !synchronous);

    newRPC->rpc->setData(newRPC);

    // sync RPCs posted from a callback (or while doing event handling) require
    // special handling to avoid problems with recursive event handling
    if( synchronous && isInEventHandling() ) {
        eventHandler_->registerCallbackRPC(newRPC);
    }

    // Post the iRPC
    newRPC->thread = thread;
    if( !thread->postIRPC(newRPC) ) {
        proccontrol_printf("%s[%d]: failed to post RPC to thread %d\n",
                FILE__, __LINE__, thread->getLWP());
        delete newRPC;
        return false;
    }

    // Fill in the rest of inferiorRPCinProgress, now that the address of the RPC is known
    newRPC->rpcStartAddr = newRPC->get_address();
    newRPC->rpcCompletionAddr = newRPC->get_address() + breakOffset;

#if defined(bug_syscall_changepc_rewind)
    // Some Linux kernels have the following behavior:
    // Process is in a system call;
    // We interrupt the system call;
    // We say "change PC to address N"
    // The kernel helpfully changes it to (N - address width)
    // The program crashes
    // See a more complete comment above.
    // For now, we pad the start of our code with NOOPS and change to just
    // after those; if we hit rewind behavior, then we're executing safe code.
    newRPC->rpcStartAddr += proc()->getAddressWidth();
#endif

    newRPC->rpc->setStartOffset(newRPC->rpcStartAddr - newRPC->get_address());
    
    proccontrol_printf("%s[%d]: created iRPC %lu on thread %d/%d, base = 0x%lx, start = 0x%lx, complete = 0x%lx\n",
            FILE__, __LINE__, newRPC->rpc->getID(), getPid(), thread->getLWP(),
            newRPC->get_address(), newRPC->rpcStartAddr,
            newRPC->rpcCompletionAddr);

    if( tempStop ) {
        if( !continueProcess() ) {
            proccontrol_printf("%s[%d]: failed to continue process after code gen\n",
                    FILE__, __LINE__);
            delete newRPC;
            return false;
        }
    }

    /*
    // Store the range, use removeOrigRange on completion

    // Only store the range if it doesn't overlap with an existing range ( when
    // loading the RT library on Linux, we need to execute the iRPC from within
    // libc, thus the code range for libc and the iRPC overlap)
    if( findOrigByAddr(newRPC->get_address()) == NULL ) {
        addOrigRange(newRPC);
    }
    */

    if( synchronous ) {
        // Ensure that the process runs until the RPC is completed
        setDesiredProcessState(ps_running);
        addSyncRPCThread(thread);

        if( !thread->isRunning() ) {
            proccontrol_printf("%s[%d]: thread %d/%d not running, continuing thread to run RPC\n",
                    FILE__, __LINE__, getPid(), thread->getLWP());
            if( !thread->continueThread() ) {
                proccontrol_printf("%s[%d]: failed to continue thread %lu, process %d to run RPC\n",
                        FILE__, __LINE__, thread->getLWP(), getPid());
                // removeOrigRange(newRPC);
                delete newRPC;
                return false;
            }
        }

        while( !newRPC->isComplete ) {
            if( !thread->isLive() ) {
                proccontrol_printf("%s[%d]: thread %d/%d no longer exists, failed to finish RPC\n",
                        FILE__, __LINE__, getPid(), thread->getLWP());
                // removeOrigRange(newRPC);
                delete newRPC;
                return false;
            }

            if( !isInEventHandling() ) {
                proccontrol_printf("%s[%d]: waiting for the RPC to complete\n",
                        FILE__, __LINE__);
                // This implicitly does the necessary handling for the completion of the iRPC
                if( eventHandler_->waitForEvents(true) != PCEventHandler::EventsReceived ) {
                    proccontrol_printf("%s[%d]: failed to wait for completion of iRPC\n",
                            FILE__, __LINE__, thread->getLWP(), getPid());
                    // removeOrigRange(newRPC);
                    delete newRPC;
                    return false;
                }
            }else{
                proccontrol_printf("%s[%d]: waiting for the callback RPC to complete\n",
                        FILE__, __LINE__);
                if( eventHandler_->waitForCallbackRPC() != PCEventHandler::EventsReceived ) {
                    proccontrol_printf("%s[%d]: failed to wait for completion of iRPC\n",
                            FILE__, __LINE__, thread->getLWP(), getPid());
                    delete newRPC;
                    return false;
                }
            }
        }

        if( result ) {
            *result = newRPC->returnValue;
        }
    }

    return true;
}

BPatch_hybridMode PCProcess::getHybridMode() {
    return BPatch_normalMode;
}

bool PCProcess::isExploratoryModeOn() const {
    return BPatch_exploratoryMode == analysisMode_ ||
           BPatch_defensiveMode   == analysisMode_;
}

bool PCProcess::isRuntimeHeapAddr(Address addr) const {
    for (unsigned hidx=0; hidx < dyninstRT_heaps_.size(); hidx++) {
        if (addr >= dyninstRT_heaps_[hidx]->addr &&
            addr < dyninstRT_heaps_[hidx]->addr + dyninstRT_heaps_[hidx]->length) {
            return true;
        }
    }
    return false;
}

/* returns true if blocks were overwritten, initializes overwritten
 * blocks and ranges by contrasting shadow pages with current memory
 * contents
 * 1. reads shadow pages in from memory
 * 2. constructs overwritten region list
 * 3. constructs overwrittn basic block list
 * 4. determines if the last of the blocks has an abrupt end, in which
 *    case it marks it as overwritten
 */
bool PCProcess::getOverwrittenBlocks
( std::map<Address, unsigned char *>& overwrittenPages,//input
  std::map<Address,Address>& overwrittenRanges,//output
  std::set<bblInstance *> &writtenBBIs)//output
{
    const unsigned MEM_PAGE_SIZE = getMemoryPageSize();
    unsigned char * memVersion = (unsigned char *) ::malloc(MEM_PAGE_SIZE);
    Address regionStart;
    bool foundStart = false;
    map<Address, unsigned char*>::iterator pIter = overwrittenPages.begin();
    set<mapped_object*> owObjs;
    for (; pIter != overwrittenPages.end(); pIter++) {
        Address curPageAddr = (*pIter).first / MEM_PAGE_SIZE * MEM_PAGE_SIZE;
        unsigned char *curShadow = (*pIter).second;

        // 0. check to make sure curShadow is non-null, if it is null,
        //    that means it hasn't been written to
        if ( ! curShadow ) {
            continue;
        }

        mapped_object* obj = findObject(curPageAddr);
        if (owObjs.end() != owObjs.find(obj)) {
            obj->clearUpdatedRegions();
        }

        // 1. Read the modified page in from memory
        readTextSpace((void*)curPageAddr, MEM_PAGE_SIZE, memVersion);

        // 2. Compare modified page to shadow copy, construct overwritten region list
        for (unsigned mIdx = 0; mIdx < MEM_PAGE_SIZE; mIdx++) {
            if ( ! foundStart && curShadow[mIdx] != memVersion[mIdx] ) {
                foundStart = true;
                regionStart = curPageAddr+mIdx;
            } else if (foundStart && curShadow[mIdx] == memVersion[mIdx]) {
                foundStart = false;
                overwrittenRanges[regionStart] = curPageAddr+mIdx;
            }
        }
        if (foundStart) {
            foundStart = false;
            overwrittenRanges[regionStart] = curPageAddr+MEM_PAGE_SIZE;
        }
    }

    std::map<Address,Address>::iterator rIter = overwrittenRanges.begin();
    std::vector<bblInstance*> curBBIs;
    while (rIter != overwrittenRanges.end()) {
        mapped_object *curObject = findObject((*rIter).first);

        // 3. Determine which basic blocks have been overwritten
        curObject->findBBIsByRange((*rIter).first,(*rIter).second,curBBIs);
        if (curBBIs.size()) {
            mal_printf("overwrote %d blocks in range %lx %lx \n",
                       curBBIs.size(),(*rIter).first,(*rIter).second);
            writtenBBIs.insert(curBBIs.begin(),curBBIs.end());
        }

        // 4. determine if the last of the blocks has an abrupt end, in which
        //    case, mark it as overwritten
        if (    curBBIs.size()
             && ! curObject->proc()->isCode((*rIter).second - 1) )
        {
            bblInstance *lastBBI = curBBIs.back();
            int_function *lastFunc = lastBBI->func();
            const set<instPoint*> abruptEnds = lastFunc->funcAbruptEnds();
            set<instPoint*>::const_iterator aIter = abruptEnds.begin();
            while (aIter != abruptEnds.end()) {
                if (lastBBI->block() == (*aIter)->block()) {
                    writtenBBIs.insert(lastBBI);
                    break;
                }
                aIter++;
            }
        }

        curBBIs.clear();
        rIter++;
    }

    free(memVersion);
    if (writtenBBIs.size()) {
        return true;
    } else {
        return false;
    }
}

// distribute the work to mapped_objects
// currently asserts if there are overwrites to multiple objects
void PCProcess::updateMappedFile
    ( std::map<Dyninst::Address,unsigned char*>& owPages, //input
      std::map<Address,Address> owRanges )
{
    std::map<Dyninst::Address,unsigned char*>::iterator pIter = owPages.begin();
    assert( owPages.end() != pIter );

    mapped_object *curObj = findObject((*pIter).first);

    std::map<Address,Address> objRanges;
    for(; pIter != owPages.end(); pIter++) {
        assert ( curObj == findObject((*pIter).first) );
        std::map<Address,Address>::iterator rIter = owRanges.begin();
        for (; rIter != owRanges.end(); rIter++) {
            objRanges[(*rIter).first] = (*rIter).second;
        }
    }

    curObj->updateMappedFile(objRanges);
    objRanges.clear();
}


/* Summary
 * If the entry point of a function is overwritten, purge the overwritten part
 * of that function, taking care to account for currently executing code.
 * Given a list of dead functions, find the affected blocks.
 */
bool PCProcess::getDeadCodeFuncs
( std::set<bblInstance *> &deadBlocks, // we add unreachable blocks to this
  std::set<int_function*> &affectedFuncs, //output
  std::set<int_function*> &deadFuncs) //output
{

    // do a stackwalk to see if this function is currently executing
    pdvector<pdvector<Frame> >  stacks;
    pdvector<Address> pcs;
    if (!walkStacks(stacks)) {
        inst_printf("%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
        return false;
    }
    for (unsigned i = 0; i < stacks.size(); ++i) {
        pdvector<Frame> &stack = stacks[i];
        for (unsigned int j = 0; j < stack.size(); ++j) {
            pcs.push_back( (Address) stack[j].getPC());
        }
    }

    // set affected functions
    for (set<bblInstance *>::iterator bIter=deadBlocks.begin();
         bIter != deadBlocks.end();
         bIter++)
    {
        affectedFuncs.insert((*bIter)->func());
    }

    // get unreachable image blocks, identify functions with
    // overwritten entry points
    image *blockImg = (*deadBlocks.begin())->func()->ifunc()->img();
    set<image_basicBlock*> deadImgBs;
    set<image_func*> deadImgFuncs;
    for (set<bblInstance *>::iterator bIter=deadBlocks.begin();
         bIter != deadBlocks.end();
         bIter++)
    {
        image_basicBlock *imgB = (*bIter)->block()->llb();
        if ( imgB->getEntryFunc() ) {
            deadImgFuncs.insert( imgB->getEntryFunc() );
        }
        if ( !imgB->getEntryFunc() || imgB->isShared() ) {
            deadImgBs.insert(imgB);
        }
        assert(blockImg == (*bIter)->func()->ifunc()->img());
    }

    set<image_basicBlock*> unreachableImgBs;
    vector<bblInstance*> unreachableBlocks;
    image_func::getUnreachableBlocks(deadImgBs, unreachableImgBs);

    // if we're executing in a block that's been marked as unreachable, don't
    // eliminate the unreachable blocks
    bool inUnreachable = false;
    vector<ParseAPI::Function *> blockfuncs;
    for (set<image_basicBlock *>::iterator bIter=unreachableImgBs.begin();
         !inUnreachable && bIter != unreachableImgBs.end();
         bIter++)
    {
        (*bIter)->getFuncs(blockfuncs);
        image_func *bfunc = dynamic_cast<image_func*>(blockfuncs[0]);
        int_basicBlock *unreachBlock =
            findFuncByInternalFunc(bfunc)->findBlockByAddr
                ( (*bIter)->firstInsnOffset() +
                  bfunc->img()->desc().loadAddr() );
        blockfuncs.clear();
        unreachableBlocks.push_back(unreachBlock->origInstance());
        for (unsigned pcI = 0; !inUnreachable && pcI < pcs.size(); pcI++) {
            Address pc = pcs[pcI];
            int_basicBlock *pcblock = findBasicBlockByAddr(pc);
            if (!pcblock) {
                pcblock = findBasicBlockByAddr
                    (findMultiTrampByAddr(pc)->instToUninstAddr(pc));
            }
            if (pcblock == unreachBlock) {
                mal_printf("WARNING: executing in block[%lx %lx] that "
                        "is only reachable from overwritten blocks, so "
                        "will not delete any of the blocks that fit this "
                        "description %s[%d]\n",
                        unreachBlock->origInstance()->firstInsnAddr(),
                        unreachBlock->origInstance()->endAddr(),
                        FILE__,__LINE__);
                inUnreachable = true;
            }
        }
    }
    if (!inUnreachable) {
        deadBlocks.insert(unreachableBlocks.begin(), unreachableBlocks.end());
    }

    // Lots of special case code for the limited instance in which a block
    // is overwritten that is at the start of a function, in which case the
    // whole function can go away.
    // If we're executing the entry block though, re-parse the function
    for (set<image_func *>::iterator fIter=deadImgFuncs.begin();
         fIter != deadImgFuncs.end();
         fIter++)
    {
        bool inEntryBlock = false;
        int_function *deadFunc = findFuncByInternalFunc(*fIter);
        int_basicBlock *entryBlock = deadFunc->findBlockByAddr
            ((*fIter)->entryBlock()->firstInsnOffset()
             + (*fIter)->img()->desc().loadAddr());

        // see if we're executing in the entryBlock
        for (unsigned pcI = 0; !inEntryBlock && pcI < pcs.size(); pcI++) {
            Address func_pc = pcs[pcI];
            int_basicBlock *pcblock = findBasicBlockByAddr(func_pc);
            if (!pcblock) {
                pcblock = findBasicBlockByAddr
                    (findMultiTrampByAddr(func_pc)->instToUninstAddr(func_pc));
            }
            if (pcblock == entryBlock) {
                inEntryBlock = true;
            }
        }

        // add all function blocks to deadBlocks
        std::set< int_basicBlock* , int_basicBlock::compare >
            fblocks = deadFunc->blocks();
        std::set<int_basicBlock*,int_basicBlock::compare>::iterator
            fbIter = fblocks.begin();
        while (fbIter != fblocks.end()) {
            deadBlocks.insert((*fbIter)->origInstance());
            fbIter++;
        }

        if (!inEntryBlock) {
            // mark func dead
            deadFuncs.insert(deadFunc);
            affectedFuncs.erase(affectedFuncs.find(deadFunc));
        }
    }// for all dead image funcs

    return true;
}

void PCProcess::getActiveMultiMap(std::map<Address, multiTramp *> &map) {
    for(set<multiTramp *>::iterator mIter = activeMultis_.begin();
            mIter != activeMultis_.end();
            ++mIter)
    {
        map[(*mIter)->instAddr()] = *mIter;
    }
}

void PCProcess::addActiveMulti(multiTramp *multi) {
    activeMultis_.insert(multi);
}

/* This function does the following:
 *
 * walk the stacks
 * build up set of currently active tramps
 * find multiTramps that are no longer active
 * for every multiTramp that has become inactive
 *     mark the tramp as inactive (is this flag necessary any more?)
 *     if the multitramp is partlyGone
 *         then delete it
 * update process's set of currently active tramps
 */
void PCProcess::updateActiveMultis() {
    // return if cached results are valid
    if ( isAMcacheValid_ ||
         ( analysisMode_ != BPatch_exploratoryMode &&
           analysisMode_ != BPatch_defensiveMode      ) )
    {
        return;
    }

    // walk the stacks
    pdvector<pdvector<Frame> >  stacks;
    if ( false == walkStacks(stacks) ) {
        fprintf(stderr,"ERROR: %s[%d], walkStacks failed\n", FILE__, __LINE__);
        assert(0);
    }

    // build up new set of active tramps
    std::set<multiTramp*> newActiveMultis;
    std::map<bblInstance*,Address> newActiveBBIs;
    for (unsigned int i = 0; i < stacks.size(); ++i) {
        pdvector<Frame> &stack = stacks[i];
        mal_printf("updateActiveMultis stackwalk:\n");
        int_function *calleeFunc = NULL;

        // mark the multiTramps that contain calls as active
        for (unsigned int j = 0; j < stack.size(); ++j) {
            Frame *curFrame = &stack[j];
            if (j < 64) {
                mal_printf(" stackpc[%d]=0x%lx fp %lx sp %lx pcloc %lx\n", j,
                        stack[j].getPC(),stack[j].getFP(),
                        stack[j].getSP(), stack[j].getPClocation());
            }
            multiTramp *multi = findMultiTrampByAddr( curFrame->getPC() );
            bblInstance *activebbi;
            Address funcRelocAddr = 0;
            if (NULL != multi) {
                activebbi = findOrigByAddr
                    ( multi->instAddr() + multi->instSize() -1 )->
                    is_basicBlockInstance();
                // Make the multi active if we're likely to execute it
                // again, i.e., if it contains a call or if we're in a frame
                // corresponding to a faulting instruction.
                // The multi on the top-most stack frame may not contain a call,
                // we mark it as active only if it does, as we will not re-enter
                // it and its trampEnd should not have to be updated
                if (activebbi &&
                    (activebbi->block()->containsCall() ||
                     activebbi->func()->obj()->parse_img()->codeObject()->
                     defensiveMode()))
                {
                    do {
                        newActiveMultis.insert( multi );
                        multi->setIsActive(true);
                        assert(NULL != activebbi);
                        funcRelocAddr = multi->getFuncBaseInMutatee();
                        multi = multi->getStompMulti();
                    } while (NULL != multi);
                } else {
                    activebbi = NULL;
                }

            } else { // This is either the first frame on the call-stack,
                     // a call bbi on the stack,
                     // or a fault-raising instruction.
                     // Save the block & set relocAddr

                // find the active block
                mapped_object *activeobj = findObject(curFrame->getPC());
                if ( (calleeFunc && calleeFunc->isSignalHandler() ) ||
                     ( 0==j && activeobj &&
                       activeobj->parse_img()->codeObject()->defensiveMode() ) )
                {   // there is a fault-raising instruction in this block, use framePC
                    activebbi = findOrigByAddr( curFrame->getPC() )->
                        is_basicBlockInstance();

                } else { // there's a call in the preceding block, use framePC-1
                    activebbi = findOrigByAddr( curFrame->getPC()-1 )->
                        is_basicBlockInstance();
                }

                // don't bother about multiTramps that have been removed since
                // the previous stackwalk or about blocks in system libraries,
                // or non-heap code in the runtime library
                if ( NULL == activebbi ||
                     (activebbi->func()->obj()->isSharedLib() &&
                      (activebbi->func()->obj()->parse_img()->codeObject()->
                       defensiveMode() ||
                       (runtime_lib.end() != runtime_lib.find(activebbi->func()->obj()) &&
                        !isRuntimeHeapAddr(curFrame->getPC())))))
                {
                    calleeFunc = findFuncByAddr(curFrame->getPC());
                    continue;
                }

                mal_printf("Adding activeBBI %lx[%lx %lx] for framePC %lx "
                           "%s[%d]\n",
                           activebbi->block()->origInstance()->firstInsnAddr(),
                           activebbi->firstInsnAddr(), activebbi->endAddr(),
                           curFrame->getPC(), FILE__,__LINE__);
                newActiveBBIs[ activebbi ] = curFrame->getPC();

                // calculate funcRelocBase, the base address of the relocated
                // function in which the activebbi resides
                if ( 0 == activebbi->version() ) {
                    // easy case, the function is not relocated
                    funcRelocAddr = activebbi->func()->getAddress();
                }

                else if ( activebbi->version() > activebbi->func()->version() ||
                          activebbi !=
                          activebbi->block()->instVer( activebbi->version() ) )
                {   // the block is a remnant of an invalidated function
                    // relocation, in which case we've saved the funcRelocBase
                    assert( 0 != activebbi->getFuncRelocBase() );
                    funcRelocAddr = activebbi->getFuncRelocBase();

                } else {
                    // funcRelocBase is the address of the first block in
                    // the function to contain a bbi that matches activebbi's
                    // function version.
                    const set< int_basicBlock* , int_basicBlock::compare > *
                        blocks = & activebbi->func()->blocks();
                    set<int_basicBlock*,int_basicBlock::compare>::const_iterator
                        bIter = blocks->begin();
                    for( ;
                        bIter != blocks->end() &&
                        activebbi->version() >= (int)(*bIter)->instances().size();
                        bIter++);
                    assert( bIter != blocks->end() );
                    funcRelocAddr = (*bIter)->instVer( activebbi->version() )->
                                            firstInsnAddr();
                    activebbi->setFuncRelocBase(funcRelocAddr);
                }
            }

            // save the block's function relocation, if the block
            // is part of a relocated function
            if ( activebbi != NULL && activebbi->version() > 0 ) {
                if (am_funcRelocs_.end() ==
                    am_funcRelocs_.find(activebbi->func()))
                {
                    am_funcRelocs_[activebbi->func()] = new set<Address>;
                }
                am_funcRelocs_[activebbi->func()]->insert( funcRelocAddr );
            }
            calleeFunc = findFuncByAddr(curFrame->getPC());
        }
    }

    // identify multiTramps that are no longer active
    set<multiTramp*> prevActiveMultis;
    std::set_difference(activeMultis_.begin(), activeMultis_.end(),
                        newActiveMultis.begin(), newActiveMultis.end(),
                        inserter(prevActiveMultis,
                                 prevActiveMultis.begin()));
    // identify block instances that are no longer active
    map<bblInstance*,Address> prevActiveBBIs;
    std::set_difference(activeBBIs_.begin(), activeBBIs_.end(),
                        newActiveBBIs.begin(), newActiveBBIs.end(),
                        inserter(prevActiveBBIs,
                                 prevActiveBBIs.begin()));

    map<Address,int_function*> relocsToRemove;

    // for each multitramp that has become inactive:
    for (set<multiTramp*>::iterator mIter = prevActiveMultis.begin();
         mIter != prevActiveMultis.end();
         mIter++)
    {
        multiTramp *curMulti = *mIter;
        // mark the tramp as inactive (is this flag necessary any more?)
        curMulti->setIsActive(false);

        // mark the reloc for possible removal and delete the block if the
        // reloc has been invalidated and no other active multis are
        // installed at the same block
        bblInstance *bbi =
            findOrigByAddr(curMulti->instAddr())->is_basicBlockInstance();
        assert(bbi);
        if ( bbi->version() <= bbi->func()->version() &&
             bbi != bbi->block()->instVer( bbi->version() ) )
        {
            bool blockStillActive = false;
            for(set<multiTramp*>::iterator mIter = activeMultis_.begin();
                !blockStillActive && mIter != activeMultis_.end(); mIter++)
            {
                if (bbi->firstInsnAddr() == (*mIter)->instAddr()) {
                    blockStillActive = true;
                }
            }
            if ( !blockStillActive ) {
                if ( bbi->version() > 0 ) {
                    relocsToRemove[curMulti->getFuncBaseInMutatee()] =
                        bbi->func();
                }
                removeOrigRange(bbi);
                bbi->block()->func()->deleteBBLInstance(bbi);
                delete(bbi);
            }
        }

        // if the multitramp is partlyGone, remove and delete it
        if ( true == curMulti->getPartlyGone() ) {
            // Unfortunately, we can't remove the multi without removing its
            // BPatchSnippetHandles or we'll have dangling pointers

            // remove the mutator's knowledge of the link to the multiTramp
            // since we can't delete it directly,
            // otherwise we'll find this multitramp by looking up its instAddr
            instArea *jump =
                dynamic_cast<instArea *>(findModByAddr(curMulti->instAddr()));
            if (jump && jump->multi == curMulti) {
                removeModifiedRange(jump);
                delete jump;
            }

        }
    }

    // for each block that has become inactive:
    for (map<bblInstance*,Address>::iterator bIter = prevActiveBBIs.begin();
         bIter != prevActiveBBIs.end();
         bIter++)
    {
        bblInstance *bbi = bIter->first;
        if ( NULL == bbi->block() ) {
            delete(bbi); // it's been wiped out by removeBlock
        }
        // if the block and its func reloc have been invalidated
        else if ( bbi->version() > bbi->func()->version() ||
             bbi != bbi->block()->instVer( bbi->version() ) )
        {
            // mark the reloc for possible removal
            assert( 0 != bbi->getFuncRelocBase() );//set by relocationInvalidate
            if ( bbi->version() > 0 ) {
                relocsToRemove[bbi->getFuncRelocBase()] = bbi->func();
            }

            // remove block from all datastructures
            removeOrigRange(bbi);
            bbi->block()->func()->deleteBBLInstance(bbi);
            delete(bbi);
        }
    }

    // update process's set of currently active tramps
    activeMultis_.clear();
    activeMultis_.insert(newActiveMultis.begin(),newActiveMultis.end());
    activeBBIs_.clear();
    activeBBIs_.insert(newActiveBBIs.begin(),newActiveBBIs.end());

    // from the set of relocs that lost an active multiTramp, identify the
    // relocs that no longer have ANY active multitramps or active bblInstances
    for (set<multiTramp*>::iterator mIter = activeMultis_.begin();
         mIter != activeMultis_.end();
         mIter++)
    {
        if ( relocsToRemove.end() !=
             relocsToRemove.find((*mIter)->getFuncBaseInMutatee()) )
        {   // has an active multiTramp, can't remove this reloc
            relocsToRemove.erase((*mIter)->getFuncBaseInMutatee());
        }
    }
    for (map<bblInstance*,Address>::iterator bIter = activeBBIs_.begin();
         bIter != activeBBIs_.end();
         bIter++)
    {
        if ( 0 == bIter->first->version() ) {
            continue; // there is no reloc for origInstance blocks
        }
        // the bbi is invalidated, so the reloc_info should have been set by
        // relocationInvalidate
        Address funcRelocAddr = bIter->first->getFuncRelocBase();
        assert( 0 != funcRelocAddr );
        if ( relocsToRemove.end() !=
             relocsToRemove.find(funcRelocAddr) )
        {   // has an active bblInstance, can't remove this reloc
            relocsToRemove.erase(funcRelocAddr);
        }
    }

    // remove the relocs that are safe to remove
    for (map<Address,int_function*>::iterator rIter = relocsToRemove.begin();
         rIter != relocsToRemove.end();
         rIter++)
    {
        assert ( am_funcRelocs_.end() != am_funcRelocs_.find(rIter->second) );
        assert ( NULL != am_funcRelocs_[rIter->second] );
        am_funcRelocs_[rIter->second]->erase( rIter->first );
        if (true == am_funcRelocs_[rIter->second]->empty()) {
            delete am_funcRelocs_[rIter->second];
            am_funcRelocs_.erase( rIter->second );
        }
        mal_printf("freeing function relocation at %lx for func at %lx %s[%d]\n",
                rIter->first, rIter->second->getAddress(), FILE__, __LINE__);
        inferiorFree(rIter->first);
    }

    isAMcacheValid_ = true;
}

/* Update the trampEnds of active multiTramps
 * Update return addresses to invalidated function relocations
 */
void PCProcess::fixupActiveStackTargets() {
    if ( !isAMcacheValid_ ||
         ( analysisMode_ != BPatch_exploratoryMode &&
           analysisMode_ != BPatch_defensiveMode      ) )
    {
        return; // if it's not valid, the analysis did not change
    }

    map<Address,Address> pcUpdates;

    /* Update the trampEnds of active multiTramps */
    for (set<multiTramp*>::iterator mIter = activeMultis_.begin();
         mIter != activeMultis_.end();
         mIter++)
    {
        multiTramp *multi = *mIter;

        // figure out what the target should be
        int_basicBlock *targBlock = findBasicBlockByAddr
            ( multi->instAddr() + multi->instSize() );
        bblInstance *targBBI = NULL;
        if (targBlock) {
            targBBI = targBlock->instVer( targBlock->func()->version() );
        }
        else {
            // No block after the [instAddr instAddr+instSize] block,
            // one possibility is that we've updated the analysis
            // and removed other bblInstances for this function,
            // Start at tramp install block and move to whatever is at
            // instAddr+instSize
            // (they're not the same block, because if the install size is
            // smaller than the block size, we would not have left the block)
            long instSize = multi->instSize();
            targBBI = findOrigByAddr( multi->instAddr() )->
                is_basicBlockInstance();
            assert(targBBI);
            int bbiVersion = targBBI->version();
            mal_printf("finding multi targ block, going from source block: "
                       "[%lx %lx][%lx %lx] ",
                       targBBI->block()->origInstance()->firstInsnAddr(),
                       targBBI->block()->origInstance()->endAddr(),
                       targBBI->firstInsnAddr(), targBBI->endAddr());
            // move to the next block if instSize is smaller than the block size
            while ( targBBI && instSize >= (long)(targBBI->getSize()) )
            {
                int sizeDiff = 0;
                // if other bbi's in this function have been deleted
                // and we've switched to origBBIs, the block may have
                // been expanded in the relocated versions, so instSize
                // should be adjusted according to the expanded size
                if (0 == targBBI->version() && 0 < bbiVersion &&
                    instruction::maxJumpSize(getAddressWidth()) < targBBI->getSize())
                {
                    sizeDiff = instruction::maxJumpSize(getAddressWidth());
                } else {
                    sizeDiff = targBBI->getSize();
                }
                bblInstance *nextBBI = targBBI->getFallthroughBBL();
                // getFallthroughBBI may have returned to version 0 if the
                // function relocation is gone, in which we should adjust by
                // the size of the origInstance block
                if (nextBBI && targBBI->version() != nextBBI->version()) {
                    mal_printf("switching function relocation versions in "
                               "fixUpActiveTramps for func at %lx block %lx "
                               "%s[%d]\n", targBBI->func()->getAddress(),
                               targBBI->firstInsnAddr(), FILE__,__LINE__);
                }
                targBBI = nextBBI;
                instSize -= sizeDiff;
            }
            if (!targBBI) { //could be sign of bad stackwalk, as in case of FSG
                mal_printf("WARNING: Couldn't fix target of trampEnd for "
                        "active multiTramp installed at [%lx %lx], originally "
                        "[%lx], it has no fallthrough block %s[%d]\n",
                        multi->instAddr(),
                        multi->instAddr() + multi->instSize(),
                        findOrigByAddr(multi->instAddr())->
                            is_basicBlockInstance()->block()->
                            origInstance()->firstInsnAddr(),
                        FILE__,__LINE__);
                continue;
            }

            targBBI = targBBI->block()->instVer(targBBI->func()->version());
            targBlock = targBBI->block();

            mal_printf("to targBlock [%lx %lx][%lx %lx] %s[%d]\n",
                       targBBI->block()->origInstance()->firstInsnAddr(),
                       targBBI->block()->origInstance()->endAddr(),
                       targBBI->firstInsnAddr(), targBBI->endAddr(),
                       FILE__,__LINE__);
        }

        // update the trampEnd's target, if it's wrong
        do { // loop through the chain of multis that stomp the current multi
            trampEnd *end = multi->getTrampEnd();
                    assert(NULL != end);
            if ( end->target() < targBBI->firstInsnAddr() ||
                 end->target() >= targBBI->endAddr() )
            {
                bblInstance *oldTargBBI = findOrigByAddr(end->target())->
                    is_basicBlockInstance();
                Address newTarget = targBBI->firstInsnAddr();
                // if the target is not to the beginning of the block, set
                // newTarget to point to the equivalent address in the block
                if (oldTargBBI && oldTargBBI->firstInsnAddr() != end->target())
                {
                    newTarget =
                        oldTargBBI->equivAddr(targBBI->version(), newTarget);
                }
                mal_printf("updating trampEnd at %lx in multi [%lx %lx][%lx "
                           "%lx]: oldTarget=%lx, newTarget=%lx to block "
                           "originally at [%lx %lx] now at [%lx %lx] %s[%d]\n",
                           end->get_address(),
                           multi->instAddr(), multi->instSize(),
                           multi->get_address(),
                           multi->get_address() + multi->get_size(),
                           end->target(), newTarget,
                           targBlock->origInstance()->firstInsnAddr(),
                           targBlock->origInstance()->endAddr(),
                           targBBI->firstInsnAddr(),
                           targBBI->endAddr(),
                           FILE__,__LINE__);
                // trigger new code generation for the trampEnd
                end->changeTarget( newTarget );
                codeGen endGen(multi->getAddress()
                               + multi->get_size()
                               - end->get_address());
                end->generateCode(endGen, end->get_address(), NULL);
                // copy the newly generated code to the mutatee
                writeTextSpace((void*)end->get_address(),
                               end->get_size(),
                               endGen.start_ptr());
            }

            // keep changing trampEnds if this multitramp was stomped by
            // a newer multi
            if (multi->getStompMulti()) {
                Address jumpTarg = 0;
                codeRange *range = findModByAddr(multi->instAddr());
                instArea *jump = dynamic_cast<instArea*>(range);
                functionReplacement *fjump = range->is_function_replacement();
                if (jump) {
                    jumpTarg = jump->multi->getAddress();
                } else if (fjump) {
                    jumpTarg = fjump->target()->instVer
                        (fjump->targetVersion())->firstInsnAddr();
                }
                mal_printf("multi at %lx is stomped, jump is to instAddr=%lx, "
                           "trampAddr=%lx\n",
                           multi->getAddress(), multi->instAddr(),
                           jumpTarg);
            }
            multi = multi->getStompMulti();
        } while( NULL != multi );
    }

    /* Iterate through activeBBIs_ to detect if we are planning to
       return to an activeBBI in an invalidated function relocation */
    for (map<bblInstance*,Address>::iterator bIter = activeBBIs_.begin();
         bIter != activeBBIs_.end();
         bIter++)
    {
        /* We start out with the old target BBI, but it may have been
         * overwritten, its relocation may have been invalidated, or
         * it may have been split.
         * So: safeguard against overwrites by translating back to the
         * original address, deal with splits by traversing to the latter
         * half of split blocks, while keeping in mind that the relocation
         * may have been invalidated
         */

        const bblInstance *oldTargBBI = bIter->first;
        bblInstance *bbi = bIter->first;
        mal_printf("fixing activeBlock [%lx %lx][%lx %lx] with PC=%lx %s[%d]\n",
                   bbi->block()->origInstance()->firstInsnAddr(),
                   bbi->block()->origInstance()->endAddr(),
                   bbi->firstInsnAddr(), bbi->endAddr(), bIter->second,
                   FILE__,__LINE__);

        // if the block was overwritten, switch to the original instance
        if (NULL == bbi->block()) {
            if (bbi->version() == 0) {
                bbi = findOrigByAddr(bIter->second)->is_basicBlockInstance();
            } else {
                bbi = findOrigByAddr(bbi->equivAddr(0,bIter->second))->
                    is_basicBlockInstance();
            }

        } else { // account for block splitting and relocation invalidations

            Address prevStart = 0; //used to check for reverting to origInst
            while (bbi && bbi->endAddr() < bIter->second &&
                   prevStart < bbi->firstInsnAddr())
            {
                prevStart = bbi->firstInsnAddr();
                bbi = bbi->getFallthroughBBL();
                mal_printf("activeBlock split response, moving to block "
                           "[%lx %lx][%lx %lx]\n",
                           bbi->block()->origInstance()->firstInsnAddr(),
                           bbi->block()->origInstance()->endAddr(),
                           bbi->firstInsnAddr(), bbi->endAddr(), bIter->second,
                           FILE__,__LINE__);
            }

            if (!bbi) {
                mal_printf("WARNING: bbi[%lx %lx] on stack does not contain or "
                        "immediately precede the PC and has no fallthrough "
                        "block %s[%d]\n", bIter->first->firstInsnAddr(),
                        bIter->first->endAddr(), FILE__,__LINE__);
                continue;
            }
            // if all we did was account for block splitting, w/o switching
            // from one relocation to another, set oldTargBBI to bbi
            if (bbi->firstInsnAddr() > oldTargBBI->firstInsnAddr()) {
                oldTargBBI = bbi;
            }
        }

        // find a valid bbi to return to if there's been a change
        bblInstance *newbbi = bbi->block()->instVer( bbi->func()->version() );
        if ( oldTargBBI != newbbi ) {
            // if we're returning after a call BBI
            if (oldTargBBI->endAddr() == bIter->second) {
                assert(bbi->block()->containsCall());
                while ( ! newbbi->block()->containsCall() ) {
                    newbbi = newbbi->getFallthroughBBL();
                }
                pcUpdates[bIter->second] = newbbi->endAddr();
            } else {
                // we're returning after a fault bbi, most likely
                assert(bbi->firstInsnAddr() <= bIter->second &&
                       bIter->second < bbi->endAddr() && newbbi);
                Address translAddr =
                    bbi->equivAddr(newbbi->version(), bIter->second);
                pcUpdates[bIter->second] = translAddr;
            }
        }
    }

    /* Update return addresses that used to point to invalidated function
       relocations */
    if ( ! pcUpdates.empty() ) {

        // walk the stacks
        pdvector<pdvector<Frame> >  stacks;
        if ( false == walkStacks(stacks) ) {
            fprintf(stderr,"ERROR: %s[%d], walkStacks failed\n",
                    FILE__, __LINE__);
            assert(0);
        }

        // match pcUpdate pairs to the pc's on the call stack
        for(map<Address,Address>::iterator pIter = pcUpdates.begin();
            pIter != pcUpdates.end();
            pIter++)
        {
            bool foundBlock = false;
            for (unsigned int i = 0; !foundBlock && i < stacks.size(); ++i) {
                pdvector<Frame> &stack = stacks[i];
                mal_printf("fixupActiveMultis stackwalk:\n");
                for (unsigned int j=0; !foundBlock && j < stack.size(); ++j) {
                    mal_printf(" stackpc[%d]=0x%lx fp %lx sp %lx pcloc %lx\n",
                               j, stack[j].getPC(),stack[j].getFP(),
                               stack[j].getSP(), stack[j].getPClocation());
                    if ( pIter->first == stack[j].getPC() ) {
                        stack[j].setPC( pIter->second ); //update pc
                        foundBlock = true;
                    }
                }
            }
            if ( ! foundBlock ) {
                fprintf(stderr,"failed to find block pc in need of update "
                        "from %lx to %lx %s[%d]\n",pIter->first, pIter->second,
                        FILE__,__LINE__);
                assert(0);
            }
        }
    }
}

/* Given an address that's on the call stack, find the function that's
 * actively executing that address.  This makes most sense for finding the
 * address that's triggered a context switch back to Dyninst, either
 * through instrumentation or a signal
 */
int_function *PCProcess::findActiveFuncByAddr(Address addr)
{
    bblInstance *bbi = findOrigByAddr(addr)->is_basicBlockInstance();
    assert(bbi);
    int_function *func = findFuncByAddr(addr);
    if (!func) {
        return NULL;
    }
    int_basicBlock *block = func->findBlockByAddr(addr);
    if (!block) {
        return NULL;
    }
    if (block->llb()->isShared()) {
        bool foundFrame = false;
        pdvector<pdvector<Frame> >  stacks;
        if ( false == walkStacks(stacks) ) {
            fprintf(stderr,"ERROR: %s[%d], walkStacks failed\n",
                    FILE__, __LINE__);
            assert(0);
        }
        for (unsigned int i = 0; !foundFrame && i < stacks.size(); ++i) {
            pdvector<Frame> &stack = stacks[i];
            for (unsigned int j = 0; !foundFrame && j < stack.size(); ++j) {
                int_function *frameFunc = NULL;
                Frame *curFrame = &stack[j];
                Address framePC = curFrame->getPC();
                codeRange *pcRange = findOrigByAddr(framePC);
                // if we're in instrumentation, use the multiTramp function
                if (pcRange->is_multitramp() || pcRange->is_minitramp()) {
                    frameFunc = curFrame->getFunc();
                } else if (bbi->firstInsnAddr() <= framePC &&
                           framePC <= bbi->lastInsnAddr() &&
                           j < stack.size()-1) {
                    // find the function by looking at the previous stack
                    // frame's call target
                    Address callerPC = stack[j+1].getPC();
                    bblInstance *callerBBI = findOrigByAddr(callerPC-1)->
                        is_basicBlockInstance();
                    if (callerBBI) {
                        instPoint *callPt = callerBBI->func()->findInstPByAddr
                            (callerBBI->block()->origInstance()->endAddr());
                        if (callPt && callPt->callTarget()) {
                            image *img = callPt->func()->obj()->parse_img();
                            frameFunc = findFuncByInternalFunc(
                                img->findFuncByEntry
                                (callPt->callTarget()-img->desc().loadAddr()));
                        }
                    }
                }
                if (frameFunc && bbi->block() == frameFunc->findBlockByAddr
                        (bbi->block()->origInstance()->firstInsnAddr())) {
                    foundFrame = true;
                    func = frameFunc;
                }
            }
        }
        assert(foundFrame);
    }
    return func;
}

/* debugSuicide is a kind of alternate debugging continueProc.  It runs the
 * process until terminated in single step mode, printing each instruction as
 * it executes.
 */
void PCProcess::debugSuicide() {
    if( isTerminated() ) return;

    isInDebugSuicide_ = true;

    pdvector<Frame> activeFrames;
    getAllActiveFrames(activeFrames);

    for(unsigned i=0; i < activeFrames.size(); ++i) {
        Address addr = activeFrames[i].getPC();
        codeRange *range = findOrigByAddr(addr);
        fprintf(stderr, "Frame %u @ 0x%lx\n", i , addr);
        if( range ) range->print_range();
        else fprintf(stderr, "\n");
    }

    Thread::ptr initialThread = pcProc_->threads().getInitialThread();

    initialThread->setSingleStepMode(true);
    while( !isTerminated() && isAttached() && initialThread->isLive() ) {
        // Get the current PC
        MachRegister pcReg = MachRegister::getPC(getArch());
        MachRegisterVal resultVal;
        if( !initialThread->getRegister(pcReg, resultVal) ) {
            fprintf(stderr, "%s[%d]: failed to retreive register from thread %d/%d\n",
                    FILE__, __LINE__, getPid(), initialThread->getLWP());
            return;
        }

        codeRange *range = findOrigByAddr((Address)resultVal);
        fprintf(stderr, "0x%lx\n", (Address)resultVal);
        if( range ) range->print_range();
        else fprintf(stderr, "\n");
    }
}

pdvector<int_function *> PCProcess::pcsToFuncs(pdvector<Frame> stackWalk) {
    pdvector <int_function *> ret;
    unsigned i;
    int_function *fn;
    for(i=0;i<stackWalk.size();i++) {
        fn = (int_function *)findFuncByAddr(stackWalk[i].getPC());
        // no reason to add a null function to ret
        if (fn != 0) ret.push_back(fn);
    }
    return ret;
}

bool PCProcess::isInSignalHandler(Address addr) {
    codeRange *range;
    if( signalHandlerLocations_.find(addr, range) ) {
        return true;
    }

    return false;
}

void PCProcess::addSignalHandler(Address addr, unsigned size) {
    signal_handler_location *newSig = new signal_handler_location(addr, size);

    signalHandlerLocations_.insert(newSig);
}

bool PCProcess::mappedObjIsDeleted(mapped_object *obj) {
    for(unsigned i = 0; i < deletedObjects_.size(); ++i) {
        if( obj == deletedObjects_[i] ) return true;
    }

    return false;
}

// AddressSpace Implementation //
Address PCProcess::offset() const {
    assert(!"This function is not implemented");
    return 0;
}

Address PCProcess::length() const {
    assert(!"This function is not implemented");
    return 0;
}

Architecture PCProcess::getArch() const {
    return savedArch_;
}

bool PCProcess::multithread_ready(bool ignoreIfMtNotSet) {
    // Since ProcControlAPI has taken over handling thread creation
    // and destruction from the RT library, as soon as the process reaches
    // the initialized state, the process is multithread ready if it
    // is multithread capable.

    if( !hasReachedBootstrapState(bs_initialized) ) return false;
    if( !multithread_capable(ignoreIfMtNotSet) ) return false;

    return true;
}

bool PCProcess::needsPIC() {
    return false;
}

bool PCProcess::isInDebugSuicide() const {
    return isInDebugSuicide_;
}

PCProcess::processState_t PCProcess::getDesiredProcessState() const {
    return processState_;
}

void PCProcess::setDesiredProcessState(PCProcess::processState_t pc) {
    processState_ = pc;
}

bool PCProcess::walkStack(pdvector<Frame> &stackWalk,
                          PCThread *thread)
{
  if( stackwalker_ == NULL ) return false;

  vector<Dyninst::Stackwalker::Frame> swWalk;

  if (!stackwalker_->walkStack(swWalk, thread->getLWP()))
  {
    return false;
  }

  for (vector<Dyninst::Stackwalker::Frame>::iterator SWB = swWalk.begin(),
       SWI = SWB,
       SWE = swWalk.end();
       SWI != SWE;
       ++SWI)
  {
    stackWalk.push_back(Frame(*SWI, this, thread, (SWI == SWB)));
  }

  return true;
}

bool PCProcess::getActiveFrame(Frame &frame, PCThread *thread)
{
  Dyninst::Stackwalker::Frame swFrame;
  if (!stackwalker_->getInitialFrame(swFrame, thread->getLWP()))
  {
    return false;
  }

  frame = Frame(swFrame, this, thread, true);
  return true;
}

/* This is the simple version
 * 1. Need three pieces of information:
 * 1a. The instrumentation point that triggered the stopThread event (pointAddress)
 * 1b. The ID of the callback function given at the registration
 *     of the stopThread snippet
 * 1c. The result of the snippet calculation that was given by the user,
 *     if the point is a return instruction, read the return address
 * 2. If the calculation is an address that is meant to be interpreted, do that
 * 3. Invoke the callback
 */
bool PCProcess::triggerStopThread(Address pointAddress, int callbackID, void *calculation) {
    // get instPoint from point address
    int_function *pointfunc = findActiveFuncByAddr(pointAddress);
    if (!pointfunc) {
        mal_printf("%s[%d]: failed to find active function at 0x%lx\n",
                FILE__, __LINE__, pointAddress);
        return false;
    }

    instPoint *intPoint = pointfunc->findInstPByAddr(pointAddress);
    if (!intPoint) {
        mal_printf("%s[%d]: failed to find inst point at 0x%lx\n",
                FILE__, __LINE__, pointAddress);
        return false;
    }

    /* 2. If the callbackID is negative, the calculation is meant to be
      interpreted as the address of code, so we call stopThreadCtrlTransfer
      to translate the target to an unrelocated address */
    if (callbackID < 0) {
        callbackID *= -1;
        calculation = (void*)
            stopThreadCtrlTransfer(intPoint, (Address)calculation);
    }

    /* 3. Trigger the callback for the stopThread
      using the correct snippet instance ID & event type */
    ((BPatch_process*)up_ptr())->triggerStopThread
        (intPoint, pointfunc, callbackID, (void*)calculation);

    return true;
}

/*    If calculation is a relocated address, translate it to the original addr
 *    case 1: The point is at a return instruction
 *    case 2: The point is a control transfer into the runtime library
 *    Mark returning functions as returning
 *    Save the targets of indirect control transfers (not regular returns)
 */
Address PCProcess::stopThreadCtrlTransfer(instPoint *intPoint, Address target) {
    int_function *pointfunc = intPoint->func();
    Address pointAddress = intPoint->addr();

    // if the point is a real return instruction and its target is a stack
    // address, get the return address off of the stack
    Address returnAddrAddr=0;
    if ( intPoint->isReturnInstruction() &&
         ! intPoint->func()->isSignalHandler() &&
         NULL == findObject(target) )
    {
        returnAddrAddr = target + intPoint->preBaseTramp()->savedFlagSize;
        assert (getAddressWidth() == // otherwise we'll have cached the wrong address in the runtime library
                intPoint->preBaseTramp()->savedFlagSize);
        if (!readDataSpace((void *)returnAddrAddr,
                           getAddressWidth(), &target, false))
        {
            fprintf(stderr, "%s[%d] WARNING: reading from the stopthread "
                    "return addr addr %lx failed\n",FILE__,__LINE__,target);
            assert(0);
        }
        mal_printf("%s[%d]: return address address %lx contains %lx\n", FILE__,
                __LINE__,returnAddrAddr,target);
    }

    Address unrelocTarget = target;
    bblInstance *pointBBI = intPoint->block()->instVer(pointfunc->version());

    if ( isRuntimeHeapAddr( target ) ) {
        // case 1: The point is at a return instruction
        if (intPoint->getPointType() == functionExit &&
            ! intPoint->func()->isSignalHandler())
        {
            // case 1a: at return instruction not in signal handler
            bblInstance *callBBI = NULL;
            // set callBBI
            codeRange *callRange = findOrigByAddr(target-1);
            if (! callRange ) {
                fprintf(stderr,"ERROR STOPTHREAD FOUND NO MATCH FOR RUNTIME "
                        "HEAP ADDR %lx %s[%d]\n",target,FILE__,__LINE__);
                assert(0 && "stopThread target is unmatched rt-lib heap addr");
            }
            callBBI = callRange->is_basicBlockInstance();
            if ( ! callBBI ) {
                multiTramp *callMulti = callRange->is_multitramp();
                if (callMulti) {
                    Address uninstrumented =
                        callMulti->instToUninstAddr(target-1)+1;
                    callRange = findOrigByAddr(uninstrumented-1);
                    if ( callRange ) {
                        callBBI = callRange->is_basicBlockInstance();
                    }
                    assert(callBBI);
                }
            }
            while ( callBBI && ! callBBI->block()->containsCall() ) {
                                if ( ! callBBI->block()->getFallthrough() ) {
                                        break;
                                }
                callBBI = callBBI->block()->getFallthrough()->
                    instVer(callBBI->version());
            }
            assert( callBBI );

            // Translate the address
            unrelocTarget = callBBI->block()->origInstance()->endAddr();

            // Set the return status of the function containing the
            // return instPoint to returning
            pointfunc->ifunc()->set_retstatus(ParseAPI::RETURN);
        }
        else if (intPoint->getPointType() == functionExit &&
                 intPoint->func()->isSignalHandler())
        {
            // case 1a: at return instruction in signal handler
            bblInstance *targBBI = findOrigByAddr(target)->
                is_basicBlockInstance();
            multiTramp *targMulti = findOrigByAddr(target)->is_multitramp();
            if (targMulti) {
                Address uninstrumented =
                    targMulti->instToUninstAddr(target);
                targBBI = findOrigByAddr(uninstrumented)->is_basicBlockInstance();
            }
            assert(targBBI);
            unrelocTarget = targBBI->equivAddr(0, target);
        }
        // case 2: The point is a control transfer into the runtime library
        else {
            // This is an indirect jump or call into the dyninst runtime
            // library heap, meaning that the indirect target is calculated
            // by basing off of the program counter at the source block, so
            // adjust the target by determining the uninstrumented address of
            // the source instruction
            bblInstance *origPointBBI = intPoint->block()->origInstance();
            assert(pointAddress >= origPointBBI->firstInsnAddr() &&
                   pointAddress < origPointBBI->endAddr());
            Address relocPointAddr = origPointBBI->equivAddr
                (pointBBI->version(), pointAddress);
            Address adjustment;
            if (relocPointAddr > pointAddress) {
                adjustment = relocPointAddr - pointAddress;
            } else {
                adjustment = pointAddress - relocPointAddr;
            }
            unrelocTarget = target - adjustment;
            mal_printf("%s[%d] WARNING: stopThread caught transfer "
                    "%lx[%lx]=>%lx[%lx] whose target is "
                    "in runtime library heap, will translate target addr\n",
                    FILE__, __LINE__, relocPointAddr, pointAddress,
                    target, target);
        }
        mal_printf("%s[%d] translated %lx to %lx\n",
                FILE__, __LINE__, target, unrelocTarget);
    }
    else { // target is not relocated, find the mapped_object
        mapped_object *obj = findObject(target);
        if (!obj) {
            obj = createObjectNoFile(target);
            if (!obj) {
                fprintf(stderr,"ERROR, point %lx has target %lx that responds "
                        "to no object %s[%d]\n", pointAddress, target,
                        FILE__,__LINE__);
                assert(0 && "stopThread snippet has an invalid target");
                return 0;
            }
        }
    }

    // save the targets of indirect control transfers, save them in the point
    if (intPoint->isDynamic() && ! intPoint->isReturnInstruction()) {
        intPoint->setSavedTarget(unrelocTarget);
    }
    else if ( ! intPoint->isDynamic() &&
              functionExit != intPoint->getPointType())
    {
        // remove unresolved status from point if it is a static ctrl transfer
        if ( false == intPoint->setResolved() ) {
            fprintf(stderr,"We seem to have tried resolving this point[0x%lx] "
                    "twice, why? %s[%d]\n", pointAddress, FILE__,__LINE__);
        }
    }
    return unrelocTarget;
}

void PCProcess::triggerNormalExit(int exitcode) {
    for(std::map<dynthread_t, PCThread *>::iterator i = threadsByTid_.begin();
            i != threadsByTid_.end(); ++i)
    {
        if( i->second != initialThread_ ) 
            BPatch::bpatch->registerThreadExit(this, i->second);
    }
    BPatch::bpatch->registerNormalExit(this, exitcode);

    // Let the event handler know that the process should be moved to
    // an exited state
    setExiting(true);
}

// Debugging only
bool PCProcess::setBreakpoint(Address addr) {
    Breakpoint::ptr brkPt = Breakpoint::newBreakpoint();
    if( !pcProc_->addBreakpoint(addr, brkPt) ) {
        proccontrol_printf("%s[%d]: failed to set breakpoint at 0x%lx\n",
                FILE__, __LINE__, addr);
        return false;
    }

    return true;
}

static
Address getVarAddr(PCProcess *proc, std::string str) {
    Address retAddr = 0;

    pdvector<int_variable *> vars;
    if( proc->findVarsByAll(str, vars) ) {
        if( vars.size() != 1 ) {
            proccontrol_printf("%s[%d]: WARNING: multiple copies of %s found\n",
                    FILE__, __LINE__, str.c_str());
        }else{
            retAddr = vars[0]->getAddress();
        }
    }else{
        proccontrol_printf("%s[%d]: failed to find variable %s\n",
                FILE__, __LINE__, str.c_str());
    }
    return retAddr;
}

Address PCProcess::getRTEventBreakpointAddr() {
    if( sync_event_breakpoint_addr_ == 0 ) {
        sync_event_breakpoint_addr_ = getVarAddr(this, "DYNINST_break_point_event");
    }

    return sync_event_breakpoint_addr_;
}

Address PCProcess::getRTEventIdAddr() {
    if( sync_event_id_addr_ == 0 ) {
        sync_event_id_addr_ = getVarAddr(this, "DYNINST_synch_event_id");
    }

    return sync_event_id_addr_;
}

Address PCProcess::getRTEventArg1Addr() {
    if( sync_event_arg1_addr_ == 0 ) {
        sync_event_arg1_addr_ = getVarAddr(this, "DYNINST_synch_event_arg1");
    }

    return sync_event_arg1_addr_;
}

Address PCProcess::getRTEventArg2Addr() {
    if( sync_event_arg2_addr_ == 0 ) {
        sync_event_arg2_addr_ = getVarAddr(this, "DYNINST_synch_event_arg2");
    }

    return sync_event_arg2_addr_;
}

Address PCProcess::getRTEventArg3Addr() {
    if( sync_event_arg3_addr_ == 0 ) {
        sync_event_arg3_addr_ = getVarAddr(this, "DYNINST_synch_event_arg3");
    }

    return sync_event_arg3_addr_;
}

bool PCProcess::hasPendingEvents() {
    bool retval;
    eventCountLock_.lock();
    retval = ( eventCount_ != 0 );
    eventCountLock_.unlock();
    return retval;
}

void PCProcess::incPendingEvents() {
    eventCountLock_.lock();
    eventCount_++;
    eventCountLock_.unlock();
}

void PCProcess::decPendingEvents() {
    eventCountLock_.lock();
    assert(eventCount_ > 0);
    eventCount_--;
    eventCountLock_.unlock();
}

bool PCProcess::hasRunningSyncRPC() const {
    return (syncRPCThreads_.size() > 0);
}

void PCProcess::addSyncRPCThread(PCThread *thr) {
    proccontrol_printf("%s[%d]: added sync rpc thread %d/%d\n",
            FILE__, __LINE__, getPid(), thr->getLWP());
    syncRPCThreads_.insert(thr);
}

void PCProcess::removeSyncRPCThread(PCThread *thr) {
    proccontrol_printf("%s[%d]: removed sync rpc thread %d/%d\n",
            FILE__, __LINE__, getPid(), thr->getLWP());
    syncRPCThreads_.erase(thr);
}

bool PCProcess::continueSyncRPCThreads() {
    for(set<PCThread *>::iterator i = syncRPCThreads_.begin();
            i != syncRPCThreads_.end(); ++i)
    {
        if( !(*i)->continueThread() ) {
            proccontrol_printf("%s[%d]: failed to continue thread %d/%d for sync RPC\n",
                    FILE__, __LINE__, getPid(), (*i)->getLWP());
            return false;
        }
    }

    return true;
}

bool PCProcess::registerTrapMapping(Address from, Address to) {
    if( installedCtrlBrkpts.count(from) != 0 ) {
        proccontrol_printf("%s[%d]: there already exists a ctrl transfer breakpoint from "
                "0x%lx\n", FILE__, __LINE__, from);
        return true;
    }

    Breakpoint::ptr newBreak = Breakpoint::newTransferBreakpoint(to);

    if( !pcProc_->addBreakpoint(from, newBreak) ) {
        proccontrol_printf("%s[%d]: failed to add ctrl transfer breakpoint from "
                "0x%lx to 0x%lx\n", FILE__, __LINE__, from, to);
        return false;
    }

    installedCtrlBrkpts.insert(make_pair(from, newBreak));

    return true;
}

bool PCProcess::unregisterTrapMapping(Address from) {
    map<Address, Breakpoint::ptr>::iterator breakIter = 
        installedCtrlBrkpts.find(from);
    if( breakIter == installedCtrlBrkpts.end() ) return false;

    if( !pcProc_->rmBreakpoint(from, breakIter->second) ) {
        proccontrol_printf("%s[%d]: failed to remove ctrl transfer breakpoint from 0x%lx\n",
                FILE__, __LINE__, from);
        return false;
    }

    installedCtrlBrkpts.erase(breakIter);

    return true;
}

void PCProcess::invalidateMTCache() {
    mt_cache_result_ = not_cached;
}

StackwalkSymLookup::StackwalkSymLookup(PCProcess *p)
  : proc_(p)
{}

StackwalkSymLookup::~StackwalkSymLookup()
{}

bool StackwalkSymLookup::lookupAtAddr(Dyninst::Address addr, std::string &out_name, void* &out_value)
{
  mapped_object *mobj = proc_->findObject(addr);
  codeRange *range = NULL;
  int_function *func = NULL;
  bool result = false;

  // set out_name to the name of the function at this addr
  // set out_value to be the codeRange* for this addr

  if (mobj)
  {
    result = mobj->analyze();
    assert(result);
  }

  range = proc_->findOrigByAddr(addr);

  out_value = (void*) range;

  func = range->is_function();

  if (func)
  {
    out_name = func->prettyName();
  }
  else
  {
    out_name = string("[UNKNOWN]");
  }
  
  return true;
}

StackwalkInstrumentationHelper::StackwalkInstrumentationHelper(PCProcess *p)
  : proc_(p)
{}

StackwalkInstrumentationHelper::~StackwalkInstrumentationHelper()
{}

DynFrameHelper::DynFrameHelper(PCProcess *p)
  : FrameFuncHelper(NULL),
  proc_(p)
{}

DynFrameHelper::~DynFrameHelper()
{}

DynWandererHelper::DynWandererHelper(PCProcess *p)
  : WandererHelper(NULL),
  proc_(p)
{}

DynWandererHelper::~DynWandererHelper()
{}
