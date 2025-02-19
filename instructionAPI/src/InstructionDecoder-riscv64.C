#include "InstructionDecoder-Capstone.h"

using std::make_pair;

namespace Dyninst {

namespace InstructionAPI {

void InstructionDecoder_Capstone::decodeOperands_riscv64(const Instruction* insn_to_complete, cs_detail* d) {
    bool isCFT = false;         // branch instructions
    //bool isJumpReg = false;     // jump to an address in a register
    //bool isJumpOffset = false;  // relative jump from program counter
    bool isCall = false;        // call instructions
    bool isIndirect = false;    // indirect jump
    bool isConditional = false; // conditional / unconditional branch

    bool err = false;
    entryID eid = insn_to_complete->getOperation().getID();
    cs_riscv* detail = &(d->riscv);

    MachRegister (InstructionDecoder_Capstone::*regTrans)(uint32_t);
    regTrans = &InstructionDecoder_Capstone::registerTranslation_riscv64;

    // This is the index of the operand register to branch to
    //int jumpOpIndex = -1;
    // This is the index of the link register
    //int linkRegIndex = -1;

    if (eid == riscv64_op_jal) {

        assert(detail->op_count == 1 || detail->op_count == 2);

        unsigned int rd = 0;
        int64_t imm = 0;
        // If rd == x1, Capstone will STRIP THE WHOLE RD ARGUMENT AWAY
        // and leave only the offset there
        // Which means that if rd == x1, detail->op_count becomes 1
        if (detail->op_count == 1) {
            rd = RISCV_REG_X1;
            isCall = true; // jal is a call instruction only if rd == x1 (for now)
                           // TODO: The obnoxious alternative link register
            assert(detail->operands[0].type == RISCV_OP_IMM);
            imm = detail->operands[0].imm;
        } else {
            assert(detail->operands[0].type == RISCV_OP_REG);
            rd = detail->operands[0].reg;
            assert(detail->operands[1].type == RISCV_OP_IMM);
            imm = detail->operands[1].imm;
        }

        isCFT = true;
        isIndirect = false;
        isConditional = false;

        Expression::Ptr rdAST = makeRegisterExpression((this->*regTrans)(rd));
        // rd is not read, written, not implicit
        insn_to_complete->appendOperand(rdAST, false, true, false);

        Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, imm));
        Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
        Expression::Ptr target(makeAddExpression(IP, immAST, s32));
        insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
    }
    else if (eid == riscv64_op_jalr) {

        assert(detail->op_count == 3);

        unsigned int rd = 0;
        unsigned int rs = 0;
        int64_t imm = 0;

        assert(detail->operands[0].type == RISCV_OP_REG);
        rd = detail->operands[0].reg;
        assert(detail->operands[1].type == RISCV_OP_REG);
        rs = detail->operands[1].reg;
        assert(detail->operands[2].type == RISCV_OP_IMM);
        imm = detail->operands[2].imm;
        
        isCFT = true;
        isCall = (rd == RISCV_REG_X1); // jal is a call instruction only if rd == x1 (for now)
                                       // TODO: The obnoxious alternative link register
        isIndirect = true;
        isConditional = false;

        Expression::Ptr rdAST = makeRegisterExpression((this->*regTrans)(rd));
        // rd is not read, written, not implicit
        insn_to_complete->appendOperand(rdAST, false, true, false);

        Expression::Ptr rsAST = makeRegisterExpression((this->*regTrans)(rs));
        Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, imm));
        Expression::Ptr target(makeAddExpression(rsAST, immAST, s32));
        insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
    }
    else if (eid == riscv64_op_c_j) {

        assert(detail->op_count == 1);

        int64_t imm = 0;

        assert(detail->operands[0].type == RISCV_OP_IMM);
        imm = detail->operands[0].imm;
        
        isCFT = true;
        isCall = false;
        isIndirect = false;
        isConditional = false;

        Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, imm));
        Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
        Expression::Ptr target(makeAddExpression(IP, immAST, s32));
        insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
    }
    else if (eid == riscv64_op_c_jr) {

        assert(detail->op_count == 1);

        unsigned int rs = 0;

        assert(detail->operands[0].type == RISCV_OP_REG);
        rs = detail->operands[0].reg;
        
        isCFT = true;
        isCall = false;
        isIndirect = true;
        isConditional = false;

        Expression::Ptr rsAST = makeRegisterExpression((this->*regTrans)(rs));
        insn_to_complete->addSuccessor(rsAST, isCall, isIndirect, isConditional, false);
    }
    else if (eid == riscv64_op_c_jalr) {

        assert(detail->op_count == 1);

        unsigned int rs = 0;

        assert(detail->operands[0].type == RISCV_OP_REG);
        rs = detail->operands[0].reg;
        
        isCFT = true;
        isCall = true;
        isIndirect = true;
        isConditional = false;

        Expression::Ptr rsAST = makeRegisterExpression((this->*regTrans)(rs));
        insn_to_complete->addSuccessor(rsAST, isCall, isIndirect, isConditional, false);
    }
    else if (eid == riscv64_op_c_jal) {

        assert(detail->op_count == 1);

        int64_t imm = 0;

        assert(detail->operands[0].type == RISCV_OP_IMM);
        imm = detail->operands[0].imm;
        
        isCFT = true;
        isCall = true;
        isIndirect = false;
        isConditional = false;

        Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, imm));
        Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
        Expression::Ptr target(makeAddExpression(IP, immAST, s32));
        insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
    }
    else if (eid == riscv64_op_beq || eid == riscv64_op_bne || eid == riscv64_op_blt ||
             eid == riscv64_op_bge || eid == riscv64_op_bltu || eid == riscv64_op_bgeu) {

        assert(detail->op_count == 3);

        unsigned int rs1 = 0;
        unsigned int rs2 = 0;
        int64_t imm = 0;

        assert(detail->operands[0].type == RISCV_OP_REG);
        rs1 = detail->operands[0].reg;
        assert(detail->operands[1].type == RISCV_OP_REG);
        rs2 = detail->operands[1].reg;
        assert(detail->operands[2].type == RISCV_OP_IMM);
        imm = detail->operands[2].imm;
        
        isCFT = true;
        isCall = false;
        isIndirect = false;
        isConditional = true;

        Expression::Ptr rs1AST = makeRegisterExpression((this->*regTrans)(rs1));
        Expression::Ptr rs2AST = makeRegisterExpression((this->*regTrans)(rs2));
        // rd is read, not written, not implicit
        insn_to_complete->appendOperand(rs1AST, true, false, false);
        insn_to_complete->appendOperand(rs2AST, true, false, false);

        Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, imm));
        Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
        Expression::Ptr target(makeAddExpression(IP, immAST, s32));
        insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
        insn_to_complete->addSuccessor(IP, isCall, isIndirect, isConditional, true);
    }
    else if (eid == riscv64_op_c_beqz || eid == riscv64_op_c_bnez) {

        assert(detail->op_count == 2);

        unsigned int rs = 0;
        int64_t imm = 0;

        assert(detail->operands[0].type == RISCV_OP_REG);
        rs = detail->operands[0].reg;
        assert(detail->operands[1].type == RISCV_OP_IMM);
        imm = detail->operands[1].imm;
        
        isCFT = true;
        isCall = false;
        isIndirect = false;
        isConditional = true;

        Expression::Ptr rsAST = makeRegisterExpression((this->*regTrans)(rs));
        // rd is read, not written, not implicit
        insn_to_complete->appendOperand(rsAST, true, false, false);

        Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, imm));
        Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
        Expression::Ptr target(makeAddExpression(IP, immAST, s32));
        insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
        insn_to_complete->addSuccessor(IP, isCall, isIndirect, isConditional, true);
    }
    else {
        for (uint8_t i = 0; i < detail->op_count; ++i) {
            cs_riscv_op* operand = &(detail->operands[i]);
            // jal & jalr: Determine whether the link register is ra (x1)
            //if ((eid == riscv64_op_jal || eid == riscv64_op_jalr) && i == linkRegIndex) {
                //assert(operand->type == RISCV_OP_REG);
                //isCall = (operand->reg == 1);
            //}
            if (operand->type == RISCV_OP_REG) {
                Expression::Ptr regAST = makeRegisterExpression((this->*regTrans)(operand->reg));
                //if (isCFT && isJumpReg && jumpOpIndex == i) {
                    //// Special case: jalr rd, rs, offset.
                    //if (isJumpOffset) {
                        //// It jumps to [rs] + offset, so we need to know the offset in advance
                        //// to construct the jump target
                        //cs_riscv_op* nextOperand = &(detail->operands[i + 1]);
                        //assert(nextOperand->type == RISCV_OP_IMM);
                        //Expression::Ptr immAST = Immediate::makeImmediate(Result(u32, nextOperand->imm));
                        //Expression::Ptr target(makeAddExpression(regAST, immAST, s32));
                        //insn_to_complete->addSuccessor(target, isCall, isIndirect, false, false);
                        //// The next operand is already handled. skip it
                        //++i;
                    //}
                    //// c.j
                    //else {
                        //// If a call or a jump has a register as an operand,
                        //// It should not be a conditional jump
                        //insn_to_complete->addSuccessor(regAST, isCall, isIndirect, false, false);
                    //}
                //} else {
                    bool isRead = ((operand->access & CS_AC_READ) != 0);
                    bool isWritten = ((operand->access & CS_AC_WRITE) != 0);
                    if (!isRead && !isWritten) {
                        isRead = isWritten = true;
                    }
                    insn_to_complete->appendOperand(regAST, isRead, isWritten, false);
                //}
            } else if (operand->type == RISCV_OP_IMM) {
                Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, operand->imm));
                //if (isCFT && isJumpOffset && jumpOpIndex == i) {
                    //Expression::Ptr IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
                    //Expression::Ptr target(makeAddExpression(IP, immAST, s32));
                    //insn_to_complete->addSuccessor(target, isCall, isIndirect, isConditional, false);
                    //if (isConditional) {
                        //insn_to_complete->addSuccessor(IP, isCall, isIndirect, true, true);
                    //}
                //} else {
                    insn_to_complete->appendOperand(immAST, false, false, false);
                //}
            } else if (operand->type == RISCV_OP_MEM) {
                riscv_op_mem* mem = &(operand->mem);
                Expression::Ptr effectiveAddr = makeRegisterExpression((this->*regTrans)(mem->base));
                Result_Type type = invalid_type;
                bool isLoad = false, isStore = false;
                // Memory access type depends on the instruction
                // TODO Add RISC-V access size support directly to Capstone
                switch (eid) {
                    case riscv64_op_lb:
                    case riscv64_op_lbu:
                        type = u8;
                        isLoad = true;
                        break;
                    case riscv64_op_sb:
                        type = u8;
                        isStore = true;
                        break;
                    case riscv64_op_lh:
                    case riscv64_op_lhu:
                        isLoad = true;
                        type = u16;
                        break;
                    case riscv64_op_sh:
                        isStore = true;
                        type = u16;
                        break;
                    case riscv64_op_c_flw:
                    case riscv64_op_c_flwsp:
                    case riscv64_op_c_lw:
                    case riscv64_op_c_lwsp:
                    case riscv64_op_flw:
                    case riscv64_op_lw:
                    case riscv64_op_lr_w:
                    case riscv64_op_lwu:
                        isLoad = true;
                        type = u32;
                        break;
                    case riscv64_op_c_fsw:
                    case riscv64_op_c_fswsp:
                    case riscv64_op_c_sw:
                    case riscv64_op_c_swsp:
                    case riscv64_op_fsw:
                    case riscv64_op_sw:
                    case riscv64_op_sc_w:
                    case riscv64_op_sc_w_aq:
                    case riscv64_op_sc_w_aq_rl:
                    case riscv64_op_sc_w_rl:
                        isStore = true;
                        type = u32;
                        break;
                    case riscv64_op_c_fldsp:
                    case riscv64_op_c_ldsp:
                    case riscv64_op_c_ld:
                    case riscv64_op_fld:
                    case riscv64_op_ld:
                    case riscv64_op_lr_d:
                        isLoad = true;
                        type = u64;
                        break;
                    case riscv64_op_c_fsdsp:
                    case riscv64_op_c_sd:
                    case riscv64_op_c_sdsp:
                    case riscv64_op_fsd:
                    case riscv64_op_sd:
                    case riscv64_op_sc_d:
                    case riscv64_op_sc_d_aq:
                    case riscv64_op_sc_d_aq_rl:
                    case riscv64_op_sc_d_rl:
                        isStore = true;
                        type = u64;
                        break;
                    case riscv64_op_amoswap_w:
                    case riscv64_op_amoswap_w_aq:
                    case riscv64_op_amoswap_w_aq_rl:
                    case riscv64_op_amoswap_w_rl:
                    case riscv64_op_amoadd_w:
                    case riscv64_op_amoadd_w_aq:
                    case riscv64_op_amoadd_w_aq_rl:
                    case riscv64_op_amoadd_w_rl:
                    case riscv64_op_amoxor_w:
                    case riscv64_op_amoxor_w_aq:
                    case riscv64_op_amoxor_w_aq_rl:
                    case riscv64_op_amoxor_w_rl:
                    case riscv64_op_amoand_w:
                    case riscv64_op_amoand_w_aq:
                    case riscv64_op_amoand_w_aq_rl:
                    case riscv64_op_amoand_w_rl:
                    case riscv64_op_amoor_w:
                    case riscv64_op_amoor_w_aq:
                    case riscv64_op_amoor_w_aq_rl:
                    case riscv64_op_amoor_w_rl:
                    case riscv64_op_amomin_w:
                    case riscv64_op_amomin_w_aq:
                    case riscv64_op_amomin_w_aq_rl:
                    case riscv64_op_amomin_w_rl:
                    case riscv64_op_amomax_w:
                    case riscv64_op_amomax_w_aq:
                    case riscv64_op_amomax_w_aq_rl:
                    case riscv64_op_amomax_w_rl:
                    case riscv64_op_amominu_w:
                    case riscv64_op_amominu_w_aq:
                    case riscv64_op_amominu_w_aq_rl:
                    case riscv64_op_amominu_w_rl:
                    case riscv64_op_amomaxu_w:
                    case riscv64_op_amomaxu_w_aq:
                    case riscv64_op_amomaxu_w_aq_rl:
                    case riscv64_op_amomaxu_w_rl:
                        isLoad = true;
                        isStore = true;
                        type = u32;
                        break;
                    case riscv64_op_amoswap_d:
                    case riscv64_op_amoswap_d_aq:
                    case riscv64_op_amoswap_d_aq_rl:
                    case riscv64_op_amoswap_d_rl:
                    case riscv64_op_amoadd_d:
                    case riscv64_op_amoadd_d_aq:
                    case riscv64_op_amoadd_d_aq_rl:
                    case riscv64_op_amoadd_d_rl:
                    case riscv64_op_amoxor_d:
                    case riscv64_op_amoxor_d_aq:
                    case riscv64_op_amoxor_d_aq_rl:
                    case riscv64_op_amoxor_d_rl:
                    case riscv64_op_amoand_d:
                    case riscv64_op_amoand_d_aq:
                    case riscv64_op_amoand_d_aq_rl:
                    case riscv64_op_amoand_d_rl:
                    case riscv64_op_amoor_d:
                    case riscv64_op_amoor_d_aq:
                    case riscv64_op_amoor_d_aq_rl:
                    case riscv64_op_amoor_d_rl:
                    case riscv64_op_amomin_d:
                    case riscv64_op_amomin_d_aq:
                    case riscv64_op_amomin_d_aq_rl:
                    case riscv64_op_amomin_d_rl:
                    case riscv64_op_amomax_d:
                    case riscv64_op_amomax_d_aq:
                    case riscv64_op_amomax_d_aq_rl:
                    case riscv64_op_amomax_d_rl:
                    case riscv64_op_amominu_d:
                    case riscv64_op_amominu_d_aq:
                    case riscv64_op_amominu_d_aq_rl:
                    case riscv64_op_amominu_d_rl:
                    case riscv64_op_amomaxu_d:
                    case riscv64_op_amomaxu_d_aq:
                    case riscv64_op_amomaxu_d_aq_rl:
                    case riscv64_op_amomaxu_d_rl:
                        isLoad = true;
                        isStore = true;
                        type = u64;
                        break;
                    default:
                        break;
                }
                // Offsets are 12 bits long signed integers
                Expression::Ptr immAST = Immediate::makeImmediate(Result(s32, mem->disp));
                effectiveAddr = makeAddExpression(effectiveAddr, immAST, s32);
                if (type == invalid_type) {
                    err = true;
                }
                Expression::Ptr memAST = makeDereferenceExpression(effectiveAddr, type);
                bool isRead = ((operand->access & CS_AC_READ) != 0);
                bool isWritten = ((operand->access & CS_AC_WRITE) != 0);
                if (!isRead && !isWritten) {
                    isRead = isWritten = true;
                }
                insn_to_complete->appendOperand(memAST, isLoad, isStore, false);
            } else {
                fprintf(stderr, "Unhandled capstone operand type %d\n", operand->type);
            }
        }
    }

    // Capstone does not handle implicit registers
    // Handle the program counter explicitly

    if (isCFT || eid == riscv64_op_auipc) {
        int isPcRead = eid != riscv64_op_c_jr;
        int isPcWrite = isCFT;
        MachRegister reg = riscv64::pc;
        Expression::Ptr regAST = makeRegisterExpression(reg);
        insn_to_complete->appendOperand(regAST, isPcRead, isPcWrite, true);
    }

    if (err) fprintf(stderr, "\tinstruction %s\n", insn_to_complete->format().c_str());
}

entryID InstructionDecoder_Capstone::opcodeTranslation_riscv64(unsigned int cap_id) {
    switch (cap_id) {
        case RISCV_INS_ADD:
            return riscv64_op_add;
        case RISCV_INS_ADDI:
            return riscv64_op_addi;
        case RISCV_INS_ADDIW:
            return riscv64_op_addiw;
        case RISCV_INS_ADDW:
            return riscv64_op_addw;
        case RISCV_INS_AMOADD_D:
            return riscv64_op_amoadd_d;
        case RISCV_INS_AMOADD_D_AQ:
            return riscv64_op_amoadd_d_aq;
        case RISCV_INS_AMOADD_D_AQ_RL:
            return riscv64_op_amoadd_d_aq_rl;
        case RISCV_INS_AMOADD_D_RL:
            return riscv64_op_amoadd_d_rl;
        case RISCV_INS_AMOADD_W:
            return riscv64_op_amoadd_w;
        case RISCV_INS_AMOADD_W_AQ:
            return riscv64_op_amoadd_w_aq;
        case RISCV_INS_AMOADD_W_AQ_RL:
            return riscv64_op_amoadd_w_aq_rl;
        case RISCV_INS_AMOADD_W_RL:
            return riscv64_op_amoadd_w_rl;
        case RISCV_INS_AMOAND_D:
            return riscv64_op_amoand_d;
        case RISCV_INS_AMOAND_D_AQ:
            return riscv64_op_amoand_d_aq;
        case RISCV_INS_AMOAND_D_AQ_RL:
            return riscv64_op_amoand_d_aq_rl;
        case RISCV_INS_AMOAND_D_RL:
            return riscv64_op_amoand_d_rl;
        case RISCV_INS_AMOAND_W:
            return riscv64_op_amoand_w;
        case RISCV_INS_AMOAND_W_AQ:
            return riscv64_op_amoand_w_aq;
        case RISCV_INS_AMOAND_W_AQ_RL:
            return riscv64_op_amoand_w_aq_rl;
        case RISCV_INS_AMOAND_W_RL:
            return riscv64_op_amoand_w_rl;
        case RISCV_INS_AMOMAXU_D:
            return riscv64_op_amomaxu_d;
        case RISCV_INS_AMOMAXU_D_AQ:
            return riscv64_op_amomaxu_d_aq;
        case RISCV_INS_AMOMAXU_D_AQ_RL:
            return riscv64_op_amomaxu_d_aq_rl;
        case RISCV_INS_AMOMAXU_D_RL:
            return riscv64_op_amomaxu_d_rl;
        case RISCV_INS_AMOMAXU_W:
            return riscv64_op_amomaxu_w;
        case RISCV_INS_AMOMAXU_W_AQ:
            return riscv64_op_amomaxu_w_aq;
        case RISCV_INS_AMOMAXU_W_AQ_RL:
            return riscv64_op_amomaxu_w_aq_rl;
        case RISCV_INS_AMOMAXU_W_RL:
            return riscv64_op_amomaxu_w_rl;
        case RISCV_INS_AMOMAX_D:
            return riscv64_op_amomax_d;
        case RISCV_INS_AMOMAX_D_AQ:
            return riscv64_op_amomax_d_aq;
        case RISCV_INS_AMOMAX_D_AQ_RL:
            return riscv64_op_amomax_d_aq_rl;
        case RISCV_INS_AMOMAX_D_RL:
            return riscv64_op_amomax_d_rl;
        case RISCV_INS_AMOMAX_W:
            return riscv64_op_amomax_w;
        case RISCV_INS_AMOMAX_W_AQ:
            return riscv64_op_amomax_w_aq;
        case RISCV_INS_AMOMAX_W_AQ_RL:
            return riscv64_op_amomax_w_aq_rl;
        case RISCV_INS_AMOMAX_W_RL:
            return riscv64_op_amomax_w_rl;
        case RISCV_INS_AMOMINU_D:
            return riscv64_op_amominu_d;
        case RISCV_INS_AMOMINU_D_AQ:
            return riscv64_op_amominu_d_aq;
        case RISCV_INS_AMOMINU_D_AQ_RL:
            return riscv64_op_amominu_d_aq_rl;
        case RISCV_INS_AMOMINU_D_RL:
            return riscv64_op_amominu_d_rl;
        case RISCV_INS_AMOMINU_W:
            return riscv64_op_amominu_w;
        case RISCV_INS_AMOMINU_W_AQ:
            return riscv64_op_amominu_w_aq;
        case RISCV_INS_AMOMINU_W_AQ_RL:
            return riscv64_op_amominu_w_aq_rl;
        case RISCV_INS_AMOMINU_W_RL:
            return riscv64_op_amominu_w_rl;
        case RISCV_INS_AMOMIN_D:
            return riscv64_op_amomin_d;
        case RISCV_INS_AMOMIN_D_AQ:
            return riscv64_op_amomin_d_aq;
        case RISCV_INS_AMOMIN_D_AQ_RL:
            return riscv64_op_amomin_d_aq_rl;
        case RISCV_INS_AMOMIN_D_RL:
            return riscv64_op_amomin_d_rl;
        case RISCV_INS_AMOMIN_W:
            return riscv64_op_amomin_w;
        case RISCV_INS_AMOMIN_W_AQ:
            return riscv64_op_amomin_w_aq;
        case RISCV_INS_AMOMIN_W_AQ_RL:
            return riscv64_op_amomin_w_aq_rl;
        case RISCV_INS_AMOMIN_W_RL:
            return riscv64_op_amomin_w_rl;
        case RISCV_INS_AMOOR_D:
            return riscv64_op_amoor_d;
        case RISCV_INS_AMOOR_D_AQ:
            return riscv64_op_amoor_d_aq;
        case RISCV_INS_AMOOR_D_AQ_RL:
            return riscv64_op_amoor_d_aq_rl;
        case RISCV_INS_AMOOR_D_RL:
            return riscv64_op_amoor_d_rl;
        case RISCV_INS_AMOOR_W:
            return riscv64_op_amoor_w;
        case RISCV_INS_AMOOR_W_AQ:
            return riscv64_op_amoor_w_aq;
        case RISCV_INS_AMOOR_W_AQ_RL:
            return riscv64_op_amoor_w_aq_rl;
        case RISCV_INS_AMOOR_W_RL:
            return riscv64_op_amoor_w_rl;
        case RISCV_INS_AMOSWAP_D:
            return riscv64_op_amoswap_d;
        case RISCV_INS_AMOSWAP_D_AQ:
            return riscv64_op_amoswap_d_aq;
        case RISCV_INS_AMOSWAP_D_AQ_RL:
            return riscv64_op_amoswap_d_aq_rl;
        case RISCV_INS_AMOSWAP_D_RL:
            return riscv64_op_amoswap_d_rl;
        case RISCV_INS_AMOSWAP_W:
            return riscv64_op_amoswap_w;
        case RISCV_INS_AMOSWAP_W_AQ:
            return riscv64_op_amoswap_w_aq;
        case RISCV_INS_AMOSWAP_W_AQ_RL:
            return riscv64_op_amoswap_w_aq_rl;
        case RISCV_INS_AMOSWAP_W_RL:
            return riscv64_op_amoswap_w_rl;
        case RISCV_INS_AMOXOR_D:
            return riscv64_op_amoxor_d;
        case RISCV_INS_AMOXOR_D_AQ:
            return riscv64_op_amoxor_d_aq;
        case RISCV_INS_AMOXOR_D_AQ_RL:
            return riscv64_op_amoxor_d_aq_rl;
        case RISCV_INS_AMOXOR_D_RL:
            return riscv64_op_amoxor_d_rl;
        case RISCV_INS_AMOXOR_W:
            return riscv64_op_amoxor_w;
        case RISCV_INS_AMOXOR_W_AQ:
            return riscv64_op_amoxor_w_aq;
        case RISCV_INS_AMOXOR_W_AQ_RL:
            return riscv64_op_amoxor_w_aq_rl;
        case RISCV_INS_AMOXOR_W_RL:
            return riscv64_op_amoxor_w_rl;
        case RISCV_INS_AND:
            return riscv64_op_and;
        case RISCV_INS_ANDI:
            return riscv64_op_andi;
        case RISCV_INS_AUIPC:
            return riscv64_op_auipc;
        case RISCV_INS_BEQ:
            return riscv64_op_beq;
        case RISCV_INS_BGE:
            return riscv64_op_bge;
        case RISCV_INS_BGEU:
            return riscv64_op_bgeu;
        case RISCV_INS_BLT:
            return riscv64_op_blt;
        case RISCV_INS_BLTU:
            return riscv64_op_bltu;
        case RISCV_INS_BNE:
            return riscv64_op_bne;
        case RISCV_INS_CSRRC:
            return riscv64_op_csrrc;
        case RISCV_INS_CSRRCI:
            return riscv64_op_csrrci;
        case RISCV_INS_CSRRS:
            return riscv64_op_csrrs;
        case RISCV_INS_CSRRSI:
            return riscv64_op_csrrsi;
        case RISCV_INS_CSRRW:
            return riscv64_op_csrrw;
        case RISCV_INS_CSRRWI:
            return riscv64_op_csrrwi;
        case RISCV_INS_C_ADD:
            return riscv64_op_c_add;
        case RISCV_INS_C_ADDI:
            return riscv64_op_c_addi;
        case RISCV_INS_C_ADDI16SP:
            return riscv64_op_c_addi16sp;
        case RISCV_INS_C_ADDI4SPN:
            return riscv64_op_c_addi4spn;
        case RISCV_INS_C_ADDIW:
            return riscv64_op_c_addiw;
        case RISCV_INS_C_ADDW:
            return riscv64_op_c_addw;
        case RISCV_INS_C_AND:
            return riscv64_op_c_and;
        case RISCV_INS_C_ANDI:
            return riscv64_op_c_andi;
        case RISCV_INS_C_BEQZ:
            return riscv64_op_c_beqz;
        case RISCV_INS_C_BNEZ:
            return riscv64_op_c_bnez;
        case RISCV_INS_C_EBREAK:
            return riscv64_op_c_ebreak;
        case RISCV_INS_C_FLD:
            return riscv64_op_c_fld;
        case RISCV_INS_C_FLDSP:
            return riscv64_op_c_fldsp;
        case RISCV_INS_C_FLW:
            return riscv64_op_c_flw;
        case RISCV_INS_C_FLWSP:
            return riscv64_op_c_flwsp;
        case RISCV_INS_C_FSD:
            return riscv64_op_c_fsd;
        case RISCV_INS_C_FSDSP:
            return riscv64_op_c_fsdsp;
        case RISCV_INS_C_FSW:
            return riscv64_op_c_fsw;
        case RISCV_INS_C_FSWSP:
            return riscv64_op_c_fswsp;
        case RISCV_INS_C_J:
            return riscv64_op_c_j;
        case RISCV_INS_C_JAL:
            return riscv64_op_c_jal;
        case RISCV_INS_C_JALR:
            return riscv64_op_c_jalr;
        case RISCV_INS_C_JR:
            return riscv64_op_c_jr;
        case RISCV_INS_C_LD:
            return riscv64_op_c_ld;
        case RISCV_INS_C_LDSP:
            return riscv64_op_c_ldsp;
        case RISCV_INS_C_LI:
            return riscv64_op_c_li;
        case RISCV_INS_C_LUI:
            return riscv64_op_c_lui;
        case RISCV_INS_C_LW:
            return riscv64_op_c_lw;
        case RISCV_INS_C_LWSP:
            return riscv64_op_c_lwsp;
        case RISCV_INS_C_MV:
            return riscv64_op_c_mv;
        case RISCV_INS_C_NOP:
            return riscv64_op_c_nop;
        case RISCV_INS_C_OR:
            return riscv64_op_c_or;
        case RISCV_INS_C_SD:
            return riscv64_op_c_sd;
        case RISCV_INS_C_SDSP:
            return riscv64_op_c_sdsp;
        case RISCV_INS_C_SLLI:
            return riscv64_op_c_slli;
        case RISCV_INS_C_SRAI:
            return riscv64_op_c_srai;
        case RISCV_INS_C_SRLI:
            return riscv64_op_c_srli;
        case RISCV_INS_C_SUB:
            return riscv64_op_c_sub;
        case RISCV_INS_C_SUBW:
            return riscv64_op_c_subw;
        case RISCV_INS_C_SW:
            return riscv64_op_c_sw;
        case RISCV_INS_C_SWSP:
            return riscv64_op_c_swsp;
        case RISCV_INS_C_UNIMP:
            return riscv64_op_c_unimp;
        case RISCV_INS_C_XOR:
            return riscv64_op_c_xor;
        case RISCV_INS_DIV:
            return riscv64_op_div;
        case RISCV_INS_DIVU:
            return riscv64_op_divu;
        case RISCV_INS_DIVUW:
            return riscv64_op_divuw;
        case RISCV_INS_DIVW:
            return riscv64_op_divw;
        case RISCV_INS_EBREAK:
            return riscv64_op_ebreak;
        case RISCV_INS_ECALL:
            return riscv64_op_ecall;
        case RISCV_INS_FADD_D:
            return riscv64_op_fadd_d;
        case RISCV_INS_FADD_S:
            return riscv64_op_fadd_s;
        case RISCV_INS_FCLASS_D:
            return riscv64_op_fclass_d;
        case RISCV_INS_FCLASS_S:
            return riscv64_op_fclass_s;
        case RISCV_INS_FCVT_D_L:
            return riscv64_op_fcvt_d_l;
        case RISCV_INS_FCVT_D_LU:
            return riscv64_op_fcvt_d_lu;
        case RISCV_INS_FCVT_D_S:
            return riscv64_op_fcvt_d_s;
        case RISCV_INS_FCVT_D_W:
            return riscv64_op_fcvt_d_w;
        case RISCV_INS_FCVT_D_WU:
            return riscv64_op_fcvt_d_wu;
        case RISCV_INS_FCVT_LU_D:
            return riscv64_op_fcvt_lu_d;
        case RISCV_INS_FCVT_LU_S:
            return riscv64_op_fcvt_lu_s;
        case RISCV_INS_FCVT_L_D:
            return riscv64_op_fcvt_l_d;
        case RISCV_INS_FCVT_L_S:
            return riscv64_op_fcvt_l_s;
        case RISCV_INS_FCVT_S_D:
            return riscv64_op_fcvt_s_d;
        case RISCV_INS_FCVT_S_L:
            return riscv64_op_fcvt_s_l;
        case RISCV_INS_FCVT_S_LU:
            return riscv64_op_fcvt_s_lu;
        case RISCV_INS_FCVT_S_W:
            return riscv64_op_fcvt_s_w;
        case RISCV_INS_FCVT_S_WU:
            return riscv64_op_fcvt_s_wu;
        case RISCV_INS_FCVT_WU_D:
            return riscv64_op_fcvt_wu_d;
        case RISCV_INS_FCVT_WU_S:
            return riscv64_op_fcvt_wu_s;
        case RISCV_INS_FCVT_W_D:
            return riscv64_op_fcvt_w_d;
        case RISCV_INS_FCVT_W_S:
            return riscv64_op_fcvt_w_s;
        case RISCV_INS_FDIV_D:
            return riscv64_op_fdiv_d;
        case RISCV_INS_FDIV_S:
            return riscv64_op_fdiv_s;
        case RISCV_INS_FENCE:
            return riscv64_op_fence;
        case RISCV_INS_FENCE_I:
            return riscv64_op_fence_i;
        case RISCV_INS_FENCE_TSO:
            return riscv64_op_fence_tso;
        case RISCV_INS_FEQ_D:
            return riscv64_op_feq_d;
        case RISCV_INS_FEQ_S:
            return riscv64_op_feq_s;
        case RISCV_INS_FLD:
            return riscv64_op_fld;
        case RISCV_INS_FLE_D:
            return riscv64_op_fle_d;
        case RISCV_INS_FLE_S:
            return riscv64_op_fle_s;
        case RISCV_INS_FLT_D:
            return riscv64_op_flt_d;
        case RISCV_INS_FLT_S:
            return riscv64_op_flt_s;
        case RISCV_INS_FLW:
            return riscv64_op_flw;
        case RISCV_INS_FMADD_D:
            return riscv64_op_fmadd_d;
        case RISCV_INS_FMADD_S:
            return riscv64_op_fmadd_s;
        case RISCV_INS_FMAX_D:
            return riscv64_op_fmax_d;
        case RISCV_INS_FMAX_S:
            return riscv64_op_fmax_s;
        case RISCV_INS_FMIN_D:
            return riscv64_op_fmin_d;
        case RISCV_INS_FMIN_S:
            return riscv64_op_fmin_s;
        case RISCV_INS_FMSUB_D:
            return riscv64_op_fmsub_d;
        case RISCV_INS_FMSUB_S:
            return riscv64_op_fmsub_s;
        case RISCV_INS_FMUL_D:
            return riscv64_op_fmul_d;
        case RISCV_INS_FMUL_S:
            return riscv64_op_fmul_s;
        case RISCV_INS_FMV_D_X:
            return riscv64_op_fmv_d_x;
        case RISCV_INS_FMV_W_X:
            return riscv64_op_fmv_w_x;
        case RISCV_INS_FMV_X_D:
            return riscv64_op_fmv_x_d;
        case RISCV_INS_FMV_X_W:
            return riscv64_op_fmv_x_w;
        case RISCV_INS_FNMADD_D:
            return riscv64_op_fnmadd_d;
        case RISCV_INS_FNMADD_S:
            return riscv64_op_fnmadd_s;
        case RISCV_INS_FNMSUB_D:
            return riscv64_op_fnmsub_d;
        case RISCV_INS_FNMSUB_S:
            return riscv64_op_fnmsub_s;
        case RISCV_INS_FSD:
            return riscv64_op_fsd;
        case RISCV_INS_FSGNJN_D:
            return riscv64_op_fsgnjn_d;
        case RISCV_INS_FSGNJN_S:
            return riscv64_op_fsgnjn_s;
        case RISCV_INS_FSGNJX_D:
            return riscv64_op_fsgnjx_d;
        case RISCV_INS_FSGNJX_S:
            return riscv64_op_fsgnjx_s;
        case RISCV_INS_FSGNJ_D:
            return riscv64_op_fsgnj_d;
        case RISCV_INS_FSGNJ_S:
            return riscv64_op_fsgnj_s;
        case RISCV_INS_FSQRT_D:
            return riscv64_op_fsqrt_d;
        case RISCV_INS_FSQRT_S:
            return riscv64_op_fsqrt_s;
        case RISCV_INS_FSUB_D:
            return riscv64_op_fsub_d;
        case RISCV_INS_FSUB_S:
            return riscv64_op_fsub_s;
        case RISCV_INS_FSW:
            return riscv64_op_fsw;
        case RISCV_INS_JAL:
            return riscv64_op_jal;
        case RISCV_INS_JALR:
            return riscv64_op_jalr;
        case RISCV_INS_LB:
            return riscv64_op_lb;
        case RISCV_INS_LBU:
            return riscv64_op_lbu;
        case RISCV_INS_LD:
            return riscv64_op_ld;
        case RISCV_INS_LH:
            return riscv64_op_lh;
        case RISCV_INS_LHU:
            return riscv64_op_lhu;
        case RISCV_INS_LR_D:
            return riscv64_op_lr_d;
        case RISCV_INS_LR_D_AQ:
            return riscv64_op_lr_d_aq;
        case RISCV_INS_LR_D_AQ_RL:
            return riscv64_op_lr_d_aq_rl;
        case RISCV_INS_LR_D_RL:
            return riscv64_op_lr_d_rl;
        case RISCV_INS_LR_W:
            return riscv64_op_lr_w;
        case RISCV_INS_LR_W_AQ:
            return riscv64_op_lr_w_aq;
        case RISCV_INS_LR_W_AQ_RL:
            return riscv64_op_lr_w_aq_rl;
        case RISCV_INS_LR_W_RL:
            return riscv64_op_lr_w_rl;
        case RISCV_INS_LUI:
            return riscv64_op_lui;
        case RISCV_INS_LW:
            return riscv64_op_lw;
        case RISCV_INS_LWU:
            return riscv64_op_lwu;
        case RISCV_INS_MRET:
            return riscv64_op_mret;
        case RISCV_INS_MUL:
            return riscv64_op_mul;
        case RISCV_INS_MULH:
            return riscv64_op_mulh;
        case RISCV_INS_MULHSU:
            return riscv64_op_mulhsu;
        case RISCV_INS_MULHU:
            return riscv64_op_mulhu;
        case RISCV_INS_MULW:
            return riscv64_op_mulw;
        case RISCV_INS_OR:
            return riscv64_op_or;
        case RISCV_INS_ORI:
            return riscv64_op_ori;
        case RISCV_INS_REM:
            return riscv64_op_rem;
        case RISCV_INS_REMU:
            return riscv64_op_remu;
        case RISCV_INS_REMUW:
            return riscv64_op_remuw;
        case RISCV_INS_REMW:
            return riscv64_op_remw;
        case RISCV_INS_SB:
            return riscv64_op_sb;
        case RISCV_INS_SC_D:
            return riscv64_op_sc_d;
        case RISCV_INS_SC_D_AQ:
            return riscv64_op_sc_d_aq;
        case RISCV_INS_SC_D_AQ_RL:
            return riscv64_op_sc_d_aq_rl;
        case RISCV_INS_SC_D_RL:
            return riscv64_op_sc_d_rl;
        case RISCV_INS_SC_W:
            return riscv64_op_sc_w;
        case RISCV_INS_SC_W_AQ:
            return riscv64_op_sc_w_aq;
        case RISCV_INS_SC_W_AQ_RL:
            return riscv64_op_sc_w_aq_rl;
        case RISCV_INS_SC_W_RL:
            return riscv64_op_sc_w_rl;
        case RISCV_INS_SD:
            return riscv64_op_sd;
        case RISCV_INS_SFENCE_VMA:
            return riscv64_op_sfence_vma;
        case RISCV_INS_SH:
            return riscv64_op_sh;
        case RISCV_INS_SLL:
            return riscv64_op_sll;
        case RISCV_INS_SLLI:
            return riscv64_op_slli;
        case RISCV_INS_SLLIW:
            return riscv64_op_slliw;
        case RISCV_INS_SLLW:
            return riscv64_op_sllw;
        case RISCV_INS_SLT:
            return riscv64_op_slt;
        case RISCV_INS_SLTI:
            return riscv64_op_slti;
        case RISCV_INS_SLTIU:
            return riscv64_op_sltiu;
        case RISCV_INS_SLTU:
            return riscv64_op_sltu;
        case RISCV_INS_SRA:
            return riscv64_op_sra;
        case RISCV_INS_SRAI:
            return riscv64_op_srai;
        case RISCV_INS_SRAIW:
            return riscv64_op_sraiw;
        case RISCV_INS_SRAW:
            return riscv64_op_sraw;
        case RISCV_INS_SRET:
            return riscv64_op_sret;
        case RISCV_INS_SRL:
            return riscv64_op_srl;
        case RISCV_INS_SRLI:
            return riscv64_op_srli;
        case RISCV_INS_SRLIW:
            return riscv64_op_srliw;
        case RISCV_INS_SRLW:
            return riscv64_op_srlw;
        case RISCV_INS_SUB:
            return riscv64_op_sub;
        case RISCV_INS_SUBW:
            return riscv64_op_subw;
        case RISCV_INS_SW:
            return riscv64_op_sw;
        case RISCV_INS_UNIMP:
            return riscv64_op_unimp;
        case RISCV_INS_URET:
            return riscv64_op_uret;
        case RISCV_INS_WFI:
            return riscv64_op_wfi;
        case RISCV_INS_XOR:
            return riscv64_op_xor;
        case RISCV_INS_XORI:
            return riscv64_op_xori;
        default:
            return e_No_Entry;
    }
}

MachRegister InstructionDecoder_Capstone::registerTranslation_riscv64(uint32_t cap_reg) {
    switch (cap_reg) {
        case RISCV_REG_X0:
            return riscv64::x0;
        case RISCV_REG_X1:
            return riscv64::x1;
        case RISCV_REG_X2:
            return riscv64::x2;
        case RISCV_REG_X3:
            return riscv64::x3;
        case RISCV_REG_X4:
            return riscv64::x4;
        case RISCV_REG_X5:
            return riscv64::x5;
        case RISCV_REG_X6:
            return riscv64::x6;
        case RISCV_REG_X7:
            return riscv64::x7;
        case RISCV_REG_X8:
            return riscv64::x8;
        case RISCV_REG_X9:
            return riscv64::x9;
        case RISCV_REG_X10:
            return riscv64::x10;
        case RISCV_REG_X11:
            return riscv64::x11;
        case RISCV_REG_X12:
            return riscv64::x12;
        case RISCV_REG_X13:
            return riscv64::x13;
        case RISCV_REG_X14:
            return riscv64::x14;
        case RISCV_REG_X15:
            return riscv64::x15;
        case RISCV_REG_X16:
            return riscv64::x16;
        case RISCV_REG_X17:
            return riscv64::x17;
        case RISCV_REG_X18:
            return riscv64::x18;
        case RISCV_REG_X19:
            return riscv64::x19;
        case RISCV_REG_X20:
            return riscv64::x20;
        case RISCV_REG_X21:
            return riscv64::x21;
        case RISCV_REG_X22:
            return riscv64::x22;
        case RISCV_REG_X23:
            return riscv64::x23;
        case RISCV_REG_X24:
            return riscv64::x24;
        case RISCV_REG_X25:
            return riscv64::x25;
        case RISCV_REG_X26:
            return riscv64::x26;
        case RISCV_REG_X27:
            return riscv64::x27;
        case RISCV_REG_X28:
            return riscv64::x28;
        case RISCV_REG_X29:
            return riscv64::x29;
        case RISCV_REG_X30:
            return riscv64::x30;
        case RISCV_REG_X31:
            return riscv64::x31;
        case RISCV_REG_F0_32:
            return riscv64::f0_32;
        case RISCV_REG_F0_64:
            return riscv64::f0_64;
        case RISCV_REG_F1_32:
            return riscv64::f1_32;
        case RISCV_REG_F1_64:
            return riscv64::f1_64;
        case RISCV_REG_F2_32:
            return riscv64::f2_32;
        case RISCV_REG_F2_64:
            return riscv64::f2_64;
        case RISCV_REG_F3_32:
            return riscv64::f3_32;
        case RISCV_REG_F3_64:
            return riscv64::f3_64;
        case RISCV_REG_F4_32:
            return riscv64::f4_32;
        case RISCV_REG_F4_64:
            return riscv64::f4_64;
        case RISCV_REG_F5_32:
            return riscv64::f5_32;
        case RISCV_REG_F5_64:
            return riscv64::f5_64;
        case RISCV_REG_F6_32:
            return riscv64::f6_32;
        case RISCV_REG_F6_64:
            return riscv64::f6_64;
        case RISCV_REG_F7_32:
            return riscv64::f7_32;
        case RISCV_REG_F7_64:
            return riscv64::f7_64;
        case RISCV_REG_F8_32:
            return riscv64::f8_32;
        case RISCV_REG_F8_64:
            return riscv64::f8_64;
        case RISCV_REG_F9_32:
            return riscv64::f9_32;
        case RISCV_REG_F9_64:
            return riscv64::f9_64;
        case RISCV_REG_F10_32:
            return riscv64::f10_32;
        case RISCV_REG_F10_64:
            return riscv64::f10_64;
        case RISCV_REG_F11_32:
            return riscv64::f11_32;
        case RISCV_REG_F11_64:
            return riscv64::f11_64;
        case RISCV_REG_F12_32:
            return riscv64::f12_32;
        case RISCV_REG_F12_64:
            return riscv64::f12_64;
        case RISCV_REG_F13_32:
            return riscv64::f13_32;
        case RISCV_REG_F13_64:
            return riscv64::f13_64;
        case RISCV_REG_F14_32:
            return riscv64::f14_32;
        case RISCV_REG_F14_64:
            return riscv64::f14_64;
        case RISCV_REG_F15_32:
            return riscv64::f15_32;
        case RISCV_REG_F15_64:
            return riscv64::f15_64;
        case RISCV_REG_F16_32:
            return riscv64::f16_32;
        case RISCV_REG_F16_64:
            return riscv64::f16_64;
        case RISCV_REG_F17_32:
            return riscv64::f17_32;
        case RISCV_REG_F17_64:
            return riscv64::f17_64;
        case RISCV_REG_F18_32:
            return riscv64::f18_32;
        case RISCV_REG_F18_64:
            return riscv64::f18_64;
        case RISCV_REG_F19_32:
            return riscv64::f19_32;
        case RISCV_REG_F19_64:
            return riscv64::f19_64;
        case RISCV_REG_F20_32:
            return riscv64::f20_32;
        case RISCV_REG_F20_64:
            return riscv64::f20_64;
        case RISCV_REG_F21_32:
            return riscv64::f21_32;
        case RISCV_REG_F21_64:
            return riscv64::f21_64;
        case RISCV_REG_F22_32:
            return riscv64::f22_32;
        case RISCV_REG_F22_64:
            return riscv64::f22_64;
        case RISCV_REG_F23_32:
            return riscv64::f23_32;
        case RISCV_REG_F23_64:
            return riscv64::f23_64;
        case RISCV_REG_F24_32:
            return riscv64::f24_32;
        case RISCV_REG_F24_64:
            return riscv64::f24_64;
        case RISCV_REG_F25_32:
            return riscv64::f25_32;
        case RISCV_REG_F25_64:
            return riscv64::f25_64;
        case RISCV_REG_F26_32:
            return riscv64::f26_32;
        case RISCV_REG_F26_64:
            return riscv64::f26_64;
        case RISCV_REG_F27_32:
            return riscv64::f27_32;
        case RISCV_REG_F27_64:
            return riscv64::f27_64;
        case RISCV_REG_F28_32:
            return riscv64::f28_32;
        case RISCV_REG_F28_64:
            return riscv64::f28_64;
        case RISCV_REG_F29_32:
            return riscv64::f29_32;
        case RISCV_REG_F29_64:
            return riscv64::f29_64;
        case RISCV_REG_F30_32:
            return riscv64::f30_32;
        case RISCV_REG_F30_64:
            return riscv64::f30_64;
        case RISCV_REG_F31_32:
            return riscv64::f31_32;
        case RISCV_REG_F31_64:
            return riscv64::f31_64;
        default:
            return InvalidReg;
    }
}
}  // namespace InstructionAPI
}  // namespace Dyninst
