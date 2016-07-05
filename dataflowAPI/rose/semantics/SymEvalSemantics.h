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

                    SValue(Dyninst::Absloc r, Dyninst::ParseAPI::Address addr): BaseSemantics::SValue(0) {
                        //FIXME
                    }

                    SValue(size_t nbits, uint64_t num): BaseSemantics::SValue(nbits) {
                        expr = Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(num, nbits));
                    }

                    //FIXME
                    SValue(Dyninst::AST::Ptr expr): BaseSemantics::SValue(0) {
                        this->expr = expr;
                    }

                public:
                    virtual BaseSemantics::SValuePtr undefined_(size_t nbits) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::BottomAST::create(false)));
                    }

                    virtual BaseSemantics::SValuePtr unspecified_(size_t nbits) const {
                        //FIXME
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
                        SValuePtr retval(new SValue(*this));
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
                    //TODO
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

                    virtual void print(std::ostream &, BaseSemantics::Formatter &) const {
                        //TODO
                    }

                };

                typedef boost::shared_ptr<class RegisterStateARM64> RegisterStateARM64Ptr;

                class RegisterStateARM64 : public BaseSemantics::RegisterState {
                protected:
                    explicit RegisterStateARM64(const BaseSemantics::SValuePtr &protoval, const RegisterDictionary *regdict): RegisterState(protoval, regdict) {
                        clear();
                    }

                    RegisterStateARM64(Dyninst::DataflowAPI::Result_t &r,
                                       Dyninst::Address a,
                                       Dyninst::Architecture ac,
                                       Dyninst::InstructionAPI::Instruction::Ptr insn_,
                                       const BaseSemantics::SValuePtr &protoval,
                                       const RegisterDictionary *regdict) : RegisterState(protoval, regdict), res(r),
                                                                            addr(a), arch(ac), insn(insn_) {
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
                    virtual BaseSemantics::RegisterStatePtr create(const SValuePtr &protoval, const RegisterDictionary *regdict) const {
                        return RegisterStateARM64Ptr(new RegisterStateARM64(protoval, regdict));
                    }

                    //FIXME
                    virtual BaseSemantics::RegisterStatePtr clone() const {
                        ASSERT_not_implemented("Yet to be implemented");
                    }

                    static RegisterStateARM64Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                        RegisterStateARM64Ptr  retval = boost::dynamic_pointer_cast<RegisterStateARM64>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual void clear();
                    virtual void zero();
                    virtual BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &dflt, BaseSemantics::RiscOperators *ops);
                    virtual void writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *ops);
                    virtual void print(std::ostream &, BaseSemantics::Formatter &) const;
                    virtual bool merge(const RegisterDescriptor &other, BaseSemantics::RiscOperators *ops);

                private:
                    Dyninst::AST::Ptr wrap(Dyninst::Absloc r) {
                        return Dyninst::DataflowAPI::VariableAST::create(Dyninst::DataflowAPI::Variable(Dyninst::AbsRegion(r), addr));
                    }

                    Dyninst::Absloc convert(ARMv8GeneralPurposeRegister r);

                    Dyninst::DataflowAPI::Result_t &res;
                    Dyninst::Architecture arch;
                    Dyninst::Address addr;
                    Dyninst::InstructionAPI::Instruction::Ptr insn;

                    std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> aaMap;
                };


                class MemoryState : public BaseSemantics::MemoryState {

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
                    explicit RiscOperators(const BaseSemantics::SValuePtr &protoval, SMTSolver *solver = NULL)
                            : BaseSemantics::RiscOperators(protoval, solver) {
                        //SValue::promote(protoval);
                    }

                    explicit RiscOperators(const BaseSemantics::StatePtr &state, SMTSolver *solver = NULL)
                            : BaseSemantics::RiscOperators(state, solver) {
                        //SValue::promote(state->protoval());
                    }

                public:
                    static RiscOperatorsPtr instance(const RegisterDictionary *regdict, SMTSolver *solver = NULL) {
                        //init SValue
                        //init RegisterState
                        //init MemoryState
                        //init State
                        //return new RiscOperatorsPtr(new RiscOperators(state, solver));
                    }

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
                        RiscOperatorsPtr retval = boost::dynamic_pointer_cast<RiscOperatorsPtr>(x);
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
