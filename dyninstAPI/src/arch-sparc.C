/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * inst-power.C - Identify instrumentation points for a RS6000/PowerPCs
 * $Id: arch-sparc.C,v 1.1 2005/07/29 19:22:49 bernat Exp $
 */

#include "common/h/Types.h"
#include "arch-sparc.h"
#include "inst-sparc.h"
#include "util.h"
#include "showerror.h"

//inline unsigned getMaxBranch1Insn() {
//   // The length we can branch using just 1 instruction is dictated by the
//   // sparc instruction set.
//   return (0x1 << 23);
//}

/****************************************************************************/
/****************************************************************************/

void instruction::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    insn.generate(gen);
}

void instruction::generateIllegal(codeGen &gen) {
    instruction insn(0);
    insn.generate(gen);
}

bool instruction::offsetWithinRangeOfBranchInsn(int offset) {
    // The pc-relative offset range which we can branch to with a single sparc
    // branch instruction is dictated by the sparc instruction set.
    // There are 22 bits available...however, you really get 2 extra bits
    // because the CPU multiplies the 22-bit signed offset by 4.
    // The only downside is that the offset must be a multiple of 4, which we 
    // check.
    
    unsigned abs_offset = ABS(offset);
    assert(abs_offset % 4 == 0);
    
    // divide by 4.  After the divide, the result must fit in 22 bits.
    offset /= 4;
    
    // low 21 bits all 1's, the high bit (#22) is 0
    const int INT22_MAX = 0x1FFFFF; 
    
    // in 2's comp, negative numbers get 1 extra value
    const int INT22_MIN = -(INT22_MAX+1);
    
    assert(INT22_MAX > 0);
    assert(INT22_MIN < 0);
    
    if (offset < INT22_MIN) {
        return false;
    } else if (offset > INT22_MAX) {
        return false;
    } else {
        return true;
    }
}

void instruction::generateNOOP(codeGen &gen,
                               unsigned size)
{
    assert((size % instruction::size()) == 0);
    instruction insn;

    (*insn).raw = 0;
    (*insn).branch.op = 0;
    (*insn).branch.op2 = NOOPop2;
    // logLine("nop\n");
    while (size > 0) {
        insn.generate(gen);
        size -= instruction::size();
    }
}

void instruction::generateTrapRegisterSpill(codeGen &gen) {
    instruction insn;
    (*insn).raw = SPILL_REGISTERS_INSN;
    insn.generate(gen);
}

void instruction::generateFlushw(codeGen &gen) {
    instruction insn;
    
    (*insn).raw = 0;
    (*insn).rest.op = RESTop;
    (*insn).rest.op3 = FLUSHWop3;
    (*insn).rest.i = 0;
    insn.generate(gen);
}

void instruction::generateBranch(codeGen &gen, int jump_off)
{
    instruction insn;
    
    if (!offsetWithinRangeOfBranchInsn(jump_off)) {
        char buffer[100];
	sprintf(buffer, "a Branch too far; offset=%d\n", jump_off);
	logLine(buffer);
	//showErrorCallback(52, buffer);
	assert(false && "a Branch too far");
	return;
    }

    (*insn).raw = 0;
    (*insn).branch.op = 0;
    (*insn).branch.cond = BAcond;
    (*insn).branch.op2 = BICCop2;
    (*insn).branch.anneal = true;
    (*insn).branch.disp22 = jump_off >> 2;
    // logLine("ba,a %x\n", offset);
    insn.generate(gen);
}

void instruction::generateBranch(codeGen &gen, Address from, Address to)
{
    int disp = to - from;
    generateBranch(gen, disp);
}

void instruction::generateCall(codeGen &gen, Address fromAddr, 
                               Address toAddr)
{
    instruction insn;
    int dist = toAddr - fromAddr;
    (*insn).call.op = 01;
    (*insn).call.disp30 = dist >> 2;
    insn.generate(gen);
}

void instruction::generateJmpl(codeGen &gen, int rs1, int jump_off, 
                                   int rd)
{
    instruction insn;

    (*insn).resti.op = 10;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = JMPLop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(jump_off >= MIN_IMM13 && jump_off <= MAX_IMM13);
    (*insn).resti.simm13 = jump_off;
    insn.generate(gen);
}    

void instruction::generateCondBranch(codeGen &gen, int jump_off, 
                                     unsigned condition, bool annul) 
{
    instruction insn;
    
    if (!offsetWithinRangeOfBranchInsn(jump_off)) {
        char buffer[80];
	sprintf(buffer, "a branch too far, jump_off=%d\n", jump_off);
	logLine(buffer);
	showErrorCallback(52, buffer);
	abort();
    }

    (*insn).raw = 0;
    (*insn).branch.op = 0;
    (*insn).branch.cond = condition;
    (*insn).branch.op2 = BICCop2;
    (*insn).branch.anneal = annul;
    (*insn).branch.disp22 = jump_off >> 2;
    insn.generate(gen);
}

void instruction::generateAnnulledBranch(codeGen &gen, int 
                                         jump_off)
{
    generateCondBranch(gen, jump_off, BAcond, true);
}


void instruction::generateSimple(codeGen &gen, int op,
        Register rs1, Register rs2, Register rd)
{
    instruction insn;

    (*insn).raw = 0;
    (*insn).rest.op = RESTop;
    (*insn).rest.rd = rd;
    (*insn).rest.op3 = op;
    (*insn).rest.rs1 = rs1;
    (*insn).rest.rs2 = rs2;
    insn.generate(gen);
}

void instruction::generateImm(codeGen &gen, int op,
        Register rs1, int immd, Register rd)
{
    instruction insn;

    (*insn).raw = 0;
    (*insn).resti.op = RESTop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = op;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(immd >= MIN_IMM13 && immd <= MAX_IMM13);
    (*insn).resti.simm13 = immd;
    insn.generate(gen);
}

void instruction::generateImmRelOp(codeGen &gen, int cond, Register rs1,
		        int immd, Register rd)
{
    // cmp rs1, rs2
    generateImm(gen, SUBop3cc, rs1, immd, 0);
    // mov 1, rd
    generateImm(gen, ORop3, 0, 1, rd);

    // b??,a +2

    generateCondBranch(gen, 2*instruction::size(), cond, true);

    // clr rd
    generateImm(gen, ORop3, 0, 0, rd);
}

void instruction::generateRelOp(codeGen &gen, int cond, Register rs1,
                                Register rs2, Register rd)
{
    // cmp rs1, rs2
    generateSimple(gen, SUBop3cc, rs1, rs2, 0);
    // mov 1, rd
    generateImm(gen, ORop3, 0, 1, rd);

    // b??,a +2
    generateCondBranch(gen, 2*instruction::size(), cond, true);

    // clr rd
    generateImm(gen, ORop3, 0, 0, rd);
}

void instruction::generateSetHi(codeGen &gen, int src1, int dest)
{
    instruction insn;

    (*insn).raw = 0;
    (*insn).sethi.op = FMT2op;
    (*insn).sethi.rd = dest;
    (*insn).sethi.op2 = SETHIop2;
    (*insn).sethi.imm22 = HIGH22(src1);
    insn.generate(gen);
}

// st rd, [rs1 + jump_off]
void instruction::generateStore(codeGen &gen, int rd, int rs1, 
                                int store_off)
{
    instruction insn;

    (*insn).resti.op = STop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = STop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(store_off >= MIN_IMM13 && store_off <= MAX_IMM13);
    (*insn).resti.simm13 = store_off;
    insn.generate(gen);
}

// sll rs1,rs2,rd
void instruction::generateLShift(codeGen &gen, int rs1, int shift, 
                                 int rd)
{
    instruction insn;

    (*insn).restix.op = SLLop;
    (*insn).restix.op3 = SLLop3;
    (*insn).restix.rd = rd;
    (*insn).restix.rs1 = rs1;
    (*insn).restix.i = 1;
    (*insn).restix.x = 0;
    (*insn).restix.rs2 = shift;
    insn.generate(gen);
}

// sll rs1,rs2,rd
void instruction::generateRShift(codeGen &gen, int rs1, int shift, 
                                 int rd)
{
    instruction insn;

    (*insn).restix.op = SRLop;
    (*insn).restix.op3 = SRLop3;
    (*insn).restix.rd = rd;
    (*insn).restix.rs1 = rs1;
    (*insn).restix.i = 1;
    (*insn).restix.x = 0;
    (*insn).restix.rs2 = shift;
    insn.generate(gen);
}

// load [rs1 + jump_off], rd
void instruction::generateLoad(codeGen &gen, int rs1, int load_off, 
                               int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insn.generate(gen);
}

#if 0
// Unused
// swap [rs1 + jump_off], rd
void instruction::generateSwap(codeGen &gen, int rs1, int jump_off, 
                               int rd)
{
    instruction insn;

    (*insn).resti.op = SWAPop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = SWAPop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 0;
    assert(jump_off >= MIN_IMM13 && jump_off <= MAX_IMM13);
    (*insn).resti.simm13 = jump_off;
}    
#endif

// std rd, [rs1 + jump_off]
void instruction::generateStoreD(codeGen &gen, int rd, int rs1, 
                            int store_off)
{
    instruction insn;

    (*insn).resti.op = STop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = STDop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(store_off >= MIN_IMM13 && store_off <= MAX_IMM13);
    (*insn).resti.simm13 = store_off;
    insn.generate(gen);
}

// ldd [rs1 + jump_off], rd
void instruction::generateLoadD(codeGen &gen, int rs1, int load_off, 
                           int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDDop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insn.generate(gen);
}

// ldub [rs1 + jump_off], rd
void instruction::generateLoadB(codeGen &gen, int rs1, int load_off, 
                           int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDSBop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insn.generate(gen);
}

// lduh [rs1 + jump_off], rd
void instruction::generateLoadH(codeGen &gen, int rs1, int load_off, 
                           int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDSHop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insn.generate(gen);
}

// std rd, [rs1 + jump_off]
void instruction::generateStoreFD(codeGen &gen, int rd, int rs1, 
                             int store_off)
{
    instruction insn;

    (*insn).resti.op = STop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = STDFop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(store_off >= MIN_IMM13 && store_off <= MAX_IMM13);
    (*insn).resti.simm13 = store_off;
    insn.generate(gen);
}

// ldd [rs1 + jump_off], rd
void instruction::generateLoadFD(codeGen &gen, int rs1, int load_off, 
                                 int rd)
{
    instruction insn;
    
    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDDFop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insn.generate(gen);
}

/*
 * Return the displacement of the relative call or branch instruction.
 * 
 */
int instruction::get_disp()
{

  int disp = 0;
  
  // If the instruction is a CALL instruction
  if (isInsnType(CALLmask, CALLmatch)) {
      disp = (insn_.call.disp30 << 2);
  } 
  else {

    // If the instruction is a Branch instruction
      if (isInsnType(BRNCHmask, BRNCHmatch)||
          isInsnType(FBRNCHmask, FBRNCHmatch)) {
          disp = (insn_.branch.disp22 << 2);
      }
  }
  return disp;
}

/****************************************************************************/

/****************************************************************************/

/*
 * Upon setDisp being true, set the target of a relative jump or call insn.
 * Otherwise, return 0
 * 
 */
void instruction::set_disp(bool setDisp, 
                           int newOffset, bool /* outOfFunc */ ) 
{
    if (!setDisp)
        return;

    // If the instruction is a CALL instruction
    if (isInsnType(CALLmask, CALLmatch)) {
        insn_.call.disp30 = newOffset >> 2;
    }
    else {
        
        // If the instruction is a Branch instruction
        if (isInsnType(BRNCHmask, BRNCHmatch)||
            isInsnType(FBRNCHmask, FBRNCHmatch)) {
            insn_.branch.disp22 = newOffset >> 2;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

InsnRegister::InsnRegister() 
	: wordCount(0),
	  regType(InsnRegister::None),
	  regNumber(-1) {};

InsnRegister::InsnRegister(char isD,InsnRegister::RegisterType rt,
                           short rn)
	: wordCount(isD),
	  regType(rt),
	  regNumber(rn) {};

void InsnRegister::setWordCount(char isD) { wordCount = isD; }

void InsnRegister::setType(InsnRegister::RegisterType rt) { regType = rt ; }

void InsnRegister::setNumber(short rn) { regNumber = rn ; }

bool InsnRegister::is_o7(){
	if(regType == InsnRegister::GlobalIntReg)
		for(int i=0;i<wordCount;i++)
			if((regNumber+i) == 0xf)
				return true;
	return false;
}

#ifdef DEBUG
void InsnRegister::print(){
	switch(regType){
		case InsnRegister::GlobalIntReg: 
			cerr << "R[ "; break;
		case InsnRegister::FloatReg    : 
			cerr << "F[ "; break;
		case InsnRegister::CoProcReg   : 
			cerr << "C[ "; break;
		case InsnRegister::SpecialReg  : 
			cerr << "S[ "; break;
		default : 
			cerr << "NONE[ "; break;
	}

	if((regType != InsnRegister::SpecialReg) &&
	   (regType != InsnRegister::None))
		for(int i=0;i<wordCount;i++)
			cerr << regNumber+i;
	cerr << "] ";
}
#endif


inline unsigned getMaxBranch3() {
   // The length we can branch using 3 instructions
   // isn't limited.
   unsigned result = 1;
   result <<= 31;
   return result;

   // probably returning ~0 would be better, since there's no limit
}

// "Casting" methods. We use a "base + offset" model, but often need to 
// turn that into "current instruction pointer".
instructUnion *instruction::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

// Same as above, but increment offset to point at the next insn.
instructUnion *instruction::ptrAndInc(codeGen &gen) {
    instructUnion *ret = insnPtr(gen);
    gen.moveIndex(instruction::size());
    return ret;
}

void instruction::setInstruction(codeBuf_t *ptr, Address) {
    instructUnion *insnPtr = (instructUnion *)ptr;
    insn_ = *insnPtr;
}

void instruction::generate(codeGen &gen) {
    instructUnion *ptr = ptrAndInc(gen);
    *ptr = insn_;
}

void instruction::write(codeGen &gen) {
    instructUnion *ptr = insnPtr(gen);
    *ptr = insn_;
}

Address instruction::getOffset() const {
    Address ret = 0;
    if(insn_.branch.op == 0)
        ret = insn_.branch.disp22;
    else if(insn_.call.op == 0x1)
        ret = insn_.call.disp30;
    ret <<= 2;
    return (Address)ret;
}

Address instruction::getTarget(Address addr) const {
    return addr + getOffset();
}

unsigned instruction::size() {
    return 4;
}

bool instruction::isCall() const {
    return isTrueCallInsn() || isJmplCallInsn();
}

void instruction::get_register_operands(InsnRegister* rd,
                                        InsnRegister* rs1, InsnRegister* rs2)
{
        *rd = InsnRegister();
        *rs1 = InsnRegister();
        *rs2 = InsnRegister();
        
        if(!valid())
            return;
        
        switch(insn_.call.op){
        case 0x0:
            {
                if((insn_.sethi.op2 == 0x4) &&
                   (insn_.sethi.rd || insn_.sethi.imm22))
                    *rd = InsnRegister(1,
                                       InsnRegister::GlobalIntReg,
                                       (short)(insn_.sethi.rd));
                break;
            }
        case 0x2:
            {
                unsigned firstTag = insn_.rest.op3 & 0x30;
                unsigned secondTag = insn_.rest.op3 & 0xf;
                
                if((firstTag == 0x00) ||
                   (firstTag == 0x10) ||
                   ((firstTag == 0x20) && (secondTag < 0x8)) ||
                   ((firstTag == 0x30) && (secondTag >= 0x8)) ||
                   ((firstTag == 0x30) && (secondTag < 0x4)))
                    {
                        *rs1 = InsnRegister(1, InsnRegister::GlobalIntReg,
                                            (short)(insn_.rest.rs1));
                        if(!insn_.rest.i)
                            *rs2 = InsnRegister(1, InsnRegister::GlobalIntReg,
                                                (short)(insn_.rest.rs2));
                        
                        if((firstTag == 0x30) && (secondTag < 0x4))
                            *rd = InsnRegister(1, InsnRegister::SpecialReg, -1);
                        else if((firstTag != 0x30) || 
                                (secondTag <= 0x8) || 
                                (secondTag >= 0xc))
                            *rd = InsnRegister(1, InsnRegister::GlobalIntReg,
                                               (short)(insn_.rest.rd));
                    }
                else if((firstTag == 0x20) && (secondTag >= 0x8))
                    {
                        *rs1 = InsnRegister(1, InsnRegister::SpecialReg, -1);
                        *rd = InsnRegister(1, InsnRegister::GlobalIntReg,
                                           (short)(insn_.rest.rd));
                    }
                else if((secondTag == 0x6) || (secondTag == 0x7))
                    {
                        *rs1 = InsnRegister(1, InsnRegister::CoProcReg,
                                            (short)(insn_.restix.rs1));
                        *rs2 = InsnRegister(1, InsnRegister::CoProcReg,
                                            (short)(insn_.restix.rs2));
                        *rd = InsnRegister(1, InsnRegister::CoProcReg,
                                           (short)(insn_.restix.rd));
                    }
                else if((secondTag == 0x4) || (secondTag == 0x5))
                    {
                        char wC = 0;
                        switch(insn_.rest.unused & 0x03){
                        case 0x0:
                        case 0x1: wC = 1; break;
                        case 0x2: wC = 2; break;
                        case 0x3: wC = 4; break;
                        default: break; 
                        }
                        
                        *rs2 = InsnRegister(wC, InsnRegister::FloatReg,
                                            (short)(insn_.rest.rs2));
                        
                        firstTag = insn_.rest.unused & 0xf0;
                        secondTag = insn_.rest.unused & 0xf;
                        if((firstTag == 0x40) || (firstTag == 0x60)){
                            *rs1 = *rs2;
                            rs1->setNumber((short)(insn_.rest.rs1));
                        }
                        
                        if(firstTag < 0x60){
                            *rd = *rs2;
                            rd->setNumber((short)(insn_.rest.rd));
                        }
                        else{
                            if(secondTag < 0x8)
                                wC = 1;
                            else if(secondTag < 0xc)
                                wC = 2;
                            else
                                wC = 4;
                            
                            *rd = InsnRegister(wC, InsnRegister::FloatReg,
                                               (short)(insn_.rest.rd));
                        }
                    }
                
                break;
            }
        case 0x3:
            {
                *rs1 = InsnRegister(1, InsnRegister::GlobalIntReg,
                                    (short)(insn_.rest.rs1));
                if(!insn_.rest.i)
                    *rs2 = InsnRegister(1, InsnRegister::GlobalIntReg,
                                        (short)(insn_.rest.rs2));
                char wC = 1;
                InsnRegister::RegisterType rt = InsnRegister::None;
                short rn = -1;
                
                unsigned firstTag = insn_.rest.op3 & 0x30;
                unsigned secondTag = insn_.rest.op3 & 0xf;
                
                switch(firstTag){
                case 0x00:
                case 0x10:
                    rt = InsnRegister::GlobalIntReg;
                    rn = (short)(insn_.rest.rd);
                    break;
                case 0x20:
                case 0x30:
                    if((secondTag == 0x1) ||
                       (secondTag == 0x5) ||
                       (secondTag == 0x6))
                        rt = InsnRegister::SpecialReg;
                    else{
                        if(firstTag == 0x20)
                            rt = InsnRegister::FloatReg;
                        else if(firstTag == 0x30)
                            rt = InsnRegister::CoProcReg;
                        rn = (short)(insn_.rest.rd);
                    }
                    break;
                default: break;
                }
                
                if((secondTag == 0x3) ||
                   (secondTag == 0x7))
                    wC = 2;
                
                rd->setNumber(rn);
                rd->setType(rt);
                rd->setWordCount(wC);
                break;
            }
        default:
            break;
        }
        return;
}
