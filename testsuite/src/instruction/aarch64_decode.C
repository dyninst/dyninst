/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "instruction_comp.h"
#include "test_lib.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "dyn_regs.h"
#include <boost/assign/list_of.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <deque>
#include <iostream>
using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace boost::assign;

using namespace std;

class aarch64_decode_Mutator : public InstructionMutator {
private:
	void setupRegisters();
	void reverseBuffer(const unsigned char  *, int);
public:
    aarch64_decode_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* aarch64_decode_factory()
{
   return new aarch64_decode_Mutator();
}

void aarch64_decode_Mutator::reverseBuffer(const unsigned char *buffer, int bufferSize)
{
    int elementCount = bufferSize/4;
    unsigned char *currentElement = const_cast<unsigned char *>(buffer);

    for(int loop_index = 0; loop_index < elementCount; loop_index++)
    {
	std::swap(currentElement[0], currentElement[3]);
	std::swap(currentElement[1], currentElement[2]);

	currentElement += 4;
    }	
}

test_results_t aarch64_decode_Mutator::executeTest()
{
  const unsigned char buffer[] =
  {
	0x0B, 0xC0, 0x04, 0x00,		// ADD W0, W0, W0, ROR #1
	0x0B, 0x0C, 0x01, 0x41,		// ADD W1, W10, W12
	0x0B, 0x08, 0x14, 0xA0,		// ADD W0, W5, W8, LSL #5
	0x8B, 0x49, 0x28, 0xE4,		// ADD X4, X7, X9, LSR #10
	0x4B, 0x04, 0x00, 0x40,		// SUB W0, W2, W4
	0xCB, 0x8B, 0x1D, 0x06,		// SUB X6, X8, X11, ASR #7
	0x2B, 0x08, 0x14, 0xA0,		// ADDS W0, W5, W8, LSL #5
	0x0B, 0x2F, 0x69, 0x45,		// ADD W5, W10, W15
	0xCB, 0x21, 0x80, 0x20,		// SUB X0, X1, X1, UXTB #0
	0xEB, 0x22, 0x70, 0x42,		// SUBS X2, X2, X2, SXTW #4
	0x1A, 0x19, 0x02, 0xC5,		// ADC W5, W22, W25
	0xDA, 0x02, 0x00, 0x20,		// SBC X0, X1, X2
	0x3A, 0x5E, 0x58, 0xEB,		// CCMN W7, #30, #11, 5
	0xFA, 0x48, 0xFA, 0x88,		// CCMP X20, #8, #0, 15
	0x3A, 0x4A, 0x10, 0xA7,		// CCMN W5, W10, #7, 1
	0xFA, 0x42, 0xA0, 0x84,		// CCMP X2, X4, #4, 10
	0x1A, 0x8F, 0x11, 0x45,		// CSEL W5, W10, W15, 1
	0x9A, 0x84, 0x54, 0x40, 	// CSINC X0, X2, X4, 5
	0xDA, 0x96, 0x72, 0xB4,		// CSINV X20, X21, X22, 7
	0x5A, 0x8A, 0xA4, 0xA1,		// CSNEG W1, W5, W9, 10
	0x5A, 0xC0, 0x00, 0x41,		// RBIT W1, W2
	0xDA, 0xC0, 0x0E, 0x8A,		// REV X10, X20
	0x5A, 0xC0, 0x13, 0xBE,		// CLZ W30, W29
	0xDA, 0xC0, 0x15, 0x8B,		// CLS X11, X12
	0x5A, 0xC0, 0x05, 0x80,		// REV16 W0, W12
	0x1A, 0xC4, 0x08, 0x40,		// UDIV W0, W2, W4
	0x9A, 0xD9, 0x0E, 0x8F,		// SDIV X15, X20, X25
	0x1A, 0xCB, 0x21, 0x05,		// LSLV W5, W8, W11
	0x9A, 0xCB, 0x29, 0x27,		// ASRV X7, X9, X11
	0x1A, 0xC4, 0x2C, 0x10,		// RORV W16, W0, W4
	0x1B, 0x02, 0x00, 0x61,		// MADD W1, W3, W2, W0
	0x9B, 0x10, 0xF9, 0x04,		// MSUB X4, X8, X16, X30
	0x9B, 0x21, 0x84, 0x00,		// SMSUBL X0, X0, X1, X1
	0x9B, 0xCA, 0x28, 0xA5,		// UMULH W5, W5, W10, W10
	0x0A, 0x03, 0x00, 0x41,		// AND W1, W2, W3
	0x8A, 0x2A, 0x14, 0xA0,		// BIC X0, X5, X10, LSL #5
	0x2A, 0x62, 0x28, 0x00,		// ORN W0, W0, W2, LSR #10
	0xCA, 0x96, 0x0A, 0xB4,		// EOR X20, X21, X22, ASR #2
	0xEA, 0xE1, 0x20, 0x21,		// BICS X1, X1, X1, ROR #8
	0x11, 0x00, 0x2F, 0xE0,		// ADD W0, WSP, #11
	0x31, 0x40, 0x01, 0x45,		// ADDS W5, W10, #0, LSL #12
	0xD1, 0x40, 0x31, 0x5F,		// SUB SP, X10, #12
	0x13, 0x19, 0x29, 0xCC,		// SBFM W12, W14, #25, #10
	0xB3, 0x40, 0x07, 0xC0,		// BFM X0, X30, #63, #0
	0xD3, 0x41, 0x20, 0x14,		// UBFM X20, X0, #8, #1
	0x13, 0x9E, 0x16, 0x8A,		// EXTR W10, W20, W30, #5
	0x93, 0xD0, 0xFD, 0x00,		// EXTR X0, X8, X16, #63
	0x12, 0x2A, 0x1F, 0x1F,		// AND WSP, W24, #63
	0xB2, 0x00, 0x03, 0xDF,		// ORR SP, X30, #0
	0xD2, 0x7F, 0xAF, 0x34,		// EOR X20, X25, #
	0x72, 0x00, 0x25, 0x45,		// ANDS W5, W10, #9
	0x12, 0xA0, 0x02, 0xE4,		// MOVN W4, #23, LSL #1
	0xD2, 0xC0, 0x02, 0x54,		// MOVZ X20, #18, LSL #2
	0xF2, 0xE0, 0x20, 0x01,		// MOVK X1, #256, LSL #3
	0x12, 0x80, 0x01, 0x08,		// MOVN W8, #8
	0x10, 0x80, 0x00, 0x00,		// ADR X0, #
	0xF0, 0x00, 0x00, 0x3E,		// ADRP X30, #7
	0x34, 0xFF, 0xFF, 0xEF,		// CBZ W15, #
	0xB5, 0x00, 0x00, 0x3E,		// CBNZ X30, #1
	0x54, 0xFF, 0xFF, 0xE1,		// B.NE #
	0x54, 0x00, 0x07, 0xEC,		// B.GT #63
	0x36, 0xF7, 0xFF, 0xE4,		// TBZ W4, #30, #
	0xB7, 0x80, 0x00, 0x19,		// TBNZ X25, #0, #16
	0x37, 0x60, 0x01, 0x9F,		// TBNZ WZR, #9, #12
	0x17, 0xFF, 0xFF, 0xFF,		// B #
	0x94, 0x00, 0x00, 0x08,		// BL #8
	0xD6, 0x1F, 0x01, 0x80,		// BR X12
	0xD6, 0x3F, 0x03, 0xC0,		// BLR X30
	0xD6, 0x5F, 0x00, 0x00,		// RET X0
	//0xD6, 0x9F, 0x03, 0xE0,		// ERET
	//0xD6, 0xBF, 0x03, 0xE0,		// DRPS
	0x1E, 0x3F, 0x20, 0x00,		// FCMP S0, S31
	0x1E, 0x30, 0x21, 0x08,		// FCMP D16, #0.0
	0x1E, 0x7F, 0x23, 0xC0,		// FCMP D31, D32
	0x1E, 0x3F, 0xA6, 0x88,		// FCCMP S20, S31, #8, 10
	0x1E, 0x62, 0x04, 0x25,		// FCCMP D1, D2, #5, 0
	0x1E, 0x6B, 0x55, 0x59,		// FCCMPE D10, D1,, #9, 5
	0x1E, 0x23, 0x4C, 0x41,		// FCSEL S1, S, S3, 4
	0x1E, 0x20, 0x41, 0x45,		// FMOV S5, S10
	0x1E, 0x60, 0xC3, 0xFF,		// FABS D30, D31
	0x1E, 0x64, 0xC0, 0x40,		// FRINTP D0, D2
	0x1E, 0xE2, 0x40, 0xA4,		// FCVT S4, H5
	0x1E, 0xE2, 0xC3, 0xE0,		// FCVT D0, H31
	0x1E, 0x22, 0xC0, 0x02,		// FCVT D2, S0
	0x1E, 0x63, 0xC3, 0xFF,		// FCVT H31, D31
	0x1E, 0x62, 0x40, 0x21,		// FCVT S1, D1
	0x1E, 0x23, 0xC2, 0x08,		// FCVT H8, S16
	0x1E, 0x22, 0x08, 0x20,		// FMUL S0, S1, S2
	0x1E, 0x7F, 0x3B, 0xDD,		// FSUB D29, D30, D31
	0x1E, 0x6F, 0x19, 0x45,		// FDIV D5, D10, D15
	0x1E, 0x20, 0x4A, 0x08,		// FMAX S8, S16, S0
	0x1E, 0x21, 0x78, 0x21,		// FNINNM S1, S1, S1
	0x1F, 0x02, 0x0C, 0x20,		// FMADD S0, S1, S2, S3
	0x1F, 0x48, 0xC0, 0x82,		// FMSUB D2, D4, D8, D16
	0x1F, 0x2B, 0x35, 0x6A,		// FNMADD S10, S11, S11, S13
	0x1F, 0x62, 0x84, 0x88,		// FNMSUB D8, D4, D2, D1
	0x1E, 0x31, 0x10, 0x00,		// FMOV S0, #88
	0x1E, 0x67, 0xF0, 0x1F,		// FMOV D31, #7F
	0x1E, 0x02, 0x17, 0xC0,		// SCVTF S0, W30, #59
	0x9E, 0x43, 0x24, 0x01,		// UCVTF D1, X0, #55
	0x1E, 0x02, 0x01, 0x45,		// SCVTF S5, W10, #64
	0x1E, 0x43, 0x04, 0x48,		// UCVTF D8, W2, #63
	0x1E, 0x19, 0x04, 0x0B,		// FCVTZU W12, S0, #63
	0x9E, 0x58, 0xFF, 0xFE,		// FCVTZS X30, D31, #0
	0x9E, 0x19, 0xE1, 0x41,		// FCVTZU X1, S10, #8
	0x1E, 0x58, 0xF1, 0x29,		// FCVTZS W9, D9, #4
	0x1E, 0x20, 0x00, 0xA8,		// FCVTNS W8, S5
	0x1E, 0x27, 0x03, 0xC1,		// FMOV S1, W30
	0xD4, 0x10, 0x00, 0x01,		// SVC #32768
	0xD4, 0x00, 0x00, 0x03, 	// SMC #0
	0xD4, 0x40, 0x03, 0xC0,		// HLT #30
	0xD4, 0xA0, 0x00, 0x42,		// DCPS2 #2
	0xD5, 0x03, 0x30, 0x5F,		// CLREX
	0xD5, 0x03, 0x34, 0x9F,		// DSB #4
	0xD5, 0x03, 0x31, 0xBF,		// DMB #1
	0xD5, 0x03, 0x20, 0xBF,		// HINT #5
	0xD5, 0x03, 0x45, 0xDF,		// MSR 30, #5
	0xD5, 0x09, 0x23, 0x80,		// SYS #1, #2, #3, #4, X0
	0xD5, 0x29, 0x23, 0x9E,		// SYSL #1, #2, #3, #4, X30
	0xD5, 0x3B, 0x9C, 0xC1,		// MRS X1, PMCEID0_EL0
	0xD5, 0x3B, 0xE8, 0x40,		// MRS X0, PMEVCNTR2_EL0
	0xD5, 0x1B, 0xE0, 0x21,		// MSR CNTPCT_EL0, X1
	0xD5, 0x1B, 0xEF, 0xC0,		// MSR PMEVTYPER30_EL0, X0
    0x00, 0x00, 0x00, 0x00      // INVALID
  };

  unsigned int size = sizeof(buffer);
  unsigned int expectedInsns = size/4;

  ++expectedInsns;
  reverseBuffer(buffer, size);
  InstructionDecoder d(buffer, size, Dyninst::Arch_aarch64);

  std::deque<Instruction::Ptr> decodedInsns;
  Instruction::Ptr i;
  do
  {
    i = d.decode();
    decodedInsns.push_back(i);
    if(i != NULL)
      decodedInsns.back()->format();
  }
  while(i && i->isValid());

  if(decodedInsns.size() != expectedInsns)
  {
    logerror("FAILED: Expected %d instructions, decoded %d\n", expectedInsns, decodedInsns.size());
    for(std::deque<Instruction::Ptr>::iterator curInsn = decodedInsns.begin();
	curInsn != decodedInsns.end();
	++curInsn)
    {
        if(*curInsn) logerror("\t%s\n", (*curInsn)->format().c_str());
    }

    return FAILED;
  }

  if(decodedInsns.back() && decodedInsns.back()->isValid())
  {
    logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
    return FAILED;
  }

  RegisterAST::Ptr x0 (new RegisterAST(aarch64::x0));
  RegisterAST::Ptr x1 (new RegisterAST(aarch64::x1));
  RegisterAST::Ptr x2 (new RegisterAST(aarch64::x2));
  RegisterAST::Ptr x3 (new RegisterAST(aarch64::x3));
  RegisterAST::Ptr x4 (new RegisterAST(aarch64::x4));
  RegisterAST::Ptr x5 (new RegisterAST(aarch64::x5));
  RegisterAST::Ptr x6 (new RegisterAST(aarch64::x6));
  RegisterAST::Ptr x7 (new RegisterAST(aarch64::x7));
  RegisterAST::Ptr x8 (new RegisterAST(aarch64::x8));
  RegisterAST::Ptr x9 (new RegisterAST(aarch64::x9));
  RegisterAST::Ptr x10(new RegisterAST(aarch64::x10));
  RegisterAST::Ptr x11(new RegisterAST(aarch64::x11));
  RegisterAST::Ptr x12(new RegisterAST(aarch64::x12));
  RegisterAST::Ptr x13(new RegisterAST(aarch64::x13));
  RegisterAST::Ptr x14(new RegisterAST(aarch64::x14));
  RegisterAST::Ptr x15(new RegisterAST(aarch64::x15));
  RegisterAST::Ptr x16(new RegisterAST(aarch64::x16));
  RegisterAST::Ptr x17(new RegisterAST(aarch64::x17));
  RegisterAST::Ptr x18(new RegisterAST(aarch64::x18));
  RegisterAST::Ptr x19(new RegisterAST(aarch64::x19));
  RegisterAST::Ptr x20(new RegisterAST(aarch64::x20));
  RegisterAST::Ptr x21(new RegisterAST(aarch64::x21));
  RegisterAST::Ptr x22(new RegisterAST(aarch64::x22));
  RegisterAST::Ptr x23(new RegisterAST(aarch64::x23));
  RegisterAST::Ptr x24(new RegisterAST(aarch64::x24));
  RegisterAST::Ptr x25(new RegisterAST(aarch64::x25));
  RegisterAST::Ptr x26(new RegisterAST(aarch64::x26));
  RegisterAST::Ptr x27(new RegisterAST(aarch64::x27));
  RegisterAST::Ptr x28(new RegisterAST(aarch64::x28));
  RegisterAST::Ptr x29(new RegisterAST(aarch64::x29));
  RegisterAST::Ptr x30(new RegisterAST(aarch64::x30));

  RegisterAST::Ptr w0 (new RegisterAST(aarch64::w0));
  RegisterAST::Ptr w1 (new RegisterAST(aarch64::w1));
  RegisterAST::Ptr w2 (new RegisterAST(aarch64::w2));
  RegisterAST::Ptr w3 (new RegisterAST(aarch64::w3));
  RegisterAST::Ptr w4 (new RegisterAST(aarch64::w4));
  RegisterAST::Ptr w5 (new RegisterAST(aarch64::w5));
  RegisterAST::Ptr w6 (new RegisterAST(aarch64::w6));
  RegisterAST::Ptr w7 (new RegisterAST(aarch64::w7));
  RegisterAST::Ptr w8 (new RegisterAST(aarch64::w8));
  RegisterAST::Ptr w9 (new RegisterAST(aarch64::w9));
  RegisterAST::Ptr w10(new RegisterAST(aarch64::w10));
  RegisterAST::Ptr w11(new RegisterAST(aarch64::w11));
  RegisterAST::Ptr w12(new RegisterAST(aarch64::w12));
  RegisterAST::Ptr w13(new RegisterAST(aarch64::w13));
  RegisterAST::Ptr w14(new RegisterAST(aarch64::w14));
  RegisterAST::Ptr w15(new RegisterAST(aarch64::w15));
  RegisterAST::Ptr w16(new RegisterAST(aarch64::w16));
  RegisterAST::Ptr w17(new RegisterAST(aarch64::w17));
  RegisterAST::Ptr w18(new RegisterAST(aarch64::w18));
  RegisterAST::Ptr w19(new RegisterAST(aarch64::w19));
  RegisterAST::Ptr w20(new RegisterAST(aarch64::w20));
  RegisterAST::Ptr w21(new RegisterAST(aarch64::w21));
  RegisterAST::Ptr w22(new RegisterAST(aarch64::w22));
  RegisterAST::Ptr w23(new RegisterAST(aarch64::w23));
  RegisterAST::Ptr w24(new RegisterAST(aarch64::w24));
  RegisterAST::Ptr w25(new RegisterAST(aarch64::w25));
  RegisterAST::Ptr w26(new RegisterAST(aarch64::w26));
  RegisterAST::Ptr w27(new RegisterAST(aarch64::w27));
  RegisterAST::Ptr w28(new RegisterAST(aarch64::w28));
  RegisterAST::Ptr w29(new RegisterAST(aarch64::w29));
  RegisterAST::Ptr w30(new RegisterAST(aarch64::w30));

  RegisterAST::Ptr q0 (new RegisterAST(aarch64::q0));
  RegisterAST::Ptr q1 (new RegisterAST(aarch64::q1));
  RegisterAST::Ptr q2 (new RegisterAST(aarch64::q2));
  RegisterAST::Ptr q3 (new RegisterAST(aarch64::q3));
  RegisterAST::Ptr q4 (new RegisterAST(aarch64::q4));
  RegisterAST::Ptr q5 (new RegisterAST(aarch64::q5));
  RegisterAST::Ptr q6 (new RegisterAST(aarch64::q6));
  RegisterAST::Ptr q7 (new RegisterAST(aarch64::q7));
  RegisterAST::Ptr q8 (new RegisterAST(aarch64::q8));
  RegisterAST::Ptr q9 (new RegisterAST(aarch64::q9));
  RegisterAST::Ptr q10(new RegisterAST(aarch64::q10));
  RegisterAST::Ptr q11(new RegisterAST(aarch64::q11));
  RegisterAST::Ptr q12(new RegisterAST(aarch64::q12));
  RegisterAST::Ptr q13(new RegisterAST(aarch64::q13));
  RegisterAST::Ptr q14(new RegisterAST(aarch64::q14));
  RegisterAST::Ptr q15(new RegisterAST(aarch64::q15));
  RegisterAST::Ptr q16(new RegisterAST(aarch64::q16));
  RegisterAST::Ptr q17(new RegisterAST(aarch64::q17));
  RegisterAST::Ptr q18(new RegisterAST(aarch64::q18));
  RegisterAST::Ptr q19(new RegisterAST(aarch64::q19));
  RegisterAST::Ptr q20(new RegisterAST(aarch64::q20));
  RegisterAST::Ptr q21(new RegisterAST(aarch64::q21));
  RegisterAST::Ptr q22(new RegisterAST(aarch64::q22));
  RegisterAST::Ptr q23(new RegisterAST(aarch64::q23));
  RegisterAST::Ptr q24(new RegisterAST(aarch64::q24));
  RegisterAST::Ptr q25(new RegisterAST(aarch64::q25));
  RegisterAST::Ptr q26(new RegisterAST(aarch64::q26));
  RegisterAST::Ptr q27(new RegisterAST(aarch64::q27));
  RegisterAST::Ptr q28(new RegisterAST(aarch64::q28));
  RegisterAST::Ptr q29(new RegisterAST(aarch64::q29));
  RegisterAST::Ptr q30(new RegisterAST(aarch64::q30));
  RegisterAST::Ptr q31(new RegisterAST(aarch64::q31));

  RegisterAST::Ptr s0 (new RegisterAST(aarch64::s0));
  RegisterAST::Ptr s1 (new RegisterAST(aarch64::s1));
  RegisterAST::Ptr s2 (new RegisterAST(aarch64::s2));
  RegisterAST::Ptr s3 (new RegisterAST(aarch64::s3));
  RegisterAST::Ptr s4 (new RegisterAST(aarch64::s4));
  RegisterAST::Ptr s5 (new RegisterAST(aarch64::s5));
  RegisterAST::Ptr s6 (new RegisterAST(aarch64::s6));
  RegisterAST::Ptr s7 (new RegisterAST(aarch64::s7));
  RegisterAST::Ptr s8 (new RegisterAST(aarch64::s8));
  RegisterAST::Ptr s9 (new RegisterAST(aarch64::s9));
  RegisterAST::Ptr s10(new RegisterAST(aarch64::s10));
  RegisterAST::Ptr s11(new RegisterAST(aarch64::s11));
  RegisterAST::Ptr s12(new RegisterAST(aarch64::s12));
  RegisterAST::Ptr s13(new RegisterAST(aarch64::s13));
  RegisterAST::Ptr s14(new RegisterAST(aarch64::s14));
  RegisterAST::Ptr s15(new RegisterAST(aarch64::s15));
  RegisterAST::Ptr s16(new RegisterAST(aarch64::s16));
  RegisterAST::Ptr s17(new RegisterAST(aarch64::s17));
  RegisterAST::Ptr s18(new RegisterAST(aarch64::s18));
  RegisterAST::Ptr s19(new RegisterAST(aarch64::s19));
  RegisterAST::Ptr s20(new RegisterAST(aarch64::s20));
  RegisterAST::Ptr s21(new RegisterAST(aarch64::s21));
  RegisterAST::Ptr s22(new RegisterAST(aarch64::s22));
  RegisterAST::Ptr s23(new RegisterAST(aarch64::s23));
  RegisterAST::Ptr s24(new RegisterAST(aarch64::s24));
  RegisterAST::Ptr s25(new RegisterAST(aarch64::s25));
  RegisterAST::Ptr s26(new RegisterAST(aarch64::s26));
  RegisterAST::Ptr s27(new RegisterAST(aarch64::s27));
  RegisterAST::Ptr s28(new RegisterAST(aarch64::s28));
  RegisterAST::Ptr s29(new RegisterAST(aarch64::s29));
  RegisterAST::Ptr s30(new RegisterAST(aarch64::s30));
  RegisterAST::Ptr s31(new RegisterAST(aarch64::s31));

  RegisterAST::Ptr h0 (new RegisterAST(aarch64::h0));
  RegisterAST::Ptr h1 (new RegisterAST(aarch64::h1));
  RegisterAST::Ptr h2 (new RegisterAST(aarch64::h2));
  RegisterAST::Ptr h3 (new RegisterAST(aarch64::h3));
  RegisterAST::Ptr h4 (new RegisterAST(aarch64::h4));
  RegisterAST::Ptr h5 (new RegisterAST(aarch64::h5));
  RegisterAST::Ptr h6 (new RegisterAST(aarch64::h6));
  RegisterAST::Ptr h7 (new RegisterAST(aarch64::h7));
  RegisterAST::Ptr h8 (new RegisterAST(aarch64::h8));
  RegisterAST::Ptr h9 (new RegisterAST(aarch64::h9));
  RegisterAST::Ptr h10(new RegisterAST(aarch64::h10));
  RegisterAST::Ptr h11(new RegisterAST(aarch64::h11));
  RegisterAST::Ptr h12(new RegisterAST(aarch64::h12));
  RegisterAST::Ptr h13(new RegisterAST(aarch64::h13));
  RegisterAST::Ptr h14(new RegisterAST(aarch64::h14));
  RegisterAST::Ptr h15(new RegisterAST(aarch64::h15));
  RegisterAST::Ptr h16(new RegisterAST(aarch64::h16));
  RegisterAST::Ptr h17(new RegisterAST(aarch64::h17));
  RegisterAST::Ptr h18(new RegisterAST(aarch64::h18));
  RegisterAST::Ptr h19(new RegisterAST(aarch64::h19));
  RegisterAST::Ptr h20(new RegisterAST(aarch64::h20));
  RegisterAST::Ptr h21(new RegisterAST(aarch64::h21));
  RegisterAST::Ptr h22(new RegisterAST(aarch64::h22));
  RegisterAST::Ptr h23(new RegisterAST(aarch64::h23));
  RegisterAST::Ptr h24(new RegisterAST(aarch64::h24));
  RegisterAST::Ptr h25(new RegisterAST(aarch64::h25));
  RegisterAST::Ptr h26(new RegisterAST(aarch64::h26));
  RegisterAST::Ptr h27(new RegisterAST(aarch64::h27));
  RegisterAST::Ptr h28(new RegisterAST(aarch64::h28));
  RegisterAST::Ptr h29(new RegisterAST(aarch64::h29));
  RegisterAST::Ptr h30(new RegisterAST(aarch64::h30));
  RegisterAST::Ptr h31(new RegisterAST(aarch64::h31));

  RegisterAST::Ptr d0 (new RegisterAST(aarch64::d0));
  RegisterAST::Ptr d1 (new RegisterAST(aarch64::d1));
  RegisterAST::Ptr d2 (new RegisterAST(aarch64::d2));
  RegisterAST::Ptr d3 (new RegisterAST(aarch64::d3));
  RegisterAST::Ptr d4 (new RegisterAST(aarch64::d4));
  RegisterAST::Ptr d5 (new RegisterAST(aarch64::d5));
  RegisterAST::Ptr d6 (new RegisterAST(aarch64::d6));
  RegisterAST::Ptr d7 (new RegisterAST(aarch64::d7));
  RegisterAST::Ptr d8 (new RegisterAST(aarch64::d8));
  RegisterAST::Ptr d9 (new RegisterAST(aarch64::d9));
  RegisterAST::Ptr d10(new RegisterAST(aarch64::d10));
  RegisterAST::Ptr d11(new RegisterAST(aarch64::d11));
  RegisterAST::Ptr d12(new RegisterAST(aarch64::d12));
  RegisterAST::Ptr d13(new RegisterAST(aarch64::d13));
  RegisterAST::Ptr d14(new RegisterAST(aarch64::d14));
  RegisterAST::Ptr d15(new RegisterAST(aarch64::d15));
  RegisterAST::Ptr d16(new RegisterAST(aarch64::d16));
  RegisterAST::Ptr d17(new RegisterAST(aarch64::d17));
  RegisterAST::Ptr d18(new RegisterAST(aarch64::d18));
  RegisterAST::Ptr d19(new RegisterAST(aarch64::d19));
  RegisterAST::Ptr d20(new RegisterAST(aarch64::d20));
  RegisterAST::Ptr d21(new RegisterAST(aarch64::d21));
  RegisterAST::Ptr d22(new RegisterAST(aarch64::d22));
  RegisterAST::Ptr d23(new RegisterAST(aarch64::d23));
  RegisterAST::Ptr d24(new RegisterAST(aarch64::d24));
  RegisterAST::Ptr d25(new RegisterAST(aarch64::d25));
  RegisterAST::Ptr d26(new RegisterAST(aarch64::d26));
  RegisterAST::Ptr d27(new RegisterAST(aarch64::d27));
  RegisterAST::Ptr d28(new RegisterAST(aarch64::d28));
  RegisterAST::Ptr d29(new RegisterAST(aarch64::d29));
  RegisterAST::Ptr d30(new RegisterAST(aarch64::d30));
  RegisterAST::Ptr d31(new RegisterAST(aarch64::d31));

  RegisterAST::Ptr b0 (new RegisterAST(aarch64::b0));
  RegisterAST::Ptr b1 (new RegisterAST(aarch64::b1));
  RegisterAST::Ptr b2 (new RegisterAST(aarch64::b2));
  RegisterAST::Ptr b3 (new RegisterAST(aarch64::b3));
  RegisterAST::Ptr b4 (new RegisterAST(aarch64::b4));
  RegisterAST::Ptr b5 (new RegisterAST(aarch64::b5));
  RegisterAST::Ptr b6 (new RegisterAST(aarch64::b6));
  RegisterAST::Ptr b7 (new RegisterAST(aarch64::b7));
  RegisterAST::Ptr b8 (new RegisterAST(aarch64::b8));
  RegisterAST::Ptr b9 (new RegisterAST(aarch64::b9));
  RegisterAST::Ptr b10(new RegisterAST(aarch64::b10));
  RegisterAST::Ptr b11(new RegisterAST(aarch64::b11));
  RegisterAST::Ptr b12(new RegisterAST(aarch64::b12));
  RegisterAST::Ptr b13(new RegisterAST(aarch64::b13));
  RegisterAST::Ptr b14(new RegisterAST(aarch64::b14));
  RegisterAST::Ptr b15(new RegisterAST(aarch64::b15));
  RegisterAST::Ptr b16(new RegisterAST(aarch64::b16));
  RegisterAST::Ptr b17(new RegisterAST(aarch64::b17));
  RegisterAST::Ptr b18(new RegisterAST(aarch64::b18));
  RegisterAST::Ptr b19(new RegisterAST(aarch64::b19));
  RegisterAST::Ptr b20(new RegisterAST(aarch64::b20));
  RegisterAST::Ptr b21(new RegisterAST(aarch64::b21));
  RegisterAST::Ptr b22(new RegisterAST(aarch64::b22));
  RegisterAST::Ptr b23(new RegisterAST(aarch64::b23));
  RegisterAST::Ptr b24(new RegisterAST(aarch64::b24));
  RegisterAST::Ptr b25(new RegisterAST(aarch64::b25));
  RegisterAST::Ptr b26(new RegisterAST(aarch64::b26));
  RegisterAST::Ptr b27(new RegisterAST(aarch64::b27));
  RegisterAST::Ptr b28(new RegisterAST(aarch64::b28));
  RegisterAST::Ptr b29(new RegisterAST(aarch64::b29));
  RegisterAST::Ptr b30(new RegisterAST(aarch64::b30));
  RegisterAST::Ptr b31(new RegisterAST(aarch64::b31));

  RegisterAST::Ptr zr (new RegisterAST(aarch64::zr));
  RegisterAST::Ptr wzr (new RegisterAST(aarch64::wzr));
  RegisterAST::Ptr sp (new RegisterAST(aarch64::sp));
  RegisterAST::Ptr wsp (new RegisterAST(aarch64::wsp));
  RegisterAST::Ptr pc (new RegisterAST(aarch64::pc));
  RegisterAST::Ptr pstate (new RegisterAST(aarch64::pstate));

  RegisterAST::Ptr pmceid0_el0(new RegisterAST(aarch64::pmceid0_el0));
  RegisterAST::Ptr pmevcntr2_el0(new RegisterAST(aarch64::pmevcntr2_el0));
  RegisterAST::Ptr cntpct_el0(new RegisterAST(aarch64::cntpct_el0));
  RegisterAST::Ptr pmevtyper30_el0(new RegisterAST(aarch64::pmevtyper30_el0));

  std::deque<registerSet> expectedRead, expectedWritten;
  registerSet tmpRead, tmpWritten;

  test_results_t retVal = PASSED;

expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w12,w10};
	tmpWritten = {w1};
#else
	tmpRead = list_of(w12)(w10);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w8,w5};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w8)(w5);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x9,x7};
	tmpWritten = {x4};
#else
	tmpRead = list_of(x9)(x7);
	tmpWritten = list_of(x4);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w4,w2};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w4)(w2);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x11,x8};
	tmpWritten = {x6};
#else
	tmpRead = list_of(x11)(x8);
	tmpWritten = list_of(x6);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w8,w5};
	tmpWritten = {w0,pstate};
#else
	tmpRead = list_of(w8)(w5);
	tmpWritten = list_of(w0)(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x15,w10};
	tmpWritten = {w5};
#else
	tmpRead = list_of(x15)(w10);
	tmpWritten = list_of(w5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w1,x1};
	tmpWritten = {x0};
#else
	tmpRead = list_of(w1)(x1);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2,x2};
	tmpWritten = {x2,pstate};
#else
	tmpRead = list_of(x2)(x2);
	tmpWritten = list_of(x2)(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w25,w22};
	tmpWritten = {w5};
#else
	tmpRead = list_of(w25)(w22);
	tmpWritten = list_of(w5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2,x1};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x2)(x1);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w7,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(w7)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x20,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(x20)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w10,w5,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(w10)(w5)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2,x4,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(x2)(x4)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w15,w10,pstate};
	tmpWritten = {w5};
#else
	tmpRead = list_of(w15)(w10)(pstate);
	tmpWritten = list_of(w5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2,x4,pstate};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x2)(x4)(pstate);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x21,x22,pstate};
	tmpWritten = {x20};
#else
	tmpRead = list_of(x21)(x22)(pstate);
	tmpWritten = list_of(x20);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w5,w10,pstate};
	tmpWritten = {w1};
#else
	tmpRead = list_of(w5)(w10)(pstate);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(w2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x20};
	tmpWritten = {x10};
#else
	tmpRead = list_of(x20);
	tmpWritten = list_of(x10);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w29};
	tmpWritten = {w30};
#else
	tmpRead = list_of(w29);
	tmpWritten = list_of(w30);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x12};
	tmpWritten = {x11};
#else
	tmpRead = list_of(x12);
	tmpWritten = list_of(x11);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w12};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w12);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w4,w2};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w4)(w2);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x25,x20};
	tmpWritten = {x15};
#else
	tmpRead = list_of(x25)(x20);
	tmpWritten = list_of(x15);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w11,w8};
	tmpWritten = {w5};
#else
	tmpRead = list_of(w11)(w8);
	tmpWritten = list_of(w5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x11,x9};
	tmpWritten = {x7};
#else
	tmpRead = list_of(x11)(x9);
	tmpWritten = list_of(x7);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w4,w0};
	tmpWritten = {w16};
#else
	tmpRead = list_of(w4)(w0);
	tmpWritten = list_of(w16);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w2,w0,w3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(w2)(w0)(w3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x16,x30,x8};
	tmpWritten = {x4};
#else
	tmpRead = list_of(x16)(x30)(x8);
	tmpWritten = list_of(x4);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1,x1,x0};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x1)(x1)(x0);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x10,x5};
	tmpWritten = {x5};
#else
	tmpRead = list_of(x10)(x5);
	tmpWritten = list_of(x5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w3,w2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(w3)(w2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x10,x5};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x10)(x5);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w2,w0};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w2)(w0);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x22,x21};
	tmpWritten = {x20};
#else
	tmpRead = list_of(x22)(x21);
	tmpWritten = list_of(x20);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1,x1};
	tmpWritten = {x1,pstate};
#else
	tmpRead = list_of(x1)(x1);
	tmpWritten = list_of(x1)(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {wsp};
	tmpWritten = {w0};
#else
	tmpRead = list_of(wsp);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w10};
	tmpWritten = {w5,pstate};
#else
	tmpRead = list_of(w10);
	tmpWritten = list_of(w5)(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x10};
	tmpWritten = {sp};
#else
	tmpRead = list_of(x10);
	tmpWritten = list_of(sp);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w14};
	tmpWritten = {w12};
#else
	tmpRead = list_of(w14);
	tmpWritten = list_of(w12);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x30);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead ={x0};
	tmpWritten = {x20};
#else
	tmpRead = list_of(x0);
	tmpWritten = list_of(x20);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w30,w20};
	tmpWritten = {w10};
#else
	tmpRead = list_of(w30)(w20);
	tmpWritten = list_of(w10);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x16,x8};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x16)(x8);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w24};
	tmpWritten = {wsp};
#else
	tmpRead = list_of(w24);
	tmpWritten = list_of(wsp);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30};
	tmpWritten = {sp};
#else
	tmpRead = list_of(x30);
	tmpWritten = list_of(sp);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x25};
	tmpWritten = {x20};
#else
	tmpRead = list_of(x25);
	tmpWritten = list_of(x20);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w10};
	tmpWritten = {w5,pstate};
#else
	tmpRead = list_of(w10);
	tmpWritten = list_of(w5)(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {w4};
#else
	tmpWritten = list_of(w4);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {x20};
#else
	tmpWritten = list_of(x20);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {x1};
#else
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {w8};
#else
	tmpWritten = list_of(w8);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {x0};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {x30};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(x30);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w15,pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(w15)(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30,pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(x30)(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc,pstate};
	tmpWritten = {pc};
#else
	tmpRead = list_of(pc)(pstate);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc,pstate};
	tmpWritten = {pc};
#else
	tmpRead = list_of(pc)(pstate);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w4,pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(w4)(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x25,pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(x25)(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {wzr,pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(wzr)(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {pc};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x12};
	tmpWritten = {pc};
#else
	tmpRead = list_of(x12);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30};
	tmpWritten = {pc};
#else
	tmpRead = list_of(x30);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x0};
	tmpWritten = {pc};
#else
	tmpRead = list_of(x0);
	tmpWritten = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s31,s0};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(s31)(s0);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s8};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(s8);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d31,d30};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(d31)(d30);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s31,s20,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(s31)(s20)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d2,d1,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(d2)(d1)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d11,d10,pstate};
	tmpWritten = {pstate};
#else
	tmpRead = list_of(d11)(d10)(pstate);
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s3,s2,pstate};
	tmpWritten = {s1};
#else
	tmpRead = list_of(s3)(s2)(pstate);
	tmpWritten = list_of(s1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s10};
	tmpWritten = {s5};
#else
	tmpRead = list_of(s10);
	tmpWritten = list_of(s5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d31};
	tmpWritten = {d31};
#else
	tmpRead = list_of(d31);
	tmpWritten = list_of(d31);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d2};
	tmpWritten = {d0};
#else
	tmpRead = list_of(d2);
	tmpWritten = list_of(d0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {h5};
	tmpWritten = {s4};
#else
	tmpRead = list_of(h5);
	tmpWritten = list_of(s4);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {h31};
	tmpWritten = {d0};
#else
	tmpRead = list_of(h31);
	tmpWritten = list_of(d0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s0};
	tmpWritten = {d2};
#else
	tmpRead = list_of(s0);
	tmpWritten = list_of(d2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d31};
	tmpWritten = {h31};
#else
	tmpRead = list_of(d31);
	tmpWritten = list_of(h31);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d1};
	tmpWritten = {s1};
#else
	tmpRead = list_of(d1);
	tmpWritten = list_of(s1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s16};
	tmpWritten = {h8};
#else
	tmpRead = list_of(s16);
	tmpWritten = list_of(h8);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s2,s1};
	tmpWritten = {s0};
#else
	tmpRead = list_of(s2)(s1);
	tmpWritten = list_of(s0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d31,d30};
	tmpWritten = {d29};
#else
	tmpRead = list_of(d31)(d30);
	tmpWritten = list_of(d29);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d15,d10};
	tmpWritten = {d5};
#else
	tmpRead = list_of(d15)(d10);
	tmpWritten = list_of(d5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s0,s16};
	tmpWritten = {s8};
#else
	tmpRead = list_of(s0)(s16);
	tmpWritten = list_of(s8);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s1,s1};
	tmpWritten = {s1};
#else
	tmpRead = list_of(s1)(s1);
	tmpWritten = list_of(s1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s2,s3,s1};
	tmpWritten = {s0};
#else
	tmpRead = list_of(s2)(s3)(s1);
	tmpWritten = list_of(s0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d8,d16,d4};
	tmpWritten = {d2};
#else
	tmpRead = list_of(d8)(d16)(d4);
	tmpWritten = list_of(d2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s11,s13,s11};
	tmpWritten = {s10};
#else
	tmpRead = list_of(s11)(s13)(s11);
	tmpWritten = list_of(s10);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d2,d1,d4};
	tmpWritten = {d8};
#else
	tmpRead = list_of(d2)(d1)(d4);
	tmpWritten = list_of(d8);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {s0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(s0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {d31};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(d31);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w30};
	tmpWritten = {s0};
#else
	tmpRead = list_of(w30);
	tmpWritten = list_of(s0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x0};
	tmpWritten = {d1};
#else
	tmpRead = list_of(x0);
	tmpWritten = list_of(d1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w10};
	tmpWritten = {s5};
#else
	tmpRead = list_of(w10);
	tmpWritten = list_of(s5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w2};
	tmpWritten = {d8};
#else
	tmpRead = list_of(w2);
	tmpWritten = list_of(d8);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s0};
	tmpWritten = {w11};
#else
	tmpRead = list_of(s0);
	tmpWritten = list_of(w11);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d31};
	tmpWritten = {x30};
#else
	tmpRead = list_of(d31);
	tmpWritten = list_of(x30);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s10};
	tmpWritten = {x1};
#else
	tmpRead = list_of(s10);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {d9};
	tmpWritten = {w9};
#else
	tmpRead = list_of(d9);
	tmpWritten = list_of(w9);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {s5};
	tmpWritten = {w8};
#else
	tmpRead = list_of(s5);
	tmpWritten = list_of(w8);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w30};
	tmpWritten = {s1};
#else
	tmpRead = list_of(w30);
	tmpWritten = list_of(s1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pstate};
#else
	tmpRead = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pstate};
#else
	tmpRead = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pstate};
#else
	tmpRead = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pstate};
#else
	tmpRead = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {pstate};
#else
	tmpWritten = list_of(pstate);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x0};
#else
	tmpRead = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {x30};
#else
	tmpWritten = list_of(x30);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {x1};
	tmpRead = {pmceid0_el0};
#else
	tmpWritten = list_of(x1);
	tmpRead = list_of(pmceid0_el0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpWritten = {x0};
	tmpRead = {pmevcntr2_el0};
#else
	tmpWritten = list_of(x0);
	tmpWritten = list_of(pmevcntr2_el0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1};
	tmpWritten = {cntpct_el0};
#else
	tmpRead = list_of(x1);
	tmpWritten = list_of(cntpct_el0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x0};
	tmpWritten = {pmevtyper30_el0};
#else
	tmpRead = list_of(x0);
	tmpWritten = list_of(pmevtyper30_el0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

  decodedInsns.pop_back();
  while(!decodedInsns.empty())
  {
      retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead.front(),
                                   expectedWritten.front()));
      decodedInsns.pop_front();

  	  expectedRead.pop_front();
      expectedWritten.pop_front();
  }

  return retVal;
}

