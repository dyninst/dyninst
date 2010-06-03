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
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>

char * P_cplus_demangle( const char * symbol, bool,
				bool includeTypes ) 
{
   int opts = 0;
   opts |= includeTypes ? DMGL_PARAMS | DMGL_ANSI : 0;

   char * demangled = cplus_demangle( const_cast< char *>(symbol), opts);
   if( demangled == NULL ) { return NULL; }

   if( ! includeTypes ) {
        /* de-demangling never increases the length */   
        char * dedemangled = strdup( demangled );   
        assert( dedemangled != NULL );
        dedemangle( demangled, dedemangled );
        assert( dedemangled != NULL );

        free( demangled );
        return dedemangled;
   }

   return demangled;
}

// Process Information Queries //
// No procfs mounted by default -- need to rely on sysctl //

/*
 * Gets the full path of the executable for the specified process
 *
 * pid  The pid for the process
 *
 * Returns the full path (caller is responsible for free'ing)
 */
char *sysctl_getExecPathname(pid_t pid) {
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = pid;

    size_t length;
    if( sysctl(mib, 4, NULL, &length, NULL, 0) ) {
        return NULL;
    }

    char *pathname = (char *)calloc(length, sizeof(char));

    if( !pathname ) return NULL;

    if( sysctl(mib, 4, pathname, &length, NULL, 0) ) {
        free(pathname);
        return NULL;
    }

    return pathname;
}

static struct kinfo_proc *getProcInfo(pid_t pid, size_t &length, bool getThreads) {
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID | ( getThreads ? KERN_PROC_INC_THREAD : 0 );
    mib[3] = pid;

    if( sysctl(mib, 4, NULL, &length, NULL, 0) ) {
        return NULL;
    }

    struct kinfo_proc *procInfo = (struct kinfo_proc *)malloc(length);
    if( !procInfo ) return NULL;

    if( sysctl(mib, 4, procInfo, &length, NULL, 0) ) {
        free(procInfo);
        return NULL;
    }

    return procInfo;
}

int sysctl_computeAddrWidth(pid_t pid) {
    int retSize = sizeof(void *);

#if defined(arch_64bit)
    const int X86_ADDR_WIDTH = 4;

    size_t length;
    struct kinfo_proc *procInfo = getProcInfo(pid, length, false);
    if( NULL == procInfo ) {
        fprintf(stderr, "Failed to determine address width of process %d\n", pid);
        return -1;
    }

    if( std::string(procInfo->ki_emul).find("ELF32") != std::string::npos ) {
        retSize = X86_ADDR_WIDTH;
    }
    free(procInfo);
#endif

    return retSize;
}

bool sysctl_findProcLWPs(pid_t pid, std::vector<pid_t> &lwps) {
    size_t length;
    struct kinfo_proc *procInfo = getProcInfo(pid, length, true);
    if( NULL == procInfo ) {
        fprintf(stderr, "Failed to determine LWP ids for process %d\n", pid);
        return false;
    }

    assert( length > 0 && "process information not parsed correctly" );

    int numEntries = length / procInfo->ki_structsize;
    for(int i = 0; i < numEntries; ++i) {
        lwps.push_back(procInfo[i].ki_tid);
    }
    free(procInfo);

    return true;
}
