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

#include "common/src/Types.h"
#include "common/src/wtxKludges.h"
#include "common/src/pathName.h"
#include "symtabAPI/h/Symtab.h"
#include <elf.h>

HWTX wtxh = NULL;
wtx_state_t wtx_state = WTX_UNKNOWN;
std::vector<WTX_MODULE_INFO *> wtxDynLoadedMods;
dyn_hash_map<std::string, WTX_MODULE_INFO *> wtxMods;
dyn_hash_map<Address, Dyninst::SymtabAPI::relocationEntry> wtxReloc;

bool wtxDisconnect(void)
{
    if (wtx_state < WTX_CONNECTED) {
        fprintf(stderr, "Error: Attempt to disconnect while not connected.\n");
        return false;
    }

    STATUS result = wtxToolDetach(wtxh);
    if (result == WTX_OK) {
        wtx_state = WTX_INITIALIZED;
    } else {
        fprintf(stderr, "Error on wtxToolDetach(): %s\n", wtxErrMsgGet(wtxh));
        fprintf(stderr, "Forcing disconnection by terminating WTX handle.\n");
    }

    result = wtxTerminate(wtxh);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxTerminate(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    wtx_state = WTX_UNKNOWN;
    return true;
}

void setAgentMode(WTX_AGENT_MODE_TYPE /*type*/)
{
/*
    if (type != WTX_AGENT_MODE_TASK) return;

    STATUS result = wtxAgentModeSet(wtxh, type);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxAgentModeSet(): %s\n", wtxErrMsgGet(wtxh));
    }
*/
}

bool wtxReadMem(const void *inTarget, u_int nbytes, void *inSelf)
{
    if (wtx_state < WTX_CONNECTED) {
        fprintf(stderr, "Attempting to read target memory while disconnected.\n");
        return false;
    }

    UINT32 result = wtxMemRead(wtxh,
                               0x0,
                               (WTX_TGT_ADDR_T)inTarget,
                               inSelf,
                               nbytes,
                               WTX_MEM_CACHE_BYPASS);
    if (result == static_cast<UINT32>( WTX_ERROR ))
        fprintf(stderr, "wtxMemRead() error: %s\n", wtxErrMsgGet(wtxh));
    return (result != static_cast<UINT32>( WTX_ERROR ));
}

bool wtxWriteMem(void *inTarget, u_int nbytes, const void *inSelf)
{
    if (wtx_state < WTX_CONNECTED) {
        fprintf(stderr, "Attempting to write target memory while disconnected.\n");
        return false;
    }

    /* DEBUG
    fprintf(stderr, "Attempting to write to range 0x%x-0x%x (%d bytes)\n",
            (unsigned int)inTarget,
            (unsigned int)(((unsigned char *)inTarget)) + nbytes,
            nbytes); // */

    UINT32 result = wtxMemWrite(wtxh,
                                0x0,
                                const_cast<void *>(inSelf),
                                (WTX_TGT_ADDR_T)inTarget,
                                nbytes,
                                WTX_MEM_CACHE_BYPASS);
    if (result == static_cast<UINT32>( WTX_ERROR )) {
        fprintf(stderr, "wtxMemWrite() error: %s\n", wtxErrMsgGet(wtxh));
        return false;
    }

    result = wtxCacheTextUpdate(wtxh, (WTX_TGT_ADDR_T)inTarget, nbytes);
    if (result == static_cast<UINT32>( WTX_ERROR )) {
        fprintf(stderr, "wtxCacheTextUpdate() error: %s\n",
                wtxErrMsgGet(wtxh));
        return false;
    }

    return true;
}

bool wtxFindSymbol(const char *name, WTX_SYMBOL_TYPE type, WTX_TGT_ID_T modId,
                   Address &addr)
{
    WTX_SYM_FIND_CRITERIA crit;

    if (wtx_state < WTX_CONNECTED) {
        // There are legitimate reasons to do this.
        // We should fail without warning.

        //fprintf(stderr, "Attempting to read symbol information disconnected.\n");
        return false;
    }

    memset(&crit, 0, sizeof(WTX_SYM_FIND_CRITERIA));
    crit.options  = WTX_SYM_FIND_BY_EXACT_NAME;
    crit.pdId     = 0x0;
    crit.findName = const_cast<char *>(name);

    if (type) {
        crit.options = crit.options | WTX_SYM_FILTER_ON_TYPE;
        crit.type    = type;
    }

    if (modId) {
        crit.options  = crit.options | WTX_SYM_FILTER_ON_MODULE_NAME;
        crit.moduleId = modId;
    }

    WTX_SYMBOL *sym = wtxSymFind(wtxh, &crit);
    if (sym) {
        addr = sym->value;
        wtxResultFree(wtxh, sym);
        return true;
    }
    return false;
}

bool wtxFindFunction(const char *name, WTX_TGT_ID_T modId, Address &addr)
{
    return wtxFindSymbol(name, WTX_SYMBOL_TEXT, modId, addr);
}

bool wtxFindVariable(const char *name, WTX_TGT_ID_T modId, Address &addr)
{
    return (wtxFindSymbol(name, WTX_SYMBOL_DATA, modId, addr) ||
            wtxFindSymbol(name, WTX_SYMBOL_BSS,  modId, addr) ||
            wtxFindSymbol(name, WTX_SYMBOL_COMM, modId, addr));
}

bool wtxSuspendTask(WTX_TGT_ID_T ctxID)
{
    WTX_CONTEXT ctx;
    ctx.contextType = WTX_CONTEXT_TASK;
    ctx.contextId = ctxID;
    ctx.contextSubId = 0;

    STATUS result = wtxContextSuspend(wtxh, &ctx);
    if (result != WTX_OK) {
        fprintf(stderr, "Error on wtxContextSuspend(): %s\n", wtxErrMsgGet(wtxh));
        return false;
    }
    return true;
}

#if defined(arch_power)
bool relocationTarget(const Address addr, Address *target)
{
    if (wtxReloc.count(addr) && target) {
        
        WTX_SYMBOL_TYPE any = (WTX_SYMBOL_TYPE)0x0;
        if (wtxFindSymbol(wtxReloc[addr].name().c_str(), any, 0x0, *target)) {

            switch (wtxReloc[addr].getRelType()) {
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
    }
    return false;
}

#elif defined(arch_x86)
bool relocationTarget(const Address addr, Address *target)
{
    if (wtxReloc.count(addr) && target) {
        wtxFindSymbol(wtxReloc[addr].name().c_str(),
                      (WTX_SYMBOL_TYPE)0x0,
                      0x0,
                      *target);
        return true;
    }
    return false;
}
#endif

WTX_MODULE_INFO *wtxLoadObject(const std::string &objname)
{
//    std::string shortname = extract_pathname_tail(objname);
    std::string shortname = objname;

    // Library already loaded.
    if (wtxMods.count(shortname)) return wtxMods[shortname];

    WTX_MODULE_FILE_DESC desc;
    memset(&desc, 0, sizeof(WTX_MODULE_FILE_DESC));
    desc.filename = const_cast<char *>(objname.c_str());
    desc.loadFlag = WTX_LOAD_ALL_SYMBOLS;

    WTX_MODULE_INFO *info;
    info = wtxObjModuleLoad(wtxh, 0, &desc, WTX_LOAD_FROM_TOOL);
    if (info == NULL) {
        fprintf(stderr, "Error on wtxObjModuleLoad(): %s\n", wtxErrMsgGet(wtxh));
        wtxDisconnect();
        return NULL;
    }
    wtxDynLoadedMods.push_back(info);
    wtxMods[shortname] = info;

    return info;
}

void printEventpoints(void)
{
    WTX_EVTPT_LIST *pts = wtxEventpointListGet(wtxh);
    if (!pts) {
        fprintf(stderr, "Error on wtxEventpointListGet(): %s\n", wtxErrMsgGet(wtxh));
        return;
    }

    fprintf(stderr, "------------------------------------\n");
    fprintf(stderr, "%d eventpoints.\n\n", pts->nEvtpt);
    for (unsigned int i = 0; i < pts->nEvtpt; ++i) {
        fprintf(stderr, "[%d] evtpt:{ event:{ type:%d narg:%d } ctxid:0x%lx action:{ type:0x%x arg:%d } tool:0x%lx ptnum:%d\n", i,
                pts->pEvtptInfo[i].wtxEvtpt.event.eventType,
                pts->pEvtptInfo[i].wtxEvtpt.event.numArgs,
                pts->pEvtptInfo[i].wtxEvtpt.context.contextId,
                pts->pEvtptInfo[i].wtxEvtpt.action.actionType,
                pts->pEvtptInfo[i].wtxEvtpt.action.actionArg,
                pts->pEvtptInfo[i].toolId,
                pts->pEvtptInfo[i].evtptNum);
    }
    fprintf(stderr, "------------------------------------\n");
}

void printModuleInfo(WTX_MODULE_INFO *info)
{
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
}

