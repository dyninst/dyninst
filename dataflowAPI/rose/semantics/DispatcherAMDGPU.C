#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "BaseSemantics2.h"
//#include "Diagnostics.h"
#include "DispatcherAMDGPU.h"
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
 *                                      Base AMDGPU instruction processor
 *******************************************************************************************************************************/
            namespace AMDGPU {

                void
                InsnProcessor::process(const BaseSemantics::DispatcherPtr &dispatcher_, SgAsmInstruction *insn_) {
                    DispatcherAMDGPUPtr dispatcher = DispatcherAMDGPU::promote(dispatcher_);
                    BaseSemantics::RiscOperatorsPtr operators = dispatcher->get_operators();
                    SgAsmAmdgpuInstruction *insn = isSgAsmAmdgpuInstruction(insn_);
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
 *                                      Functors that handle individual AMDGPU instructions kinds
 *******************************************************************************************************************************/
                typedef InsnProcessor P;
                struct IP_add_u32_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        assert (0 && "called add_u32_execute ");
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

                struct IP_addc_u32_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        assert (0 && "called addc_u32_execute ");
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


                struct IP_getpc_b64_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        assert (0 && "called getpc_execute ");
                    }
                };
                struct IP_setpc_b64_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr target_sgpr = d->read(args[0]);
                        
                        bool carry_in;
                        BaseSemantics::SValuePtr pcVal = d->readRegister(d->REG_PC);
                        BaseSemantics::SValuePtr imm4  = ops->number_(48,4);

                        BaseSemantics::SValuePtr n, z, c, v;
                        result = d->doAddOperation(pcVal, imm4, carry_in, ops->boolean_(false), n, z, c, v);

                        d->writeRegister(d->REG_SCC,c);

                        //assert (0 && "called setpc_execute ");
                    }
                };
                struct IP_swappc_b64_execute : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr target_sgpr = d->read(args[0]);
                        BaseSemantics::SValuePtr storage_sgpr = d->read(args[1]);
                        
                        bool carry_in;
                        BaseSemantics::SValuePtr pcVal = d->readRegister(d->REG_PC);
                        BaseSemantics::SValuePtr imm4  = ops->number_(48,4);

                        BaseSemantics::SValuePtr n, z, c, v;
                        result = d->doAddOperation(pcVal, imm4, carry_in, ops->boolean_(false), n, z, c, v);

                        d->writeRegister(d->REG_SCC,c);

                        //assert (0 && "called swappc_execute ");
                    }
                };




            } // namespace

/*******************************************************************************************************************************
 *                                      DispatcherAMDGPU
 *******************************************************************************************************************************/

            void
            DispatcherAMDGPU::iproc_init() {
                iproc_set(rose_amdgpu_op_s_add_u32, new AMDGPU::IP_add_u32_execute);
                iproc_set(rose_amdgpu_op_s_addc_u32, new AMDGPU::IP_addc_u32_execute);

                iproc_set(rose_amdgpu_op_s_getpc_b64, new AMDGPU::IP_getpc_b64_execute);
                iproc_set(rose_amdgpu_op_s_setpc_b64, new AMDGPU::IP_setpc_b64_execute);
                iproc_set(rose_amdgpu_op_s_swappc_b64, new AMDGPU::IP_swappc_b64_execute);
            }

            void
            DispatcherAMDGPU::regcache_init() {
                if (regdict) {
                    REG_PC = findRegister("amdgpu_pc", 48);
                    REG_SCC = findRegister("amdgpu_scc", 1);
                }
            }

            void
            DispatcherAMDGPU::memory_init() {
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
            DispatcherAMDGPU::instructionPointerRegister() const {
                return REG_PC;
            }

            RegisterDescriptor
            DispatcherAMDGPU::stackPointerRegister() const {
                return REG_PC;
            }

            static bool
            isStatusRegister(const RegisterDescriptor &reg) {
                // TODO: deal with status register
                return false;
                //return reg.get_major() == amdgpu_regclass_pstate && reg.get_minor() == 0;
            }

            RegisterDictionary::RegisterDescriptors
            DispatcherAMDGPU::get_usual_registers() const {
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
            DispatcherAMDGPU::set_register_dictionary(const RegisterDictionary *regdict) {
                BaseSemantics::Dispatcher::set_register_dictionary(regdict);
                regcache_init();
            }

            void
            DispatcherAMDGPU::setFlagsForResult(const BaseSemantics::SValuePtr &result,
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
            DispatcherAMDGPU::parity(const BaseSemantics::SValuePtr &v) {
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
            DispatcherAMDGPU::invertMaybe(const BaseSemantics::SValuePtr &value, bool maybe) {
                return maybe ? operators->invert(value) : value;
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::isZero(const BaseSemantics::SValuePtr &value) {
                return operators->equalToZero(value);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::ConditionHolds(const BaseSemantics::SValuePtr &cond) {
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
            DispatcherAMDGPU::NOT(const BaseSemantics::SValuePtr &expr) {
                return operators->invert(expr);
            }

            void
            DispatcherAMDGPU::BranchTo(const BaseSemantics::SValuePtr &target) {
                ASSERT_require(target != NULL);
                writeRegister(REG_PC, operators->extract(target, 0, 64));
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::Zeros(const unsigned int nbits) {
                ASSERT_require(nbits > 0);
                return operators->number_(nbits, 0);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::SignExtend(const BaseSemantics::SValuePtr &expr, size_t newsize) {
                ASSERT_require(newsize > 0);
                return operators->signExtend(expr, newsize);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::ZeroExtend(const BaseSemantics::SValuePtr &expr, size_t newsize) {
                ASSERT_require(newsize > 0);
                return operators->unsignedExtend(expr, newsize);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::ROR(const BaseSemantics::SValuePtr &expr, const BaseSemantics::SValuePtr &amt) {
                ASSERT_not_null(amt);
                return operators->rotateRight(expr, amt);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::Replicate(const BaseSemantics::SValuePtr &expr) {
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
            DispatcherAMDGPU::getBitfieldMask(int immr, int imms,
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
            DispatcherAMDGPU::getRegSize(uint32_t raw) {
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
            DispatcherAMDGPU::ldStrLiteralAccessSize(uint32_t raw) {
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
            DispatcherAMDGPU::inzero(uint32_t raw) {
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
            DispatcherAMDGPU::extend(uint32_t raw) {
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
            DispatcherAMDGPU::op(uint32_t raw) {
                switch (IntegerOps::extract2<uint32_t>(29, 30, raw)) {
                    case 0:
                    case 3:
                        return static_cast<int>(AMDGPU::InsnProcessor::LogicalOp_AND);
                    case 1:
                        return static_cast<int>(AMDGPU::InsnProcessor::LogicalOp_ORR);
                    case 2:
                        return static_cast<int>(AMDGPU::InsnProcessor::LogicalOp_EOR);
                    default:
                        assert(!"Invalid code for opc field in logical OR instruction variant!");
                        break;
                }
            }

            bool
            DispatcherAMDGPU::setflags(uint32_t raw) {
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
            DispatcherAMDGPU::getDatasize(uint32_t raw) {
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
            DispatcherAMDGPU::getShiftType(uint32_t raw) {
                int v10_11 = IntegerOps::extract2(10, 11, raw);

                switch(v10_11) {
                    case 0: return static_cast<int>(AMDGPU::InsnProcessor::ShiftType_LSL);
                    case 1: return static_cast<int>(AMDGPU::InsnProcessor::ShiftType_LSR);
                    case 2: return static_cast<int>(AMDGPU::InsnProcessor::ShiftType_ASR);
                    case 3: return static_cast<int>(AMDGPU::InsnProcessor::ShiftType_ROR);
                    default: ASSERT_not_reachable("Cannot have a value greater than 3 for a 2-bit field (field: op2, bits: 10..11)!");
                }
            }

            int
            DispatcherAMDGPU::getConditionVal(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(24, 31, raw) == 0x54 && IntegerOps::extract2<uint32_t>(4, 4, raw) == 0)
                    return IntegerOps::extract2(0, 3, raw);
                else
                    return IntegerOps::extract2(12, 15, raw);
            }

            int
            DispatcherAMDGPU::opcode(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(23, 28, raw) == 0x25)
                    return IntegerOps::extract2<uint32_t>(29, 30, raw);
                else
                    return IntegerOps::extract2<uint32_t>(10, 10, raw);
            }

            bool
            DispatcherAMDGPU::subop(uint32_t raw) {
                if(IntegerOps::extract2<uint32_t>(24, 28, raw) == 0x1B) {
                    return IntegerOps::extract2<uint32_t>(15, 15, raw) == 0x1;
                } else {
                    return IntegerOps::extract2<uint32_t>(30, 30, raw) == 0x1;
                }
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::doAddOperation(BaseSemantics::SValuePtr a, BaseSemantics::SValuePtr b,
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
            DispatcherAMDGPU::effectiveAddress(SgAsmExpression *e, size_t nbits) {
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
            DispatcherAMDGPU::readRegister(const RegisterDescriptor &reg) {
                /*if (reg.get_major() == amdgpu_regclass_gpr &&
                    reg.get_minor() == amdgpu_gpr_zr) {
                    return operators->number_(reg.get_nbits(), 0);
                }*/

                return operators->readRegister(reg);
            }

            void
            DispatcherAMDGPU::writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value) {
                // TODO: deal with writes to upper /lower half of register
                /*if (reg.get_major() == amdgpu_regclass_gpr || reg.get_major() == amdgpu_regclass_pc ||
                    reg.get_major() == amdgpu_regclass_sp) {
                    if (reg.get_minor() == amdgpu_gpr_zr) {
                        return;
                    }

                    if (reg.get_nbits() == 32 && reg.get_offset() == 0) {
                        RegisterDescriptor upperHalf(reg.get_major(), reg.get_minor(), 32, 32);
                        operators->writeRegister(upperHalf, operators->number_(32, 0));
                    }
                }*/

                operators->writeRegister(reg, value);
            }

            void
            DispatcherAMDGPU::write(SgAsmExpression *e, const BaseSemantics::SValuePtr &value,
                                   size_t addr_nbits/*=0*/) {
                if (SgAsmDirectRegisterExpression *re = isSgAsmDirectRegisterExpression(e)) {
                    writeRegister(re->get_descriptor(), value);
                } else {
                    Dispatcher::write(e, value, addr_nbits);        // defer to super class
                }
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::fixMemoryAddress(const BaseSemantics::SValuePtr &addr) const {
                if (size_t addrWidth = addressWidth()) {
                    if (addr->get_width() < addrWidth)
                        return operators->signExtend(addr, addrWidth);
                    if (addr->get_width() > addrWidth)
                        return operators->unsignedExtend(addr, addrWidth);
                }
                return addr;
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::readMemory(const BaseSemantics::SValuePtr &addr, size_t readSize) {
                SymEvalSemantics::StateASTPtr state = SymEvalSemantics::StateAST::promote(operators->currentState());

                //The second, third and fourth arguments will remain unused
                return state->readMemory(addr, operators->unspecified_(1), NULL, NULL, readSize);
            }

            void
            DispatcherAMDGPU::writeMemory(const BaseSemantics::SValuePtr &addr, size_t writeSize, const BaseSemantics::SValuePtr &data) {
                SymEvalSemantics::StateASTPtr state = SymEvalSemantics::StateAST::promote(operators->currentState());

                //The third and fourth arguments will remain unused
                state->writeMemory(addr, data, NULL, NULL, writeSize);
            }

            SgAsmExpression *
            DispatcherAMDGPU::getWriteBackTarget(SgAsmExpression *expr) {
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
            DispatcherAMDGPU::UInt(const BaseSemantics::SValuePtr &expr) {
                ASSERT_not_null(expr);
                return expr;
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::ShiftReg(const BaseSemantics::SValuePtr &src, int shiftType, const BaseSemantics::SValuePtr &amount) {
                ASSERT_not_null(amount);

                switch(static_cast<AMDGPU::InsnProcessor::ShiftType>(shiftType)) {
                    case AMDGPU::InsnProcessor::ShiftType_LSL: return operators->shiftLeft(src, amount);
                    case AMDGPU::InsnProcessor::ShiftType_LSR: return operators->shiftRight(src, amount);
                    case AMDGPU::InsnProcessor::ShiftType_ASR: return operators->shiftRightArithmetic(src, amount);
                    case AMDGPU::InsnProcessor::ShiftType_ROR: return operators->rotateRight(src, amount);
                    default: ASSERT_not_reachable("Found invalid shift type for shift instruction!");
                }
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::CountLeadingZeroBits(const BaseSemantics::SValuePtr &expr) {
                size_t len = expr->get_width();

                for(int idx = len - 1; idx >= 0; idx--)
                    if(operators->isEqual(operators->extract(expr, len, len + 1), operators->number_(1, 1)))
                        return operators->number_(expr->get_width(), len - 1 - idx);

                return operators->number_(expr->get_width(), len);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::CountLeadingSignBits(const BaseSemantics::SValuePtr &expr) {
                size_t len = expr->get_width();
                BaseSemantics::SValuePtr arg = operators->xor_(operators->extract(expr, 1, len), operators->extract(expr, 0, len - 1));
                return CountLeadingZeroBits(arg);
            }

            BaseSemantics::SValuePtr
            DispatcherAMDGPU::Int(const BaseSemantics::SValuePtr &expr, bool isUnsigned) {
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
            DispatcherAMDGPU::RoundTowardsZero(const BaseSemantics::SValuePtr &expr) {
                return expr;
            }
        } // namespace
    } // namespace
} // namespace

//using namespace rose::Diagnostics;

