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
	    cout<<decodedInsns.back()->format()<<endl;
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
	tmpWritten = {d5};
    #else
	tmpWritten = list_of(d5);
    #endif
    expectedRead.push_back(tmpRead);
    expectedWritten.push_back(tmpWritten);
    tmpRead.clear();
    tmpWritten.clear();

    //orr
    #if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
	tmpWritten = {d5};
    #else
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
	tmpWritten = {q8};
    #else
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
