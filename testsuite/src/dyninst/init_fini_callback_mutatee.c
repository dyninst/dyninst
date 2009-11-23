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
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "mutatee_util.h"
#include <stdlib.h>

#include <dlfcn.h>
#include <fcntl.h>

int called_entry = 0;
int called_exit = 0;
int expected_entry = 2;
int expected_exit = 2;

void entry_call(const char* msg)
{
    called_entry++;
    dprintf("entry_call %d/%d (%s)\n", called_entry, expected_entry, msg);
}

void exit_call(const char* msg)
{
    if(called_entry != expected_entry)
    {
        const char* buf = "NO";
        int fd = creat("init_fini_log", 0x4 << 6);
        write(fd, buf, 2);
        fsync(fd);
        close(fd);
    }
    called_exit++;
    dprintf("exit_call %d/%d (%s)\n", called_exit, expected_exit, msg);
    
    if(called_exit == expected_exit)
    {
        const char* buf = "OK";
        int fd = creat("init_fini_log", 0x4 << 6);
        write(fd, buf, 2);
        fsync(fd);
        close(fd);
    }
}
        
int init_fini_callback_mutatee()
{
    void* libhandle;
    libhandle = dlopen("libtestA.so", RTLD_LAZY);
    if(!libhandle)
        libhandle = dlopen("libtestA_m32.so", RTLD_LAZY);
    if(libhandle)
    dlclose(libhandle);
    test_passes("init_fini_callback");
    return 0;
}
