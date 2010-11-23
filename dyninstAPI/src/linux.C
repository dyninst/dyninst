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

// $Id: linux.C,v 1.279 2008/09/03 06:08:44 jaw Exp $

#include "binaryEdit.h"
#include "pcProcess.h"
#include "symtab.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "os.h"
#include "debug.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "linux.h"

#include "common/h/headers.h"
#include "common/h/linuxKludges.h"

#include "symtabAPI/h/Symtab.h"
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::ProcControlAPI;

using std::string;

void printStackWalk( PCProcess *p ) {
  Frame theFrame = p->getInitialThread()->getActiveFrame();
  while (true) {
    // do we have a match?
    const Address framePC = theFrame.getPC();
    proccontrol_cerr << "stack frame pc @ " << (void*)framePC << endl;
    
    if (theFrame.isLastFrame())
      // well, we've gone as far as we can, with no match.
      break;
    
    // else, backtrace 1 more level
    theFrame = theFrame.getCallerFrame();
  }
}
 
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

/**
 * Return the state of the process from /proc/pid/stat.
 * File format is:
 *   pid (executablename) state ...
 * where state is a character.  Returns '\0' on error.
 **/
static char getState(int pid) {
    char procName[64];
    char sstat[256];
    char *status;
    int paren_level = 1;

    sprintf(procName,"/proc/%d/stat", pid);
    FILE *sfile = P_fopen(procName, "r");
    if (sfile == NULL) return '\0';
    fread( sstat, 1, 256, sfile );
    fclose( sfile );
    sstat[255] = '\0';
    status = sstat;

    while (*status != '\0' && *(status++) != '(') ;
    while (*status != '\0' && paren_level != 0) {
        if (*status == '(') paren_level++;
        if (*status == ')') paren_level--;
        status++;
    }

    while (*status == ' ') status++;
    return *status;
}

bool PCProcess::getOSRunningState(int pid) {
    char result = getState(pid);
    if (result == '\0') {
        return false;
    }
    return (result != 'T');
}

static const Address lowest_addr = 0x0;
void PCProcess::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
#if !defined(arch_x86_64) && !defined(arch_power)
      lo = region_lo(near);
      hi = region_hi(near);  
#else
      if (getAddressWidth() == 8) {
	  lo = region_lo_64(near);
	  hi = region_hi_64(near);
      }
      else {
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

bool PCProcess::getExecFileDescriptor(string filename,
        bool, fileDescriptor &desc)
{
    desc = fileDescriptor(filename.c_str(),
            0, // code
            0, // data
            false); // a.out
    return true;
}

static
string deref_link(const char *path)
{
    char *p = realpath(path, NULL);
    if (p == NULL) {
        return string();
    }
    string sp = p;
    free(p);
    return sp;
}

/**
 * Strategy:  The program entry point is in /lib/ld-2.x.x at the 
 * _start function.  Get the current PC, parse /lib/ld-2.x.x, and 
 * compare the two points.
 **/
bool PCProcess::hasPassedMain() 
{
   // We only need to parse /lib/ld-2.x.x once for any process,
   // so just do it once for any process.  We'll cache the result
   // in lib_to_addr.
   static dictionary_hash<string, Address> lib_to_addr(::Dyninst::stringhash);
   Symtab *ld_file = NULL;
   Address entry_addr, ldso_start_addr;

   //Get current PC
   Frame active_frame = initialThread_->getActiveFrame();
   Address current_pc = active_frame.getPC();

   // Get the interpreter name from SymtabAPI
   const char *path = getAOut()->parse_img()->getObject()->getInterpreterName();

   if (!path) {
      //Strange... This shouldn't happen on a normal linux system
      startup_printf("[%s:%u] - Couldn't find /lib/ld-x.x.x in hasPassedMain\n",
                     FILE__, __LINE__);
      return true;
   }

   std::string derefPath = deref_link(path);

   // Search for the dynamic linker in the loaded libraries
   const LibraryPool &libraries = pcProc_->libraries();
   bool foundDynLinker = false;
   for(LibraryPool::const_iterator i = libraries.begin(); i != libraries.end();
           ++i)
   {
       if( (*i)->getName() == derefPath ) {
           foundDynLinker = true;
           ldso_start_addr = (*i)->getLoadAddress();
       }
   }

   if( !foundDynLinker ) {
       // This means that libraries haven't been loaded yet which implies
       // that main hasn't been reached yet
       return false;
   }

   if (lib_to_addr.defines(derefPath)) {
      //We've already parsed this library.  Use those results.
      Address start_in_ld = lib_to_addr[path];
      return (start_in_ld != current_pc);
   }

   //Open /lib/ld-x.x.x and find the entry point
   if (!Symtab::openFile(ld_file, derefPath)) {
      startup_printf("[%s:%u] - Unable to open %s in hasPassedMain\n", 
                     FILE__, __LINE__, path);
      return true;
   }

   entry_addr = ld_file->getEntryOffset();
   if (!entry_addr) {
      startup_printf("[%s:%u] - No entry addr for %s\n", 
                     FILE__, __LINE__, path);
      return true;
   }

   entry_addr += ldso_start_addr;
   
   lib_to_addr[path] = entry_addr;
   bool result = (entry_addr != current_pc);
   startup_printf("[%s:%u] - hasPassedMain returning %d (%lx %lx)\n",
                  FILE__, __LINE__, (int) result, entry_addr, current_pc);

   return result;
}

Address PCProcess::setAOutLoadAddress(fileDescriptor &desc) {
   //The load address of the a.out isn't correct.  We can't read a
   // correct one out of ld-x.x.x.so because it may not be initialized yet,
   // and it won't be initialized until we reach main.  But we need the load
   // address to find main.  Darn.
   //
   //Instead we'll read the entry out of /proc/pid/maps, and try to make a good
   // effort to correctly match the fileDescriptor to an entry.  Unfortunately,
   // symlinks can complicate this, so we'll stat the files and compare inodes

   struct stat aout, maps_entry;
   map_entries *maps = NULL;
   unsigned maps_size = 0, i;
   char proc_path[128];
   int result;

   //Get the inode for the a.out
   startup_printf("[%s:%u] - a.out is a shared library, computing load addr\n",
                  FILE__, __LINE__);
   memset(&aout, 0, sizeof(aout));
   proc_path[127] = '\0';
   snprintf(proc_path, 127, "/proc/%d/exe", getPid());
   result = stat(proc_path, &aout);
   if (result == -1) {
      startup_printf("[%s:%u] - setAOutLoadAddress couldn't stat %s: %s\n",
                     FILE__, __LINE__, proc_path, strerror(errno));
      goto done;
   }
                    
   //Get the maps
   maps = getVMMaps(getPid(), maps_size);
   if (!maps) {
      startup_printf("[%s:%u] - setAOutLoadAddress, getLinuxMaps return NULL\n",
                     FILE__, __LINE__);
      goto done;
   }
   
   //Compare the inode of each map entry to the a.out's
   for (i=0; i<maps_size; i++) {
      memset(&maps_entry, 0, sizeof(maps_entry));
      result = stat(maps[i].path, &maps_entry);
      if (result == -1) {
         startup_printf("[%s:%u] - setAOutLoadAddress couldn't stat %s: %s\n",
                        FILE__, __LINE__, maps[i].path, strerror(errno));
         continue;
      }
      if (maps_entry.st_dev == aout.st_dev && maps_entry.st_ino == aout.st_ino)
      {
         //We have a match
         desc.setLoadAddr(maps[i].start);
         goto done;
      }
   }
        
 done:
   if (maps)
      free(maps);

   return desc.loadAddr();
}

bool PCProcess::usesDataLoadAddress() const {
    return false;
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

      pdvector<int_function *> hijacks;
      if (!findFuncsByAll(func_name, hijacks))
          return 0;
      codeBase = hijacks[0]->getAddress();

      if (codeBase)
          break;
   }
   if( codeBase != 0 ) {
     proccontrol_printf("%s[%d]: found hijack function %s = 0x%lx\n",
           FILE__, __LINE__, DYNINST_LOAD_HIJACK_FUNCTIONS[i], codeBase);
   }

  return codeBase;
} /* end findFunctionToHijack() */

/**
 * Searches for function in order, with preference given first
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(PCProcess *p, std::string func,
                            pdvector<int_function *> &result) {
    bool found = false;
    mapped_module *lpthread = p->findModule("libpthread*", true);
    if (lpthread)
        found = lpthread->findFuncVectorByPretty(func, result);
    if (found)
        return;

    mapped_module *lc = p->findModule("libc.so*", true);
    if (lc)
        found = lc->findFuncVectorByPretty(func, result);
    if (found)
        return;

    p->findFuncsByPretty(func, result);
}

bool PCProcess::instrumentMTFuncs() {
    unsigned i;
    bool res;

#if !defined(cap_threads)
    return true;
#endif

    /**
     * Instrument thread_create with calls to DYNINST_dummy_create
     **/
    //Find create_thread
    pdvector<int_function *> thread_init_funcs;
    findThreadFuncs(this, "create_thread", thread_init_funcs);
    findThreadFuncs(this, "start_thread", thread_init_funcs);
    if (thread_init_funcs.size() < 1) {
        fprintf(stderr, "[%s:%d] - Found no copies of create_thread, expected 1\n",
                __FILE__, __LINE__);
        return false;
    }
    //Find DYNINST_dummy_create
    int_function *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
    if (!dummy_create) {
        fprintf(stderr, "[%s:%d] - Couldn't find DYNINST_dummy_create",
                __FILE__, __LINE__);
        return false;
    }
    //Instrument
    for (i=0; i<thread_init_funcs.size(); i++) {
        pdvector<AstNodePtr> args;
        AstNodePtr ast = AstNode::funcCallNode(dummy_create, args);
        const pdvector<instPoint *> &ips = thread_init_funcs[i]->funcEntries();
        for (unsigned j=0; j<ips.size(); j++) {
            miniTramp *mt;
            mt = ips[j]->instrument(ast, callPreInsn, orderFirstAtPoint, false,
                                    false);
            if (!mt) {
                fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                        __FILE__, __LINE__);
            }
            //TODO: Save the mt objects for detach
        }
    }

    //Find functions that are run on pthread exit
    pdvector<int_function *> thread_dest_funcs;
    findThreadFuncs(this, "__pthread_do_exit", thread_dest_funcs);
    findThreadFuncs(this, "pthread_exit", thread_dest_funcs);
    findThreadFuncs(this, "deallocate_tsd", thread_dest_funcs);
    if (!thread_dest_funcs.size()) {
        fprintf(stderr,"[%s:%d] - Found 0 copies of pthread_exit, expected 1\n",
                __FILE__, __LINE__);
        return false;
    }
    //Find DYNINSTthreadDestroy
    int_function *threadDestroy = findOnlyOneFunction("DYNINSTthreadDestroy");
    if (!threadDestroy) {
        fprintf(stderr, "[%s:%d] - Couldn't find DYNINSTthreadDestroy",
                __FILE__, __LINE__);
        return false;
    }

    for (i=0; i<thread_dest_funcs.size(); i++) {
        pdvector<AstNodePtr> args;
        AstNodePtr ast = AstNode::funcCallNode(threadDestroy, args);
        const pdvector<instPoint *> &ips = thread_dest_funcs[i]->funcExits();
        for (unsigned j=0; j<ips.size(); j++) {
            miniTramp *mt;
            mt = ips[j]->instrument(ast, callPreInsn, orderFirstAtPoint, false,
                                    false);
            if (!mt) {
                fprintf(stderr, "[%s:%d] - Couldn't instrument thread_exit\n",
                        __FILE__, __LINE__);
            }
            //TODO: Save the mt objects for detach
        }
    }

    /**
     * Have dyn_pthread_self call the actual pthread_self
     **/
    //Find dyn_pthread_self
    pdvector<int_variable *> ptself_syms;
    res = findVarsByAll("DYNINST_pthread_self", ptself_syms);
    if (!res) {
        fprintf(stderr, "[%s:%d] - Couldn't find any dyn_pthread_self, expected 1\n",
                __FILE__, __LINE__);
    }
    assert(ptself_syms.size() == 1);
    Address dyn_pthread_self = ptself_syms[0]->getAddress();
    //Find pthread_self
    pdvector<int_function *> pthread_self_funcs;
    findThreadFuncs(this, "pthread_self", pthread_self_funcs);
    if (pthread_self_funcs.size() != 1) {
        fprintf(stderr, "[%s:%d] - Found %ld pthread_self functions, expected 1\n",
                __FILE__, __LINE__, (long) pthread_self_funcs.size());
        for (unsigned j=0; j<pthread_self_funcs.size(); j++) {
            int_function *ps = pthread_self_funcs[j];
            fprintf(stderr, "[%s:%u] - %s in module %s at %lx\n", __FILE__, __LINE__,
                    ps->prettyName().c_str(), ps->mod()->fullName().c_str(),
                    ps->getAddress());
        }
        return false;
    }
    //Replace
    res = writeFunctionPtr(this, dyn_pthread_self, pthread_self_funcs[0]);
    if (!res) {
        fprintf(stderr, "[%s:%d] - Couldn't update dyn_pthread_self\n",
                __FILE__, __LINE__);
        return false;
    }

    return true;
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
    libPathStr = strdup(getenv("LD_LIBRARY_PATH"));
    libPath = strtok(libPathStr, ":");
    while (libPath != NULL) {
        libPaths.push_back(string(libPath));
        libPath = strtok(NULL, ":");
    }
    free(libPathStr);

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
            const pdvector<int_function *> *tmpFuncs = 
                mobj->findFuncVectorByPretty(pthreadSyms[i]);
            if( tmpFuncs != NULL && tmpFuncs->size() ) numSymsFound++;
        }

        if( numSymsFound == NUM_PTHREAD_SYMS ) return true;
    }

    return false;
}

bool PCProcess::readAuxvInfo()
{
   if (auxv_parser_) return false;

   auxv_parser_ = AuxvParser::createAuxvParser(getPid(), getAddressWidth());
   if (!auxv_parser_) {
      startup_printf("%s[%u]: - ERROR, failed to parse Auxv\n", FILE__, __LINE__);
      vsys_status_ = vsys_notfound;
      return false;
   }

   vsyscall_start_ = auxv_parser_->getVsyscallBase();
   vsyscall_end_ = auxv_parser_->getVsyscallEnd();
   vsyscall_text_ = auxv_parser_->getVsyscallText();
   vsys_status_ = vsys_found;

   return true;
}
