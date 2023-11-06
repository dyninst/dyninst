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

#include "freebsdKludges.h"
#include "common/src/headers.h"
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/proc.h>
#include <libutil.h>

#include <map>
using std::map;

#include "symbolDemangleWithCache.h"

std::string P_cplus_demangle( const std::string &symbol, bool includeTypes )
{
    return symbol_demangle_with_cache(symbol, includeTypes);
} /* end P_cplus_demangle() */

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

    assert( length > 0 && "process information not parsed correctly" );

    return procInfo;
}

map_entries *getVMMaps(int pid, unsigned &maps_size) {
    const int VM_PATH_MAX = 512;
    struct kinfo_vmentry *maps;
    map_entries *retMaps;

    maps = kinfo_getvmmap(pid, (int *)&maps_size);
    if( !maps ) {
        return NULL;
    }

    retMaps = (map_entries *)calloc(maps_size, sizeof(map_entries));
    if( !retMaps ) {
        free(maps);
        return NULL;
    }

    // Translate from one structure to another
    for(unsigned i = 0; i < maps_size; ++i) {
        retMaps[i].start = maps[i].kve_start;
        retMaps[i].end = maps[i].kve_end;
        retMaps[i].offset = maps[i].kve_offset;
        retMaps[i].dev_major = 0; // N/A
        retMaps[i].dev_minor = 0; // N/A
        retMaps[i].inode = maps[i].kve_vn_fileid;

        retMaps[i].prems = 0;
        if( KVME_PROT_READ & maps[i].kve_protection ) {
            retMaps[i].prems |= PREMS_READ;
        }

        if( KVME_PROT_WRITE & maps[i].kve_protection ) {
            retMaps[i].prems |= PREMS_WRITE;
        }

        if( KVME_PROT_EXEC & maps[i].kve_protection ) {
            retMaps[i].prems |= PREMS_EXEC;
        }

        strncpy(retMaps[i].path, maps[i].kve_path, VM_PATH_MAX-1);
        retMaps[i].path[VM_PATH_MAX-1] = '\0';
    }

    free(maps);
    return retMaps;
}

int sysctl_computeAddrWidth(pid_t pid) {
    int retSize = sizeof(void *);

#if defined(arch_64bit)
    const int X86_ADDR_WIDTH = 4;

    size_t length;
    struct kinfo_proc *procInfo = getProcInfo(pid, length, false);
    if( NULL == procInfo ) {
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
        return false;
    }

    int numEntries = length / procInfo->ki_structsize;
    for(int i = 0; i < numEntries; ++i) {
        lwps.push_back(procInfo[i].ki_tid);
    }
    free(procInfo);

    return true;
}

lwpid_t sysctl_getInitialLWP(pid_t pid) {
    size_t length;
    struct kinfo_proc *procInfo = getProcInfo(pid, length, true);
    if( NULL == procInfo ) {
      fprintf(stderr, "no proc info\n");
        return -1;
    }


    int numEntries = length / procInfo->ki_structsize;
    // The string won't be set until there are multiple LWPs
    if( numEntries == 1 ) {
        lwpid_t ret = procInfo->ki_tid;
        free(procInfo);
        return ret;
    }

    // By experimentation, we appear to want the last
    // entry.
    //
    // "BSD, what the hell" - bill, 24SEP2012, personal communication

    lwpid_t ret = procInfo[numEntries-1].ki_tid;
    free(procInfo);
    return ret;
#if 0

    for(int i = 0; i < numEntries; ++i) {
      fprintf(stderr, "%d: %s\n", i+1, procInfo[i].ki_ocomm);
        if( std::string(procInfo[i].ki_ocomm).find("initial") != std::string::npos ) {
            lwpid_t ret = procInfo[i].ki_tid;
            free(procInfo);
            return ret;
        }
    }

    free(procInfo);
    fprintf(stderr, "Failed to find initial thread\n");
    return -1;
#endif
}

// returns true if the process is running
bool sysctl_getRunningStates(pid_t pid, map<Dyninst::LWP, bool> &runningStates) {
    size_t length;
    struct kinfo_proc *procInfo = getProcInfo(pid, length, true);
    if( NULL == procInfo ) {
        return false;
    }

    int numEntries = length / procInfo->ki_structsize;

    for(int i = 0; i < numEntries; ++i) {
        runningStates.insert(std::make_pair(procInfo[i].ki_tid,
                (procInfo[i].ki_flag & P_STOPPED) == 0));
    }

    return true;
}

static bool PtraceBulkAccess(Dyninst::Address inTraced, unsigned size, void *inSelf, int pid, bool read) {
    struct ptrace_io_desc io;

    io.piod_op = (read ? PIOD_READ_D : PIOD_WRITE_D);
    io.piod_addr = inSelf;
    io.piod_offs = (void *)inTraced;
    io.piod_len = size;

    unsigned amountRead = 0;

    while( amountRead < size ) {
        io.piod_len -= amountRead;
        io.piod_addr = ((unsigned char *)io.piod_addr) + amountRead;
        io.piod_offs = ((unsigned char *)io.piod_offs) + amountRead;

        if( 0 != ptrace(PT_IO, pid, (caddr_t)&io, 0) ){
            kreturn false;
        }
        amountRead += io.piod_len;
    }

    return true;
}

bool PtraceBulkRead(Dyninst::Address inTraced, unsigned size, void *inSelf, int pid) {
    return PtraceBulkAccess(inTraced, size, inSelf, pid, true);
}

bool PtraceBulkWrite(Dyninst::Address inTraced, unsigned size, const void *inSelf, int pid) {
    return PtraceBulkAccess(inTraced, size, (void *)inSelf, pid, false);
}
