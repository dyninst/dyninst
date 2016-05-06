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

class aarch64_decode_ldst_Mutator : public InstructionMutator {
private:
	void setupRegisters();
	void reverseBuffer(const unsigned char *, int);
public:
    aarch64_decode_ldst_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* aarch64_decode_ldst_factory()
{
   return new aarch64_decode_ldst_Mutator();
}

void aarch64_decode_ldst_Mutator::reverseBuffer(const unsigned char *buffer, int bufferSize)
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

test_results_t aarch64_decode_ldst_Mutator::executeTest()
{
  const unsigned char buffer[] =
  {
    

    //literal
    0x58, 0x00, 0x00, 0x21,         // ldr x1,  #4
    0x58, 0x00, 0x08, 0x01,         // ldr x1,  #256
    0x18, 0x00, 0x00, 0x21,         // ldr w1,  #4
    0x18, 0x00, 0x08, 0x01,         // ldr w1,  #1048576

    //0xd5,   0x03,   0x20,   0x1f,        //nop

    //post-inc
    0xf8, 0x40, 0x14, 0x41,         // ldr,     x1, [x2], #1
    0xf8, 0x4f, 0xf4, 0x41,         // ldr,     x1, [x2], #255
    0x38, 0x40, 0x14, 0x41,         // ldrb,    w1, [x2], #1
    0x38, 0xc0, 0x14, 0x41,         // ldrsb,   w1, [x2], #1
    0x78, 0x40, 0x14, 0x41,         // ldrh,    w1, [x2], #1
    0x78, 0xc0, 0x14, 0x41,         // ldrsh,    x1, [x2], #1

    //imm
    0xf9, 0x40, 0x04, 0x41,         // ldr,     x1, [x2, #8]
    0xf9, 0x40, 0x80, 0x41,         // ldr,     x1, [x2, #256]
    0x39, 0x40, 0x10, 0x41,         // ldrb,    w1, [x2, #4]
    0x39, 0xc0, 0x10, 0x41,         // ldrsb,   w1, [x2, #4]
    0x79, 0x40, 0x08, 0x41,         // ldrh,    w1, [x2, #4]
    0x79, 0xc0, 0x08, 0x41,         // ldrsh,   w1, [x2, #4]
    0xb9, 0x80, 0x04, 0x41,         // ldrsw,   x1, [x2, #4]

    //register off not ext
    0xf8, 0x63, 0x68, 0x41,         // ldr,     x1, [x2, x3]
    0x38, 0x63, 0x68, 0x41,         // ldrb,    w1, [x2, x3]
    0x38, 0xe3, 0x68, 0x41,         // ldrsb,   w1, [x2, x3]
    0x78, 0x63, 0x68, 0x41,         // ldrh,    w1, [x2, x3]
    0x78, 0xe3, 0x68, 0x41,         // ldrsh,   w1, [x2, x3]
    0xb8, 0xa3, 0x68, 0x41,         // ldrsw,    x1, [x2, x3]

    //pre-inc
    0xf8, 0x40, 0x1c, 0x41,         // ldr,     x1, [x2, #1]!
    0xf8, 0x4f, 0xfc, 0x41,         // ldr,     x1, [x2, #255]!
    0x38, 0x40, 0x1c, 0x41,         // ldrb,    w1, [x2, #1]!
    0x38, 0xc0, 0x1c, 0x41,         // ldrsb,   w1, [x2, #1]!
    0x78, 0x40, 0x1c, 0x41,         // ldrh,    w1, [x2, #1]!
    0x78, 0xc0, 0x1c, 0x41,         // ldrsh,   w1, [x2, #1]!
    0xb8, 0x80, 0x1c, 0x41,         // ldrsw,   x1, [x2, #1]!

    //exclusive
    0xc8, 0xdf, 0xfc, 0x41,          //ldar    x1, [x2]
    0x88, 0xdf, 0xfc, 0x41,          //ldar    w1, [x2]
    0x08, 0x5f, 0x7c, 0x41,          //ldxrb   w1, [x2]
    0x08, 0x5f, 0xfc, 0x41,          //ldaxrb  w1, [x2]

    0x08, 0xdf, 0xfc, 0x41,          //ldarb   w1, [x2]
    0x48, 0x5f, 0x7c, 0x41,          //ldxrh   w1, [x2]
    0x48, 0x5f, 0xfc, 0x41,          //ldaxrh  w1, [x2]
    0x48, 0xdf, 0xfc, 0x41,          //ldarh   w1, [x2]

    0xc8, 0x5f, 0x7c, 0x41,          //ldxr    x1, [x2]
    0xc8, 0x5f, 0xfc, 0x41,          //ldaxr   x1, [x2]
    0xc8, 0xdf, 0xfc, 0x41,          //ldar    x1, [x2]
    0x88, 0x5f, 0x7c, 0x41,          //ldxr    w1, [x2]

    0x88, 0x5f, 0xfc, 0x41,          //ldaxr   w1, [x2]
    0x88, 0xdf, 0xfc, 0x41,          //ldar    w1, [x2]
    0xc8, 0x7f, 0x0c, 0x41,          //ldxp    x1, x3, [x2]
    0xc8, 0x7f, 0x8c, 0x41,          //ldaxp   x1, x3, [x2]

    0x88, 0x7f, 0x0c, 0x41,          //ldxp    w1, w3, [x2]
    0x88, 0x7f, 0x8c, 0x41,          //ldaxp   w1, w3, [x2]

    //pair
    0xa8,   0x40,   0x88,   0x61,        //ldnp    x1, x2, [x3,#8]
    0xa9,   0x40,   0x88,   0x61,        //ldp     x1, x2, [x3,#8]
    0xa9,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3,#8]!
    0xa8,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3],#8
    0xa8,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3],#8

    //unsacled
    0x38,   0x40,   0x10,   0x61,        //ldurb   w1, [x3,#1]
    0x38,   0x80,   0x10,   0x61,        //ldursb  x1, [x3,#1]
    0xf8,   0x40,   0x10,   0x61,        //ldur     x1, [x3,#1]
    0x78,   0x40,   0x10,   0x61,        //ldurh    w1, [x3,#1]
    0x78,   0x80,   0x10,   0x61,        //ldursh   x1, [x3,#1]
    0xb8,   0x80,   0x10,   0x61,        //ldursw   x1, [x3,#1]

    //unprevlidged
    0x38,   0x40,   0x18,   0x61,        //ldtrb   w1, [x3,#1]
    0x38,   0x80,   0x18,   0x61,        //ldtrsb  x1, [x3,#1]
    0xf8,   0x40,   0x18,   0x61,        //ldtr    x1, [x3,#1]
    0x78,   0x40,   0x18,   0x61,        //ldtrh   w1, [x3,#1]
    0x78,   0x80,   0x18,   0x61,        //ldtrsh  x1, [x3,#1]
    0xb8,   0x80,   0x18,   0x61,        //ldtrsw  x1, [x3,#1]

    //----store----
    0xf8,   0x00,   0x14,   0x41,        //str     x1, [x2],#1
    0xf8,   0x0f,   0xf4,   0x41,        //str     x1, [x2],#255
    0x38,   0x00,   0x14,   0x41,        //strb    w1, [x2],#1
    0x78,   0x00,   0x14,   0x41,        //strh    w1, [x2],#1

    0xf9,   0x00,   0x04,   0x41,        //str     x1, [x2,#8]
    0xf9,   0x00,   0x80,   0x41,        //str     x1, [x2,#256]
    0x39,   0x00,   0x10,   0x41,        //strb    w1, [x2,#4]
    0x79,   0x00,   0x08,   0x41,        //strh    w1, [x2,#4]

    0xf8,   0x23,   0x68,   0x41,        //str     x1, [x2,x3]
    0xf8,   0x23,   0x68,   0x41,        //str     x1, [x2,x3]
    0x38,   0x23,   0x68,   0x41,        //strb    w1, [x2,x3]
    0x78,   0x23,   0x68,   0x41,        //strh    w1, [x2,x3]

    0xf8,   0x00,   0x1c,   0x41,        //str     x1, [x2,#1]!
    0xf8,   0x0f,   0xfc,   0x41,        //str     x1, [x2,#255]!
    0x38,   0x00,   0x1c,   0x41,        //strb    w1, [x2,#1]!
    0x78,   0x00,   0x1c,   0x41,        //strh    w1, [x2,#1]!

    0x08,   0x00,   0x7c,   0x41,        //stxrb   w0, w1, [x2]
    0x48,   0x00,   0x7c,   0x41,        //stxrh   w0, w1, [x2]
    0x88,   0x00,   0x7c,   0x41,        //stxr    w0, w1, [x2]
    0x88,   0x20,   0x0c,   0x41,        //stxp    w0, w1, w3, [x2]

    0xa8,   0x00,   0x88,   0x61,        //stnp    x1, x2, [x3,#8]
    0xa9,   0x00,   0x88,   0x61,        //stp     x1, x2, [x3,#8]
    0xa9,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3,#8]!
    0xa8,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3],#8
    0xa8,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3],#8

    0x38,   0x00,   0x10,   0x61,        //sturb   w1, [x3,#1]
    0xf8,   0x00,   0x10,   0x61,        //str     x1, [x3,#1]
    0x78,   0x00,   0x10,   0x61,        //strh    w1, [x3,#1]

    0xf8,   0x00,   0x18,   0x61,        //sttr    x1, [x3,#1]
    0x38,   0x00,   0x18,   0x61,        //sttrb   w1, [x3,#1]
    0x78,   0x00,   0x18,   0x61,        //sttrh   w1, [x3,#1]

    0x08,   0x9f,   0xfc,   0x61,        //stlrb   w1, [x3]
    0xc8,   0x9f,   0xfc,   0x61,        //stlr    x1, [x3]
    0x48,   0x9f,   0xfc,   0x61,        //stlrh   w1, [x3]
    0xc8,   0x20,   0x88,   0x61,        //stlxp   w0, x1, x2, [x3]
    0x08,   0x00,   0xfc,   0x61,        //stlxrb  w0, w1, [x3]
    0xc8,   0x00,   0xfc,   0x61,        //stlxr   w0, x1, [x3]
    0x48,   0x00,   0xfc,   0x61,        //stlxrh  w0, w1, [x3]

    0xf8,   0x63,   0x68,   0x41,        //ldr     x1, [x2,x3]
    0xf8,   0x63,   0x78,   0x41,        //ldr     x1, [x2,x3,lsl #3]
    0xb8,   0x63,   0x78,   0x41,        //ldr     w1, [x2,x3,lsl #2]
    0xf8,   0x63,   0x48,   0x41,        //ldr     x1, [x2,w3,uxtw]

    0xf8,   0x63,   0xe8,   0x41,        //ldr     x1, [x2,x3,sxtx]
    0x9a,   0x82,   0x04,   0x20,        //csinc   x0, x1, x2, eq
    0xda,   0x82,   0x00,   0x20,        //csinv   x0, x1, x2, eq
    0xda,   0x82,   0x04,   0x20,        //csneg   x0, x1, x2, eq

    0xd3,   0x7f,   0x00,   0x20,        //ubfiz   x0, x1, #1, #1
    0x53,   0x00,   0x7c,   0x20,        //lsr     w0, w1, #0
    0x92,   0x9f,   0xff,   0xe0,        //mov     x0, #0xffffffffffff0000         // #-65536
    0x92,   0xff,   0xff,   0xe0,        //mov     x0, #0xffffffffffff             // #281474976710655

    0x12,   0xbf,   0xff,   0xe0,        //movn    w0, #0xffff, lsl #16
    0xd2,   0x9f,   0xff,   0xe0,        //mov     x0, #0xffff                     // #65535
    0xd2,   0xff,   0xff,   0xe0,        //mov     x0, #0xffff000000000000         // #-281474976710656
    0x52,   0xbf,   0xff,   0xe0,        //mov     w0, #0xffff0000                 // #-65536

    0x18,   0x7f,   0xff,   0xbe,        //ldr     w30, 500630 0xffff4
    0x58,   0xf3,   0xcb,   0x1e,        //ldr     x30, 3e7fa0  -0x184d0
    0xb8,   0x4f,   0xf4,   0x0f,        //ldr     w15, [x0],#255
    0xf8,   0x51,   0x07,   0xcf,        //ldr     x15, [x30],#-240

    0xb9,   0x7f,   0xff,   0xfd,        //ldr     w29, [sp,#16380]
    0xf9,   0x7f,   0xff,   0xfd,        //ldr     x29, [sp,#32760]
    0xf8,   0x7f,   0x68,   0x41,        //ldr     x1, [x2,x31]
    0xf8,   0x4f,   0xff,   0xe1,        //ldr     x1, [sp,#255]!

    //prfm
    0xd8,   0x00,   0x00,   0x3f,	//prfm	   1F, [pc + 4]
    0xd8,   0xff,   0xff,   0xe0,	//prfm	   0, [pc + fffffffffffffffc]
    0xd8,   0x0f,   0xd8,   0x08,	//prfm	   8, [pc + 1fb00]
    0xf9,   0x80,   0x04,   0x3d,	//prfm	   1d, [x1 + 8]
    0xf9,   0xbf,   0xff,   0xdf,	//prfm	   1f, [x30 + 7ff8]
    0xf8,   0xa5,   0x68,   0x44,	//prfm	   4, [x2 + x5 << 0]
    0xf8,   0xa1,   0x5b,   0xc5,	//prfm	   5, [x30 + w1 << 3]
    0xf8,   0xa5,   0xe9,   0x06,	//prfm	   6, [x8 + x5 << 0]
    
    0xd5,   0x03,   0x20,   0x1f,        //nop
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
        i->format();
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

  std::deque<registerSet> expectedRead, expectedWritten;
  registerSet tmpRead, tmpWritten;

  test_results_t retVal = PASSED;

  //ldr x1, #4
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {x1};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// ldr x1, #256
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {x1};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// ldr w1, #4
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {w1};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// ldr w1, #256
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {w1};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// post-inc
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// imm
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// reg offset not ext
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// pre inc
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// ex
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1, x3};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1)(x3);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {x1, x3};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(x1)(x3);
#endif

//
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1, w3};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1)(w3);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2};
	tmpWritten = {w1, w3};
#else
	tmpRead = list_of(x2);
	tmpWritten = list_of(w1)(w3);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// pair
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1, x2};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1)(x2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1, x2};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1)(x2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1, x2};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1)(x2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1, x2};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1)(x2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1, x2};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1)(x2);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// unscaled
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// unpre
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// store 155
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)( w1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//161
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 166
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1,x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 171
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(w1);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 176
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {w0};
#else
	tmpRead = list_of(x2)(w1);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {w0};
#else
	tmpRead = list_of(x2)(w1);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1};
	tmpWritten = {w0};
#else
	tmpRead = list_of(x2)(w1);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, w1, w3};
	tmpWritten = {w0};
#else
	tmpRead = list_of(x2)(w1)(w3);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//  181
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x1, x3};
	tmpWritten = {};
#else
	tmpRead = list_of(x2)(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 187
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 191
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 195
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x1};
	tmpWritten = {};
#else
	tmpRead = list_of(x1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, w1};
	tmpWritten = {};
#else
	tmpRead = list_of(w1)(x3);
	//tmpWritten = list_of();
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 198
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x1, x2};
	tmpWritten = {w0};
#else
	tmpRead = list_of(x1)(x2)(x3);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w1, x3};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w1)(x3);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1, x3};
	tmpWritten = {w0};
#else
	tmpRead = list_of(x1)(x3);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w1, x3};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w1)(x3);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//203
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x2};
	tmpWritten = {w1};
#else
	tmpRead = list_of(x2)(x3);
	tmpWritten = list_of(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w3, x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(w3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//208
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x3, x2};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(w3);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1, x2, pstate};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x2)(x1)(pstate);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1, x2, pstate};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x2)(x1)(pstate);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1, x2, pstate};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x2)(x1)(pstate);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//213
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1};
	tmpWritten = {x0};
#else
	tmpRead = list_of(x1);
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {w1};
	tmpWritten = {w0};
#else
	tmpRead = list_of(w1);
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {x0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {x0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

//218
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {w0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {x0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {x0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(x0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {};
	tmpWritten = {w0};
#else
	//tmpRead = list_of();
	tmpWritten = list_of(w0);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// 223
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
	tmpWritten = {w30};
#else
	tmpRead = list_of(pc);
	tmpWritten = list_of(w30);
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
	tmpRead = {x0};
	tmpWritten = {w15};
#else
	tmpRead = list_of(x0);
	tmpWritten = list_of(w15);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30};
	tmpWritten = {x15};
#else
	tmpRead = list_of(x30);
	tmpWritten = list_of(x15);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();


// 228
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {sp};
	tmpWritten = {w29};
#else
	tmpRead = list_of(sp);
	tmpWritten = list_of(w29);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {sp};
	tmpWritten = {x29};
#else
	tmpRead = list_of(sp);
	tmpWritten = list_of(x29);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, zr};
	tmpWritten = {x1};
#else
	tmpRead = list_of(x2)(zr);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {sp};
	tmpWritten = {x1};
#else
	tmpRead = list_of(sp);
	tmpWritten = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();


#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
#else
	tmpRead = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
#else
	tmpRead = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {pc};
#else
	tmpRead = list_of(pc);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x1};
#else
	tmpRead = list_of(x1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30};
#else
	tmpRead = list_of(x30);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x2, x5};
#else
	tmpRead = list_of(x2)(x5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x30, w1};
#else
	tmpRead = list_of(x30)(w1);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
	tmpRead = {x8, x5};
#else
	tmpRead = list_of(x8)(x5);
#endif
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

// nop
expectedRead.push_back(tmpRead);
expectedWritten.push_back(tmpWritten);
tmpRead.clear();
tmpWritten.clear();

  decodedInsns.pop_back();
  while(!decodedInsns.empty())
  {
      retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead.front(),
                                   expectedWritten.front()));
//      cout<<decodedInsns.front()->format()<<" "<<retVal<<endl;
      decodedInsns.pop_front();

  	  expectedRead.pop_front();
      expectedWritten.pop_front();
  }

  return retVal;
}

