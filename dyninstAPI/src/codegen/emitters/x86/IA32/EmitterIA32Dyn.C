#include "arch-regs-x86.h"
#include "EmitterIA32Dyn.h"
#include "function.h"
#include "inst-x86.h"
#include "registerSpace/RealRegister.h"
#include "registerSpace/registerSpace.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterIA32Dyn::emitCallInstruction(codeGen &gen, func_instance *callee, Register ret) {
    // make the call
    // we are using an indirect call here because we don't know the
    // address of this instruction, so we can't use a relative call.
    // The only direct, absolute calls available on x86 are far calls,
    // which require the callee to be aware that they're being far-called.
    // So we grit our teeth and deal with the indirect call.
    // Physical register
    // Same regs on Win/x86 and linux/x86
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen); // caller saved regs
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ECX), gen);
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDX), gen);

    Register placeholder1 = gen.rs()->allocateRegister(gen);
    Register placeholder2 = gen.rs()->allocateRegister(gen);
    gen.rs()->noteVirtualInReal(ret, RealRegister(REGNUM_EAX));
    gen.rs()->noteVirtualInReal(placeholder1, RealRegister(REGNUM_ECX));
    gen.rs()->noteVirtualInReal(placeholder2, RealRegister(REGNUM_EDX));

    if(gen.startAddr() == (Address)-1) {
      emitMovImmToReg(RealRegister(REGNUM_EAX), callee->addr(), gen);
      emitOpExtReg(CALL_RM_OPC1, CALL_RM_OPC2, RealRegister(REGNUM_EAX), gen);
    } else {
      Address dest = callee->addr();
      Address src = gen.currAddr() + 5;
      emitCallRel32(dest - src, gen);
    }

    gen.rs()->freeRegister(placeholder1);
    gen.rs()->freeRegister(placeholder2);
    return true;
  }

}}
