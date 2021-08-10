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

#include "common/src/addrtranslate.h"
#include "common/src/addrtranslate-sysv.h"
#include "common/src/linuxKludges.h"
#include "common/src/parseauxv.h"
#include "common/src/pathName.h"

#include <elf.h>
#include <cstdio>
#include <linux/limits.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h> // for getpid()


using namespace Dyninst;

class ProcessReaderPtrace : public ProcessReader {
   int pid;
public:
   ProcessReaderPtrace(int pid_);
   virtual bool start();
   virtual bool ReadMem(Address inTraced, void *inSelf, unsigned amount);
   virtual bool GetReg(MachRegister /*reg*/, MachRegisterVal &/*val*/) { 
     assert(0); 
     return false;
   }
   virtual bool done();

   virtual ~ProcessReaderPtrace();
};

bool ProcessReaderPtrace::start()
{
   long result;
   bool is_attached = false;

   result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
   if (result == -1)
      goto done;
   is_attached = true;
   
   int status;
   for (;;) {
      result = (long) waitpid(pid, &status, 0);
      if (result == -1 && errno == EINTR)
         continue;
      else if (result == -1)
         goto done;
      else if (WIFEXITED(status) || WIFSIGNALED(status))
         goto done;
      else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP)
         break;
      else if (WIFSTOPPED(status) && WSTOPSIG(status) != SIGSTOP) {
         result = ptrace(PTRACE_CONT, pid, NULL, (void *) (long) WSTOPSIG(status));
         if (result == -1)
            goto done;
      }
   }

   result = true;
done:
   if (!result && is_attached)
      done();
   return result;
}

bool ProcessReaderPtrace::done()
{
   long result;
   result = ptrace(PTRACE_DETACH, pid, NULL, NULL);
   if (result == -1)
      return false;
   return true;
}

ProcessReaderPtrace::ProcessReaderPtrace(int pid_) :
   pid(pid_)
{
}

ProcessReaderPtrace::~ProcessReaderPtrace() 
{
}

bool ProcessReaderPtrace::ReadMem(Address inTraced, void *inSelf, unsigned amount)
{
   bool result;
   result = PtraceBulkRead(inTraced, amount, inSelf, pid);
   return result;
}

ProcessReader *AddressTranslateSysV::createDefaultDebugger(int pid_)
{
  return new ProcessReaderPtrace(pid_);
}

bool AddressTranslateSysV::setInterpreter()
{
   bool result;

   if (interpreter)
      return true;

   string sname = getExecName();
   string interp_name;

   FCNode *exe = files.getNode(sname, symfactory);
   if (!exe) {
      result = false;
      goto done;
   }

   interp_name = exe->getInterpreter();
   interpreter = files.getNode(interp_name, symfactory);
   if (interpreter)
      interpreter->markInterpreter();
   result = true;

 done:
   return result;
}

bool AddressTranslateSysV::setAddressSize() 
{
   bool result;
   if (address_size)
      return true;

   result = setInterpreter();
   if (!result)
      return false;

   if (interpreter)
      address_size = interpreter->getAddrSize();
   else if (pid == getpid()) {
      address_size = sizeof(void *);
      return true;
   }
   else {
      char name[64];
      sprintf(name, "/proc/%d/exe", pid);
      string sname(name);
      FCNode *exe = files.getNode(sname, symfactory);
      if (!exe) 
         return false;
      address_size = exe->getAddrSize();
      if (!address_size)
        return false;
   }
      
   return true;
}

string AddressTranslateSysV::getExecName() 
{
   if (exec_name.empty()) {
      char name[64];
      snprintf(name, 64, "/proc/%d/exe", pid);
      exec_name = resolve_file_path(name);
   }
   return exec_name;
}

#include <iostream>
using namespace std;

LoadedLib *AddressTranslateSysV::getAOut()
{
   if (exec)
      return exec;

   //
   // 

   // Get the base address
   Address baseAddr = 0;
   if (program_base && reader) {
      bool done = false;
      Address phdr_vaddr = (Address) -1;
      Address load_vaddr = (Address) -1;
      Address header_addr = program_base;

      while (!done) {
         Address p_vaddr;
         unsigned type;
         if (address_size == 4) {
            Elf32_Phdr header;
            if (!reader->ReadMem(header_addr, &header, sizeof(Elf32_Phdr))) {
               break;
            }
            p_vaddr = header.p_vaddr;
            type = header.p_type;
            header_addr += sizeof(Elf32_Phdr);
         }
         else {
            Elf64_Phdr header;
            if (!reader->ReadMem(header_addr, &header, sizeof(Elf64_Phdr))) {
               break;
            }
            p_vaddr = header.p_vaddr;
            type = header.p_type;
            header_addr += sizeof(Elf64_Phdr);
         }
         switch (type) {
            case PT_PHDR:
               phdr_vaddr = p_vaddr;
               break;
            case PT_LOAD:
               if (load_vaddr == (Address) -1) {
                  load_vaddr = p_vaddr;
               }
               break;
            case PT_NULL:
            case PT_DYNAMIC:
            case PT_INTERP:
            case PT_NOTE:
            case PT_SHLIB:
            case PT_TLS:
               break;
            default:
               done = true;
         }
         if (phdr_vaddr != (Address) -1 &&
             load_vaddr != (Address) -1) {
            done = true;
         }
      }
      if (load_vaddr != 0) {
         baseAddr = 0;
      }
      else {
         // PIE binary, find it
         assert(phdr_vaddr != (Address) -1);
         baseAddr = program_base - phdr_vaddr;
      }
   }         

   LoadedLib *ll = new LoadedLib(getExecName(), baseAddr);
   ll->setFactory(symfactory);
   exec = ll;
   return ll;
}
