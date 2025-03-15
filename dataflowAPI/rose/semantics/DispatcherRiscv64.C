#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "BaseSemantics2.h"
//#include "Diagnostics.h"
#include "DispatcherRiscv64.h"
#include "../integerOps.h"
#include "SymEvalSemantics.h"

#include "../SgAsmExpression.h"
#include "../conversions.h"

#undef si_value                                         // name pollution from siginfo.h

// RISC-V is not supported by ROSE
// Instead, we're using SAIL (https://github.com/riscv/sail-riscv) for instruction semantics

namespace rose {
    namespace BinaryAnalysis {
        namespace InstructionSemantics2 {

#define XLENBITS 64

#define EXTR(lo, hi)    IntegerOps::extract2<B>(lo, hi, raw)

#define PC ops->number_(64, insn->get_address())

/*******************************************************************************************************************************
 *                                      Support functions
 *******************************************************************************************************************************/

            static inline size_t asm_type_width(SgAsmType *ty) {
                ASSERT_not_null(ty);
                return ty->get_nBits();
            }

/*******************************************************************************************************************************
 *                                      Base Riscv64 instruction processor
 *******************************************************************************************************************************/
            namespace Riscv64 {

                void
                InsnProcessor::process(const BaseSemantics::DispatcherPtr &dispatcher_, SgAsmInstruction *insn_) {
                    DispatcherRiscv64Ptr dispatcher = DispatcherRiscv64::promote(dispatcher_);
                    BaseSemantics::RiscOperatorsPtr operators = dispatcher->get_operators();
                    SgAsmRiscv64Instruction *insn = isSgAsmRiscv64Instruction(insn_);
                    ASSERT_require(insn != NULL && insn == operators->currentInstruction());
                    dispatcher->advanceInstructionPointer(insn);
                    SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();
                    //check_arg_width(dispatcher.get(), insn, operands);

                    uint32_t raw = 0;
                    std::vector<unsigned char> rawBytes = insn->get_raw_bytes();
                    for (int idx = 0; idx < rawBytes.size(); idx++) {
                        raw |= (rawBytes[idx] << (8 * idx));
                    }
                    p(dispatcher.get(), operators.get(), insn, operands, raw);
                    
                    // Hardwire x0 to 0
                    SgAsmDirectRegisterExpression dre0{dispatcher->findRegister("x0", 64)};
                    dispatcher->writeRegister(dre0.get_descriptor(), operators->number_(64, 0));
                }

                void
                InsnProcessor::assert_args(I insn, A args, size_t nargs) {
                    if (args.size() != nargs) {
                        std::string mesg = "instruction has incorrect number of args";
                        throw BaseSemantics::Exception(mesg, insn);
                    }
                }

/*******************************************************************************************************************************
 *                                      Helper functions                                    
 *******************************************************************************************************************************/

                inline size_t
                lrsc_width_str(SgAsmRiscv64Instruction *insn) {
                    if (insn->get_mnemonic().find(".b") != std::string::npos) {
                        return 1;
                    }
                    else if (insn->get_mnemonic().find(".h") != std::string::npos) {
                        return 2;
                    }
                    else if (insn->get_mnemonic().find(".w") != std::string::npos) {
                        return 4;
                    }
                    else if (insn->get_mnemonic().find(".d") != std::string::npos) {
                        return 8;
                    }
                    else {
                        assert(0 && "Invalid RISC-V lrsc width");
                    }
                }

                inline size_t
                amo_signed_str(SgAsmRiscv64Instruction *insn) {
                    if (insn->get_mnemonic().find("u_") != std::string::npos) {
                        return 1;
                    }
                    else {
                        return 0;
                    }
                }

                inline bool
                is_c_ext(SgAsmRiscv64Instruction *insn) {
                    return insn->get_mnemonic().find(".c") != std::string::npos;
                }

                inline size_t
                get_next_pc(SgAsmRiscv64Instruction *insn) {
                    return insn->get_address() + insn->get_size();
                }


/*******************************************************************************************************************************
 *                                      Functors that handle individual Riscv64 instructions kinds
 *******************************************************************************************************************************/
                typedef InsnProcessor P;

                struct IP_UTYPE : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *imm = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr off = d->SignExtend(ops->shiftLeft(d->read(imm, 64, 0), ops->number_(64, 12)), 64);
                        BaseSemantics::SValuePtr ret;
                        switch (op) {
                            case rose_riscv64_op_lui:
                                ret = off;
                                break;
                            case rose_riscv64_op_auipc:
                                ret = ops->add(ops->number_(64, insn->get_address()), off);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->write(rd, ret);
                    }
                };

                struct IP_RISCV_JAL : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *imm = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr t = ops->add(PC, d->SignExtend(d->read(imm, 64, 0), 64));
                        BaseSemantics::SValuePtr target = t;
                        d->write(rd, (ops->number_(64, insn->get_address() + insn->get_size())));
                        d->BranchTo(target);
                    }
                };

                struct 
                IP_RISCV_JALR : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *imm = args[2];

                        BaseSemantics::SValuePtr t = ops->add(d->read(rs1, 64, 0), d->SignExtend(d->read(imm, 64, 0), 64));
                        d->write(rd, ops->number_(XLENBITS, get_next_pc(insn)));
                        d->BranchTo(t);
                    }
                };
                struct IP_BTYPE : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *imm = args[2];
                        SgAsmExpression *rs2 = args[1];
                        SgAsmExpression *rs1 = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);
                        BaseSemantics::SValuePtr taken;
                        switch (op) {
                            case rose_riscv64_op_beq:
                                taken = ops->isEqual(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_bne:
                                taken = ops->isNotEqual(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_blt:
                                taken = ops->isSignedLessThan(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_bge:
                                taken = ops->isSignedGreaterThanOrEqual(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_bltu:
                                taken = ops->isUnsignedLessThan(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_bgeu:
                                taken = ops->isUnsignedGreaterThanOrEqual(rs1_val, rs2_val);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        BaseSemantics::SValuePtr t = ops->add(PC, d->SignExtend(d->read(imm, 64, 0), 64));
                        BaseSemantics::SValuePtr target = ops->ite(taken, t, PC);
                        d->BranchTo(target);
                    }
                };

                struct IP_ITYPE : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *imm = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr immext = d->SignExtend(d->read(imm, 64, 0), 64);
                        BaseSemantics::SValuePtr result;
                        switch (op) {
                            case rose_riscv64_op_addi:
                                result = ops->add(rs1_val, immext);
                                break;
                            case rose_riscv64_op_slti:
                                result = d->ZeroExtend(ops->isSignedLessThan(rs1_val, immext), 64);
                                break;
                            case rose_riscv64_op_sltiu:
                                result = d->ZeroExtend(ops->isUnsignedLessThan(rs1_val, immext), 64);
                                break;
                            case rose_riscv64_op_andi:
                                result = ops->and_(rs1_val, immext);
                                break;
                            case rose_riscv64_op_ori:
                                result = ops->or_(rs1_val, immext);
                                break;
                            case rose_riscv64_op_xori:
                                result = ops->xor_(rs1_val, immext);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->write(rd, result);
                    }
                };

                struct IP_SHIFTIOP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *shamt = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr result;
                        switch (op) {
                            case rose_riscv64_op_slli:
                                result = ops->shiftLeft(rs1_val, d->read(shamt, 64, 0));
                                break;
                            case rose_riscv64_op_srli:
                                result = ops->shiftRight(rs1_val, d->read(shamt, 64, 0));
                                break;
                            case rose_riscv64_op_srai:
                                result = ops->shiftRightArithmetic(rs1_val, d->read(shamt, 64, 0));
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->write(rd, result);
                    }
                };

                struct IP_RTYPE : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rs2 = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);
                        BaseSemantics::SValuePtr result;
                        switch (op) {
                            case rose_riscv64_op_add:
                                result = ops->add(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_slt:
                                result = d->ZeroExtend(ops->isSignedLessThan(rs1_val, rs2_val), 64);
                                break;
                            case rose_riscv64_op_sltu:
                                result = d->ZeroExtend(ops->isUnsignedLessThan(rs1_val, rs2_val), 64);
                                break;
                            case rose_riscv64_op_and:
                                result = ops->and_(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_or:
                                result = ops->or_(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_xor:
                                result = ops->xor_(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_sll:
                                result = ops->shiftLeft(rs1_val, ops->extract(rs2_val, 0, 5));
                                break;
                            case rose_riscv64_op_srl:
                                result = ops->shiftRight(rs1_val, ops->extract(rs2_val, 0, 5));
                                break;
                            case rose_riscv64_op_sub:
                                result = ops->subtract(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_sra:
                                result = ops->shiftRightArithmetic(rs1_val, ops->extract(rs2_val, 0, 5));
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->write(rd, result);
                    }
                };

                struct IP_LOAD : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr read_addr = d->effectiveAddress(args[1]);
                        size_t width_bytes;
                        int is_unsigned;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_lb:
                                width_bytes = 1;
                                is_unsigned = 1;
                                break;
                            case rose_riscv64_op_lbu:
                                width_bytes = 1;
                                is_unsigned = 0;
                                break;
                            case rose_riscv64_op_lh:
                                width_bytes = 2;
                                is_unsigned = 1;
                                break;
                            case rose_riscv64_op_lhu:
                                width_bytes = 2;
                                is_unsigned = 0;
                                break;
                            case rose_riscv64_op_lw:
                                width_bytes = 4;
                                is_unsigned = 1;
                                break;
                            case rose_riscv64_op_lwu:
                                width_bytes = 4;
                                is_unsigned = 0;
                                break;
                            case rose_riscv64_op_ld:
                                width_bytes = 8;
                                is_unsigned = 1;
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }

                        BaseSemantics::SValuePtr read_data = d->readMemory(read_addr, width_bytes);

                        BaseSemantics::SValuePtr result = read_data;
                        d->write(rd, (is_unsigned ? d->SignExtend(result, 64) : d->ZeroExtend(result, 64)));
                    }
                };

                struct IP_STORE : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rs2 = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr write_addr = d->effectiveAddress(args[1]);
                        size_t width_bytes;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_sb:
                                width_bytes = 1;
                                break;
                            case rose_riscv64_op_sh:
                                width_bytes = 2;
                                break;
                            case rose_riscv64_op_sw:
                                width_bytes = 4;
                                break;
                            case rose_riscv64_op_sd:
                                width_bytes = 8;
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }

                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);
                        d->writeMemory(write_addr, width_bytes, rs2_val);
                    }
                };

                struct IP_ADDIW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *imm = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr result = ops->add(d->SignExtend(d->read(imm, 64, 0), 64), d->read(rs1, 64, 0));
                        d->write(rd, d->SignExtend(ops->extract(result, 0, 31), 64));
                    }
                };

                struct IP_SHIFTW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = ops->extract(d->read(rs1, 64, 0), 0, 31);
                        BaseSemantics::SValuePtr result;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_sllw:
                                result = ops->shiftLeft(rs1_val, d->read(rs2, 64, 0));
                                break;
                            case rose_riscv64_op_srlw:
                                result = ops->shiftRight(rs1_val, d->read(rs2, 64, 0));
                                break;
                            case rose_riscv64_op_sraw:
                                result = ops->shiftRightArithmetic(rs1_val, d->read(rs2, 64, 0));
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }
                        d->write(rd, d->SignExtend(result, XLENBITS));
                    }
                };

                struct IP_RTYPEW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rs2 = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr rs1_val = ops->extract(d->read(rs1, 64, 0), 0, 31);
                        BaseSemantics::SValuePtr rs2_val = ops->extract(d->read(rs2, 64, 0), 0, 31);
                        BaseSemantics::SValuePtr result;
                        switch (op) {
                            case rose_riscv64_op_addw:
                                result = ops->add(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_subw:
                                result = ops->subtract(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_sllw:
                                result = ops->shiftLeft(rs1_val, ops->extract(rs2_val, 0, 4));
                                break;
                            case rose_riscv64_op_srlw:
                                result = ops->shiftRight(rs1_val, ops->extract(rs2_val, 0, 4));
                                break;
                            case rose_riscv64_op_sraw:
                                result = ops->shiftRightArithmetic(rs1_val, ops->extract(rs2_val, 0, 4));
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->write(rd, d->SignExtend(result, 64));
                    }
                };

                struct IP_SHIFTIWOP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *shamt = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        enum Riscv64InstructionKind op = insn->get_kind();
                        BaseSemantics::SValuePtr rs1_val = ops->extract(d->read(rs1, 64, 0), 0, 31);
                        BaseSemantics::SValuePtr result;
                        switch (op) {
                            case rose_riscv64_op_slliw:
                                result = ops->shiftLeft(rs1_val, d->read(shamt, 64, 0));
                                break;
                            case rose_riscv64_op_srliw:
                                result = ops->shiftRight(rs1_val, d->read(shamt, 64, 0));
                                break;
                            case rose_riscv64_op_sraiw:
                                result = ops->shiftRightArithmetic(rs1_val, d->read(shamt, 64, 0));
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->write(rd, d->SignExtend(result, 64));
                    }
                };

                struct IP_MUL : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);

                        BaseSemantics::SValuePtr result;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_mul:
                                result = ops->extract(ops->signedMultiply(rs1_val, rs2_val), 0, 31);
                                break;
                            case rose_riscv64_op_mulh:
                                result = ops->extract(ops->signedMultiply(rs1_val, rs2_val), 32, 63);
                                break;
                            case rose_riscv64_op_mulhsu:
                                result = ops->extract(ops->signedUnsignedMultiply(rs1_val, rs2_val), 32, 63);
                                break;
                            case rose_riscv64_op_mulhu:
                                result = ops->extract(ops->unsignedMultiply(rs1_val, rs2_val), 32, 63);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }
                        d->write(rd, result);
                    }
                };

                struct IP_DIV : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);

                        BaseSemantics::SValuePtr result;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_div:
                                result = ops->signedDivide(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_divu:
                                result = ops->unsignedDivide(rs1_val, rs2_val);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }
                        d->write(rd, result);
                    }
                };

                struct IP_REM : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = d->read(rs1, 64, 0);
                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);

                        BaseSemantics::SValuePtr result;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_rem:
                                result = ops->signedModulo(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_remu:
                                result = ops->unsignedModulo(rs1_val, rs2_val);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }
                        d->write(rd, result);
                    }
                };

                struct IP_MULW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = ops->extract(d->read(rs1, 32, 0), 0, 31);
                        BaseSemantics::SValuePtr rs2_val = ops->extract(d->read(rs2, 32, 0), 0, 31);
                        BaseSemantics::SValuePtr result32 = ops->extract(ops->signedMultiply(rs1_val, rs2_val), 0, 31);
                        BaseSemantics::SValuePtr result = d->SignExtend(result32, XLENBITS);
                        d->write(rd, result);
                    }
                };

                struct IP_DIVW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = ops->extract(d->read(rs1, 32, 0), 0, 31);
                        BaseSemantics::SValuePtr rs2_val = ops->extract(d->read(rs2, 32, 0), 0, 31);
                        BaseSemantics::SValuePtr q;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_divw:
                                q = ops->signedDivide(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_divuw:
                                q = ops->unsignedDivide(rs1_val, rs2_val);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }
                        d->write(rd, d->SignExtend(q, XLENBITS));
                    }
                };

                struct IP_REMW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rd = args[0];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rs2 = args[2];

                        BaseSemantics::SValuePtr rs1_val = ops->extract(d->read(rs1, 32, 0), 0, 31);
                        BaseSemantics::SValuePtr rs2_val = ops->extract(d->read(rs2, 32, 0), 0, 31);
                        BaseSemantics::SValuePtr result;
                        switch (insn->get_kind()) {
                            case rose_riscv64_op_remw:
                                result = ops->signedModulo(rs1_val, rs2_val);
                                break;
                            case rose_riscv64_op_remuw:
                                result = ops->unsignedModulo(rs1_val, rs2_val);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        }
                        d->write(rd, d->SignExtend(result, XLENBITS));
                    }
                };

                struct IP_LOADRES : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        size_t width_bytes = lrsc_width_str(insn);
                        BaseSemantics::SValuePtr read_addr = d->effectiveAddress(rs1);
                        enum Riscv64InstructionKind op = insn->get_kind();

                        BaseSemantics::SValuePtr read_data = d->readMemory(read_addr, width_bytes);

                        BaseSemantics::SValuePtr result = read_data;

                        d->write(rd, d->SignExtend(result, 64));
                    }
                };

                struct IP_STORECON : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rs2 = args[2];
                        SgAsmExpression *rs1 = args[1];
                        SgAsmExpression *rd = args[0];
                        size_t width_bytes = lrsc_width_str(insn);
                        BaseSemantics::SValuePtr write_addr = d->effectiveAddress(rs1);
                        enum Riscv64InstructionKind op = insn->get_kind();

                        BaseSemantics::SValuePtr rs2_val = d->read(rs2, 64, 0);
                        d->writeMemory(write_addr, width_bytes, rs2_val);
                    }
                };

                struct IP_AMO : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpression *rs2 = args[1];
                        SgAsmExpression *rs1 = args[2];
                        SgAsmExpression *rd = args[0];
                        size_t width_bytes = lrsc_width_str(insn);
                        bool is_unsigned = amo_signed_str(insn);
                        BaseSemantics::SValuePtr addr = d->effectiveAddress(rs1);
                        BaseSemantics::SValuePtr read_addr = addr;
                        BaseSemantics::SValuePtr write_addr = addr;
                        enum Riscv64InstructionKind op = insn->get_kind();

                        BaseSemantics::SValuePtr rs2_val = ops->extract(d->read(rs2, 64, 0), 0, ((width_bytes * 8) - 1));
                        BaseSemantics::SValuePtr read_data = d->readMemory(read_addr, width_bytes);

                        BaseSemantics::SValuePtr loaded = read_data;
                        BaseSemantics::SValuePtr result;
                        switch (op) {
                            case rose_riscv64_op_amoswap_w:
                            case rose_riscv64_op_amoswap_w_aq:
                            case rose_riscv64_op_amoswap_w_aq_rl:
                            case rose_riscv64_op_amoswap_w_rl:
                            case rose_riscv64_op_amoswap_d:
                            case rose_riscv64_op_amoswap_d_aq:
                            case rose_riscv64_op_amoswap_d_aq_rl:
                            case rose_riscv64_op_amoswap_d_rl:
                                result = rs2_val;
                                break;
                            case rose_riscv64_op_amoadd_w:
                            case rose_riscv64_op_amoadd_w_aq:
                            case rose_riscv64_op_amoadd_w_aq_rl:
                            case rose_riscv64_op_amoadd_w_rl:
                            case rose_riscv64_op_amoadd_d:
                            case rose_riscv64_op_amoadd_d_aq:
                            case rose_riscv64_op_amoadd_d_aq_rl:
                            case rose_riscv64_op_amoadd_d_rl:
                                result = ops->add(rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amoxor_w:
                            case rose_riscv64_op_amoxor_w_aq:
                            case rose_riscv64_op_amoxor_w_aq_rl:
                            case rose_riscv64_op_amoxor_w_rl:
                            case rose_riscv64_op_amoxor_d:
                            case rose_riscv64_op_amoxor_d_aq:
                            case rose_riscv64_op_amoxor_d_aq_rl:
                            case rose_riscv64_op_amoxor_d_rl:
                                result = ops->xor_(rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amoand_w:
                            case rose_riscv64_op_amoand_w_aq:
                            case rose_riscv64_op_amoand_w_aq_rl:
                            case rose_riscv64_op_amoand_w_rl:
                            case rose_riscv64_op_amoand_d:
                            case rose_riscv64_op_amoand_d_aq:
                            case rose_riscv64_op_amoand_d_aq_rl:
                            case rose_riscv64_op_amoand_d_rl:
                                result = ops->and_(rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amoor_w:
                            case rose_riscv64_op_amoor_w_aq:
                            case rose_riscv64_op_amoor_w_aq_rl:
                            case rose_riscv64_op_amoor_w_rl:
                            case rose_riscv64_op_amoor_d:
                            case rose_riscv64_op_amoor_d_aq:
                            case rose_riscv64_op_amoor_d_aq_rl:
                            case rose_riscv64_op_amoor_d_rl:
                                result = ops->or_(rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amomin_w:
                            case rose_riscv64_op_amomin_w_aq:
                            case rose_riscv64_op_amomin_w_aq_rl:
                            case rose_riscv64_op_amomin_w_rl:
                            case rose_riscv64_op_amomin_d:
                            case rose_riscv64_op_amomin_d_aq:
                            case rose_riscv64_op_amomin_d_aq_rl:
                            case rose_riscv64_op_amomin_d_rl:
                                result = ops->ite(ops->isSignedLessThan(rs2_val, loaded), rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amomax_w:
                            case rose_riscv64_op_amomax_w_aq:
                            case rose_riscv64_op_amomax_w_aq_rl:
                            case rose_riscv64_op_amomax_w_rl:
                            case rose_riscv64_op_amomax_d:
                            case rose_riscv64_op_amomax_d_aq:
                            case rose_riscv64_op_amomax_d_aq_rl:
                            case rose_riscv64_op_amomax_d_rl:
                                result = ops->ite(ops->isSignedGreaterThan(rs2_val, loaded), rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amominu_w:
                            case rose_riscv64_op_amominu_w_aq:
                            case rose_riscv64_op_amominu_w_aq_rl:
                            case rose_riscv64_op_amominu_w_rl:
                            case rose_riscv64_op_amominu_d:
                            case rose_riscv64_op_amominu_d_aq:
                            case rose_riscv64_op_amominu_d_aq_rl:
                            case rose_riscv64_op_amominu_d_rl:
                                result = ops->ite(ops->isUnsignedLessThan(rs2_val, loaded), rs2_val, loaded);
                                break;
                            case rose_riscv64_op_amomaxu_w:
                            case rose_riscv64_op_amomaxu_w_aq:
                            case rose_riscv64_op_amomaxu_w_aq_rl:
                            case rose_riscv64_op_amomaxu_w_rl:
                            case rose_riscv64_op_amomaxu_d:
                            case rose_riscv64_op_amomaxu_d_aq:
                            case rose_riscv64_op_amomaxu_d_aq_rl:
                            case rose_riscv64_op_amomaxu_d_rl:
                                result = ops->ite(ops->isUnsignedGreaterThan(rs2_val, loaded), rs2_val, loaded);
                                break;
                            default:
                                assert(0 && "Invalid RISC-V instruction kind");
                        };
                        d->writeMemory(write_addr, width_bytes, rs2_val);
                    }
                };
                struct IP_C_NOP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{};
                    }
                };

                struct IP_C_ADDI4SPN : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1], args[2]};
                        SgAsmRiscv64Instruction new_insn{0, "addi", rose_riscv64_op_addi};
                        auto conv = Riscv64::IP_ITYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_LW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "lw", rose_riscv64_op_lw};
                        auto conv = Riscv64::IP_LOAD{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_LD : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "ld", rose_riscv64_op_ld};
                        auto conv = Riscv64::IP_LOAD{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "sw", rose_riscv64_op_sw};
                        auto conv = Riscv64::IP_STORE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SD : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "sd", rose_riscv64_op_sd};
                        auto conv = Riscv64::IP_STORE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_ADDI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "addi", rose_riscv64_op_addi};
                        auto conv = Riscv64::IP_ITYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_JAL : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "jal", rose_riscv64_op_jal};
                        auto conv = Riscv64::IP_RISCV_JAL{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_ADDIW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "addiw", rose_riscv64_op_addiw};
                        auto conv = Riscv64::IP_ADDIW{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_LI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmDirectRegisterExpression dre0{d->findRegister("x0", 64)};
                        SgAsmExpressionPtrList new_args{args[0], &dre0, args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "addi", rose_riscv64_op_addi};
                        auto conv = Riscv64::IP_ITYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_ADDI16SP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmDirectRegisterExpression dre2{d->findRegister("x2", 64)};
                        SgAsmExpressionPtrList new_args{&dre2, &dre2, args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "addi", rose_riscv64_op_addi};
                        auto conv = Riscv64::IP_ITYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_LUI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "lui", rose_riscv64_op_lui};
                        auto conv = Riscv64::IP_UTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SRLI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "srli", rose_riscv64_op_srli};
                        auto conv = Riscv64::IP_SHIFTIOP{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SRAI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "srai", rose_riscv64_op_srai};
                        auto conv = Riscv64::IP_SHIFTIOP{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_ANDI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "andi", rose_riscv64_op_andi};
                        auto conv = Riscv64::IP_ITYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SUB : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "sub", rose_riscv64_op_sub};
                        auto conv = Riscv64::IP_RTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_XOR : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "xor", rose_riscv64_op_xor};
                        auto conv = Riscv64::IP_RTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_OR : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "or", rose_riscv64_op_or};
                        auto conv = Riscv64::IP_RTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_AND : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "and", rose_riscv64_op_and};
                        auto conv = Riscv64::IP_RTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SUBW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "subw", rose_riscv64_op_subw};
                        auto conv = Riscv64::IP_RTYPEW{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_ADDW : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "addw", rose_riscv64_op_addw};
                        auto conv = Riscv64::IP_RTYPEW{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_J : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "jal", rose_riscv64_op_jal};
                        auto conv = Riscv64::IP_RISCV_JAL{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_BEQZ : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmDirectRegisterExpression dre0{d->findRegister("x0", 64)};
                        SgAsmExpressionPtrList new_args{args[0], &dre0, args[1]};

                        SgAsmRiscv64Instruction new_insn{0, "beq", rose_riscv64_op_beq};
                        auto conv = Riscv64::IP_BTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_BNEZ : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmDirectRegisterExpression dre0{d->findRegister("x0", 64)};
                        SgAsmExpressionPtrList new_args{args[0], &dre0, args[1]};

                        SgAsmRiscv64Instruction new_insn{0, "bne", rose_riscv64_op_bne};
                        auto conv = Riscv64::IP_BTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SLLI : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "slli", rose_riscv64_op_slli};
                        auto conv = Riscv64::IP_SHIFTIOP{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_LWSP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "lw", rose_riscv64_op_lw};
                        auto conv = Riscv64::IP_LOAD{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_LDSP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "ld", rose_riscv64_op_ld};
                        auto conv = Riscv64::IP_LOAD{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SWSP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "sw", rose_riscv64_op_sw};
                        auto conv = Riscv64::IP_STORE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_SDSP : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "sd", rose_riscv64_op_sd};
                        auto conv = Riscv64::IP_STORE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_JR : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1], args[2]};
                        SgAsmRiscv64Instruction new_insn{0, "jalr", rose_riscv64_op_jalr};
                        auto conv = Riscv64::IP_RISCV_JALR{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_JALR : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[1], args[2]};
                        SgAsmRiscv64Instruction new_insn{0, "jalr", rose_riscv64_op_jalr};
                        auto conv = Riscv64::IP_RISCV_JALR{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_MV : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmDirectRegisterExpression dre0{d->findRegister("x0", 64)};
                        SgAsmExpressionPtrList new_args{args[0], &dre0, args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "add", rose_riscv64_op_add};
                        auto conv = Riscv64::IP_RTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };

                struct IP_C_ADD : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        SgAsmExpressionPtrList new_args{args[0], args[0], args[1]};
                        SgAsmRiscv64Instruction new_insn{0, "add", rose_riscv64_op_add};
                        auto conv = Riscv64::IP_RTYPE{};
                        conv.p(d, ops, &new_insn, new_args, raw);
                    }
                };
            } // namespace

/*******************************************************************************************************************************
 *                                      DispatcherRiscv64
 *******************************************************************************************************************************/

            void
            DispatcherRiscv64::iproc_init() {
                iproc_set(rose_riscv64_op_add, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_addi, new Riscv64::IP_ITYPE);
                iproc_set(rose_riscv64_op_addiw, new Riscv64::IP_ADDIW);
                iproc_set(rose_riscv64_op_addw, new Riscv64::IP_RTYPEW);
                iproc_set(rose_riscv64_op_amoadd_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoadd_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoand_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomax_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomaxu_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amomin_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amominu_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoor_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoswap_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_d, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_d_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_d_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_d_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_w, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_w_aq, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_w_aq_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_amoxor_w_rl, new Riscv64::IP_AMO);
                iproc_set(rose_riscv64_op_and, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_andi, new Riscv64::IP_ITYPE);
                iproc_set(rose_riscv64_op_auipc, new Riscv64::IP_UTYPE);
                iproc_set(rose_riscv64_op_beq, new Riscv64::IP_BTYPE);
                iproc_set(rose_riscv64_op_bge, new Riscv64::IP_BTYPE);
                iproc_set(rose_riscv64_op_bgeu, new Riscv64::IP_BTYPE);
                iproc_set(rose_riscv64_op_blt, new Riscv64::IP_BTYPE);
                iproc_set(rose_riscv64_op_bltu, new Riscv64::IP_BTYPE);
                iproc_set(rose_riscv64_op_bne, new Riscv64::IP_BTYPE);
                iproc_set(rose_riscv64_op_c_add, new Riscv64::IP_C_ADD);
                iproc_set(rose_riscv64_op_c_addi, new Riscv64::IP_C_ADDI);
                iproc_set(rose_riscv64_op_c_addi16sp, new Riscv64::IP_C_ADDI16SP);
                iproc_set(rose_riscv64_op_c_addi4spn, new Riscv64::IP_C_ADDI4SPN);
                iproc_set(rose_riscv64_op_c_addiw, new Riscv64::IP_C_ADDIW);
                iproc_set(rose_riscv64_op_c_addw, new Riscv64::IP_C_ADDW);
                iproc_set(rose_riscv64_op_c_and, new Riscv64::IP_C_AND);
                iproc_set(rose_riscv64_op_c_andi, new Riscv64::IP_C_ANDI);
                iproc_set(rose_riscv64_op_c_beqz, new Riscv64::IP_C_BEQZ);
                iproc_set(rose_riscv64_op_c_bnez, new Riscv64::IP_C_BNEZ);
                iproc_set(rose_riscv64_op_c_j, new Riscv64::IP_C_J);
                iproc_set(rose_riscv64_op_c_jal, new Riscv64::IP_C_JAL);
                iproc_set(rose_riscv64_op_c_jalr, new Riscv64::IP_C_JALR);
                iproc_set(rose_riscv64_op_c_jr, new Riscv64::IP_C_JR);
                iproc_set(rose_riscv64_op_c_ld, new Riscv64::IP_C_LD);
                iproc_set(rose_riscv64_op_c_ldsp, new Riscv64::IP_C_LDSP);
                iproc_set(rose_riscv64_op_c_li, new Riscv64::IP_C_LI);
                iproc_set(rose_riscv64_op_c_lui, new Riscv64::IP_C_LUI);
                iproc_set(rose_riscv64_op_c_lw, new Riscv64::IP_C_LW);
                iproc_set(rose_riscv64_op_c_lwsp, new Riscv64::IP_C_LWSP);
                iproc_set(rose_riscv64_op_c_mv, new Riscv64::IP_C_MV);
                iproc_set(rose_riscv64_op_c_nop, new Riscv64::IP_C_NOP);
                iproc_set(rose_riscv64_op_c_or, new Riscv64::IP_C_OR);
                iproc_set(rose_riscv64_op_c_sd, new Riscv64::IP_C_SD);
                iproc_set(rose_riscv64_op_c_sdsp, new Riscv64::IP_C_SDSP);
                iproc_set(rose_riscv64_op_c_slli, new Riscv64::IP_C_SLLI);
                iproc_set(rose_riscv64_op_c_srai, new Riscv64::IP_C_SRAI);
                iproc_set(rose_riscv64_op_c_srli, new Riscv64::IP_C_SRLI);
                iproc_set(rose_riscv64_op_c_sub, new Riscv64::IP_C_SUB);
                iproc_set(rose_riscv64_op_c_subw, new Riscv64::IP_C_SUBW);
                iproc_set(rose_riscv64_op_c_sw, new Riscv64::IP_C_SW);
                iproc_set(rose_riscv64_op_c_swsp, new Riscv64::IP_C_SWSP);
                iproc_set(rose_riscv64_op_c_xor, new Riscv64::IP_C_XOR);
                iproc_set(rose_riscv64_op_div, new Riscv64::IP_DIV);
                iproc_set(rose_riscv64_op_divu, new Riscv64::IP_DIV);
                iproc_set(rose_riscv64_op_divuw, new Riscv64::IP_DIVW);
                iproc_set(rose_riscv64_op_divw, new Riscv64::IP_DIVW);
                iproc_set(rose_riscv64_op_jal, new Riscv64::IP_RISCV_JAL);
                iproc_set(rose_riscv64_op_jalr, new Riscv64::IP_RISCV_JALR);
                iproc_set(rose_riscv64_op_lb, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_lbu, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_ld, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_lh, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_lhu, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_lr_d, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_d_aq, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_d_aq_rl, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_d_rl, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_w, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_w_aq, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_w_aq_rl, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lr_w_rl, new Riscv64::IP_LOADRES);
                iproc_set(rose_riscv64_op_lui, new Riscv64::IP_UTYPE);
                iproc_set(rose_riscv64_op_lw, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_lwu, new Riscv64::IP_LOAD);
                iproc_set(rose_riscv64_op_mul, new Riscv64::IP_MUL);
                iproc_set(rose_riscv64_op_mulh, new Riscv64::IP_MUL);
                iproc_set(rose_riscv64_op_mulhsu, new Riscv64::IP_MUL);
                iproc_set(rose_riscv64_op_mulhu, new Riscv64::IP_MUL);
                iproc_set(rose_riscv64_op_mulw, new Riscv64::IP_MULW);
                iproc_set(rose_riscv64_op_or, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_ori, new Riscv64::IP_ITYPE);
                iproc_set(rose_riscv64_op_rem, new Riscv64::IP_REM);
                iproc_set(rose_riscv64_op_remu, new Riscv64::IP_REM);
                iproc_set(rose_riscv64_op_remuw, new Riscv64::IP_REMW);
                iproc_set(rose_riscv64_op_remw, new Riscv64::IP_REMW);
                iproc_set(rose_riscv64_op_sb, new Riscv64::IP_STORE);
                iproc_set(rose_riscv64_op_sc_d, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_d_aq, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_d_aq_rl, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_d_rl, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_w, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_w_aq, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_w_aq_rl, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sc_w_rl, new Riscv64::IP_STORECON);
                iproc_set(rose_riscv64_op_sd, new Riscv64::IP_STORE);
                iproc_set(rose_riscv64_op_sh, new Riscv64::IP_STORE);
                iproc_set(rose_riscv64_op_sll, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_slli, new Riscv64::IP_SHIFTIOP);
                iproc_set(rose_riscv64_op_slliw, new Riscv64::IP_SHIFTIWOP);
                iproc_set(rose_riscv64_op_sllw, new Riscv64::IP_SHIFTW);
                iproc_set(rose_riscv64_op_slt, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_slti, new Riscv64::IP_ITYPE);
                iproc_set(rose_riscv64_op_sltiu, new Riscv64::IP_ITYPE);
                iproc_set(rose_riscv64_op_sltu, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_sra, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_srai, new Riscv64::IP_SHIFTIOP);
                iproc_set(rose_riscv64_op_sraiw, new Riscv64::IP_SHIFTIWOP);
                iproc_set(rose_riscv64_op_sraw, new Riscv64::IP_SHIFTW);
                iproc_set(rose_riscv64_op_srl, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_srli, new Riscv64::IP_SHIFTIOP);
                iproc_set(rose_riscv64_op_srliw, new Riscv64::IP_SHIFTIWOP);
                iproc_set(rose_riscv64_op_srlw, new Riscv64::IP_SHIFTW);
                iproc_set(rose_riscv64_op_sub, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_subw, new Riscv64::IP_RTYPEW);
                iproc_set(rose_riscv64_op_sw, new Riscv64::IP_STORE);
                iproc_set(rose_riscv64_op_xor, new Riscv64::IP_RTYPE);
                iproc_set(rose_riscv64_op_xori, new Riscv64::IP_ITYPE);
            }

            void
            DispatcherRiscv64::regcache_init() {
                if (regdict) {
                    REG_PC = findRegister("pc", 64);
                    REG_RA = findRegister("x1", 64);
                    REG_SP = findRegister("x2", 64);
                }
            }

            void
            DispatcherRiscv64::memory_init() {
                if (BaseSemantics::StatePtr state = currentState()) {
                    if (BaseSemantics::MemoryStatePtr memory = state->memoryState()) {
                        switch (memory->get_byteOrder()) {
                            case ByteOrder::ORDER_LSB:
                                break;
                            case ByteOrder::ORDER_MSB:
                                break;
                            case ByteOrder::ORDER_UNSPECIFIED:
                                memory->set_byteOrder(ByteOrder::ORDER_LSB);
                                break;
                        }
                    }
                }
            }

            RegisterDescriptor
            DispatcherRiscv64::instructionPointerRegister() const {
                return REG_PC;
            }

            RegisterDescriptor
            DispatcherRiscv64::stackPointerRegister() const {
                return REG_SP;
            }

            static bool
            isStatusRegister(const RegisterDescriptor &reg) {
                assert( 0 && "RISC-V does not have status registers");
                return false;
            }

            RegisterDictionary::RegisterDescriptors
            DispatcherRiscv64::get_usual_registers() const {
                RegisterDictionary::RegisterDescriptors registers = regdict->get_largest_registers();
                registers.erase(std::remove_if(registers.begin(), registers.end(), isStatusRegister),
                                registers.end());
                BOOST_FOREACH(
                const RegisterDescriptor &reg, regdict->get_smallest_registers()) {
                    if (isStatusRegister(reg))
                        registers.push_back(reg);
                }
                return registers;
            }

            void
            DispatcherRiscv64::set_register_dictionary(const RegisterDictionary *regdict) {
                BaseSemantics::Dispatcher::set_register_dictionary(regdict);
                regcache_init();
            }

            void
            DispatcherRiscv64::setFlagsForResult(const BaseSemantics::SValuePtr &result,
                                               const BaseSemantics::SValuePtr &carries,
                                               bool invertCarries, size_t nbits,
                                               BaseSemantics::SValuePtr &n, BaseSemantics::SValuePtr &z,
                                               BaseSemantics::SValuePtr &c, BaseSemantics::SValuePtr &v) {
                size_t width = result->get_width();

                n = operators->extract(result, width - 1, width);
                z = operators->equalToZero(result);

                BaseSemantics::SValuePtr sign = operators->extract(carries, nbits - 1, nbits);
                BaseSemantics::SValuePtr ofbit = operators->extract(carries, nbits - 2, nbits - 1);
                c = invertMaybe(sign, invertCarries);
                v = operators->xor_(sign, ofbit);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::parity(const BaseSemantics::SValuePtr &v) {
                ASSERT_require(v->get_width() == 8);
                BaseSemantics::SValuePtr p1 = operators->extract(v, 1, 2);
                BaseSemantics::SValuePtr p01 = operators->xor_(operators->extract(v, 0, 1), p1);
                BaseSemantics::SValuePtr p3 = operators->extract(v, 3, 4);
                BaseSemantics::SValuePtr p23 = operators->xor_(operators->extract(v, 2, 3), p3);
                BaseSemantics::SValuePtr p5 = operators->extract(v, 5, 6);
                BaseSemantics::SValuePtr p45 = operators->xor_(operators->extract(v, 4, 5), p5);
                BaseSemantics::SValuePtr p7 = operators->extract(v, 7, 8);
                BaseSemantics::SValuePtr p67 = operators->xor_(operators->extract(v, 6, 7), p7);
                BaseSemantics::SValuePtr p0123 = operators->xor_(p01, p23);
                BaseSemantics::SValuePtr p4567 = operators->xor_(p45, p67);
                BaseSemantics::SValuePtr pall = operators->xor_(p0123, p4567);
                return operators->invert(pall);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::invertMaybe(const BaseSemantics::SValuePtr &value, bool maybe) {
                return maybe ? operators->invert(value) : value;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::isZero(const BaseSemantics::SValuePtr &value) {
                return operators->equalToZero(value);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::ConditionHolds(const BaseSemantics::SValuePtr &cond) {

                BaseSemantics::SValuePtr result = operators->unspecified_(1);
                /*Dyninst::AST::Ptr condExpr = SymEvalSemantics::SValue::promote(cond)->get_expression();
                Dyninst::DataflowAPI::ConstantAST *
                constAST = dynamic_cast<Dyninst::DataflowAPI::ConstantAST *>(condExpr.get());

                ASSERT_not_null(constAST);
                Dyninst::DataflowAPI::Constant constVal = constAST->val();
                uint64_t condVal = constVal.val;

                BaseSemantics::SValuePtr nVal = readRegister(REG_N);
                BaseSemantics::SValuePtr zVal = readRegister(REG_Z);
                BaseSemantics::SValuePtr cVal = readRegister(REG_C);
                BaseSemantics::SValuePtr vVal = readRegister(REG_V);
                BaseSemantics::SValuePtr result = operators->unspecified_(1);

                switch (((condVal & 0xF) >> 1)) {
                    case 0:
                        result = operators->isEqual(zVal, operators->number_(1, 1));
                        break;
                    case 1:
                        result = operators->isEqual(cVal, operators->number_(1, 1));
                        break;
                    case 2:
                        result = operators->isEqual(nVal, operators->number_(1, 1));
                        break;
                    case 3:
                        result = operators->isEqual(vVal, operators->number_(1, 1));
                        break;
                    case 4:
                        result = operators->ite(operators->isEqual(cVal, operators->number_(1, 1)),
                                                operators->ite(
                                                        operators->isEqual(zVal, operators->number_(1, 0)),
                                                        operators->boolean_(true), operators->boolean_(false)),
                                                operators->boolean_(false));
                        break;
                    case 5:
                        result = operators->isEqual(nVal, vVal);
                        break;
                    case 6:
                        result = operators->ite(operators->isEqual(nVal, vVal),
                                                operators->ite(operators->isEqual(zVal,
                                                                                  operators->number_(1, 0)),
                                                               operators->boolean_(true),
                                                               operators->boolean_(false)),
                                                operators->boolean_(false));
                        break;
                    case 7:
                        result = operators->boolean_(true);
                        break;
                    default:
                        assert(!"invalid 3-bit value!");
                        break;
                }

                if ((condVal & 0x1) == 1 && (condVal & 0xF) != 0xF)
                    result = operators->invert(result);*/

                return result;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::NOT(const BaseSemantics::SValuePtr &expr) {
                return operators->invert(expr);
            }

            void
            DispatcherRiscv64::BranchTo(const BaseSemantics::SValuePtr &target) {
                ASSERT_require(target != NULL);
                writeRegister(REG_PC, operators->extract(target, 0, 64));
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::Zeros(const unsigned int nbits) {
                ASSERT_require(nbits > 0);
                return operators->number_(nbits, 0);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::SignExtend(const BaseSemantics::SValuePtr &expr, size_t newsize) {
                ASSERT_require(newsize > 0);
                return operators->signExtend(expr, newsize);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::ZeroExtend(const BaseSemantics::SValuePtr &expr, size_t newsize) {
                ASSERT_require(newsize > 0);
                return operators->unsignedExtend(expr, newsize);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::ROR(const BaseSemantics::SValuePtr &expr, const BaseSemantics::SValuePtr &amt) {
                ASSERT_not_null(amt);
                return operators->rotateRight(expr, amt);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::Replicate(const BaseSemantics::SValuePtr &expr) {
                ASSERT_not_null(expr);
                ASSERT_always_require(64 % expr->get_width() == 0);

                int blocknums = 64 / expr->get_width();
                BaseSemantics::SValuePtr ret = expr;
                for (int idx = 0; idx < blocknums; idx++) {
                    ret = operators->or_(ret,
                                         operators->shiftLeft(ret, operators->number_(8, expr->get_width() * idx)));
                }

                return ret;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::getBitfieldMask(int immr, int imms,
                                             int N, bool iswmask, int datasize) {
                int hsbarg = (N << 6) | (~imms);
                int len;
                for (int idx = 0; idx < 7; idx++) {
                    if ((hsbarg & 0x1) == 1)
                        len = idx;
                    hsbarg >>= 1;
                }

                if (len < 1 || (1 << len) > 64)
                    assert(!"Reserved value found in bitfield extract instruction!");

                int levels = ((1 << len) - 1);
                int S = imms & levels;
                int R = immr & levels;
                int diff = S - R;

                int d = diff & levels;
                int welem = (1 << (S + 1)) - 1, telem = (1 << (d + 1)) - 1;

                if (iswmask) {
                    BaseSemantics::SValuePtr wmask = operators->number_(datasize, welem);
                    wmask = ROR(wmask, operators->number_(32, R));
                    return Replicate(wmask);
                } else {
                    BaseSemantics::SValuePtr tmask = operators->number_(datasize, telem);
                    return Replicate(tmask);
                }
            }

            size_t
            DispatcherRiscv64::getRegSize(uint32_t raw) {
                if (IntegerOps::extract2<uint32_t>(23, 23, raw) == 0) {
                    if (IntegerOps::extract2<uint32_t>(30, 31, raw) == 0x3)
                        return 64;
                    else
                        return 32;
                } else {
                    if (IntegerOps::extract2<uint32_t>(22, 22, raw) == 1)
                        return 32;
                    else
                        return 64;
                }
            }

            size_t
            DispatcherRiscv64::ldStrLiteralAccessSize(uint32_t raw) {
                int opc = IntegerOps::extract2<uint32_t>(30, 31, raw);

                switch (opc) {
                    case 2:
                    case 0:
                        return 32;
                    case 1:
                        return 64;
                    default:
                        assert("Memory prefetch instruction not implemented yet!");
                        return 0;
                }
            }

            bool
            DispatcherRiscv64::inzero(uint32_t raw) {
                switch (IntegerOps::extract2<uint32_t>(29, 30, raw)) {
                    case 0:
                    case 2:
                        return true;
                    case 1:
                        return false;
                    default:
                        assert(!"Bitfield extract instruction has invalid opc field for inzero!");
                        break;
                }
            }

            bool
            DispatcherRiscv64::extend(uint32_t raw) {
                switch (IntegerOps::extract2<uint32_t>(29, 30, raw)) {
                    case 2:
                    case 1:
                        return false;
                    case 0:
                        return true;
                    default:
                        assert(!"Bitfield extract instruction has invalid opc field for extend!");
                        break;
                }
            }

            int
            DispatcherRiscv64::op(uint32_t raw) {
                switch (IntegerOps::extract2<uint32_t>(29, 30, raw)) {
                    case 0:
                    case 3:
                        return static_cast<int>(Riscv64::InsnProcessor::LogicalOp_AND);
                    case 1:
                        return static_cast<int>(Riscv64::InsnProcessor::LogicalOp_ORR);
                    case 2:
                        return static_cast<int>(Riscv64::InsnProcessor::LogicalOp_EOR);
                    default:
                        assert(!"Invalid code for opc field in logical OR instruction variant!");
                        break;
                }
            }

            bool
            DispatcherRiscv64::setflags(uint32_t raw) {
                if (IntegerOps::extract2(24, 28, raw) == 0x0A) {
                    switch (IntegerOps::extract2(29, 30, raw)) {
                        case 0:
                        case 1:
                        case 2:
                            return false;
                        case 3:
                            return true;
                        default:
                            assert(!"Invalid code for opc field in logical instruction!");
                            break;
                    }
                } else {
                    return IntegerOps::extract2(29, 29, raw) == 1;
                }
            }

            int
            DispatcherRiscv64::getDatasize(uint32_t raw) {
                if(IntegerOps::extract2(25, 27, raw) == 0x5) {
                    return 32 * (1 + (IntegerOps::extract2(31, 31, raw) & 0x1));
                } else {
                    int v27_29 = IntegerOps::extract2<uint32_t>(27, 29, raw), v23_25 = IntegerOps::extract2<uint32_t>(23, 25, raw);
                    int retval, v30_31 = IntegerOps::extract2<uint32_t>(30, 31, raw);;

                    if (v27_29 == 0x5 && v23_25 < 0x4) {
                        retval = 0x8 << (2 + (v30_31 & 0x1));
                    } else {
                        retval = 0x8 << v30_31;
                    }

                    return retval * 8;
                }
            }

            int
            DispatcherRiscv64::getShiftType(uint32_t raw) {
                int v10_11 = IntegerOps::extract2(10, 11, raw);

                switch(v10_11) {
                    case 0: return static_cast<int>(Riscv64::InsnProcessor::ShiftType_LSL);
                    case 1: return static_cast<int>(Riscv64::InsnProcessor::ShiftType_LSR);
                    case 2: return static_cast<int>(Riscv64::InsnProcessor::ShiftType_ASR);
                    case 3: return static_cast<int>(Riscv64::InsnProcessor::ShiftType_ROR);
                    default: ASSERT_not_reachable("Cannot have a value greater than 3 for a 2-bit field (field: op2, bits: 10..11)!");
                }
            }

            int
            DispatcherRiscv64::getConditionVal(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(24, 31, raw) == 0x54 && IntegerOps::extract2<uint32_t>(4, 4, raw) == 0)
                    return IntegerOps::extract2(0, 3, raw);
                else
                    return IntegerOps::extract2(12, 15, raw);
            }

            int
            DispatcherRiscv64::opcode(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(23, 28, raw) == 0x25)
                    return IntegerOps::extract2<uint32_t>(29, 30, raw);
                else
                    return IntegerOps::extract2<uint32_t>(10, 10, raw);
            }

            bool
            DispatcherRiscv64::subop(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(24, 28, raw) == 0x1B) {
                    return IntegerOps::extract2<uint32_t>(15, 15, raw) == 0x1;
                } else {
                    return IntegerOps::extract2<uint32_t>(30, 30, raw) == 0x1;
                }
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::doAddOperation(BaseSemantics::SValuePtr a, BaseSemantics::SValuePtr b,
                                            bool invertCarries, const BaseSemantics::SValuePtr &carryIn,
                                            BaseSemantics::SValuePtr &n, BaseSemantics::SValuePtr &z,
                                            BaseSemantics::SValuePtr &c, BaseSemantics::SValuePtr &v) {
                if (a->get_width() > b->get_width()) {
                    b = operators->signExtend(b, a->get_width());
                } else if (a->get_width() < b->get_width()) {
                    a = operators->signExtend(a, b->get_width());
                }

                ASSERT_require(1 == carryIn->get_width());
                size_t nbits = a->get_width();
                BaseSemantics::SValuePtr carries;
                BaseSemantics::SValuePtr result = operators->addWithCarries(a, b,
                                                                            invertMaybe(carryIn, invertCarries),
                                                                            carries/*out*/);
                setFlagsForResult(result, carries, invertCarries, a->get_width(), n, z, c, v);
                return result;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::effectiveAddress(SgAsmExpression *e, size_t nbits) {
                SgAsmExpression *addressExpression;
                if (SgAsmMemoryReferenceExpression *memoryReferenceExpression = isSgAsmMemoryReferenceExpression(e))
                    addressExpression = memoryReferenceExpression->get_address();
                else
                    addressExpression = e;

                BaseSemantics::SValuePtr retval;

                if (SgAsmRegisterReferenceExpression *rre = isSgAsmRegisterReferenceExpression(addressExpression)) {
                    const RegisterDescriptor &reg = rre->get_descriptor();
                    retval = operators->readRegister(reg);
                } else if (SgAsmBinaryAdd *op = isSgAsmBinaryAdd(addressExpression)) {
                    BaseSemantics::SValuePtr lhs = effectiveAddress(op->get_lhs(), nbits);
                    BaseSemantics::SValuePtr rhs = effectiveAddress(op->get_rhs(), nbits);
                    retval = operators->add(lhs, rhs);
                } else if (SgAsmIntegerValueExpression *ival = isSgAsmIntegerValueExpression(addressExpression)) {
                    retval = operators->number_(ival->get_significantBits(), ival->get_value());
                } else if (SgAsmDoubleWordValueExpression *ival = isSgAsmDoubleWordValueExpression(addressExpression)) {
                    retval = operators->number_(ival->get_bit_size(), ival->get_value());
                }

                ASSERT_not_null(retval);
                if (retval->get_width() < nbits) {
                    retval = operators->signExtend(retval, nbits);
                } else if (retval->get_width() > nbits) {
                    retval = operators->extract(retval, 0, nbits);
                }

                return retval;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::readRegister(const RegisterDescriptor &reg) {
                return operators->readRegister(reg);
            }

            void
            DispatcherRiscv64::writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value) {
                operators->writeRegister(reg, value);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::read(SgAsmExpression *e, size_t value_nbits/*=0*/, size_t addr_nbits/*=0*/) {
                // Hardwire x0 to value 0
                if (SgAsmDirectRegisterExpression *re = isSgAsmDirectRegisterExpression(e)) {
                    const struct RegisterDescriptor &reg = re->get_descriptor();
                    if (reg.get_major() == riscv64_regclass_gpr && reg.get_minor() == riscv64_gpr_x0) {
                        return operators->number_(64, 0);
                    }
                }
                return Dispatcher::read(e, value_nbits, addr_nbits);
            }
            void
            DispatcherRiscv64::write(SgAsmExpression *e, const BaseSemantics::SValuePtr &value,
                                   size_t addr_nbits/*=0*/) {
                if (SgAsmDirectRegisterExpression *re = isSgAsmDirectRegisterExpression(e)) {
                    const struct RegisterDescriptor &reg = re->get_descriptor();
                    // Write to x0 means nop
                    if (reg.get_major() == riscv64_regclass_gpr && reg.get_minor() == riscv64_gpr_x0) {
                        return;
                    }
                    writeRegister(re->get_descriptor(), value);
                } else {
                    Dispatcher::write(e, value, addr_nbits);        // defer to super class
                }
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::fixMemoryAddress(const BaseSemantics::SValuePtr &addr) const {
                if (size_t addrWidth = addressWidth()) {
                    if (addr->get_width() < addrWidth)
                        return operators->signExtend(addr, addrWidth);
                    if (addr->get_width() > addrWidth)
                        return operators->unsignedExtend(addr, addrWidth);
                }
                return addr;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::readMemory(const BaseSemantics::SValuePtr &addr, size_t readSize) {
                SymEvalSemantics::StateASTPtr state = SymEvalSemantics::StateAST::promote(operators->currentState());

                //The second, third and fourth arguments will remain unused
                return state->readMemory(addr, operators->unspecified_(1), NULL, NULL, readSize);
            }

            void
            DispatcherRiscv64::writeMemory(const BaseSemantics::SValuePtr &addr, size_t writeSize, const BaseSemantics::SValuePtr &data) {
                SymEvalSemantics::StateASTPtr state = SymEvalSemantics::StateAST::promote(operators->currentState());

                //The third and fourth arguments will remain unused
                state->writeMemory(addr, data, NULL, NULL, writeSize);
            }

            SgAsmExpression *
            DispatcherRiscv64::getWriteBackTarget(SgAsmExpression *expr) {
                SgAsmMemoryReferenceExpression *memoryExpression = isSgAsmMemoryReferenceExpression(expr);
                ASSERT_not_null(memoryExpression);

                SgAsmExpression *address = memoryExpression->get_address();
                ASSERT_not_null(address);

                if (isSgAsmBinaryAdd(address)) {
                    return isSgAsmBinaryAdd(address)->get_lhs();
                } else {
                    SgAsmRegisterReferenceExpression *retval = isSgAsmRegisterReferenceExpression(address);
                    ASSERT_not_null(retval);

                    return retval;
                }
            }

            /*Just returning the expression as-is, since there is no flag or variable in semantics indicating whether or not
            a SValue should be treated as signed/unsigned. The signed-ness should be taken into account when performing an
            operation on this value instead */
            BaseSemantics::SValuePtr
            DispatcherRiscv64::UInt(const BaseSemantics::SValuePtr &expr) {
                ASSERT_not_null(expr);
                return expr;
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::ShiftReg(const BaseSemantics::SValuePtr &src, int shiftType, const BaseSemantics::SValuePtr &amount) {
                ASSERT_not_null(amount);

                switch(static_cast<Riscv64::InsnProcessor::ShiftType>(shiftType)) {
                    case Riscv64::InsnProcessor::ShiftType_LSL: return operators->shiftLeft(src, amount);
                    case Riscv64::InsnProcessor::ShiftType_LSR: return operators->shiftRight(src, amount);
                    case Riscv64::InsnProcessor::ShiftType_ASR: return operators->shiftRightArithmetic(src, amount);
                    case Riscv64::InsnProcessor::ShiftType_ROR: return operators->rotateRight(src, amount);
                    default: ASSERT_not_reachable("Found invalid shift type for shift instruction!");
                }
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::CountLeadingZeroBits(const BaseSemantics::SValuePtr &expr) {
                size_t len = expr->get_width();

                for(int idx = len - 1; idx >= 0; idx--)
                    if(operators->isEqual(operators->extract(expr, len, len + 1), operators->number_(1, 1)))
                        return operators->number_(expr->get_width(), len - 1 - idx);

                return operators->number_(expr->get_width(), len);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::CountLeadingSignBits(const BaseSemantics::SValuePtr &expr) {
                size_t len = expr->get_width();
                BaseSemantics::SValuePtr arg = operators->xor_(operators->extract(expr, 1, len), operators->extract(expr, 0, len - 1));
                return CountLeadingZeroBits(arg);
            }

            BaseSemantics::SValuePtr
            DispatcherRiscv64::Int(const BaseSemantics::SValuePtr &expr, bool isUnsigned) {
                if(isUnsigned)
                    return UInt(expr);
                else {
                    int len = expr->get_width();
                    BaseSemantics::SValuePtr ret = Zeros(len);

                    for(int idx = 0; idx < len; idx++)
                        if(operators->isEqual(operators->extract(expr, idx, idx + 1), operators->number_(1, 1)))
                            ret = operators->add(ret, operators->number_(len, 2<<idx));

                    if(operators->isEqual(operators->extract(expr, len, len + 1), operators->number_(1, 1)))
                        ret = operators->add(ret, operators->negate(operators->number_(len + 1, 2<<len)));

                    return ret;
                }
            }

            /* Return the input expression as-is. At this point, it is assumed that any RiscOperators implementation
             * that performs division and passes the result to this method would have already performed the rounding
             * (most likely by using the / operator) to match how integer division is normally performed in C/C++.
             * Of course, this would be invalid for floating point division but that is not currently supported. */
            BaseSemantics::SValuePtr
            DispatcherRiscv64::RoundTowardsZero(const BaseSemantics::SValuePtr &expr) {
                return expr;
            }
        } // namespace
    } // namespace
} // namespace

//using namespace rose::Diagnostics;

