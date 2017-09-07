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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/walker.h"

#include "stackwalk/src/sw.h"

#include "get_trap_instruction.h"

#include "stackwalk/src/aarch32-swk.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
    assert(0);
    return false;
}

Dyninst::Architecture ProcSelf::getArchitecture()
{
    return Arch_aarch32;
}

bool Walker::checkValidFrame(const Frame & /*in*/, const Frame & /*out*/)
{
    return true;
}

FrameFuncStepperImpl::FrameFuncStepperImpl(Walker *w,
                                           FrameStepper *parent_,
                                           FrameFuncHelper *helper_) :
    FrameStepper(w),
    parent(parent_),
    helper(helper_)
{
    helper = helper_
        ? helper_
        : aarch32_LookupFuncStart::getLookupFuncStart(getProcessState());
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
    assert(0);
    return gcf_success;
}

unsigned FrameFuncStepperImpl::getPriority() const
{
    return frame_priority;
}

FrameFuncStepperImpl::~FrameFuncStepperImpl()
{
    aarch32_LookupFuncStart *lookup = dynamic_cast<aarch32_LookupFuncStart*>(helper);
    if (lookup)
        lookup->releaseMe();
    else if (helper)
        delete helper;
}

WandererHelper::WandererHelper(ProcessState *proc_) :
    proc(proc_)
{
}

bool WandererHelper::isPrevInstrACall(Address, Address&)
{
    sw_printf("[%s:%u] - Unimplemented on this platform!\n");
    assert(0);
    return false;
}

WandererHelper::pc_state WandererHelper::isPCInFunc(Address, Address)
{
    sw_printf("[%s:%u] - Unimplemented on this platform!\n");
    assert(0);
    return unknown_s;
}

bool WandererHelper::requireExactMatch()
{
    sw_printf("[%s:%u] - Unimplemented on this platform!\n");
    assert(0);
    return true;
}

WandererHelper::~WandererHelper()
{
}

gcframe_ret_t DyninstInstrStepperImpl::getCallerFrameArch(const Frame &/*in*/, Frame &/*out*/,
                                                          Address /*base*/, Address /*lib_base*/,
                                                          unsigned /*size*/, unsigned /*stack_height*/)
{
    return gcf_not_me;
}

gcframe_ret_t DyninstDynamicStepperImpl::getCallerFrameArch(const Frame &in, Frame &out,
                                                            Address /*base*/, Address /*lib_base*/,
                                                            unsigned /*size*/, unsigned stack_height,
                                                            bool /* aligned */,
                                                            Address /*orig_ra*/, bool pEntryExit)
{
    assert(0);
    return gcf_success;
}

std::map<Dyninst::PID, aarch32_LookupFuncStart*> aarch32_LookupFuncStart::all_func_starts;

static int hash_address(Address a)
{
    return (int) a;
}

// to handle leaf functions
aarch32_LookupFuncStart::aarch32_LookupFuncStart(ProcessState *proc_) :
   FrameFuncHelper(proc_),
   cache(cache_size, hash_address)
{
    all_func_starts[proc->getProcessId()] = this;
    ref_count = 1;
}

aarch32_LookupFuncStart::~aarch32_LookupFuncStart()
{
    Dyninst::PID pid = proc->getProcessId();
    all_func_starts.erase(pid);
}

aarch32_LookupFuncStart *aarch32_LookupFuncStart::getLookupFuncStart(ProcessState *p)
{
    Dyninst::PID pid = p->getProcessId();
    std::map<Dyninst::PID, aarch32_LookupFuncStart*>::iterator i = all_func_starts.find(pid);
    if (i == all_func_starts.end()) {
        return new aarch32_LookupFuncStart(p);
    }
    (*i).second->ref_count++;
    return (*i).second;
}

void aarch32_LookupFuncStart::releaseMe()
{
    ref_count--;
    if (!ref_count)
        delete this;
}

FrameFuncHelper::alloc_frame_t aarch32_LookupFuncStart::allocatesFrame(Address addr)
{
    assert(0);
    return alloc_frame_t(unknown_t, unknown_s);
}

void aarch32_LookupFuncStart::updateCache(Address addr, alloc_frame_t result)
{
    cache.insert(addr, result);
}

bool aarch32_LookupFuncStart::checkCache(Address addr, alloc_frame_t &result)
{
    return cache.lookup(addr, result);
}

namespace Dyninst {
namespace Stackwalker {

void getTrapInstruction(char *buffer, unsigned buf_size,
                        unsigned &actual_len, bool include_return)
{
    //trap
    //ret
    assert(buf_size >= 4);
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x20;
    buffer[3] = 0xd4;
    actual_len = 4;
    if (include_return) {
        assert(buf_size >= 8);
        buffer[4] = 0xc0;
        buffer[5] = 0x03;
        buffer[6] = 0x5f;
        buffer[7] = 0xd6;
        actual_len = 8;
        return;
    }

    assert(buf_size >= 1);
    actual_len = 1;
    return;
}

} // End namespace Stackwalker
} // End namespace Dyninst
