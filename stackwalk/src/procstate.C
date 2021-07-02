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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/src/libstate.h"
#include "common/src/headers.h"
#include <assert.h>
#include <string>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace std;

class DefaultLibState : public LibraryState
{
public:
   DefaultLibState(ProcessState *parent) : 
      LibraryState(parent) 
   {
   }

   virtual bool getLibraryAtAddr(Address, LibAddrPair &) {
      return false;
   }

   virtual bool getLibraries(std::vector<LibAddrPair> &, bool) {
      return false;
   }

   virtual void notifyOfUpdate() {
   }

   virtual Address getLibTrapAddress() {
      return 0x0;
   }

   ~DefaultLibState() { 
   }
};

std::map<Dyninst::PID, ProcessState *> ProcessState::proc_map;

ProcessState::ProcessState(Dyninst::PID pid_, std::string executable_path_) :
   pid(NULL_PID),
   library_tracker(NULL),
   walker(NULL),
   executable_path(executable_path_)
{
   std::map<PID, ProcessState *>::iterator i = proc_map.find(pid_);
   if (i != proc_map.end())
   {
      sw_printf("[%s:%d] - Already attached to debuggee %d\n",
                FILE__, __LINE__, pid_);
      setLastError(err_badparam, "Attach requested to already " \
                   "attached process");
      return;
   }
   setPid(pid_);
}

void ProcessState::setPid(Dyninst::PID pid_)
{
   pid = pid_;
   proc_map[pid] = this;
}

Dyninst::PID ProcessState::getProcessId() 
{
   return pid;
}

bool ProcessState::preStackwalk(Dyninst::THR_ID)
{
   return true;
}

bool ProcessState::postStackwalk(Dyninst::THR_ID)
{
   return true;
}

void ProcessState::setDefaultLibraryTracker()
{
  if (library_tracker) return;

  std::string execp("");
  ProcDebug *pd = dynamic_cast<ProcDebug *>(this);
  if (!library_tracker) {
     if (pd) 
        execp = pd->getExecutablePath();
     library_tracker = new TrackLibState(this, execp);
  }
}

Walker *ProcessState::getWalker() const
{
   return walker;
}

ProcessState::~ProcessState()
{
   if (library_tracker)
      delete library_tracker;
   proc_map.erase(pid);
}

ProcessState *ProcessState::getProcessStateByPid(Dyninst::PID pid) {
   map<PID, ProcessState *>::iterator i = proc_map.find(pid);
   if (i == proc_map.end())
      return NULL;
   return i->second;
}

unsigned ProcSelf::getAddressWidth()
{
   return sizeof(void *);
}

bool ProcSelf::isFirstParty()
{
  return true;
}

ProcSelf::~ProcSelf()
{
}

LibraryState::LibraryState(ProcessState *parent) :
   procstate(parent)
{
}

LibraryState::~LibraryState()
{
}

LibraryState *ProcessState::getLibraryTracker()
{
   return library_tracker;
}

