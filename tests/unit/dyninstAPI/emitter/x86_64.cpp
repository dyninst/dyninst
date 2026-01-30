#include "Architecture.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/emit-x86.h"
#include "registerSpace.h"
#include "emitter_test.h"

int main() {
  using Dyninst::verify_emitter;
  using Dyninst::emitter_buffer_t;

  constexpr auto arch = Dyninst::Arch_x86_64;
  auto const size = Dyninst::getArchAddressWidth(arch);
  auto *rs = registerSpace::getRegisterSpace(size);

  BinaryEdit bin_edit{};

  // The largest code sequence I've seen is 23 bytes, but there's no
  // technical limitation on how much code could be generated for any
  // operation. 128 bytes should be plenty.
  constexpr auto buffer_size = 128;

  codeGen gen(buffer_size);
  gen.setAddrSpace(&bin_edit);
  gen.setRegisterSpace(rs);

  auto *emitter = static_cast<EmitterAMD64Stat*>(gen.emitter());

  bool failed = false;

  // Move value from rbp to rax
  emitter->emitMoveRegToReg(REGNUM_EBP, REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<3>{{
    0x48, 0x8b, 0xc5  // mov rax, rbp
  }});

  // Load immediate into rax (of size 64 bits; "movabs")
  emitter->emitLoadConst(REGNUM_EAX, 0x12345678, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<10>{{
    0x48, 0xb8, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0  // mov rax, 0x12345678
  }});

  // Store an immediate value at a fixed address
  emitter->emitStoreImm(0x12345678, 0x9abcdef, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<11>{{
    // mov dword ptr [0x12345678], 0x9abcdef
    0xc7, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab, 0x09
  }});

  // Load value from fixed address into rax (tmp is assumed to be r15)
  emitter->emitLoad(REGNUM_EAX, 0x12345678, 8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<13>{{
    0x49, 0xbf, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0, // mov tmp, 0x12345678
    0x49, 0x8b, 0x07                                        // mov rax, qword ptr [tmp]
  }});

  // Store at a fixed address the value contained in rax (tmp is assumed to be r15)
  emitter->emitStore(0x12345678, REGNUM_EAX, 0x8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<13>{{
    0x49, 0xbf, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0,   // mov tmp, 0x12345678
    0x49, 0x89, 0x07                                          // mov qword ptr [tmp], rax
  }});

  // Load value into rax using register-indirect addressing w/o displacement
  emitter->emitLoadIndir(REGNUM_EAX, REGNUM_ESI, 0x4, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<2>{{
    0x8b, 0x06    // mov eax, dword ptr [rsi]
  }});

  // Store value contained in rax using register-indirect addressing w/o displacement
  emitter->emitStoreIndir(REGNUM_EBP, REGNUM_EAX, 0x8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x48, 0x89, 0x45, 0x0   // mov qword ptr [rbp], rax
  }});

  // Load value into rax using register-indirect addressing with displacement
  emitter->emitLoadRelative(REGNUM_EAX, 0x12345678, REGNUM_EBP, 0x8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0x48, 0x8b, 0x85, 0x78, 0x56, 0x34, 0x12   // mov rax, qword ptr [rbp+0x12345678]
  }});

  // Store value contained in rax using register-indirect addressing with displacement
  emitter->emitStoreRelative(REGNUM_EAX, 0x12345678, REGNUM_EBP, 0x8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0x48, 0x89, 0x85, 0x78, 0x56, 0x34, 0x12   // mov qword ptr [rbp+0x12345678], rax
  }});

  // Load value into rax using direct addressing with with segment override
  emitter->emitLoadRelativeSegReg(REGNUM_EAX, 0x12345678, REGNUM_GS, 0x8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<9>{{
    0x65, 0x49, 0x8b, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12   // mov rax, qword ptr gs:[0x12345678]
  }});

  // Multiply rax by 3
  emitter->emitTimesImm(REGNUM_EAX, REGNUM_EAX, 0x03, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0x48, 0x69, 0xc0, 0x03, 0x0, 0x0, 0x0   // imul rax, rax, 3
  }});

  //  Divide rdx by rcx
  emitter->emitDiv(REGNUM_EAX, REGNUM_EDX, REGNUM_ECX, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x48, 0x8b, 0xc2,   // mov rax, rdx
    0x48, 0x99,         // cqo  # sign-extend rax
    0x48, 0xf7, 0xf1    // div rcx
  }});

  // Divide rsi by 0x12345678, store results in rdx
  emitter->emitDivImm(REGNUM_EDX, REGNUM_ESI, 0x12345678, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<29>{{
    0x48, 0x8b, 0xc6,                                     // mov rax, rsi
    0x48, 0xba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,   // mov rdx, 0x0
    0x68, 0x78, 0x56, 0x34, 0x12,                         // push 0x12345678
    0x48, 0xf7, 0x34, 0x24,                               // div qword ptr [rsp]
    0x48, 0x8b, 0xd0,                                     // mov rdx, rax
    0x48, 0x83, 0xc4, 0x08                                // add rsp, 8
    }});

  // pushfq
  emitter->emitPushFlags(gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x9c}});

  // push rax
  emitter->emitPush(gen, REGNUM_EAX);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x50}});

  // pop rax
  emitter->emitPop(gen, REGNUM_EAX);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x58}});

  // Restore flags that are located at 'rbp + (4 * 8)`
  emitter->emitRestoreFlags(gen, 0x4);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0xff, 0x75, 0x20, // push	qword ptr [rbp + 0x20]
    0x9d              // popfq
  }});

  // Pop eflags from stack (popqf)
  emitter->emitRestoreFlags(gen, 0x0);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x9d}});

  //  Move stack pointer by 0x20 and align it to AMD64_STACK_ALIGNMENT
  emitter->emitStackAlign(0x20, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<48>{{
    0x48, 0x8d, 0x64, 0x24, 0xb0,       // lea	rsp, [rsp - 0x50]
    0x48, 0x89, 0x44, 0x24, 0x20,       // mov	qword ptr [rsp + 0x20], rax
    0x9f,                               // lahf
    0x0f, 0x90, 0xc0,                   // seto	al
    0x48, 0x89, 0x44, 0x24, 0x28,       // mov	qword ptr [rsp + 0x28], rax
    0x48, 0x8d, 0x44, 0x24, 0x50,       // lea	rax, [rsp + 0x50]
    0x48, 0x83, 0xe4, 0xe0,             // and	rsp, 0xffffffffffffffe0
    0x48, 0x89, 0x04, 0x24,             // mov	qword ptr [rsp], rax
    0x48, 0x8b, 0x40, 0xd8,             // mov	rax, qword ptr [rax - 0x28]
    0x80, 0xc0, 0x7f,                   // add	al, 0x7f
    0x9e,                               // sahf
    0x48, 0x8b, 0x04, 0x24,             // mov	rax, qword ptr [rsp]
    0x48, 0x8b, 0x40, 0xd0,             // mov	rax, qword ptr [rax - 0x30]
  }});

  // Restore rflags from stack
  emitter->emitRestoreFlagsFromStackSlot(gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0xff, 0x75, 0x0, 0x9d   // push qword ptr [rbp]
  }});

  // Restore rflags from stack at offset 0x1234
  emitter->emitRestoreFlags(gen, 0x1234);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0xff, 0xb5, 0xa0, 0x91, 0x0, 0x0, 0x9d  // push qword ptr [rbp+0x91A0]
  }});

  //  lea rax, qword ptr [rsi+rbx*4+0x12345678]
  emitter->emitLEA(REGNUM_ESI, REGNUM_EBX, 0X2, 0X12345678, REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{0x48, 0x8d, 0x84, 0x9e, 0x78, 0x56, 0x34, 0x12}});

  // xor rax, rcx
  emitter->emitXorRegReg(REGNUM_EAX, REGNUM_ECX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<3>{{0x48, 0x33, 0xc1}});

  // xor rax, qword ptr [rcx+0x12]
  emitter->emitXorRegRM(REGNUM_EAX, REGNUM_ECX, 0x12, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{0x48, 0x33, 0x41, 0x12}});

  // xor eax, 0x12345678   (Why no REX?)
  emitter->emitXorRegImm(REGNUM_EAX, 0x12345678, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<6>{{0x81, 0xf0, 0x78, 0x56, 0x34, 0x12}});

  // xor rax, qword ptr gs:[0x0000000012345678]
  emitter->emitXorRegSegReg(REGNUM_EAX, REGNUM_GS, 0x12345678, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<9>{{0x65, 0x49, 0x33, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12}});

  // add rsp, 0x91A0
  emitter->emitAdjustStackPointer(0x1234, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{0x48, 0x81, 0xc4, 0xa0, 0x91, 0x0, 0x0}});

  // add qword ptr [0x12345678], 0x04
  emitter->emitAddSignedImm(0x12345678, 0x4, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<9>{{0x48, 0x83, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0x04}});

  // Load BPatch_countSpec_NP (I don't actually know what this is supposed to do)
  emitter->emitCSload(-1, -1, 4, 0x1234, REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<10>{{
    0x48, 0xb8, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0   // mov rax, 0x1234
  }});

  // Load BPatch_addrSpec_NP (I don't actually know what this is supposed to do)
  emitter->emitASload(-1, -1, 4, 0x1234, REGNUM_EAX, 0x0, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<5>{{
    0xb8, 0x34, 0x12, 0x0, 0x0   // mov eax, 0x1234
  }});

  // Load return value into ABI return register (rax)
  emitter->emitGetRetVal(REGNUM_ESI, false, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<3>{{
    0x48, 0x8b, 0xf0    // mov rsi, rax
  }});

  // Load return address from stack into rsi
  emitter->emitGetRetAddr(REGNUM_ESI, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x48, 0x8d, 0x75, 0x00    // lea rsi, qword ptr [rbp]
  }});

  // Add immediate to value in memory at [rax]
  emitAddRM64(REGNUM_EAX, 0x12345678, true, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x48,  0x48, 0x81, 0x0, 0x78, 0x56, 0x34, 0x12,  // add qword ptr [rax], 0x12345678
  }});

  // Load immediate into memory location [rax + 0x20]
  emitMovImmToRM64(REGNUM_EAX, 0x20, 0x12345678, true, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0xc7, 0x40, 0x20, 0x78, 0x56, 0x34, 0x12  // mov dword ptr [rax + 0x20], 0x12345678
  }});

  // Load address 0x20 bytes ahead of current address
  emitMovPCRMToReg64(REGNUM_EAX, 0x20, 0x8, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0x48, 0x8d, 0x05, 0x19, 0x0, 0x0, 0x0  // lea rax, [rip + 0x19]
  }});

  // Load value at the address 0x20 bytes ahead of current address
  emitMovPCRMToReg64(REGNUM_EAX, 0x20, 0x8, gen, true);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0x48, 0x8b, 0x05, 0x19, 0x0, 0x0, 0x0  // mov rax, qword ptr [rip + 0x19]
  }});

  // Load rsi into rax
  emitMovRegToReg64(REGNUM_EAX, REGNUM_ESI, true, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<3>{{
    0x48, 0x8b, 0xc6  // mov rax, rsi
  }});

  // Load value at top of stack into rax
  emitPopReg64(REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{
    0x58  // pop rax
  }});

  // Store rax on stack
  emitPushReg64(REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{
    0x50  // push rax
  }});

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*

These require a valid baseTramp (codeGen::bt())

  bool emitBTRestores(baseTramp* bt, codeGen &gen);
  bool emitBTSaves(baseTramp* bt, codeGen &gen);
  void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
  void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
  void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
  void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);

These require additional mocking to test.

  bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
  Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee);
  bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
  void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
  bool emitPLTCall(func_instance *dest, codeGen &gen);
  bool emitPLTJump(func_instance *dest, codeGen &gen);
  void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local,int size, codeGen &gen, Address offset);
  void emitStoreShared(Register source, const image_variable *var, bool is_local,int size, codeGen &gen);
  codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);

These need to expand over many 'opcode' values

  void emitOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen);
  void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm, codeGen &gen);
  void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm, bool is_64, codeGen &gen);
  void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
  void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);

These are not implemented

  bool emitCallRelative(Register, Address, Register, codeGen &) {assert (0); return false; }

*/
