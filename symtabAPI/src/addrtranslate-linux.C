/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "symtabAPI/src/addrtranslate.h"
#include "symtabAPI/src/addrtranslate-sysv.h"
#include "common/h/linuxKludges.h"

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


using namespace Dyninst;
using namespace SymtabAPI;

class ProcessReaderPtrace : public ProcessReader {
public:
   ProcessReaderPtrace(int pid_);
   bool start();
   bool readAddressSpace(Address inTraced, unsigned amount,
                         void *inSelf);
   bool done();

   virtual ~ProcessReaderPtrace();
};

bool ProcessReaderPtrace::start()
{
   long result;
   bool is_attached;

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
   ProcessReader(pid_)
{
}

ProcessReaderPtrace::~ProcessReaderPtrace() 
{
}

bool ProcessReaderPtrace::readAddressSpace(Address inTraced, unsigned amount,
                                           void *inSelf)
{
   bool result;
   result = PtraceBulkRead(inTraced, amount, inSelf, pid);
   return result;
}

ProcessReader *AddressTranslateSysV::createDefaultDebugger(int pid)
{
  return new ProcessReaderPtrace(pid);
}

bool AddressTranslateSysV::setInterpreter()
{
   bool result;

   if (interpreter)
      return true;

   char name[64];
   sprintf(name, "/proc/%d/exe", pid);
   string sname(name);
   string interp_name;

   FCNode *exe = files.getNode(sname);
   if (!exe) {
      result = false;
      goto done;
   }

   interp_name = exe->getInterpreter();
   interpreter = files.getNode(interp_name);

   result = true;

 done:
   if (exe)
      delete exe;
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
      return sizeof(void *);
   }
   else {
      char name[64];
      sprintf(name, "/proc/%d/exe", pid);
      string sname(name);
      FCNode *exe = files.getNode(sname);
      if (!exe) 
         return false;
      return exe->getAddrSize();
   }
      
   return true;
}

static char *deref_link(const char *path)
{
   static char buffer[PATH_MAX], *p;
   buffer[PATH_MAX-1] = '\0';
   p = realpath(path, buffer);
   return p;
}

LoadedLib *AddressTranslateSysV::getAOut()
{
  char name[64];
  snprintf(name, 64, "/proc/%d/exe", pid);
  LoadedLib *ll = new LoadedLib(deref_link(name), 0);
  return ll;
}


