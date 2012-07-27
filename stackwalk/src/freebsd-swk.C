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
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/libstate.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <setjmp.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

int P_gettid() {
    static int gettid_not_valid = 0;
    int result;
    long lwp_id = -1;

    if( gettid_not_valid ) return getpid();

    result = syscall(SYS_thr_self, &lwp_id);
    if( result && errno == ENOSYS ) {
        gettid_not_valid = 1;
        return getpid();
    }

    return lwp_id;
}

bool LibraryState::updateLibsArch(std::vector<std::pair<LibAddrPair, unsigned int> > &) {
    return true;
}
