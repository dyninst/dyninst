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
#include "proccontrol_comp.h"
#include "communication.h"

using namespace std;

class pc_addlibraryMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_addlibrary_factory()
{
  return new pc_addlibraryMutator();
}

static std::set<Process::const_ptr> lib_success;
static bool had_error;

static Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   logerror("Should not have received breakpoint callback\n");
   had_error = true;
   return Process::cbDefault;
}

static Process::cb_ret_t on_irpc(Event::const_ptr ev)
{
   logerror("Should not have received irpc callback\n");
   had_error = true;
   return Process::cbDefault;
}

static Process::cb_ret_t on_library(Event::const_ptr ev)
{
   EventLibrary::const_ptr evlib = ev->getEventLibrary();
   if (!evlib) {
      logerror("Error, received non library event\n");
      had_error = true;
      return Process::cbDefault;
   }

   if (!evlib->libsRemoved().empty()) {
      logerror("Error, did not expect to have removed a library\n");
      had_error = true;
      return Process::cbDefault;      
   }
   if (evlib->libsAdded().empty()) {
      logerror("Error, empty library callback\n");
      had_error = true;
      return Process::cbDefault;
   }

   bool foundlibtestA = false;
   for (set<Library::ptr>::iterator i = evlib->libsAdded().begin(); i != evlib->libsAdded().end(); i++) {
      Library::ptr lib = *i;
      if (lib->getName().find("testA") != string::npos) {
         if (foundlibtestA) {
            logerror("Error, found libtestA twice");
            had_error = false;
         }
         foundlibtestA = true;
      }
   }

   if (!foundlibtestA) {
      logerror("Error, didn't load libtestA\n");
      had_error = true;
      return Process::cbDefault;
   }

   if (lib_success.find(ev->getProcess()) != lib_success.end()) {
      logerror("Error, library cb delived twice\n");
      had_error = true;
      return Process::cbDefault;      
   }
   lib_success.insert(ev->getProcess());
   return Process::cbDefault;
}

#if !defined(os_windows_test)

#define LIBTESTA "libtestA.so"
#define LIBTESTA_32 "libtestA_m32.so"

#else

#define LIBTESTA "testA.dll"
#define LIBTESTA_32 "testA_m32.dll"

#endif

test_results_t pc_addlibraryMutator::executeTest()
{
   lib_success.clear();
   had_error = false;

   Process::registerEventCallback(EventType::Breakpoint, on_breakpoint);
   Process::registerEventCallback(EventType::Library, on_library);
   Process::registerEventCallback(EventType::RPC, on_irpc);

   std::vector<Process::ptr>::iterator i;

   /* Force load libraries */
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;

      std::string libname;
      if (sizeof(void *) == 8) {
         // 64-bit platform...
         if (Dyninst::getArchAddressWidth(proc->getArchitecture()) == 8) {
            libname = LIBTESTA;
         }
         else {
            libname = LIBTESTA_32;
         }
      }
      else {
         libname = LIBTESTA;
      }

      bool result = proc->addLibrary(libname);
      if (!result) {
         logerror("Error returned from addLibrary call\n");
         had_error = true;
         continue;
      }
      if (lib_success.find(proc) == lib_success.end()) {
         logerror("Library load did not produce callback\n");
         had_error = true;
         continue;
      }
      
   }

   /* Signal that they can now run to completion */
   syncloc loc;
   loc.code = SYNCLOC_CODE;
   bool result = comp->send_broadcast((unsigned char *) &loc, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync message\n");
      had_error = true;
   }

   /* Run processes */
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      bool result = (*i)->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         had_error = true;
      }
   }

   Process::removeEventCallback(on_library);
   Process::removeEventCallback(on_breakpoint);
   Process::removeEventCallback(on_irpc);

   return had_error ? FAILED : PASSED;
}
