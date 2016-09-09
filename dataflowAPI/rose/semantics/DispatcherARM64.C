#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "BaseSemantics2.h"
//#include "Diagnostics.h"
#include "DispatcherARM64.h"
#include "../integerOps.h"
#include "SymEvalSemantics.h"

#undef si_value                                         // name pollution from siginfo.h

//using namespace rose::Diagnostics;

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
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_adds_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_sub_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_subs_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_add_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_adds_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_sub_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_subs_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_add_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_adds_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_sub_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_subs_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_adc_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        result =
                                d->doAddOperation(operand1, operand2,
                                                  ops->and_(d->readRegister(d->REG_NZCV),
                                                            ops->number_(32, 0x2)),
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_adcs_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);
                        }

                        result =
                                d->doAddOperation(operand1, operand2,
                                                  ops->and_(d->readRegister(d->REG_NZCV),
                                                            ops->number_(32, 0x2)),
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_adr_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr base = d->readRegister(d->REG_PC);

                        if ((EXTR (31, 31) == 1)) {
                            base =
                                    ops->or_(ops->and_(base, ops->number_(12, 0xfff)),
                                             d->Zeros(12));
                        }

                        d->write(args[0], ops->add(base, d->read(args[0])));
                    }
                };

                struct IP_adrp_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr base = d->readRegister(d->REG_PC);

                        if ((EXTR (31, 31) == 1)) {
                            base =
                                    ops->or_(ops->and_(base, ops->number_(12, 0xfff)),
                                             d->Zeros(12));
                        }

                        d->write(args[0], ops->add(base, d->read(args[0])));
                    }
                };

                struct IP_b_uncond_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        if (EXTR (31, 31) == 1)
                            d->writeRegister(d->findRegister("x30", 64),
                                             ops->add(d->readRegister(d->REG_PC),
                                                      ops->number_(32, 4)));

                        d->BranchTo(d->read(args[0]));
                    }
                };

                struct IP_b_cond_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        d->BranchTo(ops->ite(
                                ops->isEqual(d->ConditionHolds(ops->number_(4, EXTR (0, 4))), ops->boolean_(true)),
                                d->read(args[0]), d->readRegister(d->REG_PC)));
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
                                ops->and_(ops->shiftRight(operand, d->read(args[0])), ops->number_(1, 1)),
                                ops->number_(1, EXTR (24, 24))),
                                             d->read(args[2]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_tbnz_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand = d->read(args[0]);
                        d->BranchTo(ops->ite(ops->isEqual(
                                ops->and_(ops->shiftRight(operand, d->read(args[0])), ops->number_(1, 1)),
                                ops->number_(1, EXTR (24, 24))),
                                             d->read(args[2]), d->readRegister(d->REG_PC)));
                    }
                };

                struct IP_cmp_subs_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_cmp_subs_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_cmp_subs_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        d->write(args[0], result);
                    }
                };

                struct IP_cmn_adds_addsub_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_cmn_adds_addsub_ext_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
                        }

                        if (EXTR (0, 4) == 31 && !(EXTR (29, 29) == 1)) {
                            d->write(args[0], result);
                        }
                        else {
                            d->write(args[0], result);
                        }
                    }
                };

                struct IP_cmn_adds_addsub_shift_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr operand1 = d->read(args[1]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[2]);
                        BaseSemantics::SValuePtr nzcv;

                        bool carry_in;

                        if ((EXTR (30, 30) == 1)) {
                            operand2 = d->NOT(operand2);

                            carry_in = true;

                        }
                        else {
                            carry_in = false;
                        }

                        result =
                                d->doAddOperation(operand1, operand2, carry_in,
                                                  ops->boolean_(false), nzcv);

                        if ((EXTR (29, 29) == 1)) {
                            d->writeRegister(d->REG_NZCV, nzcv);
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
                                 d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), nzcv),
                                 ops->unspecified_(1));

                        d->writeRegister(d->REG_NZCV, nzcv);
                    }
                };

                struct IP_ccmn_imm_execute : P {                      //
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr operand1 = d->read(args[0]);
                        BaseSemantics::SValuePtr operand2 = d->read(args[1]);
                        BaseSemantics::SValuePtr nzcv = d->read(args[2]);
                        bool carry_in = false;

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
                                 d->doAddOperation(operand1, operand2, carry_in, ops->boolean_(false), nzcv),
                                 ops->unspecified_(1));

                        d->writeRegister(d->REG_NZCV, nzcv);
                    }
                };

                struct IP_ldr_imm_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_str_imm_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrb_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_strb_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrh_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrsb_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrsh_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrsw_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_strh_imm_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch (EXTR(22, 22) ^ EXTR(23, 23)) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldr_reg_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrb_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrh_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrsb_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrsh_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldrsw_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_str_reg_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_strb_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_strh_reg_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr offset = d->read(args[2]);
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;
                        bool wb_unknown = false;
                        bool rt_unknown = false;

                        if (!(EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                            address = ops->add(address, d->read(args[2]));
                        }

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_STORE: {

                                if (rt_unknown) {
                                    data = ops->unspecified_(1);
                                }
                                else {
                                    data = d->read(args[0]);
                                }
                                d->writeMemory(address, 0x8 << EXTR(30, 31), data);
                            }
                                break;
                            case MemOp_LOAD: {
                                data = d->readMemory(address, 0x8 << EXTR(30, 31));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, d->getRegSize(raw)));
                                }
                                else {
                                    d->write(args[0], d->ZeroExtend(data, d->getRegSize(raw)));
                                }
                            }
                                break;
                        }

                        if ((EXTR(24, 24) == 0)) {

                            if (wb_unknown) {
                                address = ops->unspecified_(1);
                            }

                            else if ((EXTR(11, 11) == 0 && EXTR(24, 24) == 0)) {
                                address = ops->add(address, d->read(args[2]));
                            }

                            if (EXTR(5, 9) == 31) {
                                d->writeRegister(d->REG_SP, address);
                            }
                            else {
                                d->write(args[1], address);
                            }
                        }

                    }
                };

                struct IP_ldr_lit_gen_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->ldStrLiteralAccessSize(raw));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, 64));
                                }
                                else {
                                    d->write(args[0], data);
                                }
                            }
                                break;
                        }

                    }
                };

                struct IP_ldrsw_lit_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr address = d->read(args[1]);
                        BaseSemantics::SValuePtr data;

                        switch ((EXTR(22, 22) ^ EXTR(23, 23))) {
                            case MemOp_LOAD: {
                                data = d->readMemory(address, d->ldStrLiteralAccessSize(raw));

                                if ((EXTR(23, 23) == 1)) {
                                    d->write(args[0], d->SignExtend(data, 64));
                                }
                                else {
                                    d->write(args[0], data);
                                }
                            }
                                break;
                        }

                    }
                };


            } // namespace

/*******************************************************************************************************************************
 *                                      DispatcherARM64
 *******************************************************************************************************************************/

            void
            DispatcherARM64::iproc_init() {
                iproc_set (rose_aarch64_op_add_addsub_imm, new ARM64::IP_add_addsub_imm_execute);
                iproc_set (rose_aarch64_op_adds_addsub_imm, new ARM64::IP_adds_addsub_imm_execute);
                iproc_set (rose_aarch64_op_sub_addsub_imm, new ARM64::IP_sub_addsub_imm_execute);
                iproc_set (rose_aarch64_op_subs_addsub_imm, new ARM64::IP_subs_addsub_imm_execute);
                iproc_set (rose_aarch64_op_add_addsub_ext, new ARM64::IP_add_addsub_ext_execute);
                iproc_set (rose_aarch64_op_adds_addsub_ext, new ARM64::IP_adds_addsub_ext_execute);
                iproc_set (rose_aarch64_op_sub_addsub_ext, new ARM64::IP_sub_addsub_ext_execute);
                iproc_set (rose_aarch64_op_subs_addsub_ext, new ARM64::IP_subs_addsub_ext_execute);
                iproc_set (rose_aarch64_op_add_addsub_shift, new ARM64::IP_add_addsub_shift_execute);
                iproc_set (rose_aarch64_op_adds_addsub_shift, new ARM64::IP_adds_addsub_shift_execute);
                iproc_set (rose_aarch64_op_sub_addsub_shift, new ARM64::IP_sub_addsub_shift_execute);
                iproc_set (rose_aarch64_op_subs_addsub_shift, new ARM64::IP_subs_addsub_shift_execute);
                iproc_set (rose_aarch64_op_adc, new ARM64::IP_adc_execute);
                iproc_set (rose_aarch64_op_adcs, new ARM64::IP_adcs_execute);
                iproc_set (rose_aarch64_op_adr, new ARM64::IP_adr_execute);
                iproc_set (rose_aarch64_op_adrp, new ARM64::IP_adrp_execute);
                iproc_set (rose_aarch64_op_b_uncond, new ARM64::IP_b_uncond_execute);
                iproc_set (rose_aarch64_op_b_cond, new ARM64::IP_b_cond_execute);
                iproc_set (rose_aarch64_op_br, new ARM64::IP_br_execute);
                iproc_set (rose_aarch64_op_blr, new ARM64::IP_blr_execute);
                iproc_set (rose_aarch64_op_bl, new ARM64::IP_bl_execute);
                iproc_set (rose_aarch64_op_cbz, new ARM64::IP_cbz_execute);
                iproc_set (rose_aarch64_op_cbnz, new ARM64::IP_cbnz_execute);
                iproc_set (rose_aarch64_op_tbz, new ARM64::IP_tbz_execute);
                iproc_set (rose_aarch64_op_tbnz, new ARM64::IP_tbnz_execute);
                iproc_set (rose_aarch64_op_cmp_subs_addsub_imm, new ARM64::IP_cmp_subs_addsub_imm_execute);
                iproc_set (rose_aarch64_op_cmp_subs_addsub_ext, new ARM64::IP_cmp_subs_addsub_ext_execute);
                iproc_set (rose_aarch64_op_cmp_subs_addsub_shift, new ARM64::IP_cmp_subs_addsub_shift_execute);
                iproc_set (rose_aarch64_op_cmn_adds_addsub_imm, new ARM64::IP_cmn_adds_addsub_imm_execute);
                iproc_set (rose_aarch64_op_cmn_adds_addsub_ext, new ARM64::IP_cmn_adds_addsub_ext_execute);
                iproc_set (rose_aarch64_op_cmn_adds_addsub_shift, new ARM64::IP_cmn_adds_addsub_shift_execute);
                iproc_set (rose_aarch64_op_ccmn_reg, new ARM64::IP_ccmn_reg_execute);
                iproc_set (rose_aarch64_op_ccmn_imm, new ARM64::IP_ccmn_imm_execute);
                iproc_set (rose_aarch64_op_ldr_imm_gen, new ARM64::IP_ldr_imm_gen_execute);
                iproc_set (rose_aarch64_op_str_imm_gen, new ARM64::IP_str_imm_gen_execute);
                iproc_set (rose_aarch64_op_ldrb_imm, new ARM64::IP_ldrb_imm_execute);
                iproc_set (rose_aarch64_op_strb_imm, new ARM64::IP_strb_imm_execute);
                iproc_set (rose_aarch64_op_ldrh_imm, new ARM64::IP_ldrh_imm_execute);
                iproc_set (rose_aarch64_op_ldrsb_imm, new ARM64::IP_ldrsb_imm_execute);
                iproc_set (rose_aarch64_op_ldrsh_imm, new ARM64::IP_ldrsh_imm_execute);
                iproc_set (rose_aarch64_op_ldrsw_imm, new ARM64::IP_ldrsw_imm_execute);
                iproc_set (rose_aarch64_op_strh_imm, new ARM64::IP_strh_imm_execute);
                iproc_set (rose_aarch64_op_ldr_reg_gen, new ARM64::IP_ldr_reg_gen_execute);
                iproc_set (rose_aarch64_op_ldrb_reg, new ARM64::IP_ldrb_reg_execute);
                iproc_set (rose_aarch64_op_ldrh_reg, new ARM64::IP_ldrh_reg_execute);
                iproc_set (rose_aarch64_op_ldrsb_reg, new ARM64::IP_ldrsb_reg_execute);
                iproc_set (rose_aarch64_op_ldrsh_reg, new ARM64::IP_ldrsh_reg_execute);
                iproc_set (rose_aarch64_op_ldrsw_reg, new ARM64::IP_ldrsw_reg_execute);
                iproc_set (rose_aarch64_op_str_reg_gen, new ARM64::IP_str_reg_gen_execute);
                iproc_set (rose_aarch64_op_strb_reg, new ARM64::IP_strb_reg_execute);
                iproc_set (rose_aarch64_op_strh_reg, new ARM64::IP_strh_reg_execute);
                iproc_set (rose_aarch64_op_ldr_lit_gen, new ARM64::IP_ldr_lit_gen_execute);
                iproc_set (rose_aarch64_op_ldrsw_lit, new ARM64::IP_ldrsw_lit_execute);
            }

            void
            DispatcherARM64::regcache_init() {
                if (regdict) {
                    REG_PC = findRegister("pc", 64);
                    REG_NZCV = findRegister("nzcv", 4);
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
                return reg.get_major() == armv8_regclass_pstate && reg.get_minor() == armv8_pstatefield_nzcv;
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
                                               BaseSemantics::SValuePtr &nzcv) {
                size_t width = result->get_width();

                BaseSemantics::SValuePtr n = operators->extract(result, width - 1, width);
                BaseSemantics::SValuePtr z = operators->equalToZero(result);

                BaseSemantics::SValuePtr sign = operators->extract(carries, nbits - 1, nbits);
                BaseSemantics::SValuePtr ofbit = operators->extract(carries, nbits - 2, nbits - 1);
                BaseSemantics::SValuePtr c = invertMaybe(sign, invertCarries);
                BaseSemantics::SValuePtr v = operators->xor_(sign, ofbit);

                BaseSemantics::SValuePtr nz = operators->shiftLeft(
                        operators->or_(operators->shiftLeft(n, operators->number_(1, 1)), z),
                        operators->number_(8, 2));
                BaseSemantics::SValuePtr cv = operators->or_(operators->shiftLeft(c, operators->number_(1, 1)), v);

                nzcv = operators->extract(operators->or_(nz, cv), 0, 4);
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
                BaseSemantics::SValuePtr baseCond = operators->extract(cond, 1, 4);
                BaseSemantics::SValuePtr nzcvVal = readRegister(REG_NZCV);
                BaseSemantics::SValuePtr result;

                if (operators->isEqual(baseCond, operators->number_(3, 0)))
                    result = operators->isEqual(operators->extract(nzcvVal, 2, 3), operators->number_(1, 1));
                if (operators->isEqual(baseCond, operators->number_(3, 1)))
                    result = operators->isEqual(operators->extract(nzcvVal, 1, 2), operators->number_(1, 1));
                if (operators->isEqual(baseCond, operators->number_(3, 2)))
                    result = operators->isEqual(operators->extract(nzcvVal, 3, 4), operators->number_(1, 1));
                if (operators->isEqual(baseCond, operators->number_(3, 3)))
                    result = operators->isEqual(operators->extract(nzcvVal, 0, 1), operators->number_(1, 1));
                if (operators->isEqual(baseCond, operators->number_(3, 4)))
                    result = operators->ite(
                            operators->isEqual(operators->extract(nzcvVal, 1, 2), operators->number_(1, 1)),
                            operators->ite(
                                    operators->isEqual(operators->extract(nzcvVal, 2, 3), operators->number_(1, 0)),
                                    operators->boolean_(true), operators->boolean_(false)),
                            operators->boolean_(false));
                if (operators->isEqual(baseCond, operators->number_(3, 5)))
                    result = operators->isEqual(operators->extract(nzcvVal, 3, 4),
                                                operators->extract(nzcvVal, 0, 1));
                if (operators->isEqual(baseCond, operators->number_(3, 6)))
                    result = operators->ite(operators->isEqual(operators->extract(nzcvVal, 3, 4),
                                                               operators->extract(nzcvVal, 0, 1)),
                                            operators->ite(operators->isEqual(operators->extract(nzcvVal, 2, 3),
                                                                              operators->number_(1, 0)),
                                                           operators->boolean_(true), operators->boolean_(false)),
                                            operators->boolean_(false));
                if (operators->isEqual(baseCond, operators->number_(3, 7)))
                    result = operators->boolean_(true);

                result = operators->ite(
                        operators->isEqual(operators->extract(baseCond, 0, 1), operators->number_(1, 1)),
                        operators->ite(operators->isNotEqual(baseCond, operators->number_(4, 0xF)),
                                       operators->invert(result), result), result);

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

            size_t
            DispatcherARM64::getRegSize(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(23, 23, raw) == 0) {
                    if(IntegerOps::extract2<uint32_t>(30, 31, raw) == 0x3)
                        return 64;
                    else
                        return 32;
                } else {
                    if(IntegerOps::extract2<uint32_t>(122, 22, raw) == 1)
                        return 32;
                    else
                        return 64;
                }
            }

            size_t
            DispatcherARM64::ldStrLiteralAccessSize(uint32_t raw) {
                return 0;
            }

            BaseSemantics::SValuePtr
            DispatcherARM64::doAddOperation(BaseSemantics::SValuePtr a, BaseSemantics::SValuePtr b,
                                            bool invertCarries, const BaseSemantics::SValuePtr &carryIn,
                                            BaseSemantics::SValuePtr &nzcv) {
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
                setFlagsForResult(result, carries, invertCarries, a->get_width(), nzcv);
                return result;
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
                SymEvalSemantics::StateARM64Ptr state = SymEvalSemantics::StateARM64::promote(operators->currentState());

                //The second, third and fourth arguments will remain and unused and hence these values are used
                return state->readMemory(addr, operators->unspecified_(1), NULL, NULL, readSize);
            }

            void
            DispatcherARM64::writeMemory(const BaseSemantics::SValuePtr &addr, size_t writeSize, const BaseSemantics::SValuePtr &data) {
                SymEvalSemantics::StateARM64Ptr state = SymEvalSemantics::StateARM64::promote(operators->currentState());

                //The third and fourth arguments will remain and unused and hence these values are used
                state->writeMemory(addr, data, NULL, NULL, writeSize);
            }
        } // namespace
    } // namespace
} // namespace

