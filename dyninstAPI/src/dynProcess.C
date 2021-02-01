/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "dynProcess.h"
#include "dynThread.h"
#include "pcEventHandler.h"
#include "pcEventMuxer.h"
#include "function.h"
#include "os.h"
#include "debug.h"
#include "instPoint.h"
#include "BPatch.h"
#include "mapped_module.h"
#include "baseTramp.h"
#include "registerSpace.h"
#include "mapped_object.h"
#include "image.h"

#include "common/src/pathName.h"

#include "PCErrors.h"
#include "MemoryEmulator/memEmulator.h"
#include <boost/tuple/tuple.hpp>

#include "symtabAPI/h/SymtabReader.h"
#include "patchAPI/h/PatchMgr.h"
#include "patchAPI/h/Point.h"


#include <sstream>

using namespace Dyninst::ProcControlAPI;
using std::map;
using std::vector;
using std::string;
using std::stringstream;

Dyninst::SymtabAPI::SymtabReaderFactory *PCProcess::symReaderFactory_;

PCProcess *PCProcess::createProcess(const string file, std::vector<string> &argv,
                                    BPatch_hybridMode analysisMode,
                                    std::vector<string> &envp,
                                    const string dir, int stdin_fd, int stdout_fd,
                                    int stderr_fd)
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

    initSymtabReader();

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
       cerr << "Failed to create process " << path << endl;
       const char *lastErrMsg = getLastErrorMsg();
        startup_printf("%s[%d]: failed to create process for %s: %s\n", __FILE__,
                __LINE__, file.c_str(), lastErrMsg);
        string msg = string("Failed to create process for ") + file +
           string(": ") + lastErrMsg;
        showErrorCallback(68, msg.c_str());
        return NULL;
    }

    startup_cerr << "Created process " << tmpPcProc->getPid() << endl;

    PCProcess *ret = new PCProcess(tmpPcProc, file, analysisMode);
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
                                    BPatch_hybridMode analysisMode)
{
    initSymtabReader();

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
        
    PCProcess *ret = new PCProcess(tmpPcProc, analysisMode);
    assert(ret);

    tmpPcProc->setData(ret);

    ret->runningWhenAttached_ = tmpPcProc->allThreadsRunningWhenAttached();
    ret->file_ = tmpPcProc->libraries().getExecutable()->getAbsoluteName();

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
    if (parent->tracedSyscalls_)
      ret->tracedSyscalls_ = new syscallNotification(parent->tracedSyscalls_, ret);
    else
      ret->tracedSyscalls_ = NULL;

    // Check if RT library exists in child
    if( ret->runtime_lib.size() == 0 ) {
        // Set the RT library name
        if( !ret->getDyninstRTLibName() ) {
            startup_printf("%s[%d]: failed to get Dyninst RT lib name\n",
                    FILE__, __LINE__);
            delete ret;
            return NULL;
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

    // TODO hybrid mode stuff

    // Copy signal handlers
    std::vector<codeRange *> sigHandlers;
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

    ret->setInEventHandling(true);

    if( !ret->bootstrapProcess() ) {
        startup_cerr << "Failed to bootstrap process " << ret->getPid()
                     << ": terminating..." << endl;
        ret->terminateProcess();

        delete ret;
        return NULL;
    }

    ret->setDesiredProcessState(parent->getDesiredProcessState());

    return ret;
}

PCProcess *PCProcess::setupExecedProcess(PCProcess *oldProc, std::string execPath) {
    BPatch::bpatch->registerExecCleanup(oldProc, NULL);

    PCProcess *newProc = new PCProcess(oldProc->pcProc_, execPath, oldProc->analysisMode_);

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
    //newProc->incPendingEvents();

    BPatch::bpatch->registerExecExit(newProc);

    newProc->setExecing(false);
    newProc->setDesiredProcessState(ps_running);

    return newProc;
}

PCProcess::~PCProcess() {
        proccontrol_printf("%s[%d]: destructing PCProcess %d\n",
                FILE__, __LINE__, getPid());

    if( tracedSyscalls_ ) delete tracedSyscalls_;
    tracedSyscalls_ = NULL;

    if( irpcTramp_ ) delete irpcTramp_;
    irpcTramp_ = NULL;

    signalHandlerLocations_.clear();

    trapMapping.clearTrapMappings();

    if(pcProc_ && pcProc_->getData() == this) pcProc_->setData(NULL);
}

void PCProcess::initSymtabReader()
{
   //Set SymbolReaderFactory in Stackwalker before create/attach
   if (!symReaderFactory_) {
      symReaderFactory_ = new Dyninst::SymtabAPI::SymtabReaderFactory();
      Dyninst::Stackwalker::Walker::setSymbolReader(symReaderFactory_);
   }
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
      startup_printf("%s[%d]: inserting breakpoint at main\n", FILE__, __LINE__);
        if( !insertBreakpointAtMain() ) {
            startup_printf("%s[%d]: bootstrap failed while setting a breakpoint at main\n",
                    FILE__, __LINE__);
            return false;
        }
	startup_printf("%s[%d]: continuing process to breakpoint\n", FILE__, __LINE__);
        if( !continueProcess() ) {
            startup_printf("%s[%d]: bootstrap failed while continuing the process\n",
                    FILE__, __LINE__);
            return false;
        }

        while( !hasReachedBootstrapState(bs_readyToLoadRTLib) ) {
	  startup_printf("%s[%d]: waiting for main() loop\n", FILE__, __LINE__);
            if( isStopped() ) {
	      startup_printf("%s[%d]: We think the process is stopped, continuing\n", FILE__, __LINE__);
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
            if( PCEventMuxer::wait(true) == PCEventMuxer::Error) {
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


    std::vector<int_variable *> obsCostVec;
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

        // Initialize the MT stuff
        if (multithread_capable()) {
            if( !instrumentMTFuncs() ) {
                startup_printf("%s[%d]: Failed to instrument MT funcs\n",
                        FILE__, __LINE__);
                return false;
            }
        }
    }

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

bool PCProcess::createStackwalker()
{
  using namespace Stackwalker;
  ProcDebug *procDebug = NULL;
  StackwalkSymLookup *symLookup = NULL;

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

    startup_printf("Processing initial shared objects\n");
    startup_printf("----\n");

    initPatchAPI();

    // Do the a.out first...
    mapped_object *aout = mapped_object::createMappedObject(pcProc_->libraries().getExecutable(), this, analysisMode_);
    addASharedObject(aout);

    // Set the RT library name
    if( !getDyninstRTLibName() ) {
      bperr("Dyninst was unable to find the dyninst runtime library.");
        startup_printf("%s[%d]: failed to get Dyninst RT lib name\n",
                FILE__, __LINE__);
        return false;
    }

    // Find main
    startup_printf("%s[%d]:  leave setAOut/setting main\n", FILE__, __LINE__);
    setMainFunction();

    // Create mapped objects for any loaded shared libraries
    const LibraryPool &libraries = pcProc_->libraries();
    for(LibraryPool::const_iterator i = libraries.begin(); i != libraries.end(); ++i) {
       // Some platforms don't use the data load address field
       if ((*i) == libraries.getExecutable()) continue;

       startup_cerr << "Library: " << (*i)->getAbsoluteName() 
            << hex << " / " << (*i)->getLoadAddress() 
            << ", " << ((*i)->isSharedLib() ? "<lib>" : "<aout>") << dec << endl;

       mapped_object *newObj = mapped_object::createMappedObject(*i, 
                                                                 this, analysisMode_);
       if( newObj == NULL ) {
           startup_printf("%s[%d]: failed to create mapped object for library %s\n",
                   FILE__, __LINE__, (*i)->getAbsoluteName().c_str());
           return false;
       }

       const fileDescriptor &desc = newObj->getFileDesc();
       fileDescriptor tmpDesc(dyninstRT_name, desc.code(), desc.data(), true);
       if( desc == tmpDesc ) {
          startup_printf("%s[%d]: RT library already loaded, manual loading not necessary\n",
                         FILE__, __LINE__);
          runtime_lib.insert(newObj);
       }

       if (analysisMode_ == BPatch_defensiveMode) {
           std::string lib_name = newObj->fileName();
           if (lib_name == "dyninstAPI_RT.dll" ||
               lib_name == "ntdll.dll" ||
               lib_name == "kernel32.dll" ||
               lib_name == "user32.dll" ||
               lib_name == "KERNELBASE.dll" ||
               lib_name == "msvcrt.dll" ||
               lib_name == "msvcr80.dll" ||
               lib_name == "msvcr100d.dll" ||
               lib_name == "msvcp100d.dll" ||
               lib_name == "MSVCR100.dll") {
                   startup_cerr << "Running library " << lib_name
                       << " in normal mode because it is trusted.\n";
                   newObj->enableDefensiveMode(false);
           }
       }

       addASharedObject(newObj);
    }


    startup_printf("----\n");

    return true;
}

// creates an image, creates new resources for a new shared object
// adds it to the collection of mapped_objects
void PCProcess::addASharedObject(mapped_object *newObj) {
    assert(newObj);

    addMappedObject(newObj);

    findSignalHandler(newObj);

    startup_printf("%s[%d]: adding shared object %s, addr range 0x%lx to 0x%lx\n",
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

    // TODO Signal handler...
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

// NUMBER_OF_MAIN_POSSIBILITIES is defined in image.h
void PCProcess::setMainFunction() {
    assert(!main_function_);

    for (unsigned i = 0; i < NUMBER_OF_MAIN_POSSIBILITIES; i++) {
        main_function_ = findOnlyOneFunction(main_function_names[i]);
        if (main_function_) {
           break;
        }
    }
}
 
/*
 * Given an image, add all static heaps inside it
 * (DYNINSTstaticHeap...) to the buffer pool.
 */
void PCProcess::addInferiorHeap(mapped_object *obj) {
    std::vector<heapDescriptor> infHeaps;
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
      startup_printf("%s[%d]: RT library already loaded\n",
                     FILE__, __LINE__);

      bootstrapState_ = bs_loadedRTLib;
   }
   else {
     if (!pcProc_->addLibrary(dyninstRT_name)) {
       startup_printf("%s[%d]: failed to start loading RT lib\n", FILE__,
		      __LINE__);
       return false;
     }
     bootstrapState_ = bs_loadedRTLib;
     
     // Process the library load (we hope)
     PCEventMuxer::handle(this);
     
     if( runtime_lib.size() == 0 ) {
       startup_printf("%s[%d]: failed to load RT lib\n", FILE__,
		      __LINE__);
       return false;
     }
     
     bootstrapState_ = bs_loadedRTLib;
   }
   int loaded_ok = 0;
   std::vector<int_variable *> vars;
   if (!findVarsByAll("DYNINSThasInitialized", vars)) {
        startup_printf("%s[%d]: no DYNINSThasInitialized variable\n", FILE__, __LINE__);
		return false;
   }
   if (!readDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *)&loaded_ok, false)) {
        startup_printf("%s[%d]: readDataWord failed\n", FILE__, __LINE__);
        return false;
   }
   if(!loaded_ok)
   {
	   startup_printf("%s[%d]: DYNINSTinit not called automatically\n", FILE__, __LINE__);
   }

   // Install a breakpoint in DYNINSTtrapFunction.
   // This is used as RT signal.
   Address addr = getRTTrapFuncAddr();
   if (addr == 0) {
       startup_printf("%s[%d]: Cannot find DYNINSTtrapFunction. Needed as RT signal\n", FILE__, __LINE__);
       return false;
   }
   if (!setBreakpoint(addr)) {
       startup_printf("%s[%d]: Cannot set breakpoint in DYNINSTtrapFunction.\n", FILE__, __LINE__);
       return false;
   }
   startup_printf("%s[%d]: DYNINSTinit succeeded\n", FILE__, __LINE__);
   return setRTLibInitParams();
}

// Set up the parameters for DYNINSTinit in the RT lib
bool PCProcess::setRTLibInitParams() {
    startup_printf("%s[%d]: welcome to PCProcess::setRTLibInitParams\n",
            FILE__, __LINE__);

    int pid = P_getpid();


    // Now we write these variables into the following global vrbles
    // in the dyninst library:
    // libdyninstAPI_RT_init_localCause
    // libdyninstAPI_RT_init_localPid

    std::vector<int_variable *> vars;


    if (!findVarsByAll("libdyninstAPI_RT_init_localPid", vars)) {
        if (!findVarsByAll("_libdyninstAPI_RT_init_localPid", vars)) {
            startup_printf("%s[%d]: could not find necessary internal variable\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    assert(vars.size() >= 1);
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

    assert(vars.size() >= 1);
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

    assert(vars.size() >= 1);
    if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *) &dyn_debug_rtlib)) {
        startup_printf("%s[%d]: writeDataWord failed\n", FILE__, __LINE__);
        return false;
    }
    vars.clear();
    if (dyn_debug_rtlib) {
        fprintf(stderr, "%s[%d]:  set var in RTlib for debug...\n", FILE__, __LINE__);
    }

    int static_mode = 0;
    if (!findVarsByAll("DYNINSTstaticMode", vars)) {
        if (!findVarsByAll("DYNINSTstaticMode", vars)) {
            startup_printf("%s[%d]: could not find necessary internal variable\n",
                    FILE__, __LINE__);
            return false;
        }
    }

    assert(vars.size() >= 1);
    if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *) &static_mode)) {
        startup_printf("%s[%d]: writeDataWord failed\n", FILE__, __LINE__);
        return false;
    }
    vars.clear();
    return true;
}


bool PCProcess::insertBreakpointAtMain() {
    if( main_function_ == NULL ) {
        startup_printf("%s[%d]: main function not yet found, cannot insert breakpoint\n",
                FILE__, __LINE__);
        return false;
    }
    Address addr = main_function_->addr();

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

bool PCProcess::removeBreakpointAtMain() {
    if( main_function_ == NULL || mainBrkPt_ == Breakpoint::ptr() ) {
        startup_printf("%s[%d]: no breakpoint set at main function, not removing\n",
                FILE__, __LINE__);
        return true;
    }

    Address addr = main_function_->addr();

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

    forcedTerminating_ = true;

    proccontrol_printf("%s[%d]: Terminating process %d\n", FILE__, __LINE__, getPid());
    if( !pcProc_->terminate() ) {
        proccontrol_printf("%s[%d]: Failed to terminate process %d\n", FILE__, __LINE__, 
                getPid());
        return false;
    }
    PCEventMuxer::handle();

    proccontrol_printf("%s[%d]: finished terminating process %d\n", FILE__, __LINE__, getPid());

    return true;
}

bool PCProcess::detachProcess(bool cont = true) {
    if( isTerminated() ) return true;

    if( !isAttached() ) return false;

    if (tracedSyscalls_) {
        // Process needs to be stopped to change instrumentation
        bool needToContinue = false;
        if( !isStopped() ) {
            needToContinue = true;
            if( !stopProcess() ) {
                proccontrol_printf("%s[%d]: failed to stop process for removing syscalls\n",
                        FILE__, __LINE__);
		        return false;
            }
        }

        tracedSyscalls_->removePreFork();
        tracedSyscalls_->removePostFork();
        tracedSyscalls_->removePreExec();
        tracedSyscalls_->removePostExec();
        tracedSyscalls_->removePreExit();
        tracedSyscalls_->removePreLwpExit();
        if (cont) {
            if( needToContinue ) {
                if( !continueProcess() ) {
                    proccontrol_printf("%s[%d]: failed to continue process after removing syscalls\n",
                            FILE__, __LINE__);
                }
            }
        }
    }

    // TODO figure out if ProcControl should care about continuing a process
    // after detach

    // NB: it's possible to get markExited() while handling events for the
    // tracedSyscalls_->remove* calls above, clearing pcProc_.
    if( isTerminated() || pcProc_->detach(!cont) ) {
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
    pcProc_.reset();
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

    if( amount > 0 ) {
       const unsigned char *buffer = (const unsigned char *)inSelf;
       for(unsigned i = 0; i < amount-1; ++i) {
           if( i % 10 == 0 ) write_printf("\n");
           write_printf("0x%02hhx, ", buffer[i]);
       }
       write_printf("0x%02hhx", buffer[amount-1]);
    }
    write_printf("\n};\n");
}

bool PCProcess::writeDataSpace(void *inTracedProcess, u_int amount,
                               const void *inSelf) {
    if( isTerminated() ) {
       cerr << "Writing to terminated process!" << endl;
       return false;
    }
    bool result = pcProc_->writeMemory((Address)inTracedProcess, inSelf,
                                       amount);

    if( BPatch_defensiveMode == proc()->getHybridMode() && !result ) {
        // the write may have failed because we've removed write permissions
        // from the page, remove them and try again

        PCMemPerm origRights, rights(true, true, true);
        if (!pcProc_->setMemoryAccessRights((Address)inTracedProcess,
                                            amount, rights, origRights)) {
            cerr << "Fail to set memory permissions!" << endl;
            return false;
        }

        /*
        int oldRights = pcProc_->setMemoryAccessRights((Address)inTracedProcess,
                                                       amount,
                                                       PAGE_EXECUTE_READWRITE);

        if( oldRights == PAGE_EXECUTE_READ || oldRights == PAGE_READONLY ) {
        */

        if( origRights.isRX() || origRights.isR() ) {
            result = pcProc_->writeMemory((Address)inTracedProcess, inSelf,
                                          amount);

            /*
            if( pcProc_->setMemoryAccessRights((Address)inTracedProcess,
                                               amount, oldRights) == false ) {
            */

            PCMemPerm tmpRights;
            if( !pcProc_->setMemoryAccessRights((Address)inTracedProcess,
                                                amount, origRights, tmpRights)) {
                result = false;
            }
        } else {
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
    bool result = pcProc_->writeMemory((Address)inTracedProcess, inSelf, amount);
    if( result && dyn_debug_write ) writeDebugDataSpace(inTracedProcess, amount, inSelf);
    return result;
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
    map<dynthread_t, PCThread *>::iterator result;
    result = threadsByTid_.find(tid);

    if( result == threadsByTid_.end() ) return false;

    PCThread *toDelete = result->second;

    //if( !unregisterThread(toDelete) ) return false;

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
extern Address getVarAddr(PCProcess *proc, std::string str);

#if 0
bool PCProcess::registerThread(PCThread *thread) {
  
   Address tid = (Address) thread->getTid();
   Address index = thread->getIndex();
   
   Address tmp = 0;
   unsigned ptrsize = getAddressWidth();

   if (tid == (Address) -1) return true;
   if (index == (Address) -1) return true;

   if (!initializeRegisterThread()) {
      startup_printf("%s[%d]: initializeRegisterThread failed\n",
                     FILE__, __LINE__);
	   
	   return false;
   }
   // Must match the "hash" algorithm used in the RT lib
   int working = (tid % thread_hash_size);
   while(1) {
      tmp = 0;
      if (!readDataWord(( void *)(thread_hash_indices + (working * ptrsize)), ptrsize, &tmp, false)) {
         startup_printf("%s[%d]: Failed to read index slot, base 0x%lx, active 0x%lx\n", FILE__, __LINE__,
                        thread_hash_indices, thread_hash_indices + (working * ptrsize));
         return false;
      }
      startup_printf("%s[%d]: value of tid in slot %p is 0x%lx\n",
                     FILE__, __LINE__, thread_hash_indices + (working * ptrsize), tmp);
      if (ptrsize == 4 && tmp == 0xffffffff) {
         int index_int = (int) index;
         int tid_int = (int) tid;
         startup_printf("%s[%d]: writing %d to %p and 0x%x to %p\n",
                        FILE__, __LINE__, index_int, thread_hash_indices + (working * ptrsize),
                        tid_int, thread_hash_tids + (working * ptrsize));
         writeDataWord(( void *)(thread_hash_indices + (working * ptrsize)), ptrsize, &index_int);
         writeDataWord(( void *)(thread_hash_tids + (working * ptrsize)), ptrsize, &tid_int);
         break;
      }
      else if (ptrsize == 8 && tmp == (Address)-1)  {
         writeDataWord(( void *)(thread_hash_indices + (working * ptrsize)), ptrsize, &index);
         writeDataWord(( void *)(thread_hash_tids + (working * ptrsize)), ptrsize, &tid);
         break;
      }
      working++;
      if (working == thread_hash_size) working = 0;
      if (working == (int) (tid % thread_hash_size)) {
         startup_printf("%s[%d]: Failed to find empty tid slot\n", FILE__, __LINE__);
         return false;
      }
   }
   return true;
}
bool PCProcess::unregisterThread(PCThread *thread) {	
   return true;
   Address tid = (Address) thread->getTid();
   Address index = thread->getIndex();
   Address tmp = 0;
   
   unsigned ptrsize = getAddressWidth();
   if (tid == (Address) -1) return true;
   if (index == (Address) -1) return true;

   initializeRegisterThread();

   // Must match the "hash" algorithm used in the RT lib
   int working = tid % thread_hash_size;
   while(1) {
      tmp = 0;
      if (!readDataWord((void *)(thread_hash_tids + (working * ptrsize)), ptrsize, &tmp, false)) return false;
      if (tmp == tid) {
         // Zero it out
         tmp = (Address) -1;
         writeDataWord(( void *)(thread_hash_indices + (working * ptrsize)), ptrsize, &tmp);
         break;
      }
      working++;
      if (working == thread_hash_size) working = 0;
      if (working == (int) (tid % thread_hash_size)) return false;
   }
   return true;
}

bool PCProcess::initializeRegisterThread() {
//   if (thread_hash_tids) return true;

   unsigned ptrsize = getAddressWidth();
   
   Address tidPtr = getVarAddr(this, "DYNINST_thread_hash_tids");
   if (!tidPtr) return false;
   Address indexPtr = getVarAddr(this, "DYNINST_thread_hash_indices");
   if (!indexPtr) return false;
   Address sizePtr = getVarAddr(this, "DYNINST_thread_hash_size");
   if (!sizePtr) return false;
   
   if (!readDataWord((const void *)tidPtr, ptrsize, &thread_hash_tids, false)) return false;

   if (!readDataWord((const void *)indexPtr, ptrsize, &thread_hash_indices, false)) return false;

   if (!readDataWord((const void *)sizePtr, sizeof(int), &thread_hash_size, false)) return false;

   return true;
}
#endif


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
   assert(pcProc_);
   return pcProc_->getMemoryPageSize();
}

int PCProcess::getPid() const {
    return savedPid_;
}

int PCProcess::incrementThreadIndex() {
    int ret = curThreadIndex_;
    curThreadIndex_++;
    return ret;
}

PCEventHandler * PCProcess::getPCEventHandler() const {
    return eventHandler_;
}

bool PCProcess::walkStacks(std::vector<std::vector<Frame> > &stackWalks) {
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

        std::vector<Frame> stackWalk;
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
bool PCProcess::getAllActiveFrames(std::vector<Frame> &activeFrames) {
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

#define HEAP_DYN_BUF_SIZE (0x100000)

static const Address ADDRESS_LO = ((Address)0x10000);
static const Address ADDRESS_HI = ((Address)~((Address)0));

Address PCProcess::inferiorMalloc(unsigned size, inferiorHeapType type,
                                  Address near_, bool *err) 
{
   if(bootstrapState_ <= bs_readyToLoadRTLib) {
      return 0;
   }

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

    inferiorMallocAlign(size); // align size
    // Set the lo/hi constraints (if necessary)
    inferiorMallocConstraints(near_, lo, hi, type);

    infmalloc_printf("%s[%d]: inferiorMalloc entered; size %d, type %d, near 0x%lx (0x%lx to 0x%lx)\n",
                     FILE__, __LINE__, size, type, near_, lo, hi);

    // find free memory block (multiple attempts)
    int freeIndex = -1;
    int ntry = 0;
    for (ntry = 0; freeIndex == -1; ntry++) {
        switch(ntry) {
        case AsIs: 
            infmalloc_printf("%s[%d]:  (1) AsIs\n", FILE__, __LINE__);
            break;
	    //#if defined(cap_dynamic_heap)
        case DeferredFree: 
            infmalloc_printf("%s[%d]:  (2) garbage collecting and compacting\n",
                             FILE__, __LINE__);
            inferiorFreeCompact();
            break;
        case NewSegment1MBConstrained: 
            infmalloc_printf("%s[%d]:  (3) inferiorMallocDynamic "
                    "for %d (0x%x) bytes between 0x%lx - 0x%lx\n", FILE__, __LINE__,
                    HEAP_DYN_BUF_SIZE, HEAP_DYN_BUF_SIZE, lo, hi);
            inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
            break;
        case NewSegmentSizedConstrained: 
            infmalloc_printf("%s[%d]:  (4) inferiorMallocDynamic "
                    "for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, size, size, lo, hi);
            inferiorMallocDynamic(size, lo, hi);
            break;
        case RemoveRangeConstraints: 
            infmalloc_printf("%s[%d]:  (5) inferiorMalloc: removing range constraints\n",
                             FILE__, __LINE__);
            lo = ADDRESS_LO;
            hi = ADDRESS_HI;
            if (err) {
                infmalloc_printf("%s[%d]: error in inferiorMalloc\n", FILE__, __LINE__);
                *err = true;
            }
            break;
        case NewSegment1MBUnconstrained: 
            infmalloc_printf("%s[%d]:  (6) inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, HEAP_DYN_BUF_SIZE, HEAP_DYN_BUF_SIZE, lo, hi);
            inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
            break;
        case NewSegmentSizedUnconstrained: 
            infmalloc_printf("%s[%d]:  (7) inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, size, size, lo, hi);
            inferiorMallocDynamic(size, lo, hi);
            break;
        case DeferredFreeAgain: 
            infmalloc_printf("%s[%d]: inferiorMalloc: recompacting\n", FILE__, __LINE__);
            inferiorFreeCompact();
            break;
	    //#else /* !(cap_dynamic_heap) */
	    //case DeferredFree: // deferred free, compact free blocks
            //inferiorFreeCompact();
            //break;
	    //#endif /* cap_dynamic_heap */

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
	if(bootstrapState_ <= bs_readyToLoadRTLib) {
      return true;
   }
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
    std::vector<AstNodePtr> args(3);
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

// A copy of the BPatch-level instrumentation installer
void PCProcess::installInstrRequests(const std::vector<instMapping*> &requests) {
    if (requests.size() == 0) {
        return;
    }

    // Instrumentation is now generated on a per-function basis, while
    // the requests are per-inst, not per-function. So 
    // accumulate functions, then generate afterwards. 

    vector<func_instance *> instrumentedFuncs;

    for (unsigned lcv=0; lcv < requests.size(); lcv++) {
      inst_printf("%s[%d]: handling request %d of %d\n", FILE__, __LINE__, lcv+1, requests.size());

        instMapping *req = requests[lcv];
        
        if(!multithread_capable() && req->is_MTonly())
            continue;
        
        std::vector<func_instance *> matchingFuncs;
        
        if (!findFuncsByAll(req->func, matchingFuncs, req->lib)) {
            inst_printf("%s[%d]: failed to find any functions matching %s (lib %s), returning failure from installInstrRequests\n", FILE__, __LINE__, req->func.c_str(), req->lib.c_str());
            return;
        }
        else {
            inst_printf("%s[%d]: found %d functions matching %s (lib %s), instrumenting...\n",
                        FILE__, __LINE__, matchingFuncs.size(), req->func.c_str(), req->lib.c_str());
        }

        for (unsigned funcIter = 0; funcIter < matchingFuncs.size(); funcIter++) {
           func_instance *func = matchingFuncs[funcIter];
           if (!func) {
              inst_printf("%s[%d]: null int_func detected\n",
                          FILE__,__LINE__);
              continue;  // probably should have a flag telling us whether errors
           }

	   inst_printf("%s[%d]: Instrumenting %s at 0x%lx, offset 0x%lx in %s\n",
		       FILE__, __LINE__, 
		       func->symTabName().c_str(),
		       func->addr(),
		       func->addr() - func->obj()->codeBase(),
		       func->obj()->fullName().c_str());
            
           // should be silently handled or not
           AstNodePtr ast;
           if ((req->where & FUNC_ARG) && req->args.size()>0) {
              ast = AstNode::funcCallNode(req->inst, 
                                          req->args,
                                          this);
           }
           else {
              std::vector<AstNodePtr> def_args;
              def_args.push_back(AstNode::operandNode(AstNode::Constant,
                                                      (void *)0));
              ast = AstNode::funcCallNode(req->inst,
                                          def_args);
           }
           // We mask to strip off the FUNC_ARG bit...
           std::vector<Point *> points;
           switch ( ( req->where & 0x7) ) {
              case FUNC_EXIT:
                 mgr()->findPoints(Dyninst::PatchAPI::Scope(func),
                                   Point::FuncExit,
                                   std::back_inserter(points));
                 break;
              case FUNC_ENTRY:
                 mgr()->findPoints(Dyninst::PatchAPI::Scope(func),
                                   Point::FuncEntry,
                                   std::back_inserter(points));
                 break;
              case FUNC_CALL:
                 mgr()->findPoints(Dyninst::PatchAPI::Scope(func),
                                   Point::PreCall,
                                   std::back_inserter(points));
                 break;
              default:
                 fprintf(stderr, "Unknown where: %d\n",
                         req->where);
                 break;
           } // switch
	   inst_printf("%s[%d]: found %d points to instrument\n", FILE__, __LINE__, points.size());
           for (std::vector<Point *>::iterator iter = points.begin();
                iter != points.end(); ++iter) {
              Dyninst::PatchAPI::Instance::Ptr inst = (req->order == orderFirstAtPoint) ? 
                 (*iter)->pushFront(ast) :
                 (*iter)->pushBack(ast);
              if (inst) {
                 if (!req->useTrampGuard) inst->disableRecursiveGuard();
                 req->instances.push_back(inst);
              }
              else {
                 fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
              }
           }        } // matchingFuncs        
        
    } // requests
    relocate();
    return;
}

static const unsigned MAX_IRPC_SIZE = 0x100000;


bool PCProcess::postIRPC(void* buffer, int size, void* userData, bool runProcessWhenDone,
                         PCThread* thread, bool synchronous, void** result,
                         bool userRPC, bool isMemAlloc, Address addr)
{
   return postIRPC_internal(buffer,
                            size,
                            size,
                            REG_NULL,
                            addr,
                            userData,
                            runProcessWhenDone,
                            thread,
                            synchronous,
                            userRPC,
                            isMemAlloc,
                            result);    
}

bool PCProcess::postIRPC(AstNodePtr action, void *userData, 
                         bool runProcessWhenDone, PCThread *thread, bool synchronous,
                         void **result, bool userRPC, bool isMemAlloc, Address addr)
{   
   // Generate the code for the iRPC
   codeGen irpcBuf(MAX_IRPC_SIZE);
   irpcBuf.setAddrSpace(this);
   irpcBuf.setRegisterSpace(registerSpace::irpcRegSpace(proc()));
   irpcBuf.beginTrackRegDefs();
   irpcBuf.setThread(thread);
   
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

   irpcTramp_->setIRPCAST(action);
   
   // Create a stack frame for the RPC
   if( !irpcTramp_->generateSaves(irpcBuf, irpcBuf.rs()) ) {
      proccontrol_printf("%s[%d]: failed to generate saves via baseTramp\n",
                         FILE__, __LINE__);
      return false;
   }
   
   Register resultReg = REG_NULL;
   if( !action->generateCode(irpcBuf, false, resultReg) ) {
      proccontrol_printf("%s[%d]: failed to generate code from AST\n",
                         FILE__, __LINE__);
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

    //#sasha printing code patch for DYNINSTos_malloc
    //cerr << "BUFFER for IRPC" << endl;
    //cerr << irpcBuf.format() << endl;

    return postIRPC_internal(irpcBuf.start_ptr(),
                             irpcBuf.used(),
                             breakOffset,
                             resultReg,
                             addr,
                             userData,
                             runProcessWhenDone,
                             thread,
                             synchronous,
                             userRPC,
                             isMemAlloc,
                             result);    
}

// DEBUG
#include "instructionAPI/h/InstructionDecoder.h"

bool PCProcess::postIRPC_internal(void *buf,
                                  unsigned size,
                                  unsigned breakOffset,
                                  Register resultReg,
                                  Address addr,
                                  void *userData,
                                  bool runProcessWhenDone,
                                  PCThread *thread,
                                  bool synchronous,
                                  bool userRPC,
                                  bool isMemAlloc,
                                  void **result) {
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


   inferiorRPCinProgress *newRPC = new inferiorRPCinProgress;
   newRPC->runProcWhenDone = runProcessWhenDone;
   newRPC->deliverCallbacks = userRPC;
   newRPC->userData = userData;
   newRPC->synchronous = synchronous;

   newRPC->resultRegister = resultReg;
   
   // Create the iRPC at the ProcControl level
   if( addr == 0 ) {
      bool err = false;
      if( isMemAlloc ) {
         // This assumes that there will always be space
         addr = inferiorMalloc(size, lowmemHeap, 0, &err);
      }else{
         // recursive RPCs are okay when this isn't an inferiorMalloc RPC
         addr = inferiorMalloc(size, anyHeap, 0, &err);
      }
      
      if( err ) {
         proccontrol_printf("%s[%d]: failed to allocate memory for RPC\n",
                            FILE__, __LINE__);
         delete newRPC;
         return false;
      }
      newRPC->memoryAllocated = true;
   }
   
    if (addr)
       newRPC->rpc = IRPC::createIRPC(buf, size, addr);
    else
       newRPC->rpc = IRPC::createIRPC(buf, size);

#if 0
   // DEBUG
   InstructionAPI::InstructionDecoder d(buf,size,getArch());
   Address foo = addr;
   InstructionAPI::Instruction::Ptr insn = d.decode();
   while(insn) {
      cerr << "\t" << hex << foo << ": " << insn->format(foo) << dec << endl;
      foo += insn->size();
      insn = d.decode();
   }
#endif
    newRPC->rpc->setData(newRPC);

    unsigned int start_offset = 0;
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
    //
    // Matt Note:  The above comment is slightly incorrect.  The kernel subracts
    //  the length of the syscall/int instruction that triggered the system call,
    //  not the address width.  Still address width is big enough, so I'm not
    //  changing anything.
    start_offset = proc()->getAddressWidth();
    newRPC->rpcStartAddr += start_offset;
#endif
    newRPC->rpc->setStartOffset(start_offset);
    newRPC->rpcCompletionAddr = addr + breakOffset;

    // Post the iRPC
    Thread::ptr t;
    if (thread) {
       t = thread->pcThr();
    }
    newRPC->thread = t;
    
    bool res = false;
    proccontrol_printf("%s[%d]: Launching IRPC\n", FILE__, __LINE__);
    if (synchronous) {
       // We have an interesting problem here. ProcControl allows callbacks to specify whether the 
       // process should stop or run; however, that allows us to stop a process in the middle of an
       // inferior RPC. If that happens, manually execute a continue and wait for completion ourselves.
       if (t)
          res = t->runIRPCSync(newRPC->rpc);
       else
          res = pcProc_->runIRPCSync(newRPC->rpc);
       if (!res) {
          bool done = false;
          while (!done) {
             proccontrol_printf("%s[%d]: Iterating in loop waiting for IRPC to complete\n", FILE__, __LINE__);
             if (isTerminated()) {
                fprintf(stderr, "IRPC on terminated process, ret false!\n");
                delete newRPC;
                return false;
             }

            if (ProcControlAPI::getLastError() != ProcControlAPI::err_notrunning) {
                // Something went wrong
               proccontrol_printf("%s[%d]: failed to post %s RPC to %s, error %s\n",
                                  FILE__, __LINE__, (synchronous ? "sync" : "async"), 
                                  ((thread == NULL) ? "thread" : "process"),
                                  ProcControlAPI::getLastErrorMsg());
               delete newRPC;
               return false;
            }
            else {
               proccontrol_printf("%s[%d]: ProcControl reported IRPC thread stopped, continuing and consuming events\n", FILE__, __LINE__);
               newRPC->rpc->continueStoppedIRPC();
               proccontrol_printf("%s[%d]: handling events in ProcControl\n", FILE__, __LINE__);
               res = pcProc_->handleEvents(true);
               PCEventMuxer::muxer().handle(NULL);
               if (newRPC->rpc->state() == ProcControlAPI::IRPC::Done) {
                  proccontrol_printf("%s[%d]: IRPC complete\n", FILE__, __LINE__);
                  done = true;
               }
            }
          }
       }
    }
    else {
       if (t)
          res = t->runIRPCAsync(newRPC->rpc);
       else
          res = pcProc_->runIRPCAsync(newRPC->rpc);
    }
    if(!res) {
       proccontrol_printf("%s[%d]: failed to post %s RPC to %s\n",
                          FILE__, __LINE__, (synchronous ? "sync" : "async"), ((thread == NULL) ? "thread" : "process"));
       delete newRPC;
       return false;
    }
    
    if( result ) {
       *result = newRPC->returnValue;
    }
    
    // Make sure Dyninst has worked everything out
    PCEventMuxer::muxer().wait(false);

   return true;
}


BPatch_hybridMode PCProcess::getHybridMode() {
    return analysisMode_;
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
    std::list<std::pair<Address,Address> >& overwrittenRanges,//output
    std::list<block_instance *> &writtenBBIs)//output
{
    const unsigned MEM_PAGE_SIZE = getMemoryPageSize();
    unsigned char * memVersion = (unsigned char *) ::malloc(MEM_PAGE_SIZE);
    Address regionStart = 0;
    bool foundStart = false;
    map<Address, unsigned char*>::iterator pIter = overwrittenPages.begin();
    set<mapped_object*> owObjs;
    for (; pIter != overwrittenPages.end(); pIter++) {
        Address curPageAddr = (*pIter).first / MEM_PAGE_SIZE * MEM_PAGE_SIZE;
        unsigned char *curShadow = (*pIter).second;

        // 0. check to make sure curShadow is non-null, if it is null, 
        //    that means it hasn't been written to
        if ( ! curShadow ) {
                        cerr << "\t\t No current shadow, continuing" << endl;
                        continue;
        }

        mapped_object* obj = findObject(curPageAddr);
        if (owObjs.end() != owObjs.find(obj)) {
            obj->setCodeBytesUpdated(false);
        }

        // 1. Read the modified page in from memory
        Address readAddr = curPageAddr;
        if (isMemoryEmulated()) {
            bool valid = false;
            boost::tie(valid,readAddr) = getMemEm()->translate(curPageAddr);
                        cerr << "\t\t Reading from shadow page " << hex << readAddr << " instead of original " << curPageAddr << endl;
            assert(valid);
        }
        readTextSpace((void*)readAddr, MEM_PAGE_SIZE, memVersion);

        // 2. build overwritten region list by comparing shadow, memory
        for (unsigned mIdx = 0; mIdx < MEM_PAGE_SIZE; mIdx++) {
            if ( ! foundStart && curShadow[mIdx] != memVersion[mIdx] ) {
                foundStart = true;
                regionStart = curPageAddr+mIdx;
            } else if (foundStart && curShadow[mIdx] == memVersion[mIdx]) {
                foundStart = false;
                                cerr << "\t\t Adding overwritten range " << hex << regionStart << " -> " << curPageAddr + mIdx << dec << endl;

                overwrittenRanges.push_back(
                    pair<Address,Address>(regionStart,curPageAddr+mIdx));
            }
        }
        if (foundStart) {

            foundStart = false;
                        cerr << "\t\t Adding overwritten range " << hex << regionStart << " -> " << curPageAddr + MEM_PAGE_SIZE << dec << endl;

            overwrittenRanges.push_back(
                pair<Address,Address>(regionStart,curPageAddr+MEM_PAGE_SIZE));
        }
    }

    // 3. Determine which basic blocks have been overwritten
    list<pair<Address,Address> >::const_iterator rIter = overwrittenRanges.begin();
    std::list<block_instance*> curBBIs;
    while (rIter != overwrittenRanges.end()) {
        mapped_object *curObject = findObject((*rIter).first);

        curObject->findBlocksByRange((*rIter).first,(*rIter).second,curBBIs);
        if (curBBIs.size()) {
            mal_printf("overwrote %d blocks in range %lx %lx \n",
                       curBBIs.size(),(*rIter).first,(*rIter).second);
            writtenBBIs.splice(writtenBBIs.end(), curBBIs);
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
void PCProcess::updateCodeBytes
    ( const list<pair<Address, Address> > &owRanges ) // input
{
    std::map<mapped_object *,list<pair<Address,Address> >*> objRanges;
    list<pair<Address,Address> >::const_iterator rIter = owRanges.begin();
    for (; rIter != owRanges.end(); rIter++) {
        mapped_object *obj = findObject((*rIter).first);
        if (objRanges.find(obj) == objRanges.end()) {
            objRanges[obj] = new list<pair<Address,Address> >();
        }
        objRanges[obj]->push_back(pair<Address,Address>(rIter->first, rIter->second));
    }

    std::map<mapped_object *,list<pair<Address,Address> > *>::iterator oIter = 
        objRanges.begin();
    for (; oIter != objRanges.end(); oIter++) 
    {
        oIter->first->updateCodeBytes( *(oIter->second) );
        delete (oIter->second);
    }
    assert(objRanges.size() <= 1); //o/w analysis code may not be prepared for other cases
}

#if 0
static void otherFuncBlocks(func_instance *func, 
                            const set<block_instance*> &blks, 
                            set<block_instance*> &otherBlks)
{
    const func_instance::BlockSet &allBlocks = 
        func->blocks();
    for (func_instance::BlockSet::const_iterator bit =
         allBlocks.begin();
         bit != allBlocks.end(); 
         bit++) 
    {
        if (blks.end() == blks.find((*bit))) {
            otherBlks.insert((*bit));
        }
    }
}
#endif

/* Summary
 * Given a list of overwritten blocks, find blocks that are unreachable,
 * functions that have been overwritten at their entry points and can go away,
 * and new function entry for functions that are being overwritten while still
 * executing
 *
 * variables
 * f:  the overwritten function
 * ow: the set of overwritten blocks
 * ex: the set of blocks that are executing on the call stack that were not overwritten
 * 
 * primitives
 * R(b,s): yields set of reachable blocks for collection of blocks b, starting
 *         at seed blocks s.
 * B(f):   the blocks pertaining to function f
 * EP(f):  the entry point of function f
 * F(b):   functions containing block b
 * 
 * calculations
 * Elim(f): the set of blocks to eliminate from function f.
 *          Elim(f) = B(f) - R( B(f)-ow , EP(f) )
 * New(f):  new function entry candidates for f's surviving blocks.
 *          If EB(f) not in ow(f), empty set
 *          Else, all blocks b such that ( b in ex AND e in Elim(f) )
 *          Eliminate New(f) elements that have ancestors in New(f)
 * Del(f):  A block can be deleted altogether if
 *          forall f in F(b): B(F) - R( B(f) - ow , New(f) U (EP(f) \ ow(f)) U (ex(f) intersect Elim(f)) ),
 *          b is not in the resulting set. In other words, b is not
 *          reachable from non-overwritten blocks in the functions in
 *          which it appears, seeded at new entry points and original
 *          non-overwritten entry points to the function, and at f's
 *          executing blocks if these will be deleted from the
 *          function (they constitute an entry point into the function 
 *          even if they've been overwritten). 
 * DeadF:   the set of functions that have no executing blocks 
 *          and were overwritten in their entry blocks
 *          EP(f) in ow(f) AND ex(f) is empty
 */
bool PCProcess::getDeadCode
( const std::list<block_instance*> & /*owBlocks*/, // input
  std::set<block_instance*> & /*delBlocks*/, //output: Del(for all f)
  std::map<func_instance*,set<block_instance*> > & /*elimMap*/, //output: elimF
  std::list<func_instance*> & /*deadFuncs*/, //output: DeadF
  std::map<func_instance*,block_instance*> & /*newFuncEntries*/) //output: newF
{
   assert(0 && "TODO");
   return false;
#if 0
    // do a stackwalk to see which functions are currently executing
    std::vector<std::vector<Frame> >  stacks;
    std::vector<Address> pcs;
    if (!walkStacks(stacks)) {
        inst_printf("%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
        return false;
    }
    for (unsigned i = 0; i < stacks.size(); ++i) {
        std::vector<Frame> &stack = stacks[i];
        for (unsigned int j = 0; j < stack.size(); ++j) {
            Address origPC = 0;
            vector<func_instance*> dontcare1;
            baseTramp *dontcare2 = NULL;
            getAddrInfo(stack[j].getPC(), origPC, dontcare1, dontcare2);
            pcs.push_back( origPC );
        }
    }

    // group blocks by function
    std::map<func_instance*,set<block_instance*> > deadMap;
    std::set<func_instance*> deadEntryFuncs;
    std::set<Address> owBlockAddrs;
    for (list<block_instance*>::const_iterator bIter=owBlocks.begin();
         bIter != owBlocks.end(); 
         bIter++) 
    {
       deadMap[(*bIter)->func()].insert(*bIter);
       owBlockAddrs.insert((*bIter)->start());
       if ((*bIter)->llb() == (*bIter)->func()->ifunc()->entry()) {
          deadEntryFuncs.insert((*bIter)->func());
       }
    }

    // for each modified function, calculate ex, ElimF, NewF, DelF
    for (map<func_instance*,set<block_instance*> >::iterator fit = deadMap.begin();
         fit != deadMap.end(); 
         fit++) 
    {

        // calculate ex(f)
        set<block_instance*> execBlocks;
        for (unsigned pidx=0; pidx < pcs.size(); pidx++) {
            std::set<block_instance *> candidateBlocks;
            fit->first->findBlocksByAddr(pcs[pidx], candidateBlocks);
            for (std::set<block_instance *>::iterator cb_iter = candidateBlocks.begin();
                cb_iter != candidateBlocks.end(); ++cb_iter) {
                block_instance *exB = *cb_iter;
                if (exB && owBlockAddrs.end() == owBlockAddrs.find(
                                                        exB->start())) 
                {
                    execBlocks.insert(exB);
                }
            }
        }

        // calculate DeadF: EP(f) in ow and EP(f) not in ex
        if ( 0 == execBlocks.size() ) {
            set<block_instance*>::iterator eb = fit->second.find(
                fit->first->entryBlock());
            if (eb != fit->second.end()) {
                deadFuncs.push_back(fit->first);
                continue;// treated specially, don't need elimF, NewF or DelF
            }
        } 

        // calculate elimF
        set<block_instance*> keepF;
        list<block_instance*> seedBs;
        seedBs.push_back(fit->first->entryBlock());
        fit->first->getReachableBlocks(fit->second, seedBs, keepF);
        otherFuncBlocks(fit->first, keepF, elimMap[fit->first]);

        // calculate NewF
        if (deadEntryFuncs.end() != deadEntryFuncs.find(fit->first)) {
            for (set<block_instance*>::iterator bit = execBlocks.begin();
                 bit != execBlocks.end();
                 bit++) 
            {
                if (elimMap[fit->first].end() != 
                    elimMap[fit->first].find(*bit)) 
                {
                    newFuncEntries[fit->first] = *bit;
                    break; // just need one candidate
                }
            }
        }

        // calculate Del(f)
        seedBs.clear();
        if (deadEntryFuncs.end() == deadEntryFuncs.find(fit->first)) {
            seedBs.push_back(fit->first->entryBlock());
        }
        else if (newFuncEntries.end() != newFuncEntries.find(fit->first)) {
            seedBs.push_back(newFuncEntries[fit->first]);
        }
        for (set<block_instance*>::iterator xit = execBlocks.begin();
             xit != execBlocks.end();
             xit++) 
        {
            if (elimMap[fit->first].end() != elimMap[fit->first].find(*xit)) {
                seedBs.push_back(*xit);
            }
        }
        keepF.clear();
        fit->first->getReachableBlocks(fit->second, seedBs, keepF);
        otherFuncBlocks(fit->first, keepF, delBlocks);
        
    }

    return true;
#endif
}

// will flush addresses of all addresses in the specified range, if the
// range is null, flush all addresses from the cache.  Also flush 
// rt-lib heap addrs that correspond to the range
void PCProcess::flushAddressCache_RT(Address start, unsigned size)
{
    if (start != 0) {
        mal_printf("Flushing address cache of range [%lx %lx]\n",
                   start, 
                   start + size);
    } else {
        mal_printf("Flushing address cache of rt_lib heap addrs only \n");
    }

    // Find the runtime cache's address if it hasn't been set yet
    if (0 == RT_address_cache_addr_) {
        std::string arg_str ("DYNINST_target_cache");
        std::vector<int_variable *> vars;
        if ( ! findVarsByAll(arg_str, vars) ) {
            fprintf(stderr, "%s[%d]:  cannot find var %s\n", 
                    FILE__, __LINE__, arg_str.c_str());
            assert(0);
        }
        if (vars.size() != 1) {
            fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
                    FILE__, __LINE__, (int)vars.size(), arg_str.c_str());
            assert(0);
        }
        RT_address_cache_addr_ = vars[0]->getAddress();
    }

    // Clear all cache entries that match the runtime library
    // Read in the contents of the cache
    Address* cacheCopy = (Address*)malloc(TARGET_CACHE_WIDTH*sizeof(Address));
    if ( ! readDataSpace( (void*)RT_address_cache_addr_, 
                          sizeof(Address)*TARGET_CACHE_WIDTH,(void*)cacheCopy,
                          false ) ) 
    {
        assert(0);
    }

    assert(dyninstRT_heaps_.size());
    bool flushedHeaps = false;

    while ( true ) // iterate twice, once to flush the heaps, 
    {              // and once to flush the flush range
        Address flushStart=0;
        Address flushEnd=0;
        if (!flushedHeaps) {
            // figure out the range of addresses we'll want to flush from

            flushStart = dyninstRT_heaps_[0]->addr;
            flushEnd = flushStart + dyninstRT_heaps_[0]->length;
            for (unsigned idx=1; idx < dyninstRT_heaps_.size(); idx++) {
                Address curAddr = dyninstRT_heaps_[idx]->addr;
                if (flushStart > curAddr) {
                    flushStart = curAddr;
                }
                curAddr += dyninstRT_heaps_[idx]->length;
                if (flushEnd < curAddr) {
                    flushEnd = curAddr;
                }
            }
        } else {
            flushStart = start;
            flushEnd = start + size;
        }
        //zero out entries that lie in the runtime heaps
        for(int idx=0; idx < TARGET_CACHE_WIDTH; idx++) {
            //printf("cacheCopy[%d]=%lx\n",idx,cacheCopy[idx]);
            if (flushStart <= cacheCopy[idx] &&
                flushEnd   >  cacheCopy[idx]) {
                cacheCopy[idx] = 0;
            }
        }
        if ( flushedHeaps || (start == 0) ) {
            break;
        }
        flushedHeaps = true;
    }

    // write the modified cache back into the RT_library
    if ( ! writeDataSpace( (void*)RT_address_cache_addr_,
                           sizeof(Address)*TARGET_CACHE_WIDTH,
                           (void*)cacheCopy ) ) {
        assert(0);
    }
    free(cacheCopy);
}

/* Given an address that's on the call stack, find the function that's
 * actively executing that address.  This makes most sense for finding the
 * address that's triggered a context switch back to Dyninst, either
 * through instrumentation or a signal
 */
func_instance *PCProcess::findActiveFuncByAddr(Address addr)
{
    std::set<func_instance *> funcs;
    // error checking by size...
    (void)findFuncsByAddr(addr, funcs, true);
    if (funcs.empty()) return NULL;

    if (funcs.size() == 1) {
        return *(funcs.begin());
    }

    // unrelocated shared function address, do a stack walk to figure 
    // out which of the shared functions is on the call stack
    bool foundFrame = false;
    func_instance *activeFunc = NULL; 
    std::vector<std::vector<Frame> >  stacks;
    if ( false == walkStacks(stacks) ) {
        fprintf(stderr,"ERROR: %s[%d], walkStacks failed\n", 
                FILE__, __LINE__);
        assert(0);
    }
    for (unsigned int i = 0; !foundFrame && i < stacks.size(); ++i) {
        std::vector<Frame> &stack = stacks[i];
        for (unsigned int j = 0; !foundFrame && j < stack.size(); ++j) {
            Frame *curFrame = &stack[j];
            Address framePC = curFrame->getPC();

            // if we're at a relocated address, we can translate 
            // back to the right function, if translation fails 
            // frameFunc will still be NULL
            RelocInfo ri;
            func_instance *frameFunc = NULL;

            if (getRelocInfo(framePC, ri) &&
                ri.func) {
               frameFunc = ri.func;
            }
            else if (j < (stack.size() - 1)) {
                // Okay, crawl original code. 
                // Step 1: get our current function
                std::set<func_instance *> curFuncs;
                findFuncsByAddr(framePC, curFuncs);
                // Step 2: get return addresses one frame up and map to possible callers
                std::set<block_instance *> callerBlocks;
                findBlocksByAddr(stack[j+1].getPC() - 1, callerBlocks);
                for (std::set<block_instance *>::iterator cb_iter = callerBlocks.begin();
                    cb_iter != callerBlocks.end(); ++cb_iter)
                {
                    if (!(*cb_iter)->containsCall()) continue;
                    // We have a call point; now see if it called the entry of any function
                    // that maps to a curFunc.
                    for (std::set<func_instance *>::iterator cf_iter = curFuncs.begin();
                         cf_iter != curFuncs.end(); ++cf_iter) {
                       if ((*cf_iter) == (*cb_iter)->callee()) {
                          frameFunc = *cf_iter;
                       }
                    }
                }
            }
            if (frameFunc) {
                foundFrame = true;
                activeFunc = frameFunc;
            }
        }
    }
    if (!foundFrame) {
        activeFunc = *(funcs.begin());
    }
                
    return activeFunc;
}

bool PCProcess::patchPostCallArea(instPoint *callPt) {
   // 1) Find all the post-call patch areas that correspond to this 
   //    call point
   // 2) Generate and install the branches that will be inserted into 
   //    these patch areas
   
   // 1...
   AddrPairSet patchAreas;
   if ( ! generateRequiredPatches(callPt, patchAreas) ) {
      return false;
   }
   
   // 2...
   generatePatchBranches(patchAreas);
   return true;
}

bool PCProcess::generateRequiredPatches(instPoint *callPoint, 
                                        AddrPairSet &patchAreas)
{
    // We need to figure out where this patch should branch to.
    // To do that, we're going to:
    // 1) Forward map the entry of the ft block to
    //    its most recent relocated version (if that exists)
    // 2) For each padding area, create a (padAddr,target) pair

    // 3)

    block_instance *callB = callPoint->block();
    block_instance *ftBlk = callB->getFallthrough()->trg();
    if (!ftBlk) {
        // find the block at the next address, if there's no fallthrough block
        ftBlk = callB->obj()->findBlockByEntry(callB->end());
        assert(ftBlk);
    }

    // ensure that we patch other callPts at the same address

    vector<ParseAPI::Function*> callFuncs;
    callPoint->block()->llb()->getFuncs(callFuncs);
    for (vector<ParseAPI::Function*>::iterator fit = callFuncs.begin();
         fit != callFuncs.end();
         fit++)
    {
        func_instance *callF = findFunction((parse_func*)*fit);
        instPoint *callP = instPoint::preCall(callF, callB);
        Relocation::CodeTracker::RelocatedElements reloc;
        CodeTrackers::reverse_iterator rit;
        for (rit = relocatedCode_.rbegin(); rit != relocatedCode_.rend(); rit++)
        {
            if ((*rit)->origToReloc(ftBlk->start(), ftBlk, callF, reloc)) {
                break;
            }
        }
        if (rit == relocatedCode_.rend()) {
            mal_printf("WARNING: no relocs of call-fallthrough at %lx "
                       "in func at %lx, will not patch its post-call "
                       "padding\n", callP->block()->last(),callF->addr());
            (*relocatedCode_.rbegin())->debug();
            continue;
        }

        Address to = reloc.instruction;
        if (!reloc.instrumentation.empty()) {
           // There could be a lot of instrumentation at this point. Bias towards the lowest,
           // non-edge instrumentation
           for (std::map<instPoint *, Address>::iterator inst_iter = reloc.instrumentation.begin();
                inst_iter != reloc.instrumentation.end(); ++inst_iter) {
              if (inst_iter->first->type() == PatchAPI::Point::EdgeDuring) continue;
              to = (inst_iter->second < to) ? inst_iter->second : to;
           }
        }

        // 2) 
        Address callInsnAddr = callP->block()->last();
        if (forwardDefensiveMap_.end() != forwardDefensiveMap_.find(callInsnAddr)) {
            map<func_instance*,set<DefensivePad> >::iterator mit = forwardDefensiveMap_[callInsnAddr].begin();
            for (; mit != forwardDefensiveMap_[callInsnAddr].end(); ++mit) {
              if (callF == mit->first) {
                  set<DefensivePad>::iterator dit = mit->second.begin();
                  for (; dit != mit->second.end(); ++dit) {
                     Address jumpAddr = dit->first;
                     patchAreas.insert(std::make_pair(jumpAddr, to));
                     mal_printf("patching post-call pad for %lx[%lx] with %lx %s[%d]\n",
                                callB->end(), jumpAddr, to, FILE__,__LINE__);
                  }
              }
            }
        }
    }
    if (patchAreas.empty()) {
       mal_printf("WARNING: no relocs to patch for call at %lx, block end %lx\n", 
                  callPoint->addr_compat(),ftBlk->start());
    }
    return ! patchAreas.empty();
}

void PCProcess::generatePatchBranches(AddrPairSet &branchesNeeded) {
  for (AddrPairSet::iterator iter = branchesNeeded.begin();
       iter != branchesNeeded.end(); ++iter) 
  {
    Address from = iter->first;
    Address to = iter->second;

    codeGen gen(64);
    insnCodeGen::generateBranch(gen, from, to);

    // Safety check: make sure we didn't overrun the patch area
    Address lb = 0, ub = 0;
    std::pair<func_instance*,Address> tmp;
    if (!reverseDefensiveMap_.find(from, lb, ub, tmp)) {
      // Huh? This worked before!
      assert(0);
    }
    assert((from + gen.used()) <= ub);
    if (!writeTextSpace((void *)from, 
			gen.used(),
			gen.start_ptr())) {
      assert(0);
    }
  }
}

/* debugSuicide is a kind of alternate debugging continueProc.  It runs the
 * process until terminated in single step mode, printing each instruction as
 * it executes.
 */
void PCProcess::debugSuicide() {
    if( isTerminated() ) return;

    isInDebugSuicide_ = true;

    std::vector<Frame> activeFrames;
    getAllActiveFrames(activeFrames);

    for(unsigned i=0; i < activeFrames.size(); ++i) {
        Address addr = activeFrames[i].getPC();
        fprintf(stderr, "Frame %u @ 0x%lx\n", i , addr);
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
    }
}

std::vector<func_instance *> PCProcess::pcsToFuncs(std::vector<Frame> stackWalk) {
    std::vector <func_instance *> ret;
    unsigned i;
    func_instance *fn;
    for(i=0;i<stackWalk.size();i++) {
        fn = (func_instance *)findOneFuncByAddr(stackWalk[i].getPC());
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
    codeRange *handlerLoc;
    if (signalHandlerLocations_.find(addr, handlerLoc)) {
        return; // we're already tracking this location
    }
    handlerLoc = new signal_handler_location(addr, size);
    signalHandlerLocations_.insert((signal_handler_location *)handlerLoc);
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

bool PCProcess::walkStack(std::vector<Frame> &stackWalk,
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
    AddressSpace::RelocInfo ri;
    if( !getRelocInfo(pointAddress, ri) ) {
        assert(0);
        return false;
    }

    // get instPoint from point address
    func_instance *pointfunc = ri.func;
    if (!pointfunc) {
        mal_printf("%s[%d]: failed to find active function at 0x%lx\n",
                FILE__, __LINE__, pointAddress);
        return false;
    }

    instPoint *intPoint = ri.bt->point();
    if (!intPoint) {
        mal_printf("%s[%d]: failed to find inst point at 0x%lx\n",
                FILE__, __LINE__, pointAddress);
        return false;
    }

    mal_printf("handling stopThread %lx[%lx]=>%lx %s[%d]\n",
            ri.reloc, pointAddress, (long)calculation, FILE__, __LINE__);

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
Address PCProcess::stopThreadCtrlTransfer (instPoint* intPoint, 
                                         Address target)
{
   Address pointAddr = intPoint->addr_compat();

    // if the point is a real return instruction and its target is a stack 
    // address, get the return address off of the stack 
    if (intPoint->type() == instPoint::FuncExit &&
        intPoint->block()->isFuncExit() &&
        !intPoint->func()->isSignalHandler()) 
    {
        mal_printf("%s[%d]: return address is %lx\n", FILE__,
                    __LINE__,target);
    }

    Address unrelocTarget = target;

    if ( isRuntimeHeapAddr( target ) ) {
        // get unrelocated target address, there are three possibilities
        // a. We're in post-call padding, and targBBI is the call block
        // b. We're in an analyzed fallthrough block
        // c. The stack was tampered with and we need the (mod_pc - pc) 
        //    offset to figure out where we should be
        malware_cerr << "Looking for matches to incoming address " 
            << hex << target << dec << endl;
        std::pair<func_instance*,Address> tmp;

        if ( reverseDefensiveMap_.find(target,tmp) ) {
            // a. 
           std::set<block_instance*> callBs;
           tmp.first->getBlocks(tmp.second, callBs);
           block_instance *callB = (*callBs.begin());
           edge_instance *fallthrough = callB->getFallthrough();
           if (fallthrough) {
              unrelocTarget = fallthrough->trg()->start();
           } else {
              unrelocTarget = callB->end();
           }
        }
        else {
            // b. 
            // if we're in the fallthrough block, match to call block, 
            // and if necessary, add fallthrough edge
           AddressSpace::RelocInfo ri;
           bool hasFT = getRelocInfo(target, ri);
           assert(hasFT); // otherwise we should be in the defensive map
           if (ri.pad) {
               unrelocTarget = ri.block->end();
           } else {
               unrelocTarget = ri.block->start();
           }
        }
        mal_printf("translated target %lx to %lx %s[%d]\n",
            target, unrelocTarget, FILE__, __LINE__);
    }
    else { // target is not relocated, nothing to do but find the 
           // mapped_object, creating one if necessary, for transfers
           // into memory regions that are allocated at runtime
        mapped_object *obj = findObject(target);
        if (!obj) {

#if 0           
           Frame activeFrame = threads[0]->get_lwp()->getActiveFrame();
           for (unsigned i = 0; i < 0x100; ++i) {
		          Address stackTOP = activeFrame.esp;
		          Address stackTOPVAL =0;
                readDataSpace((void *) (stackTOP + 4*i), 
                              sizeof(getAddressWidth()), 
                              &stackTOPVAL, false);
		          malware_cerr << "\tSTACK[" << hex << stackTOP+4*i << "]=" 
                             << stackTOPVAL << dec << endl;
           }
#endif

            obj = createObjectNoFile(target);
            if (!obj) {
                fprintf(stderr,"ERROR, point %lx has target %lx that responds "
                        "to no object %s[%d]\n", pointAddr, target, 
                        FILE__,__LINE__);
                assert(0 && "stopThread snippet has an invalid target");
                return 0;
            }
        }
    }

#if 0
           Frame activeFrame = threads[0]->get_lwp()->getActiveFrame();
           Address stackTOP = activeFrame.esp;
           Address stackTOPVAL =0;
           for (unsigned i = 0; 
                i < 0x100 && 0 != ((stackTOP + 4*i) % memoryPageSize_); 
                ++i) 
           {
                readDataSpace((void *) (stackTOP + 4*i), 
                              sizeof(getAddressWidth()), 
                              &stackTOPVAL, false);
		          malware_cerr << "\tSTACK[" << hex << stackTOP+4*i << "]=" 
                             << stackTOPVAL << dec << endl;
           }
#endif

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

bool PCProcess::launchDebugger() {
    // Stop the process on detach 
    std::vector<func_instance *> breakpointFuncs;
    if( !findFuncsByAll("DYNINSTsafeBreakPoint", breakpointFuncs) ) {
        fprintf(stderr, "Failed to find function DYNINSTsafeBreakPoint\n");
        return false;
    }

    func_instance *safeBreakpoint = breakpointFuncs[0];
    for(map<dynthread_t, PCThread *>::iterator i = threadsByTid_.begin();
            i != threadsByTid_.end(); ++i)
    {
        if( !i->second->pcThr_->setRegister(MachRegister::getPC(getArch()),
                    safeBreakpoint->addr()) )
        {
            fprintf(stderr, "Failed to set PC to 0x%lx\n", 
                    safeBreakpoint->addr());
            return false;
        }
    }

    // Detach the process
    if( !detachProcess(true) ) {
        fprintf(stderr, "Failed to detach from process %d\n", getPid());
        return false;
    }

    if( !startDebugger() ) {
        fprintf(stderr, "Failed to start debugger on process %d\n", getPid());
        return false;
    }

    return true;
}

// End debugging

Address getVarAddr(PCProcess *proc, std::string str) {
    Address retAddr = 0;

    std::vector<int_variable *> vars;
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

Address PCProcess::getRTTrapFuncAddr() {
    if (rt_trap_func_addr_ == 0) {
        func_instance* func = findOnlyOneFunction("DYNINSTtrapFunction");
        rt_trap_func_addr_ = func->addr();
    }
    return rt_trap_func_addr_;
}

bool PCProcess::hasPendingEvents() {
   // Go to the muxer as a final arbiter
   return PCEventMuxer::muxer().hasPendingEvents(this);
}

bool PCProcess::hasRunningSyncRPC() const {
    return (syncRPCThreads_.size() > 0);
}

void PCProcess::addSyncRPCThread(Thread::ptr thr) {
   proccontrol_printf("%s[%d]: added sync rpc thread %d/%d\n",
                      FILE__, __LINE__, getPid(), thr ? thr->getLWP() : 0);
    syncRPCThreads_.insert(thr);
}

void PCProcess::removeSyncRPCThread(Thread::ptr thr) {
    proccontrol_printf("%s[%d]: removed sync rpc thread %d/%d\n",
		FILE__, __LINE__, getPid(), thr ? thr->getLWP() : 0);
    syncRPCThreads_.erase(thr);
}

bool PCProcess::continueSyncRPCThreads() {
	for(set<Thread::ptr>::iterator i = syncRPCThreads_.begin();
            i != syncRPCThreads_.end(); ++i)
    {
		if(!(*i)) {
			if(!pcProc_->continueProc())
			{
				proccontrol_printf("%s[%d]: failed to continue entire process %d for sync RPC\n",
						FILE__, __LINE__, getPid());
				return false;
			}
		} else if( !(*i)->continueThread() ) {
            proccontrol_printf("%s[%d]: failed to continue thread %d/%d for sync RPC\n",
                    FILE__, __LINE__, getPid(), (*i)->getLWP());
            return false;
        }
    }

    return true;
}

void PCProcess::addTrap(Address from, Address to, codeGen &gen) {
   gen.invalidate();
   gen.allocate(4);
   gen.setAddrSpace(this);
   gen.setAddr(from);
   if (sigILLTrampoline_) {
      insnCodeGen::generateIllegal(gen);
   } else {
      insnCodeGen::generateTrap(gen);
   }   
   trapMapping.addTrapMapping(from, to, true);
   springboard_cerr << "Generated springboard trap " << hex << from << "->" << to << dec << endl;
}

void PCProcess::removeTrap(Address from) {
    map<Address, Breakpoint::ptr>::iterator breakIter = 
        installedCtrlBrkpts.find(from);
    if( breakIter == installedCtrlBrkpts.end() ) return;

    if( !pcProc_->rmBreakpoint(from, breakIter->second) ) {
        proccontrol_printf("%s[%d]: failed to remove ctrl transfer breakpoint from 0x%lx\n",
                FILE__, __LINE__, from);
    }

    installedCtrlBrkpts.erase(breakIter);
}

void PCProcess::invalidateMTCache() {
    mt_cache_result_ = not_cached;
}

bool PCProcess::supportsUserThreadEvents() {
    if (!pcProc_) return false;
    return pcProc_->supportsUserThreadEvents();
}

StackwalkSymLookup::StackwalkSymLookup(PCProcess *p)
  : proc_(p)
{}

StackwalkSymLookup::~StackwalkSymLookup()
{}

bool StackwalkSymLookup::lookupAtAddr(Dyninst::Address addr, std::string &out_name, void* &out_value)
{
  func_instance *func = proc_->findOneFuncByAddr(addr);
  if( func == NULL ) return false;

  // set out_name to the name of the function at this addr
  // set out_value to NULL, this value is no longer used

  out_value = NULL;

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



