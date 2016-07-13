//
// Created by ssunny on 7/1/16.
//

#include <libtasn1.h>
#include "SymEvalSemantics.h"

using namespace rose::BinaryAnalysis::InstructionSemantics2;

///////////////////////////////////////////////////////
//                                           StateARM64
///////////////////////////////////////////////////////

BaseSemantics::SValuePtr SymEvalSemantics::StateARM64::readRegister(const RegisterDescriptor &desc,
                                                                    const BaseSemantics::SValuePtr &/*dflt*/, BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_require(desc.is_valid());
    SymEvalSemantics::RegisterStateARM64Ptr registers = SymEvalSemantics::RegisterStateARM64::promote(registerState());
    return registers->readRegister(desc, addr);
}

void SymEvalSemantics::StateARM64::writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value,
                                                 BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_require(reg.is_valid());
    ASSERT_not_null(value);
    SymEvalSemantics::RegisterStateARM64Ptr registers = SymEvalSemantics::RegisterStateARM64::promote(registerState());
    registers->writeRegister(reg, value, res, aaMap);
}

void SymEvalSemantics::StateARM64::writeMemory(const BaseSemantics::SValuePtr &addr,
                                               const BaseSemantics::SValuePtr &value,
                                               BaseSemantics::RiscOperators */*addrOps*/,
                                               BaseSemantics::RiscOperators */*valOps*/) {
    ASSERT_not_null(addr);
    ASSERT_not_null(value);
    SymEvalSemantics::MemoryStateARM64Ptr memory = SymEvalSemantics::MemoryStateARM64::promote(memoryState());
    memory->writeMemory(addr, value, res, aaMap);
}

///////////////////////////////////////////////////////
//                                   RegisterStateARM64
///////////////////////////////////////////////////////

BaseSemantics::SValuePtr SymEvalSemantics::RegisterStateARM64::readRegister(const RegisterDescriptor &reg,
                                                                            Dyninst::Address addr) {
    if(reg.get_major() == armv8_regclass_gpr) {
        ARMv8GeneralPurposeRegister r = static_cast<ARMv8GeneralPurposeRegister>(reg.get_minor());
        unsigned int size = reg.get_nbits();
        return SymEvalSemantics::SValue::instance(wrap(convert(r, size), addr));
    } else {
        ASSERT_not_implemented("readRegister not yet implemented for categories other than GPR");
    }
}

BaseSemantics::SValuePtr SymEvalSemantics::RegisterStateARM64::readRegister(const RegisterDescriptor &reg,
                                                                            const BaseSemantics::SValuePtr &/*dflt*/,
                                                                            BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_always_forbid("overridden RegisterState::readRegister() should never be called for ARM64, always use the non-virtual readRegister that takes Dyninst::Address as an argument.");
}

void SymEvalSemantics::RegisterStateARM64::writeRegister(const RegisterDescriptor &reg,
                                                         const BaseSemantics::SValuePtr &value,
                                                         Dyninst::DataflowAPI::Result_t &res,
                                                         std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap) {
    if(reg.get_major() == armv8_regclass_gpr) {
        ARMv8GeneralPurposeRegister r = static_cast<ARMv8GeneralPurposeRegister>(reg.get_minor());

        std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr>::iterator i = aaMap.find(convert(r, reg.get_nbits()));
        if (i != aaMap.end()) {
            SymEvalSemantics::SValuePtr value_ = SymEvalSemantics::SValue::promote(value);
            res[i->second] = value_->get_expression();
        }
    } else {
        ASSERT_not_implemented("writeRegister not yet implemented for categories other than GPR");
    }
}

void SymEvalSemantics::RegisterStateARM64::writeRegister(const RegisterDescriptor &/*reg*/,
                                                         const BaseSemantics::SValuePtr &/*value*/,
                                                         BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_always_forbid("overridden RegisterState::writeRegister() should never be called for ARM64, always use the non-virtual writeRegister that also takes additional parameters.");
}

Dyninst::Absloc SymEvalSemantics::RegisterStateARM64::convert(ARMv8GeneralPurposeRegister r, unsigned int size) {
    Dyninst::MachRegister mreg;

    if(r != armv8_gpr_zr) {
        mreg = (size == 32)?Dyninst::aarch64::wzr:Dyninst::aarch64::zr;
    } else {
        mreg = (size == 32)?Dyninst::aarch64::w0:Dyninst::aarch64::x0;
        mreg = Dyninst::MachRegister(mreg.val() + (r - armv8_gpr_r0));
    }

    return Dyninst::Absloc(mreg);
}

///////////////////////////////////////////////////////
//                                     MemoryStateARM64
///////////////////////////////////////////////////////

//TODO: what is Len in this case?

BaseSemantics::SValuePtr SymEvalSemantics::MemoryStateARM64::readMemory(const BaseSemantics::SValuePtr &address,
                                                                        const BaseSemantics::SValuePtr &/*dflt*/,
                                                                        BaseSemantics::RiscOperators */*addrOps*/,
                                                                        BaseSemantics::RiscOperators */*valOps*/) {
    SymEvalSemantics::SValuePtr addr = SymEvalSemantics::SValue::promote(address);
    return SymEvalSemantics::SValue::instance(Dyninst::DataflowAPI::RoseAST::create(Dyninst::DataflowAPI::ROSEOperation(Dyninst::DataflowAPI::ROSEOperation::derefOp),
                                                 addr->get_expression(),
                                                 Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(1, 1))));
}

void SymEvalSemantics::MemoryStateARM64::writeMemory(const BaseSemantics::SValuePtr &address,
                                                     const BaseSemantics::SValuePtr &value,
                                                     Dyninst::DataflowAPI::Result_t &res,
                                                     std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap) {
    std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr>::iterator i = aaMap.find(Dyninst::Absloc(0));
    SymEvalSemantics::SValuePtr addr = SymEvalSemantics::SValue::promote(address);

    if (i != aaMap.end()) {
        i->second->out().setGenerator(addr->get_expression());
        //i->second->out().setSize(Len);

        SymEvalSemantics::SValuePtr data = SymEvalSemantics::SValue::promote(value);
        res[i->second] = data->get_expression();
    }
}

void SymEvalSemantics::MemoryStateARM64::writeMemory(const BaseSemantics::SValuePtr &/*addr*/,
                                                     const BaseSemantics::SValuePtr &/*value*/,
                                                     BaseSemantics::RiscOperators */*addrOps*/,
                                                     BaseSemantics::RiscOperators */*valOps*/) {
    ASSERT_always_forbid("overridden MemoryState::writeMemory() should never be called for ARM64, always use the non-virtual writeRegister that also takes additional parameters.");
}

///////////////////////////////////////////////////////
//                                        RiscOperators
///////////////////////////////////////////////////////

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::and_(const BaseSemantics::SValuePtr &a_,
                                                               const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::andOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::or_(const BaseSemantics::SValuePtr &a_,
                                                              const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::orOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::xor_(const BaseSemantics::SValuePtr &a_,
                                                               const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::xorOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::invert(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::invertOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::extract(const BaseSemantics::SValuePtr &a_, size_t begin,
                                                                  size_t end) {
    BaseSemantics::SValuePtr begin_ = SymEvalSemantics::SValue::instance(64, begin);
    BaseSemantics::SValuePtr end_ = SymEvalSemantics::SValue::instance(64, end);

    return createTernaryAST(Dyninst::DataflowAPI::ROSEOperation::extractOp, a_, begin_, end_, end - begin);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::ite(const BaseSemantics::SValuePtr &sel_,
                                                              const BaseSemantics::SValuePtr &a_,
                                                              const BaseSemantics::SValuePtr &b_) {
    return createTernaryAST(Dyninst::DataflowAPI::ROSEOperation::ifOp, sel_, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::concat(const BaseSemantics::SValuePtr &a_,
                                                                 const BaseSemantics::SValuePtr &b_) {
    //TODO: should be able to specify number of bits to concat for each expression
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::concatOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::leastSignificantSetBit(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::LSBSetOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::mostSignificantSetBit(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::MSBSetOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::rotateLeft(const BaseSemantics::SValuePtr &a_,
                                                                     const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::rotateLOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::rotateRight(const BaseSemantics::SValuePtr &a_,
                                                                      const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::rotateROp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::shiftLeft(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::shiftLOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::shiftRight(const BaseSemantics::SValuePtr &a_,
                                                                     const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::shiftROp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::shiftRightArithmetic(const BaseSemantics::SValuePtr &a_,
                                                                               const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::shiftRArithOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::equalToZero(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::equalToZeroOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::signExtend(const BaseSemantics::SValuePtr &a_,
                                                                     size_t newwidth) {
    BaseSemantics::SValuePtr width_ = SymEvalSemantics::SValue::instance(64, newwidth);

    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::signExtendOp, a_, width_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::add(const BaseSemantics::SValuePtr &a_,
                                                              const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::addOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::addWithCarries(const BaseSemantics::SValuePtr &a_,
                                                                         const BaseSemantics::SValuePtr &b_,
                                                                         const BaseSemantics::SValuePtr &c_,
                                                                         BaseSemantics::SValuePtr &carry_out) {
    BaseSemantics::SValuePtr aa_ = unsignedExtend(a_, a_->get_width() + 1);
    BaseSemantics::SValuePtr bb_ = unsignedExtend(a_, b_->get_width() + 1);
    BaseSemantics::SValuePtr sum_ = createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::addOp, aa_,
                                                    createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::addOp, bb_, c_));

    BaseSemantics::SValuePtr cc_ = createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::xorOp, aa_,
                                                   createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::xorOp, bb_, sum_));
    carry_out = extract(cc_, 1, a_->get_width() + 1);

    return extract(sum_, 0, a_->get_width());
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::negate(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::negateOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::signedDivide(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::sDivOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::signedModulo(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::sModOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::signedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                         const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::sMultOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::unsignedDivide(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::uDivOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::unsignedModulo(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::uModOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::unsignedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                         const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::uMultOp, a_, b_);
}

//TODO: do we deal with byte ordering?
BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsARM64::readMemory(const RegisterDescriptor &/*segreg*/,
                                                                     const BaseSemantics::SValuePtr &addr,
                                                                     const BaseSemantics::SValuePtr &dflt,
                                                                     const BaseSemantics::SValuePtr &/*cond*/) {
    return currentState()->readMemory(addr, dflt, this, this);
}

void SymEvalSemantics::RiscOperatorsARM64::writeMemory(const RegisterDescriptor &/*segreg*/,
                                                  const BaseSemantics::SValuePtr &addr,
                                                  const BaseSemantics::SValuePtr &data,
                                                  const BaseSemantics::SValuePtr &/*cond*/) {
    currentState()->writeMemory(addr, data, this, this);
}