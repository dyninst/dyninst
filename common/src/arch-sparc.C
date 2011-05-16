/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * inst-power.C - Identify instrumentation points for a RS6000/PowerPCs
 * $Id: arch-sparc.C,v 1.34 2008/11/03 15:19:24 jaw Exp $
 */

#include <iostream>

#include "dynutil/h/Serialization.h"
#include "dynutil/h/Annotatable.h"
#include "common/h/Types.h"
#include "common/h/arch-sparc.h"
using namespace NS_sparc;

AnnotationClass<std::vector<InsnRegister> > RegisterReadSetAnno("RegisterReadSetAnno");
AnnotationClass<std::vector<InsnRegister> > RegisterWriteSetAnno("RegisterWriteSetAnno");

//inline unsigned getMaxBranch1Insn() {
//   // The length we can branch using just 1 instruction is dictated by the
//   // sparc instruction set.
//   return (0x1 << 23);
//}

/****************************************************************************/
/****************************************************************************/

instruction *instruction::copy() const {
    return new instruction(*this);
}

bool instruction::offsetWithinRangeOfBranchInsn(long offset) {
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


unsigned instruction::jumpSize(Address from, Address to, unsigned addr_width) {
    int disp = (to - from);
    return jumpSize(disp, addr_width);
}

unsigned instruction::jumpSize(int disp, unsigned /* addr_width */) {
    if (offsetWithinRangeOfBranchInsn(disp)) {
        return instruction::size();
    }
    else {
        // Save/call/restore triplet
        return 3*instruction::size();
    }
}

unsigned instruction::maxJumpSize(unsigned /* addr_width */) {
    // Save/call/restore triplet
    return 3*instruction::size();
}

unsigned instruction::maxInterFunctionJumpSize(unsigned addr_width) {
    return maxJumpSize(addr_width);
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
	  regType(InsnRegister::NoneReg),
	  regNumber(-1) {};

InsnRegister::InsnRegister(char isD,InsnRegister::RegisterType rt,
                           short rn)
	: wordCount(isD),
	  regType(rt),
	  regNumber(rn) {};

void InsnRegister::setWordCount(char isD) { wordCount = isD; }

void InsnRegister::setType(InsnRegister::RegisterType rt) { regType = rt ; }

void InsnRegister::setNumber(short rn) { regNumber = rn ; }

int InsnRegister::getWordCount() { return wordCount; }

InsnRegister::RegisterType InsnRegister::getType() { return regType; }

int InsnRegister::getNumber() { return regNumber; }

bool InsnRegister::is_o7(){
	if(regType == InsnRegister::GlobalIntReg)
		for(int i=0;i<wordCount;i++)
			if((regNumber+i) == 0xf)
				return true;
	return false;
}

#define DEBUG
#ifdef DEBUG
using namespace std;
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
	   (regType != InsnRegister::NoneReg))
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


void instruction::setInstruction(codeBuf_t *ptr, Address) {
    instructUnion *insnPtr = (instructUnion *)ptr;
    insn_ = *insnPtr;
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

int registerNumberDecoding(unsigned reg, int word_size) {
  if(word_size == SINGLE)
    return reg;
  if(word_size == DOUBLE) {
    return ((reg & 1) << 5) + (reg & 30);
  }
  if(word_size == QUAD) {
    return ((reg & 1) << 5) + (reg & 28);
  }
  fprintf(stderr,"Should have never reached here!\n");
  return -1;
}

//void instruction::get_register_operands(InsnRegister* reads,InsnRegister* writes)
void instruction::get_register_operands()
{
#if 0
  unsigned i;

  for(i=0; i<7; i++)
    reads[i] = InsnRegister();

  for(i=0; i<5; i++)
    writes[i] = InsnRegister();
#endif

  if(!valid())
     return;

  // mark the annotations
#if 0
  Annotatable<InsnRegister, register_read_set_a> &read_regs = *this;
  Annotatable<InsnRegister, register_write_set_a> &write_regs = *this;
#endif
  std::vector<InsnRegister> *read_regs_p = NULL;
  std::vector<InsnRegister> *write_regs_p = NULL;

  bool read_regs_exists = getAnnotation(read_regs_p, RegisterReadSetAnno);
  bool write_regs_exists = getAnnotation(write_regs_p, RegisterWriteSetAnno);


  //  check to see if we already have made these register sets
  if (read_regs_exists || write_regs_exists) 
  {
     //  yep, ...  just return
     return;
  }

  read_regs_p = new std::vector<InsnRegister>();
  write_regs_p = new std::vector<InsnRegister>();

  read_regs_exists = addAnnotation(read_regs_p, RegisterReadSetAnno);
  write_regs_exists = addAnnotation(write_regs_p, RegisterWriteSetAnno);

  if (!read_regs_exists)
  {
     fprintf(stderr, "%s[%d]:  addAnnotation failed here\n", FILE__, __LINE__);
     return;
  }

  if (!write_regs_exists)
  {
     fprintf(stderr, "%s[%d]:  addAnnotation failed here\n", FILE__, __LINE__);
     return;
  }
#if 0
  //  check to see if we already have made these register sets
  if (read_regs.size() || write_regs.size()) {
     //  yep, ...  just return
     return;
  }
#endif

#if 0
  //  check to see if we already have made these register sets
  if (read_regs.size() || write_regs.size()) {
     //  yep, ...  just return them
     for (unsigned int i = 0; i < 7; ++i) {
        reads[i] = read_regs[i];
     }
     for (unsigned int i = 0; i < 5; ++i) {
        writes[i] = write_regs[i];
     }
     return;
  }
#endif

#if 0
  int read = createAnnotationType("ReadSet");
  int write = createAnnotationType("WriteSet");

  InsnRegister* read1 = (InsnRegister*)getAnnotation(read);
  InsnRegister* write1 = (InsnRegister*)getAnnotation(write);
  if(0)
     if(read1 != NULL || write1 != NULL) {
        for(i=0; i<7; i++) {
           Annotation* r = getAnnotation(read,i);
           if(r != NULL) {
              reads[i] = *((InsnRegister*)r->getItem());
           }
           else
              break;
        }
        for(i=0; i<5; i++) {
           Annotation* w = getAnnotation(write,i);
           if(w != NULL) {
              writes[i] = *((InsnRegister*)w->getItem());
           }
           else
              break;
        }
        return;
     }
#endif


  std::vector<InsnRegister> reads;
  std::vector<InsnRegister> writes;
  reads.resize(8);
  writes.resize(6);

  switch(insn_.call.op){
     case 0x0:
        {
           if((insn_.sethi.op2 == 0x4) &&
                 (insn_.sethi.rd || insn_.sethi.imm22))
              writes[0] = InsnRegister(1,
                    InsnRegister::GlobalIntReg,
                    (short)(insn_.sethi.rd));
           else if(insn_.branch.op2 == BProp2)
              reads[0] = InsnRegister(1,
                    InsnRegister::GlobalIntReg,
                    (short)(insn_.rest.rs1));
           else if(insn_.branch.cond & 7) { // if not bpa or bpn
              unsigned firstTag = (((unsigned)(insn_.branch.disp22)) >> 20) & 0x03;
              if((insn_.sethi.op2 == Bop2icc) ||
                    (insn_.sethi.op2 == BPop2cc && (!firstTag)))
                 reads[0] = InsnRegister(1,InsnRegister::SpecialReg,ICC);
              else if(insn_.sethi.op2 == BPop2cc && firstTag == 0x2)
                 reads[0] = InsnRegister(1,InsnRegister::SpecialReg,XCC);
              else if(insn_.sethi.op2 == FBop2fcc)
                 reads[0] = InsnRegister(1,InsnRegister::SpecialReg,FCC0);
              else if(insn_.sethi.op2 == FBPop2fcc)
                 reads[0] = InsnRegister(1,InsnRegister::SpecialReg,FCC0 + firstTag); // Assuming FCC0, FCC1, FCC2 and FCC3 are adjacent
           }
           break;
        }
     case 0x1:
        {
           // call instruction writes to reg 15
           writes[0] = InsnRegister(1,InsnRegister::GlobalIntReg,15);
           break;	      
        }
     case 0x2:
        {
           unsigned firstTag = insn_.rest.op3 & 0x30;
           unsigned secondTag = insn_.rest.op3 & 0xf;

           if((firstTag == 0x00) ||
                 (firstTag == 0x10) ||
                 ((firstTag == 0x20) && (secondTag < 0x8)) ||
                 ((firstTag == 0x20) && (secondTag == 0xD)) || // TUGRUL: SDIVX
                 ((firstTag == 0x30) && (secondTag >= 0x8)) ||
                 ((firstTag == 0x30) && (secondTag < 0x4)))
           {
              reads[0] = InsnRegister(1, InsnRegister::GlobalIntReg,
                    (short)(insn_.rest.rs1));
              if(!insn_.rest.i)
                 reads[1] = InsnRegister(1, InsnRegister::GlobalIntReg,
                       (short)(insn_.rest.rs2));

              if((firstTag == 0x30) && (secondTag == 0x0)) {
                 switch(insn_.rest.rd) {
                    case WRY: writes[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_Y_reg); break; // WRY = 0
                    case WRCCR: writes[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_CCR); break; // WRCCR 2
                    case WRASI: writes[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_ASI); break; // WRASI 3
                    case WRFPRS: writes[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_FPRS); // WRFPRS 6
                 }
              }
              else if((firstTag == 0x30) && (secondTag == 0x2)) {
                 unsigned num = 0;
                 switch(insn_.rest.rd) {
                    case TPC: num = REG_TPC; break;
                    case TNPC: num = REG_TNPC; break;
                    case TSTATE: num = REG_TSTATE; break;
                    case TT: num = REG_TT; break;
                    case TICK_reg: num = REG_TICK; break;
			  case TBA: num = REG_TBA; break;
			  case PSTATE: num = REG_PSTATE; break;
			  case TL: num = REG_TL; break;
			  case PIL: num = REG_PIL; break;
			  case CWP: num = REG_CWP; break;
			  case CANSAVE: num = REG_CANSAVE; break;
			  case CANRESTORE: num = REG_CANRESTORE; break;
			  case CLEANWIN: num = REG_CLEANWIN; break;
			  case OTHERWIN: num = REG_OTHERWIN; break;
			  case WSTATE: num = REG_WSTATE; break;
			  }
			  if(num != 0)
			    writes[0] = InsnRegister(1, InsnRegister::SpecialReg, num);
			}
			else if((firstTag == 0x30) && ((secondTag == 0xC) ||secondTag == 0xD)) { // SAVE, RESTORE
			  reads[2] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANSAVE);
			  if(!insn_.rest.i)
			    reads[3] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANRESTORE);
			  else
			    reads[1] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANRESTORE);
			  writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,(short)(insn_.rest.rd));			  
			  writes[1] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANSAVE);
			  writes[2] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANRESTORE);
			}
			else if((firstTag == 0x30) && (secondTag == 0xA)) { // Tcc
			  unsigned code = ((insn_.rest.unused >> 6) & 3)==0 ? ICC : XCC;
			  if(!insn_.rest.i)
			    reads[2] = InsnRegister(1, InsnRegister::SpecialReg,code);
			  else
			    reads[1] = InsnRegister(1, InsnRegister::SpecialReg,code);
			  writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,(short)(insn_.rest.rd));
			}
			// else if((firstTag == 0x30) && (secondTag < 0x4))
			//  *rd = InsnRegister(1, InsnRegister::SpecialReg, -1);// XXX
			else if((firstTag != 0x30) || 
				(secondTag <= 0x8) || 
                                (secondTag >= 0xc))
			  writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,(short)(insn_.rest.rd));
                    }
                else if((firstTag == 0x20) )//&& (secondTag >= 0x8))
                    {
		      if(secondTag == 0x8) {
			switch(insn_.rest.rs1) {
			case 0: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_Y_reg); break;
			case 2: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_CCR); break;
			case 3: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_ASI); break;
			case 4: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TICK); break;
			case 5: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_PC_reg); break;
			case 6: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_FPRS); break;
			case 15: break; // STBAR or MEMBAR
			}
                        writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,
					   (short)(insn_.restix.rd));
		      }
		      else if(secondTag == 0xA) { // RDPR
			switch(insn_.rest.rs1) {
			case 0: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TPC); break;
			case 1: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TNPC); break;
			case 2: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TSTATE); break;
			case 3: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TT); break;
			case 4: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TICK); break;
			case 5: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TBA); break;
			case 6: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_PSTATE); break;
			case 7: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_TL); break;
			case 8: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_PIL); break;
			case 9: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_CWP); break;
			case 10: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANSAVE); break;
			case 11: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_CANRESTORE); break;
			case 12: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_CLEANWIN); break;
			case 13: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_OTHERWIN); break;
			case 14: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_WSTATE); break;
			case 15: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_FQ); break;
			case 31: reads[0] = InsnRegister(1, InsnRegister::SpecialReg,REG_VER); break;
			}
                        writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,
					   (short)(insn_.restix.rd));
		      }
		      else if(secondTag == 0xC) { // MOVcc
			//			int counter=0;
			unsigned cc1_cc0 = (insn_.rest.unused >> 11) & 0x03;
			unsigned all = ((insn_.rest.rs1 >> 2 ) & 0x04) + cc1_cc0;
		        short cc = FCC0;
			switch(all) {
			case 0:
			case 1:
			case 2:
			case 3: cc += all; break;
			case 4: cc = ICC; break;
			case 6: cc = XCC;
			}
			if(insn_.resti.i == 0) {
			  reads[0] = InsnRegister(1,InsnRegister::GlobalIntReg,
					      (short)(insn_.rest.rs2));
			  if((insn_.rest.rs1 & 0x07) != 0) { // if the cond field (last 4 bits of rs1:5) is 1000 or 0000 (meaning if last 3 bits of rs1 is 000), then move never or move always
			    reads[1] = InsnRegister(1,InsnRegister::SpecialReg,cc);
			  }
			}
			else{
			  if((insn_.rest.rs1 & 0x07) != 0) { // if the cond field (last 4 bits of rs1:5) is 1000 or 0000 (meaning if last 3 bits of rs1 is 000), then move never or move always
			    reads[0] = InsnRegister(1,InsnRegister::SpecialReg,cc);
			  }
			}
			writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,
					   (short)(insn_.restix.rd));
		      }
		      else if(secondTag == 0xE) { // POPC = 0x2E = 46
			if(insn_.resti.rs1 == 0) {
			  if(insn_.resti.i == 0) {
			    reads[0] = InsnRegister(1, InsnRegister::GlobalIntReg,insn_.rest.rs2);
			  }
			  writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,(short)(insn_.restix.rd));
			}
		      }
		      else if(secondTag == 0xF) { // MOVr = 0x2F = 47
			// unsigned rcond = ((*i).rest.unused >> 5) & 0x07; // this is rcond:3, I need only the last 2 bits
			if(((insn_.rest.unused >> 5) & 0x03) != 0) {
			  reads[0] = InsnRegister(1, InsnRegister::GlobalIntReg,insn_.rest.rs1);
			  if(insn_.rest.i == 0)
			    reads[1] = InsnRegister(1, InsnRegister::GlobalIntReg,insn_.rest.rs2);
			  writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,(short)(insn_.rest.rd));
			}
		      }
		      
		      reads[0] = InsnRegister(1, InsnRegister::SpecialReg, -1);
		      writes[0] = InsnRegister(1, InsnRegister::GlobalIntReg,
					 (short)(insn_.restix.rd));
		    }
                else if((secondTag == 0x6) || (secondTag == 0x7)) // firstTag=0x30
                    {
                        reads[0] = InsnRegister(1, InsnRegister::CoProcReg,
                                            (short)(insn_.restix.rs1));
                        reads[1] = InsnRegister(1, InsnRegister::CoProcReg,
                                            (short)(insn_.restix.rs2));
                        writes[0] = InsnRegister(1, InsnRegister::CoProcReg,
                                           (short)(insn_.restix.rd));
                    }
                else if((secondTag == 0x4) || (secondTag == 0x5)) // firstTag=0x30
                    {
                        char wC = 0;
                        firstTag = insn_.rest.unused & 0xf0;
                        unsigned thirdTag = insn_.rest.unused & 0xf;
                        switch(insn_.rest.unused & 0x03){
                        case 0x0:
			  if(firstTag==0xC0) // FiTO(s,d,q)
			    wC=1;
			  else // FxTO(s,d,q)
			    wC=2;
			  break;
                        case 0x1: wC = 1; break;
                        case 0x2: wC = 2; break;
                        case 0x3: wC = 4; break;
                        default: break; 
                        }
                        
                        reads[1] = InsnRegister(wC, InsnRegister::FloatReg,
                                            (short)(insn_.rest.rs2));
                        
                        if((firstTag == 0x40) || (firstTag == 0x60) || /*TUGRUL: for FCMP instruction family*/ (firstTag == 0x50)) {
                            reads[0] = reads[1];
                            reads[0].setNumber((short)(insn_.rest.rs1));
                        }
                        
                        if(firstTag < 0x50 || (secondTag == 0x5 && firstTag != 0x50)){
                            writes[0] = reads[1];
                            writes[0].setNumber((short)(insn_.rest.rd));
                        }
                        else if(firstTag != 0x50) { // FCMP family is excluded
			  if(firstTag == 0x80 && thirdTag < 0x4) // FsTOx, FdTOx, FqTOx
			    wC = 2;
			  else if(thirdTag < 0x8)
			    wC = 1;
			  else if(thirdTag < 0xc)
			    wC = 2;
			  else
			    wC = 4;
			  
			  writes[0] = InsnRegister(wC, InsnRegister::FloatReg,
					     (short)(insn_.rest.rd));
                        }

			// floating point condition codes
			if(secondTag == 0x5 && thirdTag < 0x4) {
			  unsigned opf = ((insn_.rest.i << 8) | insn_.rest.unused) & 0x1FF;
			  switch(opf) {
			  case 0x000: reads[2] = InsnRegister(1, InsnRegister::SpecialReg,FCC0); break;
			  case 0x040: reads[2] = InsnRegister(1, InsnRegister::SpecialReg,FCC1); break;
			  case 0x050: writes[0] = InsnRegister(1, InsnRegister::SpecialReg,FCC0 + (insn_.rest.rd & 3)); break;
			  case 0x080: reads[2] = InsnRegister(1, InsnRegister::SpecialReg,FCC2); break;
			  case 0x0C0: reads[2] = InsnRegister(1, InsnRegister::SpecialReg,FCC3); break;
			  case 0x100: reads[2] = InsnRegister(1, InsnRegister::SpecialReg,ICC); break;
			  case 0x180: reads[2] = InsnRegister(1, InsnRegister::SpecialReg,XCC); break;
			  }
			}
                    }


                // Handle Condition Codes
		firstTag = insn_.rest.op3 & 0x30;
		secondTag = insn_.rest.op3 & 0x3C;
		if((firstTag == 0x10) || (secondTag == 0x20)) {
		  writes[1] = InsnRegister(1, InsnRegister::SpecialReg, ICC);
		  writes[2] = InsnRegister(1, InsnRegister::SpecialReg, XCC);
		}

		firstTag = insn_.rest.op3 & 0x2B;
		if(firstTag == 0x08) { // ADDC, ADDCcc, SUBC and SUBCcc
		  reads[2] = InsnRegister(1, InsnRegister::SpecialReg, ICC);
		}
		break;
            }
        case 0x3:
            {
                char wC = 1;
                InsnRegister::RegisterType rt = InsnRegister::NoneReg;
                short rn = -1;
                
                unsigned firstTag = insn_.rest.op3 & 0x30;
                unsigned secondTag = insn_.rest.op3 & 0xf;
                
                if(firstTag >= 0x20 && (secondTag == 0x2 || secondTag == 0x6)) { // LDQF, LDQFA, STQF, STQFA
		  wC = 4;
		}
		else if((secondTag == 0x3) ||
                   (secondTag == 0x7))
                    wC = 2;

                reads[0] = InsnRegister(1, InsnRegister::GlobalIntReg,
                                    (short)(insn_.rest.rs1));
                if(!insn_.rest.i)
                    reads[1] = InsnRegister(1, InsnRegister::GlobalIntReg,
                                        (short)(insn_.rest.rs2));

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
		      if(firstTag == 0x20 /*|| (firstTag == 0x30 && secondTag <= 0xB)*/)
			rt = InsnRegister::FloatReg;
		      else if(firstTag == 0x30)
			rt = InsnRegister::CoProcReg;
		      rn = (short)(insn_.rest.rd);
                    }
                    break;
                default: break;
                }

		if(firstTag >= 0x20) {
		  if(secondTag == 0x2 || secondTag == 0x6) {
		    rn = registerNumberDecoding(insn_.rest.rd,QUAD);
		  }
		  else if(secondTag == 0x3 || secondTag == 0x7) {
		    rn = registerNumberDecoding(insn_.rest.rd,DOUBLE);
		  }
		}
		// if store instruction, reads all three registers.
                if((firstTag <= 0x10 && ((secondTag >= 0x4 && secondTag <= 0x7) || secondTag == 0xE))
		  || (firstTag >= 0x20 && (secondTag == 0x6 || secondTag == 0x7 || secondTag == 0x4))) {
		  if(!insn_.rest.i)
		    reads[2] = InsnRegister(wC, rt,rn);
		  else
		    reads[1] = InsnRegister(wC, rt,rn);
		}
		else if(firstTag == 0x20 && secondTag == 0x1) { // LDFSR
		  writes[0] = InsnRegister(1, InsnRegister::SpecialReg,FSR);
		  writes[1] = InsnRegister(1, InsnRegister::SpecialReg,FCC0);
		  writes[2] = InsnRegister(1, InsnRegister::SpecialReg,FCC1);
		  writes[3] = InsnRegister(1, InsnRegister::SpecialReg,FCC2);
		  writes[4] = InsnRegister(1, InsnRegister::SpecialReg,FCC3);
		}
		else if(firstTag == 0x20 && secondTag == 0x5) { // STFSR
		  if(!insn_.rest.i)
		    reads[6] = InsnRegister(1, InsnRegister::SpecialReg,FSR);
		  else
		    reads[1] = InsnRegister(1, InsnRegister::SpecialReg,FSR);
		  reads[2] = InsnRegister(1, InsnRegister::SpecialReg,FCC0);
		  reads[3] = InsnRegister(1, InsnRegister::SpecialReg,FCC1);
		  reads[4] = InsnRegister(1, InsnRegister::SpecialReg,FCC2);
		  reads[5] = InsnRegister(1, InsnRegister::SpecialReg,FCC3);
		}
		else if(firstTag < 0x20 || secondTag != 0xD) { // all but PREFETCH and PREFETCHA
		  writes[0] = InsnRegister(wC,rt,rn);
		  if(firstTag == 0x30 && (secondTag == 0xC || secondTag == 0xE)) { // CASA, CASXA
		    if(!insn_.rest.i)
		      reads[2] = InsnRegister(1, InsnRegister::GlobalIntReg, (short)(insn_.rest.rd));
		    else
		      reads[1] = InsnRegister(1, InsnRegister::GlobalIntReg, (short)(insn_.rest.rd));
		  }
		}
                break;
            }
        default:
            break;
        }

  if (reads.size() > 8)
     fprintf(stderr, "%s[%d]:  WARNING:  unexpected size %ld for read registers\n", FILE__, __LINE__, (long int) reads.size());

  for (unsigned int i = 0; i < reads.size(); ++i) {
     if (reads[i].getNumber() == -1)
        break;
     read_regs_p->push_back(reads[i]);
#if 0
     read_regs.addAnnotation(reads[i]);
#endif
  }



  if (writes.size() > 6)
     fprintf(stderr, "%s[%d]:  WARNING:  unexpected size %ld for write registers\n", FILE__, __LINE__, (long int) writes.size());

  for (unsigned int i = 0; i < writes.size(); ++i) {
     if (writes[i].getNumber() == -1)
        break;
     write_regs_p->push_back(writes[i]);
#if 0
     write_regs.addAnnotation(writes[i]);
#endif
  }

#if 0
#if 0
  // mark the annotations
	read = createAnnotationType("ReadSet");
	write = createAnnotationType("WriteSet");
#endif

	for(i=0; i<7; i++) {
      read_regs.addAnnotation(reads[i]);
#if 0
	  if(reads[i].getNumber() != -1) {
	    setAnnotation(read,new Annotation(&(reads[i])));
	  }
	  else
	    break;
#endif
	}
	for(i=0; i<5; i++) {
        write_regs.addAnnotation(writes[i]);
#if 0
	  if(writes[i].getNumber() != -1) {
	    setAnnotation(write,new Annotation(&(writes[i])));
	  }
	  else
	    break;
#endif
	}
#endif	
        return;
}


unsigned instruction::spaceToRelocate() const {
    
    unsigned size_required = 0;

    if (isInsnType(CALLmask, CALLmatch)) {
        // We can use the same format.
      // We may need 3 more words if it's a getPC jump.
        size_required += 4*instruction::size();
    } else if (isInsnType(BRNCHmask, BRNCHmatch) ||
               isInsnType(FBRNCHmask, FBRNCHmatch)) {
        // Worst case: 3 insns (save, call, restore)
        size_required += 3*instruction::size();
    }
    else {
        // Relocate "in place"
        size_required += instruction::size();
    }
    
    if (isDCTI()) {
        size_required += 2*instruction::size();
    }

    return size_required;
}

bool instruction::getUsedRegs(pdvector<int> &) {
	return false;

}
