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

#include <cassert>
#include <dlfcn.h>
#include <cstdlib>

#include "binaryEdit.h"
#include "dynProcess.h"
#include "image.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "os.h"
#include "debug.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "freebsd.h"
#include "pcEventMuxer.h"
#include "common/src/headers.h"
#include "common/src/freebsdKludges.h"
#include "common/src/pathName.h"

#include "symtabAPI/h/Symtab.h"
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::ProcControlAPI;

using std::string;

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
            const std::vector<func_instance *> *tmpFuncs = 
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
           PCProcess *proc;
           if ((proc = dynamic_cast<PCProcess *>(this)) != NULL) {
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
    const char *modifier = "";
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

    if (getAddressWidth() == 4 &&
        (sizeof(void *) == 8)) {
       // Need _m32...
       if (P_strstr(name, "_m32") == NULL) {
          modifier = "_m32";
       }
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

    dyninstRT_name = std::move(resolve_file_path(dyninstRT_name));

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

/* dynamic instrumentation support */

bool PCEventMuxer::useBreakpoint(Dyninst::ProcControlAPI::EventType et) {
  // This switch statement can be derived from the EventTypes and Events
  // table in the ProcControlAPI manual -- it states what Events are
  // available on each platform

  // Pre-events are breakpoint
  // Post-events are callback
  if (et.time() == EventType::Pre &&
      ((et.code() == EventType::Exit) ||
       (et.code() == EventType::Fork) ||
       (et.code() == EventType::Exec))) return true;

  // Not sure about this one...
  if (et.time() == EventType::Post && et.code() == EventType::Fork) return true;

  return false;
}

bool PCEventMuxer::useCallback(Dyninst::ProcControlAPI::EventType et)
{
  // This switch statement can be derived from the EventTypes and Events
  // table in the ProcControlAPI manual -- it states what Events are
  // available on each platform

  // Pre-events are breakpoint
  // Post-events are callback
  if (et.time() == EventType::Post &&
      ((et.code() == EventType::Exit) ||
       (et.code() == EventType::Fork) ||
       (et.code() == EventType::Exec))) return true;
  return false;
}

void PCProcess::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
        inferiorHeapType /* type */ )
{
    if(near) {
#if !defined(arch_x86_64) && !defined(arch_power)
        lo = region_lo(near);
        hi = region_hi(near);
#else
        if( getAddressWidth() == 8 ) {
            lo = region_lo_64(near);
            hi = region_hi_64(near);
        }else{
            lo = region_lo(near);
            hi = region_hi(near);
        }
#endif
    }
}

inferiorHeapType PCProcess::getDynamicHeapType() const {
    return anyHeap;
}

bool PCProcess::dumpImage(string) {
    return false;
}

bool PCProcess::dumpCore(string) {
    return false;
}

bool PCProcess::skipHeap(const heapDescriptor &) {
    return false;
}

bool AddressSpace::usesDataLoadAddress() const {
    return false;
}

bool PCProcess::copyDanglingMemory(PCProcess *) {
    return true;
}


Address PCProcess::getTOCoffsetInfo(Address) {
    assert(!"This function is unimplemented");
    return 0;
}

Address PCProcess::getTOCoffsetInfo(func_instance *) {
    assert(!"This function is unimplemented");
    return 0;
}

bool PCProcess::getOPDFunctionAddr(Address &) {
    return true;
}

/* START unimplemented functions */

#define FREEBSD_NOT_IMPLEMENTED "This function is not implemented on FreeBSD"

void initPrimitiveCost() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
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

bool PCProcess::bindPLTEntry(Dyninst::SymtabAPI::relocationEntry const&, Address, func_instance*, Address) { return false; }
/* END unimplemented functions */
