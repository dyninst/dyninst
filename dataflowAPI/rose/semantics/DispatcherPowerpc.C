//#include "sage3basic.h"
#include "../util/StringUtility.h"
#include "BaseSemantics2.h"
//#include "Diagnostics.h"
#include "DispatcherPowerpc.h"
#include "../integerOps.h"
#include "SymEvalSemantics.h"

#include "../SgAsmExpression.h"
#include "../conversions.h"

namespace rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {

/*******************************************************************************************************************************
 *                                      Support functions
 *******************************************************************************************************************************/


/*******************************************************************************************************************************
 *                                      Functors that handle individual PowerPC instructions kinds
 *******************************************************************************************************************************/

namespace Powerpc {

// An intermediate class that reduces the amount of typing in all that follows.  Its process() method does some up-front
// checking, dynamic casting, and pointer dereferencing and then calls the p() method that does the real work.
class P: public BaseSemantics::InsnProcessor {
public:
    typedef DispatcherPowerpc *D;
    typedef BaseSemantics::RiscOperators *Ops;
    typedef SgAsmPowerpcInstruction *I;
    typedef const SgAsmExpressionPtrList &A;
    virtual void p(D, Ops, I, A) = 0;

    virtual void process(const BaseSemantics::DispatcherPtr &dispatcher_, SgAsmInstruction *insn_)  {
        DispatcherPowerpcPtr dispatcher = DispatcherPowerpc::promote(dispatcher_);
        BaseSemantics::RiscOperatorsPtr operators = dispatcher->get_operators();
        SgAsmPowerpcInstruction *insn = isSgAsmPowerpcInstruction(insn_);
        ASSERT_require(insn!=NULL && insn==operators->currentInstruction());
        dispatcher->advanceInstructionPointer(insn);
        SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();
        p(dispatcher.get(), operators.get(), insn, operands);
    }

    void assert_args(I insn, A args, size_t nargs) {
        if (args.size()!=nargs) {
            std::string mesg = "instruction must have " + StringUtility::numberToString(nargs) + "argument" + (1==nargs?"":"s");
            throw BaseSemantics::Exception(mesg, insn);
        }
    }

    // Builds mask of 1's from the bit value starting at mb_value to me_value.
    // See page 71 of PowerPC manual.
    uint32_t build_mask(uint8_t mb_value, uint8_t me_value) {
    	uint32_t mask = 0;
    	constexpr uint32_t max_bit_pos{31}, sentinnel{1};

    	if (mb_value <= me_value) {
    		// PowerPC counts bits from the left.
    		for (int i = mb_value; i <= me_value; i++)
    			mask |= (sentinnel << (max_bit_pos - i));
    	} else {
    		for (auto i = mb_value; i <= max_bit_pos; i++)
    			mask |= (sentinnel << (max_bit_pos - i));
    		for (uint8_t i = 0; i <= me_value; i++)
    			mask |= (sentinnel << (max_bit_pos - i));
    	}
    	return mask;
    }

};

// Fixed-point addition
struct IP_add: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        d->write(args[0], ops->add(d->read(args[1], 32), d->read(args[2], 32)));
    }
};

// Fixed-point add carrying
struct IP_addc: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr no = ops->boolean_(false);
        BaseSemantics::SValuePtr result = ops->addWithCarries(d->read(args[1], 32), d->read(args[2], 32),
                                                              no, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr ones = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), ones), v1));
    }
};

// Fixed-point add extended
struct IP_adde: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr carry_in = ops->extract(ops->readRegister(d->REG_XER), 29, 30);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->addWithCarries(d->read(args[1], 32), arg2, carry_in, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr ones = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), ones), v1));
    }
};

// Fixed-point addition
struct IP_addi: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr ra = d->read(args[1], 32);

        // The disassembler should have built this as a DWord with a sign extended value
        BaseSemantics::SValuePtr signExtended_si = ops->signExtend(ops->extract(d->read(args[2], 32), 0, 16), 32);

        d->write(args[0], ops->add(ra, signExtended_si));
    }
};

// Fixed-point add immediate carrying
struct IP_addic: P {
    bool record;
    IP_addic(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr no = ops->boolean_(false);
        BaseSemantics::SValuePtr v1 = ops->signExtend(ops->extract(d->read(args[2], 32), 0, 16), 32);
        BaseSemantics::SValuePtr result = ops->addWithCarries(d->read(args[1], 32), v1, no, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v2 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr ones = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), ones), v2));
        if (record)
            d->record(result);
    }
};

// Fixed-point add immediate shifted
struct IP_addis: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr v1 = ops->extract(d->read(args[2], 32), 0, 16);
        BaseSemantics::SValuePtr v2 = ops->concat(ops->number_(16, 0), v1);
	BaseSemantics::SValuePtr v3 = ops->signExtend(v2, d->addressWidth());
        d->write(args[0], ops->add(d->read(args[1], 32), v3));
    }
};

// Fixed-point add minus one extended
struct IP_addme: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr carry_in = ops->extract(ops->readRegister(d->REG_XER), 29, 30);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr ones = ops->number_(32, 0xffffffffu);
        BaseSemantics::SValuePtr result = ops->addWithCarries(d->read(args[1], 32), ones, carry_in, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffffu is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), ones), v1));
    }
};

// Fixed-point add to zero extended
struct IP_addze: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr carry_in = ops->extract(ops->readRegister(d->REG_XER), 29, 30);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr zero = d->number_(32, 0);
        BaseSemantics::SValuePtr result = ops->addWithCarries(d->read(args[1], 32), zero, carry_in, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr ones = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), ones), v1));
    }
};

// Fixed-point logical AND
struct IP_and: P {
    bool record;
    IP_and(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->and_(d->read(args[1], 32), arg2);
        d->write(args[0], result);
        if (record)
            d->record(result);
    }
};

// Fixed-point logical AND with complement
struct IP_andc: P {
    bool record;
    IP_andc(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr v1 = ops->invert(d->read(args[2], 32));
        BaseSemantics::SValuePtr result = ops->and_(d->read(args[1], 32), v1);
        d->write(args[0], result);
        if (record)
            d->record(result);
    }
};

// Fixed-point logical AND immediate shifted
struct IP_andis: P {
    bool record;
    IP_andis(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr v1 = ops->extract(d->read(args[2], 32), 0, 16);
        BaseSemantics::SValuePtr v2 = ops->concat(ops->number_(16, 0), v1);
        BaseSemantics::SValuePtr result = ops->and_(d->read(args[1], 32), v2);
        d->write(args[0], result);
        if (record)
            d->record(result);
    }
};

// Branch instruction relative (optionally save link)
struct IP_b: P {
    bool save_link;
    IP_b(bool save_link): save_link(save_link) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 1);
        if (save_link)
            ops->writeRegister(d->REG_LR, ops->number_(32, insn->get_address() + 4));
        BaseSemantics::SValuePtr v1 = ops->number_(32, insn->get_address());
        BaseSemantics::SValuePtr target = ops->add(d->read(args[0], 32), v1);
        ops->writeRegister(d->REG_IAR, target);
    }
};

// Branch absolute (optionally save link)
struct IP_ba: P {
    bool save_link;
    IP_ba(bool save_link): save_link(save_link) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 1);
        if (save_link)
            ops->writeRegister(d->REG_LR, ops->number_(32, insn->get_address() + 4));
        ops->writeRegister(d->REG_IAR, d->read(args[0], 32));
    }
};

// Branch conditional instruction relative (optionally save link)
struct IP_bc: P {
    bool save_link;
    IP_bc(bool save_link): save_link(save_link) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        if (save_link)
            ops->writeRegister(d->REG_LR, ops->number_(32, insn->get_address() + 4));
        SgAsmIntegerValueExpression *byteValue = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(byteValue);
        uint8_t boConstant = byteValue->get_value();
        // bool bo_4 = boConstant & 0x1;
        bool bo_3 = boConstant & 0x2;
        bool bo_2 = boConstant & 0x4;
        bool bo_1 = boConstant & 0x8;
        bool bo_0 = boConstant & 0x10;
        if (!bo_2) {
            BaseSemantics::SValuePtr negOne = ops->number_(32, -1);
            ops->writeRegister(d->REG_CTR, ops->add(ops->readRegister(d->REG_CTR), negOne));
        }
        BaseSemantics::SValuePtr ctr_ok;
        if (bo_2) {
            ctr_ok = ops->boolean_(true);
        } else if (bo_3) {
            ctr_ok = ops->equalToZero(ops->readRegister(d->REG_CTR));
        } else {
            ctr_ok = ops->invert(ops->equalToZero(ops->readRegister(d->REG_CTR)));
        }
        SgAsmRegisterReferenceExpression *bi = isSgAsmRegisterReferenceExpression(args[1]);
        ASSERT_require(bi && bi->get_descriptor().get_major() == powerpc_regclass_cr && bi->get_descriptor().get_nbits() == 1);
        BaseSemantics::SValuePtr cr_bi = ops->readRegister(bi->get_descriptor());
        BaseSemantics::SValuePtr cond_ok = bo_0 ? ops->boolean_(true) : bo_1 ? cr_bi : ops->invert(cr_bi);
        BaseSemantics::SValuePtr v1 = ops->number_(32, insn->get_address());
        BaseSemantics::SValuePtr target = (ops->add(d->read(args[2], 32), v1));
        BaseSemantics::SValuePtr iar = ops->readRegister(d->REG_IAR);
        ops->writeRegister(d->REG_IAR, ops->ite(ops->and_(ctr_ok, cond_ok), target, iar));
    }
};

// Branch conditional absolute (optionally save link)
struct IP_bca: P {
    bool save_link;
    IP_bca(bool save_link): save_link(save_link) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        if (save_link)
            ops->writeRegister(d->REG_LR, ops->number_(32, insn->get_address() + 4));
        SgAsmIntegerValueExpression *byteValue = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(byteValue);
        uint8_t boConstant = byteValue->get_value();
        // bool bo_4 = boConstant & 0x1;
        bool bo_3 = boConstant & 0x2;
        bool bo_2 = boConstant & 0x4;
        bool bo_1 = boConstant & 0x8;
        bool bo_0 = boConstant & 0x10;
        if (!bo_2) {
            BaseSemantics::SValuePtr negOne = ops->number_(32, -1);
            ops->writeRegister(d->REG_CTR, ops->add(ops->readRegister(d->REG_CTR), negOne));
        }
        BaseSemantics::SValuePtr ctr_ok;
        if (bo_2) {
            ctr_ok = ops->boolean_(true);
        } else if (bo_3) {
            ctr_ok = ops->equalToZero(ops->readRegister(d->REG_CTR));
        } else {
            ctr_ok = ops->invert(ops->equalToZero(ops->readRegister(d->REG_CTR)));
        }
        SgAsmRegisterReferenceExpression *bi = isSgAsmRegisterReferenceExpression(args[1]);
        ASSERT_require(bi && bi->get_descriptor().get_major() == powerpc_regclass_cr && bi->get_descriptor().get_nbits() == 1);
        BaseSemantics::SValuePtr cr_bi = ops->readRegister(bi->get_descriptor());
        BaseSemantics::SValuePtr cond_ok = bo_0 ? ops->boolean_(true) : bo_1 ? cr_bi : ops->invert(cr_bi);
        BaseSemantics::SValuePtr target = d->read(args[2], 32);
        BaseSemantics::SValuePtr iar = ops->readRegister(d->REG_IAR);
        ops->writeRegister(d->REG_IAR, ops->ite(ops->and_(ctr_ok, cond_ok), target, iar));
    }
};

// Branch conditional to count register
struct IP_bcctr: P {
    bool save_link;
    IP_bcctr(bool save_link): save_link(save_link) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        if (save_link)
            ops->writeRegister(d->REG_LR, ops->number_(32, insn->get_address() + 4));
        SgAsmIntegerValueExpression *byteValue = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(byteValue);
        uint8_t boConstant = byteValue->get_value();
        bool bo_1 = boConstant & 0x8;
        bool bo_0 = boConstant & 0x10;
        SgAsmRegisterReferenceExpression *bi = isSgAsmRegisterReferenceExpression(args[1]);
        ASSERT_require(bi && bi->get_descriptor().get_major() == powerpc_regclass_cr && bi->get_descriptor().get_nbits() == 1);
        BaseSemantics::SValuePtr cr_bi = ops->readRegister(bi->get_descriptor());
        BaseSemantics::SValuePtr cond_ok = bo_0 ? ops->boolean_(true) : bo_1 ? cr_bi : ops->invert(cr_bi);
        BaseSemantics::SValuePtr iar = ops->readRegister(d->REG_IAR);
        BaseSemantics::SValuePtr mask = ops->number_(32, 0xfffffffc);
        ops->writeRegister(d->REG_IAR, ops->ite(cond_ok, ops->and_(ops->readRegister(d->REG_CTR), mask), iar));
    }
};

// Branch conditional (optionally save link)
struct IP_bclr: P {
    bool save_link;
    IP_bclr(bool save_link): save_link(save_link) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        if (save_link)
            ops->writeRegister(d->REG_LR, ops->number_(32, insn->get_address() + 4));
        SgAsmIntegerValueExpression *byteValue = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(byteValue);
        uint8_t boConstant = byteValue->get_value();
        // bool bo_4 = boConstant & 0x1;
        bool bo_3 = boConstant & 0x2;
        bool bo_2 = boConstant & 0x4;
        bool bo_1 = boConstant & 0x8;
        bool bo_0 = boConstant & 0x10;
        if (!bo_2) {
            BaseSemantics::SValuePtr negOne = ops->number_(32, -1);
            ops->writeRegister(d->REG_CTR, ops->add(ops->readRegister(d->REG_CTR), negOne));
        }
        BaseSemantics::SValuePtr ctr_ok;
        if (bo_2) {
            ctr_ok = ops->boolean_(true);
        } else if (bo_3) {
            ctr_ok = ops->equalToZero(ops->readRegister(d->REG_CTR));
        } else {
            ctr_ok = ops->invert(ops->equalToZero(ops->readRegister(d->REG_CTR)));
        }
        SgAsmRegisterReferenceExpression *bi = isSgAsmRegisterReferenceExpression(args[1]);
        ASSERT_require(bi && bi->get_descriptor().get_major() == powerpc_regclass_cr && bi->get_descriptor().get_nbits() == 1);
        BaseSemantics::SValuePtr cr_bi = ops->readRegister(bi->get_descriptor());
        BaseSemantics::SValuePtr cond_ok = bo_0 ? ops->boolean_(true) : bo_1 ? cr_bi : ops->invert(cr_bi);
        BaseSemantics::SValuePtr mask = ops->number_(32, 0xfffffffc);
        BaseSemantics::SValuePtr target = ops->and_(ops->readRegister(d->REG_LR), mask);
        BaseSemantics::SValuePtr iar = ops->readRegister(d->REG_IAR);
        ops->writeRegister(d->REG_IAR, ops->ite(ops->and_(ctr_ok, cond_ok), target, iar));
    }
};

// Fixed-point compare
struct IP_cmp: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 4);
        // For 32-bit case we can ignore value of L
        BaseSemantics::SValuePtr ra = d->read(args[2], 32);
        BaseSemantics::SValuePtr rb = d->read(args[3], 32);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        // Need to check if ops->boolean_(false) or ops->boolean_(true) should be used!
        // Bias both sides and use unsigned compare.
        // ops->invert(ops->xor_(RA,number<32>(0x80000000U))) yields "(RA+bias)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        BaseSemantics::SValuePtr no = ops->boolean_(false);
        BaseSemantics::SValuePtr v1 = ops->xor_(rb, ops->number_(32, 0x80000000u));
        ops->addWithCarries(ops->invert(ops->xor_(ra, ops->number_(32, 0x80000000u))), v1, no, carries);
        BaseSemantics::SValuePtr two = ops->number_(3, 2);
        BaseSemantics::SValuePtr four = ops->number_(3, 4);
        BaseSemantics::SValuePtr v2 = ops->ite(ops->extract(carries, 31, 32), four, two);
        BaseSemantics::SValuePtr one = ops->number_(3, 1);
        BaseSemantics::SValuePtr c = ops->ite(ops->equalToZero(ops->xor_(ra, rb)), one, v2);
        SgAsmRegisterReferenceExpression* bf = isSgAsmRegisterReferenceExpression(args[0]);
        ASSERT_require(bf && bf->get_descriptor().get_major() == powerpc_regclass_cr && bf->get_descriptor().get_nbits() == 4);
        // This should be a helper function!
        BaseSemantics::SValuePtr  so = ops->extract(ops->readRegister(d->REG_XER), 31, 32);
        ops->writeRegister(bf->get_descriptor(), ops->concat(so, c));
    }
};

// Fixed-point compare immediate
struct IP_cmpi: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 4);
        // For 32-bit case we can ignore value of L
        BaseSemantics::SValuePtr ra = d->read(args[2], 32);
        BaseSemantics::SValuePtr si = ops->signExtend(ops->extract(d->read(args[3], 32), 0, 16), 32);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        // Need to check if ops->boolean_(false) or ops->boolean_(true) should be used!
        // Bias both sides and use unsigned compare.
        // ops->invert(ops->xor_(RA,number<32>(0x80000000U))) yields "(RA+bias)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        BaseSemantics::SValuePtr no = ops->boolean_(false);
        BaseSemantics::SValuePtr v1 = ops->xor_(si, ops->number_(32, 0x80000000u));
        ops->addWithCarries(ops->invert(ops->xor_(ra, ops->number_(32, 0x80000000u))), v1, no, carries);
        BaseSemantics::SValuePtr two = ops->number_(3, 2);
        BaseSemantics::SValuePtr four = ops->number_(3, 4);
        BaseSemantics::SValuePtr v2 = ops->ite(ops->extract(carries, 31, 32), four, two);
        BaseSemantics::SValuePtr one = ops->number_(3, 1);
        BaseSemantics::SValuePtr c = ops->ite(ops->equalToZero(ops->xor_(ra, si)), one, v2);
        SgAsmRegisterReferenceExpression *bf = isSgAsmRegisterReferenceExpression(args[0]);
        ASSERT_require(bf && bf->get_descriptor().get_major() == powerpc_regclass_cr && bf->get_descriptor().get_nbits() == 4);
        // This should be a helper function!
        BaseSemantics::SValuePtr so = ops->extract(ops->readRegister(d->REG_XER), 31, 32);
        ops->writeRegister(bf->get_descriptor(), ops->concat(so, c));
    }
};

// Fixed-point compare logical immediate
struct IP_cmpl: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 4);
        // For 32-bit case we can ignore value of L
        BaseSemantics::SValuePtr ra = d->read(args[2], 32);
        BaseSemantics::SValuePtr ui = d->read(args[3], 32);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        // Need to check if ops->boolean_(false) or ops->boolean_(true) should be used!
        // ops->invert(RA) yields "(-RA)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        BaseSemantics::SValuePtr no = ops->boolean_(false);
        ops->addWithCarries(ops->invert(ra), ui, no, carries);
        BaseSemantics::SValuePtr two = ops->number_(3, 2);
        BaseSemantics::SValuePtr four = ops->number_(3, 4);
        BaseSemantics::SValuePtr v1 = ops->ite(ops->extract(carries, 31, 32), four, two);
        BaseSemantics::SValuePtr one = ops->number_(3, 1);
        BaseSemantics::SValuePtr c = ops->ite(ops->equalToZero(ops->xor_(ra, ui)), one, v1);
        SgAsmRegisterReferenceExpression* bf = isSgAsmRegisterReferenceExpression(args[0]);
        ASSERT_require(bf && bf->get_descriptor().get_major() == powerpc_regclass_cr && bf->get_descriptor().get_nbits()==4);
        // This should be a helper function!
        BaseSemantics::SValuePtr so = ops->extract(ops->readRegister(d->REG_XER), 31, 32);
        ops->writeRegister(bf->get_descriptor(), ops->concat(so, c));
    }
};

// Count leading zeros word
struct IP_cntlzw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr rs = d->read(args[1], 32);
        // Using xor to do the subtract from 31
        BaseSemantics::SValuePtr thirtyOne = ops->number_(32, 31);
        BaseSemantics::SValuePtr v1 = ops->xor_(ops->mostSignificantSetBit(rs), thirtyOne);
        BaseSemantics::SValuePtr thirtyTwo = ops->number_(32, 32);
        BaseSemantics::SValuePtr result = ops->ite(ops->equalToZero(rs), thirtyTwo, v1);
        d->write(args[0], result);
    }
};

// Fixed-point divide word
struct IP_divw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        d->write(args[0], ops->signedDivide(d->read(args[1], 32), arg2));
    }
};

// Fixed-point divide word unsigned
struct IP_divwu: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        d->write(args[0], ops->unsignedDivide(d->read(args[1], 32), arg2));
    }
};

// Load byte and zero extend
struct IP_lbz: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr zero = ops->number_(24, 0);
        d->write(args[0], ops->concat(d->read(args[1], 8), zero));
    }
};

// Load byte and zero extend with update
struct IP_lbzu: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        SgAsmMemoryReferenceExpression *memoryReference = isSgAsmMemoryReferenceExpression(args[1]);
        SgAsmBinaryAdd *binaryAdd = isSgAsmBinaryAdd(memoryReference->get_address());
        ASSERT_not_null(binaryAdd);
        SgAsmExpression *ra = binaryAdd->get_lhs();
        BaseSemantics::SValuePtr addr = d->effectiveAddress(args[1], 32);
        BaseSemantics::SValuePtr zero = ops->number_(24, 0);
        d->write(args[0], ops->concat(d->read(args[1], 8), zero));
        d->write(ra, addr);
    }
};

// Load half-word algebraic
struct IP_lha: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        d->write(args[0], ops->signExtend(d->read(args[1], 16), 32));
    }
};

// Load half-word and zero extend
struct IP_lhz: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr zero = ops->number_(16, 0);
        d->write(args[0], ops->concat(d->read(args[1], 16), zero));
    }
};

// Load multiple word
struct IP_lmw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr base = d->effectiveAddress(args[1], 32);
        SgAsmRegisterReferenceExpression *rt = isSgAsmRegisterReferenceExpression(args[0]);
        ASSERT_require(rt && rt->get_descriptor().get_major() == powerpc_regclass_gpr);
        RegisterDescriptor reg = rt->get_descriptor();
        rose_addr_t offset = 0;
        for (unsigned minor=reg.get_minor(); minor<32; minor+=1, offset+=4) {
            BaseSemantics::SValuePtr addr = ops->add(base, ops->number_(32, offset));
            BaseSemantics::SValuePtr dflt = ops->undefined_(32);
            BaseSemantics::SValuePtr value = ops->readMemory(RegisterDescriptor(), addr, dflt, ops->boolean_(true));
            reg.set_minor(minor);
            ops->writeRegister(reg, value);
        }
    }
};

// Load word and zero with update
struct IP_lwzu: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        // FIXME: shouldn't DispatcherPowerpc::read() handle this? [Robb P. Matzke 2013-05-08]
        SgAsmMemoryReferenceExpression *memoryReference = isSgAsmMemoryReferenceExpression(args[1]);
        SgAsmBinaryAdd *binaryAdd = isSgAsmBinaryAdd(memoryReference->get_address());
        ASSERT_not_null(binaryAdd);
        SgAsmExpression *ra = binaryAdd->get_lhs();
        BaseSemantics::SValuePtr addr = d->effectiveAddress(args[1], 32);
        BaseSemantics::SValuePtr yes = ops->boolean_(true);
        d->write(args[0], ops->readMemory(RegisterDescriptor(), addr, ops->undefined_(32), yes));
        d->write(ra, addr);
    }
};

// Move from condition register
struct IP_mfcr: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 1);
        d->write(args[0], ops->readRegister(d->REG_CR));
    }
};

// Copies the value from the second argument to the first argument.  This is used for a variety of instructions.
struct IP_move: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        d->write(args[0], d->read(args[1], 32));
    }
};

// Multiply high word
struct IP_mulhw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        d->write(args[0], ops->extract(ops->signedMultiply(d->read(args[1], 32), arg2), 32, 64));
    }
};

// Multiply high word unsigned
struct IP_mulhwu: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        d->write(args[0], ops->extract(ops->unsignedMultiply(d->read(args[1], 32), arg2), 32, 64));
    }
};

// Multiply low word
struct IP_mullw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        d->write(args[0], ops->extract(ops->signedMultiply(d->read(args[1], 32), arg2), 0, 32));
    }
};

// Fixed-point negation
struct IP_neg: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        d->write(args[0], ops->negate(d->read(args[1], 32)));
    }
};

// Fixed-point logical NOR
struct IP_nor: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        d->write(args[0], ops->invert(ops->or_(d->read(args[1], 32), arg2)));
    }
};

// Fixed-point logical OR
struct IP_or: P {
    bool record;
    IP_or(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->or_(d->read(args[1], 32), arg2);
        d->write(args[0], result);
        if (record)
            d->record(result);
    }
};

// Fixed-point logical OR with complement
struct IP_orc: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr v1 = ops->invert(d->read(args[2], 32));
        d->write(args[0], ops->or_(d->read(args[1], 32), v1));
    }
};

// Fixed-point logical OR shifted
struct IP_oris: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr v1 = ops->extract(d->read(args[2], 32), 0, 16);
        BaseSemantics::SValuePtr v2 = ops->concat(ops->number_(16, 0), v1);
        d->write(args[0], ops->or_(d->read(args[1], 32), v2));
    }
};

// Rotate left double word immediate then clear right
struct IP_rldicr: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 4);
        BaseSemantics::SValuePtr rs = d->read(args[1], 32);
        BaseSemantics::SValuePtr ra = d->read(args[0], 32);
        BaseSemantics::SValuePtr sh = ops->extract(d->read(args[2], 32), 0, 5);
	/*
        SgAsmIntegerValueExpression *mb = isSgAsmIntegerValueExpression(args[3]);
        ASSERT_not_null(mb);
        int mb_value = mb->get_value();
        uint32_t mask = build_mask(mb_value, me_value);
	*/
        BaseSemantics::SValuePtr rotatedReg = ops->rotateLeft(rs, sh);
        //BaseSemantics::SValuePtr bitMask = ops->number_(32, mask);
        //BaseSemantics::SValuePtr v1 = ops->and_(ra, ops->invert(bitMask));
        //BaseSemantics::SValuePtr result = ops->or_(ops->and_(rotatedReg, bitMask), v1);
        //d->write(args[0], result);
	d->write(args[0], rotatedReg);
    }
};


// Rotate left word immediate then mask insert
struct IP_rlwimi: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 5);
        BaseSemantics::SValuePtr rs = d->read(args[1], 32);
        BaseSemantics::SValuePtr ra = d->read(args[0], 32);
        BaseSemantics::SValuePtr sh = ops->extract(d->read(args[2], 32), 0, 5);
        SgAsmIntegerValueExpression *mb = isSgAsmIntegerValueExpression(args[3]);
        ASSERT_not_null(mb);
        int mb_value = mb->get_value();
        SgAsmIntegerValueExpression *me = isSgAsmIntegerValueExpression(args[4]);
        ASSERT_not_null(me);
        int me_value = me->get_value();
        uint32_t mask = build_mask(mb_value, me_value);
        BaseSemantics::SValuePtr rotatedReg = ops->rotateLeft(rs, sh);
        BaseSemantics::SValuePtr bitMask = ops->number_(32, mask);
        BaseSemantics::SValuePtr v1 = ops->and_(ra, ops->invert(bitMask));
        BaseSemantics::SValuePtr result = ops->or_(ops->and_(rotatedReg, bitMask), v1);
        d->write(args[0], result);
    }
};

// Rotate left word immediate then AND with mask
struct IP_rlwinm: P {
    bool record;
    IP_rlwinm(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 5);
        BaseSemantics::SValuePtr rs = d->read(args[1], 32);
        BaseSemantics::SValuePtr sh = ops->extract(d->read(args[2], 32), 0, 5);
        SgAsmIntegerValueExpression *mb = isSgAsmIntegerValueExpression(args[3]);
        ASSERT_not_null(mb);
        int mb_value = mb->get_value();
        SgAsmIntegerValueExpression *me = isSgAsmIntegerValueExpression(args[4]);
        ASSERT_not_null(me);
        int me_value = me->get_value();
        uint32_t mask = build_mask(mb_value, me_value);
        BaseSemantics::SValuePtr rotatedReg = ops->rotateLeft(rs, sh);
        BaseSemantics::SValuePtr bitMask = ops->number_(32, mask);
        BaseSemantics::SValuePtr result = ops->and_(rotatedReg, bitMask);
        //d->write(args[0], result);
	d->write(args[0], rotatedReg);
        if (record)
            d->record(result);
    }
};

// System call
struct IP_sc: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 1);
        SgAsmIntegerValueExpression *bv = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(bv);
        ops->interrupt(0, bv->get_value());
    }
};

// Shift left word
struct IP_slw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr shiftCount = ops->extract(d->read(args[2], 32), 0, 6);
        BaseSemantics::SValuePtr v1 = ops->extract(shiftCount, 0, 5);
        BaseSemantics::SValuePtr v2 = ops->shiftLeft(d->read(args[1], 32), v1);
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        d->write(args[0], ops->ite(ops->extract(shiftCount, 5, 6), zero, v2));
    }
};

// Shift right algebraic word immediate
struct IP_srawi: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr rs = d->read(args[1], 32);
        BaseSemantics::SValuePtr sh = ops->extract(d->read(args[2], 32), 0, 5);
        BaseSemantics::SValuePtr negative = ops->extract(rs, 31, 32);
        BaseSemantics::SValuePtr mask = ops->invert(ops->shiftLeft(ops->number_(32, -1), sh));
        BaseSemantics::SValuePtr hasValidBits = ops->invert(ops->equalToZero(ops->and_(rs, mask)));
        BaseSemantics::SValuePtr carry_out = ops->and_(hasValidBits, negative);
        d->write(args[0], ops->shiftRightArithmetic(rs, sh));
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr v2 = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), v2), v1));
    }
};

// Shift right word
struct IP_srw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr shiftCount = ops->extract(d->read(args[2], 32), 0, 6);
        BaseSemantics::SValuePtr v1 = ops->extract(shiftCount, 0, 5);
        BaseSemantics::SValuePtr v2 = ops->shiftRight(d->read(args[1], 32), v1);
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        d->write(args[0], ops->ite(ops->extract(shiftCount, 5, 6), zero, v2));
    }
};

// Store byte
struct IP_stb: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        d->write(args[1], ops->extract(d->read(args[0], 32), 0, 8));
    }
};

// Store halfword
struct IP_sth: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        d->write(args[1], ops->extract(d->read(args[0], 32), 0, 16));
    }
};

// Store multiple word
struct IP_stmw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr base = d->effectiveAddress(args[1], 32);
        SgAsmRegisterReferenceExpression *rs = isSgAsmRegisterReferenceExpression(args[0]);
        ASSERT_require(rs && rs->get_descriptor().get_major() == powerpc_regclass_gpr);
        RegisterDescriptor reg = rs->get_descriptor();
        rose_addr_t offset = 0;
        for (unsigned minor=reg.get_minor(); minor<32; minor+=1, offset+=4) {
            BaseSemantics::SValuePtr addr = ops->add(base, ops->number_(32, offset));
            reg.set_minor(minor);
            BaseSemantics::SValuePtr value = ops->readRegister(reg);
            ops->writeMemory(RegisterDescriptor(), addr, value, ops->boolean_(true));
        }
    }
};

// Store word
struct IP_stw: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        BaseSemantics::SValuePtr result = d->read(args[0], 32);
        d->write(args[1], result);
        if (powerpc_stwcx_record==insn->get_kind())
            d->record(result);
    }
};

// Store word with update
struct IP_stwu: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        // FIXME: shouldn't this be done by DispatcherPowerpc::read()? [Robb P. Matzke 2013-05-08]
        SgAsmMemoryReferenceExpression *memoryReference = isSgAsmMemoryReferenceExpression(args[1]);
        SgAsmBinaryAdd *binaryAdd = isSgAsmBinaryAdd(memoryReference->get_address());
        ASSERT_not_null(binaryAdd);
        SgAsmExpression *ra = binaryAdd->get_lhs();
        BaseSemantics::SValuePtr addr = d->effectiveAddress(args[1], 32);
        BaseSemantics::SValuePtr yes = ops->boolean_(true);
        ops->writeMemory(RegisterDescriptor(), addr, d->read(args[0], 32), yes);
        d->write(ra, addr);
    }
};

// Fixed point subtract
struct IP_subf: P {
    bool record;
    IP_subf(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->add(ops->negate(d->read(args[1], 32)), arg2);
        d->write(args[0], result);
        if (record)
            d->record(result);
    }
};

// Fixed-point subtract from carrying
struct IP_subfc: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr yes = ops->boolean_(true);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->addWithCarries(ops->invert(d->read(args[1], 32)), arg2, yes, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr v2 = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), v2), v1));
    }
};

// Fixed-point subtract from extended
struct IP_subfe: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        // This should be a helper function to read CA (and other flags)
        BaseSemantics::SValuePtr carry_in = ops->extract(ops->readRegister(d->REG_XER), 29, 30);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->addWithCarries(ops->invert(d->read(args[1], 32)), arg2, carry_in, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr v2 = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), v2), v1));
    }
};

// Fixed-point subtract from immediate carrying
struct IP_subfic: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        // To do the subtraction we invert the first operand and add.  To add "1" we set the carry in to true.
        BaseSemantics::SValuePtr yes = ops->boolean_(true);
        BaseSemantics::SValuePtr v1 = ops->signExtend(ops->extract(d->read(args[2], 32), 0, 16), 32);
        BaseSemantics::SValuePtr result = ops->addWithCarries(ops->invert(d->read(args[1], 32)), v1, yes, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr v2 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr v3 = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), v3), v2));
    }
};

// Fixed-point subtract from zero extended
struct IP_subfze: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 2);
        // This should be a helper function to read CA (and other flags)
        BaseSemantics::SValuePtr carry_in = ops->extract(ops->readRegister(d->REG_XER), 29, 30);
        BaseSemantics::SValuePtr carries = ops->number_(32, 0);
        BaseSemantics::SValuePtr zero = ops->number_(32, 0);
        BaseSemantics::SValuePtr result = ops->addWithCarries(ops->invert(d->read(args[1], 32)), zero, carry_in, carries);
        BaseSemantics::SValuePtr carry_out = ops->extract(carries, 31, 32);
        d->write(args[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xdfffffff is the mask for the Carry (CA) flag
        BaseSemantics::SValuePtr v1 = ops->ite(carry_out, ops->number_(32, 0x20000000u), zero);
        BaseSemantics::SValuePtr v2 = ops->number_(32, 0xdfffffffu);
        ops->writeRegister(d->REG_XER, ops->or_(ops->and_(ops->readRegister(d->REG_XER), v2), v1));
    }
};

// Fixed-point logical XOR
struct IP_xor: P {
    bool record;
    IP_xor(bool record): record(record) {}
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr arg2 = d->read(args[2], 32);
        BaseSemantics::SValuePtr result = ops->xor_(d->read(args[1], 32), arg2);
        d->write(args[0], result);
        if (record)
            d->record(result);
    }
};

// Fixed-point logical XOR shifted
struct IP_xoris: P {
    void p(D d, Ops ops, I insn, A args) {
        assert_args(insn, args, 3);
        BaseSemantics::SValuePtr v1 = ops->extract(d->read(args[2], 32), 0, 16);
        BaseSemantics::SValuePtr v2 = ops->concat(ops->number_(16, 0), v1);
        d->write(args[0], ops->xor_(d->read(args[1], 32), v2));
    }
};

} // namespace

/*******************************************************************************************************************************
 *                                      DispatcherPowerpc
 *******************************************************************************************************************************/

void
DispatcherPowerpc::iproc_init()
{
    iproc_set(powerpc_add,              new Powerpc::IP_add);
    iproc_set(powerpc_addc,             new Powerpc::IP_addc);
    iproc_set(powerpc_adde,             new Powerpc::IP_adde);
    iproc_set(powerpc_addi,             new Powerpc::IP_addi);
    iproc_set(powerpc_addic,            new Powerpc::IP_addic(false));
    iproc_set(powerpc_addic_record,     new Powerpc::IP_addic(true));
    iproc_set(powerpc_addis,            new Powerpc::IP_addis);
    iproc_set(powerpc_addme,            new Powerpc::IP_addme);
    iproc_set(powerpc_addze,            new Powerpc::IP_addze);
    iproc_set(powerpc_and,              new Powerpc::IP_and(false));
    iproc_set(powerpc_and_record,       new Powerpc::IP_and(true));
    iproc_set(powerpc_andc,             new Powerpc::IP_andc(false));
    iproc_set(powerpc_andc_record,      new Powerpc::IP_andc(true));
    iproc_set(powerpc_andi_record,      new Powerpc::IP_and(true));
    iproc_set(powerpc_andis_record,     new Powerpc::IP_andis(true));
    iproc_set(powerpc_b,                new Powerpc::IP_b(false));
    iproc_set(powerpc_ba,               new Powerpc::IP_ba(false));
    iproc_set(powerpc_bc,               new Powerpc::IP_bc(false));
    iproc_set(powerpc_bca,              new Powerpc::IP_bca(false));
    iproc_set(powerpc_bcctr,            new Powerpc::IP_bcctr(false));
    iproc_set(powerpc_bcctrl,           new Powerpc::IP_bcctr(true));
    iproc_set(powerpc_bcl,              new Powerpc::IP_bc(true));
    iproc_set(powerpc_bcla,             new Powerpc::IP_bca(true));
    iproc_set(powerpc_bclr,             new Powerpc::IP_bclr(false));
    iproc_set(powerpc_bclrl,            new Powerpc::IP_bclr(true));
    iproc_set(powerpc_bl,               new Powerpc::IP_b(true));
    iproc_set(powerpc_bla,              new Powerpc::IP_ba(true));
    iproc_set(powerpc_cmp,              new Powerpc::IP_cmp);
    iproc_set(powerpc_cmpi,             new Powerpc::IP_cmpi);
    iproc_set(powerpc_cmpl,             new Powerpc::IP_cmpl);
    iproc_set(powerpc_cmpli,            new Powerpc::IP_cmpl);
    iproc_set(powerpc_cntlzw,           new Powerpc::IP_cntlzw);
    iproc_set(powerpc_fmr,              new Powerpc::IP_move);
    iproc_set(powerpc_divw,             new Powerpc::IP_divw);
    iproc_set(powerpc_divwu,            new Powerpc::IP_divwu);
    iproc_set(powerpc_extsw,            new Powerpc::IP_move);
    iproc_set(powerpc_lbz,              new Powerpc::IP_lbz);
    iproc_set(powerpc_lbzu,             new Powerpc::IP_lbzu);
    iproc_set(powerpc_lbzux,            new Powerpc::IP_lbzu);
    iproc_set(powerpc_lbzx,             new Powerpc::IP_lbz);
    iproc_set(powerpc_ld,               new Powerpc::IP_move);
    iproc_set(powerpc_lha,              new Powerpc::IP_lha);
    iproc_set(powerpc_lhax,             new Powerpc::IP_lha);
    iproc_set(powerpc_lhz,              new Powerpc::IP_lhz);
    iproc_set(powerpc_lhzx,             new Powerpc::IP_lhz);
    iproc_set(powerpc_lmw,              new Powerpc::IP_lmw);
    iproc_set(powerpc_lwarx,            new Powerpc::IP_move);
    iproc_set(powerpc_lwax,             new Powerpc::IP_move);
    iproc_set(powerpc_lwz,              new Powerpc::IP_move);
    iproc_set(powerpc_lwzu,             new Powerpc::IP_lwzu);
    iproc_set(powerpc_lwzx,             new Powerpc::IP_move);
    iproc_set(powerpc_mfcr,             new Powerpc::IP_mfcr);
    iproc_set(powerpc_mfspr,            new Powerpc::IP_move);
    iproc_set(powerpc_mtspr,            new Powerpc::IP_move);
    iproc_set(powerpc_mulhw,            new Powerpc::IP_mulhw);
    iproc_set(powerpc_mulhwu,           new Powerpc::IP_mulhwu);
    iproc_set(powerpc_mullw,            new Powerpc::IP_mullw);
    iproc_set(powerpc_mulli,            new Powerpc::IP_mullw);
    iproc_set(powerpc_neg,              new Powerpc::IP_neg);
    iproc_set(powerpc_nor,              new Powerpc::IP_nor);
    iproc_set(powerpc_or,               new Powerpc::IP_or(false));
    iproc_set(powerpc_or_record,        new Powerpc::IP_or(true));
    iproc_set(powerpc_orc,              new Powerpc::IP_orc);
    iproc_set(powerpc_ori,              new Powerpc::IP_or(false));
    iproc_set(powerpc_oris,             new Powerpc::IP_oris);
    iproc_set(powerpc_rldic,            new Powerpc::IP_rldicr);
    iproc_set(powerpc_rldicr,           new Powerpc::IP_rldicr);
    iproc_set(powerpc_rldicl,           new Powerpc::IP_rldicr);
    iproc_set(powerpc_rlwimi,           new Powerpc::IP_rlwimi);
    iproc_set(powerpc_rlwinm,           new Powerpc::IP_rlwinm(false));
    iproc_set(powerpc_rlwinm_record,    new Powerpc::IP_rlwinm(true));
    iproc_set(powerpc_sc,               new Powerpc::IP_sc);
    iproc_set(powerpc_slw,              new Powerpc::IP_slw);
    iproc_set(powerpc_srawi,            new Powerpc::IP_srawi);
    iproc_set(powerpc_srw,              new Powerpc::IP_srw);
    iproc_set(powerpc_stb,              new Powerpc::IP_stb);
    iproc_set(powerpc_stbx,             new Powerpc::IP_stb);
    iproc_set(powerpc_sth,              new Powerpc::IP_sth);
    iproc_set(powerpc_stmw,             new Powerpc::IP_stmw);
    iproc_set(powerpc_stw,              new Powerpc::IP_stw);
    iproc_set(powerpc_stwcx_record,     new Powerpc::IP_stw);
    iproc_set(powerpc_stwu,             new Powerpc::IP_stwu);
    iproc_set(powerpc_stwux,            new Powerpc::IP_stwu);
    iproc_set(powerpc_stwx,             new Powerpc::IP_stw);
    iproc_set(powerpc_subf,             new Powerpc::IP_subf(false));
    iproc_set(powerpc_subf_record,      new Powerpc::IP_subf(true));
    iproc_set(powerpc_subfc,            new Powerpc::IP_subfc);
    iproc_set(powerpc_subfe,            new Powerpc::IP_subfe);
    iproc_set(powerpc_subfic,           new Powerpc::IP_subfic);
    iproc_set(powerpc_subfze,           new Powerpc::IP_subfze);
    iproc_set(powerpc_xor,              new Powerpc::IP_xor(false));
    iproc_set(powerpc_xor_record,       new Powerpc::IP_xor(true));
    iproc_set(powerpc_xori,             new Powerpc::IP_xor(false));
    iproc_set(powerpc_xoris,            new Powerpc::IP_xoris);

}

void
DispatcherPowerpc::regcache_init()
{
    if (regdict) {
        REG_IAR = findRegister("iar", 32);              // instruction address register (instruction pointer)
        REG_LR  = findRegister("lr", 32);               // link register
        REG_XER = findRegister("xer", 32);              // fixed-point exception register
        REG_CR  = findRegister("cr", 32);               // condition register
        REG_CR0 = findRegister("cr0", 4);               // CR Field 0, result of fixed-point instruction; set by record()
        REG_CTR = findRegister("ctr", 32);              // count register
    }
}

void
DispatcherPowerpc::memory_init() {
    if (BaseSemantics::StatePtr state = currentState()) {
        if (BaseSemantics::MemoryStatePtr memory = state->memoryState()) {
            switch (memory->get_byteOrder()) {
                case ByteOrder::ORDER_LSB:
                    break;
                case ByteOrder::ORDER_MSB:
                    //mlog[WARN] <<"x86 memory state is using big-endian byte order\n";
                    break;
                case ByteOrder::ORDER_UNSPECIFIED:
                    memory->set_byteOrder(ByteOrder::ORDER_LSB);
                    break;
            }
        }
    }
}

void
DispatcherPowerpc::set_register_dictionary(const RegisterDictionary *regdict)
{
    BaseSemantics::Dispatcher::set_register_dictionary(regdict);
    regcache_init();
}

RegisterDescriptor
DispatcherPowerpc::instructionPointerRegister() const {
    return REG_IAR;
}

RegisterDescriptor
DispatcherPowerpc::stackPointerRegister() const {
    return findRegister("r1");
}

void
DispatcherPowerpc::record(const BaseSemantics::SValuePtr &result)
{
    BaseSemantics::SValuePtr two = operators->number_(3, 2);
    BaseSemantics::SValuePtr four = operators->number_(3, 4);
    BaseSemantics::SValuePtr v1 = operators->ite(operators->extract(result, 31, 32), four, two);
    BaseSemantics::SValuePtr one = operators->number_(3, 1);
    BaseSemantics::SValuePtr c = operators->ite(operators->equalToZero(result), one, v1);
    BaseSemantics::SValuePtr so = operators->extract(operators->readRegister(REG_XER), 31, 32);
    // Put "SO" into the lower bits, and "c" into the higher order bits
    operators->writeRegister(REG_CR0, operators->concat(so, c));
}

} // namespace
} // namespace
} // namespace
