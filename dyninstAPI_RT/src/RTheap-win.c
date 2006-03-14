/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

#include <windows.h>
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
    result = VirtualAlloc(addr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
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