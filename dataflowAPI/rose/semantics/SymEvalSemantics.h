//
// Created by ssunny on 7/1/16.
//

#ifndef DYNINST_SYMEVALSEMANTICS_H
#define DYNINST_SYMEVALSEMANTICS_H

#include "external/rose/armv8InstructionEnum.h"
#include "BaseSemantics2.h"
#include "../../h/SymEval.h"

namespace rose {
    namespace BinaryAnalysis {
        namespace InstructionSemantics2 {
            namespace SymEvalSemantics {

                /***************************************************************************************************/
                /*                                              SValue                                             */
                /***************************************************************************************************/
                typedef Sawyer::SharedPointer<class SValue> SValuePtr;

                class SValue : public BaseSemantics::SValue {
                protected:
                    Dyninst::AST::Ptr expr;

                    SValue(Dyninst::Absloc r, Dyninst::Address addr): BaseSemantics::SValue(64) {
                        expr = Dyninst::DataflowAPI::VariableAST::create(Variable(Dyninst::AbsRegion(r), addr));
                    }

                    SValue(size_t nbits, uint64_t num): BaseSemantics::SValue(nbits) {
                        expr = Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(num, nbits));
                    }

                    SValue(Dyninst::AST::Ptr expr): BaseSemantics::SValue(64) {
                        this->expr = expr;
                    }

                public:
                    static SValuePtr instance(Dyninst::Absloc r, Dyninst::Address addr) {
                        return SValuePtr(new SValue(r, addr));
                    }

                    static SValuePtr instance(size_t nbits, uint64_t num) {
                        return SValuePtr(new SValue(nbits, num));
                    }

                    static SValuePtr instance(Dyninst::AST::Ptr expr) {
                        return SValuePtr(new SValue(expr));
                    }

                public:
                    virtual BaseSemantics::SValuePtr undefined_(size_t nbits) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::BottomAST::create(false)));
                    }

                    virtual BaseSemantics::SValuePtr unspecified_(size_t nbits) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::BottomAST::create(false)));
                    }

                    //TODO
                    virtual BaseSemantics::SValuePtr bottom_(size_t nbits) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::BottomAST::create(true)));
                    }

                    virtual BaseSemantics::SValuePtr number_(size_t nbits, uint64_t num) const {
                        return SValuePtr(new SValue(nbits, num));
                    }

                    virtual BaseSemantics::SValuePtr boolean_(bool value) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(value?1:0, 1))));
                    }

                    virtual BaseSemantics::SValuePtr copy(size_t new_width = 0) const {
                        SValuePtr retval(new SValue(this->get_expression()));
                        if (new_width!=0 && new_width!=retval->get_width())
                            retval->set_width(new_width);
                        return retval;
                    }

                    virtual Sawyer::Optional<BaseSemantics::SValuePtr>
                            createOptionalMerge(const BaseSemantics::SValuePtr &other, const BaseSemantics::MergerPtr&, SMTSolver*) const;

                public:
                    static SValuePtr promote(const BaseSemantics::SValuePtr &v) {
                        SValuePtr retval = v.dynamicCast<SValue>();
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual Dyninst::AST::Ptr get_expression() {
                        return expr;
                    }

                    virtual bool isBottom() const {
                        return expr->getID() == Dyninst::DataflowAPI::V_BottomAST;
                    }

                    virtual bool is_number() const {
                        return expr->getID() == Dyninst::DataflowAPI::V_ConstantAST;
                    }

                    virtual uint64_t get_number() const {
                        assert(expr->getID() == Dyninst::DataflowAPI::V_ConstantAST);
                        Dyninst::DataflowAPI::Constant constant = expr->val();
                        ASSERT_not_null(constant);
                        return constant.val;
                    }

                    virtual void print(std::ostream &, BaseSemantics::Formatter &) const { }

                };


                /***************************************************************************************************/
                /*                                          Register State                                         */
                /***************************************************************************************************/

                typedef boost::shared_ptr<class RegisterStateARM64> RegisterStateARM64Ptr;

                class RegisterStateARM64 : public BaseSemantics::RegisterState {
                public:
                    RegisterStateARM64(const BaseSemantics::SValuePtr &protoval,
                                       const RegisterDictionary *regdict) : RegisterState(protoval, regdict) { }

                public:
                    static RegisterStateARM64Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                                          const RegisterDictionary *regdict) {
                        return RegisterStateARM64Ptr(new RegisterStateARM64(protoval, regdict));
                    }

                    virtual BaseSemantics::RegisterStatePtr create(const BaseSemantics::SValuePtr &protoval,
                                                                   const RegisterDictionary *regdict) const {
                        return instance(protoval, regdict);
                    }

                    virtual BaseSemantics::RegisterStatePtr clone() const {
                        ASSERT_not_implemented("RegisterState::clone() should not be called with Dyninst's SymEval policy");
                    }

                    static RegisterStateARM64Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                        RegisterStateARM64Ptr retval = boost::dynamic_pointer_cast<RegisterStateARM64>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual void clear() {
                        ASSERT_not_implemented("RegisterState::clear() should not be called with Dyninst's SymEval policy");
                    }

                    virtual void zero() {
                        ASSERT_not_implemented("RegisterState::zero() should not be called with Dyninst's SymEval policy");
                    }

                    virtual BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &dflt, BaseSemantics::RiscOperators *ops);
                    virtual void writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *ops);

                    virtual void print(std::ostream &, BaseSemantics::Formatter &) const {}

                    virtual bool merge(const RegisterDescriptor &other, BaseSemantics::RiscOperators *ops) {
                        return true;
                    }

                    void writeRegister(const RegisterDescriptor &reg, const BaseSematics::SValuePtr &value,
                                       Dyninst::DataflowAPI::Result_t &res, std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap);

                private:
                    Dyninst::AST::Ptr wrap(Dyninst::Absloc r) {
                        return Dyninst::DataflowAPI::VariableAST::create(Dyninst::DataflowAPI::Variable(Dyninst::AbsRegion(r), addr));
                    }

                    Dyninst::Absloc convert(ARMv8GeneralPurposeRegister r, unsigned int size);
                };


                /***************************************************************************************************/
                /*                                           Memory State                                          */
                /***************************************************************************************************/

                typedef boost::shared_ptr<class MemoryStateARM64> MemoryStateARM64Ptr;

                class MemoryStateARM64 : public BaseSemantics::MemoryState {
                protected:
                    MemoryStateARM64(const BaseSemantics::SValuePtr &addrProtoval, const BaseSemantics::SValuePtr &valProtoval):
                            BaseSemantics::MemoryState(addrProtoval, valProtoval) { }

                public:
                    static MemoryStateARM64Ptr instance(const BaseSemantics::SValuePtr &addrProtoval, const BaseSemantics::SValuePtr &valProtoval) {
                        return MemoryStateARM64Ptr(new MemoryStateARM64(addrProtoval, valProtoval));
                    }

                    virtual BaseSemantics::MemoryStatePtr create(const BaseSemantics::SValuePtr &addrProtoval, const BaseSemantics::SValuePtr &valProtoval) const {
                        return instance(addrProtoval, valProtoval);
                    }

                    static MemoryStateARM64Ptr promote(const BaseSemantics::MemoryStatePtr &from) {
                        MemoryStateARM64Ptr retval = boost::dynamic_pointer_cast<MemoryStateARM64>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual BaseSemantics::MemoryStatePtr clone() const {
                        ASSERT_not_implemented("MemoryState::clone() should not be called with Dyninst's SymEval policy");
                    }

                    virtual void clear() {
                        ASSERT_not_implemented("MemoryState::clear() should not be called with Dyninst's SymEval policy");
                    }

                    virtual bool merge(const BaseSemantics::MemoryStatePtr &other, BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps) {
                        return true;
                    }

                public:
                    virtual BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &address, const BaseSemantics::SValuePtr &dflt,
                                                                BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps);

                    virtual void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &value,
                                             BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps);

                    void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSematics::SValuePtr &value,
                                       Dyninst::DataflowAPI::Result_t &res, std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap);
                };


                /***************************************************************************************************/
                /*                                                State                                            */
                /***************************************************************************************************/

                typedef boost::shared_ptr<class StateARM64> StateARM64Ptr;

                class StateARM64 : public BaseSemantics::State {
                public:
                    StateARM64(Dyninst::DataflowAPI::Result_t &r,
                               Dyninst::Address a,
                               Dyninst::Architecture ac,
                               Dyninst::InstructionAPI::Instruction::Ptr insn_,
                               const BaseSemantics::RegisterStatePtr &registers,
                               const BaseSemantics::MemoryStatePtr &memory): BaseSemantics::State(registers, memory) {
                        for (Dyninst::DataflowAPI::Result_t::iterator iter = r.begin();
                             iter != r.end(); ++iter) {
                            Dyninst::Assignment::Ptr a = iter->first;
                            // For a different instruction...
                            if (a->addr() != addr)
                                continue;
                            Dyninst::AbsRegion &o = a->out();

                            if (o.containsOfType(Dyninst::Absloc::Register)) {
                                // We're assuming this is a single register...
                                //std::cerr << "Marking register " << a << std::endl;
                                aaMap[o.absloc()] = a;
                            }
                            else {
                                // Use sufficiently-unique (Heap,0) Absloc
                                // to represent a definition to a memory absloc
                                aaMap[Absloc(0)] = a;
                            }
                        }
                    }

                public:
                    static StateARM64Ptr instance(Dyninst::DataflowAPI::Result_t &r,
                                                  Dyninst::Address a,
                                                  Dyninst::Architecture ac,
                                                  Dyninst::InstructionAPI::Instruction::Ptr insn_,
                                                  const BaseSemantics::RegisterStatePtr &registers,
                                                  const BaseSemantics::MemoryStatePtr &memory) {
                        return StateARM64Ptr(new StateARM64(r, a, ac, insn_, registers, memory));
                    }

                    virtual BaseSemantics::StatePtr create(Dyninst::DataflowAPI::Result_t &r,
                                                 Dyninst::Address a,
                                                 Dyninst::Architecture ac,
                                                 Dyninst::InstructionAPI::Instruction::Ptr insn_,
                                                 const BaseSemantics::RegisterStatePtr &registers,
                                                 const BaseSemantics::MemoryStatePtr &memory) const {
                        return instance(r, a, ac, insn_, registers, memory);
                    }

                    static StateARM64Ptr promote(const BaseSemantics::StatePtr &from) {
                        StateARM64Ptr retval = boost::dynamic_pointer_cast<StateARM64>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual void writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *ops);
                    virtual void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *addrOps,
                                             BaseSemantics::RiscOperators *valOps);

                protected:
                    Dyninst::DataflowAPI::Result_t &res;
                    Dyninst::Architecture arch;
                    Dyninst::Address addr;
                    Dyninst::InstructionAPI::Instruction::Ptr insn;

                    std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> aaMap;
                };


                /***************************************************************************************************/
                /*                                          RiscOperators                                          */
                /***************************************************************************************************/
                typedef boost::shared_ptr<class RiscOperators> RiscOperatorsPtr;

                /** RISC operators for use by the Symbolic Semantics Domain of Dyninst.
                 *
                 */
                class RiscOperators : public BaseSemantics::RiscOperators {
                protected:
                    RiscOperators(const BaseSemantics::SValuePtr &protoval, SMTSolver *solver = NULL)
                            : BaseSemantics::RiscOperators(protoval, solver) {
                        (void)SValue::promote(protoval);
                    }

                    RiscOperators(const BaseSemantics::StatePtr &state, SMTSolver *solver = NULL)
                            : BaseSemantics::RiscOperators(state, solver) {
                        (void)SValue::promote(state->protoval());
                    }

                public:
                    static RiscOperatorsPtr instance(const BaseSemantics::SValuePtr &protoval,
                                                     SMTSolver *solver = NULL) {
                        return RiscOperatorsPtr(new RiscOperators(protoval, solver));
                    }

                    static RiscOperatorsPtr instance(const BaseSemantics::StatePtr &state, SMTSolver *solver = NULL) {
                        return RiscOperatorsPtr(new RiscOperators(state, solver));
                    }

                public:
                    virtual BaseSemantics::RiscOperatorsPtr create(const BaseSemantics::SValuePtr &protoval,
                                                                   SMTSolver *solver = NULL) {
                        return instance(protoval, solver);
                    }

                    virtual BaseSemantics::RiscOperatorsPtr create(const BaseSemantics::StatePtr &state,
                                                                   SMTSolver *solver = NULL) {
                        return instance(state, solver);
                    }

                public:
                    /** Run-time promotion of a base RiscOperators pointer to symbolic operators. This is a checked conversion--it
                    *  will fail if @p x does not point to a SymbolicSemantics::RiscOperators object. */
                    static RiscOperatorsPtr promote(const BaseSemantics::RiscOperatorsPtr &x) {
                        RiscOperatorsPtr retval = boost::dynamic_pointer_cast<RiscOperators>(x);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual BaseSemantics::SValuePtr boolean_(bool b) {
                        SValuePtr retVal = SValue::promote(BaseSemantics::RiscOperators::boolean_(b));
                        return retval;
                    }

                    virtual BaseSemantics::SValuePtr number_(size_t nbits, uint64_t value) {
                        SValuePtr retval = SValue::promote(BaseSemantics::RiscOperators::number_(nbits, value));
                        return retval;
                    }

                public:
                    virtual BaseSemantics::SValuePtr and_(const BaseSemantics::SValuePtr &a_,
                                                          const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr or_(const BaseSemantics::SValuePtr &a_,
                                                          const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr xor_(const BaseSemantics::SValuePtr &a_,
                                                          const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr invert(const BaseSemantics::SValuePtr &a_);
                    virtual BaseSemantics::SValuePtr extract(const BaseSemantics::SValuePtr &a_,
                                                             size_t begin, size_t end);
                    virtual BaseSemantics::SValuePtr ite(const BaseSemantics::SValuePtr &sel_,
                                                         const BaseSemantics::SValuePtr &a_,
                                                         const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &reg,
                                                                  const BaseSemantics::SValuePtr &dflt);
                    virtual void writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &a_);
                };
            }
        }
    }
}

#endif //DYNINST_SYMEVALSEMANTICS_H
