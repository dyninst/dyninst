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

#include <procfs.h>
#include "addrtranslate-sysv.h"
#include "common/h/headers.h"

namespace Dyninst {
namespace SymtabAPI {

class ProcessReaderProc : public ProcessReader {
   int as_fd;
   int ctl_fd;
   bool isStopped;
public:
   ProcessReaderProc(int pid_);
   bool start();
   bool readAddressSpace(Address inTraced, unsigned amount,
                         void *inSelf);
   bool done();

   virtual ~ProcessReaderProc();
};

ProcessReader *AddressTranslateSysV::createDefaultDebugger(int pid)
{
   return new ProcessReaderProc(pid);
}

bool AddressTranslateSysV::setInterpreter()
{
   char ld_name[128];
   bool found = false;
   prmap_t map_elm;

   if (interpreter)
      return true;
   setInterpreterBase();
   

   char name[32];
   sprintf(name, "/proc/%d/map", pid);
   int map_fd = P_open(name, O_RDONLY, 0);
   if (map_fd == -1)
     return false;

   while(read(map_fd, &map_elm, sizeof(map_elm)) == sizeof(map_elm)) {
     if (map_elm.pr_vaddr == interpreter_base) {
       sprintf(ld_name, "/proc/%d/object/%s", pid, map_elm.pr_mapname);
       found = true;
       break;
     }
   }

   P_close(map_fd);
   if (!found) 
     return false;

   interpreter = files.getNode(ld_name);
   return (interpreter != NULL);
}

bool AddressTranslateSysV::setAddressSize() 
{
  address_size = sizeof(void *);
  return true;
}

LoadedLib *AddressTranslateSysV::getAOut()
{
  return NULL;
}

#include <sys/procfs.h>

ProcessReaderProc::ProcessReaderProc(int pid_) :
   ProcessReader(pid_),
   as_fd(-1),
   ctl_fd(-1),
   isStopped(false)
{
   
}

bool ProcessReaderProc::start()
{
   char temp[128];
   int result;
   long command;

   //attach
   snprintf(temp, 128, "/proc/%d/ctl", pid);
   ctl_fd = P_open(temp, O_WRONLY | O_EXCL, 0);
   if (ctl_fd == -1)
      goto err;
   
   snprintf(temp, 128, "/proc/%d/as", pid);
   as_fd = P_open(temp, O_RDONLY, 0);
   if (as_fd == -1)
      goto err;;

   //Stop the process
   command = PCSTOP;
   result = write(ctl_fd, &command, sizeof(long));
   if (result != sizeof(long))
      goto err;
   isStopped = true;

   return true;

 err:
   done();
   return false;
}
 
bool ProcessReaderProc::readAddressSpace(Address inTraced, unsigned amount,
                                         void *inSelf)
{
   off64_t loc = (off64_t) inTraced;
   unsigned res = pread64(as_fd, inSelf, amount, loc);
   return (res == amount);
}

bool ProcessReaderProc::done()
{
   int result = 0;
   if (isStopped && ctl_fd != -1) {
      long command[2];
      command[0] = PCRUN;
      command[1] = 0;
      result = write(ctl_fd, command, 2*sizeof(long));
   }
   if (ctl_fd != -1)
      P_close(ctl_fd);
   if (as_fd != -1)
      P_close(as_fd);

   ctl_fd = as_fd = -1;

   if (result != -1)
      isStopped = false;

   return true;
}

ProcessReaderProc::~ProcessReaderProc()
{
}

}
}
