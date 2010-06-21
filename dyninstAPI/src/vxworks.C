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

// $Id: vxworks.C,v 1.279 2008/09/03 06:08:44 jaw Exp $

#include <fstream>
#include <string>

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

#ifndef HOST
#define HOST // vxWorks foundation libraries require this to be defined.
#endif
#include "wtx.h"

// #include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/eventgate.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "common/h/headers.h"
#include "common/h/linuxKludges.h"
#include "dyninstAPI/src/os.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "parseAPI/src/InstrucIter.h"

#include "ast.h" // instrumentation for MT

#include "dynamiclinking.h"
#include "symtabAPI/h/Symtab.h"

// Global declarations
// XXX Find a place for these eventually
WTX_TGT_ID_T currCtx = 0;
image *currImage;
process *currProc;

eventLock wtxLock;
std::vector<EventRecord> wtxEvents;

int wtx_state = 0;
HWTX wtxh;
dyn_hash_map<std::string, WTX_MODULE_INFO *> wtxMods;
dyn_hash_map<std::string, WTX_CONTEXT_ID_T> wtxTasks;
dyn_hash_map<Address, WTX_TGT_ID_T> wtxBreakpoints;
dyn_hash_map<Address, relocationEntry> reloc;

//TODO: Remove the writeBack functions and get rid of this include
#ifdef PAPI
#include "papi.h"
#endif

void printStackWalk( process *p ) { assert(0); return; }

#if 0
// Linux Only
WaitpidMux SignalGenerator::waitpid_mux;

bool SignalGenerator::attachToChild(int pid) { assert(0); return false; }
bool SignalGenerator::add_lwp_to_poll_list(dyn_lwp *lwp) { assert(0); return false; }
bool SignalGenerator::remove_lwp_from_poll_list(int lwp_id) { assert(0); return false; }
bool SignalGenerator::resendSuppressedSignals() { assert(0); return false; }
bool SignalGenerator::exists_dead_lwp() { assert(0); return false; }
int SignalGenerator::find_dead_lwp() { assert(0); return false; }
bool SignalGenerator::suppressSignalWhenStopping(EventRecord &ev) { assert(0); return false; }
pid_t SignalGenerator::waitpid_kludge(pid_t /*pid_arg*/, int *status, int /*options*/, int *dead_lwp) { assert(0); return (pid_t)0; }

void process::independentLwpControlInit() { assert(0); return; }
bool process::waitUntilLWPStops() { assert(0); return false; }
bool process::loadDYNINSTlib_exported(const char *) { assert(0); return false; }
bool process::loadDYNINSTlib_hidden() { assert(0); return false; }
bool process::readAuxvInfo() { assert(0); return false; }

bool dyn_lwp::isRunning() const { assert(0); return false; }
bool dyn_lwp::isWaitingForStop() const { assert(0); return false; }
bool dyn_lwp::removeSigStop() { assert(0); return false; }

bool ForkNewProcessCallback::operator()(std::string file, 
                    std::string dir, pdvector<std::string> *argv,
                    pdvector<std::string> *envp,
                    std::string inputFile, std::string outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd,
    SignalGenerator *sg) { assert(0); return false; }
bool ForkNewProcessCallback::execute_real() { assert(0); return false; }

/* get_ld_info() returns true if it filled in the base address of the
   ld.so library and its path, and false if it could not. */
bool dynamic_linking::get_ld_info( Address & addr, unsigned &size, char ** path) { assert(0); }
// getLinkMapAddrs: returns a vector of addresses corresponding to all 
// base addresses in the link maps.  Returns 0 on error.
pdvector<Address> *dynamic_linking::getLinkMapAddrs() { assert(0); }

#endif

void InstrucIter::readWriteRegisters(int* readRegs, int* writeRegs) { assert(0); }

/* **********************************************************************
 * VxWorks local functions
 * **********************************************************************/
void setAgentMode(WTX_AGENT_MODE_TYPE type)
{
/*
    if (type != WTX_AGENT_MODE_TASK) return;

    STATUS result = wtxAgentModeSet(wtxh, type);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxAgentModeSet(): %s\n", wtxErrMsgGet(wtxh));
    }
*/
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

    fprintf(stderr, "WTX Event Received '%s' + %d\n", desc->event, desc->addlDataLen);
    for (unsigned int i = 0; i < desc->addlDataLen; ++i) {
        fprintf(stderr, "0x%x ", desc->addlData[i]);
        if (i % 15 == 0) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n---END\n");

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
            fprintf(stderr, "*** ERROR: I don't know about event at address 0x%x...\n", addr);
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
    WTX_EVTPT point;
    memset(&point, 0, sizeof(WTX_EVTPT));

    point.event.eventType     = WTX_EVENT_TRIGGER;
    point.event.numArgs       = 1;

    point.context.contextType = WTX_CONTEXT_TASK;
    point.context.contextId   = ctxID;

    point.action.actionType   = (WTX_ACTION_TYPE)(WTX_ACTION_NOTIFY |
                                                  WTX_ACTION_STOP);

    for (unsigned int i = 0; triggerlist[i] != WTX_EVENT_INVALID; ++i) {
        point.event.args = &triggerlist[i];
        if (wtxEventpointAdd(wtxh, &point) == WTX_ERROR) {
            fprintf(stderr, "Error on wtxEventpointAdd(%d): %s\n", triggerlist[i], wtxErrMsgGet(wtxh));
        }
    }

    if (wtxContextExitNotifyAdd(wtxh, &point.context) == WTX_ERROR) {
        fprintf(stderr, "Error on wtxContextExitNotifyAdd(): %s\n", wtxErrMsgGet(wtxh));
    }
#if 0
    typedef struct wtx_event                /* target event                       */
    {
        WTX_EVENT_TYPE      eventType;      /* type of event                      */
        UINT32              numArgs;        /* number of arguments                */
        WTX_TGT_ARG_T *     args;           /* list of arguments                  */
    } WTX_EVENT;

    typedef struct wtx_action               /* action descriptor                  */
    {
        WTX_ACTION_TYPE     actionType;     /* action type to perform             */
        UINT32              actionArg;      /* action dependent argument          */
        WTX_TGT_ADDR_T      callRtn;        /* function to ACTION_CALL            */
        WTX_TGT_ARG_T       callArg;        /* function argument                  */
    } WTX_ACTION;
    typedef struct wtx_evtpt                /* eventpoint descriptor              */
    {
        WTX_EVENT           event;          /* event to detect                    */
        WTX_CONTEXT         context;        /* context descriptor                 */
        WTX_ACTION          action;         /* action to perform                  */
    } WTX_EVTPT;
#endif
}

void fillRelocationMap(const Symtab *obj)
{
    vector<relocationEntry> fbt;
    if (!obj) return;

    obj->getFuncBindingTable(fbt);
    for(unsigned i = 0; i < fbt.size(); ++i)
        reloc[fbt[i].rel_addr()] = fbt[i];
}

void launch_task(const std::string &filename, mapped_object *obj)
{
    // First check user-supplied init routine.
    WTX_TGT_ADDR_T initAddr = 0x0;
    if (wtxMods.count(filename))
        initAddr = wtxMods[filename]->vxCompRtn;

    if (initAddr) {
        setAgentMode(WTX_AGENT_MODE_TASK);

        WTX_CONTEXT_DESC desc;
        memset(&desc, 0, sizeof(WTX_CONTEXT_DESC));
        desc.wtxContextType = WTX_CONTEXT_TASK;
        desc.wtxContextDef.wtxTaskContextDef.name = "modInit";
        desc.wtxContextDef.wtxTaskContextDef.priority = 100;
        desc.wtxContextDef.wtxTaskContextDef.entry = initAddr;
        WTX_CONTEXT_ID_T ctxID = wtxContextCreate(wtxh, &desc);
        enableEventPoints(ctxID);
/* Don't run the task right away.

        WTX_CONTEXT ctx;
        memset(&ctx, 0, sizeof(WTX_CONTEXT));
        ctx.contextType = WTX_CONTEXT_TASK;
        ctx.contextId   = ctxID;
        if (wtxContextResume(wtxh, &ctx) != WTX_OK)
            fprintf(stderr, "Error starting init task for %s: %s\n",
                    filename.c_str(), wtxErrMsgGet(wtxh));
*/

        setAgentMode(WTX_AGENT_MODE_EXTERN);
    }

    // First check user-supplied init routine.
    WTX_TGT_ADDR_T entryAddr = 0x0;
    if (wtxMods.count(filename))
        entryAddr = wtxMods[filename]->userInitRtn;

    // Drop back to symbol main if needed.
    if (!entryAddr) {
        const pdvector <int_function *> *funcs;
        funcs = obj->findFuncVectorByMangled("main");

        if (funcs && funcs->size())
            entryAddr = (*funcs)[0]->getAddress();
    }

    if (entryAddr) {
        std::string shortname = extract_pathname_tail(filename);
        fprintf(stderr, "Launching task %s with start address 0x%x\n", shortname.c_str(), entryAddr);

        setAgentMode(WTX_AGENT_MODE_TASK);

        WTX_CONTEXT_DESC desc;
        memset(&desc, 0, sizeof(WTX_CONTEXT_DESC));
        desc.wtxContextType = WTX_CONTEXT_TASK;
        desc.wtxContextDef.wtxTaskContextDef.name = const_cast<char *>(shortname.c_str());
        desc.wtxContextDef.wtxTaskContextDef.priority = 100;
        desc.wtxContextDef.wtxTaskContextDef.entry = entryAddr;
        WTX_CONTEXT_ID_T ctxID = wtxContextCreate(wtxh, &desc);

        if (ctxID == WTX_ERROR)
            fprintf(stderr, "Error starting task for %s: %s\n",
                    filename.c_str(), wtxErrMsgGet(wtxh));
        else {
            currCtx = ctxID;
            currImage = obj->parse_img();
//            currProc = obj->proc();
        }

        setAgentMode(WTX_AGENT_MODE_EXTERN);
        enableEventPoints(ctxID);

        fakeWtxEvent(static_cast<process *>(obj->proc()), evtProcessInit, 5);
    }
    // wtxTasks[filename] = ctx;
}

bool fixup_offsets(const std::string &filename, Dyninst::SymtabAPI::Symtab *linkedFile)
{
    if (!wtxMods.count(filename))
        return false;

    WTX_MODULE_INFO *info = wtxMods[filename];
    unsigned int i;

    // Fixup loadable sections
    for (i = 0; i < info->nSections; ++i) {
        linkedFile->fixup_RegionAddr(info->section[i].name,
                                     info->section[i].baseAddr,
                                     info->segment[i].length);
    }

    // Fixup regions
    WTX_TGT_ADDR_T
        code_off = (WTX_TGT_ADDR_T) -1,
        code_len = (WTX_TGT_ADDR_T) -1,
        data_off = (WTX_TGT_ADDR_T) -1,
        data_len = (WTX_TGT_ADDR_T) -1;

    // Don't need offset locations anymore.  Still need this to find the len.
    for (i = 0; i < info->nSegments; ++i) {
        if (info->segment[i].type == WTX_SEGMENT_TEXT) {
            //linkedFile->fixup_RegionAddr("CODE_SEGMENT",
            //                             info->segment[i].addr,
            //                             info->segment[i].length);
            code_off = info->segment[i].addr;
            code_len = code_off + info->segment[i].length;

        } else if (info->segment[i].type == WTX_SEGMENT_DATA) {
            //linkedFile->fixup_RegionAddr("DATA_SEGMENT",
            //                             info->segment[i].addr,
            //                             info->segment[i].length);
            data_off = info->segment[i].addr;
            data_len = data_off + info->segment[i].length;

        } else if (info->segment[i].type == WTX_SEGMENT_BSS) {
            //linkedFile->fixup_RegionAddr("BSS_SEGMENT",
            //                             info->segment[i].addr,
            //                             info->segment[i].length);
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
    linkedFile->fixup_code_and_data(0x0 /*code_off*/, code_len,
                                    0x0 /*data_off*/, data_len);

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
        WTX_SYMBOL *curr = list->pSymbol;
        while (curr) {
            if (!linkedFile->fixup_SymbolAddr(curr->name, curr->value)) {
                fprintf(stderr, "Type: 0x%x\n", curr->type);
            }
            curr = curr->next;
        }
        wtxResultFree(wtxh, list);
    }

    fillRelocationMap(linkedFile);
    return true;
}

void addBreakpoint(Address bp)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    WTX_TGT_ID_T point = wtxBreakpointAdd(wtxh, &ctx, (WTX_TGT_ADDR_T)bp);
    if (point == WTX_ERROR) {
        fprintf(stderr, "Error on wtxBreakpointAdd(): %s\n", wtxErrMsgGet(wtxh));
        return;
    }

    fprintf(stderr, "Successfully added a breakpoint at 0x%x\n", bp);
    wtxBreakpoints[bp] = point;
}

bool wtxFindSymbol(const char *name, WTX_SYMBOL_TYPE t, Address &addr)
{
    WTX_SYM_FIND_CRITERIA crit;
    memset(&crit, 0, sizeof(WTX_SYM_FIND_CRITERIA));
    crit.options    = WTX_SYM_FIND_BY_NAME;

    crit.pdId       = 0x0;
    crit.findName   = const_cast<char *>(name);
    crit.type       = t;
    
    WTX_SYM_LIST *list = wtxSymListGet(wtxh, &crit);
    if (list) {
        if (list->pSymbol) {
            addr = list->pSymbol->value;
            wtxResultFree(wtxh, list);
            return true;
        }
        wtxResultFree(wtxh, list);
    }
    return false;
}

bool wtxFindFunction(const char *name, Address &addr)
{
    return wtxFindSymbol(name, WTX_SYMBOL_TEXT, addr);
}

bool relocationTarget(const Address addr, Address *target)
{
    if (reloc.count(addr)) {
        if (!target) return true;

        string symname = reloc[addr].name();
//        fprintf(stderr, "There is a relocation at 0x%x (to %s)\n", addr, symname.c_str());
#if 1
        if (wtxFindFunction(symname.c_str(), *target)) {
//            fprintf(stderr, "Replacing with relocation address 0x%x\n", *target);
            switch (reloc[addr].getRelType()) {
            case R_PPC_ADDR16_HA:
                *target = (*target >> 16) + ((*target & 0x8000) ? 1 : 0);
                break;
            case R_PPC_ADDR16_LO:
                *target = *target & 0xFFFF;
                break;
            default:
                break;
            }
            return true;
        }
#else
        const pdvector<image_func *> *funclist;
        funclist = currImage->findFuncVectorByPretty(symname);

        fprintf(stderr, "Looking up %s...\n", symname.c_str());
        if (funclist && funclist->size() > 0) {
            fprintf(stderr, "Replacing with relocation address 0x%x\n", (*funclist)[0]->getOffset());
            *target = (*funclist)[0]->getOffset();

            switch (reloc[addr].getRelType()) {
            case R_PPC_ADDR16_HA:
                *target = (*target >> 16) + ((*target & 0x8000) ? 1 : 0);
                break;
            case R_PPC_ADDR16_LO:
                *target = *target & 0xFFFF;
                break;
            default:
                break;
            }
            return true;
        }
#endif
    }
    return false;
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
void OS::make_tempfile(char *s) { assert(0); }
bool OS::execute_file(char *path) { assert(0); }
void OS::unlink(char *file) { assert(0); }

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
    : SignalGeneratorCommon(idstr)
{
    setupAttached(file, pid);
}
SignalGenerator::~SignalGenerator()
{

}
bool SignalGenerator::decodeSyscall(EventRecord &ev) {assert(0);}
bool SignalGenerator::forkNewProcess()
{
    STATUS result;

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
    result = wtxRegisterForEvent(wtxh, ".*");
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegisterForEvent(.*): %s\n", wtxErrMsgGet(wtxh));
        wtxTerminate(wtxh);
        return false;
    }

    result = wtxAsyncNotifyEnable(wtxh, wtxEventHandler);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxAsyncNotifyEnable(): %s\n", wtxErrMsgGet(wtxh));
        wtxTerminate(wtxh);
        return false;
    }

    WTX_MODULE_FILE_DESC f_desc;
    string fullpath = dir_ + file_;
    memset(&f_desc, 0, sizeof(WTX_MODULE_FILE_DESC));
    f_desc.filename = const_cast<char *>(fullpath.c_str());
    f_desc.loadFlag = WTX_LOAD_ALL_SYMBOLS;

    WTX_MODULE_INFO *info = wtxObjModuleLoad(wtxh, 0, &f_desc, WTX_LOAD_FROM_TARGET_SERVER);
    if (info == NULL) {
        fprintf(stderr, "Error on wtxObjModuleLoad(): %s\n", wtxErrMsgGet(wtxh));
        if (wtxToolConnected(wtxh)) {
            result = wtxToolDetach(wtxh);
        }
        wtxTerminate(wtxh);
        return false;
    }
    wtxMods[fullpath] = info;

    unsigned i;
    fprintf(stderr, "protection domain ID: 0x%x\n", (unsigned int)info->pdId);
    fprintf(stderr, "module ID: 0x%x\n", (unsigned int)info->moduleId);
    fprintf(stderr, "module name: %s\n", info->moduleName);
    fprintf(stderr, "object file format: 0x%x\n", (unsigned int)info->format);
    fprintf(stderr, "load flags: 0x%x\n", (unsigned int)info->loadFlag);
    fprintf(stderr, "component init/term routine: 0x%x\n", (unsigned int)info->vxCompRtn);
    fprintf(stderr, "user-supplied init routine: 0x%x\n", (unsigned int)info->userInitRtn);
    fprintf(stderr, "memory used by common symbols: 0x%x\n", (unsigned int)info->commTotalSize);
    fprintf(stderr, "Number of sections: %d\n", (int)info->nSections);
    for (i = 0; i < info->nSections; ++i) {
        fprintf(stderr, "\tSection: 0x%x Name: %s Type: 0x%x Flags: 0x%x BaseAddr: 0x%x Partition: 0x%x Len: 0x%x Sum: 0x%x\n",
                (unsigned int)info->section[i].id,
                info->section[i].name,
                (unsigned int)info->section[i].type,
                (unsigned int)info->section[i].flags,
                (unsigned int)info->section[i].baseAddr,
                (unsigned int)info->section[i].partId,
                (unsigned int)info->section[i].length,
                (unsigned int)info->section[i].checksum);
    }
    fprintf(stderr, "Group: 0x%x\n", (unsigned int)info->group);
    fprintf(stderr, "Number of segments: %d\n", (int)info->nSegments);
    for (i = 0; i < info->nSegments; ++i) {
        fprintf(stderr, "\tType: 0x%x  Addr: 0x%x Len: 0x%x Flags: 0x%x\n",
                (unsigned int)info->segment[i].type,
                (unsigned int)info->segment[i].addr,
                (unsigned int)info->segment[i].length,
                (unsigned int)info->segment[i].flags);
    }

    setAgentMode(WTX_AGENT_MODE_EXTERN);

//    proc->setBootstrapState(libcLoaded_bs);
    pid_ = info->moduleId;
    fakeWtxEvent(proc, evtProcessCreate, 5);
    return true;
}

bool SignalGenerator::decodeEvents(pdvector<EventRecord> &events)
{
    for (unsigned int i = 0; i < events.size(); i++) {
        // If we already know the details of this event, skip it.
        if (events[i].type != evtUndefined)
            continue;

        // Check if the task no longer exists.
/*
        assert(events[i].lwp);
        Frame af = events[i].lwp->getActiveFrame();

        // Is this trap due to an instPoint?
        if (isInstTrap(events[i], af)) {
            events[i].type = evtInstPointTrap;
            events[i].address = af.getPC();
        }

        // Is this trap due to a RPC?
        else 
*/
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

    bool ret = true;
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
        assert(0);
        desc = fileDescriptor(filename.c_str(), 0x0, 0x0, false);
        return false;
    }
    WTX_MODULE_INFO *info = wtxMods[filename];
    assert(info);

    WTX_TGT_ADDR_T text_addr = 0, data_addr = 0;
    for (unsigned i = 0; i < info->nSegments; ++i) {
        if (info->segment[i].type == WTX_SEGMENT_TEXT)
            text_addr = info->segment[i].addr;
        if (info->segment[i].type == WTX_SEGMENT_DATA)
            data_addr = info->segment[i].addr;
    }
    assert(text_addr && data_addr);

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
bool SignalGeneratorCommon::decodeRTSignal_NP(EventRecord &ev, Address rt_arg, int status) { assert(0); }

/* **********************************************************************
 * Class SignalHandler Implementations
 * **********************************************************************/
bool SignalHandler::handleExecEntry(EventRecord &ev, bool &continueHint) { assert(0); }
bool SignalHandler::handleProcessCreate(EventRecord &ev, bool &continueHint)
{
    
    return true;
}

bool SignalHandler::handleProcessAttach(EventRecord &ev, bool &continueHint) { assert(0); }
bool SignalHandler::handleThreadCreate(EventRecord &, bool &) { assert(0); }
bool SignalHandler::handleSignalHandlerCallback(EventRecord &ev) { assert(0); }
bool SignalHandler::forwardSigToProcess(EventRecord &ev, bool &continueHint) { assert(0); }


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
bool process::trapAtEntryPointOfMain(dyn_lwp *trappingLWP, Address) { assert(0); return false; }
bool process::trapDueToDyninstLib(dyn_lwp *trappingLWP) { assert(0); return false; }
bool process::setProcessFlags()
{
    // Use this to modify any necessary state of the debugging link.
    return true;
}

bool process::unsetProcessFlags() { assert(0); return false; }
bool process::isRunning_() const { assert(0); return false; }
bool process::waitUntilStopped() { assert(0); return false; }
terminateProcStatus_t process::terminateProc_() { assert(0); return terminateFailed; }
bool process::dumpCore_(const std::string/* coreFile*/) { assert(0); return false; }
std::string process::tryToFindExecutable(const std::string& /* progpath */, int pid) { assert(0); return ""; }
bool process::determineLWPs(pdvector<unsigned> &lwp_ids) { assert(0); return false; }
bool process::dumpImage( std::string ) { assert(0); return false; }
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi, inferiorHeapType /* type */ )
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
bool process::hasPassedMain() { assert(0); return false; }
bool process::initMT() { assert(0); return false; }
bool process::handleTrapAtEntryPointOfMain(dyn_lwp *trappingLWP)
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
    STATUS result;
    WTX_MODULE_FILE_DESC desc;
    memset(&desc, 0, sizeof(WTX_MODULE_FILE_DESC));
    desc.filename = const_cast<char *>(dyninstRT_name.c_str());
    desc.loadFlag = WTX_LOAD_ALL_SYMBOLS;

    setAgentMode(WTX_AGENT_MODE_TASK);

    WTX_MODULE_INFO *info;
    info = wtxObjModuleLoad(wtxh, 0, &desc, WTX_LOAD_FROM_TARGET_SERVER);
    if (info == NULL) {
        fprintf(stderr, "Error on wtxObjModuleLoad(): %s\n", wtxErrMsgGet(wtxh));
        if (wtxToolConnected(wtxh)) {
            result = wtxToolDetach(wtxh);
        }
        wtxTerminate(wtxh);
        return false;
    }
    wtxMods[dyninstRT_name] = info;

    fprintf(stderr, "FOR THE RUNTIME LIBRARY\n");
    unsigned i;
    fprintf(stderr, "protection domain ID: 0x%x\n", (unsigned int)info->pdId);
    fprintf(stderr, "module ID: 0x%x\n", (unsigned int)info->moduleId);
    fprintf(stderr, "module name: %s\n", info->moduleName);
    fprintf(stderr, "object file format: 0x%x\n", (unsigned int)info->format);
    fprintf(stderr, "load flags: 0x%x\n", (unsigned int)info->loadFlag);
    fprintf(stderr, "component init/term routine: 0x%x\n", (unsigned int)info->vxCompRtn);
    fprintf(stderr, "user-supplied init routine: 0x%x\n", (unsigned int)info->userInitRtn);
    fprintf(stderr, "memory used by common symbols: 0x%x\n", (unsigned int)info->commTotalSize);
    fprintf(stderr, "Number of sections: %d\n", (int)info->nSections);
    for (i = 0; i < info->nSections; ++i) {
        fprintf(stderr, "\tSection: 0x%x Name: %s Type: 0x%x Flags: 0x%x BaseAddr: 0x%x Partition: 0x%x Len: 0x%x Sum: 0x%x\n",
                (unsigned int)info->section[i].id,
                info->section[i].name,
                (unsigned int)info->section[i].type,
                (unsigned int)info->section[i].flags,
                (unsigned int)info->section[i].baseAddr,
                (unsigned int)info->section[i].partId,
                (unsigned int)info->section[i].length,
                (unsigned int)info->section[i].checksum);
    }
    fprintf(stderr, "Group: 0x%x\n", (unsigned int)info->group);
    fprintf(stderr, "Number of segments: %d\n", (int)info->nSegments);
    for (i = 0; i < info->nSegments; ++i) {
        fprintf(stderr, "\tType: 0x%x  Addr: 0x%x Len: 0x%x Flags: 0x%x\n",
                (unsigned int)info->segment[i].type,
                (unsigned int)info->segment[i].addr,
                (unsigned int)info->segment[i].length,
                (unsigned int)info->segment[i].flags);
    }

    setAgentMode(WTX_AGENT_MODE_EXTERN);

    WTX_TGT_ADDR_T text_addr = 0, data_addr = 0;
    for (unsigned i = 0; i < info->nSegments; ++i) {
        if (info->segment[i].type == WTX_SEGMENT_TEXT)
            text_addr = info->segment[i].addr;
        if (info->segment[i].type == WTX_SEGMENT_DATA)
            data_addr = info->segment[i].addr;
    }
    assert(text_addr && data_addr);

    fileDescriptor *fd = new fileDescriptor(dyninstRT_name,
                                            0x0 /*text_addr*/,
                                            0x0 /*data_addr*/,
                                            true);
    mapped_object *obj = mapped_object::createMappedObject(*fd, this);
    addASharedObject(obj);

    setBootstrapState(loadedRT_bs);
    return true;
}

bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
{
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

bool process::loadDYNINSTlibCleanup(dyn_lwp *trappingLWP) { assert(0); return false; }
bool process::startDebugger() { assert(0); }

// In process.C:
// bool process::stop_(bool waitUntilStop) { assert(0); return false; }
// bool process::continueProc_(int sig) { assert(0); return false; }
// Address process::setAOutLoadAddress(fileDescriptor &desc) { assert(0); return 0; }
// bool process::detachForDebugger(const EventRecord &/*crash_event*/) { assert(0); return false; }
// Frame process::preStackWalkInit(Frame startFrame) { assert(0); return startFrame; }
// Address dyn_lwp::step_next_insn() { assert(0); return 0; }

#if defined(cap_binary_rewriter)
std::pair<std::string, BinaryEdit*> BinaryEdit::openResolvedLibraryName(std::string filename) { assert(0); }
#endif

void emitCallRel32(unsigned disp32, unsigned char *&insn) { assert(0); return; }
static int lwp_kill(int pid, int sig) { assert(0); return 0; }
/**
 * Return the state of the process from /proc/pid/stat.
 * File format is:
 *   pid (executablename) state ...
 * where state is a character.  Returns '\0' on error.
 **/
static char getState(int pid) { assert(0); return 0; }
static std::string getNextLine(int fd) { assert(0); return ""; }
bool isPLT(int_function *f) { assert(0); return false; }

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

bool dyn_lwp::waitUntilStopped()
{
    unsigned char buf;
    while (1) {
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
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxContextSuspend(wtxh, &ctx);
    if (result != WTX_OK) {
        fprintf(stderr, "wtxContextSuspend() error: %s\n", wtxErrMsgGet(wtxh));
        return false;
    }
    return true;
}

void dyn_lwp::realLWP_detach_() { assert(0); return; }
void dyn_lwp::representativeLWP_detach_()
{
    return;
}
bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) { assert(0); return false; }
bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{   // No real distinction between text and data.
    return writeDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::readTextSpace(const void *inTraced, u_int amount, void *inSelf)
{   // No real distrinction between text and data.
    return readDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
    UINT32 result;

    result = wtxMemWrite(wtxh,
                         0x0,
                         const_cast<void *>(inSelf),
                         (WTX_TGT_ADDR_T)inTraced,
                         nbytes,
                         WTX_MEM_CACHE_BYPASS);
    if (result == WTX_ERROR) {
        fprintf(stderr, "wtxMemWrite() error: %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    result = wtxCacheTextUpdate(wtxh, (WTX_TGT_ADDR_T)inTraced, nbytes);
    if (result == WTX_ERROR) {
        fprintf(stderr, "wtxCacheTextUpdate() error: %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    return true;
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf)
{
    UINT32 result;

    result = wtxMemRead(wtxh,
                        0x0,
                        (WTX_TGT_ADDR_T)inTraced,
                        const_cast<void *>(inSelf),
                        nbytes,
                        WTX_MEM_CACHE_BYPASS);
    if (result == WTX_ERROR)
        fprintf(stderr, "wtxMemRead() error: %s\n", wtxErrMsgGet(wtxh));
    return (result != WTX_ERROR);
}

bool dyn_lwp::realLWP_attach_() { assert(0); return false; }
static bool is_control_stopped(int lwp) { assert(0); return false; }
bool dyn_lwp::representativeLWP_attach_()
{
    bool running = false;
    int result;

//    XXX Why does this assert fail?  Why doesn't it think we're created via fork?
//    assert(proc_->wasCreatedViaFork() && "*** Implememnt Me ***");

    if (proc_->wasCreatedViaAttach()) {
        running = proc_->isRunning_();
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
    STATUS result = wtxRegsSet(wtxh, //WTX API handle
                               &ctx, // WTX Context
                               WTX_REG_SET_IU, // type of register set
                               offsetof(struct dyn_saved_regs, sprs[3]), // first byte of register set
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

// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
    struct dyn_saved_regs r;

    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxRegsGet(wtxh, //WTX API handle
                               &ctx, // WTX Context
                               WTX_REG_SET_IU, // type of register set
                               0x0, // first byte of register set
                               sizeof(dyn_saved_regs), // number of bytes of register set
                               &r); // place holder for reg. values

    return Frame(swapBytesIfNeeded(r.sprs[ 3]), // PC
                 swapBytesIfNeeded(r.gprs[31]), // FP
                 swapBytesIfNeeded(r.gprs[ 1]), // SP
                 proc_->getPid(), proc_, NULL, this, true);
}

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs, bool includeFP)
{
    WTX_CONTEXT ctx;
    ctx.contextType  = WTX_CONTEXT_TASK;
    ctx.contextId    = currCtx;
    ctx.contextSubId = 0;

    STATUS result = wtxRegsGet(wtxh, //WTX API handle
                               &ctx, // WTX Context
                               WTX_REG_SET_IU, // type of register set
                               0x0, // first byte of register set
                               sizeof(dyn_saved_regs), // number of bytes of register set
                               regs); // place holder for reg. values
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
                               reg * instruction::size(), // first byte of register set
                               instruction::size(), // number of bytes of register set
                               &retval); // place holder for reg. values
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegsGet(): %s\n", wtxErrMsgGet(wtxh));
        return (Address)-1;
    }

    return swapBytesIfNeeded(retval);
}

bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs, bool includeFP)
{
    struct dyn_saved_regs newRegs;
    const unsigned long *hostp = (const unsigned long *)&regs;
    unsigned long *targp = (unsigned long *)&newRegs;
    unsigned long *end  = targp + (sizeof(dyn_saved_regs) /
                                   sizeof(unsigned long));
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
                               sizeof(dyn_saved_regs), // number of bytes of register set
                               &newRegs); // place holder for reg. values
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxRegsGet(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    return true;
}

int_function *dyn_thread::map_initial_func(int_function *ifunc) { assert(0); }

void loadNativeDemangler() { return; }

/* **********************************************************************
 * Class DebuggerInterface Implementations
 * **********************************************************************/
bool DebuggerInterface::bulkPtraceWrite(void *inTraced, u_int nbytes, void *inSelf, int pid, int /*address_width*/) { assert(0); return false; }
bool DebuggerInterface::bulkPtraceRead(void *inTraced, u_int nelem, void *inSelf, int pid, int /*address_width*/) { assert(0); return false; }

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
int_function *instPoint::findCallee()
{
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
    image_func *icallee = img_p_->getCallee();
    if (icallee) {
        // Now we have to look up our specialized version
        // Can't do module lookup because of DEFAULT_MODULE...
        const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName().c_str());
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
        int_function *pdf = proc()->findFuncByAddr(linkageTarget);

        if (pdf) {
            callee_ = pdf;
            return callee_;
        }
        else
            return NULL;
    }
    return NULL;

}

const unsigned int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 4;
const char DYNINST_LOAD_HIJACK_FUNCTIONS[][20] = {
  "__libc_start_main",
  "_init",
  "_start",
  "main"
};

/**
 * Returns an address that we can use to write the code that executes
 * dlopen on the runtime library.
 *
 * Inserting the code into libc is a good thing, since _dl_open
 * will sometimes check it's caller and return with a 'invalid caller'
 * error if it's called from the application.
 **/
Address findFunctionToHijack(process *p) { assert(0); return 0; }

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(process *p, std::string func, pdvector<int_function *> &result) { assert(0); return; }
void dyninst_yield() { assert(0); return; }

// ****** Support linux-specific forkNewProcess DBI callbacks ***** //

/* **********************************************************************
 * Class DBI Callback Implementations
 * **********************************************************************/
bool DebuggerInterface::forkNewProcess(std::string file, 
                    std::string dir, pdvector<std::string> *argv,
                    pdvector<std::string> *envp,
                    std::string inputFile, std::string outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd,
    SignalGenerator *sg) { assert(0); return false; }
bool SignalHandler::handleProcessExitPlat(EventRecord & /*ev*/,
                                          bool & /*continueHint */)
{
    return true;
}

static int P_gettid() { assert(0); return 0; }
void chld_handler(int) { assert(0); return; }
void chld_handler2(int, siginfo_t *, void *) { assert(0); return; }
static void kickWaitpider(int pid) { assert(0); return; }

/* **********************************************************************
 * Class WaitpidMux Implementations
 * **********************************************************************/
//Force the SignalGenerator to return -1, EINTR from waitpid
bool WaitpidMux::registerProcess(SignalGenerator *me) { assert(0); return false; }
bool WaitpidMux::registerLWP(unsigned lwpid, SignalGenerator *me) { assert(0); return false; }
bool WaitpidMux::unregisterLWP(unsigned lwpid, SignalGenerator *me) { assert(0); return false; }
bool WaitpidMux::unregisterProcess(SignalGenerator *me) { assert(0); return false; }
void WaitpidMux::forceWaitpidReturn() { assert(0); return; }
bool WaitpidMux::suppressWaitpidActivity() { assert(0); return false; }
bool WaitpidMux::resumeWaitpidActivity() { assert(0); return false; }
int WaitpidMux::waitpid(SignalGenerator *me, int *status) { assert(0); return 0; }
bool WaitpidMux::hasFirstTimer(SignalGenerator *me) { assert(0); return false; }
void WaitpidMux::addPidGen(int pid, SignalGenerator *sg) { assert(0); return; }
void WaitpidMux::removePidGen(int pid, SignalGenerator *sg) { assert(0); return; }
void WaitpidMux::removePidGen(SignalGenerator *sg) { assert(0); return; }
int WaitpidMux::enqueueWaitpidValue(waitpid_ret_pair ev, SignalGenerator *event_owner) { assert(0); return 0; }


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
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/registerSpace.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";

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


void calcVSyscallFrame(process *p) { assert(0); return; }
Frame Frame::getCallerFrame() { assert(0); }
bool Frame::setPC(Address newpc) { assert(0); return false; }
bool AddressSpace::getDyninstRTLibName() {
    startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable                                                                                              
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            fprintf(stderr, "Environment variable DYNINSTAPI_RT_LIB has not been defined.\n");
            return false;
        }
    }

    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name
            + std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }
    return true;
}

// floor of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Address region_lo(const Address x) { return 0; }

// floor of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Address region_lo_64(const Address x) { assert(0); return 0; }

// ceiling of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Address region_hi(const Address x) { return (Address)-1; }

// ceiling of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Address region_hi_64(const Address x) { assert(0); return 0; }

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

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) { assert(0); }
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
            if (wtxMods.count(curr->moduleName)) {
                info = wtxMods[curr->moduleName];

            } else {
                info = wtxObjModuleInfoGet(wtxh, 0x0, curr->moduleId);
                if (!info) {
                    fprintf(stderr, "Error on wtxObjModuleInfoGet(0x%x): %s\n",
                            curr->moduleId, wtxErrMsgGet(wtxh));
                    assert(0);
                }
                wtxMods[curr->moduleName] = info;
            }
            assert(info);

            WTX_TGT_ADDR_T text_addr = 0, data_addr = 0;
            for (unsigned i = 0; i < info->nSegments; ++i) {
                if (info->segment[i].type == WTX_SEGMENT_TEXT)
                    text_addr = info->segment[i].addr;
                if (info->segment[i].type == WTX_SEGMENT_DATA)
                    data_addr = info->segment[i].addr;
            }
            assert(text_addr && data_addr);

            fileDescriptor newDesc = fileDescriptor(curr->moduleName,
                                                    0x0 /*text_addr*/,
                                                    0x0 /*data_addr*/,
                                                    true,
                                                    text_addr);
            if (curr == list->pModule) {
                // If this is the first object returned from
                // wtxObjModuleListGet(), then it is the kernel object.
                // Mark the object so we can recognize it later.
                newDesc.setMember("<KERNEL>");
            }

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
bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord &ev,
    unsigned int & /*change_type*/) { assert(0); }

bool dynamic_linking::getChangedObjects(EventRecord & /* ev */, pdvector<mapped_object*> & /* changed_objects */) { assert(0); }

// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps,  If it is, and if the linkmaps state is
// safe, it processes the linkmaps to find out what has changed...if it
// is not safe it sets the type of change currently going on (specified by
// the value of r_debug.r_state in link.h
// The added or removed shared objects are returned in changed_objects
// the change_type value is set to indicate if the objects have been added 
// or removed
bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord &ev,
                                 pdvector<mapped_object*> &changed_objects,
    pdvector<bool> &is_new_object)
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

Frame dyn_thread::getActiveFrameMT() { assert(0); }
