#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "BaseSemantics2.h"
//#include "Diagnostics.h"
#include "DispatcherARM64.h"
#include "../integerOps.h"
#include "SymEvalSemantics.h"

#include "../SgAsmExpression.h"
#include "../conversions.h"

#undef si_value                                         // name pollution from siginfo.h

namespace rose {
    namespace BinaryAnalysis {
        namespace InstructionSemantics2 {

#define EXTR(lo, hi)    IntegerOps::extract2<B>(lo, hi, raw)

/*******************************************************************************************************************************
 *                                      Support functions
 *******************************************************************************************************************************/

            static inline size_t asm_type_width(SgAsmType *ty) {
                ASSERT_not_null(ty);
                return ty->get_nBits();
            }

/*******************************************************************************************************************************
 *                                      Base ARM64 instruction processor
 *******************************************************************************************************************************/
            namespace ARM64 {

                void
                InsnProcessor::process(const BaseSemantics::DispatcherPtr &dispatcher_, SgAsmInstruction *insn_) {
                    DispatcherARM64Ptr dispatcher = DispatcherARM64::promote(dispatcher_);
                    BaseSemantics::RiscOperatorsPtr operators = dispatcher->get_operators();
                    SgAsmArmv8Instruction *insn = isSgAsmArmv8Instruction(insn_);
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
                }

                void
                InsnProcessor::assert_args(I insn, A args, size_t nargs) {
                    if (args.size() != nargs) {
                        std::string mesg = "instruction has incorrect number of args";
                        throw BaseSemantics::Exception(mesg, insn);
                    }
                }

/*******************************************************************************************************************************
 *                                      Functors that handle individual ARM64 instructions kinds
 *******************************************************************************************************************************/
                typedef InsnProcessor P;

                struct IP_add_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_adds_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_sub_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_subs_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_add_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_adds_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_sub_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_subs_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_add_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_adds_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_sub_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_subs_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_adc_execute : P {                                    //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }
                        result = d->doAddOperation(operand1, operand2, false,
                                                   d->readRegister(d->REG_C)/*, ops->boolean_(false)*/,
                                                   n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_adcs_execute : P {                                    //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }
                        result = d->doAddOperation(operand1, operand2, false,
                                                   d->readRegister(d->REG_C)/*, ops->boolean_(false)*/,
                                                   n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_adr_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr base = d->readRegister(d->REG_PC);

                        if ((EXTR (31, 31) == 1)) {
                            base =
                                    ops->or_(ops->and_(base, ops->number_(64, 0xfffffffffffff000)),
                                             d->Zeros(12));
                        }
			// args[1] is in the form of PC + offset
			// we do not want PC to appear twice, so we extract the offset
			SgAsmBinaryExpression * addOp = dynamic_cast<SgAsmBinaryExpression*>(args[1]);
                        d->write(args[0], ops->add(base, d->read(addOp->get_rhs())));
                    }
                };

                struct IP_adrp_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr base = d->readRegister(d->REG_PC);

                        if ((EXTR (31, 31) == 1)) {
                            base =
                                    ops->or_(ops->and_(base, ops->number_(64, 0xfffffffffffff000)),
                                             d->Zeros(12));
                        }
			// args[1] is in the form of PC + offset
			// we do not want PC to appear twice, so we extract the offset
			SgAsmBinaryExpression * addOp = dynamic_cast<SgAsmBinaryExpression*>(args[1]);
                        d->write(args[0], ops->add(base, d->read(addOp->get_rhs())));
                    }
                };

                struct IP_b_cond_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        d->BranchTo(ops->ite(
                                ops->isEqual(d->ConditionHolds(ops->number_(4, EXTR (0, 4))), ops->boolean_(true)),
                                d->read(args[0]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_b_uncond_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        if (EXTR(31, 31) == 1)
                            d->writeRegister(d->findRegister("x30", 64),
                                             ops->add(d->readRegister(d->REG_PC), ops->number_(32, 4)));
                        d->BranchTo(d->read(args[0]));
                    }
                };

                struct IP_br_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr target = d->read(args[0]);
                        if (EXTR (31, 31) == 1)
                            d->writeRegister(d->findRegister("x30", 64),
                                             ops->add(d->readRegister(d->REG_PC),
                                                      ops->number_(32, 4)));

                        d->BranchTo(target);
                    }
                };

                struct IP_blr_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr target = d->read(args[0]);
                        if (EXTR (31, 31) == 1)
                            d->writeRegister(d->findRegister("x30", 64),
                                             ops->add(d->readRegister(d->REG_PC),
                                                      ops->number_(32, 4)));

                        d->BranchTo(target);
                    }
                };

                struct IP_bl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        if (EXTR (31, 31) == 1)
                            d->writeRegister(d->findRegister("x30", 64),
                                             ops->add(d->readRegister(d->REG_PC),
                                                      ops->number_(32, 4)));

                        d->BranchTo(d->read(args[0]));
                    }
                };

                struct IP_cbz_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        d->BranchTo(ops->ite(ops->isEqual(d->isZero(operand1), ops->boolean_(EXTR (24, 24) == 0)),
                                             d->read(args[1]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_cbnz_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        d->BranchTo(ops->ite(ops->isEqual(d->isZero(operand1), ops->boolean_(EXTR (24, 24) == 0)),
                                             d->read(args[1]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_tbz_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand = d->read(args[0]);
                        d->BranchTo(ops->ite(ops->isEqual(
                                ops->and_(ops->shiftRight(operand, d->read(args[1])), ops->number_(1, 1)),
                                ops->number_(1, EXTR (24, 24))),
                                             d->read(args[2]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_tbnz_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand = d->read(args[0]);
                        d->BranchTo(ops->ite(ops->isEqual(
                                ops->and_(ops->shiftRight(operand, d->read(args[1])), ops->number_(1, 1)),
                                ops->number_(1, EXTR (24, 24))),
                                             d->read(args[2]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_cmp_subs_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_cmp_subs_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_cmp_subs_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_cmn_adds_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_cmn_adds_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !(EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_cmn_adds_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if ((EXTR(29, 29) == 1)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_ccmn_reg_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr nzcv = d->read(args[2]);
                        bool carry_in = false;

                        BaseSemantics::SValuePtr n = ops->extract(nzcv, 3, 4);
                        BaseSemantics::SValuePtr z = ops->extract(nzcv, 2, 3);
                        BaseSemantics::SValuePtr c = ops->extract(nzcv, 1, 2);
                        BaseSemantics::SValuePtr v = ops->extract(nzcv, 0, 1);

                        /*if (d->read(args[3])) {
                            if ((EXTR (30, 30) == 1)) {
                                operand2 = d->NOT(operand2);

                                carry_in = true;

                            }

                            d->doAddOperation(operand1, operand2, carry_in,
                                              ops->boolean_(false), nzcv);

                        }*/

                        operand2 = ops->ite(ops->isEqual(d->ConditionHolds(d->read(args[3])), ops->boolean_(true)),
                                            (EXTR (30, 30) == 1 ? (carry_in = true, d->NOT(operand2)) : operand2),
                                            operand2);
                        ops->ite(ops->isEqual(d->ConditionHolds(d->read(args[3])), ops->boolean_(true)),
                                 d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v),
                                 ops->unspecified_(1));

                        d->writeRegister(d->REG_N, n);
                        d->writeRegister(d->REG_Z, z);
                        d->writeRegister(d->REG_C, c);
                        d->writeRegister(d->REG_V, v);
                    }
                };

                struct IP_ccmn_imm_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr nzcv = d->read(args[2]);
                        bool carry_in = false;

                        BaseSemantics::SValuePtr n = ops->extract(nzcv, 3, 4);
                        BaseSemantics::SValuePtr z = ops->extract(nzcv, 2, 3);
                        BaseSemantics::SValuePtr c = ops->extract(nzcv, 1, 2);
                        BaseSemantics::SValuePtr v = ops->extract(nzcv, 0, 1);

                        /*if (d->read(args[3])) {
                            if ((EXTR (30, 30) == 1)) {
                                operand2 = d->NOT(operand2);

                                carry_in = true;

                            }

                            d->doAddOperation(operand1, operand2, carry_in,
                                              ops->boolean_(false), nzcv);

                        }*/

                        operand2 = ops->ite(ops->isEqual(d->ConditionHolds(d->read(args[3])), ops->boolean_(true)),
                                            (EXTR (30, 30) == 1 ? (carry_in = true, d->NOT(operand2)) : operand2),
                                            operand2);
                        ops->ite(ops->isEqual(d->ConditionHolds(d->read(args[3])), ops->boolean_(true)),
                                 d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v),
                                 ops->unspecified_(1));

                        d->writeRegister(d->REG_N, n);
                        d->writeRegister(d->REG_Z, z);
                        d->writeRegister(d->REG_C, c);
                        d->writeRegister(d->REG_V, v);
                    }
                };

                struct IP_ldr_imm_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_str_imm_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrb_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_strb_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrh_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrsb_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrsh_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrsw_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
//						 fprintf(stderr, "xxx %d %d\n", 0x8 << EXTR(30, 31), EXTR(30, 31));
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_strh_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldr_reg_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrb_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrh_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrsb_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrsh_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldrsw_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_str_reg_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_strb_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_strh_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldr_lit_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->ldStrLiteralAccessSize(raw));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, 64));
                                } else {
                                    d->write(args[0], data);
                                }
                            }
                                break;
                        }

                    }
                };

                struct IP_ldrsw_lit_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->ldStrLiteralAccessSize(raw));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, 64));
                                } else {
                                    d->write(args[0], data);
                                }
                            }
                                break;
                        }

                    }
                };

                //TODO: For ubfm/sbfm based instructions, update the grammar to declare variables for imms/immr and wmask/tmask instead of reading/calculating them twice
                struct IP_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                //TODO modified manually for jump table analysis
                struct IP_uxtb_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        /*BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22), false, (EXTR(31, 31) + 1) * 32);
                            BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22), true, (EXTR(31, 31) + 1) * 32);
                            BaseSemantics::SValuePtr dst;
                            if (d->inzero(raw))
                            dst = d->Zeros(64);
                            else
                            dst = d->read(args[0]);
                            BaseSemantics::SValuePtr src = d->read(args[1]);
                            BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)), ops->and_(d->ROR(src, ops->number_(32, EXTR(16, 21))), wmask));
                            BaseSemantics::SValuePtr top;
                            if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))), ops->number_(1, 1)));
                            else
                            top = dst;
                            d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));
                            */
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr writeVal = ops->extract(src, 0, 8);
                        d->write(args[0], ops->unsignedExtend(writeVal, 32));
                    }
                };

                struct IP_uxth_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, ops->number_(32, EXTR(16, 21))),
                                                                          wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_ubfiz_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_ubfx_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_sxth_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, ops->number_(32, EXTR(16, 21))),
                                                                          wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_sxtb_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, ops->number_(32, EXTR(16, 21))),
                                                                          wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_sxtw_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, ops->number_(32, EXTR(16, 21))),
                                                                          wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_sbfiz_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_sbfx_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_movz_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;

                        if (d->opcode(raw) == MoveWideOp_K) {
                            result = d->read(args[0]);
                        } else {
                            result = d->Zeros(64);
                        }
                        result = ops->or_(ops->extract(result, 0, (EXTR(21, 22) << 4)), ops->or_(
                                ops->shiftLeft(d->read(args[1]), ops->number_(32, (EXTR(21, 22) << 4))),
                                ops->shiftLeft(ops->extract(result, (EXTR(21, 22) << 4) + 15 + 1, result->get_width()),
                                               ops->number_(32, (EXTR(21, 22) << 4) + 15 + 1))));

                        if (d->opcode(raw) == MoveWideOp_N) {
                            result = d->NOT(result);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_mov_movz_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;

                        if (d->opcode(raw) == MoveWideOp_K) {
                            result = d->read(args[0]);
                        } else {
                            result = d->Zeros(64);
                        }
                        result = ops->or_(ops->extract(result, 0, (EXTR(21, 22) << 4)), ops->or_(
                                ops->shiftLeft(d->read(args[1]), ops->number_(32, (EXTR(21, 22) << 4))),
                                ops->shiftLeft(ops->extract(result, (EXTR(21, 22) << 4) + 15 + 1, result->get_width()),
                                               ops->number_(32, (EXTR(21, 22) << 4) + 15 + 1))));

                        if (d->opcode(raw) == MoveWideOp_N) {
                            result = d->NOT(result);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_movn_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;

                        if (d->opcode(raw) == MoveWideOp_K) {
                            result = d->read(args[0]);
                        } else {
                            result = d->Zeros(64);
                        }
                        result = ops->or_(ops->extract(result, 0, (EXTR(21, 22) << 4)), ops->or_(
                                ops->shiftLeft(d->read(args[1]), ops->number_(32, (EXTR(21, 22) << 4))),
                                ops->shiftLeft(ops->extract(result, (EXTR(21, 22) << 4) + 15 + 1, result->get_width()),
                                               ops->number_(32, (EXTR(21, 22) << 4) + 15 + 1))));

                        if (d->opcode(raw) == MoveWideOp_N) {
                            result = d->NOT(result);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_mov_movn_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;

                        if (d->opcode(raw) == MoveWideOp_K) {
                            result = d->read(args[0]);
                        } else {
                            result = d->Zeros(64);
                        }
                        result = ops->or_(ops->extract(result, 0, (EXTR(21, 22) << 4)), ops->or_(
                                ops->shiftLeft(d->read(args[1]), ops->number_(32, (EXTR(21, 22) << 4))),
                                ops->shiftLeft(ops->extract(result, (EXTR(21, 22) << 4) + 15 + 1, result->get_width()),
                                               ops->number_(32, (EXTR(21, 22) << 4) + 15 + 1))));

                        if (d->opcode(raw) == MoveWideOp_N) {
                            result = d->NOT(result);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_movk_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;

                        if (d->opcode(raw) == MoveWideOp_K) {
                            result = d->read(args[0]);
                        } else {
                            result = d->Zeros(64);
                        }
                        result = ops->or_(ops->extract(result, 0, (EXTR(21, 22) << 4)), ops->or_(
                                ops->shiftLeft(d->read(args[1]), ops->number_(32, (EXTR(21, 22) << 4))),
                                ops->shiftLeft(ops->extract(result, (EXTR(21, 22) << 4) + 15 + 1, result->get_width()),
                                               ops->number_(32, (EXTR(21, 22) << 4) + 15 + 1))));

                        if (d->opcode(raw) == MoveWideOp_N) {
                            result = d->NOT(result);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_fmov_float_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr srcVal = d->read(args[1]);
                        d->write(args[0], srcVal);
                    }
                };

                struct IP_orr_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_orn_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_orr_log_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }

                        if (EXTR(0, 4) == 31 && !d->setflags(raw)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_and_log_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }

                        if (EXTR(0, 4) == 31 && !d->setflags(raw)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_and_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_ands_log_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }

                        if (EXTR(0, 4) == 31 && !d->setflags(raw)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_ands_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_eor_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_eor_log_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }

                        if (EXTR(0, 4) == 31 && !d->setflags(raw)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_eon_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                //TODO modified manually for jump table analysis
                struct IP_mov_orr_log_shift_execute : P {                                  //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        d->write(args[0], src);

                    }
                };

                //TODO modified manually for jump table analysis
                struct IP_mov_orr_log_imm_execute : P {                                    //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        d->write(args[0], src);

                    }
                };

                struct IP_lsl_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_lsr_ubfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_asr_sbfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, ops->number_(32, EXTR(10, 15))),
                                                         ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_bfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_bfxil_bfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_bfi_bfm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr tmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            false, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr wmask = d->getBitfieldMask(EXTR(16, 21), EXTR(10, 15), EXTR(22, 22),
                                                                            true, (EXTR(31, 31) + 1) * 32);
                        BaseSemantics::SValuePtr dst;
                        if (d->inzero(raw))
                            dst = d->Zeros(64);
                        else
                            dst = d->read(args[0]);
                        BaseSemantics::SValuePtr src = d->read(args[1]);
                        BaseSemantics::SValuePtr bot = ops->or_(ops->and_(dst, d->NOT(wmask)),
                                                                ops->and_(d->ROR(src, d->read(args[2])), wmask));
                        BaseSemantics::SValuePtr top;
                        if (d->extend(raw))
                            top = d->Replicate(ops->and_(ops->shiftRight(src, d->read(args[3])), ops->number_(1, 1)));
                        else
                            top = dst;
                        d->write(args[0], ops->or_(ops->and_(top, d->NOT(tmask)), ops->and_(bot, tmask)));

                    }
                };

                struct IP_bic_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_bics_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_stp_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data1;
                        BaseSemantics::SValuePtr data2;
                        int dbytes = d->getDatasize(raw) / 8;
                        bool rt_unknown = false;
                        bool wb_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown && EXTR(0, 4) == EXTR(5, 9)) {
                                    data1 = ops->unspecified_(1);
                                } else {
                                    data1 = d->read(args[0]);
                                }

                                if (rt_unknown && EXTR(10, 14) == EXTR(5, 9)) {
                                    data2 = ops->unspecified_(1);
                                } else {
                                    data2 = d->read(args[1]);
                                }
                                d->writeMemory(ops->add(address, ops->number_(32, 0)), dbytes, data1);
                                d->writeMemory(ops->add(address, ops->number_(32, dbytes)), dbytes, data2);
                            }
                                break;
                            case MemOp_LOAD: {
                                data1 = d->readMemory(ops->add(address, ops->number_(32, 0)), dbytes);
                                data2 = d->readMemory(ops->add(address, ops->number_(32, dbytes)), dbytes);

                                if (rt_unknown) {
                                    data1 = ops->unspecified_(1);
                                    data2 = ops->unspecified_(1);
                                }

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data1, 64));
                                    d->write(args[1], d->SignExtend(data2, 64));
                                } else {
                                    d->write(args[0], data1);
                                    d->write(args[1], data2);
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldp_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data1;
                        BaseSemantics::SValuePtr data2;
                        int dbytes = d->getDatasize(raw) / 8;
                        bool rt_unknown = false;
                        bool wb_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown && EXTR(0, 4) == EXTR(5, 9)) {
                                    data1 = ops->unspecified_(1);
                                } else {
                                    data1 = d->read(args[0]);
                                }

                                if (rt_unknown && EXTR(10, 14) == EXTR(5, 9)) {
                                    data2 = ops->unspecified_(1);
                                } else {
                                    data2 = d->read(args[1]);
                                }
                                d->writeMemory(ops->add(address, ops->number_(32, 0)), dbytes, data1);
                                d->writeMemory(ops->add(address, ops->number_(32, dbytes)), dbytes, data2);
                            }
                                break;
                            case MemOp_LOAD: {
                                data1 = d->readMemory(ops->add(address, ops->number_(32, 0)), dbytes);
                                data2 = d->readMemory(ops->add(address, ops->number_(32, dbytes)), dbytes);

                                if (rt_unknown) {
                                    data1 = ops->unspecified_(1);
                                    data2 = ops->unspecified_(1);
                                }

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data1, 64));
                                    d->write(args[1], d->SignExtend(data2, 64));
                                } else {
                                    d->write(args[0], data1);
                                    d->write(args[1], data2);
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldpsw_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data1;
                        BaseSemantics::SValuePtr data2;
                        int dbytes = d->getDatasize(raw) / 8;
                        bool rt_unknown = false;
                        bool wb_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown && EXTR(0, 4) == EXTR(5, 9)) {
                                    data1 = ops->unspecified_(1);
                                } else {
                                    data1 = d->read(args[0]);
                                }

                                if (rt_unknown && EXTR(10, 14) == EXTR(5, 9)) {
                                    data2 = ops->unspecified_(1);
                                } else {
                                    data2 = d->read(args[1]);
                                }
                                d->writeMemory(ops->add(address, ops->number_(32, 0)), dbytes, data1);
                                d->writeMemory(ops->add(address, ops->number_(32, dbytes)), dbytes, data2);
                            }
                                break;
                            case MemOp_LOAD: {
                                data1 = d->readMemory(ops->add(address, ops->number_(32, 0)), dbytes);
                                data2 = d->readMemory(ops->add(address, ops->number_(32, dbytes)), dbytes);

                                if (rt_unknown) {
                                    data1 = ops->unspecified_(1);
                                    data2 = ops->unspecified_(1);
                                }

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data1, 64));
                                    d->write(args[1], d->SignExtend(data2, 64));
                                } else {
                                    d->write(args[0], data1);
                                    d->write(args[1], data2);
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldnp_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data1;
                        BaseSemantics::SValuePtr data2;
                        int dbytes = d->getDatasize(raw) / 8;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown && EXTR(0, 4) == EXTR(5, 9)) {
                                    data1 = ops->unspecified_(1);
                                } else {
                                    data1 = d->read(args[0]);
                                }

                                if (rt_unknown && EXTR(10, 14) == EXTR(5, 9)) {
                                    data2 = ops->unspecified_(1);
                                } else {
                                    data2 = d->read(args[1]);
                                }
                                d->writeMemory(ops->add(address, ops->number_(32, 0)), dbytes, data1);
                                d->writeMemory(ops->add(address, ops->number_(32, dbytes)), dbytes, data2);
                            }
                                break;
                            case MemOp_LOAD: {
                                data1 = d->readMemory(ops->add(address, ops->number_(32, 0)), dbytes);
                                data2 = d->readMemory(ops->add(address, ops->number_(32, dbytes)), dbytes);

                                if (rt_unknown) {
                                    data1 = ops->unspecified_(1);
                                    data2 = ops->unspecified_(1);
                                }
                                d->write(args[0], data1);
                                d->write(args[1], data2);
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_stnp_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data1;
                        BaseSemantics::SValuePtr data2;
                        int dbytes = d->getDatasize(raw) / 8;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown && EXTR(0, 4) == EXTR(5, 9)) {
                                    data1 = ops->unspecified_(1);
                                } else {
                                    data1 = d->read(args[0]);
                                }

                                if (rt_unknown && EXTR(10, 14) == EXTR(5, 9)) {
                                    data2 = ops->unspecified_(1);
                                } else {
                                    data2 = d->read(args[1]);
                                }
                                d->writeMemory(ops->add(address, ops->number_(32, 0)), dbytes, data1);
                                d->writeMemory(ops->add(address, ops->number_(32, dbytes)), dbytes, data2);
                            }
                                break;
                            case MemOp_LOAD: {
                                data1 = d->readMemory(ops->add(address, ops->number_(32, 0)), dbytes);
                                data2 = d->readMemory(ops->add(address, ops->number_(32, dbytes)), dbytes);

                                if (rt_unknown) {
                                    data1 = ops->unspecified_(1);
                                    data2 = ops->unspecified_(1);
                                }
                                d->write(args[0], data1);
                                d->write(args[1], data2);
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldtr_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldtrb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldtrh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldtrsb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldtrsh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_sttr_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*f (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_sttrb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_sttrh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldur_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                //address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldurb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldurh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldursb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldursh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_ldursw_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_sturb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_sturh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_stur_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        /*if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }*/

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                } else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, d->getDatasize(raw) / 8, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->getDatasize(raw) / 8);

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                } else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if (((EXTR(24, 24) == 0) && EXTR(21, 21) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            } else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            } else {
                                d->write(d->getWriteBackTarget(args[1]), address);
                            }
                        }

                    }
                };

                struct IP_asr_asrv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_asrv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_lsl_lslv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_lslv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_lsr_lsrv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_lsrv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_ror_rorv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_rorv_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        result = d->ShiftReg(d->read(args[1]), d->getShiftType(raw),
                                             ops->unsignedModulo(d->UInt(operand2),
                                                                 ops->number_(32, d->getDatasize(raw))));
                        d->write(args[0], result);

                    }
                };

                struct IP_tst_ands_log_imm_execute : P {                                    //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }

                        if (EXTR(0, 4) == 31 && !d->setflags(raw)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->writeRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 64), result);
                        }

                    }
                };

                struct IP_tst_ands_log_shift_execute : P {                                  //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->writeRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 64), result);

                    }
                };

                struct IP_sbc_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }
                        result = d->doAddOperation(operand1, operand2, d->readRegister(d->REG_C), ops->boolean_(false),
                                                   n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_sbcs_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr n, z, c, v;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }
                        result = d->doAddOperation(operand1, operand2, d->readRegister(d->REG_C), ops->boolean_(false),
                                                   n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_ngc_sbc_execute : P {                                             //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->readRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 32));
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr n, z, c, v;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }
                        result = d->doAddOperation(operand1, operand2, d->readRegister(d->REG_C), ops->boolean_(false),
                                                   n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_ngcs_sbcs_execute : P {                                           //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->readRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 32));
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr n, z, c, v;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }
                        result = d->doAddOperation(operand1, operand2, d->readRegister(d->REG_C), ops->boolean_(false),
                                                   n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_neg_sub_addsub_shift_execute : P {                                //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->readRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 32));
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_negs_subs_addsub_shift_execute : P {                              //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->readRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 32));
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_mvn_orn_log_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->readRegister(RegisterDescriptor(armv8_regclass_gpr, armv8_gpr_zr, 0, 32));
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);

                        if ((EXTR(21, 21) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        switch (d->op(raw)) {
                            case LogicalOp_AND: {
                                result = ops->and_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_ORR: {
                                result = ops->or_(operand1, operand2);
                            }
                                break;
                            case LogicalOp_EOR: {
                                result = ops->xor_(operand1, operand2);
                            }
                                break;
                        }

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N,
                                             ops->extract(result, result->get_width() - 1, result->get_width()));
                            d->writeRegister(d->REG_Z, d->isZero(result));
                            d->writeRegister(d->REG_C, ops->number_(1, 0));
                            d->writeRegister(d->REG_V, ops->number_(1, 0));
                        }
                        d->write(args[0], result);

                    }
                };

                struct IP_mov_add_addsub_imm_execute : P {                                  //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->Zeros(operand1->get_width());
                        BaseSemantics::SValuePtr n, z, c, v;
                        bool carry_in;

                        if ((EXTR(30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                            carry_in = true;
                        } else {
                            carry_in = false;
                        }
                        result = d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), n, z, c, v);

                        if (d->setflags(raw)) {
                            d->writeRegister(d->REG_N, n);
                            d->writeRegister(d->REG_Z, z);
                            d->writeRegister(d->REG_C, c);
                            d->writeRegister(d->REG_V, v);
                        }

                        if (EXTR(0, 4) == 31 && !d->setflags(raw)) {
                            d->writeRegister(d->REG_SP, result);
                        } else {
                            d->write(args[0], result);
                        }

                    }
                };

                struct IP_csel_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
			BaseSemantics::SValuePtr op2 = operand2;
			if ((EXTR(30, 30) == 1)) {
			    op2 = d->NOT(op2);
			}
			if ((EXTR(10, 10) == 1)) {
			    op2 = ops->add(op2, ops->number_(32, 1));
			}
			result = ops->ite(
                                ops->isEqual(d->ConditionHolds(ops->number_(32, d->getConditionVal(raw))), ops->boolean_(true)),
                                operand1,
				op2);
                        d->write(args[0], result);

                    }
                };

                struct IP_cls_int_execute : P {                                             //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);

                        if (d->opcode(raw) == CountOp_CLZ) {
                            result = d->CountLeadingZeroBits(operand1);
                        } else {
                            result = d->CountLeadingSignBits(operand1);
                        }
                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

                struct IP_clz_int_execute : P {                                             //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);

                        if (d->opcode(raw) == CountOp_CLZ) {
                            result = d->CountLeadingZeroBits(operand1);
                        } else {
                            result = d->CountLeadingSignBits(operand1);
                        }
                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

                struct IP_madd_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->UInt(operand3),
                                              ops->negate(ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2))));
                        } else {
                            result = ops->add(d->UInt(operand3),
                                              ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2)));
                        }
                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

                struct IP_msub_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->UInt(operand3),
                                              ops->negate(ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2))));
                        } else {
                            result = ops->add(d->UInt(operand3),
                                              ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2)));
                        }
                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

                struct IP_mneg_msub_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->UInt(operand3),
                                              ops->negate(ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2))));
                        } else {
                            result = ops->add(d->UInt(operand3),
                                              ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2)));
                        }
                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

                struct IP_mul_madd_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->UInt(operand3),
                                              ops->negate(ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2))));
                        } else {
                            result = ops->add(d->UInt(operand3),
                                              ops->unsignedMultiply(d->UInt(operand1), d->UInt(operand2)));
                        }
                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

                struct IP_smaddl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_smsubl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_smnegl_smsubl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_smulh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr result;
                        result = ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true));
                        d->write(args[0], ops->extract(result, 64, 127 + 1));

                    }
                };

                struct IP_smull_smaddl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_umaddl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_umsubl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_umnegl_umsubl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_umulh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr result;
                        result = ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true));
                        d->write(args[0], ops->extract(result, 64, 127 + 1));

                    }
                };

                struct IP_umull_umaddl_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr operand3 = d->read(args[3]);
                        BaseSemantics::SValuePtr result;

                        if (d->subop(raw)) {
                            result = ops->add(d->Int(operand3, true), ops->negate(
                                    ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true))));
                        } else {
                            result = ops->add(d->Int(operand3, true),
                                              ops->unsignedMultiply(d->Int(operand1, true), d->Int(operand2, true)));
                        }
                        d->write(args[0], ops->extract(result, 0, 63 + 1));

                    }
                };

                struct IP_ldar_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        int dbytes = d->getDatasize(raw) / 8;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {
                                data = d->read(args[0]);
                                d->writeMemory(address, dbytes, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, dbytes);
                                d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                            }
                                break;
                        }

                    }
                };

                struct IP_ldarb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        int dbytes = d->getDatasize(raw) / 8;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {
                                data = d->read(args[0]);
                                d->writeMemory(address, dbytes, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, dbytes);
                                d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                            }
                                break;
                        }

                    }
                };

                struct IP_ldarh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        int dbytes = d->getDatasize(raw) / 8;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {
                                data = d->read(args[0]);
                                d->writeMemory(address, dbytes, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, dbytes);
                                d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                            }
                                break;
                        }

                    }
                };

                struct IP_stlr_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        int dbytes = d->getDatasize(raw) / 8;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {
                                data = d->read(args[0]);
                                d->writeMemory(address, dbytes, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, dbytes);
                                d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                            }
                                break;
                        }

                    }
                };

                struct IP_stlrb_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        int dbytes = d->getDatasize(raw) / 8;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {
                                data = d->read(args[0]);
                                d->writeMemory(address, dbytes, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, dbytes);
                                d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                            }
                                break;
                        }

                    }
                };

                struct IP_stlrh_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->effectiveAddress(args[1]);
                        BaseSemantics::SValuePtr data;
                        int dbytes = d->getDatasize(raw) / 8;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {
                                data = d->read(args[0]);
                                d->writeMemory(address, dbytes, data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, dbytes);
                                d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                            }
                                break;
                        }

                    }
                };

                struct IP_udiv_execute : P {                    //
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr result;
			result = d->RoundTowardsZero(ops->unsignedDivide(d->Int(operand1, true), d->Int(operand2, true)));

			result = ops->ite(
                                ops->isEqual(d->isZero(operand2), ops->boolean_(true)),
                                d->Zeros(64),
				result);

                        d->write(args[0], ops->extract(result, 0, d->getDatasize(raw) - 1 + 1));

                    }
                };

            } // namespace

/*******************************************************************************************************************************
 *                                      DispatcherARM64
 *******************************************************************************************************************************/

            void
            DispatcherARM64::iproc_init() {
                iproc_set(rose_aarch64_op_add_addsub_imm, new ARM64::IP_add_addsub_imm_execute);
                iproc_set(rose_aarch64_op_adds_addsub_imm, new ARM64::IP_adds_addsub_imm_execute);
                iproc_set(rose_aarch64_op_sub_addsub_imm, new ARM64::IP_sub_addsub_imm_execute);
                iproc_set(rose_aarch64_op_subs_addsub_imm, new ARM64::IP_subs_addsub_imm_execute);
                iproc_set(rose_aarch64_op_add_addsub_ext, new ARM64::IP_add_addsub_ext_execute);
                iproc_set(rose_aarch64_op_adds_addsub_ext, new ARM64::IP_adds_addsub_ext_execute);
                iproc_set(rose_aarch64_op_sub_addsub_ext, new ARM64::IP_sub_addsub_ext_execute);
                iproc_set(rose_aarch64_op_subs_addsub_ext, new ARM64::IP_subs_addsub_ext_execute);
                iproc_set(rose_aarch64_op_add_addsub_shift, new ARM64::IP_add_addsub_shift_execute);
                iproc_set(rose_aarch64_op_adds_addsub_shift, new ARM64::IP_adds_addsub_shift_execute);
                iproc_set(rose_aarch64_op_sub_addsub_shift, new ARM64::IP_sub_addsub_shift_execute);
                iproc_set(rose_aarch64_op_subs_addsub_shift, new ARM64::IP_subs_addsub_shift_execute);
                iproc_set(rose_aarch64_op_adc, new ARM64::IP_adc_execute);
                iproc_set(rose_aarch64_op_adcs, new ARM64::IP_adcs_execute);
                iproc_set(rose_aarch64_op_adr, new ARM64::IP_adr_execute);
                iproc_set(rose_aarch64_op_adrp, new ARM64::IP_adrp_execute);
                iproc_set(rose_aarch64_op_b_uncond, new ARM64::IP_b_uncond_execute);
                iproc_set(rose_aarch64_op_b_cond, new ARM64::IP_b_cond_execute);
                iproc_set(rose_aarch64_op_br, new ARM64::IP_br_execute);
                iproc_set(rose_aarch64_op_blr, new ARM64::IP_blr_execute);
                iproc_set(rose_aarch64_op_bl, new ARM64::IP_bl_execute);
                iproc_set(rose_aarch64_op_cbz, new ARM64::IP_cbz_execute);
                iproc_set(rose_aarch64_op_cbnz, new ARM64::IP_cbnz_execute);
                iproc_set(rose_aarch64_op_tbz, new ARM64::IP_tbz_execute);
                iproc_set(rose_aarch64_op_tbnz, new ARM64::IP_tbnz_execute);
                iproc_set(rose_aarch64_op_cmp_subs_addsub_imm, new ARM64::IP_cmp_subs_addsub_imm_execute);
                iproc_set(rose_aarch64_op_cmp_subs_addsub_ext, new ARM64::IP_cmp_subs_addsub_ext_execute);
                iproc_set(rose_aarch64_op_cmp_subs_addsub_shift, new ARM64::IP_cmp_subs_addsub_shift_execute);
                iproc_set(rose_aarch64_op_cmn_adds_addsub_imm, new ARM64::IP_cmn_adds_addsub_imm_execute);
                iproc_set(rose_aarch64_op_cmn_adds_addsub_ext, new ARM64::IP_cmn_adds_addsub_ext_execute);
                iproc_set(rose_aarch64_op_cmn_adds_addsub_shift, new ARM64::IP_cmn_adds_addsub_shift_execute);
                iproc_set(rose_aarch64_op_ccmn_reg, new ARM64::IP_ccmn_reg_execute);
                iproc_set(rose_aarch64_op_ccmn_imm, new ARM64::IP_ccmn_imm_execute);
                iproc_set(rose_aarch64_op_ldr_imm_gen, new ARM64::IP_ldr_imm_gen_execute);
                iproc_set(rose_aarch64_op_str_imm_gen, new ARM64::IP_str_imm_gen_execute);
                iproc_set(rose_aarch64_op_ldrb_imm, new ARM64::IP_ldrb_imm_execute);
                iproc_set(rose_aarch64_op_strb_imm, new ARM64::IP_strb_imm_execute);
                iproc_set(rose_aarch64_op_ldrh_imm, new ARM64::IP_ldrh_imm_execute);
                iproc_set(rose_aarch64_op_ldrsb_imm, new ARM64::IP_ldrsb_imm_execute);
                iproc_set(rose_aarch64_op_ldrsh_imm, new ARM64::IP_ldrsh_imm_execute);
                iproc_set(rose_aarch64_op_ldrsw_imm, new ARM64::IP_ldrsw_imm_execute);
                iproc_set(rose_aarch64_op_strh_imm, new ARM64::IP_strh_imm_execute);
                iproc_set(rose_aarch64_op_ldr_reg_gen, new ARM64::IP_ldr_reg_gen_execute);
                iproc_set(rose_aarch64_op_ldrb_reg, new ARM64::IP_ldrb_reg_execute);
                iproc_set(rose_aarch64_op_ldrh_reg, new ARM64::IP_ldrh_reg_execute);
                iproc_set(rose_aarch64_op_ldrsb_reg, new ARM64::IP_ldrsb_reg_execute);
                iproc_set(rose_aarch64_op_ldrsh_reg, new ARM64::IP_ldrsh_reg_execute);
                iproc_set(rose_aarch64_op_ldrsw_reg, new ARM64::IP_ldrsw_reg_execute);
                iproc_set(rose_aarch64_op_str_reg_gen, new ARM64::IP_str_reg_gen_execute);
                iproc_set(rose_aarch64_op_strb_reg, new ARM64::IP_strb_reg_execute);
                iproc_set(rose_aarch64_op_strh_reg, new ARM64::IP_strh_reg_execute);
                iproc_set(rose_aarch64_op_ldr_lit_gen, new ARM64::IP_ldr_lit_gen_execute);
                iproc_set(rose_aarch64_op_ldrsw_lit, new ARM64::IP_ldrsw_lit_execute);
                iproc_set(rose_aarch64_op_ubfm, new ARM64::IP_ubfm_execute);
                iproc_set(rose_aarch64_op_uxtb_ubfm, new ARM64::IP_uxtb_ubfm_execute);
                iproc_set(rose_aarch64_op_uxth_ubfm, new ARM64::IP_uxth_ubfm_execute);
                iproc_set(rose_aarch64_op_ubfiz_ubfm, new ARM64::IP_ubfiz_ubfm_execute);
                iproc_set(rose_aarch64_op_ubfx_ubfm, new ARM64::IP_ubfx_ubfm_execute);
                iproc_set(rose_aarch64_op_sbfm, new ARM64::IP_sbfm_execute);
                iproc_set(rose_aarch64_op_sxth_sbfm, new ARM64::IP_sxth_sbfm_execute);
                iproc_set(rose_aarch64_op_sxtb_sbfm, new ARM64::IP_sxtb_sbfm_execute);
                iproc_set(rose_aarch64_op_sxtw_sbfm, new ARM64::IP_sxtw_sbfm_execute);
                iproc_set(rose_aarch64_op_sbfiz_sbfm, new ARM64::IP_sbfiz_sbfm_execute);
                iproc_set(rose_aarch64_op_sbfx_sbfm, new ARM64::IP_sbfx_sbfm_execute);
                iproc_set(rose_aarch64_op_fmov_float_gen, new ARM64::IP_fmov_float_gen_execute);
                iproc_set(rose_aarch64_op_movz, new ARM64::IP_movz_execute);
                iproc_set(rose_aarch64_op_mov_movz, new ARM64::IP_mov_movz_execute);
                iproc_set(rose_aarch64_op_movn, new ARM64::IP_movn_execute);
                iproc_set(rose_aarch64_op_mov_movn, new ARM64::IP_mov_movn_execute);
                iproc_set(rose_aarch64_op_movk, new ARM64::IP_movk_execute);
                iproc_set(rose_aarch64_op_mov_orr_log_shift, new ARM64::IP_mov_orr_log_shift_execute);
                iproc_set(rose_aarch64_op_mov_orr_log_imm, new ARM64::IP_mov_orr_log_imm_execute);
                iproc_set(rose_aarch64_op_orr_log_shift, new ARM64::IP_orr_log_shift_execute);
                iproc_set(rose_aarch64_op_orn_log_shift, new ARM64::IP_orn_log_shift_execute);
                iproc_set(rose_aarch64_op_orr_log_imm, new ARM64::IP_orr_log_imm_execute);
                iproc_set(rose_aarch64_op_and_log_imm, new ARM64::IP_and_log_imm_execute);
                iproc_set(rose_aarch64_op_and_log_shift, new ARM64::IP_and_log_shift_execute);
                iproc_set(rose_aarch64_op_ands_log_imm, new ARM64::IP_ands_log_imm_execute);
                iproc_set(rose_aarch64_op_ands_log_shift, new ARM64::IP_ands_log_shift_execute);
                iproc_set(rose_aarch64_op_eor_log_shift, new ARM64::IP_eor_log_shift_execute);
                iproc_set(rose_aarch64_op_eor_log_imm, new ARM64::IP_eor_log_imm_execute);
                iproc_set(rose_aarch64_op_eon, new ARM64::IP_eon_execute);
                iproc_set(rose_aarch64_op_lsl_ubfm, new ARM64::IP_lsl_ubfm_execute);
                iproc_set(rose_aarch64_op_lsr_ubfm, new ARM64::IP_lsr_ubfm_execute);
                iproc_set(rose_aarch64_op_asr_sbfm, new ARM64::IP_asr_sbfm_execute);
                iproc_set(rose_aarch64_op_bfm, new ARM64::IP_bfm_execute);
                iproc_set(rose_aarch64_op_bfxil_bfm, new ARM64::IP_bfxil_bfm_execute);
                iproc_set(rose_aarch64_op_bfi_bfm, new ARM64::IP_bfi_bfm_execute);
                iproc_set(rose_aarch64_op_bic_log_shift, new ARM64::IP_bic_log_shift_execute);
                iproc_set(rose_aarch64_op_bics, new ARM64::IP_bics_execute);
                iproc_set(rose_aarch64_op_stp_gen, new ARM64::IP_stp_gen_execute);
                iproc_set(rose_aarch64_op_ldp_gen, new ARM64::IP_ldp_gen_execute);
                iproc_set(rose_aarch64_op_ldpsw, new ARM64::IP_ldpsw_execute);
                iproc_set(rose_aarch64_op_ldnp_gen, new ARM64::IP_ldnp_gen_execute);
                iproc_set(rose_aarch64_op_stnp_gen, new ARM64::IP_stnp_gen_execute);
                iproc_set(rose_aarch64_op_ldtr, new ARM64::IP_ldtr_execute);
                iproc_set(rose_aarch64_op_ldtrb, new ARM64::IP_ldtrb_execute);
                iproc_set(rose_aarch64_op_ldtrh, new ARM64::IP_ldtrh_execute);
                iproc_set(rose_aarch64_op_ldtrsb, new ARM64::IP_ldtrsb_execute);
                iproc_set(rose_aarch64_op_ldtrsh, new ARM64::IP_ldtrsh_execute);
                iproc_set(rose_aarch64_op_sttr, new ARM64::IP_sttr_execute);
                iproc_set(rose_aarch64_op_sttrb, new ARM64::IP_sttrb_execute);
                iproc_set(rose_aarch64_op_sttrh, new ARM64::IP_sttrh_execute);
                iproc_set(rose_aarch64_op_ldur_gen, new ARM64::IP_ldur_gen_execute);
                iproc_set(rose_aarch64_op_ldurb, new ARM64::IP_ldurb_execute);
                iproc_set(rose_aarch64_op_ldurh, new ARM64::IP_ldurh_execute);
                iproc_set(rose_aarch64_op_ldursb, new ARM64::IP_ldursb_execute);
                iproc_set(rose_aarch64_op_ldursh, new ARM64::IP_ldursh_execute);
                iproc_set(rose_aarch64_op_ldursw, new ARM64::IP_ldursw_execute);
                iproc_set(rose_aarch64_op_sturb, new ARM64::IP_sturb_execute);
                iproc_set(rose_aarch64_op_sturh, new ARM64::IP_sturh_execute);
                iproc_set(rose_aarch64_op_stur_gen, new ARM64::IP_stur_gen_execute);
                iproc_set(rose_aarch64_op_asr_asrv, new ARM64::IP_asr_asrv_execute);
                iproc_set(rose_aarch64_op_asrv, new ARM64::IP_asrv_execute);
                iproc_set(rose_aarch64_op_lsl_lslv, new ARM64::IP_lsl_lslv_execute);
                iproc_set(rose_aarch64_op_lslv, new ARM64::IP_lslv_execute);
                iproc_set(rose_aarch64_op_lsr_lsrv, new ARM64::IP_lsr_lsrv_execute);
                iproc_set(rose_aarch64_op_lsrv, new ARM64::IP_lsrv_execute);
                iproc_set(rose_aarch64_op_ror_rorv, new ARM64::IP_ror_rorv_execute);
                iproc_set(rose_aarch64_op_rorv, new ARM64::IP_rorv_execute);
                iproc_set(rose_aarch64_op_tst_ands_log_imm, new ARM64::IP_tst_ands_log_imm_execute);
                iproc_set(rose_aarch64_op_tst_ands_log_shift, new ARM64::IP_tst_ands_log_shift_execute);
                iproc_set(rose_aarch64_op_sbc, new ARM64::IP_sbc_execute);
                iproc_set(rose_aarch64_op_sbcs, new ARM64::IP_sbcs_execute);
                iproc_set(rose_aarch64_op_ngc_sbc, new ARM64::IP_ngc_sbc_execute);
                iproc_set(rose_aarch64_op_ngcs_sbcs, new ARM64::IP_ngcs_sbcs_execute);
                iproc_set(rose_aarch64_op_neg_sub_addsub_shift, new ARM64::IP_neg_sub_addsub_shift_execute);
                iproc_set(rose_aarch64_op_negs_subs_addsub_shift, new ARM64::IP_negs_subs_addsub_shift_execute);
                iproc_set(rose_aarch64_op_mvn_orn_log_shift, new ARM64::IP_mvn_orn_log_shift_execute);
                iproc_set(rose_aarch64_op_mov_add_addsub_imm, new ARM64::IP_mov_add_addsub_imm_execute);
                iproc_set(rose_aarch64_op_csinv, new ARM64::IP_csel_execute);
                iproc_set(rose_aarch64_op_csinc, new ARM64::IP_csel_execute);
                iproc_set(rose_aarch64_op_csneg, new ARM64::IP_csel_execute);
                iproc_set(rose_aarch64_op_csel, new ARM64::IP_csel_execute);
                iproc_set(rose_aarch64_op_cls_int, new ARM64::IP_cls_int_execute);
                iproc_set(rose_aarch64_op_clz_int, new ARM64::IP_clz_int_execute);
                iproc_set(rose_aarch64_op_madd, new ARM64::IP_madd_execute);
                iproc_set(rose_aarch64_op_msub, new ARM64::IP_msub_execute);
                iproc_set(rose_aarch64_op_mneg_msub, new ARM64::IP_mneg_msub_execute);
                iproc_set(rose_aarch64_op_mul_madd, new ARM64::IP_mul_madd_execute);
                iproc_set(rose_aarch64_op_smaddl, new ARM64::IP_smaddl_execute);
                iproc_set(rose_aarch64_op_smsubl, new ARM64::IP_smsubl_execute);
                iproc_set(rose_aarch64_op_smnegl_smsubl, new ARM64::IP_smnegl_smsubl_execute);
                iproc_set(rose_aarch64_op_smulh, new ARM64::IP_smulh_execute);
                iproc_set(rose_aarch64_op_smull_smaddl, new ARM64::IP_smull_smaddl_execute);
                iproc_set(rose_aarch64_op_umaddl, new ARM64::IP_umaddl_execute);
                iproc_set(rose_aarch64_op_umsubl, new ARM64::IP_umsubl_execute);
                iproc_set(rose_aarch64_op_umnegl_umsubl, new ARM64::IP_umnegl_umsubl_execute);
                iproc_set(rose_aarch64_op_umulh, new ARM64::IP_umulh_execute);
                iproc_set(rose_aarch64_op_umull_umaddl, new ARM64::IP_umull_umaddl_execute);
                iproc_set(rose_aarch64_op_ldar, new ARM64::IP_ldar_execute);
                iproc_set(rose_aarch64_op_ldarb, new ARM64::IP_ldarb_execute);
                iproc_set(rose_aarch64_op_ldarh, new ARM64::IP_ldarh_execute);
                iproc_set(rose_aarch64_op_stlr, new ARM64::IP_stlr_execute);
                iproc_set(rose_aarch64_op_stlrb, new ARM64::IP_stlrb_execute);
                iproc_set(rose_aarch64_op_stlrh, new ARM64::IP_stlrh_execute);
                iproc_set(rose_aarch64_op_udiv, new ARM64::IP_udiv_execute);
                iproc_set(rose_aarch64_op_sdiv, new ARM64::IP_udiv_execute);
            }

            void
            DispatcherARM64::regcache_init() {
                if (regdict) {
                    REG_PC = findRegister("pc", 64);
                    REG_N = findRegister("n", 1);
                    REG_Z = findRegister("z", 1);
                    REG_C = findRegister("c", 1);
                    REG_V = findRegister("v", 1);
                    REG_SP = findRegister("sp", 64);
                }
            }

            void
            DispatcherARM64::memory_init() {
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
            DispatcherARM64::instructionPointerRegister() const {
                return REG_PC;
            }

            RegisterDescriptor
            DispatcherARM64::stackPointerRegister() const {
                return REG_SP;
            }

            static bool
            isStatusRegister(const RegisterDescriptor &reg) {
                return reg.get_major() == armv8_regclass_pstate && reg.get_minor() == 0;
            }

            RegisterDictionary::RegisterDescriptors
            DispatcherARM64::get_usual_registers() const {
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
            DispatcherARM64::set_register_dictionary(const RegisterDictionary *regdict) {
                BaseSemantics::Dispatcher::set_register_dictionary(regdict);
                regcache_init();
            }

            void
            DispatcherARM64::setFlagsForResult(const BaseSemantics::SValuePtr &result,
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
            DispatcherARM64::parity(const BaseSemantics::SValuePtr &v) {
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
            DispatcherARM64::invertMaybe(const BaseSemantics::SValuePtr &value, bool maybe) {
                return maybe ? operators->invert(value) : value;
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::isZero(const BaseSemantics::SValuePtr &value) {
                return operators->equalToZero(value);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::ConditionHolds(const BaseSemantics::SValuePtr &cond) {
                Dyninst::AST::Ptr condExpr = SymEvalSemantics::SValue::promote(cond)->get_expression();
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
                    result = operators->invert(result);

                return result;
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::NOT(const BaseSemantics::SValuePtr &expr) {
                return operators->invert(expr);
            }

            void
            DispatcherARM64::BranchTo(const BaseSemantics::SValuePtr &target) {
                ASSERT_require(target != NULL);
                writeRegister(REG_PC, operators->extract(target, 0, 64));
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::Zeros(const unsigned int nbits) {
                ASSERT_require(nbits > 0);
                return operators->number_(nbits, 0);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::SignExtend(const BaseSemantics::SValuePtr &expr, size_t newsize) {
                ASSERT_require(newsize > 0);
                return operators->signExtend(expr, newsize);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::ZeroExtend(const BaseSemantics::SValuePtr &expr, size_t newsize) {
                ASSERT_require(newsize > 0);
                return operators->unsignedExtend(expr, newsize);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::ROR(const BaseSemantics::SValuePtr &expr, const BaseSemantics::SValuePtr &amt) {
                ASSERT_not_null(amt);
                return operators->rotateRight(expr, amt);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::Replicate(const BaseSemantics::SValuePtr &expr) {
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
            DispatcherARM64::getBitfieldMask(int immr, int imms,
                                             int N, bool iswmask, int datasize) {
                int hsbarg = (N << 6) | (~imms);
                int len = 0;
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
            DispatcherARM64::getRegSize(uint32_t raw) {
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
            DispatcherARM64::ldStrLiteralAccessSize(uint32_t raw) {
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
            DispatcherARM64::inzero(uint32_t raw) {
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
            DispatcherARM64::extend(uint32_t raw) {
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
            DispatcherARM64::op(uint32_t raw) {
                switch (IntegerOps::extract2<uint32_t>(29, 30, raw)) {
                    case 0:
                    case 3:
                        return static_cast<int>(ARM64::InsnProcessor::LogicalOp_AND);
                    case 1:
                        return static_cast<int>(ARM64::InsnProcessor::LogicalOp_ORR);
                    case 2:
                        return static_cast<int>(ARM64::InsnProcessor::LogicalOp_EOR);
                    default:
                        assert(!"Invalid code for opc field in logical OR instruction variant!");
                        break;
                }
            }

            bool
            DispatcherARM64::setflags(uint32_t raw) {
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
            DispatcherARM64::getDatasize(uint32_t raw) {
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
            DispatcherARM64::getShiftType(uint32_t raw) {
                int v10_11 = IntegerOps::extract2(10, 11, raw);

                switch(v10_11) {
                    case 0: return static_cast<int>(ARM64::InsnProcessor::ShiftType_LSL);
                    case 1: return static_cast<int>(ARM64::InsnProcessor::ShiftType_LSR);
                    case 2: return static_cast<int>(ARM64::InsnProcessor::ShiftType_ASR);
                    case 3: return static_cast<int>(ARM64::InsnProcessor::ShiftType_ROR);
                    default: ASSERT_not_reachable("Cannot have a value greater than 3 for a 2-bit field (field: op2, bits: 10..11)!");
                }
            }

            int
            DispatcherARM64::getConditionVal(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(24, 31, raw) == 0x54 && IntegerOps::extract2<uint32_t>(4, 4, raw) == 0)
                    return IntegerOps::extract2(0, 3, raw);
                else
                    return IntegerOps::extract2(12, 15, raw);
            }

            int
            DispatcherARM64::opcode(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(23, 28, raw) == 0x25)
                    return IntegerOps::extract2<uint32_t>(29, 30, raw);
                else
                    return IntegerOps::extract2<uint32_t>(10, 10, raw);
            }

            bool
            DispatcherARM64::subop(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(24, 28, raw) == 0x1B) {
                    return IntegerOps::extract2<uint32_t>(15, 15, raw) == 0x1;
                } else {
                    return IntegerOps::extract2<uint32_t>(30, 30, raw) == 0x1;
                }
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::doAddOperation(BaseSemantics::SValuePtr a, BaseSemantics::SValuePtr b,
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
            DispatcherARM64::effectiveAddress(SgAsmExpression *e, size_t nbits) {
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
                } else if (SgAsmBinaryMultiply *op = isSgAsmBinaryMultiply(addressExpression)) {
                    BaseSemantics::SValuePtr lhs = effectiveAddress(op->get_lhs(), nbits);
                    BaseSemantics::SValuePtr rhs = effectiveAddress(op->get_rhs(), nbits);
                    retval = operators->unsignedMultiply(lhs, rhs);
                } else if (SgAsmBinaryLsl *lshift = isSgAsmBinaryLsl(addressExpression)) {
                    SgAsmExpression *lhs = lshift->get_lhs();
                    SgAsmExpression *rhs = lshift->get_rhs();
                    size_t nbits = std::max(lhs->get_nBits(), rhs->get_nBits());
                    retval = operators->shiftLeft(read(lhs, lhs->get_nBits()), read(rhs, rhs->get_nBits()));
                } else if (SgAsmIntegerValueExpression *ival = isSgAsmIntegerValueExpression(addressExpression)) {
                    retval = operators->number_(ival->get_significantBits(), ival->get_value());
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
            DispatcherARM64::readRegister(const RegisterDescriptor &reg) {
                if (reg.get_major() == armv8_regclass_gpr &&
                    reg.get_minor() == armv8_gpr_zr) {
                    return operators->number_(reg.get_nbits(), 0);
                }

                return operators->readRegister(reg);
            }

            void
            DispatcherARM64::writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value) {
                if (reg.get_major() == armv8_regclass_gpr || reg.get_major() == armv8_regclass_pc ||
                    reg.get_major() == armv8_regclass_sp) {
                    if (reg.get_minor() == armv8_gpr_zr) {
                        return;
                    }

                    if (reg.get_nbits() == 32 && reg.get_offset() == 0) {
                        RegisterDescriptor upperHalf(reg.get_major(), reg.get_minor(), 32, 32);
                        operators->writeRegister(upperHalf, operators->number_(32, 0));
                    }
                }

                operators->writeRegister(reg, value);
            }

            void
            DispatcherARM64::write(SgAsmExpression *e, const BaseSemantics::SValuePtr &value,
                                   size_t addr_nbits/*=0*/) {
                if (SgAsmDirectRegisterExpression *re = isSgAsmDirectRegisterExpression(e)) {
                    writeRegister(re->get_descriptor(), value);
                } else {
                    Dispatcher::write(e, value, addr_nbits);        // defer to super class
                }
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::fixMemoryAddress(const BaseSemantics::SValuePtr &addr) const {
                if (size_t addrWidth = addressWidth()) {
                    if (addr->get_width() < addrWidth)
                        return operators->signExtend(addr, addrWidth);
                    if (addr->get_width() > addrWidth)
                        return operators->unsignedExtend(addr, addrWidth);
                }
                return addr;
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::readMemory(const BaseSemantics::SValuePtr &addr, size_t readSize) {
                SymEvalSemantics::StateASTPtr state = SymEvalSemantics::StateAST::promote(operators->currentState());

                //The second, third and fourth arguments will remain unused
                return state->readMemory(addr, operators->unspecified_(1), NULL, NULL, readSize);
            }

            void
            DispatcherARM64::writeMemory(const BaseSemantics::SValuePtr &addr, size_t writeSize, const BaseSemantics::SValuePtr &data) {
                SymEvalSemantics::StateASTPtr state = SymEvalSemantics::StateAST::promote(operators->currentState());

                //The third and fourth arguments will remain unused
                state->writeMemory(addr, data, NULL, NULL, writeSize);
            }

            SgAsmExpression *
            DispatcherARM64::getWriteBackTarget(SgAsmExpression *expr) {
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
            DispatcherARM64::UInt(const BaseSemantics::SValuePtr &expr) {
                ASSERT_not_null(expr);
                return expr;
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::ShiftReg(const BaseSemantics::SValuePtr &src, int shiftType, const BaseSemantics::SValuePtr &amount) {
                ASSERT_not_null(amount);

                switch(static_cast<ARM64::InsnProcessor::ShiftType>(shiftType)) {
                    case ARM64::InsnProcessor::ShiftType_LSL: return operators->shiftLeft(src, amount);
                    case ARM64::InsnProcessor::ShiftType_LSR: return operators->shiftRight(src, amount);
                    case ARM64::InsnProcessor::ShiftType_ASR: return operators->shiftRightArithmetic(src, amount);
                    case ARM64::InsnProcessor::ShiftType_ROR: return operators->rotateRight(src, amount);
                    default: ASSERT_not_reachable("Found invalid shift type for shift instruction!");
                }
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::CountLeadingZeroBits(const BaseSemantics::SValuePtr &expr) {
                size_t len = expr->get_width();

                for(int idx = len - 1; idx >= 0; idx--)
                    if(operators->isEqual(operators->extract(expr, len, len + 1), operators->number_(1, 1)))
                        return operators->number_(expr->get_width(), len - 1 - idx);

                return operators->number_(expr->get_width(), len);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::CountLeadingSignBits(const BaseSemantics::SValuePtr &expr) {
                size_t len = expr->get_width();
                BaseSemantics::SValuePtr arg = operators->xor_(operators->extract(expr, 1, len), operators->extract(expr, 0, len - 1));
                return CountLeadingZeroBits(arg);
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::Int(const BaseSemantics::SValuePtr &expr, bool isUnsigned) {
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
            DispatcherARM64::RoundTowardsZero(const BaseSemantics::SValuePtr &expr) {
                return expr;
            }
        } // namespace
    } // namespace
} // namespace

//using namespace rose::Diagnostics;
