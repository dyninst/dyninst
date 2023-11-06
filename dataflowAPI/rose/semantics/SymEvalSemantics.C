//
// Created by ssunny on 7/1/16.
//

#include "SymEvalSemantics.h"

#include "Register.h"
#include "BaseSemantics2.h"
#include "dyn_regs.h"

using namespace rose::BinaryAnalysis::InstructionSemantics2;
using RoseException = BaseSemantics::Exception;

///////////////////////////////////////////////////////
//                                           StateAST
///////////////////////////////////////////////////////

BaseSemantics::SValuePtr SymEvalSemantics::StateAST::readRegister(const RegisterDescriptor &desc,
                                                                    const BaseSemantics::SValuePtr &/*dflt*/, BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_require(desc.is_valid());
    SymEvalSemantics::RegisterStateASTPtr registers = SymEvalSemantics::RegisterStateAST::promote(registerState());
    return registers->readRegister(desc, addr);
}

void SymEvalSemantics::StateAST::writeRegister(const RegisterDescriptor &reg, const BaseSemantics::SValuePtr &value,
                                                 BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_require(reg.is_valid());
    ASSERT_not_null(value);
    SymEvalSemantics::RegisterStateASTPtr registers = SymEvalSemantics::RegisterStateAST::promote(registerState());
    registers->writeRegister(reg, value, res, aaMap);
}

BaseSemantics::SValuePtr SymEvalSemantics::StateAST::readMemory(const BaseSemantics::SValuePtr &address,
                                                                  const BaseSemantics::SValuePtr &/*dflt*/,
                                                                  BaseSemantics::RiscOperators */*addrOps*/,
                                                                  BaseSemantics::RiscOperators */*valOps*/,
                                                                  size_t readSize) {
    ASSERT_not_null(address);
    SymEvalSemantics::MemoryStateASTPtr memory = SymEvalSemantics::MemoryStateAST::promote(memoryState());
    return memory->readMemory(address, readSize);
}

void SymEvalSemantics::StateAST::writeMemory(const BaseSemantics::SValuePtr &addr,
                                               const BaseSemantics::SValuePtr &value,
                                               BaseSemantics::RiscOperators */*addrOps*/,
                                               BaseSemantics::RiscOperators */*valOps*/,
                                               size_t writeSize) {
    ASSERT_not_null(addr);
    ASSERT_not_null(value);
    SymEvalSemantics::MemoryStateASTPtr memory = SymEvalSemantics::MemoryStateAST::promote(memoryState());
    memory->writeMemory(addr, value, res, aaMap, writeSize);
}

void SymEvalSemantics::StateAST::writeMemory(const BaseSemantics::SValuePtr &addr, 
                                             const BaseSemantics::SValuePtr &value,
					     BaseSemantics::RiscOperators *addrOps, 
					     BaseSemantics::RiscOperators *valOps) {
    ASSERT_not_null(addr);
    ASSERT_not_null(value);
    SymEvalSemantics::MemoryStateASTPtr memory = SymEvalSemantics::MemoryStateAST::promote(memoryState());
    memory->writeMemory(addr, value, res, aaMap, value->get_width());
}					    



///////////////////////////////////////////////////////
//                                   RegisterStateAST
///////////////////////////////////////////////////////

BaseSemantics::SValuePtr SymEvalSemantics::RegisterStateAST::readRegister(const RegisterDescriptor &reg,
                                                                            Dyninst::Address addr) {
    return SymEvalSemantics::SValue::instance(wrap(convert(reg), addr),reg.get_nbits());
}

BaseSemantics::SValuePtr SymEvalSemantics::RegisterStateAST::readRegister(const RegisterDescriptor &reg,
                                                                            const BaseSemantics::SValuePtr &/*dflt*/,
                                                                            BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_always_forbid("overridden RegisterState::readRegister() should never be called for AST, always use the non-virtual readRegister that takes Dyninst::Address as an argument.");
}

void SymEvalSemantics::RegisterStateAST::writeRegister(const RegisterDescriptor &reg,
                                                         const BaseSemantics::SValuePtr &value,
                                                         Dyninst::DataflowAPI::Result_t &res,
                                                         std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap) {
    std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr>::iterator i = aaMap.find(convert(reg));
    if (i != aaMap.end()) {
        SymEvalSemantics::SValuePtr value_ = SymEvalSemantics::SValue::promote(value);
        res[i->second] = value_->get_expression();
    }
}

void SymEvalSemantics::RegisterStateAST::writeRegister(const RegisterDescriptor &/*reg*/,
                                                         const BaseSemantics::SValuePtr &/*value*/,
                                                         BaseSemantics::RiscOperators */*ops*/) {
    ASSERT_always_forbid("overridden RegisterState::writeRegister() should never be called for AST, always use the non-virtual writeRegister that also takes additional parameters.");
}

Dyninst::Absloc SymEvalSemantics::RegisterStateAST::convert(const RegisterDescriptor &reg) {
    ASSERT_always_forbid("converting ROSE register to Dyninst register is platform specific, should not call this base class method.");
}

Dyninst::Absloc SymEvalSemantics::RegisterStateAST_amdgpu_gfx908::convert(const RegisterDescriptor &reg) {
    Dyninst::MachRegister mreg;

    unsigned int major = reg.get_major();
    unsigned int minor = reg.get_minor();
    unsigned int size = reg.get_nbits();
    unsigned int offset = reg.get_offset();
    //std::cout << "in func " << __func__ << " major = " << major  << " minor = " << minor << std::endl;
    bool found = false;
    switch (major) {
    case amdgpu_regclass_sgpr : {
        Dyninst::MachRegister base = Dyninst::amdgpu_gfx908::s0;
        //std::cout << "dealing with sgpr pair in , offset = " << minor << std::endl;
        mreg  = Dyninst::MachRegister(base.val() + minor) ;

        found = true;
        break;
    }
    case amdgpu_regclass_pc : {
        mreg = Dyninst::amdgpu_gfx908::pc_all;

        found = true;
        break;
    }
    case amdgpu_regclass_hwr : {
        switch(minor){
        case amdgpu_status:{
            switch (offset) {
            case 0:
                mreg = Dyninst::amdgpu_gfx908::src_scc;
                found = true;
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;

        }
        break;
    }
    default:
        ASSERT_always_forbid("Unexpected register major type.");
    }
    if(found)
        return Dyninst::Absloc(mreg);

    ASSERT_always_forbid("Unexpected register major type.");
}

Dyninst::Absloc SymEvalSemantics::RegisterStateAST_amdgpu_gfx90a::convert(const RegisterDescriptor &reg) {
    Dyninst::MachRegister mreg;

    unsigned int major = reg.get_major();
    unsigned int minor = reg.get_minor();
    unsigned int size = reg.get_nbits();
    unsigned int offset = reg.get_offset();
    //std::cout << "in func " << __func__ << " major = " << major  << " minor = " << minor << std::endl;
    bool found = false;
    switch (major) {
    case amdgpu_regclass_sgpr : {
        Dyninst::MachRegister base = Dyninst::amdgpu_gfx90a::s0;
        //std::cout << "dealing with sgpr pair in , offset = " << minor << std::endl;
        mreg  = Dyninst::MachRegister(base.val() + minor) ;

        found = true;
        break;
    }
    case amdgpu_regclass_pc : {
        mreg = Dyninst::amdgpu_gfx90a::pc_all;

        found = true;
        break;
    }
    case amdgpu_regclass_hwr : {
        switch(minor){
        case amdgpu_status:{
            switch (offset) {
            case 0:
                mreg = Dyninst::amdgpu_gfx90a::src_scc;
                found = true;
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;

        }
        break;
    }
    default:
        ASSERT_always_forbid("Unexpected register major type.");
    }
    if(found)
        return Dyninst::Absloc(mreg);

    ASSERT_always_forbid("Unexpected register major type.");
}

Dyninst::Absloc SymEvalSemantics::RegisterStateAST_amdgpu_gfx940::convert(const RegisterDescriptor &reg) {
    Dyninst::MachRegister mreg;

    unsigned int major = reg.get_major();
    unsigned int minor = reg.get_minor();
    unsigned int size = reg.get_nbits();
    unsigned int offset = reg.get_offset();
    //std::cout << "in func " << __func__ << " major = " << major  << " minor = " << minor << std::endl;
    bool found = false;
    switch (major) {
    case amdgpu_regclass_sgpr : {
        Dyninst::MachRegister base = Dyninst::amdgpu_gfx940::s0;
        //std::cout << "dealing with sgpr pair in , offset = " << minor << std::endl;
        mreg  = Dyninst::MachRegister(base.val() + minor) ;

        found = true;
        break;
    }
    case amdgpu_regclass_pc : {
        mreg = Dyninst::amdgpu_gfx940::pc_all;

        found = true;
        break;
    }
    case amdgpu_regclass_hwr : {
        switch(minor){
        case amdgpu_status:{
            switch (offset) {
            case 0:
                mreg = Dyninst::amdgpu_gfx940::src_scc;
                found = true;
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;

        }
        break;
    }
    default:
        ASSERT_always_forbid("Unexpected register major type.");
    }
    if(found)
        return Dyninst::Absloc(mreg);

    ASSERT_always_forbid("Unexpected register major type.");
}


Dyninst::Absloc SymEvalSemantics::RegisterStateASTARM64::convert(const RegisterDescriptor &reg) {
    Dyninst::MachRegister mreg;

    unsigned int major = reg.get_major();
    unsigned int size = reg.get_nbits();

    switch (major) {
        case armv8_regclass_gpr: {
            unsigned int minor = reg.get_minor();

            if (minor == armv8_gpr_zr) {
                mreg = Dyninst::MachRegister((size == 32) ? Dyninst::aarch64::wzr : Dyninst::aarch64::xzr);
            } else {
                Dyninst::MachRegister base = (size == 32) ? Dyninst::aarch64::w0 : Dyninst::aarch64::x0;
                mreg = Dyninst::MachRegister(base.val() + (minor - armv8_gpr_r0));
            }
        }
            break;
        case armv8_regclass_simd_fpr: {
            Dyninst::MachRegister base;
            unsigned int minor = reg.get_minor();

            switch(size) {
                case 8: base = Dyninst::aarch64::b0;
                    break;
                case 16: base = Dyninst::aarch64::h0;
                    break;
                case 32: base = Dyninst::aarch64::s0;
                    break;
                case 64:
                    if (reg.get_offset() == 64) {
                        base = Dyninst::aarch64::hq0;
                    } else {
                        base = Dyninst::aarch64::d0;
                    }
                    break;
                case 128: base = Dyninst::aarch64::q0;
                    break;
                default:
                    throw RoseException("invalid size of RegisterDescriptor!", nullptr);
                    break;
            }
            mreg = Dyninst::MachRegister(base.val() + (minor - armv8_simdfpr_v0));
        }
            break;
        case armv8_regclass_pc:
            mreg = Dyninst::MachRegister(Dyninst::aarch64::pc);
            break;
        case armv8_regclass_sp:
            mreg = Dyninst::MachRegister((size == 32) ? Dyninst::aarch64::wsp : Dyninst::aarch64::sp);
            break;
        case armv8_regclass_pstate: {
            unsigned int offset = reg.get_offset();
            switch (offset) {
                case armv8_pstatefield_n:
                    mreg = Dyninst::MachRegister(Dyninst::aarch64::n);
                    break;
                case armv8_pstatefield_z:
                    mreg = Dyninst::MachRegister(Dyninst::aarch64::z);
                    break;
                case armv8_pstatefield_c:
                    mreg = Dyninst::MachRegister(Dyninst::aarch64::c);
                    break;
                case armv8_pstatefield_v:
                    mreg = Dyninst::MachRegister(Dyninst::aarch64::v);
                    break;
                default:
                    ASSERT_always_forbid("No part of the PSTATE register other than NZCV should be used.");
                    break;
            }
        }
            break;
        default:
            ASSERT_always_forbid("Unexpected register major type.");
    }

    return Dyninst::Absloc(mreg);
}

Dyninst::Absloc SymEvalSemantics::RegisterStateASTPPC32::convert(const RegisterDescriptor &reg) {
    Dyninst::MachRegister mreg;

    unsigned int major = reg.get_major();
    unsigned int size = reg.get_nbits();

    switch (major) {
        case powerpc_regclass_gpr: {
            unsigned int minor = reg.get_minor();
	    Dyninst::MachRegister base = Dyninst::ppc32::r0;

	    // For some reason, ROSE does not provide a enum representation for power registers
	    mreg = Dyninst::MachRegister(base.val() + minor);
        }
            break;
        case powerpc_regclass_fpr: {
            Dyninst::MachRegister base = Dyninst::ppc32::fpr0;
            unsigned int minor = reg.get_minor();
            mreg = Dyninst::MachRegister(base.val() + minor);
        }
            break;
	    
	case powerpc_regclass_cr: {
	    unsigned int offset = reg.get_offset();
	    if (size == 32) {
	        mreg = Dyninst::ppc32::cr;
	    }if (size == 4) {
	        Dyninst::MachRegister base = Dyninst::ppc32::cr0;
		mreg = Dyninst::MachRegister(base.val() + offset / 4);
	    } else if (size == 1) {
	        Dyninst::MachRegister base = Dyninst::ppc32::cr0l;
		mreg = Dyninst::MachRegister(base.val() + offset);
	    } else {
	        throw RoseException("bad cr register size", nullptr);
	    }
	}
	    break;
	
	case powerpc_regclass_fpscr:
	    throw RoseException("not implemented register class fpscr", nullptr);
	    break;

	case powerpc_regclass_spr: {
	    unsigned int minor = reg.get_minor();
	    switch (minor) {
	        case powerpc_spr_xer: 
		    mreg = Dyninst::ppc32::xer;
		    break;
		case powerpc_spr_lr:
		    mreg = Dyninst::ppc32::lr;
		    break;
		case powerpc_spr_ctr:
		    mreg = Dyninst::ppc32::ctr;
		    break;
		case powerpc_spr_dsisr:
		    mreg = Dyninst::ppc32::dsisr;
		    break;
		case powerpc_spr_dar:
		    mreg = Dyninst::ppc32::dar;
		    break;
		case powerpc_spr_dec:
		    mreg = Dyninst::ppc32::dec;
		    break;
		default:
		    throw RoseException("not implemented special register", nullptr);
	    }
	}
	    break;
	case powerpc_regclass_tbr:
	    throw RoseException("not implemented regclass tbr", nullptr);
	    break;
	
	case powerpc_regclass_msr:
	    mreg = Dyninst::ppc32::msr;
	    break;
	    
	case powerpc_regclass_sr:
	    throw RoseException("not implemented regclass sr", nullptr);
	    break;

        case powerpc_regclass_iar:
            mreg = Dyninst::ppc32::pc;
            break;
	    
	case powerpc_regclass_pvr:
	    mreg = Dyninst::ppc32::pvr;
	    break;
        default:
            ASSERT_always_forbid("Unexpected register major type.");
    }

    return Dyninst::Absloc(mreg);
}

Dyninst::Absloc SymEvalSemantics::RegisterStateASTPPC64::convert(const RegisterDescriptor &reg) {
    Dyninst::MachRegister mreg;

    unsigned int major = reg.get_major();
    unsigned int size = reg.get_nbits();

    switch (major) {
        case powerpc_regclass_gpr: {
            unsigned int minor = reg.get_minor();
	    Dyninst::MachRegister base = Dyninst::ppc64::r0;

	    // For some reason, ROSE does not provide a enum representation for power registers
	    mreg = Dyninst::MachRegister(base.val() + minor);
        }
            break;
        case powerpc_regclass_fpr: {
            Dyninst::MachRegister base = Dyninst::ppc64::fpr0;
            unsigned int minor = reg.get_minor();
            mreg = Dyninst::MachRegister(base.val() + minor);
        }
            break;
	    
	case powerpc_regclass_cr: {
	    unsigned int offset = reg.get_offset();
	    if (size == 32) {
	        mreg = Dyninst::ppc64::cr;
	    } else if (size == 4) {
	        Dyninst::MachRegister base = Dyninst::ppc64::cr0;
		mreg = Dyninst::MachRegister(base.val() + offset / 4);
	    } else if (size == 1) {
	        Dyninst::MachRegister base = Dyninst::ppc64::cr0l;
		mreg = Dyninst::MachRegister(base.val() + offset);
	    } else {
	        throw RoseException("bad cr register size", nullptr);
	    }
	}
	    break;
	
	case powerpc_regclass_fpscr:
	    throw RoseException("not implemented register class fpscr", nullptr);
	    break;

	case powerpc_regclass_spr: {
	    unsigned int minor = reg.get_minor();
	    switch (minor) {
	        case powerpc_spr_xer: 
		    mreg = Dyninst::ppc64::xer;
		    break;
		case powerpc_spr_lr:
		    mreg = Dyninst::ppc64::lr;
		    break;
		case powerpc_spr_ctr:
		    mreg = Dyninst::ppc64::ctr;
		    break;
		case powerpc_spr_dsisr:
		    mreg = Dyninst::ppc64::dsisr;
		    break;
		case powerpc_spr_dar:
		    mreg = Dyninst::ppc64::dar;
		    break;
		case powerpc_spr_dec:
		    mreg = Dyninst::ppc64::dec;
		    break;
		default:
		    throw RoseException("not implemented special register", nullptr);
	    }
	}
	    break;
	case powerpc_regclass_tbr:
	    throw RoseException("not implemented regclass tbr", nullptr);
	    break;
	
	case powerpc_regclass_msr:
	    mreg = Dyninst::ppc64::msr;
	    break;
	    
	case powerpc_regclass_sr:
	    throw RoseException("not implemented regclass sr", nullptr);
	    break;

        case powerpc_regclass_iar:
            mreg = Dyninst::ppc64::pc;
            break;
	    
	case powerpc_regclass_pvr:
	    mreg = Dyninst::ppc64::pvr;
	    break;
        default:
            ASSERT_always_forbid("Unexpected register major type.");
    }

    return Dyninst::Absloc(mreg);
}


///////////////////////////////////////////////////////
//                                     MemoryStateAST
///////////////////////////////////////////////////////

//TODO: what is Len in this case?

BaseSemantics::SValuePtr SymEvalSemantics::MemoryStateAST::readMemory(const BaseSemantics::SValuePtr &address,
                                                                        size_t readSize) {
    SymEvalSemantics::SValuePtr addr = SymEvalSemantics::SValue::promote(address);
    return SymEvalSemantics::SValue::instance(Dyninst::DataflowAPI::RoseAST::create(Dyninst::DataflowAPI::ROSEOperation(Dyninst::DataflowAPI::ROSEOperation::derefOp, readSize),
                                                                                    addr->get_expression(),
                                                                                    Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(1, 1))));
}

BaseSemantics::SValuePtr SymEvalSemantics::MemoryStateAST::readMemory(const BaseSemantics::SValuePtr &address,
                                                                        const BaseSemantics::SValuePtr &dflt,
                                                                        BaseSemantics::RiscOperators */*addrOps*/,
                                                                        BaseSemantics::RiscOperators */*valOps*/) {
    SymEvalSemantics::SValuePtr addr = SymEvalSemantics::SValue::promote(address);
    return SymEvalSemantics::SValue::instance(Dyninst::DataflowAPI::RoseAST::create(Dyninst::DataflowAPI::ROSEOperation(Dyninst::DataflowAPI::ROSEOperation::derefOp, dflt->get_number()),
                                                                                    addr->get_expression(),
                                                                                    Dyninst::DataflowAPI::ConstantAST::create(Dyninst::DataflowAPI::Constant(1, 1))));

}

void SymEvalSemantics::MemoryStateAST::writeMemory(const BaseSemantics::SValuePtr &address,
                                                     const BaseSemantics::SValuePtr &value,
                                                     Dyninst::DataflowAPI::Result_t &res,
                                                     std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr> &aaMap,
                                                     size_t writeSize) {
    std::map<Dyninst::Absloc, Dyninst::Assignment::Ptr>::iterator i = aaMap.find(Dyninst::Absloc(0));
    SymEvalSemantics::SValuePtr addr = SymEvalSemantics::SValue::promote(address);

    if (i != aaMap.end()) {
        i->second->out().setGenerator(addr->get_expression());
        //i->second->out().setSize(Len);

        SymEvalSemantics::SValuePtr data = SymEvalSemantics::SValue::promote(value);
        res[i->second] = data->get_expression();
    }
}

void SymEvalSemantics::MemoryStateAST::writeMemory(const BaseSemantics::SValuePtr &addr,
                                                     const BaseSemantics::SValuePtr &value,
                                                     BaseSemantics::RiscOperators */*addrOps*/,
                                                     BaseSemantics::RiscOperators */*valOps*/) {
     ASSERT_always_forbid("overridden MemoryState::writeMemory() should never be called for AST, always use the non-virtual writeMemory that also takes additional parameters.");
}

///////////////////////////////////////////////////////
//                                        RiscOperators
///////////////////////////////////////////////////////

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::and_(const BaseSemantics::SValuePtr &a_,
                                                               const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::andOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::or_(const BaseSemantics::SValuePtr &a_,
                                                              const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::orOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::xor_(const BaseSemantics::SValuePtr &a_,
                                                               const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::xorOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::invert(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::invertOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::extract(const BaseSemantics::SValuePtr &a_, uint64_t begin,
                                                                  uint64_t end) {
    BaseSemantics::SValuePtr begin_ = SymEvalSemantics::SValue::instance(64, begin);
    BaseSemantics::SValuePtr end_ = SymEvalSemantics::SValue::instance(64, end);

    return createTernaryAST(Dyninst::DataflowAPI::ROSEOperation::extractOp, a_, begin_, end_, end - begin);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::ite(const BaseSemantics::SValuePtr &sel_,
                                                              const BaseSemantics::SValuePtr &a_,
                                                              const BaseSemantics::SValuePtr &b_) {
    return createTernaryAST(Dyninst::DataflowAPI::ROSEOperation::ifOp, sel_, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::concat(const BaseSemantics::SValuePtr &a_,
                                                                 const BaseSemantics::SValuePtr &b_) {
    //TODO: should be able to specify number of bits to concat for each expression
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::concatOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::leastSignificantSetBit(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::LSBSetOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::mostSignificantSetBit(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::MSBSetOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::rotateLeft(const BaseSemantics::SValuePtr &a_,
                                                                     const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::rotateLOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::rotateRight(const BaseSemantics::SValuePtr &a_,
                                                                      const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::rotateROp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::shiftLeft(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::shiftLOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::shiftRight(const BaseSemantics::SValuePtr &a_,
                                                                     const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::shiftROp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::shiftRightArithmetic(const BaseSemantics::SValuePtr &a_,
                                                                               const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::shiftRArithOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::equalToZero(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::equalToZeroOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::signExtend(const BaseSemantics::SValuePtr &a_,
                                                                     uint64_t newwidth) {
    BaseSemantics::SValuePtr width_ = SymEvalSemantics::SValue::instance(64, newwidth);

    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::signExtendOp, a_, width_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::add(const BaseSemantics::SValuePtr &a_,
                                                              const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::addOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::addWithCarries(const BaseSemantics::SValuePtr &a_,
                                                                         const BaseSemantics::SValuePtr &b_,
                                                                         const BaseSemantics::SValuePtr &c_,
                                                                         BaseSemantics::SValuePtr &carry_out) {
    BaseSemantics::SValuePtr aa_ = unsignedExtend(a_, a_->get_width() + 1);
    BaseSemantics::SValuePtr bb_ = unsignedExtend(b_, b_->get_width() + 1);
    BaseSemantics::SValuePtr sum_ = createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::addOp, aa_,
                                                    createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::addOp, bb_, c_));

    BaseSemantics::SValuePtr cc_ = createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::xorOp, aa_,
                                                   createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::xorOp, bb_, sum_));
    carry_out = extract(cc_, 1, a_->get_width() + 1);

    return extract(sum_, 0, a_->get_width());
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::negate(const BaseSemantics::SValuePtr &a_) {
    return createUnaryAST(Dyninst::DataflowAPI::ROSEOperation::negateOp, a_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::signedDivide(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::sDivOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::signedModulo(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::sModOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::signedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                         const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::sMultOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::unsignedDivide(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::uDivOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::unsignedModulo(const BaseSemantics::SValuePtr &a_,
                                                                       const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::uModOp, a_, b_);
}

BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::unsignedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                         const BaseSemantics::SValuePtr &b_) {
    return createBinaryAST(Dyninst::DataflowAPI::ROSEOperation::uMultOp, a_, b_);
}

//TODO: do we deal with byte ordering?
BaseSemantics::SValuePtr SymEvalSemantics::RiscOperatorsAST::readMemory(const RegisterDescriptor &/*segreg*/,
                                                                     const BaseSemantics::SValuePtr &addr,
                                                                     const BaseSemantics::SValuePtr &dflt,
                                                                     const BaseSemantics::SValuePtr &/*cond*/) {
    return currentState()->readMemory(addr, dflt, this, this);
}

void SymEvalSemantics::RiscOperatorsAST::writeMemory(const RegisterDescriptor &/*segreg*/,
                                                  const BaseSemantics::SValuePtr &addr,
                                                  const BaseSemantics::SValuePtr &data,
                                                  const BaseSemantics::SValuePtr &/*cond*/) {
    currentState()->writeMemory(addr, data, this, this);
}
