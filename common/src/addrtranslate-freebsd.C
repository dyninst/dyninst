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

#include "common/src/headers.h"
#include "common/src/addrtranslate-sysv.h"
#include "common/src/freebsdKludges.h"
#include "common/src/vm_maps.h"
#include <cstdio>

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h> 

using namespace Dyninst;

class ProcessReaderPtrace : public ProcessReader {
    protected:
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

bool ProcessReaderPtrace::start() {
    if( 0 != ptrace(PT_ATTACH, pid, (caddr_t)1, 0) ) {
        translate_printf("[%s:%u] - Failed to attach to process %u: %s\n",
                __FILE__, __LINE__, pid, strerror(errno));
        return false;
    }

    int status;
    while(true) {
        int result = (long) waitpid(pid, &status, 0);
        if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1 || WIFEXITED(status) || WIFSIGNALED(status)) {
            translate_printf("[%s:%u] - Failed to stop process %u: %s\n",
                    __FILE__, __LINE__, pid, strerror(errno));
            done();
            return false;
        } else if (WIFSTOPPED(status) && (WSTOPSIG(status) == SIGSTOP 
                || WSTOPSIG(status) == SIGTRAP) ) {
            break;
        } else if (WIFSTOPPED(status) && WSTOPSIG(status) != SIGSTOP) {
            if( 0 != ptrace(PT_CONTINUE, pid, (caddr_t)1, WSTOPSIG(status)) ) {
                translate_printf("[%s:%u] - Failed to continue process %u: %s\n"
                        __FILE__, __LINE__, pid, strerror(errno));
                done();
                return false;
            }
        } else {
            translate_printf("[%s:%u] - Unknown error encountered for process %u: %s\n",
                    __FILE__, __LINE__, pid, strerror(errno));
            done();
            return false;
        }
    }

    return true;
}

bool ProcessReaderPtrace::done()
{
   return ( 0 != ptrace(PT_DETACH, pid, (caddr_t)1, 0) ) ? true : false;
}

ProcessReaderPtrace::ProcessReaderPtrace(int pid_) :
   pid(pid_)
{
}

ProcessReaderPtrace::~ProcessReaderPtrace() 
{
}

bool ProcessReaderPtrace::ReadMem(Address inTraced, void *inSelf, unsigned amount) {
    bool result = PtraceBulkRead(inTraced, amount, inSelf, pid);
    if( !result ) {
        translate_printf("[%s:%u] - Failed to read memory from process: %s\n", 
                __FILE__, __LINE__, strerror(errno));
    }

    return result;
}

/* Complete the implementation of the AddressTranslateSysV class */

ProcessReader *AddressTranslateSysV::createDefaultDebugger(int pid)
{
  return new ProcessReaderPtrace(pid);
}


bool AddressTranslateSysV::setInterpreter() {
    if( interpreter ) return true;

    string l_exec = getExecName();
    if( l_exec.empty() ) {
        return false;
    }

    FCNode *exe = files.getNode(l_exec, symfactory);
    if( !exe ) {
        translate_printf("[%s:%u] - Failed to get FCNode for %s\n",
                __FILE__, __LINE__, l_exec.c_str());
        return false;
    }

    if( exe->getInterpreter().empty() ) {
        translate_printf("[%s:%u] - No interpreter found\n",
                __FILE__, __LINE__);
        return true;
    }

    interpreter = files.getNode(exe->getInterpreter(), symfactory);
    if( interpreter ) interpreter->markInterpreter();
    else{
        translate_printf("[%s:%u] - Failed to set interpreter for %s\n",
                __FILE__, __LINE__, l_exec.c_str());
        return false;
    }

    return true;
}

bool AddressTranslateSysV::setAddressSize() {
    if (address_size) return true;

    if( (address_size = sysctl_computeAddrWidth(pid)) == -1 ) {
        address_size = 0;
        return false;
    }

    return true;
}

bool AddressTranslateSysV::setInterpreterBase() {
    if( set_interp_base ) return true;

    string l_exec = getExecName();
    if( l_exec.empty() ) {
        return false;
    }

    FCNode *exe = files.getNode(l_exec, symfactory);
    if( !exe ) {
        translate_printf("[%s:%u] - Failed to get FCNode for %s\n",
                __FILE__, __LINE__, l_exec.c_str());
        return false;
    }

    unsigned maps_size;
    map_entries *maps = getVMMaps(pid, maps_size);
    if( !maps ) {
        translate_printf("[%s:%u] - Failed to get VM maps\n",
                __FILE__, __LINE__);
        return false;
    }

    string interp_name = exe->getInterpreter();
    for(unsigned i = 0; i < maps_size; ++i) {
        if( string(maps[i].path) == interp_name ) {
            interpreter_base = maps[i].start;
            set_interp_base = true;
            break;
        }
    }
    free(maps);

    if( !set_interp_base ) {
        translate_printf("[%s:%u] - Failed to locate interpreter in memory map\n",
                __FILE__, __LINE__);
        return false;
    }

    return true;
}

string AddressTranslateSysV::getExecName() {
    if( exec_name.empty() ) {
        char *pathname = sysctl_getExecPathname(pid);
        if( NULL != pathname ) {
            exec_name = std::string(pathname);

            free(pathname);
        }
    }
    return exec_name;
}

LoadedLib *AddressTranslateSysV::getAOut()
{
   // TODO: shouldn't this just return exec if it's set?
   LoadedLib *ll = new LoadedLib(getExecName(), 0);
   ll->setFactory(symfactory);
   return ll;
}
