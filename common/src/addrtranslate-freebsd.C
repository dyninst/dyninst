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

#include "common/h/headers.h"
#include "common/src/addrtranslate-sysv.h"
#include "common/h/freebsdKludges.h"

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
    if( 0 != ptrace(PT_ATTACH, pid, (caddr_t)1, 0) ) return false;

    int status;
    for (;;) {
        int result = (long) waitpid(pid, &status, 0);
        if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1 || WIFEXITED(status) || WIFSIGNALED(status)) {
            done();
            return false;
        } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP) {
            break;
        } else if (WIFSTOPPED(status) && WSTOPSIG(status) != SIGSTOP) {
            // Continue the process, sending a stop
            if( 0 != ptrace(PT_CONTINUE, pid, (caddr_t)1, WSTOPSIG(status)) ) {
                done();
                return false;
            }
        } else {
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

bool ProcessReaderPtrace::ReadMem(Address inTraced, void *inSelf, unsigned amount)
{
    struct ptrace_io_desc io;

    io.piod_op = PIOD_READ_D;
    io.piod_addr = inSelf;
    io.piod_offs = (void *)inTraced;
    io.piod_len = amount;

    unsigned amountRead = 0;

    while( amountRead < amount ) {
        io.piod_len -= amountRead;
        io.piod_addr = ((unsigned char *)io.piod_addr) + amountRead;
        io.piod_offs = ((unsigned char *)io.piod_offs) + amountRead;

        if( 0 != ptrace(PT_IO, pid, (caddr_t)&io, 0) ) return false;
        amountRead += io.piod_len;
    }

    return true;
}

/* Complete the implementation of the AddressTranslateSysV class */

ProcessReader *AddressTranslateSysV::createDefaultDebugger(int pid)
{
  return new ProcessReaderPtrace(pid);
}


bool AddressTranslateSysV::setInterpreter() {
    assert(!"This function should not be called on FreeBSD");
    return false;
}

bool AddressTranslateSysV::setAddressSize() {
    if (address_size) return true;

    if( (address_size = sysctl_computeAddrWidth(pid)) == -1 ) {
        address_size = 0;
        return false;
    }

    return true;
}

const string& AddressTranslateSysV::getExecName() {
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
