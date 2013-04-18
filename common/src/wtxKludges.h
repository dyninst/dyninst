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


#if !defined(_wtx_kludges_h)
#define _wtx_kludges_h

#include "common/src/Types.h"
#include "common/h/dyntypes.h"

#ifndef HOST
#define HOST // vxWorks foundation libraries require this to be defined.
#endif
#ifndef NOMINMAX
#define NOMINMAX // vxWorks definition of min and max is problematic.
#endif
#include "wtx.h"

typedef enum {
    WTX_UNKNOWN,
    WTX_INITIALIZED,
    WTX_CONNECTED,
    WTX_INVALID
} wtx_state_t;

extern HWTX wtxh;
extern wtx_state_t wtx_state;
extern std::vector<WTX_MODULE_INFO *> wtxDynLoadedMods;
extern dyn_hash_map<std::string, WTX_MODULE_INFO *> wtxMods;

bool wtxDisconnect(void);
void setAgentMode(WTX_AGENT_MODE_TYPE /*type*/);
bool wtxReadMem(const void *inTarget, u_int nbytes, void *inSelf);
bool wtxWriteMem(void *inTarget, u_int nbytes, const void *inSelf);
bool wtxFindSymbol(const char *name, WTX_SYMBOL_TYPE type, WTX_TGT_ID_T modId,
                   Address &addr);
bool wtxFindFunction(const char *name, WTX_TGT_ID_T modId, Address &addr);
bool wtxFindVariable(const char *name, WTX_TGT_ID_T modId, Address &addr);
bool wtxSuspendTask(WTX_TGT_ID_T ctxID);
WTX_MODULE_INFO *wtxLoadObject(const std::string &objname);
bool relocationTarget(const Address addr, Address *target);
void printEventpoints(void);
void printModuleInfo(WTX_MODULE_INFO *info);

#endif
