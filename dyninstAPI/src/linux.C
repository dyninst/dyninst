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
#include "symtab.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "os.h"
#include "debug.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "linux.h"
#include <dlfcn.h>

#include "pcEventMuxer.h"

#include "common/h/headers.h"
#include "common/h/linuxKludges.h"

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

bool PCProcess::usesDataLoadAddress() const {
    return false;
}

bool PCProcess::copyDanglingMemory(PCProcess *) {
    return true;
}

const unsigned int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 4;
const char DYNINST_LOAD_HIJACK_FUNCTIONS[][20] = {
  "__libc_start_main",
  "_init",
  "_start",
  "main"
};

/**
 * Returns an address that we can use to write the code that executes
 * dlopen on the runtime library.
 *
 * Inserting the code into libc is a good thing, since _dl_open
 * will sometimes check it's caller and return with a 'invalid caller'
 * error if it's called from the application.
 **/
Address PCProcess::findFunctionToHijack()
{
   Address codeBase = 0;
   unsigned i;
   for(i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
      const char *func_name = DYNINST_LOAD_HIJACK_FUNCTIONS[i];

      pdvector<func_instance *> hijacks;
      if (!findFuncsByAll(func_name, hijacks)) continue;
      codeBase = hijacks[0]->addr();

      if (codeBase)
          break;
   }
   if( codeBase != 0 ) {
     proccontrol_printf("%s[%d]: found hijack function %s = 0x%lx\n",
           FILE__, __LINE__, DYNINST_LOAD_HIJACK_FUNCTIONS[i], codeBase);
   }

  return codeBase;
} /* end findFunctionToHijack() */

static const int DLOPEN_MODE = RTLD_NOW | RTLD_GLOBAL;

// Note: this is an internal libc flag -- it is only used
// when libc and ld.so don't have symbols
#ifndef __RTLD_DLOPEN
#define __RTLD_DLOPEN 0x80000000
#endif

const char *DL_OPEN_FUNC_USER = NULL;
const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_LIBC_FUNC_EXPORTED[] = "__libc_dlopen_mode";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";

bool PCProcess::postRTLoadCleanup() {
    if( rtLibLoadHeap_ ) {
        if( !pcProc_->freeMemory(rtLibLoadHeap_) ) {
            startup_printf("%s[%d]: failed to free memory used for RT library load\n",
                    FILE__, __LINE__);
            return false;
        }
        rtLibLoadHeap_ = 0;
    }
    return true;
}

AstNodePtr PCProcess::createLoadRTAST() {
    pdvector<func_instance *> dlopen_funcs;

    int mode = DLOPEN_MODE;

    // allow user to override default dlopen func names with env. var

    DL_OPEN_FUNC_USER = getenv("DYNINST_DLOPEN_FUNC");

    if( DL_OPEN_FUNC_USER ) {
        findFuncsByAll(DL_OPEN_FUNC_USER, dlopen_funcs);
    }

    bool useHiddenFunction = false;
    bool needsStackUnprotect = false;
    if( dlopen_funcs.size() == 0 ) {
        do {
            if( findFuncsByAll(DL_OPEN_FUNC_EXPORTED, dlopen_funcs) ) break;

            // This approach will work if libc and the loader have symbols
            // Note: this is more robust than the next approach
            useHiddenFunction = true;
            needsStackUnprotect = true;
            if( findFuncsByAll(DL_OPEN_FUNC_NAME, dlopen_funcs) ) break;

            // If libc and the loader don't have symbols, we need to take a
            // different approach. We still need to the stack protection turned
            // off, but since we don't have symbols we use an undocumented flag
            // to turn off the stack protection
            useHiddenFunction = false;
            needsStackUnprotect = false;
            mode |= __RTLD_DLOPEN;
            if( findFuncsByAll(DL_OPEN_LIBC_FUNC_EXPORTED, dlopen_funcs) ) break;

            useHiddenFunction = true;
            needsStackUnprotect = true;
            pdvector<func_instance *> dlopen_int_funcs;
            // If we can't find the do_dlopen function (because this library is
            // stripped, for example), try searching for the internal _dl_open
            // function and find the do_dlopen function by examining the
            // functions that call it. This depends on the do_dlopen function
            // having been parsed (though its name is not known) through
            // speculative parsing.
            if(findFuncsByAll(DL_OPEN_FUNC_INTERNAL, dlopen_int_funcs)) {
                if(dlopen_int_funcs.size() > 1) {
                    startup_printf("%s[%d] warning: found %d matches for %s\n",
                                   __FILE__,__LINE__,dlopen_int_funcs.size(),
                                   DL_OPEN_FUNC_INTERNAL);
                }
                dlopen_int_funcs[0]->getCallerFuncs(std::back_inserter(dlopen_funcs));
                if(dlopen_funcs.size() > 1) {
                    startup_printf("%s[%d] warning: found %d do_dlopen candidates\n",
                                   __FILE__,__LINE__,dlopen_funcs.size());
                }

                if(dlopen_funcs.size() > 0) {
                    // give it a name
                    dlopen_funcs[0]->addSymTabName("do_dlopen",true);
                }
                break;
            }

            startup_printf("%s[%d]: failed to find dlopen function to load RT lib\n",
                           FILE__, __LINE__);
            return AstNodePtr();
        }while(0);
    }

    assert( dlopen_funcs.size() != 0 );

    if (dlopen_funcs.size() > 1) {
        logLine("WARNING: More than one dlopen found, using the first\n");
    }

    func_instance *dlopen_func = dlopen_funcs[0];

    pdvector<AstNodePtr> sequence;
    if( needsStackUnprotect ) {
        AstNodePtr unprotectStackAST = createUnprotectStackAST();
        if( unprotectStackAST == AstNodePtr() ) {
            startup_printf("%s[%d]: failed to generate unprotect stack AST\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        sequence.push_back(unprotectStackAST);
    }

    if( !useHiddenFunction ) {
        // For now, we cannot use inferiorMalloc because that requires the RT library
        // Hopefully, we can transition inferiorMalloc to use ProcControlAPI for 
        // allocating memory
        rtLibLoadHeap_ = pcProc_->mallocMemory(dyninstRT_name.length());
        if( !rtLibLoadHeap_ ) {
            startup_printf("%s[%d]: failed to allocate memory for RT library load\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        if( !writeDataSpace((char *)rtLibLoadHeap_, dyninstRT_name.length(), dyninstRT_name.c_str()) ) {
            startup_printf("%s[%d]: failed to write RT lib name into mutatee\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        pdvector<AstNodePtr> args;
        args.push_back(AstNode::operandNode(AstNode::Constant, (void *)rtLibLoadHeap_));
        args.push_back(AstNode::operandNode(AstNode::Constant, (void *)mode));

        sequence.push_back(AstNode::funcCallNode(dlopen_func, args));
    }else{
        startup_printf("%s[%d]: Creating AST to call libc's internal dlopen\n", FILE__, __LINE__);
        struct libc_dlopen_args_32 {
            uint32_t namePtr;
            uint32_t mode;
            uint32_t linkMapPtr;
        };

        struct libc_dlopen_args_64 {
            uint64_t namePtr;
            uint32_t mode;
            uint64_t linkMapPtr;
        };

        // Construct the argument to the internal function
        struct libc_dlopen_args_32 args32;
        struct libc_dlopen_args_64 args64;

        unsigned argsSize = 0;
        void *argsPtr;
        if( getAddressWidth() == 4 ) {
            argsSize = sizeof(args32);
            argsPtr = &args32;
        }else{
            argsSize = sizeof(args64);
            argsPtr = &args64;
        }

        // Allocate memory for the arguments
        rtLibLoadHeap_ = pcProc_->mallocMemory(dyninstRT_name.length()+1 + argsSize);
        if( !rtLibLoadHeap_ ) {
            startup_printf("%s[%d]: failed to allocate memory for RT library load\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        if( !writeDataSpace((char *)rtLibLoadHeap_, dyninstRT_name.length()+1, dyninstRT_name.c_str()) ) {
            startup_printf("%s[%d]: failed to write RT lib name into mutatee\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        if( getAddressWidth() == 4 ) {
            args32.namePtr = (uint32_t)rtLibLoadHeap_;
            args32.mode = mode;
            args32.linkMapPtr = 0;
        }else{
            args64.namePtr = (uint64_t)rtLibLoadHeap_;
            args64.mode = mode;
            args64.linkMapPtr = 0;
        }

        Address argsAddr = rtLibLoadHeap_ + dyninstRT_name.length()+1;
        if( !writeDataSpace((char *) argsAddr, argsSize, argsPtr) ) {
            startup_printf("%s[%d]: failed to write arguments to libc dlopen\n",
                    FILE__, __LINE__);
            return AstNodePtr();
        }

        pdvector<AstNodePtr> args;
        args.push_back(AstNode::operandNode(AstNode::Constant, (void *)argsAddr));
        sequence.push_back(AstNode::funcCallNode(dlopen_func, args));
    }

    return AstNode::sequenceNode(sequence);
}

bool PCEventMuxer::useBreakpoint(Dyninst::ProcControlAPI::EventType et)
{
    // This switch statement can be derived from the EventTypes and Events
    // table in the ProcControlAPI manual -- it states what Events are
    // available on each platform

   // Pre-events are breakpoint
   // Post-events are callback
   if (et.time() == EventType::Pre &&
       ((et.code() == EventType::Exit) ||
        (et.code() == EventType::Fork) ||
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
   if (et.time() == EventType::Post &&
       ((et.code() == EventType::Exit) ||
        (et.code() == EventType::Fork) ||
        (et.code() == EventType::Exec))) return true;
   return false;
}

bool BinaryEdit::getResolvedLibraryPath(const string &filename, std::vector<string> &paths) {
    char *libPathStr, *libPath;
    std::vector<string> libPaths;
    struct stat dummy;
    FILE *ldconfig;
    char buffer[512];
    char *pos, *key, *val;

    // prefer qualified file paths
    if (stat(filename.c_str(), &dummy) == 0) {
        paths.push_back(filename);
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
    ldconfig = popen("/sbin/ldconfig -p", "r");
    if (ldconfig) {
        fgets(buffer, 512, ldconfig);	// ignore first line
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
    libPaths.push_back("/lib");
    libPaths.push_back("/lib64");
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
