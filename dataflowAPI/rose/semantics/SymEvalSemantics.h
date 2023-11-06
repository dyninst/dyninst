//
// Created by ssunny on 7/1/16.
//

#ifndef DYNINST_SYMEVALSEMANTICS_H
#define DYNINST_SYMEVALSEMANTICS_H

#include <assert.h>
#include <iosfwd>
#include <map>
#include <stddef.h>
#include <stdint.h>
#include "external/rose/armv8InstructionEnum.h"
#include "external/rose/amdgpuInstructionEnum.h"
#include "BaseSemantics2.h"
#include "../../h/SymEval.h"
#include "dyntypes.h"

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
                        expr = Dyninst::DataflowAPI::VariableAST::create(Dyninst::DataflowAPI::Variable(Dyninst::AbsRegion(r), addr));
                    }
                    
                    SValue(size_t nbits, uint64_t num): BaseSemantics::SValue(nbits) {
                        expr = Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(num, nbits));
                    }

                    SValue(Dyninst::AST::Ptr expr_): BaseSemantics::SValue(64) {
                        this->expr = expr_;
                    }
                    // Added this version to set register size according to descriptor 
                    SValue(Dyninst::AST::Ptr expr_, size_t nbits ): BaseSemantics::SValue(nbits) {
                        this->expr = expr_;
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

                    static SValuePtr instance(Dyninst::AST::Ptr expr, size_t nbits) {
                        return SValuePtr(new SValue(expr, nbits));
                    }


                public:
                    virtual BaseSemantics::SValuePtr undefined_(size_t nbits) const {
                        return SValuePtr(new SValue(32, nbits));
                    }

                    virtual BaseSemantics::SValuePtr unspecified_(size_t /*nbits*/) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::BottomAST::create(false)));
                    }

                    //TODO
                    virtual BaseSemantics::SValuePtr bottom_(size_t /*nbits*/) const {
                        return SValuePtr(new SValue(Dyninst::DataflowAPI::BottomAST::create(true)));
                    }

                    virtual BaseSemantics::SValuePtr number_(size_t nbits, uint64_t num) const {
                        return SValuePtr(new SValue(nbits, num));
                    }

                    virtual BaseSemantics::SValuePtr boolean_(bool value) const {
                        return SValuePtr(new SValue(1, value?1:0));
                    }

                    virtual BaseSemantics::SValuePtr copy(size_t new_width = 0) const {
                        SValuePtr retval(new SValue(get_expression()));
                        if (new_width!=0 && new_width!=retval->get_width())
                            retval->set_width(new_width);
                        return retval;
                    }

                    virtual Sawyer::Optional<BaseSemantics::SValuePtr>
                            createOptionalMerge(const BaseSemantics::SValuePtr &/*other*/, const BaseSemantics::MergerPtr&, SMTSolver*) const {
                        ASSERT_not_implemented("SValue::createOptionalMerge not implemented for use in dyninst");
                    }

                public:
                    static SValuePtr promote(const BaseSemantics::SValuePtr &v) {
                        SValuePtr retval = v.dynamicCast<SValue>();
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual Dyninst::AST::Ptr get_expression() const {
                        return expr;
                    }

                    virtual bool isBottom() const {
                        return expr->getID() == Dyninst::AST::V_BottomAST;
                    }

                    virtual bool is_number() const {
                        return expr->getID() == Dyninst::AST::V_ConstantAST;
                    }

                    virtual uint64_t get_number() const {
                        assert(expr->getID() == Dyninst::AST::V_ConstantAST);
                        //TODO
                        Dyninst::DataflowAPI::Constant constant = boost::dynamic_pointer_cast<Dyninst::DataflowAPI::ConstantAST>(expr)->val();
                        return constant.val;
                    }

                    virtual void print(std::ostream &, BaseSemantics::Formatter &) const { }

                };


                /***************************************************************************************************/
                /*                                          Register State                                         */
                /***************************************************************************************************/

                typedef boost::shared_ptr<class RegisterStateAST> RegisterStateASTPtr;
                typedef boost::shared_ptr<class RegisterStateASTARM64> RegisterStateASTARM64Ptr;
                typedef boost::shared_ptr<class RegisterStateASTPPC32> RegisterStateASTPPC32Ptr;
                typedef boost::shared_ptr<class RegisterStateASTPPC64> RegisterStateASTPPC64Ptr;
                typedef boost::shared_ptr<class RegisterStateAST_amdgpu_gfx908> RegisterStateAST_amdgpu_gfx908_Ptr;
                typedef boost::shared_ptr<class RegisterStateAST_amdgpu_gfx90a> RegisterStateAST_amdgpu_gfx90a_Ptr;
                typedef boost::shared_ptr<class RegisterStateAST_amdgpu_gfx940> RegisterStateAST_amdgpu_gfx940_Ptr;
                class RegisterStateAST : public BaseSemantics::RegisterState {
                public:
                    RegisterStateAST(const BaseSemantics::SValuePtr &protoval,
                                     const RegisterDictionary *regdict_) : RegisterState(protoval, regdict_) { }

                public:
                    static RegisterStateASTPtr instance(const BaseSemantics::SValuePtr &protoval,
                                                          const RegisterDictionary *regdict) {
                        return RegisterStateASTPtr(new RegisterStateAST(protoval, regdict));
                    }

                    virtual BaseSemantics::RegisterStatePtr create(const BaseSemantics::SValuePtr &protoval,
                                                                   const RegisterDictionary *regdict_) const {
                        return instance(protoval, regdict_);
                    }

                    virtual BaseSemantics::RegisterStatePtr clone() const {
                        ASSERT_not_implemented("RegisterState::clone() should not be called with Dyninst's SymEval policy");
                    }

                    static RegisterStateASTPtr promote(const BaseSemantics::RegisterStatePtr &from) {
                        RegisterStateASTPtr retval = boost::dynamic_pointer_cast<RegisterStateAST>(from);
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

                    virtual bool merge(const BaseSemantics::RegisterStatePtr &/*other*/, BaseSemantics::RiscOperators * /*ops*/) {
                        return true;
                    }

                    BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &reg, Dyninst::Address addr);
                    void writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value,
                                       Dyninst::DataflowAPI::Result_t &res, std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap);

                    Dyninst::AST::Ptr wrap(Dyninst::Absloc r, Dyninst::Address addr) {
                        return Dyninst::DataflowAPI::VariableAST::create(Dyninst::DataflowAPI::Variable(Dyninst::AbsRegion(r), addr));
                    }

                private:

                    virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);
                };

		class RegisterStateASTARM64 : public RegisterStateAST {
		public:
		    RegisterStateASTARM64(const BaseSemantics::SValuePtr &protoval,
                                          const RegisterDictionary *regdict_) : RegisterStateAST(protoval, regdict_) { }

                    static RegisterStateASTARM64Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                                             const RegisterDictionary *regdict) {
                        return RegisterStateASTARM64Ptr(new RegisterStateASTARM64(protoval, regdict));
                    }

                    static RegisterStateASTARM64Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                        RegisterStateASTARM64Ptr retval = boost::dynamic_pointer_cast<RegisterStateASTARM64>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

		private:
		    virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);
		};
		
		class RegisterStateASTPPC32 : public RegisterStateAST {
		public:
		    RegisterStateASTPPC32(const BaseSemantics::SValuePtr &protoval,
                                          const RegisterDictionary *regdict_) : RegisterStateAST(protoval, regdict_) { }

                    static RegisterStateASTPPC32Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                                             const RegisterDictionary *regdict) {
                        return RegisterStateASTPPC32Ptr(new RegisterStateASTPPC32(protoval, regdict));
                    }

                    static RegisterStateASTPPC32Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                        RegisterStateASTPPC32Ptr retval = boost::dynamic_pointer_cast<RegisterStateASTPPC32>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }
		private:
		    virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);

                };
		class RegisterStateASTPPC64 : public RegisterStateAST {
		public:
		    RegisterStateASTPPC64(const BaseSemantics::SValuePtr &protoval,
                                          const RegisterDictionary *regdict_) : RegisterStateAST(protoval, regdict_) { }

                    static RegisterStateASTPPC64Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                                             const RegisterDictionary *regdict) {
                        return RegisterStateASTPPC64Ptr(new RegisterStateASTPPC64(protoval, regdict));
                    }

                    static RegisterStateASTPPC64Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                        RegisterStateASTPPC64Ptr retval = boost::dynamic_pointer_cast<RegisterStateASTPPC64>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

		private:
		    virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);
		};
	
                class RegisterStateAST_amdgpu_gfx908 : public RegisterStateAST {
                    public:
                        RegisterStateAST_amdgpu_gfx908(const BaseSemantics::SValuePtr &protoval,
                                const RegisterDictionary *regdict_) : RegisterStateAST(protoval, regdict_) { }

                        static RegisterStateAST_amdgpu_gfx908_Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                const RegisterDictionary *regdict) {
                            return RegisterStateAST_amdgpu_gfx908_Ptr(new RegisterStateAST_amdgpu_gfx908(protoval, regdict));
                        }

                        static RegisterStateAST_amdgpu_gfx908_Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                            RegisterStateAST_amdgpu_gfx908_Ptr retval = boost::dynamic_pointer_cast<RegisterStateAST_amdgpu_gfx908>(from);
                            ASSERT_not_null(retval);
                            return retval;
                        }

                    private:
                        // Given a register decriptor of roseformat, convert it back to MachRegister and encapsulate it in Dyninst Abstract location
                        virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);
                };

                 class RegisterStateAST_amdgpu_gfx90a : public RegisterStateAST {
                    public:
                        RegisterStateAST_amdgpu_gfx90a(const BaseSemantics::SValuePtr &protoval,
                                const RegisterDictionary *regdict_) : RegisterStateAST(protoval, regdict_) { }

                        static RegisterStateAST_amdgpu_gfx90a_Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                const RegisterDictionary *regdict) {
                            return RegisterStateAST_amdgpu_gfx90a_Ptr(new RegisterStateAST_amdgpu_gfx90a(protoval, regdict));
                        }

                        static RegisterStateAST_amdgpu_gfx90a_Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                            RegisterStateAST_amdgpu_gfx90a_Ptr retval = boost::dynamic_pointer_cast<RegisterStateAST_amdgpu_gfx90a>(from);
                            ASSERT_not_null(retval);
                            return retval;
                        }

                    private:
                        // Given a register decriptor of roseformat, convert it back to MachRegister and encapsulate it in Dyninst Abstract location
                        virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);
                };
                 class RegisterStateAST_amdgpu_gfx940 : public RegisterStateAST {
                    public:
                        RegisterStateAST_amdgpu_gfx940(const BaseSemantics::SValuePtr &protoval,
                                const RegisterDictionary *regdict_) : RegisterStateAST(protoval, regdict_) { }

                        static RegisterStateAST_amdgpu_gfx940_Ptr instance(const BaseSemantics::SValuePtr &protoval,
                                const RegisterDictionary *regdict) {
                            return RegisterStateAST_amdgpu_gfx940_Ptr(new RegisterStateAST_amdgpu_gfx940(protoval, regdict));
                        }

                        static RegisterStateAST_amdgpu_gfx940_Ptr promote(const BaseSemantics::RegisterStatePtr &from) {
                            RegisterStateAST_amdgpu_gfx940_Ptr retval = boost::dynamic_pointer_cast<RegisterStateAST_amdgpu_gfx940>(from);
                            ASSERT_not_null(retval);
                            return retval;
                        }

                    private:
                        // Given a register decriptor of roseformat, convert it back to MachRegister and encapsulate it in Dyninst Abstract location
                        virtual Dyninst::Absloc convert(const RegisterDescriptor &reg);
                };

                /***************************************************************************************************/
                /*                                           Memory State                                          */
                /***************************************************************************************************/

                typedef boost::shared_ptr<class MemoryStateAST> MemoryStateASTPtr;

                class MemoryStateAST : public BaseSemantics::MemoryState {
                protected:
                    MemoryStateAST(const BaseSemantics::SValuePtr &addrProtoval, const BaseSemantics::SValuePtr &valProtoval):
                            BaseSemantics::MemoryState(addrProtoval, valProtoval) { }

                public:
                    static MemoryStateASTPtr instance(const BaseSemantics::SValuePtr &addrProtoval, const BaseSemantics::SValuePtr &valProtoval) {
                        return MemoryStateASTPtr(new MemoryStateAST(addrProtoval, valProtoval));
                    }

                    virtual BaseSemantics::MemoryStatePtr create(const BaseSemantics::SValuePtr &addrProtoval, const BaseSemantics::SValuePtr &valProtoval) const {
                        return instance(addrProtoval, valProtoval);
                    }

                    static MemoryStateASTPtr promote(const BaseSemantics::MemoryStatePtr &from) {
                        MemoryStateASTPtr retval = boost::dynamic_pointer_cast<MemoryStateAST>(from);
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

                    virtual void print(std::ostream&, BaseSemantics::Formatter&) const {
                        //
                    }

                    virtual bool merge(const BaseSemantics::MemoryStatePtr &/*other*/, BaseSemantics::RiscOperators * /*addrOps*/, BaseSemantics::RiscOperators * /*valOps*/) {
                        return true;
                    }

                public:
                    virtual BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &address, const BaseSemantics::SValuePtr &dflt,
                                                                BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps);

                    BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &address, size_t readSize);

                    virtual void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &value,
                                             BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps);

                    void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &value,
                                       Dyninst::DataflowAPI::Result_t &res, std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap, size_t writeSize);
                };


                /***************************************************************************************************/
                /*                                                State                                            */
                /***************************************************************************************************/

                typedef boost::shared_ptr<class StateAST> StateASTPtr;

                class StateAST : public BaseSemantics::State {
                public:
                    StateAST(Dyninst::DataflowAPI::Result_t &r,
                               Dyninst::Address a,
                               Dyninst::Architecture ac,
                               Dyninst::InstructionAPI::Instruction insn_,
                               const BaseSemantics::RegisterStatePtr &registers,
                               const BaseSemantics::MemoryStatePtr &memory): BaseSemantics::State(registers, memory), res(r), arch(ac), addr(a), insn(insn_) {
                        for (Dyninst::DataflowAPI::Result_t::iterator iter = r.begin();
                             iter != r.end(); ++iter) {
                            Dyninst::Assignment::Ptr ap = iter->first;
                            // For a different instruction...
                            if (ap->addr() != addr)
                                continue;
                            Dyninst::AbsRegion &o = ap->out();

                            if (o.containsOfType(Dyninst::Absloc::Register)) {
                                // We're assuming this is a single register...
                                //std::cerr << "Marking register " << ap << std::endl;
                                aaMap[o.absloc()] = ap;
                            }
                            else {
                                // Use sufficiently-unique (Heap,0) Absloc
                                // to represent a definition to a memory absloc
                                aaMap[Dyninst::Absloc(0)] = ap;
                            }
                        }
                    }

                public:
                    static StateASTPtr instance(Dyninst::DataflowAPI::Result_t &r,
                                                  Dyninst::Address a,
                                                  Dyninst::Architecture ac,
                                                  Dyninst::InstructionAPI::Instruction insn_,
                                                  const BaseSemantics::RegisterStatePtr &registers,
                                                  const BaseSemantics::MemoryStatePtr &memory) {
                        return StateASTPtr(new StateAST(r, a, ac, insn_, registers, memory));
                    }

                    using BaseSemantics::State::create;
                    virtual BaseSemantics::StatePtr create(Dyninst::DataflowAPI::Result_t &r,
                                                 Dyninst::Address a,
                                                 Dyninst::Architecture ac,
                                                 Dyninst::InstructionAPI::Instruction insn_,
                                                 const BaseSemantics::RegisterStatePtr &registers,
                                                 const BaseSemantics::MemoryStatePtr &memory) const {
                        return instance(r, a, ac, insn_, registers, memory);
                    }

                    static StateASTPtr promote(const BaseSemantics::StatePtr &from) {
                        StateASTPtr retval = boost::dynamic_pointer_cast<StateAST>(from);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &dflt, BaseSemantics::RiscOperators *ops);
                    virtual void writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *ops);
                    using BaseSemantics::State::readMemory;
                    virtual BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &address, const BaseSemantics::SValuePtr &dflt,
                                                                BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps, size_t readSize = 0);
                    virtual void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *addrOps,
                                             BaseSemantics::RiscOperators *valOps, size_t writeSize);
                    virtual void writeMemory(const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &value, BaseSemantics::RiscOperators *addrOps,
                                             BaseSemantics::RiscOperators *valOps);

                protected:
                    Dyninst::DataflowAPI::Result_t &res;
                    Dyninst::Architecture arch;
                    Dyninst::Address addr;
                    Dyninst::InstructionAPI::Instruction insn;

                    std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> aaMap;
                };


                /***************************************************************************************************/
                /*                                          RiscOperators                                          */
                /***************************************************************************************************/
                typedef boost::shared_ptr<class RiscOperatorsAST> RiscOperatorsASTPtr;

                /** RISC operators for use by the Symbolic Semantics Domain of Dyninst.
                 *
                 */
                class RiscOperatorsAST : public BaseSemantics::RiscOperators {
                protected:
                    RiscOperatorsAST(const BaseSemantics::SValuePtr &protoval, SMTSolver *solver = NULL)
                            : BaseSemantics::RiscOperators(protoval, solver) {
                        (void)SValue::promote(protoval);
                    }

                    RiscOperatorsAST(const BaseSemantics::StatePtr &state, SMTSolver *solver = NULL)
                            : BaseSemantics::RiscOperators(state, solver) {
                        (void)SValue::promote(state->protoval());
                    }

                public:
                    static RiscOperatorsASTPtr instance(const BaseSemantics::SValuePtr &protoval,
                                                     SMTSolver *solver = NULL) {
                        return RiscOperatorsASTPtr(new RiscOperatorsAST(protoval, solver));
                    }

                    static RiscOperatorsASTPtr instance(const BaseSemantics::StatePtr &state, SMTSolver *solver = NULL) {
                        return RiscOperatorsASTPtr(new RiscOperatorsAST(state, solver));
                    }

                public:
                    virtual BaseSemantics::RiscOperatorsPtr create(const BaseSemantics::SValuePtr &protoval,
                                                                   SMTSolver *solver = NULL) const {
                        return instance(protoval, solver);
                    }

                    virtual BaseSemantics::RiscOperatorsPtr create(const BaseSemantics::StatePtr &state,
                                                                   SMTSolver *solver = NULL) const {
                        return instance(state, solver);
                    }

                public:
                    /** Run-time promotion of a base RiscOperators pointer to symbolic operators. This is a checked conversion--it
                    *  will fail if @p x does not point to a SymbolicSemantics::RiscOperators object. */
                    static RiscOperatorsASTPtr promote(const BaseSemantics::RiscOperatorsPtr &x) {
                        RiscOperatorsASTPtr retval = boost::dynamic_pointer_cast<RiscOperatorsAST>(x);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                public:
                    virtual BaseSemantics::SValuePtr boolean_(bool b) {
                        SValuePtr retval = SValue::promote(BaseSemantics::RiscOperators::boolean_(b));
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
                                                             uint64_t begin, uint64_t end);
                    virtual BaseSemantics::SValuePtr ite(const BaseSemantics::SValuePtr &sel_,
                                                         const BaseSemantics::SValuePtr &a_,
                                                         const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr concat(const BaseSemantics::SValuePtr &a_,
                                                            const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr leastSignificantSetBit(const BaseSemantics::SValuePtr &a_);
                    virtual BaseSemantics::SValuePtr mostSignificantSetBit(const BaseSemantics::SValuePtr &a_);
                    virtual BaseSemantics::SValuePtr rotateLeft(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr rotateRight(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr shiftLeft(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr shiftRight(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr shiftRightArithmetic(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr equalToZero(const BaseSemantics::SValuePtr &a_);
                    virtual BaseSemantics::SValuePtr signExtend(const BaseSemantics::SValuePtr &a_,
                                                                uint64_t newwidth = 0);
                    virtual BaseSemantics::SValuePtr add(const BaseSemantics::SValuePtr &a_,
                                                         const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr addWithCarries(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_,
                                                                    const BaseSemantics::SValuePtr &c_,
                                                                    BaseSemantics::SValuePtr &carry_out);
                    virtual BaseSemantics::SValuePtr negate(const BaseSemantics::SValuePtr &a_);
                    virtual BaseSemantics::SValuePtr signedDivide(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr signedModulo(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr signedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr unsignedDivide(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr unsignedModulo(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);
                    virtual BaseSemantics::SValuePtr unsignedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_);

                public:
                    virtual BaseSemantics::SValuePtr readMemory(const RegisterDescriptor &segreg, const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &dflt,
                                                 const BaseSemantics::SValuePtr &cond);
                    virtual void writeMemory(const RegisterDescriptor &segreg, const BaseSemantics::SValuePtr &addr, const BaseSemantics::SValuePtr &data,
                                             const BaseSemantics::SValuePtr &cond);

                private:
                    Dyninst::AST::Ptr getUnaryAST(Dyninst::DataflowAPI::ROSEOperation::Op op,
                                         Dyninst::AST::Ptr a,
                                         size_t s = 0) {
                        return Dyninst::DataflowAPI::RoseAST::create(Dyninst::DataflowAPI::ROSEOperation(op, s), a);
                    }

                    Dyninst::AST::Ptr getBinaryAST(Dyninst::DataflowAPI::ROSEOperation::Op op,
                                                   Dyninst::AST::Ptr a,
                                                   Dyninst::AST::Ptr b,
                                          size_t s = 0) {
                        return Dyninst::DataflowAPI::RoseAST::create(Dyninst::DataflowAPI::ROSEOperation(op, s), a, b);
                    }

                    Dyninst::AST::Ptr getTernaryAST(Dyninst::DataflowAPI::ROSEOperation::Op op,
                                                    Dyninst::AST::Ptr a,
                                                    Dyninst::AST::Ptr b,
                                                    Dyninst::AST::Ptr c,
                                           size_t s = 0) {
                        return Dyninst::DataflowAPI::RoseAST::create(Dyninst::DataflowAPI::ROSEOperation(op, s), a, b, c);
                    }

                    SValuePtr createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::Op op, const BaseSemantics::SValuePtr &a_) {
                        Dyninst::AST::Ptr a = SValue::promote(a_)->get_expression();
                        Dyninst::AST::Ptr ast = getUnaryAST(op, a);
                        return SValue::instance(ast);
                    }

                    SValuePtr createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::Op op, const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
                        Dyninst::AST::Ptr a = SValue::promote(a_)->get_expression();
                        Dyninst::AST::Ptr b = SValue::promote(b_)->get_expression();
                        Dyninst::AST::Ptr ast = getBinaryAST(op, a, b);
                        return SValue::instance(ast);
                    }

                    SValuePtr createTernaryAST(Dyninst::DataflowAPI::ROSEOperation::Op op, const BaseSemantics::SValuePtr &a_,
                                               const BaseSemantics::SValuePtr &b_, const BaseSemantics::SValuePtr &c_, size_t s = 0) {
                        Dyninst::AST::Ptr a = SValue::promote(a_)->get_expression();
                        Dyninst::AST::Ptr b = SValue::promote(b_)->get_expression();
                        Dyninst::AST::Ptr c = SValue::promote(c_)->get_expression();
                        Dyninst::AST::Ptr ast = getTernaryAST(op, a, b, c, s);
                        return SValue::instance(ast);
                    }

                };
            }
        }
    }
}

#endif //DYNINST_SYMEVALSEMANTICS_H
