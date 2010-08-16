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
#include "dynutil/h/SymReader.h"
#include "dynutil/h/dyntypes.h"
#include "common/h/SymLite-elf.h"
#include "sysv.h"
#include "irpc.h"
#include "snippets.h"

#if defined(os_linux)
#include "common/h/linuxKludges.h"
#elif defined(os_freebsd)
#include "common/h/freebsdKludges.h"
#endif

#include <vector>
#include <string>
#include <set>

using namespace Dyninst;
using namespace std;

SymbolReaderFactory *sysv_process::symreader_factory = NULL;
int_breakpoint *sysv_process::lib_trap = NULL;

sysv_process::sysv_process(Dyninst::PID p, string e, vector<string> a, map<int,int> f) :
   int_process(p, e, a, f),
   translator(NULL),
   lib_initialized(false),
   procreader(NULL)
{
}

sysv_process::sysv_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
   sysv_process *sp = static_cast<sysv_process *>(p);
   breakpoint_addr = sp->breakpoint_addr;
   lib_initialized = sp->lib_initialized;
   if (sp->procreader)
      procreader = new PCProcReader(this);
   if (sp->translator)
      translator = AddressTranslate::createAddressTranslator(pid_,
                                                             procreader,
                                                             symreader_factory);
}

sysv_process::~sysv_process()
{
   if (translator) {
      delete translator;
      translator = NULL;
   }
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
}

PCProcReader::PCProcReader(int_process *proc_) :
   proc(proc_)
{
}

PCProcReader::~PCProcReader()
{
}

bool PCProcReader::start()
{
   word_cache_valid = false;
   return true;
}

bool PCProcReader::done()
{
   word_cache_valid = true;
   return true;
}

bool PCProcReader::ReadMem(Address addr, void *buffer, unsigned size)
{
   if (size != 1) {
      bool result = proc->readMem(buffer, addr, size);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
      }
      return result;
   }

   //Try to optimially handle a case where the calling code
   // reads a string one char at a time.
   assert(size == 1);
   Address aligned_addr = addr - (addr % sizeof(word_cache));
   if (!word_cache_valid || aligned_addr != word_cache_addr) {
      bool result = proc->readMem(&word_cache, aligned_addr, sizeof(word_cache));
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return false;
      }
      word_cache_addr = aligned_addr;
      word_cache_valid = true;
   }
   *((char *) buffer) = ((char *) &word_cache)[addr - aligned_addr];
   return true;
}

bool PCProcReader::GetReg(MachRegister /*reg*/, MachRegisterVal & /*val*/)
{
   assert(0); //Not needed
   return true;
}

bool sysv_process::initLibraryMechanism()
{
   if (lib_initialized) {
      return true;
   }
   lib_initialized = true;

   pthrd_printf("Initializing library mechanism for process %d\n", getPid());
   assert(!procreader);
   procreader = new PCProcReader(this);
   if (!symreader_factory) {
      symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
      assert(symreader_factory);
   }

   assert(!translator);
   translator = AddressTranslate::createAddressTranslator(getPid(), 
                                                          procreader,
                                                          symreader_factory);
   if (!translator) {
      perr_printf("Error creating address translator object\n");
      return false;
   }

   if (!lib_trap) {
      lib_trap = new int_breakpoint(Breakpoint::ptr());
   }

   breakpoint_addr = translator->getLibraryTrapAddrSysV();
   pthrd_printf("Installing library breakpoint at %lx\n", breakpoint_addr);
   bool result = false;
   if (breakpoint_addr) {
      result = addBreakpoint(breakpoint_addr, lib_trap);
   }

   return true;
}

bool sysv_process::refresh_libraries(set<int_library *> &added_libs,
                                     set<int_library *> &rmd_libs)
{
   pthrd_printf("Refreshing list of loaded libraries\n");
   bool result = initLibraryMechanism();
   if (!result) {
      pthrd_printf("refresh_libraries failed to init the libraries\n");
      return false;
   }

   result = translator->refresh();
   if (!result) {
      pthrd_printf("Failed to refresh library list for %d\n", getPid());
   }


   for (set<int_library *>::iterator i = mem->libs.begin(); 
        i != mem->libs.end(); i++) 
   {
      (*i)->setMark(false);
   }

   vector<LoadedLib *> ll_libs;
   translator->getLibs(ll_libs);
   for (vector<LoadedLib *>::iterator i = ll_libs.begin(); i != ll_libs.end(); i++)
   {
      LoadedLib *ll = *i;
      int_library *lib = (int_library *) ll->getUpPtr();
      pthrd_printf("Found library %s at %lx\n", ll->getName().c_str(), 
                   ll->getCodeLoadAddr());
      if (!lib) {
         pthrd_printf("Creating new library object for %s\n", ll->getName().c_str());
         lib = new int_library(ll->getName(), ll->getCodeLoadAddr());
         assert(lib);
         added_libs.insert(lib);
         ll->setUpPtr((void *) lib);
         mem->libs.insert(lib);
      }
      lib->setMark(true);
   }

   set<int_library *>::iterator i = mem->libs.begin();
   while (i != mem->libs.end()) {
      int_library *lib = *i;
      if (lib->isMarked()) {
         i++;
         continue;
      }
      pthrd_printf("Didn't find old library %s at %lx, unloading\n",
                   lib->getName().c_str(), lib->getAddr());
      rmd_libs.insert(lib);
      mem->libs.erase(i++);
   }

   return true;
}

Dyninst::Address sysv_process::getLibBreakpointAddr() const
{
   return breakpoint_addr;
}

bool sysv_process::plat_execed()
{
   pthrd_printf("Rebuilding library trap mechanism after exec on %d\n", getPid());
   if (translator) {
      delete translator;
      translator = NULL;
   }
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
   breakpoint_addr = 0x0;
   lib_initialized = false;
   return initLibraryMechanism();
}

/*
 * Note:
 *
 * The following functions are common to both Linux and FreeBSD.
 *
 * If there is another SysV platform that needs different versions of these
 * functions, the following functions should be factored somehow.
 */
void sysv_process::plat_execv() {
    // Never returns
    typedef const char * const_str;

    const_str *new_argv = (const_str *) calloc(argv.size()+3, sizeof(char *));
    new_argv[0] = executable.c_str();
    unsigned i;
    for (i=1; i<argv.size()+1; i++) {
        new_argv[i] = argv[i-1].c_str();
    }
    new_argv[i+1] = (char *) NULL;

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

    execv(executable.c_str(), const_cast<char * const*>(new_argv));
    int errnum = errno;         
    pthrd_printf("Failed to exec %s: %s\n", 
               executable.c_str(), strerror(errnum));
    if (errnum == ENOENT)
        setLastError(err_nofile, "No such file");
    if (errnum == EPERM || errnum == EACCES)
        setLastError(err_prem, "Permission denied");
    else
        setLastError(err_internal, "Unable to exec process");
    exit(-1);
}

void int_notify::writeToPipe()
{
   if (pipe_out == -1) 
      return;

   char c = 'e';
   ssize_t result = write(pipe_out, &c, 1);
   if (result == -1) {
      int error = errno;
      setLastError(err_internal, "Could not write to notification pipe\n");
      perr_printf("Error writing to notification pipe: %s\n", strerror(error));
      return;
   }
   pthrd_printf("Wrote to notification pipe %d\n", pipe_out);
}

void int_notify::readFromPipe()
{
   if (pipe_out == -1)
      return;

   char c;
   ssize_t result;
   int error;
   do {
      result = read(pipe_in, &c, 1);
      error = errno;
   } while (result == -1 && error == EINTR);
   if (result == -1) {
      int error = errno;
      if (error == EAGAIN) {
         pthrd_printf("Notification pipe had no data available\n");
         return;
      }
      setLastError(err_internal, "Could not read from notification pipe\n");
      perr_printf("Error reading from notification pipe: %s\n", strerror(error));
   }
   assert(result == 1 && c == 'e');
   pthrd_printf("Cleared notification pipe %d\n", pipe_in);
}

bool int_notify::createPipe()
{
   if (pipe_in != -1 || pipe_out != -1)
      return true;

   int fds[2];
   int result = pipe(fds);
   if (result == -1) {
      int error = errno;
      setLastError(err_internal, "Error creating notification pipe\n");
      perr_printf("Error creating notification pipe: %s\n", strerror(error));
      return false;
   }
   assert(fds[0] != -1);
   assert(fds[1] != -1);

   result = fcntl(fds[0], F_SETFL, O_NONBLOCK);
   if (result == -1) {
      int error = errno;
      setLastError(err_internal, "Error setting properties of notification pipe\n");
      perr_printf("Error calling fcntl for O_NONBLOCK on %d: %s\n", fds[0], strerror(error));
      return false;
   }
   pipe_in = fds[0];
   pipe_out = fds[1];


   pthrd_printf("Created notification pipe: in = %d, out = %d\n", pipe_in, pipe_out);
   return true;
}

unsigned sysv_process::getTargetPageSize() {
    static unsigned pgSize = 0;
    if( !pgSize ) pgSize = getpagesize();
    return pgSize;
}

bool installed_breakpoint::plat_install(int_process *proc, bool should_save) {
   pthrd_printf("Platform breakpoint install at %lx in %d\n", 
                addr, proc->getPid());
   if (should_save) {
     switch (proc->getTargetArch())
     {
       case Arch_x86_64:
       case Arch_x86:
         buffer_size = 1;
         break;
       default:
         assert(0);
     }
     assert((unsigned) buffer_size < sizeof(buffer));
     
     bool result = proc->readMem(&buffer, addr, buffer_size);
     if (!result) {
       pthrd_printf("Error reading from process\n");
       return result;
     }
   }
   
   bool result;
   switch (proc->getTargetArch())
   {
      case Arch_x86_64:
      case Arch_x86: {
         unsigned char trap_insn = 0xcc;
         result = proc->writeMem(&trap_insn, addr, 1);
         break;
      }
      default:
         assert(0);
   }
   if (!result) {
      pthrd_printf("Error writing breakpoint to process\n");
      return result;
   }

   return true;
}

bool iRPCMgr::collectAllocationResult(int_thread *thr, Dyninst::Address &addr, bool &err)
{
   switch (thr->llproc()->getTargetArch())
   {
      case Arch_x86_64: {
         Dyninst::MachRegisterVal val = 0;
         bool result = thr->getRegister(x86_64::rax, val);
         assert(result);
         addr = val;
         break;
      }
      case Arch_x86: {
         Dyninst::MachRegisterVal val = 0;
         bool result = thr->getRegister(x86::eax, val);
         assert(result);
         addr = val;
         break;
      }
      default:
         assert(0);
         break;
   }
   //TODO: check addr vs. possible mmap return values.
   err = false;
   return true;
}

// For compatibility 
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

bool iRPCMgr::createAllocationSnippet(int_process *proc, Dyninst::Address addr, 
                                      bool use_addr, unsigned long size, 
                                      void* &buffer, unsigned long &buffer_size, 
                                      unsigned long &start_offset)
{
   const void *buf_tmp = NULL;
   unsigned addr_size = 0;
   unsigned addr_pos = 0;
   unsigned flags_pos = 0;
   unsigned size_pos = 0;

   int flags = MAP_ANONYMOUS | MAP_PRIVATE;
   if (use_addr) 
      flags |= MAP_FIXED;
   else
      addr = 0x0;

   switch (proc->getTargetArch())
   {
      case Arch_x86_64:
         buf_tmp = x86_64_call_mmap;
         buffer_size = x86_64_call_mmap_size;
         start_offset = x86_64_mmap_start_position;
         addr_pos = x86_64_mmap_addr_position;
         flags_pos = x86_64_mmap_flags_position;
         size_pos = x86_64_mmap_size_position;
         addr_size = 8;
         break;
      case Arch_x86:
         buf_tmp = x86_call_mmap;
         buffer_size = x86_call_mmap_size;
         start_offset = x86_mmap_start_position;
         addr_pos = x86_mmap_addr_position;
         flags_pos = x86_mmap_flags_position;
         size_pos = x86_mmap_size_position;
         addr_size = 4;
         break;
      default:
         assert(0);
   }
   
   buffer = malloc(buffer_size);
   memcpy(buffer, buf_tmp, buffer_size);

   //Assuming endianess of debugger and debugee match.
   *((unsigned int *) (((char *) buffer)+size_pos)) = size;
   *((unsigned int *) (((char *) buffer)+flags_pos)) = flags;
   if (addr_size == 8)
      *((unsigned long *) (((char *) buffer)+addr_pos)) = addr;
   else if (addr_size == 4)
      *((unsigned *) (((char *) buffer)+addr_pos)) = (unsigned) addr;
   else 
      assert(0);
   return true;
}

bool iRPCMgr::createDeallocationSnippet(int_process *proc, Dyninst::Address addr, 
                                        unsigned long size, void* &buffer, 
                                        unsigned long &buffer_size, 
                                        unsigned long &start_offset)
{
   const void *buf_tmp = NULL;
   unsigned addr_size = 0;
   unsigned addr_pos = 0;
   unsigned size_pos = 0;

   switch (proc->getTargetArch())
   {
      case Arch_x86_64:
         buf_tmp = x86_64_call_munmap;
         buffer_size = x86_64_call_munmap_size;
         start_offset = x86_64_munmap_start_position;
         addr_pos = x86_64_munmap_addr_position;
         size_pos = x86_64_munmap_size_position;
         addr_size = 8;
         break;
      case Arch_x86:
         buf_tmp = x86_call_munmap;
         buffer_size = x86_call_munmap_size;
         start_offset = x86_munmap_start_position;
         addr_pos = x86_munmap_addr_position;
         size_pos = x86_munmap_size_position;
         addr_size = 4;
         break;
      default:
         assert(0);
   }
   
   buffer = malloc(buffer_size);
   memcpy(buffer, buf_tmp, buffer_size);

   //Assuming endianess of debugger and debugee match.
   *((unsigned int *) (((char *) buffer)+size_pos)) = size;
   if (addr_size == 8)
      *((unsigned long *) (((char *) buffer)+addr_pos)) = addr;
   else if (addr_size == 4)
      *((unsigned *) (((char *) buffer)+addr_pos)) = (unsigned) addr;
   else 
      assert(0);
   return true;
}

Dyninst::Address sysv_process::plat_mallocExecMemory(Dyninst::Address min, unsigned size) {
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
