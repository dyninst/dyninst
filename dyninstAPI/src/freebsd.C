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

#include <cassert>

#include "freebsd.h"
#include "dyn_lwp.h"
#include "process.h"
#include "syscallNotification.h"
#include "dynamiclinking.h"
#include "signalgenerator.h"
#include "binaryEdit.h"
#include "mapped_object.h"

void loadNativeDemangler() {}

bool BinaryEdit::getResolvedLibraryPath(const std::string &filename, std::vector<std::string> &paths) {
    char *libPathStr, *libPath;
    std::vector<std::string> libPaths;
    struct stat dummy;
    FILE *ldconfig;
    char buffer[512];

    // prefer qualified file paths
    if (stat(filename.c_str(), &dummy) == 0) {
        paths.push_back(filename);
    }

    // search paths from environment variables
    libPathStr = strdup(getenv("LD_LIBRARY_PATH"));
    libPath = strtok(libPathStr, ":");
    while (libPath != NULL) {
        libPaths.push_back(std::string(libPath));
        libPath = strtok(NULL, ":");
    }
    free(libPathStr);

    for (unsigned int i = 0; i < libPaths.size(); i++) {
        std::string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    // search ld.so hints file
    ldconfig = popen("/sbin/ldconfig -r", "r");
    if( ldconfig ) {
        // ignore first and second line
        fgets(buffer, 512, ldconfig);
        fgets(buffer, 512, ldconfig);

        // Here is the expected format:
        // [^/]* => (path)/(filename)
        while(fgets(buffer, 512, ldconfig) != NULL) {
            size_t fileBegin, pathBegin;

            // Remove any whitespace at the end
            std::string strBuf(buffer);
            strBuf = strBuf.substr(0, strBuf.find_last_not_of(" \t\n\r")+1);

            // Locate the filename
            fileBegin = strBuf.rfind("/");
            if( fileBegin == std::string::npos ||
                fileBegin+1 >= strBuf.length() ) continue;

            if( strBuf.substr(fileBegin+1) == filename ) {
                // Locate the path
                pathBegin = strBuf.find("/");
                if( pathBegin == std::string::npos ) continue;
                paths.push_back(strBuf.substr(pathBegin));
            }
        }

        pclose(ldconfig);
    }

    // search hard-coded system paths
    libPaths.clear();
    libPaths.push_back("/lib");
    libPaths.push_back("/usr/lib");
    libPaths.push_back("/usr/local/lib");
    for (unsigned int i = 0; i < libPaths.size(); i++) {
        std::string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    return ( 0 < paths.size() );
}

sharedLibHook::~sharedLibHook() {}

bool BinaryEdit::archSpecificMultithreadCapable() {
    /*
     * The heuristic on FreeBSD is to check for some symbols that are
     * only included in a binary when pthreads has been linked into the
     * binary. If the binary contains these symbols, it is multithread
     * capable.
     */
    const int NUM_PTHREAD_SYMS = 6;
    const char *pthreadSyms[NUM_PTHREAD_SYMS] =
    { "pthread_attr_get_np", "pthread_attr_getaffinity_np",
      "pthread_attr_getstack", "pthread_attr_setaffinity_np",
      "pthread_attr_setcreatesuspend_np", "pthread_attr_setstack"
    };

    if( mobj->isStaticExec() ) {
        int numSymsFound = 0;
        for(int i = 0; i < NUM_PTHREAD_SYMS; ++i) {
            const pdvector<func_instance *> *tmpFuncs = 
                mobj->findFuncVectorByPretty(pthreadSyms[i]);
            if( tmpFuncs != NULL && tmpFuncs->size() ) numSymsFound++;
        }

        if( numSymsFound == NUM_PTHREAD_SYMS ) return true;
    }

    return false;
}

bool AddressSpace::getDyninstRTLibName() {
   startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           std::string msg;
           process *proc;
           if ((proc = dynamic_cast<process *>(this)) != NULL) {
              msg = std::string("Environment variable ") +
                 std::string("DYNINSTAPI_RT_LIB") +
                 std::string(" has not been defined for process ") +
                 utos(proc->getPid());
           }
           else {
              msg = std::string("Environment variable ") +
                 std::string("DYNINSTAPI_RT_LIB") +
                 std::string(" has not been defined");
           }           
           showErrorCallback(101, msg);
           return false;
        }
    }

    // Automatically choose 32-bit library if necessary.
    const char *modifier = "_m32";
    const char *name = dyninstRT_name.c_str();

    const char *split = P_strrchr(name, '/');
    if ( !split ) split = name;
    split = P_strchr(split, '.');
    if ( !split || P_strlen(split) <= 1 ) {
        // We should probably print some error here.
        // Then, of course, the user will find out soon enough.
        startup_printf("Invalid Dyninst RT lib name: %s\n", 
                dyninstRT_name.c_str());
        return false;
    }

    if ( getAddressWidth() == sizeof(void *) || P_strstr(name, modifier) ) {
        modifier = "";
    }

    const char *suffix = split;
    if( getAOut()->isStaticExec() ) {
        suffix = ".a";
    }else{
        if( P_strncmp(suffix, ".a", 2) == 0 ) {
            // This will be incorrect if the RT library's version changes
            suffix = ".so";
        }
    }

    dyninstRT_name = std::string(name, split - name) +
                     std::string(modifier) +
                     std::string(suffix);

    startup_printf("Dyninst RT Library name set to '%s'\n",
            dyninstRT_name.c_str());

    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name
        + std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }

    return true;
}

/* START unimplemented functions */

#define FREEBSD_NOT_IMPLEMENTED "This function is not implemented on FreeBSD"

void initPrimitiveCost() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

void dyninst_yield() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool dyn_lwp::stop_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPostFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::dumpImage(std::string) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::realLWP_attach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::trapAtEntryPointOfMain(dyn_lwp*, unsigned long) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::installTracing() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePreLwpExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

Frame dyn_lwp::getActiveFrame() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    Frame nullFrame;
    return nullFrame;
}

bool dyn_lwp::waitUntilStopped() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::hasPassedMain() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::writeTextWord(char*, int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::handleTrapAtLibcStartMain(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::getRegisters_(dyn_saved_regs*, bool) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool SignalGenerator::decodeEvents(std::vector<EventRecord, std::allocator<EventRecord> >&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::initialize() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::continueLWP_(int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

syscallNotification::syscallNotification(syscallNotification*, process*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord&, unsigned int&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPostExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::loadDYNINSTlibCleanup(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::changePC(unsigned long, dyn_saved_regs*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::insertTrapAtEntryPointOfMain() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::restoreRegisters_(dyn_saved_regs const&, bool) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::readDataSpace(void const*, unsigned int, void*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

Address dyn_lwp::readRegister(unsigned int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::determineLWPs(std::vector<unsigned int, std::allocator<unsigned int> >&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::writeTextSpace(void*, unsigned int, void const*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

void dyn_lwp::realLWP_detach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool dyn_lwp::writeDataSpace(void*, unsigned int, void const*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

Frame dyn_thread::getActiveFrameMT() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    Frame nullFrame;
    return nullFrame;
}

bool syscallNotification::removePostExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

std::string process::tryToFindExecutable(std::string const&, int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return std::string("");
}


bool process::unsetProcessFlags() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::decodeStartupSysCalls(EventRecord&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool SignalGeneratorCommon::getExecFileDescriptor(std::string, int, bool, int&, fileDescriptor&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord&, std::vector<mapped_object*, std::allocator<mapped_object*> >&, 
        std::vector<bool, std::allocator<bool> >&) 
{
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

void OS::osTraceMe() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool Frame::setPC(unsigned long) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::instrumentLibcStartMain() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

void dyn_lwp::representativeLWP_detach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool process::handleTrapAtEntryPointOfMain(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::trapDueToDyninstLib(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePreExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::processLinkMaps(std::vector<fileDescriptor, std::allocator<fileDescriptor> >&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePostFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::dumpCore_(std::string) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePreFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

dyn_lwp* process::createRepresentativeLWP() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return NULL;
}

bool syscallNotification::removePreExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::loadDYNINSTlib() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool SignalHandler::handleProcessExitPlat(EventRecord&, bool&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreLwpExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::representativeLWP_attach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::isRunning_() const {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::hasBeenBound(Dyninst::SymtabAPI::relocationEntry const&, func_instance*&, unsigned long) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::readTextSpace(const void*, unsigned int, void*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::setProcessFlags() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

terminateProcStatus_t process::terminateProc_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return terminateFailed;
}

// Temporary remote debugger interface.
// I assume these will be removed when procControlAPI is complete.
bool OS_isConnected(void)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_connect(BPatch_remoteHost &/*remote*/)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_getPidList(BPatch_remoteHost &/*remote*/,
                   BPatch_Vector<unsigned int> &/*tlist*/)
{
    return false;  // Not implemented.
}

bool OS_getPidInfo(BPatch_remoteHost &/*remote*/,
                   unsigned int /*pid*/, std::string &/*pidStr*/)
{
    return false;  // Not implemented.
}

bool OS_disconnect(BPatch_remoteHost &/*remote*/)
{
    return true;
}

/* END unimplemented functions */
