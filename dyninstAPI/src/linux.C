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

// $Id: linux.C,v 1.279 2008/09/03 06:08:44 jaw Exp $

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
#include "linux.h"
#include <dlfcn.h>

#include "boost/shared_ptr.hpp"

#include "pcEventMuxer.h"

#include "common/src/headers.h"
#include "common/src/linuxKludges.h"

#include "symtabAPI/h/Symtab.h"
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::ProcControlAPI;

using std::string;

bool get_linux_version(int &major, int &minor, int &subvers)
{
    int subsub;
    return get_linux_version(major,minor,subvers,subsub); 
}

bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers)
{
   static int maj = 0, min = 0, sub = 0, subsub = 0;
   int result;
   FILE *f;
   if (maj)
   {
      major = maj;
      minor = min;
      subvers = sub;
      subsubvers = subsub;
      return true;
   }
   f = P_fopen("/proc/version", "r");
   if (!f) goto error;
   result = fscanf(f, "Linux version %d.%d.%d.%d", &major, &minor, &subvers,
                    &subsubvers);
   fclose(f);
   if (result != 3 && result != 4) goto error;

   maj = major;
   min = minor;
   sub = subvers;
   subsub = subsubvers;
   return true;

 error:
   //Assume 2.4, which is the earliest version we support
   major = maj = 2;
   minor = min = 4;
   subvers = sub = 0;
   subsubvers = subsub = 0;
   return false;
}

void PCProcess::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
        inferiorHeapType /* type */ ) 
{
    if (near) {
#if !defined(arch_x86_64) && !defined(arch_power)
        lo = region_lo(near);
        hi = region_hi(near);
#else
        if (getAddressWidth() == 8) {
            lo = region_lo_64(near);
            hi = region_hi_64(near);
        } else {
            lo = region_lo(near);
            hi = region_hi(near);
        }
#endif
    }
}

inferiorHeapType PCProcess::getDynamicHeapType() const {
    return anyHeap;
}

void loadNativeDemangler() {}

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


bool PCEventMuxer::useBreakpoint(Dyninst::ProcControlAPI::EventType et)
{
    // This switch statement can be derived from the EventTypes and Events
    // table in the ProcControlAPI manual -- it states what Events are
    // available on each platform

   // Pre-events are breakpoint
   // Post-events are callback
   if (et.time() == EventType::Pre &&
       ((et.code() == EventType::Fork) ||
	(et.code() == EventType::Exec))) return true;
   return false;
}

bool PCEventMuxer::useCallback(Dyninst::ProcControlAPI::EventType et)
{
    // This switch statement can be derived from the EventTypes and Events
    // table in the ProcControlAPI manual -- it states what Events are
    // available on each platform

   // Pre-events are breakpoint
   // Post-events are callback
  if(et.code() == EventType::Exit) return true;
  
  if (et.time() == EventType::Post &&
       ((et.code() == EventType::Fork) ||
        (et.code() == EventType::Exec))) return true;
  
   return false;
}

bool BinaryEdit::getResolvedLibraryPath(const string &filename, std::vector<string> &paths) {
    char *libPathStr, *libPath;
    std::vector<string> libPaths;
    struct stat dummy;
    char buffer[512];
    char *pos, *key, *val;

    // prefer qualified file paths
    if (stat(filename.c_str(), &dummy) == 0) {
        paths.push_back(filename);
    }

    // For cross-rewriting
    char *dyn_path = getenv("DYNINST_REWRITER_PATHS");
    if (dyn_path) { 
       libPathStr = strdup(dyn_path);
       libPath = strtok(libPathStr, ":");
       while (libPath != NULL) {
          libPaths.push_back(string(libPath));
          libPath = strtok(NULL, ":");
       }
       free(libPathStr);
    }

    // search paths from environment variables
    char *ld_path = getenv("LD_LIBRARY_PATH");
    if (ld_path) { 
       libPathStr = strdup(ld_path);
       libPath = strtok(libPathStr, ":");
       while (libPath != NULL) {
          libPaths.push_back(string(libPath));
          libPath = strtok(NULL, ":");
       }
       free(libPathStr);
    }

    //libPaths.push_back(string(getenv("PWD")));
    for (unsigned int i = 0; i < libPaths.size(); i++) {
        string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    // search ld.so.cache
    // apparently ubuntu doesn't like pclosing NULL, so a shared pointer custom
    // destructor is out. Ugh.
    FILE* ldconfig = popen("/sbin/ldconfig -p", "r");
    if (ldconfig) {
        if(!fgets(buffer, 512, ldconfig)) {	// ignore first line
          return false;
        }
        while (fgets(buffer, 512, ldconfig) != NULL) {
            pos = buffer;
            while (*pos == ' ' || *pos == '\t') pos++;
            key = pos;
            while (*pos != ' ') pos++;
            *pos = '\0';
            while (*pos != '=' && *(pos + 1) != '>') pos++;
            pos += 2;
            while (*pos == ' ' || *pos == '\t') pos++;
            val = pos;
            while (*pos != '\n' && *pos != '\0') pos++;
            *pos = '\0';
            if (strcmp(key, filename.c_str()) == 0) {
                paths.push_back(val);
            }
        }
        pclose(ldconfig);
    }

    // search hard-coded system paths
    libPaths.clear();
    libPaths.push_back("/usr/local/lib");
    libPaths.push_back("/usr/share/lib");
    libPaths.push_back("/usr/lib");
    libPaths.push_back("/usr/lib64");
    libPaths.push_back("/usr/lib/x86_64-linux-gnu");
    libPaths.push_back("/lib");
    libPaths.push_back("/lib64");
    libPaths.push_back("/lib/x86_64-linux-gnu");
    libPaths.push_back("/usr/lib/i386-linux-gnu");
    libPaths.push_back("/usr/lib32");
    for (unsigned int i = 0; i < libPaths.size(); i++) {
        string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    return ( 0 < paths.size() );
}

bool BinaryEdit::archSpecificMultithreadCapable() {
    /*
     * The heuristic for this check on Linux is that some symbols provided by
     * pthreads are only defined when the binary contains pthreads. Therefore,
     * check for these symbols, and if they are defined in the binary, then
     * conclude that the binary is multithread capable.
     */
    const int NUM_PTHREAD_SYMS = 4;
    const char *pthreadSyms[NUM_PTHREAD_SYMS] = 
        { "pthread_cancel", "pthread_once", 
          "pthread_mutex_unlock", "pthread_mutex_lock" 
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
