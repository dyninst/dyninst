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
                    SgAsmAMDGPUInstruction *insn = isSgAsmAMDGPUInstruction(insn_);
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
                struct IP_s_add_u32 : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr src1;
                        BaseSemantics::SValuePtr src0;
                        if(SgAsmIntegerValueExpression * ival = isSgAsmIntegerValueExpression(args[1])){
                            src1 = ops->number_(64, ival->get_value());
                        }else{
                            src1 = ops->unsignedExtend(d->read(args[1]),64);
                        } 
                        if(SgAsmIntegerValueExpression * ival = isSgAsmIntegerValueExpression(args[2])){
                            src0 = ops->number_(64, ival->get_value());
                        }else{
                            src0 = ops->unsignedExtend(d->read(args[2]),64);
                        } 


                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr n, z, c, v;

                        result = d->doAddOperation( src0 , src1 ,false,ops->boolean_(false),n,z,c,v);

                        BaseSemantics::SValuePtr scc_value;

                        scc_value = ops->ite(
                                ops->isUnsignedGreaterThanOrEqual(result,ops->number_(64,0x100000000)),
                                ops->number_(1,1),
                                ops->number_(1,0));

                        d->writeRegister(d->REG_SCC,scc_value);
                        
                        d->write(args[0],ops->extract(result,0,32));
                        //d->writeRegister(d->REG_SCC,c);
                    }
                };
                struct IP_s_addc_u32 : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        BaseSemantics::SValuePtr src1;
                        BaseSemantics::SValuePtr src0;

                        if(SgAsmIntegerValueExpression * ival = isSgAsmIntegerValueExpression(args[1])){
                            src1 = ops->number_(64, ival->get_value());

                        }else{
                            src1 = ops->unsignedExtend(d->read(args[1]),64);
                        } 
                        if(SgAsmIntegerValueExpression * ival = isSgAsmIntegerValueExpression(args[2])){
                            src0 = ops->number_(64, ival->get_value());
                        }else{
                            src0 = ops->unsignedExtend(d->read(args[2]),64);
                        } 


                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr n, z, c, v;
                        BaseSemantics::SValuePtr old_scc = d->readRegister(d->REG_SCC);
                        result = d->doAddOperation( src0 , src1 ,false,
                                d->readRegister(d->REG_SCC)
                                ,n,z,c,v);



                        d->write(args[0],ops->extract(result,0,32));
                        d->writeRegister(d->REG_SCC,c);

                    }
                };
 
                struct IP_s_getpc_b64 : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {

                        SgAsmIntegerValueExpression *ival = isSgAsmIntegerValueExpression(args[2]);
                        BaseSemantics::SValuePtr result = ops->number_(ival->get_significantBits(), ival->get_value());
                        BaseSemantics::SValuePtr lowPC = ops->extract(result,0,32);
                        BaseSemantics::SValuePtr highPC = ops->shiftRight(result,ops->number_(32,32)); 
 
                        d->write(args[0],lowPC);
                        d->write(args[1],highPC);
                    }
                };
 
                struct IP_s_setpc_b64 : P {
                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result;
                        BaseSemantics::SValuePtr lowPC = ops->extract(d->read(args[0]),0,32);
                        BaseSemantics::SValuePtr highPC = d->ZeroExtend(d->read(args[1]),64);
                        BaseSemantics::SValuePtr n, z, c, v;
                        result = d->doAddOperation( ops->shiftLeft(highPC, ops->number_(32,32)) , lowPC,false,ops->boolean_(false),n,z,c,v);
                        d->writeRegister(d->REG_PC, result);
                    }
                };
                struct IP_s_swappc_b64 : P {

                    void p(D d, Ops ops, I insn, A args, B raw) {
                        BaseSemantics::SValuePtr result = d->readRegister(d->REG_PC);
                        BaseSemantics::SValuePtr storelowPC = result;
                        BaseSemantics::SValuePtr storehighPC = ops->shiftRight(result,ops->number_(32,32)); 
 
                        d->write(args[0],storelowPC);
                        d->write(args[1],storehighPC);
 
                        BaseSemantics::SValuePtr n, z, c, v;
                        BaseSemantics::SValuePtr lowPC = ops->extract(d->read(args[2]),0,32);
                        BaseSemantics::SValuePtr highPC = d->ZeroExtend(d->read(args[3]),64);
                        result = d->doAddOperation( ops->shiftLeft(highPC, ops->number_(32,32)) , lowPC,false,ops->boolean_(false),n,z,c,v);
                        d->writeRegister(d->REG_PC, result);
                    }

                };


            } // namespace

/*******************************************************************************************************************************
 *                                      DispatcherAMDGPU
 *******************************************************************************************************************************/

            void
            DispatcherAMDGPU::iproc_init() {

                iproc_set(rose_amdgpu_op_s_getpc_b64, new AMDGPU::IP_s_getpc_b64);
                iproc_set(rose_amdgpu_op_s_setpc_b64, new AMDGPU::IP_s_setpc_b64);
                iproc_set(rose_amdgpu_op_s_swappc_b64, new AMDGPU::IP_s_swappc_b64);
                iproc_set(rose_amdgpu_op_s_add_u32, new AMDGPU::IP_s_add_u32);
                iproc_set(rose_amdgpu_op_s_addc_u32, new AMDGPU::IP_s_addc_u32);
            }

            void
            DispatcherAMDGPU::regcache_init() {
                if (regdict) {
                    REG_PC = findRegister("pc_all", 64);
                    REG_SCC = findRegister("src_scc", 1);
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
                assert(0 && "not implemented");
                return REG_SCC;
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
            DispatcherAMDGPU::invertMaybe(const BaseSemantics::SValuePtr &value, bool maybe) {
                return maybe ? operators->invert(value) : value;
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
            DispatcherAMDGPU::readRegister(const RegisterDescriptor &reg) {
                return operators->readRegister(reg);
            }

            void
            DispatcherAMDGPU::writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value) {
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

        } // namespace
    } // namespace
} // namespace

//using namespace rose::Diagnostics;

