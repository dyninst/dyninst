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

#include <windows.h>
#include <stdlib.h>
#include "dyninstAPI_RT/src/RTheap.h"
#include "dyninstAPI_RT/src/RTcommon.h"

int	DYNINSTheap_align = 16;
Address DYNINSTheap_loAddr = 0x400000;    //4MB mark
Address DYNINSTheap_hiAddr = 0x7FFFFFFF;  //2GB mark

int getpagesize() {
    SYSTEM_INFO info;
    static int page_size = 0;
    if (page_size)
        return page_size;
    GetSystemInfo(&info);
    page_size = info.dwPageSize;
    return page_size;
}



void *map_region(void *addr, int len, int fd) {
    void *result;
	DWORD lastError;
	char* lpMessage = NULL;
    result = VirtualAlloc(addr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if(!result) {
		lastError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				lastError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)(lpMessage),
				0,
				NULL);
		fprintf(stderr, "VirtualAlloc failed in RTlib: %s\n", lpMessage);
		LocalFree((LPVOID)lpMessage);
	} else {
		fprintf(stderr, "VirtualAlloc succeeded, %p to %p mapped for RTlib heap\n",
			addr, (char*)(addr)+len);
	}
	return result;
}

int unmap_region(void *addr, int len) {
    BOOL result;
    result = VirtualFree(addr, 0, MEM_RELEASE);
    return (int) result;
}

int DYNINSTheap_mmapFdOpen(void)
{
   return 0;
}

void DYNINSTheap_mmapFdClose(int fd)
{
}

RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi)
{
  return RT_FALSE;
}

int DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp) {
    dyninstmm_t *map = NULL;
    void *temp;
    Address cur, base;
    MEMORY_BASIC_INFORMATION mem;
    unsigned count = 0, size = 0;
    static unsigned alloc_size = 256;

    map = (dyninstmm_t *) malloc(alloc_size * sizeof(dyninstmm_t));
    memset(map, 0, alloc_size * sizeof(dyninstmm_t));

    cur = DYNINSTheap_loAddr;
    for (; cur < DYNINSTheap_hiAddr; cur += size) {
        VirtualQuery((void *) cur, &mem, sizeof(MEMORY_BASIC_INFORMATION));
        base = (Address) mem.BaseAddress;
        size = mem.RegionSize;

        if (!size) goto done_err;
        if (mem.State & MEM_FREE) continue;
      
        if (count && (base <= map[count-1].pr_vaddr + map[count-1].pr_size)) {
            //We have two continuous regions, just merge them into one
            map[count-1].pr_size = base + size - map[count-1].pr_vaddr;
            continue;
        }

        if (count >= alloc_size) {
            //Grow the allocation buffer, if we need to
            alloc_size *= 2;
            temp = realloc(map, alloc_size * sizeof(dyninstmm_t));
            if (!temp) goto done_err;
            map = (dyninstmm_t *) temp;
        }

        map[count].pr_vaddr = base;
        map[count].pr_size = size;
        count++;
    }

    *nump = count;
    *mapp = map;
    return 0;

done_err:
    free(map);
    *nump = 0;
    *mapp = NULL;
    return -1;
}
