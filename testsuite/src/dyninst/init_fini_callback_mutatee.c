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

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
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

extern int unique_id;

void exit_call(const char* msg)
{
    char filename[256];
    snprintf(filename, 256, "init_fini_log.%d", unique_id);
    if(called_entry != expected_entry)
    {
        const char* buf = "NO";
        int fd = creat(filename, 0x4 << 6);
        write(fd, buf, 2);
        fsync(fd);
        close(fd);
    }
    called_exit++;
    dprintf("exit_call %d/%d (%s)\n", called_exit, expected_exit, msg);
    
    if(called_exit == expected_exit)
    {
        const char* buf = "OK";
        int fd = creat(filename, 0x4 << 6);
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
