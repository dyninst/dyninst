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

#include "unix.h"
#include "snippets.h"
#include "procpool.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <string>
#include "common/src/vm_maps.h"
#if defined(os_linux)
#include "common/src/linuxKludges.h"
#elif defined(os_freebsd)
#include "common/src/freebsdKludges.h"
#endif

using namespace std;

unix_process::unix_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                           std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f)
{
}

unix_process::unix_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
}

unix_process::~unix_process()
{
}

void unix_process::plat_execv() {
    // Never returns
    typedef const char * const_str;

    const_str *new_argv = (const_str *) calloc(argv.size()+2, sizeof(char *));
    for (unsigned i=0; i<argv.size(); ++i) {
        new_argv[i] = argv[i].c_str();
    }
    new_argv[argv.size()] = (char *) NULL;

    const_str *new_env = (const_str *) calloc(env.size()+1, sizeof(char *));
    for (unsigned i=0; i < env.size(); ++i) {
       new_env[i] = env[i].c_str();
    }
    new_env[env.size()] = (char *) NULL;

    for(std::map<int,int>::iterator fdit = fds.begin(),
            fdend = fds.end();
            fdit != fdend;
            ++fdit) {
        int oldfd = fdit->first;
        int newfd = fdit->second;

        int result = close(newfd);
        if (result == -1) {
            pthrd_printf("Could not close old file descriptor to redirect.\n");
            setLastError(err_internal, "Unable to close file descriptor for redirection");
            exit(-1);
        }

        result = dup2(oldfd, newfd);
        if (result == -1) {
            pthrd_printf("Could not redirect file descriptor.\n");
            setLastError(err_internal, "Failed dup2 call.");
            exit(-1);
        }
        pthrd_printf("DEBUG redirected file!\n");
    }

    if( env.size() ) {
        execve(executable.c_str(), const_cast<char * const *>(new_argv),
                const_cast<char * const *>(new_env));
    }else{
        execv(executable.c_str(), const_cast<char * const*>(new_argv));
    }
    int errnum = errno;
    pthrd_printf("Failed to exec %s: %s\n", executable.c_str(), strerror(errnum));
    if (errnum == ENOENT)
        setLastError(err_nofile, "No such file");
    if (errnum == EPERM || errnum == EACCES)
        setLastError(err_prem, "Permission denied");
    else
        setLastError(err_internal, "Unable to exec process");
    exit(-1);
}

bool unix_process::post_forked()
{
   ProcPool()->condvar()->lock();

   int_thread *thrd = threadPool()->initialThread();
   //The new process is currently stopped, but should be moved to
   // a running state when handlers complete.
   pthrd_printf("Setting state of initial thread after fork in %d\n",
                getPid());
   thrd->getGeneratorState().setState(int_thread::stopped);
   thrd->getHandlerState().setState(int_thread::stopped);
   thrd->getUserState().setState(int_thread::running);

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   //TODO: Remove this and make have the translate layers' fork
   // constructors do the work.
   std::set<response::ptr> async_responses;
   async_ret_t result = initializeAddressSpace(async_responses);
   if (result == aret_async) {
      //Not going to do proper async handling here.  BG is the async platform
      //and BG doesn't have fork.  Going to just do a sync block
      //for testing purposes, but this shouldn't run in production.
      waitForAsyncEvent(async_responses);
      return true;
   }
   return (result == aret_success);
}

unsigned unix_process::getTargetPageSize() {
    static unsigned pgSize = 0;
    if( !pgSize ) pgSize = getpagesize();
    return pgSize;
}

bool unix_process::plat_decodeMemoryRights(Process::mem_perm& perm,
                                           unsigned long rights) {
    switch (rights) {
      default:                                 return false;
      case PROT_NONE:                          perm.clrR().clrW().clrX(); break;
      case PROT_READ:                          perm.setR().clrW().clrX(); break;
      case PROT_EXEC:                          perm.clrR().clrW().setX(); break;
      case PROT_READ | PROT_WRITE:             perm.setR().setW().clrX(); break;
      case PROT_READ | PROT_EXEC:              perm.setR().clrW().setX(); break;
      case PROT_READ | PROT_WRITE | PROT_EXEC: perm.setR().setW().setX(); break;
    }

    return true;
}

bool unix_process::plat_encodeMemoryRights(Process::mem_perm perm,
                                           unsigned long& rights) {
    if (perm.isNone()) {
        rights = PROT_NONE;
    } else if (perm.isR()) {
        rights = PROT_READ;
    } else if (perm.isX()) {
        rights = PROT_EXEC;
    } else if (perm.isRW()) {
        rights = PROT_READ | PROT_WRITE;
    } else if (perm.isRX()) {
        rights = PROT_READ | PROT_EXEC;
    } else if (perm.isRWX()) {
        rights = PROT_READ | PROT_WRITE | PROT_EXEC;
    } else {
        return false;
    }

    return true;
}

bool unix_process::plat_getMemoryAccessRights(Dyninst::Address addr,
                                              Process::mem_perm& perm) {
    (void)addr;
    (void)perm;
    perr_printf("Called getMemoryAccessRights on unspported platform\n");
    setLastError(err_unsupported, "Get Memory Permission not supported on this platform\n");
    // ZUYU TODO
    // parse /proc/self/maps manually for permission of given address
    return false;
}

bool unix_process::plat_setMemoryAccessRights(Dyninst::Address addr,
                                              size_t size,
                                              Process::mem_perm perm,
                                              Process::mem_perm& oldPerm) {
    unsigned long rights;
    if (!plat_encodeMemoryRights(perm, rights)) {
        pthrd_printf("ERROR: unsupported rights for page %lx\n", addr);
        return false;
    }

    if (!mprotect((void*)addr, size, (int)rights)) {
        pthrd_printf("ERROR: failed to set access rights for page %lx\n", addr);
        switch (errno) {
          case EACCES:
              setLastError(err_prem, "Permission denied");
              break;
          case EINVAL:
              setLastError(err_badparam,
                      "Given page address is invalid or not page-aligned");
              break;
          case ENOMEM:
              setLastError(err_badparam, "Insufficient memory, "
                      "or the given memory region contains invalid address space");
              break;
          default:
              setLastError(err_unsupported, "Unknown error code");
              break;
        }
        return false;
    }

    if (!plat_getMemoryAccessRights(addr, oldPerm)) {
        pthrd_printf("ERROR: failed to get access rights for page %lx\n", addr);
        return false;
    }

    return true;
}

bool unix_process::plat_findAllocatedRegionAround(Dyninst::Address addr,
                                                  Process::MemoryRegion& memRegion) {
    (void)addr;
    memRegion.first  = 0;
    memRegion.second = 0;
    perr_printf("Called findAllocatedRegionAround on unspported platform\n");
    setLastError(err_unsupported, "Find Allocated Memory Region not supported on this platform\n");
    return false;
}
//I'm not sure that unix_process is the proper place for this--it's really based on whether
// /proc/PID/maps exists.  Currently, that matches our platforms that use unix_process, so
// I'll leave it be for now.
Dyninst::Address unix_process::plat_mallocExecMemory(Dyninst::Address min, unsigned size) {
    Dyninst::Address result = 0x0;
    bool found_result = false;
    unsigned maps_size;
    map_entries *maps = getVMMaps(getPid(), maps_size);
    assert(maps); //TODO, Perhaps go to libraries for address map if no /proc/
    for (unsigned i=0; i<maps_size; i++) {
        if (!(maps[i].prems & PREMS_EXEC))
            continue;
        if (min + size > maps[i].end)
            continue;
        if (maps[i].end - maps[i].start < size)
            continue;

        if (maps[i].start > min)
            result = maps[i].start;
        else
            result = min;
        found_result = true;
        break;
    }
    assert(found_result);
    free(maps);
    return result;
}

bool unix_process::plat_supportFork()
{
   return true;
}

bool unix_process::plat_supportExec()
{
   return true;
}

std::string int_process::plat_canonicalizeFileName(std::string path)
{
   char *result = realpath(path.c_str(), NULL);
   if (result) {
      string sresult(result);
      free(result);
      return sresult;
   }
   else {
      return path;
   }
}
