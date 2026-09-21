#include "registers/loongarch64_regs.h"
/*
 * See the dyninst/COPYRIGHT file for copyright information.
 */

#include "loongarch64-swk.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "common/h/dyn_regs.h"
#include "common/src/arch-loongarch64.h"
#include "dwarf/h/dwarfFrameParser.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/ucontext.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace NS_loongarch64;

#define GET_STACK_POINTER(spr)  __asm__("move %0, $sp" : "=r"(spr))



bool Walker::createDefaultSteppers()
{
    FrameStepper *stepper;
    bool result;

    stepper = new FrameFuncStepper(this);
    result = addStepper(stepper);
    if (!result) {
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__, (void*)stepper);
        return false;
    }

    stepper = new DebugStepper(this);
    result = addStepper(stepper);
    if (!result) {
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__, (void*)stepper);
        return false;
    }

    stepper = new SigHandlerStepper(this);
    result = addStepper(stepper);
    if (!result) {
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__, (void*)stepper);
        return false;
    }

    stepper = new BottomOfStackStepper(this);
    result = addStepper(stepper);
    if (!result) {
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__, (void*)stepper);
        return false;
    }

    return true;
}

bool Walker::checkValidFrame(const Frame &/*in*/, const Frame &/*out*/)
{
    return true;
}

bool DebugStepperImpl::isFrameRegister(MachRegister reg)
{
    return (reg == Dyninst::loongarch64::fp);
}

bool DebugStepperImpl::isStackRegister(MachRegister reg)
{
    return (reg == Dyninst::loongarch64::sp);
}

gcframe_ret_t DyninstInstrStepperImpl::getCallerFrameArch(const Frame &/*in*/, Frame &/*out*/,
    Address /*orig_ra*/, Address /*orig_sp*/, unsigned /*num_ra*/, unsigned /*num_sp*/)
{
    assert(0 && "DyninstInstrStepperImpl::getCallerFrameArch not implemented for loongarch64");
    return gcf_error;
}

gcframe_ret_t DyninstDynamicStepperImpl::getCallerFrameArch(const Frame &/*in*/, Frame &/*out*/,
    Address /*orig_ra*/, Address /*orig_sp*/, unsigned /*num_ra*/, unsigned /*num_sp*/,
    bool /*aligned*/, Address /*start*/, bool /*entry_exit*/)
{
    assert(0 && "DyninstDynamicStepperImpl::getCallerFrameArch not implemented for loongarch64");
    return gcf_error;
}

static ucontext_t dummy_context;
static int sp_offset = (char*)&(dummy_context.uc_mcontext.__gregs[3]) - (char*)&dummy_context;
static int fp_offset = (char*)&(dummy_context.uc_mcontext.__gregs[22]) - (char*)&dummy_context;
static int pc_offset = (char*)&(dummy_context.uc_mcontext.__pc) - (char*)&dummy_context;

// LoongArch64 RA/FP pair structure for stack walking
struct ra_fp_pair_t {
    unsigned long FP;
    unsigned long LR;
};

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
    ra_fp_pair_t *framePointer;
    ra_fp_pair_t stackWalkFramePair;

    bool found_reg = false;

    ra_fp_pair_t *sp;
    GET_STACK_POINTER(sp);

    framePointer = (ra_fp_pair_t *) sp;
    if (!framePointer) return false;
    if (!framePointer->FP) return false;

    stackWalkFramePair = *((ra_fp_pair_t*)(framePointer->FP));

    if (reg.isStackPointer() || reg == Dyninst::StackTop) {
        val = (Dyninst::MachRegisterVal) ((ra_fp_pair_t*)framePointer->FP)->FP;
        if (val != 0) found_reg = true;
    }

    if (reg.isFramePointer()) {
        val = (Dyninst::MachRegisterVal) ((ra_fp_pair_t*)framePointer->FP)->FP;
        if (val != 0) found_reg = true;
    }

    if (reg.isPC() || reg == Dyninst::ReturnAddr) {
        val = (Dyninst::MachRegisterVal) stackWalkFramePair.LR;
        if (val != 0) found_reg = true;
    }

    sw_printf("[%s:%d] - Returning value %lx for reg %s\n",
              FILE__, __LINE__, val, reg.name().c_str());
    return found_reg;
}

Dyninst::Architecture ProcSelf::getArchitecture()
{
    return Arch_loongarch64;
}

FrameFuncStepperImpl::~FrameFuncStepperImpl() {
}

unsigned FrameFuncStepperImpl::getPriority() const {
    return 0x10;
}


FrameFuncStepperImpl::FrameFuncStepperImpl(Walker *w, FrameStepper *parent_,
                                           FrameFuncHelper *helper_) :
    FrameStepper(w),
    parent(parent_),
    helper(helper_)
{
    helper = helper_ ? helper_ : loongarch64_LookupFuncStart::getLookupFuncStart(getProcessState());
}

bool DebugStepperImpl::lookupInCache(const Frame &, Frame &)
{
    return false;
}

gcframe_ret_t DebugStepperImpl::getCallerFrameArch(Address pc, const Frame &in,
                                                   Frame &out, DwarfDyninst::DwarfFrameParserPtr dinfo,
                                                   bool isVsyscallPage)
{
    assert(0 && "DebugStepperImpl::getCallerFrameArch not implemented for loongarch64");
    return gcf_error;
}

namespace Dyninst {
  namespace Stackwalker {

    void getTrapInstruction(char *buffer, unsigned buf_size,
                            unsigned &actual_len, bool include_return)
    {
        assert(buf_size >= 4);
        buffer[0] = 0x00;
        buffer[1] = 0x00;
        buffer[2] = 0x2a;
        buffer[3] = 0x00;
        actual_len = 4;
        if (include_return) {
            assert(buf_size >= 8);
            buffer[4] = 0x00;
            buffer[5] = 0x00;
            buffer[6] = 0x00;
            buffer[7] = 0x4c;
            actual_len = 8;
        }
    }

  }
}


gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
    Address in_fp, out_sp, out_ra;
    location_t fp_loc, sp_loc, ra_loc;
    bool result;
    unsigned addrWidth = getProcessState()->getAddressWidth();

    ra_fp_pair_t this_frame_pair;

    if (!in.getFP())
        return gcf_not_me;

    FrameFuncHelper::alloc_frame_t alloc_frame;
    alloc_frame = helper->allocatesFrame(in.getRA() - 1);
    if (alloc_frame.first != FrameFuncHelper::standard_frame) {
        if (in.getPrevFrame() != NULL)
            return gcf_not_me;
    }

    if (!in.getFP())
        return gcf_stackbottom;

    in_fp = in.getFP();
    out_sp = in_fp;
    out.setSP(out_sp);

    if (sizeof(uint64_t) == addrWidth) {
        result = getProcessState()->readMem(&this_frame_pair, in_fp, sizeof(this_frame_pair));
    } else {
        assert(0);
    }

    if (!result) {
        sw_printf("[%s:%d] - Couldn't read from %lx\n", FILE__, __LINE__, in_fp);
        return gcf_error;
    }

    out_ra = this_frame_pair.LR;
    out.setRA(out_ra);

    fp_loc.location = loc_address;
    fp_loc.val.addr = in_fp;
    out.setFPLocation(fp_loc);

    sp_loc.location = loc_address;
    sp_loc.val.addr = in_fp;
    out.setSPLocation(sp_loc);

    ra_loc.location = loc_address;
    ra_loc.val.addr = in_fp + sizeof(unsigned long);
    out.setRALocation(ra_loc);

    return gcf_success;
}

gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
    bool result;
    int addr_size = 8;

    location_t sp_loc;
    sp_loc.location = loc_address;
    sp_loc.val.addr = in.getFP() + sp_offset - sizeof(dummy_context);

    Address sp = 0;
    result = getProcessState()->readMem(&sp, sp_loc.val.addr, addr_size);
    if (!result) {
        sw_printf("[%s:%d] Error reading SP from stack memory 0x%lx\n", FILE__, __LINE__, sp_loc.val.addr);
        return gcf_error;
    }

    location_t fp_loc;
    Address fp = 0x0;
    fp_loc.location = loc_address;
    fp_loc.val.addr = in.getFP() + fp_offset - sizeof(dummy_context);
    result = getProcessState()->readMem(&fp, fp_loc.val.addr, addr_size);
    if (!result) {
        sw_printf("[%s:%d] Error reading FP from stack memory 0x%lx\n", FILE__, __LINE__, fp_loc.val.addr);
        return gcf_error;
    }

    location_t pc_loc;
    Address pc = 0x0;
    pc_loc.location = loc_address;
    pc_loc.val.addr = in.getFP() + pc_offset - sizeof(dummy_context);
    result = getProcessState()->readMem(&pc, pc_loc.val.addr, addr_size);
    if (!result) {
        sw_printf("[%s:%d] Error reading PC from stack memory 0x%lx\n", FILE__, __LINE__, pc_loc.val.addr);
        return gcf_error;
    }

    out.setRA((Dyninst::MachRegisterVal) pc);
    out.setFP((Dyninst::MachRegisterVal) fp);
    out.setSP((Dyninst::MachRegisterVal) sp);
    out.setRALocation(pc_loc);
    out.setFPLocation(fp_loc);
    out.setSPLocation(sp_loc);
    out.setNonCall();
    return gcf_success;
}
using namespace Dyninst::Stackwalker;

WandererHelper::WandererHelper(ProcessState *proc_) : proc(proc_) {}

WandererHelper::~WandererHelper() {}

bool WandererHelper::isPrevInstrACall(Address, Address &) {
    return false;
}

WandererHelper::pc_state WandererHelper::isPCInFunc(Address, Address) {
    return unknown_s;
}

bool WandererHelper::requireExactMatch() {
    return true;
}
