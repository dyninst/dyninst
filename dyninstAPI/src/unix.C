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

// $Id: unix.C,v 1.243 2008/06/30 17:33:31 legendre Exp $

#include "os.h"
#include "debug.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "dynProcess.h"
#include "dynThread.h"
#include "function.h"
#include "binaryEdit.h"
#include "common/src/pathName.h"

#include <sstream>

extern char **environ;

using namespace Dyninst::ProcControlAPI;

// Functions for all Unices //

bool PCProcess::hideDebugger()
{
    return false;
}

bool OS::executableExists(const std::string &file) 
{
   struct stat file_stat;
   int stat_result;

   const char *fn = file.c_str();
   stat_result = stat(fn, &file_stat);
   return (stat_result != -1);
}

void OS::get_sigaction_names(std::vector<std::string> &names)
{
   names.push_back(string("sigaction"));
   names.push_back(string("signal"));
}



std::string PCProcess::createExecPath(const std::string &file, const std::string &dir) {
    std::string ret = file;
    if (dir.length() > 0) {
        if (!(file[0] == ('/'))) {
            // file does not start  with a '/', so it is a relative pathname
            // we modify it to prepend the given directory
            if (dir[dir.length()-1 ] == ('/') ) {
                // the dir already has a trailing '/', so we can
                // just concatenate them to get an absolute path
                ret =  dir + file;
            } else {
                // the dir does not have a trailing '/', so we must
                // add a '/' to get the absolute path
                ret =  dir + "/" + file;
            }
        } else {
            // file starts with a '/', so it is an absolute pathname
            // DO NOT prepend the directory, regardless of what the
            // directory variable holds.
            // nothing to do in this case
        }

    }
    return ret;
}

// If true is passed for ignore_if_mt_not_set, then an error won't be
// initiated if we're unable to determine if the program is multi-threaded.
// We are unable to determine this if the daemon hasn't yet figured out what
// libraries are linked against the application.  Currently, we identify an
// application as being multi-threaded if it is linked against a thread
// library (eg. libpthreads.so on Linux).  There are cases where we are
// querying whether the app is multi-threaded, but it can't be determined
// yet but it also isn't necessary to know.
bool PCProcess::multithread_capable(bool ignoreIfMtNotSet) {
    if( mt_cache_result_ != not_cached ) {
        if( mt_cache_result_ == cached_mt_true) return true;
        else return false;
    }

    if( mapped_objects.size() <= 1 ) {
        assert( ignoreIfMtNotSet && "Can't query MT state" );
        return false;
    }

    if(    findObject("libpthread.so*", true) // Linux
        || findObject("libpthread-*.so", true) // Linux
        || findObject("libthr.*", true) ) // FreeBSD
    {
        mt_cache_result_ = cached_mt_true;
        return true;
    }

    mt_cache_result_ = cached_mt_false;
    return false;
}

/**
 * Searches for function in order, with preference given first
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(PCProcess *p, std::string func,
                            pdvector<func_instance *> &result) {
    const pdvector<func_instance*>* found = NULL;
    mapped_object *lpthread = p->findObject("libpthread*", true);
    if (lpthread)
        found = lpthread->findFuncVectorByMangled(func);
    if (found) {
	result = *found;
        return;
    }
    mapped_object *lc = p->findObject("libc.so*", true);
    if (lc)
        found = lc->findFuncVectorByMangled(func);
    if (found) {
	result = *found;
        return;
    }
    p->findFuncsByPretty(func, result);
}

bool PCProcess::instrumentMTFuncs() {
    bool res;

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
    pdvector<func_instance *> pthread_self_funcs;
    findThreadFuncs(this, "pthread_self", pthread_self_funcs);
    if (pthread_self_funcs.size() != 1) {
        fprintf(stderr, "[%s:%d] - Found %ld pthread_self functions, expected 1\n",
                __FILE__, __LINE__, (long) pthread_self_funcs.size());
        for (unsigned j=0; j<pthread_self_funcs.size(); j++) {
            func_instance *ps = pthread_self_funcs[j];
            fprintf(stderr, "[%s:%u] - %s in module %s at %lx\n", __FILE__, __LINE__,
                    ps->prettyName().c_str(), ps->mod()->fullName().c_str(),
                    ps->addr());
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

bool PCEventHandler::shouldStopForSignal(int signal) {
    if( signal == SIGSTOP ) return true;
    return false;
}

bool PCEventHandler::isCrashSignal(int signal) {
    switch(signal) {
        case SIGBUS:
        case SIGSEGV:
        case SIGABRT:
        case SIGILL:
        case SIGFPE:
        case SIGTRAP:
            return true;
        default:
            return false;
    }
}

bool PCEventHandler::isKillSignal(int signal) {
    switch(signal) {
        case SIGKILL:
        case SIGTERM:
            return true;
        default:
            return false;
    }
}

bool PCEventHandler::isValidRTSignal(int signal, RTBreakpointVal breakpointVal, 
        Address arg1, int status)
{
    /* Okay... we use both DYNINST_BREAKPOINT_SIGNUM and sigstop,
     * depending on what we're trying to stop. So we have to check the
     *flags against the signal
     */
    if( signal == SIGSTOP ) {
        if( breakpointVal == SoftRTBreakpoint ) {
            if( status == DSE_forkExit ) {
                if( arg1 == 0 ) return true;

                proccontrol_printf("%s[%d]: parent process received SIGSTOP\n",
                        FILE__, __LINE__);
            }else{
                proccontrol_printf("%s[%d]: SIGSTOP wasn't due to fork exit\n",
                        FILE__, __LINE__);
            }
        }else{
            proccontrol_printf("%s[%d]: mismatch in signal for breakpoint type\n",
                    FILE__, __LINE__);
        }
    }else if( signal == DYNINST_BREAKPOINT_SIGNUM ) {
        if( breakpointVal == NormalRTBreakpoint ) {
            if( (status != DSE_forkExit) || (arg1 != 0) ) return true;

            proccontrol_printf("%s[%d]: child received signal %d\n",
                    FILE__, __LINE__, DYNINST_BREAKPOINT_SIGNUM);
        }else{
            proccontrol_printf("%s[%d]: mismatch in signal for breakpoint type\n",
                    FILE__, __LINE__);
        }
    }else{
        proccontrol_printf("%s[%d]: signal wasn't sent by RT library\n",
                FILE__, __LINE__);
    }

    return false;
}

mapped_object *PCProcess::createObjectNoFile(Address) {
    assert(0); // not implemented on UNIX
    return NULL;
}

void PCProcess::changeMemoryProtections(Address addr, size_t size,
                                        PCMemPerm rights, bool setShadow) {
    (void)addr;
    (void)size;
    (void)rights;
    (void)setShadow;
    assert(!"Not implemented yet");
}

bool PCProcess::setMemoryAccessRights(Address start, size_t size,
                                      PCMemPerm rights) {
    mal_printf("setMemoryAccessRights to %s [%lx %lx]\n",
               rights.getPermName().c_str(), start, start+size);
    assert(!"Not implemented yet");
    return false;
}

bool PCProcess::getMemoryAccessRights(Address start, PCMemPerm& rights) {
    mal_printf("getMemoryAccessRights at %lx\n", start);
    assert(!"Not implemented yet");
    (void)rights; // unused parameter
    return false;
}

void PCProcess::redirectFds(int stdin_fd, int stdout_fd, int stderr_fd,
        std::map<int, int> &fds)
{
    if( stdin_fd != 0 ) fds.insert(std::make_pair(stdin_fd, 0));
    if( stdout_fd != 1 ) fds.insert(std::make_pair(stdout_fd, 1));
    if( stderr_fd != 2 ) fds.insert(std::make_pair(stderr_fd, 2));
}

bool PCProcess::getDyninstRTLibName()
{
    startup_printf("Begin getDyninstRTLibName\n");
    bool use_abi_rt = false;
#if defined(arch_64bit)
    use_abi_rt = (getAddressWidth() == 4);
#endif

    std::vector<std::string> rt_paths;
    std::string rt_base = "libdyninstAPI_RT";
    if(use_abi_rt) rt_base += "_m32";
    rt_base += ".so";
    if(!BinaryEdit::getResolvedLibraryPath(rt_base, rt_paths) || rt_paths.empty())
    {
	startup_printf("%s[%d]: Could not find libdyninstAPI_RT.so in search path\n", FILE__, __LINE__);
	return false;
    }
    for(auto i = rt_paths.begin();
	i != rt_paths.end();
	++i)
    {
	startup_printf("%s[%d]: Candidate RTLib is %s\n", FILE__, __LINE__, i->c_str());
    }
    dyninstRT_name = rt_paths[0];
    return true;
}

bool PCProcess::setEnvPreload(std::vector<std::string> &envp, std::string fileName) {
    const unsigned int ERROR_CODE = 101;
    bool use_abi_rt = false;

#if defined(arch_64bit)
    SymtabAPI::Symtab *symt_obj;
    bool result = SymtabAPI::Symtab::openFile(symt_obj, fileName);
    if( !result ) return false;

    use_abi_rt = (symt_obj->getAddressWidth() == 4);
#endif

    std::vector<std::string> rt_paths;
    std::string rt_base = "libdyninstAPI_RT";
    if(use_abi_rt) rt_base += "_m32";
    rt_base += ".so";
    if(!BinaryEdit::getResolvedLibraryPath(rt_base, rt_paths) || rt_paths.empty())
    {
	startup_printf("%s[%d]: Could not find libdyninstAPI_RT.so in search path\n", FILE__, __LINE__);
      return false;
    }
    for(auto i = rt_paths.begin();
	i != rt_paths.end();
	++i)
    {
	startup_printf("%s[%d]: Candidate RTLib is %s\n", FILE__, __LINE__, i->c_str());
    }
    std::string rt_lib_name = rt_paths[0];

    // Check to see if the library given exists.
    if (access(rt_lib_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + rt_lib_name +
                          std::string(" does not exist or cannot be accessed!");
        cerr << msg << endl;
        showErrorCallback(ERROR_CODE, msg);
        return false;
    }

    const char *var_name = "LD_PRELOAD";
    if (envp.size()) {
        // Check if some LD_PRELOAD is already part of the environment.
        std::vector<std::string>::iterator ldPreloadVal =  envp.end();
        for(std::vector<std::string>::iterator i = envp.begin();
                i != envp.end(); ++i)
        {
            if( (*i) == var_name ) {
                ldPreloadVal = i;
                break;
            }
        }

        if (ldPreloadVal == envp.end()) {
            // Not found, append an entry
            std::string ld_preload = std::string(var_name) + std::string("=") +
                                     rt_lib_name;
            startup_printf("LD_PRELOAD=%s\n", ld_preload.c_str());
            envp.push_back(ld_preload);
        } else {
            // Found, modify envs in-place
            std::string ld_preload = *ldPreloadVal + std::string(":") +
                                     rt_lib_name;
            startup_printf("LD_PRELOAD=%s\n", ld_preload.c_str());
            *ldPreloadVal = ld_preload;
        }
    } else {
        // Environment inherited from this process, copy the current
        // environment to envp, modifying/adding LD_PRELOAD
        char *ld_preload_orig = NULL;
        int i = 0;
        while( environ[i] != NULL ) {
            std::string envVar(environ[i]);
            if( envVar.find("LD_PRELOAD=") == 0 ) {
                ld_preload_orig = environ[i]+11;
            }else{
                envp.push_back(envVar);
            }
            i++;
        }

        std::string ld_preload;
        if (ld_preload_orig) {
            // Append to existing var
            ld_preload = std::string(var_name) + std::string("=") +
                         std::string(ld_preload_orig) + std::string(":") +
                         rt_lib_name;
        } else {
            // Define a new var
            ld_preload = std::string(var_name) + std::string("=") +
                         rt_lib_name;
        }
        envp.push_back(ld_preload);
    }

    return true;
}


bool PCProcess::getExecFileDescriptor(string filename,
        bool, fileDescriptor &desc)
{
   Address base = 0;

    desc = fileDescriptor(filename.c_str(),
                          base, // code
                          base, // data
                          false); // a.out
    return true;
}

/**
 * Strategy:  The program entry point is in /lib/ld-2.x.x at the 
 * _start function.  Get the current PC, parse /lib/ld-2.x.x, and 
 * compare the two points.
 **/
bool PCProcess::hasPassedMain() 
{
   using namespace SymtabAPI;
   Symtab *ld_file = NULL;
   Address entry_addr, ldso_start_addr = 0;

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

   std::string derefPath = resolve_file_path(path);

   // Search for the dynamic linker in the loaded libraries
   const LibraryPool &libraries = pcProc_->libraries();
   bool foundDynLinker = false;
   for(LibraryPool::const_iterator i = libraries.begin(); i != libraries.end();
           ++i)
   {
       if( (*i)->getAbsoluteName() == derefPath ) {
           foundDynLinker = true;
           ldso_start_addr = (*i)->getLoadAddress();
       }
   }

   if( !foundDynLinker ) {
       // This means that libraries haven't been loaded yet which implies
       // that main hasn't been reached yet
       return false;
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

   if( !getOPDFunctionAddr(entry_addr) ) {
       startup_printf("[%s:%u] - failed to read entry addr function pointer\n",
               FILE__, __LINE__);
       return false;
   }

   if( entry_addr < ldso_start_addr ) {
       entry_addr += ldso_start_addr;
   }
   
   bool result = (entry_addr != current_pc);
   startup_printf("[%s:%u] - hasPassedMain returning %d (%lx %lx)\n",
                  FILE__, __LINE__, (int) result, entry_addr, current_pc);

   return result;
}

bool PCProcess::startDebugger() {
    std::stringstream pidStr;
    pidStr << getPid();

    const char *args[4];
    args[0] = dyn_debug_crash_debugger;
    args[1] = file_.c_str();
    args[2] = pidStr.str().c_str();
    args[3] = NULL;

    proccontrol_printf("%s[%d]: Launching %s %s %s\n", FILE__, __LINE__,
            args[0], args[1], args[2]);
    if( execv(args[0], const_cast<char **>(args)) == -1 ) {
        perror("execv");
        return false;
    }

    return true;
}

// The following functions are only implemented on some Unices //

#if defined(os_linux) || defined(os_freebsd)

#include "dyninstAPI/src/binaryEdit.h"
#include "symtabAPI/h/Archive.h"

using namespace Dyninst::SymtabAPI;


mapped_object *BinaryEdit::openResolvedLibraryName(std::string filename,
                                                   std::map<std::string, BinaryEdit*> &retMap) {
    std::vector<std::string> paths;
    std::vector<std::string>::iterator pathIter;
    // First, find the specified library file
    bool resolved = getResolvedLibraryPath(filename, paths);

    // Second, create a set of BinaryEdits for the found library
    if ( resolved ) {
        startup_printf("[%s:%u] - Opening dependent file %s\n",
                       FILE__, __LINE__, filename.c_str());

        Symtab *origSymtab = getMappedObject()->parse_img()->getObject();
	assert(mgr());
        // Dynamic case
        if ( !origSymtab->isStaticBinary() ) {
            for(pathIter = paths.begin(); pathIter != paths.end(); ++pathIter) {
               BinaryEdit *temp = BinaryEdit::openFile(*pathIter, mgr(), patcher());

                if (temp && temp->getAddressWidth() == getAddressWidth()) {
                    retMap.insert(std::make_pair(*pathIter, temp));
                    return temp->getMappedObject();
                }
                delete temp;
            }
        } else {
            // Static executable case

            /* 
             * Alright, this is a kludge, but even though the Archive is opened
             * twice (once here and once by the image class later on), it is
             * only parsed once because the Archive class keeps track of all
             * open Archives.
             *
             * This is partly due to the fact that Archives are collections of
             * Symtab objects and their is one Symtab for each BinaryEdit. In
             * some sense, an Archive is a collection of BinaryEdits.
             */
            for(pathIter = paths.begin(); pathIter != paths.end(); ++pathIter) {
                Archive *library;
                Symtab *singleObject;
                if (Archive::openArchive(library, *pathIter)) {
                    std::vector<Symtab *> members;
                    if (library->getAllMembers(members)) {
                        std::vector <Symtab *>::iterator member_it;
                        for (member_it = members.begin(); member_it != members.end();
                             ++member_it) 
                        {
                           BinaryEdit *temp = BinaryEdit::openFile(*pathIter, 
                                                                   mgr(), patcher(), (*member_it)->memberName());

                            if (temp && temp->getAddressWidth() == getAddressWidth()) {
                                std::string mapName = *pathIter + string(":") +
                                    (*member_it)->memberName();
                                retMap.insert(std::make_pair(mapName, temp));
                            }else{
                                if(temp) delete temp;
                                retMap.clear();
                                break;
                            }
                        }

                        if (retMap.size() > 0) {
                            origSymtab->addLinkingResource(library);
                            // So we tried loading "libc.a", and got back a swarm of individual members. 
                            // Lovely. 
                            // Just return the first thing...
                            return retMap.begin()->second->getMappedObject();
                        }
                        //if( library ) delete library;
                    }
                } else if (Symtab::openFile(singleObject, *pathIter)) {
                   BinaryEdit *temp = BinaryEdit::openFile(*pathIter, mgr(), patcher());


                    if (temp && temp->getAddressWidth() == getAddressWidth()) {
                        if( singleObject->getObjectType() == obj_SharedLib ||
                            singleObject->getObjectType() == obj_Executable ) 
                        {
                          startup_printf("%s[%d]: cannot load dynamic object(%s) when rewriting a static binary\n", 
                                  FILE__, __LINE__, pathIter->c_str());
                          std::string msg = std::string("Cannot load a dynamic object when rewriting a static binary");
                          showErrorCallback(71, msg.c_str());

                          delete singleObject;
                        }else{
                            retMap.insert(std::make_pair(*pathIter, temp));
                            return temp->getMappedObject();
                        }
                    }
                    if(temp) delete temp;
                }
            }
        }
    }

    startup_printf("[%s:%u] - Creation error opening %s\n",
                   FILE__, __LINE__, filename.c_str());
    // If the only thing we could find was a dynamic lib for a static executable, we can reach here; caller should handle this.
    return NULL;
}

#endif

#if defined(os_linux) || defined(os_freebsd)

#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/parse-cfg.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/addressSpace.h"
#include "symtabAPI/h/Symtab.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/debug.h"
#include "boost/tuple/tuple.hpp"
#include <elf.h>

#if defined(os_linux)
#include "dyninstAPI/src/linux.h"
#else
#include "dyninstAPI/src/freebsd.h"
#endif

// The following functions were factored from linux.C to be used
// on both Linux and FreeBSD

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's func_instance.  
// If the function has not yet been bound, then "target" is set to the 
// func_instance associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
// HACK: made an func_instance method to remove from instPoint class...
// FURTHER HACK: made a block_instance method so we can share blocks
func_instance *block_instance::callee() {
   // Check 1: pre-computed callee via PLT
   func_instance *ret = obj()->getCallee(this);
   if (ret) return ret;

   // See if we've already done this
   edge_instance *tEdge = getTarget();
   if (!tEdge) {
      return NULL;
   }

   if (!tEdge->sinkEdge()) {
      func_instance *tmp = obj()->findFuncByEntry(tEdge->trg());
      if (tmp && !(tmp->ifunc()->isPLTFunction())) {
         return tmp;
      }
   }

   

   // Do this the hard way - an inter-module jump
   // get the target address of this function
   Address target_addr; bool success;
   boost::tie(success, target_addr) = llb()->callTarget();
   if(!success) {
      // this is either not a call instruction or an indirect call instr
      // that we can't get the target address
      //fprintf(stderr, "%s[%d]:  returning NULL\n", FILE__, __LINE__);
      return NULL;
   }
   
   // get the relocation information for this image
   Symtab *sym = obj()->parse_img()->getObject();
   pdvector<relocationEntry> fbt;
   vector <relocationEntry> fbtvector;
   if (!sym->getFuncBindingTable(fbtvector)) {
      //fprintf(stderr, "%s[%d]:  returning NULL\n", FILE__, __LINE__);
      return NULL;
   }


   /**
    * Object files and static binaries will not have a function binding table
    * because the function binding table holds relocations used by the dynamic
    * linker
    */
   if (!fbtvector.size() && !sym->isStaticBinary() && 
           sym->getObjectType() != obj_RelocatableFile ) 
   {
      fprintf(stderr, "%s[%d]:  WARN:  zero func bindings\n", FILE__, __LINE__);
   }

   for (unsigned index=0; index< fbtvector.size();index++)
      fbt.push_back(fbtvector[index]);
   
   Address base_addr = obj()->codeBase();
   
   std::map<Address, std::string> pltFuncs;
   obj()->parse_img()->getPltFuncs(pltFuncs);

   // find the target address in the list of relocationEntries
   if (pltFuncs.find(target_addr) != pltFuncs.end()) {
      for (u_int i=0; i < fbt.size(); i++) {
         if (fbt[i].target_addr() == target_addr) 
         {
            // check to see if this function has been bound yet...if the
            // PLT entry for this function has been modified by the runtime
            // linker
            func_instance *target_pdf = 0;
            if (proc()->hasBeenBound(fbt[i], target_pdf, base_addr)) {
               updateCallTarget(target_pdf);
               obj()->setCalleeName(this, target_pdf->symTabName());
               obj()->setCallee(this, target_pdf);
               return target_pdf;
            }
         }
      }
      const char *target_name = pltFuncs[target_addr].c_str();
      PCProcess *dproc = dynamic_cast<PCProcess *>(proc());

      BinaryEdit *bedit = dynamic_cast<BinaryEdit *>(proc());
      obj()->setCalleeName(this, std::string(target_name));
      pdvector<func_instance *> pdfv;

      // See if we can name lookup
      if (dproc) {
         if (proc()->findFuncsByMangled(target_name, pdfv)) {
            obj()->setCallee(this, pdfv[0]);
            updateCallTarget(pdfv[0]);
            return pdfv[0];
         }
      }
      else if (bedit) {
         std::vector<BinaryEdit *>::iterator i;
         for (i = bedit->getSiblings().begin(); i != bedit->getSiblings().end(); i++)
         {
            if ((*i)->findFuncsByMangled(target_name, pdfv)) {
               obj()->setCallee(this, pdfv[0]);
               updateCallTarget(pdfv[0]);
               return pdfv[0];
            }
         }
      }
      else 
         assert(0);
   }
   
   //fprintf(stderr, "%s[%d]:  returning NULL: target addr = %p\n", FILE__, __LINE__, (void *)target_addr);
   return NULL;
}

void BinaryEdit::makeInitAndFiniIfNeeded()
{
    using namespace Dyninst::SymtabAPI;

    Symtab* linkedFile = getAOut()->parse_img()->getObject();

    // Disable this for .o's and static binaries
    if( linkedFile->isStaticBinary() || 
        linkedFile->getObjectType() == obj_RelocatableFile ) 
    {
        return;
    }

    bool foundInit = false;
    bool foundFini = false;
    vector <Function *> funcs;
    if (linkedFile->findFunctionsByName(funcs, "_init")) {
        foundInit = true;
    }
    if (linkedFile->findFunctionsByName(funcs, "_fini")) {
        foundFini = true;
    }
    if( !foundInit )
    {
        Offset initOffset = linkedFile->getInitOffset();
        Region *initsec = linkedFile->findEnclosingRegion(initOffset);
        if(!initOffset || !initsec)
        {
            unsigned char* emptyFunction = NULL;
            int emptyFuncSize = 0;
#if defined(arch_x86) || defined(arch_x86_64)
            static unsigned char empty_32[] = { 0x55, 0x89, 0xe5, 0xc9, 0xc3 };
            static unsigned char empty_64[] = { 0x55, 0x48, 0x89, 0xe5, 0xc9, 0xc3 };
            if(linkedFile->getAddressWidth() == 8)
            {
                emptyFunction = empty_64;
                emptyFuncSize = 6;
            }
            else
            {
                emptyFunction = empty_32;
                emptyFuncSize = 5;
            }
#elif defined (arch_power)
            static unsigned char empty[] = { 0x4e, 0x80, 0x00, 0x20};
             emptyFunction = empty;
             emptyFuncSize = 4;
#endif //defined(arch_x86) || defined(arch_x86_64)
            linkedFile->addRegion(highWaterMark_, (void*)(emptyFunction), emptyFuncSize, ".init.dyninst",
                                  Dyninst::SymtabAPI::Region::RT_TEXT, true);
            highWaterMark_ += emptyFuncSize;
            lowWaterMark_ += emptyFuncSize;
            linkedFile->findRegion(initsec, ".init.dyninst");
            assert(initsec);
            linkedFile->addSysVDynamic(DT_INIT, initsec->getMemOffset());
            startup_printf("%s[%d]: creating .init.dyninst region, region addr 0x%lx\n",
                           FILE__, __LINE__, initsec->getMemOffset());
        }
        startup_printf("%s[%d]: ADDING _init at 0x%lx\n", FILE__, __LINE__, initsec->getMemOffset());
        Symbol *initSym = new Symbol( "_init",
                                      Symbol::ST_FUNCTION,
                                      Symbol::SL_GLOBAL,
                                      Symbol::SV_DEFAULT,
                                      initsec->getMemOffset(),
                                      linkedFile->getDefaultModule(),
                                      initsec,
                                      UINT_MAX );
        linkedFile->addSymbol(initSym);
    }
    if( !foundFini )
    {
        Offset finiOffset = linkedFile->getFiniOffset();
        Region *finisec = linkedFile->findEnclosingRegion(finiOffset);
        if(!finiOffset || !finisec)
        {
            unsigned char* emptyFunction = NULL;
            int emptyFuncSize = 0;
#if defined(arch_x86) || defined(arch_x86_64)
            static unsigned char empty_32[] = { 0x55, 0x89, 0xe5, 0xc9, 0xc3 };
            static unsigned char empty_64[] = { 0x55, 0x48, 0x89, 0xe5, 0xc9, 0xc3 };
            if(linkedFile->getAddressWidth() == 8)
            {
                emptyFunction = empty_64;
                emptyFuncSize = 6;
            }
            else
            {
                emptyFunction = empty_32;
                emptyFuncSize = 5;
            }

#elif defined (arch_power)
            static unsigned char empty[] = { 0x4e, 0x80, 0x00, 0x20};
             emptyFunction = empty;
             emptyFuncSize = 4;
#endif //defined(arch_x86) || defined(arch_x86_64)
            linkedFile->addRegion(highWaterMark_, (void*)(emptyFunction), emptyFuncSize, ".fini.dyninst",
                                  Dyninst::SymtabAPI::Region::RT_TEXT, true);
            highWaterMark_ += emptyFuncSize;
            lowWaterMark_ += emptyFuncSize;
            linkedFile->findRegion(finisec, ".fini.dyninst");
            assert(finisec);
            linkedFile->addSysVDynamic(DT_FINI, finisec->getMemOffset());
            startup_printf("%s[%d]: creating .fini.dyninst region, region addr 0x%lx\n",
                           FILE__, __LINE__, finisec->getMemOffset());

        }
        startup_printf("%s[%d]: ADDING _fini at 0x%lx\n", FILE__, __LINE__, finisec->getMemOffset());
        Symbol *finiSym = new Symbol( "_fini",
                                      Symbol::ST_FUNCTION,
                                      Symbol::SL_GLOBAL,
                                      Symbol::SV_DEFAULT,
                                      finisec->getMemOffset(),
                                      linkedFile->getDefaultModule(),
                                      finisec,
                                      UINT_MAX );
        linkedFile->addSymbol(finiSym);
    }
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
   Address loadAddr = 0;

   //Get the inode for the a.out
   startup_printf("[%s:%u] - a.out is a shared library, computing load addr\n",
                  FILE__, __LINE__);
   memset(&aout, 0, sizeof(aout));
   result = stat(pcProc_->libraries().getExecutable()->getAbsoluteName().c_str(), &aout);
   if (result == -1) {
      startup_printf("[%s:%u] - setAOutLoadAddress couldn't stat %s: %s\n",
                     FILE__, __LINE__, proc_path, strerror(errno));
      goto done;
   }
                    
   //Get the maps
   maps = getVMMaps(getPid(), maps_size);
   if (!maps) {
      startup_printf("[%s:%u] - setAOutLoadAddress, getVMMaps return NULL\n",
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
         loadAddr = maps[i].start;
      }
   }
        
 done:
   if (maps)
      free(maps);

   return loadAddr;
}

#endif

