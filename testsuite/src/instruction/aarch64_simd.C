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

class aarch64_simd_Mutator : public InstructionMutator {
    private:
	void setupRegisters();
	void reverseBuffer(const unsigned char  *, int);
    public:
	aarch64_simd_Mutator() { };
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* aarch64_simd_factory()
{
    return new aarch64_simd_Mutator();
}

void aarch64_simd_Mutator::reverseBuffer(const unsigned char *buffer, int bufferSize)
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

test_results_t aarch64_simd_Mutator::executeTest()
{
    const unsigned char buffer[] = 
    {
	0x4E, 0x30, 0x38, 0x20,	    //SADDLV  H0, Q1.8B
	0x0E, 0x30, 0xA9, 0x0F,	    //SMAXV   B15, D8
	0x6E, 0x70, 0x38, 0x42,	    //UADDLV  S2, Q2
	0x2E, 0x31, 0xA8, 0x05,	    //UMINV   B5, D0
	0x6E, 0x30, 0xC8, 0x25,	    //FMAXNMV S5, Q1
	0x0E, 0x11, 0x04, 0x41,	    //DUP     D1, Q2
	0x4E, 0x02, 0x0C, 0xD8,	    //DUP     Q20, W5
	0x0E, 0x18, 0x0C, 0x40,	    //DUP     D0, X2
	0x4E, 0x08, 0x1C, 0x04,	    //INS     Q4, X0
	0x4E, 0x01, 0x1F, 0xFF,	    //INS     Q31, WZR
	0x0E, 0x05, 0x2C, 0xA1,	    //SMOV    W1, D5
	0x4E, 0x10, 0x3C, 0x42,	    //UMOV    X2, Q2
	0x6E, 0x01, 0x04, 0x00,	    //INS     Q0, D0
	0x2E, 0x02, 0x28, 0x20,	    //EXT     D0, D1, D2, #5
	0x6E, 0x02, 0x7A, 0x08,	    //EXT     Q8, Q16, Q2, #15
	0x0F, 0x00, 0xE5, 0x00,	    //MOVI    D0, #8
	0x4F, 0x07, 0xA7, 0xE1,	    //MOVI    Q1, FF LSL #8
	0x0F, 0x00, 0x74, 0x05,	    //ORR     D5, #0 LSL #24         ****
	0x0F, 0x00, 0xB4, 0x05,	    //ORR     D5, #0 LSL #8          ****
	0x4F, 0x07, 0xF7, 0xE8,	    //FMOV    Q8, FF                 ****
	0x6F, 0x00, 0xC5, 0x80,	    //MVNI    Q0, #12 LSL #8         ****
	0x6F, 0x00, 0x97, 0x08,	    //BIC     D8, #24 LSL #0         ****
	0x2F, 0x07, 0xE7, 0xE2,	    //MOVI    D2, (all ones)
	0x0E, 0x00, 0x1A, 0x08,	    //UZP1    D8, D16, D0
	0x4E, 0x04, 0x28, 0x62,	    //TRN1    Q2, Q3, Q4
	0x0E, 0x08, 0x79, 0x08,	    //ZIP2    D8, D8, D8
	0x5E, 0x11, 0x04, 0x25,	    //DUP     B5, Q1
	0x5E, 0x08, 0x05, 0x08,	    //DUP     D8, D8
	0x5E, 0xF1, 0xB8, 0xA2,	    //ADDP    D2, Q5
	0x7E, 0x30, 0xC8, 0x81,	    //FMAXNMP S1, D4
	0x7E, 0xF0, 0xC9, 0xEF,	    //FMINNMP D15, Q15
	0x5F, 0x78, 0x04, 0x82,	    //SSHR    D2, D4, #8	
	0x5F, 0x40, 0x14, 0x20,	    //SSRA    D0, D1, #64
	0x5F, 0x41, 0x57, 0xFF,	    //SHL     D31, D31, #1
	0x5F, 0x0E, 0x76, 0x08,	    //SQSHL   B8, B16, 6
	0x5F, 0x10, 0x9D, 0x02,	    //SQRSHRN H2, H8, #16
	0x5F, 0x20, 0xFC, 0x00,	    //FCVTZS  S0, S0, #32
	0x7F, 0x48, 0x55, 0x04,	    //SLI     D4, D8, #8
	0x7F, 0x09, 0x94, 0x42,	    //UQSHRN  B2, H2, #7
	0x5E, 0x60, 0x91, 0x08,	    //SQDMLAL S8, H8, H0
	0x5E, 0xA1, 0xB0, 0x82,	    //SQDMLSL D2, S4, S1
	0x5E, 0x7F, 0xD0, 0x00,	    //SQDMULL S0, H0, H31
	0x5E, 0xE0, 0x34, 0x22,	    //CMGT    D2, D1, D0
	0x5E, 0x64, 0x4C, 0x40,	    //SQSHL   H0, H2, H4
	0x5E, 0xA0, 0xB4, 0x3F,	    //SQDMULH S31, S1, S0
	0x5E, 0x68, 0xFD, 0x08,	    //FRECPS  D8, D8, D8
	0x7E, 0xE2, 0x57, 0xE4,	    //URSHL   D4, D31, D2
	0x7E, 0xB0, 0xE5, 0x02,	    //FCMGT   S2, S8, S16
	0x5E, 0x20, 0x39, 0x02,	    //SUQADD  B2, B8
	0x5E, 0x61, 0x48, 0x04,	    //SQXTN   H4, S0
	0x5E, 0x61, 0xB8, 0x3F,	    //FCVTMS  D31, D1
	0x5E, 0xA1, 0xD8, 0xA9,	    //FRECPE  S9, S5
	0x7E, 0x21, 0x28, 0x48,	    //SQXTUN  B8, H2
	0x7E, 0x61, 0x68, 0x4F,	    //FCVTXN  S15, D2
	0x7E, 0xA1, 0xDB, 0xFF,	    //FRSQRTE S31, S31
	0x5F, 0x49, 0x38, 0xA2,	    //SQDMLAL S2, H5, Q9
	0x5F, 0x9f, 0x73, 0xE0,	    //SQDMLSL D0, S31, D31
	0x5F, 0x4F, 0xD0, 0x88,	    //SQRDMULH H8, H4, D15
	0x5F, 0x94, 0x18, 0x49,	    //FMLA    S9, S2, Q20
	0x5F, 0xD1, 0x50, 0xA8,	    //FMLS    D8, D5, D17
	0x4E, 0x09, 0x01, 0x04,	    //TBL     Q4, Q8, Q9
	0x0E, 0x03, 0x30, 0x20,	    //TBX     D0, D1, D2, D3
	0x4E, 0x1F, 0x43, 0x00,	    //TBL     Q0, Q24, Q25, Q26, Q31
	0x0E, 0x15, 0x70, 0x14,	    //TBX     D20, D0, D1, D2, D3, D21
	0x4E, 0x23, 0x00, 0x41,	    //SADDL   Q1, HQ2, HQ3
	0x0E, 0x25, 0x20, 0x9F,	    //SSUBL   Q31, D4, D5
	0x4E, 0x30, 0x41, 0x02,	    //ADDHN   HQ2, Q8, Q16
	0x0E, 0x68, 0x70, 0x45,	    //SABDL   Q5, D2, D8
	0x4E, 0xA2, 0xB0, 0x20,	    //SQDMLSL Q0, HQ1, HQ2
	0x2E, 0x3F, 0x61, 0xE2,	    //RSUBHN  D2, Q15, Q31
	0x4E, 0xE3, 0x60, 0x48,	    //SUBHN   HQ8, Q2,Q3
	0x0E, 0x25, 0x0C, 0x62,	    //SQADD   D2, D3, D5
	0x4E, 0x3F, 0x34, 0x20,	    //CMGT    Q0, Q1, Q31
	0x0E, 0x2A, 0x7D, 0x28,	    //SABA    D8, D9, D10
	0x6E, 0x66, 0x1C, 0xA4,	    //BSL     Q4, Q5, Q6
	0x0E, 0x20, 0x59, 0x28,	    //CNT     D8, D9
	0x4E, 0x21, 0x2B, 0xE0,	    //XTN     Q0, Q31
	0x4E, 0x61, 0xC8, 0x62,	    //FCVTAS  Q2, Q3
	0x2E, 0xA1, 0xF9, 0xFF,	    //FSQRT   D31, D15
	0x0F, 0x40, 0x20, 0xA2,	    //SMLAL   Q2, D5, D0
	0x4F, 0x85, 0x6B, 0xE1,	    //SMLSL   Q1, HQ31, Q5
	0x4F, 0x9F, 0x80, 0x49,	    //MUL     Q9, Q2, D31
	0x0F, 0x45, 0xB8, 0x88,	    //SQDMULL Q8, D4, Q5
	0x0F, 0x83, 0xD0, 0x41,	    //SQRDMULH D1. D2, D3
	0x4F, 0x8B, 0x59, 0x45,	    //FMLS    Q5, Q10, Q11
	0x6F, 0x9F, 0x21, 0x04,	    //UMLAL   Q4, HQ8, D31
	0x0F, 0x88, 0x19, 0x84,	    //FMLA    D4, D12, Q8
	0x4C, 0x00, 0x71, 0x04,	    //ST1     Q4, [X8]
	0x0C, 0x00, 0x83, 0xE9,	    //ST2     D9, D10, [SP]
	0x4C, 0x00, 0xA3, 0xC2,	    //ST1     Q2, Q3, [X30]
	0x4C, 0x40, 0x63, 0xFE,	    //LD1     Q30, Q31, Q0, [SP]
	0x0C, 0x40, 0x20, 0x01,	    //LD1     D1, D2, D3, D4, [X0]
	0x4C, 0x40, 0x40, 0x25,	    //LD3     Q5, Q6, Q7, [X1]
	0x0C, 0x94, 0x00, 0x40,	    //ST4     D0, D1, D2, D3, [X2], X20
	0x4C, 0x85, 0x23, 0xFE,	    //ST1     Q30, Q31, Q0, Q1, [SP], X5	
	0x4C, 0x87, 0x61, 0x04,	    //ST1     Q4, Q5, Q6, [X8], X7
	0x0C, 0xDE, 0xA3, 0xFF,	    //LD1     D31, D0, [SP], X30
	0x4C, 0xC0, 0x71, 0x45,	    //LD1     Q5, [X10], X0
	0x0C, 0x9F, 0x43, 0xE5,	    //ST3     D5, D6, D7, [SP], #24
	0x4C, 0x9F, 0x20, 0x5F,	    //ST1     Q31, Q0, Q1, Q2, [X2], #48
	0x4C, 0xDF, 0x63, 0xE1,	    //LD1     Q1, Q2, Q3, [SP], #48
	0x0C, 0xDF, 0xA3, 0xDF,	    //LD1     D31, D0, [X30], #16
	0x4C, 0x9F, 0x70, 0x04,	    //ST1     Q4, [X0], #16
	0x4D, 0x00, 0x00, 0x45,	    //ST1     Q5, [X2]
	0x0D, 0x20, 0x43, 0xE4,	    //ST2     D4, D5, [SP]
	0x4D, 0x00, 0xA1, 0x1F,	    //ST3     Q31, Q0, Q1, [X8]
	0x0D, 0x20, 0xA7, 0xE0,	    //ST4     D0, D1, D2, D3, [SP]
	0x4D, 0x20, 0x43, 0xD5,	    //ST2     Q21, Q22, [X30]
	0x0D, 0x20, 0xA4, 0x1E,	    //ST4     D30, D31, D0, D1, [X0]
	0x4D, 0x40, 0x00, 0x45,	    //LD1     Q5, [X2]
	0x0D, 0x60, 0x43, 0xE4,	    //LD2     D4, D5, [SP]
	0x4D, 0x40, 0xA1, 0x1F,	    //LD3     Q31, Q0, Q1, [X8]
	0x0D, 0x60, 0xA7, 0xE0,	    //LD4     D0, D1, D2, D3, [SP]
	0x0D, 0x40, 0x80, 0x09,	    //LD1     D9, [X0]
	0x4D, 0x40, 0xA4, 0x21,	    //LD3     Q1, Q2, Q3, [X1]
	0x4D, 0x88, 0x00, 0x45,	    //ST1     Q5, [X2], X8
	0x0D, 0xBF, 0x43, 0xE4,	    //ST2     D4, D5, [SP], #4
	0x4D, 0x9E, 0xA1, 0x1F,	    //ST3     Q31, Q0, Q1, [X8], X30
	0x0D, 0xBF, 0xA7, 0xE0,	    //ST4     D0, D1, D2, D3, [SP], #32
	0x4D, 0xDF, 0x00, 0x45,	    //LD1     Q5,[X2], #1
	0x0D, 0xE0, 0x43, 0xE4,	    //LD2     D4, D5, [SP], X0
	0x4D, 0xDF, 0xA1, 0x1F,	    //LD3     Q31, Q0, Q1, [X8], #12
	0x0D, 0xE9, 0xA7, 0xE0,	    //LD4     D0, D1, D2, D3, [SP], X9
	0x0F, 0x08, 0x04, 0x82,	    //SSHR    D2, D4, #8
	0x4F, 0x40, 0x14, 0x20,	    //SSRA    Q0, Q1, #64
	0x0F, 0x09, 0x57, 0xFF,	    //SHL     D31, D31, #1
	0x4F, 0x30, 0x76, 0x08,	    //SQSHL   Q8, Q16, #16
	0x0F, 0x10, 0x9D, 0x02,	    //SQRSHRN D2, D8, #16
	0x4F, 0x20, 0xFC, 0x00,	    //FCVTZS  Q0, Q0, #32
	0x2F, 0x28, 0x55, 0x04,	    //SLI     D4, D8, #8
	0x6F, 0x09, 0x94, 0xA2,	    //UQSHRN  Q2, Q6, #7
	0x00, 0x00, 0x00, 0x00
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
	    /*cout<<*/decodedInsns.back()->format()/*<<endl*/;
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

    RegisterAST::Ptr zr (new RegisterAST(aarch64::zr));
    RegisterAST::Ptr wzr (new RegisterAST(aarch64::wzr));

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

    RegisterAST::Ptr hq0 (new RegisterAST(aarch64::hq0));
    RegisterAST::Ptr hq1 (new RegisterAST(aarch64::hq1));
    RegisterAST::Ptr hq2 (new RegisterAST(aarch64::hq2));
    RegisterAST::Ptr hq3 (new RegisterAST(aarch64::hq3));
    RegisterAST::Ptr hq4 (new RegisterAST(aarch64::hq4));
    RegisterAST::Ptr hq5 (new RegisterAST(aarch64::hq5));
    RegisterAST::Ptr hq6 (new RegisterAST(aarch64::hq6));
    RegisterAST::Ptr hq7 (new RegisterAST(aarch64::hq7));
    RegisterAST::Ptr hq8 (new RegisterAST(aarch64::hq8));
    RegisterAST::Ptr hq9 (new RegisterAST(aarch64::hq9));
    RegisterAST::Ptr hq10(new RegisterAST(aarch64::hq10));
    RegisterAST::Ptr hq11(new RegisterAST(aarch64::hq11));
    RegisterAST::Ptr hq12(new RegisterAST(aarch64::hq12));
    RegisterAST::Ptr hq13(new RegisterAST(aarch64::hq13));
    RegisterAST::Ptr hq14(new RegisterAST(aarch64::hq14));
    RegisterAST::Ptr hq15(new RegisterAST(aarch64::hq15));
    RegisterAST::Ptr hq16(new RegisterAST(aarch64::hq16));
    RegisterAST::Ptr hq17(new RegisterAST(aarch64::hq17));
    RegisterAST::Ptr hq18(new RegisterAST(aarch64::hq18));
    RegisterAST::Ptr hq19(new RegisterAST(aarch64::hq19));
    RegisterAST::Ptr hq20(new RegisterAST(aarch64::hq20));
    RegisterAST::Ptr hq21(new RegisterAST(aarch64::hq21));
    RegisterAST::Ptr hq22(new RegisterAST(aarch64::hq22));
    RegisterAST::Ptr hq23(new RegisterAST(aarch64::hq23));
    RegisterAST::Ptr hq24(new RegisterAST(aarch64::hq24));
    RegisterAST::Ptr hq25(new RegisterAST(aarch64::hq25));
    RegisterAST::Ptr hq26(new RegisterAST(aarch64::hq26));
    RegisterAST::Ptr hq27(new RegisterAST(aarch64::hq27));
    RegisterAST::Ptr hq28(new RegisterAST(aarch64::hq28));
    RegisterAST::Ptr hq29(new RegisterAST(aarch64::hq29));
    RegisterAST::Ptr hq30(new RegisterAST(aarch64::hq30));
    RegisterAST::Ptr hq31(new RegisterAST(aarch64::hq31));
    
    RegisterAST::Ptr sp(new RegisterAST(aarch64::sp));

    std::deque<registerSet> expectedRead, expectedWritten;
    registerSet tmpRead, tmpWritten;

    //saddlv
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q1};
	tmpWritten = {h0};
    #else
	tmpRead = list_of(q1);
	tmpWritten = list_of(h0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //smaxv
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {b15};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(b15);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //uaddlv
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q2};
	tmpWritten = {s2};
    #else
	tmpRead = list_of(q2);
	tmpWritten = list_of(s2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //uminv
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d0};
	tmpWritten = {b5};
    #else
	tmpRead = list_of(d0);
	tmpWritten = list_of(b5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmaxnmv
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q1};
	tmpWritten = {s5};
    #else
	tmpRead = list_of(q1);
	tmpWritten = list_of(s5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();
    
    //dup
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q2};
	tmpWritten = {d1};
    #else
	tmpRead = list_of(q2);
	tmpWritten = list_of(d1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //dup 
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {w6};
	tmpWritten = {q24};
    #else
	tmpRead = list_of(w6);
	tmpWritten = list_of(q24);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //dup
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {x2};
	tmpWritten = {d0};
    #else
	tmpRead = list_of(x2);
	tmpWritten = list_of(d0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ins
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {x0};
	tmpWritten = {q4};
    #else
	tmpRead = list_of(x0);
	tmpWritten = list_of(q4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ins
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {wzr};
	tmpWritten = {q31};
    #else
	tmpRead = list_of(wzr);
	tmpWritten = list_of(q31);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //smov
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d5};
	tmpWritten = {w1};
    #else
	tmpRead = list_of(d5);
	tmpWritten = list_of(w1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //umov
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q2};
	tmpWritten = {x2};
    #else
	tmpRead = list_of(q2);
	tmpWritten = list_of(x2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ins
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d0};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(d0);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ext
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d1, d2};
	tmpWritten = {d0};
    #else
	tmpRead = list_of(d1)(d2);
	tmpWritten = list_of(d0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ext
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q16, q2};
	tmpWritten = {q8};
    #else
	tmpRead = list_of(q16)(q2);
	tmpWritten = list_of(q8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //movi
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d0};
    #else
	tmpWritten = list_of(d0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //movi
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q1};
    #else
	tmpWritten = list_of(q1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //orr
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d5};
	tmpWritten = {d5};
    #else
	tmpRead = list_of(d5);
	tmpWritten = list_of(d5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //orr
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d5};
	tmpWritten = {d5};
    #else
	tmpRead = list_of(d5);
	tmpWritten = list_of(d5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmov
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q8};
    #else
	tmpWritten = list_of(q8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //mvni
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q0};
    #else
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //bic
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q8};
	tmpWritten = {q8};
    #else
	tmpRead = list_of(q8);
	tmpWritten = list_of(q8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //movi
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d2};
    #else
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //uzp1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d16, d0};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d16)(d0);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //trn1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q3, q4};
	tmpWritten = {q2};
    #else
	tmpRead = list_of(q3)(q4);
	tmpWritten = list_of(q2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //zip2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //dup
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q1};
	tmpWritten = {b5};
    #else
	tmpRead = list_of(q1);
	tmpWritten = list_of(b5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //dup
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //addp
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q5};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(q5);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmaxnmp
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4};
	tmpWritten = {s1};
    #else
	tmpRead = list_of(d4);
	tmpWritten = list_of(s1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fminnmp
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q15};
	tmpWritten = {d15};
    #else
	tmpRead = list_of(q15);
	tmpWritten = list_of(d15);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sshr
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(q4);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ssra
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d1};
	tmpWritten = {d0};
    #else
	tmpRead = list_of(d1);
	tmpWritten = list_of(d0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //shl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
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

    //sqshl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {b16};
	tmpWritten = {b8};
    #else
	tmpRead = list_of(b16);
	tmpWritten = list_of(b8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqrshrn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s8};
	tmpWritten = {h2};
    #else
	tmpRead = list_of(s8);
	tmpWritten = list_of(h2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fcvtzs
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s0};
	tmpWritten = {s0};
    #else
	tmpRead = list_of(s0);
	tmpWritten = list_of(s0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sli
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {d4};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(d4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //uqshrn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {h2};
	tmpWritten = {b2};
    #else
	tmpRead = list_of(h2);
	tmpWritten = list_of(b2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmlal
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {h8, h0};
	tmpWritten = {s8};
    #else
	tmpRead = list_of(h8)(h0);
	tmpWritten = list_of(s8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmlsl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s4, s1};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(s4)(s1);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmull
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {h0, h31};
	tmpWritten = {s0};
    #else
	tmpRead = list_of(h0)(h31);
	tmpWritten = list_of(s0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //cmgt
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d1, d0};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(d1)(d0);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqshl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {h2, h4};
	tmpWritten = {h0};
    #else
	tmpRead = list_of(h2)(h4);
	tmpWritten = list_of(h0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmulh
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s1, s0};
	tmpWritten = {s31};
    #else
	tmpRead = list_of(s0)(s1);
	tmpWritten = list_of(s31);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //frecps
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //urshl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d31, d2};
	tmpWritten = {d4};
    #else
	tmpRead = list_of(d2)(d31);
	tmpWritten = list_of(d4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fcmgt
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s8, s16};
	tmpWritten = {s2};
    #else
	tmpRead = list_of(s8)(s16);
	tmpWritten = list_of(s2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //suqadd
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {b8};
	tmpWritten = {b2};
    #else
	tmpRead = list_of(b8);
	tmpWritten = list_of(b2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqxtn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s0};
	tmpWritten = {h4};
    #else
	tmpRead = list_of(s0);
	tmpWritten = list_of(h4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fcvtms
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d1};
	tmpWritten = {d31};
    #else
	tmpRead = list_of(d1);
	tmpWritten = list_of(d31);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //frecpe
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s5};
	tmpWritten = {s9};
    #else
	tmpRead = list_of(s5);
	tmpWritten = list_of(s9);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqxtun
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {h2};
	tmpWritten = {b8};
    #else
	tmpRead = list_of(h2);
	tmpWritten = list_of(b8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fcvtxn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d2};
	tmpWritten = {s15};
    #else
	tmpRead = list_of(d2);
	tmpWritten = list_of(s15);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //frsqrte
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s31};
	tmpWritten = {s31};
    #else
	tmpRead = list_of(s31);
	tmpWritten = list_of(s31);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmlal
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s2, h5, q9};
	tmpWritten = {s2};
    #else
	tmpRead = list_of(s2)(h5)(q9);
	tmpWritten = list_of(s2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmlsl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d0, d31, s31};
	tmpWritten = {d0};
    #else
	tmpRead = list_of(d0)(s31)(s31);
	tmpWritten = list_of(d0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqrdmulh
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {h4, d15};
	tmpWritten = {h8};
    #else
	tmpRead = list_of(h4)(d15);
	tmpWritten = list_of(h8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmla
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {s2, s9, q20};
	tmpWritten = {s9};
    #else
	tmpRead = list_of(s2)(s9)(q20);
	tmpWritten = list_of(s9);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmls
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d5, d8, d17};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d5)(d6)(d17);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //tbl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q8, q9};
	tmpWritten = {q4};
    #else
	tmpRead = list_of(q8)(q9);
	tmpWritten = list_of(q4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //tbx
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d1, d2, d3};
	tmpWritten = {d0};
    #else
	tmpRead = list_of(d1)(d2)(d3);
	tmpWritten = list_of(d0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //tbl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q24, q25, q26, q31};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(q24)(q25)(q26)(q31);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //tbx
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d0, d1, d2, d3, d21};
	tmpWritten = {d20};
    #else
	tmpRead = list_of(d0)(d1)(d2)(d3)(d21);
	tmpWritten = list_of(d20);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //saddl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {hq2, hq3};
	tmpWritten = {q1};
    #else
	tmpRead = list_of(hq2)(hq3);
	tmpWritten = list_of(q1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ssubl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4, d5};
	tmpWritten = {q31};
    #else
	tmpRead = list_of(d4)(d5);
	tmpWritten = list_of(q31);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //addhn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q8, q16};
	tmpWritten = {hq2};
    #else
	tmpRead = list_of(q8)(q16);
	tmpWritten = list_of(hq2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sabdl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d2, d8};
	tmpWritten = {q5};
    #else
	tmpRead = list_of(d2)(d8);
	tmpWritten = list_of(q5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmlsl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {hq1, hq2};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(hq1)(hq2);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //rsubhn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q15, q31};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(q15)(q31);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //subhn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q2, q3};
	tmpWritten = {hq8};
    #else
	tmpRead = list_of(q2)(q3);
	tmpWritten = list_of(hq8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqadd
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d3, d5};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(d3)(d5);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //cmgt
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q1, q31};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(q1)(q31);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //saba
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d9, d10};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d9)(d10);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //bsl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q5, q6};
	tmpWritten = {q4};
    #else
	tmpRead = list_of(q5)(q6);
	tmpWritten = list_of(q4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //cnt
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d9};
	tmpWritten = {d8};
    #else
	tmpRead = list_of(d9);
	tmpWritten = list_of(d8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //xtn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q31};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(q31);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fcvtas
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q3};
	tmpWritten = {q2};
    #else
	tmpRead = list_of(q3);
	tmpWritten = list_of(q2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fsqrt
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d15};
	tmpWritten = {d31};
    #else
	tmpRead = list_of(d15);
	tmpWritten = list_of(d31);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //smlal
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d5, d0, q2};
	tmpWritten = {q2};
    #else
	tmpRead = list_of(d5)(d0(q2);
	tmpWritten = list_of(q2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //smlsl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q1, hq31, q5};
	tmpWritten = {q1};
    #else
	tmpRead = list_of(q1)(hq31)(q5);
	tmpWritten = list_of(q1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //mul
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q2, d31};
	tmpWritten = {q9};
    #else
	tmpRead = list_of(q2)(d31);
	tmpWritten = list_of(q9);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqdmull
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4, q5};
	tmpWritten = {q8};
    #else
	tmpRead = list_of(d4)(q5);
	tmpWritten = list_of(q8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqrdmulh
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d2, d3};
	tmpWritten = {d1};
    #else
	tmpRead = list_of(d2)(d3);
	tmpWritten = list_of(d1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmls
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q5, q10, q11};
	tmpWritten = {q5};
    #else
	tmpRead = list_of(q5)(q10)(q11);
	tmpWritten = list_of(q5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //umlal
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q4, hq8, d31};
	tmpWritten = {q4};
    #else
	tmpRead = list_of(q4)(hq8)(d31);
	tmpWritten = list_of(q4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fmla
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4, d12, q8};
	tmpWritten = {d4};
    #else
	tmpRead = list_of(d4)(d12)(q8);
	tmpWritten = list_of(d4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q4, x8};
    #else
	tmpRead = list_of(x8)(q4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d9, d10, sp};
    #else
	tmpRead = list_of(d9)(d10)(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q2, q3, x30};
    #else
	tmpRead = list_of(q2)(q3)(x30);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q0, q30, q31};
	tmpRead = {sp};
    #else
	tmpWritten = list_of(q0)(q30)(q31);
	tmpRead = list_of(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d1, d2, d3, d4};
	tmpRead = {x0};
    #else
	tmpWritten = list_of(d1)(d2)(d3)(d4);
	tmpRead = list_of(x0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q5, q6, q7};
	tmpRead = {x1};
    #else
	tmpWritten = list_of(q5)(q6)(q7);
	tmpRead = list_of(x1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st4
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {x2};
	tmpRead = {x2, x20, d0, d1, d2, d3};
    #else
	tmpWritten = list_of(x2);
	tmpRead = list_of(x2)(x20)(d0)(d1)(d2)(d3);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {sp};
	tmpRead = {q30, q31, q0, q1, sp, x5};
    #else
	tmpWritten = list_of(sp);
	tmpRead = list_of(q30)(q31)(q0)(q1)(sp)(x5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {x8};
	tmpRead = {q4, q5, q6, x7, x8};
    #else
	tmpWritten = list_of(x8);
	tmpRead = list_of(q4)(q5)(q6)(x7)(x8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d0, d31, sp};
	tmpRead = {sp, x30};
    #else
	tmpWritten = list_of(sp)(d0)(d31);
	tmpRead = list_of(sp)(x30);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q5, x10};
	tmpRead = {x10, x0};
    #else
	tmpWritten = list_of(x10)(q5);
	tmpRead = list_of(q5)(x10)(x0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {sp};
	tmpRead = {d5, d6, d7, sp};
    #else
	tmpWritten = list_of(sp);
	tmpRead = list_of(d5)(d6)(d7)(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {x2};
	tmpRead = {q0, q1, q2, q31, x2};
    #else
	tmpWritten = list_of(x2);
	tmpRead = list_of(q0)(q1)(q2)(q31)(x2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q1, q2, q3, sp};
	tmpRead = {sp};
    #else
	tmpWritten = list_of(q1)(q2)(q3);
	tmpRead = list_of(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d0, d31, x30};
	tmpRead = {x30};
    #else
	tmpWritten = list_of(d0)(d31)(x30);
	tmpRead = list_of(x30);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {x0};
	tmpRead = {q4, x0};
    #else
	tmpWritten = list_of(x0);
	tmpRead = list_of(q4)(x0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q5, x2};
    #else
	tmpRead = list_of(q5)(x2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4, d5, sp};
    #else
	tmpRead = list_of(d4)(d5)(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q0, q1, q31, x8};
    #else
	tmpRead = list_of(q0)(q1)(q31)(x8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st4
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d0, d1, d2, d3, sp};
    #else
	tmpRead = list_of(d0)(d1)(d2)(d3)(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q21, q22, x30};
    #else
	tmpRead = list_of(q21)(q22)(x30);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st4
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d0, d1, d30, d31, x0};
    #else
	tmpRead = list_of(d0)(d1)(d30)(d31)(x0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q5};
	tmpRead = {x2};
    #else
	tmpWritten = list_of(q5);
	tmpRead = list_of(x2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d4, d5};
	tmpRead = {sp};
    #else
	tmpWritten = list_of(d4)(d5);
	tmpRead = list_of(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q0, q1, q31};
	tmpRead = {x8};
    #else
	tmpWritten = list_of(q0)(q1)(q31);
	tmpRead = list_of(x8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld4
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d0, d1, d2, d3};
	tmpRead = {sp};
    #else
	tmpWritten = list_of(d0)(d1)(d2)(d3);
	tmpRead = list_of(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d9};
	tmpRead = {x0};
    #else
	tmpWritten = list_of(d9);
	tmpRead = list_of(x0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q1, q2, q3};
	tmpRead = {x1};
    #else
	tmpWritten = list_of(q1)(q2)(q3);
	tmpRead = list_of(x1);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {x2};
	tmpRead = {q5, x2, x8};
    #else
	tmpWritten = list_of(x2);
	tmpRead = list_of(q5)(x8)(x2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {sp};
	tmpRead = {d4, d5, sp};
    #else
	tmpWritten = list_of(sp);
	tmpRead = list_of(d4)(d5)(sp);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {x8};
	tmpRead = {q0, q1, q31, x8, x30};
    #else
	tmpWritten = list_of(x8);
	tmpRead = list_of(x8)(q0)(q1)(q31)(x30);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //st4
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {sp};
	tmpRead = {d0, d1, d2, d3, sp};
    #else
	tmpWritten = list_of(sp);
	tmpRead = list_of(sp)(d0)(d1)(d2)(d3);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld1
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q5, x2};
	tmpRead = {x2};
    #else
	tmpWritten = list_of(q5)(x2);
	tmpRead = list_of(x2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld2
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {sp, d4, d5};
	tmpRead = {sp, x0};
    #else
	tmpWritten = list_of(d4)(d5)(sp);
	tmpRead = list_of(sp)(x0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld3
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {q0, q1, q31, x8};
	tmpRead = {x8};
    #else
	tmpWritten = list_of(q0)(q1)(q31)(x8);
	tmpRead = list_of(x8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ld4
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d0, d1, d2, d3, sp};
	tmpRead = {sp, x9};
    #else
	tmpWritten = list_of(d0)(d1)(d2)(d3)(sp);
	tmpRead = list_of(sp)(x9);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sshr
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d4};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(d4);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //ssra
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q1};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(q1);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //shl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
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

    //sqshl
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q16};
	tmpWritten = {q8};
    #else
	tmpRead = list_of(q16);
	tmpWritten = list_of(q8);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sqrshrn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {d2};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(d2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //fcvtzs
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q0};
	tmpWritten = {q0};
    #else
	tmpRead = list_of(q0);
	tmpWritten = list_of(q0);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //sli
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {d8};
	tmpWritten = {d4};
    #else
	tmpRead = list_of(d8);
	tmpWritten = list_of(d4);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //uqshrn
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpRead = {q5};
	tmpWritten = {q2};
    #else
	tmpRead = list_of(q5);
	tmpWritten = list_of(q2);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    test_results_t retVal = PASSED;
    decodedInsns.pop_back();
    while(!decodedInsns.empty())
    {
	retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead.front(), expectedWritten.front()));
	decodedInsns.pop_front();

	expectedRead.pop_front();
	expectedWritten.pop_front();
    }

    return retVal;
}
