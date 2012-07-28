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
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTcommon.h"
#include "taskLib.h"

#include <unistd.h>

void DYNINSTos_init(int calledByFork, int calledByAttach)
{
    rtdebug_printf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
}

void DYNINSTsafeBreakPoint()
{
    taskSuspend( taskIdSelf() );
}

int DYNINST_am_initial_thread( dyntid_t tid )
{
    return taskIdSelf();
}

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev)
{
    return taskIdSelf();
}

int dyn_pid_self()
{
    return taskIdSelf();
}

int dyn_lwp_self()
{
    return taskIdSelf();
}

dyntid_t dyn_pthread_self()
{
    return (void *)0x0;
}

int getpagesize()
{
    return 1024; // No VM support for vxWorks (yet).
}

void DYNINSTbreakPoint()
{  
    /* We set a global flag here so that we can tell
      if we're ever in a call to this when we get a
      SIGBUS */
    if (DYNINSTstaticMode)
        return;

    DYNINST_break_point_event = 1;
    while (DYNINST_break_point_event) {
        kill(dyn_lwp_self(), 7);
    }
    /* Mutator resets to 0... */
}

unsigned int DYNINSTtaskListMax = 0;
unsigned int DYNINSTtaskListCount = 0;
int *DYNINSTtaskList = NULL;
int *DYNINSTrefreshTasks(void)
{
    int newMax;
    int *newList;

    /* Grow the task list until it's not completely full. */
    while (1) {
        memset(DYNINSTtaskList, 0, sizeof(int) * DYNINSTtaskListMax);
        DYNINSTtaskListCount = taskIdListGet(DYNINSTtaskList,
                                             DYNINSTtaskListMax);

        if (DYNINSTtaskListCount >= DYNINSTtaskListMax) {
            // Grow the task list.
            newMax  = (!DYNINSTtaskListMax ? 16
                                           : DYNINSTtaskListMax * 2);
            newList = realloc(DYNINSTtaskList, sizeof(int) * newMax);
            if (newList) {
                DYNINSTtaskListMax = newMax;
                DYNINSTtaskList    = newList;
                continue;
            }
            fprintf(stderr, "Warning: tasklist realloc failure (%d bytes).\n",
                    sizeof(int) * newMax);
        }
        break;
    }
/*
    fprintf(stderr, "Tasks I've found:\n");
    for (newMax = 0; newMax < DYNINSTtaskListCount; ++newMax) {
        fprintf(stderr, "\t0x%x (%d)\n",
                DYNINSTtaskList[newMax], DYNINSTtaskList[newMax]);
    }
*/
    return DYNINSTtaskList;
}

unsigned int DYNINSTtaskNameSize = 0;
char *DYNINSTtaskNameBuf = NULL;
char *DYNINSTtaskInfo(int taskid)
{
    int newSize = 0, statCount = 0;
    char *newBuf = NULL;

    int   firstStat = 1;
    char  statStr[64];  // Status string will never be larger than this.
    int   delayTicks;
    char *nameStr;

    nameStr = taskName(taskid);
    strcpy(statStr, " (");
    if (taskIsReady(taskid)) {
        strcat(statStr, "READY");
        firstStat = 0;
    }
    if (taskIsSuspended(taskid)) {
        if (!firstStat) strcat(statStr, "|");
        strcat(statStr, "SUSPENDED");
        firstStat = 0;
    }        
    if (taskIsStopped(taskid)) {
        if (!firstStat) strcat(statStr, "|");
        strcat(statStr, "STOPPED");
        firstStat = 0;
    }
    if (taskIsPended(taskid)) {
        if (!firstStat) strcat(statStr, "|");
        strcat(statStr, "PENDED");
        firstStat = 0;
    }
    if (taskIsDelayed(taskid, &delayTicks)) {
        if (!firstStat) strcat(statStr, "|");
        sprintf(statStr + strlen(statStr), "DELAYED:%d", delayTicks);
    }
    strcat(statStr, ")");

    newSize = strlen(nameStr) + strlen(statStr) + 1;
    if (newSize > DYNINSTtaskNameSize) {
        newBuf = (char *)realloc(DYNINSTtaskNameBuf, newSize);
        if (newBuf) {
            DYNINSTtaskNameSize = newSize;
            DYNINSTtaskNameBuf  = newBuf;
        } else {
            fprintf(stderr, "Warning: nameBuf realloc failure (%d bytes).\n",
                    newSize);
        }
    }

    DYNINSTtaskNameBuf[0] = '\0';
    strncpy(DYNINSTtaskNameBuf, nameStr, DYNINSTtaskNameSize);
    strncat(DYNINSTtaskNameBuf, statStr, DYNINSTtaskNameSize);

    return DYNINSTtaskNameBuf;
}

#include "dyninstAPI_RT/src/RTheap.h"
int DYNINSTheap_align = 4;
RT_Boolean DYNINSTheap_useMalloc(void *hi, void *lo)
{
    return RT_TRUE;
}


int DYNINSTgetMemoryMap(unsigned *a, dyninstmm_t **b)
{
    return -1; // I suppose this could be implemented from memInfo
}

int DYNINSTheap_mmapFdOpen(void)
{
    return -1;
}

void DYNINSTheap_mmapFdClose(int fd)
{
    return;
}
