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

// $Id: vxworks.C,v 1.279 2008/09/03 06:08:44 jaw Exp $

#include <fstream>
#include <string>

#include "dyninstAPI/src/vxworks.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

#include <sys/ptrace.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <dlfcn.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h>
#include <poll.h>
#include <string>
#include <elf.h>

// #include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/eventgate.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "common/src/headers.h"
#include "common/src/wtxKludges.h"
#include "dyninstAPI/src/os.h"
#include "common/src/stats.h"
#include "common/src/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/src/pathName.h"
#include "mapped_object.h"
#include "mapped_module.h"

#include "ast.h" // instrumentation for MT

#include "dynamiclinking.h"
#include "symtabAPI/h/Symtab.h"

using namespace Dyninst::SymtabAPI;

// Global declarations
// XXX Find a place for these eventually
WTX_TGT_ID_T currCtx = 0x0;
//image *currImage;
process *currProc;

eventLock wtxLock;
std::vector<EventRecord> wtxEvents;

dyn_hash_map<std::string, WTX_CONTEXT_ID_T> wtxTasks;
dyn_hash_map<Address, WTX_TGT_ID_T> wtxBreakpoints;
extern dyn_hash_map<Address, relocationEntry> wtxReloc;

//TODO: Remove the writeBack functions and get rid of this include
#ifdef PAPI
#include "papi.h"
#endif

void printStackWalk( process * /*p*/ ) { assert(0); return; }
// void InstrucIter::readWriteRegisters(int* /*readRegs*/, int* /*writeRegs*/) { assert(0); }

/* **********************************************************************
 * VxWorks local functions
 * **********************************************************************/
bool getRTlibName(std::string &name) {
    // Get env variable.
    if (getenv("DYNINSTAPI_RT_LIB") == NULL) {
        fprintf(stderr, "Environment variable DYNINSTAPI_RT_LIB has not been defined.\n");
        return false;
    }
    name = getenv("DYNINSTAPI_RT_LIB");

    // Check to see if the library given exists.
    if (access(name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + name
            + std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }
    return true;
}

void fakeWtxEvent(process *proc,
                  eventType type,
                  eventWhat_t what,
                  eventStatusCode_t status = statusNormal,
                  eventInfo_t info = 0x0,
                  eventMoreInfo_t info2 = 0x0,
                  eventAddress_t address = 0x0,
                  eventFileDesc_t fd = 0x0)
{
    EventRecord ev;

    ev.proc = proc;
    ev.lwp  = proc->getRepresentativeLWP();
    ev.type = type;
    ev.what = what;
    ev.status = status;
    ev.info = info;
    ev.info2 = info2;
    ev.address = address;
    ev.fd = fd;

    //proc->getSG()->_Lock(FILE__, __LINE__);
    wtxLock._Lock(FILE__, __LINE__);

    wtxEvents.push_back(ev);

    wtxLock._Broadcast(FILE__, __LINE__);
    wtxLock._Unlock(FILE__, __LINE__);
    //proc->getSG()->_Broadcast(FILE__, __LINE__);
    //proc->getSG()->_Unlock(FILE__, __LINE__);
}

eventType decodeWtxEvent(const char *eventStr)
{
    if (!eventStr)
        return evtUndefined;

    else if (strcmp("", eventStr) == 0)
        return evtUndefined;

    return evtUndefined;
}

WTX_EVENT_DESC copyWtxDesc(WTX_EVENT_DESC *desc)
{
    WTX_EVENT_DESC result;
    unsigned int len = strlen(desc->event) + 1;

    result.event = (char *)malloc(len + desc->addlDataLen);
    result.addlDataLen = desc->addlDataLen;
    result.addlData = result.event + len;

    strcpy(result.event, desc->event);
    memcpy(result.addlData, desc->addlData, desc->addlDataLen);

    return result;
}

void freeWtxDesc(WTX_EVENT_DESC *desc)
{
    free(desc->event);
    desc->event = NULL;
}

int strprefix(const char *s1, const char *s2) { return strncmp(s1, s2, strlen(s2)) == 0; }
void wtxEventHandler(WTX_EVENT_DESC *desc, HWTX wtxHandle)
{
    WTX_CONTEXT ctx;
    ctx.contextType = WTX_CONTEXT_TASK;
    ctx.contextId = currCtx;
    ctx.contextSubId = 0;
    STATUS result;

    assert(wtxHandle == wtxh);

/* -
    fprintf(stderr, "WTX Event Received '%s' + %d\n", desc->event, desc->addlDataLen);
    for (unsigned int i = 0; i < desc->addlDataLen; ++i) {
        fprintf(stderr, "0x%x ", desc->addlData[i]);
        if (i % 15 == 0) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n---END\n");
*/
    EventRecord ev;
    if (!desc->event) {
        fprintf(stderr, "Unknown event!\n");
        return;

    } else if (strprefix(desc->event, "CTX_EXIT")) {
        ev.type = evtProcessExit;
        ev.status = statusNormal;
        ev.what = 0;

    } else if (strprefix(desc->event, "TEXT_ACCESS")) {
        // Suspend the child task.
        result = wtxContextSuspend(wtxh, &ctx);
        if (result != WTX_OK) {
            fprintf(stderr, "%s(%d): wtxContextSuspend() error: %s\n", FILE__, __LINE__, wtxErrMsgGet(wtxh));
            return;
        }

        // Remove the stop state of the child task,
        // leaving it in a suspended state.
        result = wtxContextCont(wtxh, &ctx);
        if (result != WTX_OK) {
            fprintf(stderr, "%s(%d): wtxContextCont() error: %s\n", FILE__, __LINE__, wtxErrMsgGet(wtxh));
            return;
        }

        // Clear the breakpoint from our record.
        int pc_idx = strlen(desc->event);
        for (int count = 0; pc_idx > 0 && count < 3; --pc_idx)
            if (desc->event[pc_idx - 1] == ' ') ++count;

        Address addr = strtol(&desc->event[pc_idx], NULL, 0);
        if (wtxBreakpoints.count(addr) == 0) {
            fprintf(stderr, "*** ERROR: I don't know about event at address 0x%lx...\n", addr);
        } else {
            if (wtxEventpointDelete(wtxh, wtxBreakpoints[addr]) != WTX_OK)
                fprintf(stderr, "%s(%d): wtxEventPointDelete() error: %s\n", FILE__, __LINE__, wtxErrMsgGet(wtxh));
            wtxBreakpoints.erase(addr);
        }

        ev.what = 5; // Simulated SIGTRAP;
    }

    // Broadcast the event to Dyninst.
    wtxLock._Lock(FILE__, __LINE__);
    wtxEvents.push_back(ev);
    wtxLock._Broadcast(FILE__, __LINE__);
    wtxLock._Unlock(FILE__, __LINE__);
}

WTX_TGT_ARG_T triggerlist[] = { WTX_EVENT_OBJ_LOADED,
                                WTX_EVENT_OBJ_UNLOADED,
                                WTX_EVENT_CTX_STOP,

                                WTX_EVENT_INVALID };

void enableEventPoints(WTX_CONTEXT_ID_T ctxID)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = ctxID;
    ctx.contextSubId = 0x0;

    WTX_TGT_ID_T result = wtxContextExitNotifyAdd(wtxh, &ctx);
    if (result == static_cast<WTX_TGT_ID_T>( WTX_ERROR )) {
        fprintf(stderr, "Error on wtxContextExitNotifyAdd(): %s\n", wtxErrMsgGet(wtxh));
    }
    //printEventpoints();

/*
    WTX_EVTPT point;
    memset(&point, 0, sizeof(WTX_EVTPT));

    point.context.contextType = WTX_CONTEXT_TASK;
    point.context.contextId   = ctxID;

    if (wtxRegisterForEvent(wtxh, ".*") != WTX_OK) {
        fprintf(stderr, "Error on wtxRegisterForEvent(.*): %s\n", wtxErrMsgGet(wtxh));
    }

    point.event.eventType     = WTX_EVENT_TRIGGER;
    point.event.numArgs       = 1;

    point.action.actionType   = (WTX_ACTION_TYPE)(WTX_ACTION_NOTIFY |
                                                  WTX_ACTION_STOP);

    WTX_TGT_ARG_T end = WTX_EVENT_INVALID;
    for (unsigned int i = 0; triggerlist[i] != end; ++i) {
        point.event.args = &triggerlist[i];

        if (wtxEventpointAdd(wtxh, &point) == static_cast<WTX_TGT_ID_T>(WTX_ERROR)) {
            fprintf(stderr, "Error on wtxEventpointAdd(%d): %s\n", static_cast<int>(triggerlist[i]), wtxErrMsgGet(wtxh));
        }
    }
*/
}
/*
void fillRelocationMap(Symtab *obj)
{
    std::vector<Region *> reg;
    if (!obj || !obj->getAllRegions(reg)) return;

    for (unsigned i = 0; i < reg.size(); ++i) {
        std::vector<relocationEntry> &reloc = reg[i]->getRelocations();
        for(unsigned j = 0; j < reloc.size(); ++j) {
//            fprintf(stderr, "Adding address 0x%lx\n", reloc[j].rel_addr());
            wtxReloc[reloc[j].rel_addr()] = reloc[j].name();
        }
    }
}
*/
void freeObjects()
{
    STATUS result;

    while (!wtxDynLoadedMods.empty()) {
        WTX_MODULE_INFO *info = *(wtxDynLoadedMods.rbegin());
        result = wtxObjModuleUnload(wtxh,
                                    0x0, 0x0, /* pdID and unload options */
                                    info->moduleId);
        if (result != WTX_OK) {
            fprintf(stderr, "Error on wtxObjModuleUnload(): %s\n",
                    wtxErrMsgGet(wtxh));
            //wtxTerminate(wtxh);
            return;
        }

        std::string filename = info->moduleName;
        assert(wtxMods.count(filename));
        wtxMods.erase(filename);

        wtxResultFree(wtxh, info);
        wtxDynLoadedMods.pop_back();
    }
}

/* **********************************************************************
 * VxWorks exported functions
 * **********************************************************************/
bool OS_isConnected(void)
{
    return (wtx_state == WTX_CONNECTED);
}

bool OS_connect(BPatch_remoteHost &remote)
{
    assert(remote.type == BPATCH_REMOTE_DEBUG_WTX);

    STATUS result;
    const char *errstr;

    if (wtx_state < WTX_INITIALIZED) {
        result = wtxInitialize(&wtxh);
        if (result != WTX_OK) {
            errstr = wtxErrMsgGet(wtxh);
            fprintf(stderr, "Error on wtxInitialize(): %s\n", errstr);
            return false;
        }
        wtx_state = WTX_INITIALIZED;
    }

    BPatch_remoteWtxInfo *info =
        static_cast<BPatch_remoteWtxInfo *>( remote.info );
    result = wtxToolOnHostAttach(wtxh,
                                 const_cast<char *>(info->target),
                                 const_cast<char *>(info->tool),
                                 const_cast<char *>(info->host));
    if (result != WTX_OK) {
        errstr = wtxErrMsgGet(wtxh);
        fprintf(stderr, "Error on wtxToolOnHostAttach(): %s\n", errstr);
        return false;
    }

    wtx_state = WTX_CONNECTED;

    result = wtxAsyncNotifyEnable(wtxh, wtxEventHandler);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxAsyncNotifyEnable(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    result = wtxRegisterForEvent(wtxh, ".*");
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegisterForEvent(.*): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    return true;
}

bool OS_getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &tlist)
{
    assert(remote.type == BPATCH_REMOTE_DEBUG_WTX);

    if (wtx_state < WTX_CONNECTED) {
        fprintf(stderr, "Error: Attempted to get pid list while disconnected.\n");
        return false;
    }

    std::string rtlib_name;
    if (!getRTlibName(rtlib_name)) {
        fprintf(stderr, "Could not determine runtime library pathname.\n");
        return false;
    }

    if (!wtxLoadObject(rtlib_name)) {
        fprintf(stderr, "Could not load runtime library on target.\n");
        return false;
    }

    // Get address of DYNINSTrefreshTasks().
    WTX_TGT_ADDR_T funcaddr;
    if (!wtxFindFunction("DYNINSTrefreshTasks", 0x0, funcaddr)) {
        fprintf(stderr, "Could not find function named DYNINSTrefreshTasks\n");
        return false;
    }

    // Call the function.
    STATUS result;
    WTX_TGT_ADDR_T listaddr;
    result = wtxDirectCall(wtxh, funcaddr, &listaddr, 0);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxDirectCall(): %s\n",  wtxErrMsgGet(wtxh));
        return false;
    }

    // Find and read the count variable.
    unsigned int taskcount;
    WTX_TGT_ADDR_T countaddr;
    if (!wtxFindVariable("DYNINSTtaskListCount", 0x0, countaddr)) {
        fprintf(stderr, "Cound not find variable DYNINSTtaskListCount\n");
        return false;
    }
    if (!wtxReadMem((void *)countaddr, sizeof(unsigned int), &taskcount)) {
        fprintf(stderr, "Could not read DYNINSTtaskListCount at 0x%lx.\n",
                countaddr);
        return false;
    }
    taskcount = swapBytesIfNeeded(taskcount);

    // Then find and read the pid list.
    static unsigned int tasklist_max = 0;
    static int *tasklist = NULL;

    if (tasklist_max < taskcount) {
        int *newlist = (int *)realloc(tasklist, taskcount * sizeof(int));
        if (!newlist) {
            fprintf(stderr, "Could not allocate %d bytes for tasklist\n",
                    taskcount * sizeof(int));
            return false;
        }
        tasklist = newlist;
        tasklist_max = taskcount;
    }

    if (!wtxReadMem((void *)listaddr, taskcount * sizeof(int), tasklist)) {
        fprintf(stderr, "Could not read DYNINSTtaskList at 0x%lx.\n",
                listaddr);
        return false;
    }

    tlist.clear();
    for (unsigned int i = 0; i < taskcount; ++i) {
        tlist.push_back(swapBytesIfNeeded(tasklist[i]));
    }

    return true;
}

bool OS_getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr)
{
    assert(remote.type == BPATCH_REMOTE_DEBUG_WTX);

    if (wtx_state < WTX_CONNECTED) {
        fprintf(stderr, "Error: Attempted to get pid list while disconnected.\n");
        return false;
    }

    std::string rtlib_name;
    if (!getRTlibName(rtlib_name)) {
        fprintf(stderr, "Could not determine runtime library pathname.\n");
        return false;
    }

    if (!wtxLoadObject(rtlib_name)) {
        fprintf(stderr, "Could not load runtime library on target.\n");
        return false;
    }

    // Get address of DYNINSTtaskInfo().
    WTX_TGT_ADDR_T funcaddr;
    if (!wtxFindFunction("DYNINSTtaskInfo", 0x0, funcaddr)) {
        fprintf(stderr, "Could not find function named DYNINSTtaskInfo\n");
        return false;
    }

    // Call the function.
    void *bufAddr;
    STATUS result;
    result = wtxDirectCall(wtxh, funcaddr, &bufAddr, 1, pid);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxDirectCall(): %s\n",  wtxErrMsgGet(wtxh));
        return false;
    }

    // Find and read the name buffer size.
    unsigned int bufSize;
    WTX_TGT_ADDR_T sizeAddr;
    if (!wtxFindVariable("DYNINSTtaskNameSize", 0x0, sizeAddr)) {
        fprintf(stderr, "Cound not find variable DYNINSTtaskNameSize\n");
        return false;
    }
    if (!wtxReadMem((void *)sizeAddr, sizeof(unsigned int), &bufSize)) {
        fprintf(stderr, "Could not read DYNINSTtaskNameSize at 0x%lx.\n",
                sizeAddr);
        return false;
    }
    bufSize = swapBytesIfNeeded(bufSize);

    // Read the string buffer.
    char tmpBuf[1024];
    assert(sizeof(tmpBuf) > bufSize);
    if (!wtxReadMem(bufAddr, bufSize, tmpBuf)) {
        fprintf(stderr, "Could not read %d bytes of DYNINSTtaskNameBuf at 0x%lx.\n",
                bufSize, (unsigned long)bufAddr);
        return false;
    }

    // Then find and read the pid list.
    pidStr = std::string(tmpBuf);
    return true;
}

bool OS_disconnect(BPatch_remoteHost &remote)
{
    assert(remote.type == BPATCH_REMOTE_DEBUG_WTX);
    return wtxDisconnect();
}

bool wtxCreateTask(const std::string &filename, WTX_MODULE_INFO *modinfo)
{
    WTX_CONTEXT_DESC desc;
    memset(&desc, 0, sizeof(WTX_CONTEXT_DESC));
    desc.wtxContextType = WTX_CONTEXT_TASK;
    desc.wtxContextDef.wtxTaskContextDef.priority = 100;

    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextSubId = 0;

    // First, check for default component level init/term and entry routines.
    WTX_TGT_ADDR_T initAddr  = 0x0;
    WTX_TGT_ADDR_T entryAddr = 0x0;

    if (wtxMods.count(filename)) {
        initAddr  = wtxMods[filename]->vxCompRtn;
        entryAddr = wtxMods[filename]->userInitRtn;
    }

    if (initAddr) {
        desc.wtxContextDef.wtxTaskContextDef.name = "modInit";
        desc.wtxContextDef.wtxTaskContextDef.entry = initAddr;

        ctx.contextId = wtxContextCreate(wtxh, &desc);
        if (ctx.contextId == static_cast<WTX_CONTEXT_ID_T>( WTX_ERROR )) {
            fprintf(stderr, "Error calling init routine for %s: %s\n",
                    filename.c_str(), wtxErrMsgGet(wtxh));

        } else {
            if (wtxContextResume(wtxh, &ctx) != WTX_OK)
                fprintf(stderr, "Error starting init task for %s: %s\n",
                        filename.c_str(), wtxErrMsgGet(wtxh));

            // Should we wait until this routine completes?
        }
    }

    // Drop back to symbol "main" if needed.
    if (!entryAddr) {
        wtxFindFunction("main", modinfo->moduleId, entryAddr);
        //const pdvector <func_instance *> *funcs;
        //funcs = obj->findFuncVectorByMangled("main");
        //if (funcs && funcs->size())
        //    entryAddr = (*funcs)[0]->getAddress();
    }

    if (entryAddr) {
//        fprintf(stderr, "Launching task %s with start address 0x%x\n", filename.c_str(), entryAddr);

        desc.wtxContextDef.wtxTaskContextDef.name = const_cast<char *>(filename.c_str());
        desc.wtxContextDef.wtxTaskContextDef.entry = entryAddr;

        ctx.contextId = wtxContextCreate(wtxh, &desc);
        if (ctx.contextId == static_cast<WTX_CONTEXT_ID_T>( WTX_ERROR )) {
            fprintf(stderr, "Error creating task for %s: %s\n",
                    filename.c_str(), wtxErrMsgGet(wtxh));
            return false;
        }

        currCtx = ctx.contextId;
        enableEventPoints(ctx.contextId);

        //currImage = obj->parse_img();
        //currProc = obj->proc();

/*      - Have to find a way to stop the entire system.

        memset(&desc, 0, sizeof(WTX_CONTEXT_DESC));
        desc.wtxContextType = WTX_CONTEXT_SYSTEM;
        ctxID = wtxContextCreate(wtxh, &desc);
        if (ctxID == WTX_ERROR)
            fprintf(stderr, "Error creating system-wide context: %s\n",
                    wtxErrMsgGet(wtxh));
        else {
            sysCtx = ctxID;
        }
*/

        //fakeWtxEvent(static_cast<process *>(obj->proc()), evtProcessInit, 5);
    }
    // wtxTasks[filename] = ctx;
    return true;
}

bool fixup_offsets(const std::string &filename, Dyninst::SymtabAPI::Symtab *linkedFile)
{
    if (!wtxMods.count(filename))
        return false;

    WTX_MODULE_INFO *info = wtxMods[filename];
    unsigned int i;

    // Fixup loadable sections
    for (i = 0; i < info->nSections; ++i) {
        if (linkedFile->fixup_RegionAddr(info->section[i].name,
                                         info->section[i].baseAddr,
                                         info->section[i].length)) {
            // Record the relocations
            Region *reg = NULL;
            assert(linkedFile->findRegion(reg, info->section[i].baseAddr,
                                               info->section[i].length));
            std::vector<relocationEntry> &reloc = reg->getRelocations();
            for (unsigned j = 0; j < reloc.size(); ++j)
                wtxReloc[ reloc[j].rel_addr() ] = reloc[j];

            // Region exists in file.  It's been updated and our job is done.
            continue;
        }

        // Skip zero length sections
        if (info->section[i].length == 0x0)
            continue;

        // Otherwise, we must add the section manually.
        Region::RegionType rtype;
        switch (info->section[i].type) {
        case WTX_SECTION_TEXT:    rtype = Region::RT_TEXT; break;
        case WTX_SECTION_RODATA:
        case WTX_SECTION_DATA:
        case WTX_SECTION_BSS:     rtype = Region::RT_DATA; break;
        case WTX_SECTION_UNKNOWN:
        default:
            fprintf(stderr, "Skipping section %s of unknown type.\n",
                    info->section[i].name);
            continue;
        }

        // Allocate host memory to hold data from target memory.
        char *data = (char *)malloc(info->section[i].length);
        if (!data) {
            fprintf(stderr, "Malloc error allocating %d bytes.\n",
                    info->section[i].length);
            continue;
        }

        // Read target memory into host buffer.
        if (!wtxReadMem(reinterpret_cast<void *>(info->section[i].baseAddr),
                        info->section[i].length, data)) {
            fprintf(stderr, "Could not read '%s' section data.  Skipping.\n",
                    info->section[i].name);
            continue;
        }

        if (!linkedFile->addRegion(info->section[i].baseAddr,
                                   data,
                                   info->section[i].length,
                                   info->section[i].name,
                                   rtype,       /* type      */
                                   true,        /* loadable  */
                                   sizeof(int)  /* memalign  */,
                                   false        /* tls       */))
            fprintf(stderr, "*** Failed to add section %s.\n",
                    info->section[i].name);
    }

    // Fixup regions
    WTX_TGT_ADDR_T
        code_off = (WTX_TGT_ADDR_T) -1,
        code_len = (WTX_TGT_ADDR_T) -1,
        data_off = (WTX_TGT_ADDR_T) -1,
        data_len = (WTX_TGT_ADDR_T) -1,
        bss_off  = (WTX_TGT_ADDR_T) -1,
        bss_len  = (WTX_TGT_ADDR_T) -1,
        max_addr = (WTX_TGT_ADDR_T) 0;

    // Don't need offset locations anymore.  Still need this to find the len.
    for (i = 0; i < info->nSegments; ++i) {
        if (info->segment[i].type == WTX_SEGMENT_TEXT) {
            //linkedFile->fixup_RegionAddr("CODE_SEGMENT",
            //                             info->segment[i].addr,
            //                             info->segment[i].length);
            code_off = info->segment[i].addr;
            code_len = info->segment[i].length;
            if (max_addr < code_off + info->segment[i].length)
                max_addr = code_off + info->segment[i].length;

        } else if (info->segment[i].type == WTX_SEGMENT_DATA) {
            //linkedFile->fixup_RegionAddr("DATA_SEGMENT",
            //                             info->segment[i].addr,
            //                             info->segment[i].length);
            data_off = info->segment[i].addr;
            data_len = info->segment[i].length;
            if (max_addr < data_off + info->segment[i].length)
                max_addr = data_off + info->segment[i].length;

        } else if (info->segment[i].type == WTX_SEGMENT_BSS) {
            //linkedFile->fixup_RegionAddr("BSS_SEGMENT",
            //                             info->segment[i].addr,
            //                             info->segment[i].length);
            bss_off = info->segment[i].addr;
            bss_len = info->segment[i].length;
            if (max_addr < bss_off + info->segment[i].length)
                max_addr = bss_off + info->segment[i].length;

        } else {
            fprintf(stderr, "Unknown segment type 0x%x\n",
                    info->segment[i].type);
            assert(0);
        }
    }

    // Fixup code and data pointers
    //assert(code_off != (WTX_TGT_ADDR_T) -1);
    assert(code_len != (WTX_TGT_ADDR_T) -1);
    //assert(data_off != (WTX_TGT_ADDR_T) -1);
    assert(data_len != (WTX_TGT_ADDR_T) -1);
    linkedFile->fixup_code_and_data(/*0x0*/ code_off, code_len,
                                    /*0x0*/ data_off, data_len);

    // Fixup symbols
    WTX_SYM_FIND_CRITERIA crit;
    memset(&crit, 0, sizeof(WTX_SYM_FIND_CRITERIA));
    crit.options    = (WTX_SYM_FIND_BY_NAME |
                       WTX_SYM_FILTER_ON_MODULE_ID);
                       //WTX_SYM_FILTER_ON_TYPE);
    crit.pdId       = 0x0;
    crit.findName   = ".*";
    crit.moduleId   = info->moduleId;
    //crit.type       = WTX_SYMBOL_COMM;

    WTX_SYM_LIST *list = wtxSymListGet(wtxh, &crit);
    if (list) {
        // Make a pass through the symbols to gather default size values.
        // This doesn't account for non-continuous functions, but an initial
        // size value will be required for individual function parsing.
        WTX_SYMBOL *curr = list->pSymbol;
        std::set<unsigned long> addr_set;
        std::set<unsigned long>::iterator iter;
        while (curr) {
            addr_set.insert(curr->value);
            curr = curr->next;
        }
        // Make sure length can be calculated for all symbols.
        addr_set.insert(max_addr);
        
        // Second pass through the symbols.
        curr = list->pSymbol;
        for (curr = list->pSymbol; curr; curr = curr->next) {
            // Find the symbol with the next higher value.
            iter = addr_set.find(curr->value);
            assert(iter != addr_set.end());
            ++iter;

            if (linkedFile->fixup_SymbolAddr(curr->name, curr->value)) {

                if ((curr->type & WTX_SYMBOL_COMM) == WTX_SYMBOL_COMM) {
                    /* DEBUG
                    fprintf(stderr,
                            "Creating common variable addr:0x%08lx size:%d"
                            " next_addr:0x%08lx type:0x%08x %s\n",
                            curr->value, curr->size,
                            (iter == addr_set.end() ? 0 : *iter),
                            curr->type, curr->name); // */
                }

                // Symbol exists and has been updated.  Our job is done.
                continue;
            }

            // Skip absolute symbols (and local symbols for now).
            if ((curr->type & WTX_SYMBOL_ABS) == WTX_SYMBOL_ABS ||
                (curr->type & WTX_SYMBOL_LOCAL) == WTX_SYMBOL_LOCAL) {
                continue;
            }

            if ((curr->type & WTX_SYMBOL_TEXT) == WTX_SYMBOL_TEXT) {

                // This is a horrible hack:
                // If we don't have an object file, we cannot distinguish
                // between code objects in the text section (functions), 
                // and data objects in the text section (type info/vtables).
                //
                // We could use the demangler to figure some of this out.
                // Using hard-coded string prefixes for now.
                if (strncmp(curr->name, "_ZTI", 4) == 0) continue; // Typeinfo
                if (strncmp(curr->name, "_ZTS", 4) == 0) continue; // Typename
                if (strncmp(curr->name, "_ZTV", 4) == 0) continue; // vtable

                // These are symbols in the text section known to hold data.
                if (strcmp(curr->name, "hcfDeviceList") == 0) continue;
                if (strcmp(curr->name, "ipcom_ipd_products") == 0) continue;
                if (strcmp(curr->name, "ipnet_conf_sysvar_ext") == 0) continue;
                if (strcmp(curr->name, "ipnet_conf_link_layer") == 0) continue;
                if (strcmp(curr->name, "__clz_tab") == 0) continue;
                if (strcmp(curr->name, "__popcount_tab") == 0) continue;
                if (strcmp(curr->name, "__thenan_sf") == 0) continue;
                if (strcmp(curr->name, "__thenan_df") == 0) continue;
                if (strcmp(curr->name, "netVersionString") == 0) continue;
                if (strcmp(curr->name, "ipcom_priority_map") == 0) continue;
                if (strcmp(curr->name, "MD5_version") == 0) continue;

                linkedFile->createFunction(curr->name, curr->value,
                                           *iter - curr->value);

            } else if ((curr->type & WTX_SYMBOL_DATA) == WTX_SYMBOL_DATA ||
                       (curr->type & WTX_SYMBOL_BSS)  == WTX_SYMBOL_BSS) {
                linkedFile->createVariable(curr->name, curr->value,
                                           *iter - curr->value);

            } else if ((curr->type & WTX_SYMBOL_COMM) == WTX_SYMBOL_COMM) {
                //fprintf(stderr, "Creating common variable addr:0x%08lx size:%d est_size:%10ld type:0x%08x %s\n",
                //        curr->value, curr->size, *iter - curr->value, curr->type, curr->name);
                linkedFile->createVariable(curr->name, curr->value, 4);

            } else {
                fprintf(stderr, "Unhandled wtx type: 0x%x\n", curr->type);
            }
        }
        wtxResultFree(wtxh, list);
    }

    //fillRelocationMap(linkedFile);
    return true;
}

void addBreakpoint(Address bp)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    WTX_TGT_ID_T point = wtxBreakpointAdd(wtxh, &ctx, (WTX_TGT_ADDR_T)bp);
    if (point == static_cast<WTX_TGT_ID_T>( WTX_ERROR )) {
        fprintf(stderr, "Error on wtxBreakpointAdd(): %s\n", wtxErrMsgGet(wtxh));
        return;
    }

    //fprintf(stderr, "Successfully added a breakpoint at 0x%lx\n", bp);
    wtxBreakpoints[bp] = point;
}

bool doNotParseList(const std::vector< std::string > &names)
{
    static dyn_hash_set<std::string> list;
    if (list.empty()) {
        list.insert("<<oogabooga");
//        list.insert("_mBlkClFree");
//        list.insert("pciConfigOutByte");
    }

    for (unsigned int i = 0; i < names.size(); ++i)
        if (list.find(names[i]) != list.end())
            return true;

    return false;
}

void disasAddress(const Address /*addr*/)
{
/*
    WTX_DASM_INST_LIST *insn;
    fprintf(stderr, "0x%lx: ---\n", addr);
    insn = wtxMemDisassemble(wtxh, 0x0, addr, 3, 0x0, TRUE, TRUE, TRUE);

    if (insn) {
        char *strs[4], *curr = insn->pInst;
        while (curr && *curr) {
            for (int i = 0; i < 4; ++i) {
                curr = strchr(curr, '{');
                if (!curr) break;
                *curr++ = '[';
                strs[i] = curr;

                curr = strchr(curr, '}');
                if (!curr) break;
                *curr++ = '\0';
            }
            if (!curr) break;

            fprintf(stderr, "\t%20s %08x %08x %s\n",
                    strs[0], strs[1], strs[2], strs[3]);
        }
        wtxResultFree(wtxh, insn);
    }
    fprintf(stderr, "---\n");
*/
}

/* **********************************************************************
 * Class OS Implementations
 * **********************************************************************/

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) { assert(0); }
void OS::osTraceMe(void) { assert(0); }
bool OS::executableExists(const std::string &file)
{
    struct stat file_stat;
    int stat_result;

    const char *fn = file.c_str();
    stat_result = stat(fn, &file_stat);
    return (stat_result != -1);
}
void OS::make_tempfile(char * /*s*/) { assert(0); }
bool OS::execute_file(char * /*path*/) { assert(0); }
void OS::unlink(char * /*file*/) { assert(0); }

/* **********************************************************************
 * Class EventRecord Implementations
 * **********************************************************************/
void EventRecord::clear() {
    proc = NULL;
    lwp = NULL;
    type = evtUndefined;
    what = 0;
    status = statusUnknown;
    info = 0;
    address = 0;
    fd = 0;
}

/* **********************************************************************
 * Class SignalGenerator Implementations
 * **********************************************************************/
SignalGenerator::SignalGenerator(char *idstr, std::string file, int pid)
    : SignalGeneratorCommon(idstr) ,
      waiting_for_stop(false)
{
    setupAttached(file, pid);
}
SignalGenerator::~SignalGenerator()
{

}
bool SignalGenerator::decodeSyscall(EventRecord & /*ev*/) {assert(0);}
bool SignalGenerator::forkNewProcess()
{
/*
    result = wtxInitialize(&wtxh);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxInitialize(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    result = wtxToolAttach(wtxh, "ppc@bixi.cs.umd.edu", "DyninstTool");
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxToolAttach(): %s\n", wtxErrMsgGet(wtxh));
        wtxTerminate(wtxh);
        return false;
    }
*/
    setAgentMode(WTX_AGENT_MODE_TASK);

/*
    result = wtxRegisterForEvent(wtxh, "CTX.*");
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegisterForEvent(CTX): %s\n", wtxErrMsgGet(wtxh));
        wtxTerminate(wtxh);
        return false;
    }

    result = wtxRegisterForEvent(wtxh, "OBJ.*");
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegisterForEvent(OBJ): %s\n", wtxErrMsgGet(wtxh));
        wtxTerminate(wtxh);
        return false;
    }

    result = wtxRegisterForEvent(wtxh, "CALL_RETURN");
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegisterForEvent(CALL_RETURN): %s\n", wtxErrMsgGet(wtxh));
        wtxTerminate(wtxh);
        return false;
    }
*/
    string fullpath = file_;
    if (file_.find(dir_) == string::npos)
        fullpath = dir_ + file_;

    WTX_MODULE_INFO *info = wtxLoadObject(fullpath);
    if (!info) {
        fprintf(stderr, "Could not load %s on target\n", fullpath.c_str());
        return false;
    }

    if (!wtxCreateTask(fullpath, info)) {
        fprintf(stderr, "Could not create a task for %s\n", fullpath.c_str());
        return false;
    }

    pid_ = currCtx;
    fakeWtxEvent(proc, evtProcessCreate, 5);
    fakeWtxEvent(proc, evtProcessInit,   5);

    return true;
}

bool SignalGenerator::decodeEvents(pdvector<EventRecord> &events)
{
    for (unsigned int i = 0; i < events.size(); i++) {
        // If we already know the details of this event, skip it.
        if (events[i].type != evtUndefined)
            continue;

        if (proc->getRpcMgr()->decodeEventIfDueToIRPC(events[i])) {
            signal_printf("%s[%d]:  SIGTRAP due to RPC\n", FILE__, __LINE__);
        }
    }
    return true;
}


bool SignalGenerator::waitForEventsInternal(pdvector<EventRecord> &events)
{
    std::vector<EventRecord> local_buf;
    assert(events.size() == 0);

    signal_printf("%s[%d][%s]:  waitNextEvent\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()));

    //assert(getExecThreadID() == getThreadID());
    //assert(getExecThreadID() != primary_thread_id);
    assert(proc);
    //assert(proc->getRepresentativeLWP());

    waitingForOS_ = false;
    __UNLOCK;

    wtxLock._Lock(FILE__, __LINE__);

    if (wtxEvents.size() == 0)
        wtxLock._WaitForSignal(FILE__, __LINE__);

    for (unsigned int i = 0; i < wtxEvents.size(); ++i) {
        if (!wtxEvents[i].proc) {
            wtxEvents[i].proc = proc;
            wtxEvents[i].lwp  = proc->getRepresentativeLWP();
        }
        local_buf.push_back(wtxEvents[i]);
    }
    wtxEvents.clear();

    wtxLock._Unlock(FILE__, __LINE__);

    __LOCK;
    waitingForOS_ = false;

    for (unsigned int i = 0; i < local_buf.size(); ++i) {
        if (local_buf[i].proc->status() == running)
            local_buf[i].proc->set_status(stopped);
        events.push_back(local_buf[i]);
    }

    return true;
}

bool SignalGenerator::waitForStopInline() { assert(0); return false; }
bool SignalGeneratorCommon::getExecFileDescriptor(std::string filename,
                                                  int /*pid*/,
                                                  bool /*whocares*/,
                                                  int &,
                                                  fileDescriptor &desc)
{
    if (!wtxMods.count(filename)) {
        desc = fileDescriptor(filename.c_str(), 0x0, 0x0, false);
        return true;
    }
    WTX_MODULE_INFO *info = wtxMods[filename];
    assert(info);

    bool found_text = false, found_data = false;
    WTX_TGT_ADDR_T text_addr = 0, data_addr = 0;
    for (unsigned i = 0; i < info->nSegments; ++i) {
        if (info->segment[i].type == WTX_SEGMENT_TEXT) {
            found_text = true;
            text_addr = info->segment[i].addr;
        }
        if (info->segment[i].type == WTX_SEGMENT_DATA) {
            found_data = true;
            data_addr = info->segment[i].addr;
        }
    }
    assert(found_text && found_data);

    desc = fileDescriptor(filename.c_str(),
                          0x0 /*text_addr*/,
                          0x0 /*data_addr*/,
                          false);
    return true;
}

bool SignalGeneratorCommon::postSignalHandler()
{
    return true;
}
bool SignalGeneratorCommon::decodeRTSignal_NP(EventRecord & /*ev*/, Address /*rt_arg*/, int /*status*/) { assert(0); }

/* **********************************************************************
 * Class SignalHandler Implementations
 * **********************************************************************/
bool SignalHandler::handleProcessCreate(EventRecord & /*ev*/, bool & /*continueHint*/)
{
    return true;
}

bool SignalHandler::handleProcessExitPlat(EventRecord & /*ev*/,
                                          bool & /*continueHint */)
{
    freeObjects();
    currCtx = 0x0;
/*
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;
    STATUS result;
    result = wtxContextKill(wtxh, &ctx, 0x0);
    if (result != WTX_OK)
        fprintf(stderr, "Error on wtxContextKill(): %s\n",
                wtxErrMsgGet(wtxh));
*/
    
    //wtxTerminate(wtxh);
    return true;
}

bool SignalHandler::handleCodeOverwrite(EventRecord &)
{
    assert(0);//not implemented for unix 
    return false;
}

bool SignalHandler::handleExecEntry(EventRecord & /*ev*/, bool & /*continueHint*/) { assert(0); }
bool SignalHandler::handleProcessAttach(EventRecord & /*ev*/, bool & /*continueHint*/) { assert(0); }
bool SignalHandler::handleThreadCreate(EventRecord &, bool &) { assert(0); }
bool SignalHandler::handleSignalHandlerCallback(EventRecord & /*ev*/) { assert(0); }
bool SignalHandler::forwardSigToProcess(EventRecord & /*ev*/, bool & /*continueHint*/) { assert(0); }

/* **********************************************************************
 * Class process Implementations
 * **********************************************************************/
dyn_lwp *process::createRepresentativeLWP()
{
    // the initial lwp has a lwp_id with the value of the pid

    // if we identify this linux process as multi-threaded, then later we will
    // adjust this lwp to be identified as a real lwp.
    dyn_lwp *initialLWP = createFictionalLWP(getPid());
    representativeLWP = initialLWP;
    return initialLWP;
}
bool process::trapAtEntryPointOfMain(dyn_lwp * /*trappingLWP*/, Address) { assert(0); return false; }
bool process::trapDueToDyninstLib(dyn_lwp * /*trappingLWP*/) { assert(0); return false; }
bool process::setProcessFlags()
{
    // Use this to modify any necessary state of the debugging link.
    return true;
}

bool process::unsetProcessFlags()
{
    // No vxWorks or wtx flags to unset.
    return true;
}

bool process::isRunning_() const
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = getPid();
    ctx.contextSubId = 0;

    WTX_CONTEXT_STATUS result = wtxContextStatusGet(wtxh, &ctx);
    return (result & WTX_CONTEXT_RUNNING);
}
bool process::waitUntilStopped() { assert(0); return false; }

terminateProcStatus_t process::terminateProc_()
{
    WTX_CONTEXT ctx;

    if (currCtx == 0x0) return alreadyTerminated;

    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxContextKill(wtxh, &ctx, 0x0);
    if (result != WTX_OK) {
        fprintf(stderr, "wtxContextKill() error: %s\n", wtxErrMsgGet(wtxh));
        return terminateFailed;
    }

    currCtx = 0x0;
    return terminateSucceeded;
}

bool process::dumpCore_(const std::string/* coreFile*/) { assert(0); return false; }
std::string process::tryToFindExecutable(const std::string& /*progpath*/, int /*pid*/)
{
    // There's no easy way to associate a vxWorks task with an object file.
    return "[No file for attach]";
}
bool process::determineLWPs(pdvector<unsigned> & /*lwp_ids*/) { assert(0); return false; }
bool process::dumpImage( std::string ) { assert(0); return false; }
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address /*near*/, Address &lo, Address &hi, inferiorHeapType /* type */ )
{
    lo = 0;
    hi = (Address)-1;
    return;
}

/**
 * Strategy:  The program entry point is in /lib/ld-2.x.x at the 
 * _start function.  Get the current PC, parse /lib/ld-2.x.x, and 
 * compare the two points.
 **/
bool process::hasPassedMain()
{
    // Main holds no significance for vxWorks kernel tasks.
    return true;
}
bool process::initMT() { assert(0); return false; }
bool process::handleTrapAtEntryPointOfMain(dyn_lwp * /*trappingLWP*/)
{
    // We don't need to do anything for the RTlib, right?
    return true;
}

bool process::insertTrapAtEntryPointOfMain()
{
    // We don't need any extra processing of the RTlib.
    return true;
}

bool process::handleTrapAtLibcStartMain(dyn_lwp *)  { assert(0); }
bool process::instrumentLibcStartMain() { assert(0); }
bool process::decodeStartupSysCalls(EventRecord &) { assert(0); }
void process::setTraceSysCalls(bool) { assert(0); }
void process::setTraceState(traceState_t) { assert(0); }
bool process::getSysCallParameters(dyn_saved_regs *, long *, int) { assert(0); }
int process::getSysCallNumber(dyn_saved_regs *) { assert(0); }
long process::getSysCallReturnValue(dyn_saved_regs *) { assert(0); }
Address process::getSysCallProgramCounter(dyn_saved_regs *) { assert(0); }
bool process::isMmapSysCall(int) { assert(0); }
Offset process::getMmapLength(int, dyn_saved_regs *) { assert(0); }
Address process::getLibcStartMainParam(dyn_lwp *) { assert(0); }

bool process::loadDYNINSTlib()
{
    WTX_MODULE_INFO *info = wtxLoadObject(dyninstRT_name);
    if (!info) {
        fprintf(stderr, "Could not load runtime library on target.\n");
        return false;
    }

    bool found_text = false, found_data = false;
    WTX_TGT_ADDR_T text_addr = 0, data_addr = 0;
    for (unsigned i = 0; i < info->nSegments; ++i) {
        if (info->segment[i].type == WTX_SEGMENT_TEXT) {
            found_text = true;
            text_addr = info->segment[i].addr;
        }
        if (info->segment[i].type == WTX_SEGMENT_DATA) {
            found_data = true;
            data_addr = info->segment[i].addr;
        }
    }
    assert(found_text && found_data);

    fileDescriptor *fd = new fileDescriptor(dyninstRT_name,
                                            0x0 /*text_addr*/,
                                            0x0 /*data_addr*/,
                                            true);
    mapped_object *obj = mapped_object::createMappedObject(*fd, this);
    addASharedObject(obj);

    setBootstrapState(loadedRT_bs);
    return true;
}

extern int dyn_debug_rtlib;
bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
{
    // ---------------------------------------------------------------------
    // First, re-run DYNINSTinit() from runtime library.

    // Get address of DYNINSTinit().
    WTX_TGT_ADDR_T funcaddr;
    if (!wtxFindFunction("DYNINSTinit", 0x0, funcaddr)) {
        fprintf(stderr, "Could not find function named DYNINSTinit\n");
        return false;
    }

    // Call the function.
    // Parameters mirrored from setDyninstLibInitParams(), except we always
    // want to look like the process was created. (not attached or forked).
    unsigned int resultBuf;
    STATUS result;
    result = wtxDirectCall(wtxh, funcaddr, &resultBuf, 4,
                           1,                       /* created_cm   */
                           P_getpid(),              /* mutator pid  */
                           maxNumberOfThreads(),    /* max threads  */
                           dyn_debug_rtlib);        /* rtdebug flag */
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxDirectCall(): %s\n",  wtxErrMsgGet(wtxh));
        return false;
    }

    // ---------------------------------------------------------------------
    // Now the DYNINST_bootstrap_info is guarenteed to be initialized.

    const std::string vrbleName = "DYNINST_bootstrap_info";

    pdvector<int_variable *> bootstrapInfoVec;
    if (!findVarsByAll(vrbleName, bootstrapInfoVec))
        assert(0);
    assert(bootstrapInfoVec.size() == 1);

    Address symAddr = bootstrapInfoVec[0]->getAddress();

    // bulk read of bootstrap structure
    if (!readDataSpace((const void*)symAddr, sizeof(*bs_record), bs_record, true)) {
        cerr << "extractBootstrapStruct failed because readDataSpace failed" << endl;
        return false;
    }

    unsigned long *curr = (unsigned long *)bs_record;
    unsigned long *end  = curr + (sizeof(DYNINST_bootstrapStruct) /
                                  sizeof(unsigned long));
    while (curr < end) {
        *curr = swapBytesIfNeeded(*curr);
        ++curr;
    }

    return true;
}

bool process::loadDYNINSTlibCleanup(dyn_lwp * /*trappingLWP*/) { assert(0); return false; }
bool process::startDebugger() { assert(0); }
bool process::hideDebugger()
{
    return false;
}

mapped_object *process::createObjectNoFile(Address)
{
    assert(0); // Not implemented for unix or vxWorks
    return NULL;
}

// VxWorks Kernel Modules don't use relocation entries so, until we enable
// the binary rewriter on this platform, relocation entries are always bound.
bool process::hasBeenBound(const SymtabAPI::relocationEntry &,
                           func_instance *&,
                           Address )
{
    return true;
}

// In process.C:
// bool process::stop_(bool waitUntilStop) { assert(0); return false; }
// bool process::continueProc_(int sig) { assert(0); return false; }
// Address process::setAOutLoadAddress(fileDescriptor &desc) { assert(0); return 0; }
// bool process::detachForDebugger(const EventRecord &/*crash_event*/) { assert(0); return false; }

#if defined(cap_binary_rewriter)
mapped_object *BinaryEdit::openResolvedLibraryName(std::string, 
                                                   std::map<std::string, BinaryEdit *> &) { assert(0); }
#endif

void emitCallRel32(unsigned /*disp32*/, unsigned char *& /*insn*/) { assert(0); return; }

//static int lwp_kill(int /*pid*/, int /*sig*/) { assert(0); return 0; }
/**
 * Return the state of the process from /proc/pid/stat.
 * File format is:
 *   pid (executablename) state ...
 * where state is a character.  Returns '\0' on error.
 **/
//static char getState(int /*pid*/) { assert(0); return 0; }
//static std::string getNextLine(int /*fd*/) { assert(0); return ""; }

bool isPLT(func_instance * /*f*/) { assert(0); return false; }

/* **********************************************************************
 * Class dyn_lwp Implementations
 * **********************************************************************/
bool dyn_lwp::continueLWP_(int /*signalToContinueWith*/)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxContextResume(wtxh, &ctx);
    if (result != WTX_OK) {
        fprintf(stderr, "wtxContextResume() error: %s\n", wtxErrMsgGet(wtxh));
        return false;
    }
    return true;
}

// XXX Replace this with a target-level kernel call.
bool dyn_lwp::waitUntilStopped()
{
    unsigned char buf;
    while (1) {
        void *addr = reinterpret_cast<void *>(currCtx + 0x73);
        if (!wtxReadMem(addr, 1, &buf)) {
            fprintf(stderr, "Could not read task state.\n");
            return false;
        }
#if 0
        unsigned int result;
        result = wtxMemRead(wtxh,
                            0x0,   // Protection domain
                            (WTX_TGT_ADDR_T)(currCtx + 0x73),
                            &buf,
                            1,     // Read 1 byte
                            WTX_MEM_CACHE_BYPASS);
        if (result == WTX_ERROR) {
            fprintf(stderr, "%s(%d): wtxMemRead() error: %s\n", FILE__, __LINE__, wtxErrMsgGet(wtxh));
            return false;
        }
#endif
        // Here's the magic.
        if (buf & 0x1 == 1) break;

        sleep(1);
    }
    return true;

#if 0
    // Apparently, wtxContextStatusGet() doesn't work on tasks.
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    WTX_CONTEXT_STATUS status;
    do {
        status = wtxContextStatusGet(wtxh, &ctx);
        if (status == WTX_ERROR) {
            fprintf(stderr, "wtxContextStatusGet() error: %s\n", wtxErrMsgGet(wtxh));
            return false;
        }
    } while (status == WTX_CONTEXT_RUNNING);

    return true;
#endif
}

bool dyn_lwp::stop_()
{
    return wtxSuspendTask(currCtx);
}

void dyn_lwp::realLWP_detach_() { assert(0); return; }
void dyn_lwp::representativeLWP_detach_()
{
    return;
}
bool dyn_lwp::writeTextWord(caddr_t /*inTraced*/, int /*data*/) { assert(0); return false; }
bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{   // No real distinction between text and data.
    return wtxWriteMem(inTraced, amount, inSelf);
}

bool dyn_lwp::readTextSpace(const void *inTraced, u_int amount, void *inSelf)
{   // No real distrinction between text and data.
    return wtxReadMem(inTraced, amount, inSelf);
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
    return wtxWriteMem(inTraced, nbytes, inSelf);
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf)
{
    return wtxReadMem(inTraced, nbytes, inSelf);
}

bool dyn_lwp::realLWP_attach_() { assert(0); return false; }
//static bool is_control_stopped(int /*lwp*/) { assert(0); return false; }
bool dyn_lwp::representativeLWP_attach_()
{
    bool running = false;

//    XXX Why does this assert fail?  Why doesn't it think we're created via fork?
//    assert(proc_->wasCreatedViaFork() && "*** Implememnt Me ***");

    if (proc_->wasCreatedViaAttach()) {
        currCtx = proc_->getPid();
        stop_();
        proc_->set_status(stopped);
        enableEventPoints(currCtx);
    }

    startup_printf("%s[%d]: in representative lwp attach, isRunning %d\n",
                   FILE__, __LINE__, running);

#if 0
    // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
    // to paradynd in all cases?  How about when we are attaching to an
    // already-running process?  (Seems that in the latter case, no SIGTRAP
    // is automatically generated)
   
    // Only if we are really attaching rather than spawning the inferior
    // process ourselves do we need to call PTRACE_ATTACH
    if (proc_->wasCreatedViaAttach() || 
        proc_->wasCreatedViaFork() ||
        proc_->wasCreatedViaAttachToCreated()) {

        int ptrace_errno = 0;
        int address_width = sizeof(Address);
        assert(address_width);
        startup_printf("%s[%d]: process attach doing PT_ATTACH to %d\n",
                       FILE__, __LINE__, get_lwp_id());
        if( 0 != DBI_ptrace(PTRACE_ATTACH, getPid(), 0, 0, &ptrace_errno, 
                            address_width, __FILE__, __LINE__) ) {
            startup_printf("%s[%d]:  ptrace attach to pid %d failing: %s\n", 
                           FILE__, __LINE__, getPid(), strerror(ptrace_errno));
            return false;
        }
        startup_printf("%s[%d]: attached via DBI\n", FILE__, __LINE__);
        //proc_->sh->add_lwp_to_poll_list(this);

        proc_->set_status(stopped);
    }
#endif
    return true;
}

bool dyn_lwp::changePC(Address loc, struct dyn_saved_regs */*ignored registers*/)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    loc = swapBytesIfNeeded(loc);

    STATUS result;
    result = wtxRegsSet(wtxh, //WTX API handle
                        &ctx, // WTX Context
                        WTX_REG_SET_IU, // type of register set
                        offsetof(struct dyn_saved_regs, iu[WTX_REG_IU_PC]), // first byte of register set
                        sizeof(unsigned int), // number of bytes of register set
                        &loc); // place holder for reg. values

    if (result != WTX_OK) {
        fprintf(stderr, "wtxRegsSet() error: %s\n", wtxErrMsgGet(wtxh));
    }

#if 0
    struct dyn_saved_regs r;
    r.sprs[3] = loc;

    while (1) {
        memset(&r, 0, sizeof(struct dyn_saved_regs));
        ctx.contextType = WTX_CONTEXT_TASK;
        result = wtxRegsGet(wtxh, &ctx, WTX_REG_SET_IU, 0x0, sizeof(dyn_saved_regs), &r);
        if (result != WTX_OK) { fprintf(stderr, "wtxRegsGet() error: %s\n", wtxErrMsgGet(wtxh)); assert(0); }
        wtxErrClear(wtxh);

        fprintf(stderr, "VERIFY! 0x%x\n", r.sprs[3]);
        ctx.contextType = WTX_CONTEXT_TASK;
        result = wtxContextStep(wtxh, &ctx, 0, 0);
        if (result != WTX_OK) { fprintf(stderr, "wtxContextStep() error: %s\n", wtxErrMsgGet(wtxh)); assert(0); }
        wtxErrClear(wtxh);
    }
#endif
    return (result == WTX_OK);
}

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs, bool /*includeFP*/)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxRegsGet(wtxh, //WTX API handle
                               &ctx, // WTX Context
                               WTX_REG_SET_IU, // type of register set
                               0x0, // first byte of register set
                               sizeof(regs->iu), // number of bytes of register set
                               regs->iu); // place holder for reg. values
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegsGet(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    unsigned long *curr = (unsigned long *)regs;
    unsigned long *end  = curr + (sizeof(dyn_saved_regs) /
                                  sizeof(unsigned long));
    while (curr < end) {
        *curr = swapBytesIfNeeded(*curr);
        ++curr;
    }

    return true;
}

Address dyn_lwp::readRegister(Register reg)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    Address retval;
    STATUS result = wtxRegsGet(wtxh, //WTX API handle
                               &ctx, // WTX Context
                               WTX_REG_SET_IU, // type of register set
                               reg * proc_->getAddressWidth(), // first byte of register set
                               proc_->getAddressWidth(), // number of bytes of register set
                               &retval); // place holder for reg. values
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegsGet(): %s\n", wtxErrMsgGet(wtxh));
        return (Address)-1;
    }

    return swapBytesIfNeeded(retval);
}

bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs, bool /*includeFP*/)
{
    struct dyn_saved_regs newRegs;
    const unsigned int *hostp = regs.iu;
    unsigned int *targp = newRegs.iu;
    unsigned int *end  = targp + (sizeof(dyn_saved_regs) /
                                  sizeof(unsigned int));
    while (targp < end) {
        *targp = swapBytesIfNeeded(*hostp);
        ++targp;
        ++hostp;
    }

    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxRegsSet(wtxh, //WTX API handle
                               &ctx, // WTX Context
                               WTX_REG_SET_IU, // type of register set
                               0x0, // first byte of register set
                               sizeof(newRegs.iu), // number of bytes of register set
                               &newRegs.iu); // place holder for reg. values
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegsGet(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    return true;
}

int dyn_lwp::changeMemoryProtections(Address , Offset , unsigned )
{
    assert(0); // Not implemented for unix or vxWorks.
    return 0;
}

func_instance *dyn_thread::map_initial_func(func_instance * /*ifunc*/) { assert(0); }

void loadNativeDemangler() { return; }

/* **********************************************************************
 * Class DebuggerInterface Implementations
 * **********************************************************************/
bool DebuggerInterface::bulkPtraceWrite(void * /*inTraced*/, u_int /*nbytes*/, void * /*inSelf*/, int /*pid*/, int /*address_width*/) { assert(0); return false; }
bool DebuggerInterface::bulkPtraceRead(void * /*inTraced*/, u_int /*nelem*/, void * /*inSelf*/, int /*pid*/, int /*address_width*/) { assert(0); return false; }

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's func_instance.  
// If the function has not yet been bound, then "target" is set to the 
// func_instance associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
func_instance *instPoint::findCallee()
{
    assert(0);
#if 0
    if (callee_) {
        return callee_;
    }

    if (ipType_ != callSite) {
        return NULL;
    }

    if (isDynamic()) {
        return NULL;
    }

    // Check if we parsed an intra-module static call
    assert(img_p_);
    parse_func *icallee = img_p_->getCallee();
    if (icallee) {
        // Now we have to look up our specialized version
        // Can't do module lookup because of DEFAULT_MODULE...
        const pdvector<func_instance *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName().c_str());
        if (!possibles) {
            return NULL;
        }
        for (unsigned i = 0; i < possibles->size(); i++) {
            if ((*possibles)[i]->ifunc() == icallee) {
                callee_ = (*possibles)[i];
                return callee_;
            }
        }
        // No match... very odd

        assert(0);
        return NULL;
    }
    
    // Other possibilities: call through a function pointer,
    // or a inter-module call. We handle inter-module calls as
    // a static function call, since they're bound at load time.

    // We figure out the linkage by running a quick instructiter over the target...
    // Code is similar to archCheckEntry in image-power.C

    // TODO: encapsulate this in the instPoint so we can handle absolute branches
    // acceptably.

    if (!proc()->isValidAddress(callTarget())) {
        return NULL;
    }
   
    InstrucIter targetIter(callTarget(), proc());
    if (!targetIter.getInstruction().valid()) {
        return NULL;
    }
    Address toc_offset = 0;
    
    if (targetIter.isInterModuleCallSnippet(toc_offset)) {
        Address TOC_addr = (func()->obj()->parse_img()->getObject())->getTOCoffset();

        // We need to read out of memory rather than disk... so this is a call to
        // readDataSpace. Yummy.

        Address linkageAddr = 0;
        Address linkageTarget = 0;
        // Basically, load r12, <x>(r2)
        if (!proc()->readDataSpace((void *)(TOC_addr + toc_offset),
                                   sizeof(Address),
                                   (void *)&linkageAddr, false))
            return NULL;
        // And load r0, 0(r12)
        if (!proc()->readDataSpace((void *)linkageAddr,
                                   sizeof(Address),
                                   (void *)&linkageTarget, false))
            return NULL;

        if (linkageTarget == 0) {
            // No error for this one... looks like unloaded libs
            // do it.
            return NULL;
        }
        // Again, by definition, the function is not in owner.
        // So look it up.
        func_instance *pdf = proc()->findFuncByAddr(linkageTarget);

        if (pdf) {
            callee_ = pdf;
            return callee_;
        }
        else
            return NULL;
    }
    return NULL;
#endif
}

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
//static void findThreadFuncs(process * /*p*/, std::string /*func*/, pdvector<func_instance *> & /*result*/) { assert(0); return; }
void dyninst_yield() { assert(0); return; }

// ****** Support linux-specific forkNewProcess DBI callbacks ***** //

/* **********************************************************************
 * Class DBI Callback Implementations
 * **********************************************************************/
bool DebuggerInterface::forkNewProcess(std::string /*file*/, 
                                       std::string /*dir*/,
                                       pdvector<std::string> * /*argv*/,
                                       pdvector<std::string> * /*envp*/,
                                       std::string /*inputFile*/, std::string /*outputFile*/, int & /*traceLink*/,
                                       pid_t & /*pid*/, int /*stdin_fd*/, int /*stdout_fd*/, int /*stderr_fd*/,
                                       SignalGenerator * /*sg*/) { assert(0); return false; }

//static int P_gettid() { assert(0); return 0; }
void chld_handler(int) { assert(0); return; }
void chld_handler2(int, siginfo_t *, void *) { assert(0); return; }
//static void kickWaitpider(int /*pid*/) { assert(0); return; }

/* **********************************************************************
 * Class WaitpidMux Implementations
 * **********************************************************************/
//Force the SignalGenerator to return -1, EINTR from waitpid
bool WaitpidMux::registerProcess(SignalGenerator * /*me*/) { assert(0); return false; }
bool WaitpidMux::registerLWP(unsigned /*lwpid*/, SignalGenerator * /*me*/) { assert(0); return false; }
bool WaitpidMux::unregisterLWP(unsigned /*lwpid*/, SignalGenerator * /*me*/) { assert(0); return false; }
bool WaitpidMux::unregisterProcess(SignalGenerator * /*me*/) { assert(0); return false; }
void WaitpidMux::forceWaitpidReturn() { assert(0); return; }
bool WaitpidMux::suppressWaitpidActivity() { assert(0); return false; }
bool WaitpidMux::resumeWaitpidActivity() { assert(0); return false; }
int WaitpidMux::waitpid(SignalGenerator * /*me*/, int * /*status*/) { assert(0); return 0; }
bool WaitpidMux::hasFirstTimer(SignalGenerator * /*me*/) { assert(0); return false; }
void WaitpidMux::addPidGen(int /*pid*/, SignalGenerator * /*sg*/) { assert(0); return; }
void WaitpidMux::removePidGen(int /*pid*/, SignalGenerator * /*sg*/) { assert(0); return; }
void WaitpidMux::removePidGen(SignalGenerator * /*sg*/) { assert(0); return; }
int WaitpidMux::enqueueWaitpidValue(waitpid_ret_pair /*ev*/, SignalGenerator * /*event_owner*/) { assert(0); return 0; }


#include <string>
#include <dlfcn.h>

#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/registerSpace.h"

#define P_offsetof(s, m) (Address) &(((s *) NULL)->m)

#if defined(arch_64bit)
#define PT_REGS_OFFSET(m, mutatee_address_width)           \
        (                                                  \
            ((mutatee_address_width) == sizeof(uint64_t))  \
            ? (   /* 64-bit mutatee */                     \
                  P_offsetof(struct pt_regs, m)            \
              )                                            \
            : (   /* 32-bit mutatee */                     \
                  P_offsetof(struct pt_regs32, m)          \
              )                                            \
        )
#else
#define PT_REGS_OFFSET(m, mutatee_address_width)           \
        (                                                  \
            P_offsetof(struct pt_regs, m)                  \
        )
#endif


#define PT_FPSCR_OFFSET(mutatee_address_width)                  \
        (                                                       \
            ((mutatee_address_width) == sizeof(PTRACE_RETURN))  \
            ? (   /* N-bit mutatee, N-bit mutator   */          \
                  PT_FPSCR * sizeof(PTRACE_RETURN)              \
              )                                                 \
            : (   /* 32-bit mutatee, 64-bit mutator */          \
                  (PT_FPR0 + 2*32 + 1) * sizeof(uint32_t)       \
              )                                                 \
        )


#define SIZEOF_PTRACE_DATA(mutatee_address_width)  (mutatee_address_width)

bool Frame::setPC(Address /*newpc*/) { assert(0); return false; }

bool AddressSpace::getDyninstRTLibName()
{
    bool retval = getRTlibName(dyninstRT_name);
    startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    return retval;
}

// floor of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Address region_lo(const Address /*x*/) { return 0; }

// floor of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Address region_lo_64(const Address /*x*/) { assert(0); return 0; }

// ceiling of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Address region_hi(const Address /*x*/) { return (Address)-1; }

// ceiling of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Address region_hi_64(const Address /*x*/) { assert(0); return 0; }

//
// All costs are based on Measurements on a SPARC station 10/40.
// They haven't actually been updated for VxWorks yet.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    primitiveCosts["DYNINSTprintCost"] = 1;

    //
    // I can't find DYNINSTincrementCounter or DYNINSTdecrementCounter
    // I think they are not being used anywhere - naim
    //
    // isthmus acutal numbers from 7/3/94 -- jkh
    // 240 ns
    primitiveCosts["DYNINSTincrementCounter"] = 16;
    // 240 ns
    primitiveCosts["DYNINSTdecrementCounter"] = 16;

    // Values (in cycles) benchmarked on a Pentium III 700MHz
    // Level 2 - Software Level
    //primitiveCosts["DYNINSTstartWallTimer"] = 719;
    //primitiveCosts["DYNINSTstopWallTimer"] = 737;
    primitiveCosts["DYNINSTstartProcessTimer"] = 587;
    primitiveCosts["DYNINSTstopProcessTimer"] = 607;

    /* Level 1 - Hardware Level
    // Implementation still needs to be added to handle start/stop
    // timer costs for multiple levels
    */
    primitiveCosts["DYNINSTstartWallTimer"] = 145;
    primitiveCosts["DYNINSTstopWallTimer"] = 163;
   
    //primitiveCosts["DYNINSTstartProcessTimer"] = 195;
    //primitiveCosts["DYNINSTstopProcessTimer"] = 207;

    // These happen async of the rest of the system.
    // 133.86 usecs * 67Mhz
    primitiveCosts["DYNINSTalarmExpire"] = 8968;
    primitiveCosts["DYNINSTsampleValues"] = 29;
    // 6.41 usecs * 67Mhz
    primitiveCosts["DYNINSTreportTimer"] = 429;
    // 89.85 usecs * 67Mhz
    primitiveCosts["DYNINSTreportCounter"] = 6019;
    primitiveCosts["DYNINSTreportCost"] = 167;
    primitiveCosts["DYNINSTreportNewTags"] = 40; 
}

/* **********************************************************************
 * Dynamic linking implementation
 * A lot of this may not make sense for VxWorks.
 * **********************************************************************/

sharedLibHook::sharedLibHook(process * /*p*/, sharedLibHookType /*t*/, Address /*b*/) { assert(0); }
sharedLibHook::~sharedLibHook() { assert(0); }

// processLinkMaps: This routine is called by getSharedObjects to  
// process all shared objects that have been mapped into the process's
// address space.  This routine reads the link maps from the application 
// process to find the shared object file base mappings. It returns 0 on error.
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &descs)
{
    WTX_MOD_FIND_CRITERIA crit;
    memset(&crit, 0, sizeof(WTX_MOD_FIND_CRITERIA));
    crit.options    = WTX_MOD_FIND_ALL;
    crit.pdId       = WTX_MOD_FIND_IN_ALL_PD;

    WTX_MODULE_LIST *list = wtxObjModuleListGet(wtxh, &crit);
    if (list) {
        WTX_MODULE *curr = list->pModule;
        WTX_MODULE_INFO *info;
        while (curr) {
            std::string filename = curr->moduleName;
            if (wtxMods.count(filename)) {
                info = wtxMods[filename];

            } else {
                info = wtxObjModuleInfoGet(wtxh, 0x0, curr->moduleId);
                if (!info) {
                    fprintf(stderr, "Error on wtxObjModuleInfoGet(0x%lx): %s\n",
                            curr->moduleId, wtxErrMsgGet(wtxh));
                    assert(0);
                }
//                if (strcmp(curr->moduleName, "/var/ftp/pub/vxWorks") == 0) {
                if (strstr(curr->moduleName, "/vxWorks")) {
                    curr->moduleName[0] = '['; // Force incremental parsing.
                    filename = curr->moduleName;
                }
                wtxMods[filename] = info;
            }
            assert(info);

            bool found_text = false, found_data = false;
            WTX_TGT_ADDR_T text_addr = 0, data_addr = 0;
            for (unsigned i = 0; i < info->nSegments; ++i) {
                if (info->segment[i].type == WTX_SEGMENT_TEXT) {
                    found_text = true;
                    text_addr = info->segment[i].addr;
                }
                if (info->segment[i].type == WTX_SEGMENT_DATA) {
                    found_data = true;
                    data_addr = info->segment[i].addr;
                }
            }
            assert(found_text && found_data);

            fileDescriptor newDesc = fileDescriptor(curr->moduleName,
                                                    0x0 /*text_addr*/,
                                                    0x0 /*data_addr*/,
                                                    true);
            descs.push_back(newDesc);
            curr = curr->next;
        }
        // Don't free for now.  We'll do a full copy of
        // the WTX_MODULE_INFO structure later.
        // wtxResultFree(wtxh, list);

    } else {
        fprintf(stderr, "Error on wtxObjModuleListGet(): %s\n", wtxErrMsgGet(wtxh));
        assert(0);
    }

    return true;
}

// initialize: perform initialization tasks on a platform-specific level
bool dynamic_linking::initialize()
{
    dynlinked = true;
    return true;
}

/* Return true if the exception was caused by the hook in the linker
 * code, we'll worry about whether or not any libraries were
 * added/removed later on when we handle the exception
 */
bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord & /*ev*/,
    unsigned int & /*change_type*/) { assert(0); }

bool dynamic_linking::getChangedObjects(EventRecord & /* ev */, pdvector<mapped_object*> & /* changed_objects */) { assert(0); }

// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps,  If it is, and if the linkmaps state is
// safe, it processse the linkmaps to find out what has changed...if it
// is not safe it sets the type of change currently going on (specified by
// the value of r_debug.r_state in link.h
// The added or removed shared objects are returned in changed_objects
// the change_type value is set to indicate if the objects have been added 
// or removed
bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord & /*ev*/,
                                                       pdvector<mapped_object*> & /*changed_objects*/,
                                                       pdvector<bool> & /*is_new_object*/)
{
    // assert(0);
    return true;
}

// This function performs all initialization necessary to catch shared object
// loads and unloads (symbol lookups and trap setting)
bool dynamic_linking::installTracing()
{
    // Hopefully, we can get WTX's event system to notify us of such events.
    return true;
}
