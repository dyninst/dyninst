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
#include "stackwalk/src/bluegenel-swk.h"

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/libstate.h"

#include <string>

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <assert.h>
#include <sstream>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace DebuggerInterface;
using namespace std;

namespace Dyninst {
  namespace Stackwalker {

     static string deref_link(const char *path) {
        char buffer[PATH_MAX];
        char *p = realpath(path, buffer);
        return p ? string(p) : string("");
     }

    // ============================================================ //
    // ProcDebugBG
    // This is the factor function that initially creates a BGL stackwalker.
    // ============================================================ //
    ProcDebug *ProcDebugBG::createProcDebugBG(PID pid, string executable) {
      return new ProcDebugBGL(pid, executable);
    }

    ProcDebugBGL::ProcDebugBGL(PID pid, std::string exe) : ProcDebugBG(pid, exe) { }
    
    ProcDebugBGL::~ProcDebugBGL() { }

    bool ProcDebugBGL::debug_post_attach(ThreadState *) {
       if (executable_path.empty()) {
          char *jobid = getenv("BG_JOBID");
          if (jobid) {
             ostringstream linkstream;
             linkstream << "/jobs/" << jobid << "/exe";
     
             string linkname(linkstream.str());
             executable_path = deref_link(linkname.c_str());
             sw_printf("[%s:%u] - Set executable path to %s\n", FILE__, __LINE__, executable_path.c_str());
          }
       }
       library_tracker = new StaticBinaryLibState(this, executable_path);
       return true;
    }

  } // namespace Stackwalker
} // namespace Dyninst

