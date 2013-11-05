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

#include <linux/limits.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 

#include <cstdlib>
#include <cerrno>
#include <sstream>
using namespace std;

using namespace Dyninst;

ProcessReader *AddressTranslateSysV::createDefaultDebugger(int) {
  return NULL;
}


bool AddressTranslateSysV::setAddressSize() 
{
  address_size = sizeof(void *);
  translate_printf("[%s:%u] - Set address size to %z.\n", __FILE__, __LINE__, address_size);
  return true;
}


static string deref_link(const char *path) {
  char buffer[PATH_MAX];
  char *p = realpath(path, buffer);
  return p ? string(p) : string(path);
}


string AddressTranslateSysV::getExecName() 
{
  if (exec_name.empty())
  {
     ostringstream linkstream;
     linkstream << "/jobs/" << getenv("BG_JOBID") << "/exe";
     
     string linkname(linkstream.str());
     exec_name = deref_link(linkname.c_str());
     
     translate_printf("[%s:%u] - Got excutable path from %s: '%s'\n",
                      __FILE__, __LINE__, linkname.c_str(), exec_name.c_str());
  }
  return exec_name;  
}


LoadedLib *AddressTranslateSysV::getAOut()
{
   return new LoadedLib(getExecName(), 0);
}


bool AddressTranslateSysV::setInterpreter() 
{
  const char *fullpath = getExecName().c_str();
  FCNode *exe = files.getNode(fullpath, symfactory);
  if (!exe) {
    translate_printf("[%s:%u] - Unable to get FCNode: '%s'\n", __FILE__, __LINE__, fullpath);
    return false;
  }

  translate_printf("[%s:%u] - About to set interpreter.\n", __FILE__, __LINE__);
  string interp_name = exe->getInterpreter();
  if (interp_name == std::string("")) {
     return false;
  }
  interpreter = files.getNode(interp_name, symfactory);
  if (interpreter)
     interpreter->markInterpreter();
  translate_printf("[%s:%u] - Set interpreter name: '%s'\n", __FILE__, __LINE__, interp_name.c_str());

  return true;
}

#if defined(os_bg_compute)

#include <elf.h>
#include <link.h>
extern char **environ;

bool AddressTranslateSysV::setInterpreterBase() {
    if (set_interp_base) return true;

    Elf32_auxv_t *auxv;
    char **env_end = environ;
    while (*env_end++ != NULL);
    
    auxv = (Elf32_auxv_t *) env_end;
    for (;;) {
       if (auxv->a_type == AT_BASE) {
          interpreter_base = auxv->a_un.a_val;
          set_interp_base = true;
          return true;
       }
       if (auxv->a_type == AT_NULL) {
          set_interp_base = true;
          interpreter_base = 0;
          return 0;
       }
       auxv++;
    }

}
#endif
