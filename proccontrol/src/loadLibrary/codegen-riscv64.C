// RISC-V Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "codegen.h"
#include "Symbol.h"
#include "PCProcess.h"

#include <climits>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

// U-type: LUI
#define RV_UTYPE(imm20, rd, op)  \
    (((unsigned int)((imm20) & 0xfffff) << 12) | (((rd) & 0x1f) << 7) | ((op) & 0x7f))

// I-type: ADDI / JALR / etc.
#define RV_ITYPE(imm12, rs1, funct3, rd, op)  \
    (((unsigned int)((imm12) & 0xfff) << 20) | (((rs1) & 0x1f) << 15) | \
     (((funct3) & 0x7) << 12) | (((rd) & 0x1f) << 7) | ((op) & 0x7f))

// RISC-V opcodes
#define RV_OP_LUI   0x37
#define RV_OP_IMM   0x13
#define RV_OP_JALR  0x67

// RISC-V funct3
#define RV_F3_ADDI  0x0
#define RV_F3_SLLI  0x1
#define RV_F3_JALR  0x0

// RISC-V registers (calling convention)
#define RV_REG_ZERO  0
#define RV_REG_RA    1
#define RV_REG_SP    2
#define RV_REG_T0    5   // temporary
#define RV_REG_A0   10   // first argument

static inline unsigned int rvLui(unsigned rd, unsigned imm20) {
    return RV_UTYPE(imm20, rd, RV_OP_LUI);
}

static inline unsigned int rvAddi(unsigned rd, unsigned rs, unsigned imm12) {
    return RV_ITYPE(imm12, rs, RV_F3_ADDI, rd, RV_OP_IMM);
}

static inline unsigned int rvSlli(unsigned rd, unsigned rs, unsigned shamt) {
    return RV_ITYPE(shamt, rs, RV_F3_SLLI, rd, RV_OP_IMM);
}

static inline unsigned int rvJalr(unsigned rd, unsigned rs, unsigned offset) {
    return RV_ITYPE(offset, rs, RV_F3_JALR, rd, RV_OP_JALR);
}

// Generate the instruction sequence to load a 64-bit immediate into register rd.
// In case the value fits in 12 bits, we can use a single ADDI.  If it fits in 32 bits,
// we can use LUI + ADDI.  Otherwise, we need multiple instructions to construct the full 64-bit value.
void Codegen::generateRISCV64(Address val, unsigned reg) {
    unsigned int insn;
    unsigned long a;

    // Case 1: fits in 12-bit signed immediate
    if ((long)val >= -2048 && (long)val < 2048) {
        insn = rvAddi(reg, RV_REG_ZERO, val & 0xfff);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # addi r%u, zero, 0x%lx\n", a, insn, reg, (unsigned long)val);
        return;
    }

    // Case 2: fits in 32-bit signed immediate (lui + addi)
    if ((long)val >= (long)INT_MIN && (long)val <= (long)INT_MAX) {
        unsigned lui_imm = (val >> 12) & 0xfffff;
        unsigned addi_imm = val & 0xfff;
        // Adjust for sign extension of addi
        if (addi_imm & 0x800)
            lui_imm = (lui_imm + 1) & 0xfffff;

        insn = rvLui(reg, lui_imm);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # lui r%u\n", a, insn, reg);

        insn = rvAddi(reg, reg, addi_imm);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # addi r%u\n", a, insn, reg);
        return;
    }

    // Case 3: full 64-bit immediate
    unsigned lui_imm = (val >> 44) & 0xfffff;
    unsigned addi_imm0 = (val >> 32) & 0xfff;
    unsigned addi_imm1 = (val >> 21) & 0x7ff;
    unsigned addi_imm2 = (val >> 10) & 0x7ff;
    unsigned addi_imm3 = val & 0x3ff;

    if (addi_imm0 & 0x800)
        lui_imm = (lui_imm + 1) & 0xfffff;

    insn = rvLui(reg, lui_imm);
    a = copyInt(insn);
    pthrd_printf("0x%8lx: 0x%8x  # lui r%u\n", a, insn, reg);

    if (addi_imm0 != 0) {
        insn = rvAddi(reg, reg, addi_imm0);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # addi r%u\n", a, insn, reg);
    }

    insn = rvSlli(reg, reg, 11);
    a = copyInt(insn);
    pthrd_printf("0x%8lx: 0x%8x  # slli r%u, 11\n", a, insn, reg);

    if (addi_imm1 != 0) {
        insn = rvAddi(reg, reg, addi_imm1);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # addi r%u\n", a, insn, reg);
    }

    insn = rvSlli(reg, reg, 11);
    a = copyInt(insn);
    pthrd_printf("0x%8lx: 0x%8x  # slli r%u, 11\n", a, insn, reg);

    if (addi_imm2 != 0) {
        insn = rvAddi(reg, reg, addi_imm2);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # addi r%u\n", a, insn, reg);
    }

    insn = rvSlli(reg, reg, 10);
    a = copyInt(insn);
    pthrd_printf("0x%8lx: 0x%8x  # slli r%u, 10\n", a, insn, reg);

    if (addi_imm3 != 0) {
        insn = rvAddi(reg, reg, addi_imm3);
        a = copyInt(insn);
        pthrd_printf("0x%8lx: 0x%8x  # addi r%u\n", a, insn, reg);
    }
}


bool Codegen::generatePreambleRISCV64(){
    // addi sp, sp, -48
    // Encoding: addi x2, x2, -48
    // -48 & 0xfff = 0xfd0
    unsigned int insn = rvAddi(RV_REG_SP, RV_REG_SP, 0xfd0);
    unsigned long addr = copyInt(insn);
    pthrd_printf("generate Preamble:\n");
    pthrd_printf("0x%8lx: 0x%8x  # addi sp, sp, -48\n", addr, insn);
    return true;
}

bool Codegen::generateCallRISCV64(Address addr, const std::vector<Address> &args){
    if( args.size() > 8)
        return false;

    pthrd_printf("generate Call riscv64:\n");

    unsigned int regIndex = 0;
    for(std::vector<Address>::const_iterator itA = args.begin();
            itA != args.end(); itA++){
        assert( regIndex < 8 );
        unsigned long val = *itA;
        generateRISCV64(val, RV_REG_A0 + regIndex);
        regIndex++;
    }

    generateRISCV64(addr, RV_REG_T0);

    // jalr ra, t0, 0  — call through t0
    unsigned int jalrInsn = rvJalr(RV_REG_RA, RV_REG_T0, 0);
    unsigned long retAddr = copyInt(jalrInsn);
    pthrd_printf("0x%8lx: 0x%8x  # jalr ra, t0, 0\n", retAddr, jalrInsn);

    return true;
}
