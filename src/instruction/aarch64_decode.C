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
public:
    aarch64_decode_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* aarch64_decode_factory()
{
   return new aarch64_decode_Mutator();
}

void aarch64_decode_Mutator::setupRegisters()
{
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
	   
	  RegisterAST::Ptr zr (new RegisterAST(aarch64::zr));
	  RegisterAST::Ptr wzr (new RegisterAST(aarch64::wzr));
	  RegisterAST::Ptr sp (new RegisterAST(aarch64::sp));
	  RegisterAST::Ptr wsp (new RegisterAST(aarch64::wsp));
	  RegisterAST::Ptr pc (new RegisterAST(aarch64::pc));
	  RegisterAST::Ptr pstate (new RegisterAST(aarch64::pstate));
}

test_results_t aarch64_decode_Mutator::executeTest()
{
  const unsigned char buffer[] =
  {
	0x0B, 0x0C, 0x01, 0x41,		// ADD W1, W10, W12
	0x0B, 0x08, 0x14, 0xA0,		// ADD W0, W5, W8, LSL #5
	0x8B, 0x49, 0x28, 0xE4,		// ADD X4, X7, X9, LSR #10
	0x8B, 0xC9, 0x28, 0xE4,		// ADD X4, X7, X9, ROR #10 (invalid)
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
	0x12, 0x00, 0xFF, 0xFF,		// AND WSP, WSP, #63
	0xB2, 0x00, 0x03, 0xDF,		// ORR SP, X30, #0
	0xD2, 0x7F, 0xFF, 0x34,		// EOR X20, X25, #
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
	0xD6, 0x9F, 0x03, 0xE0,		// ERET
	0xD6, 0xBF, 0x03, 0xE0,		// DRPS
	0x1E, 0x3F, 0x20, 0x00,		// FCMP S0, S32
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
    0x00, 0x00, 0x00, 0x00      // INVALID
  };

  unsigned int size = sizeof(buffer);
  unsigned int expectedInsns = size/4;

  ++expectedInsns;
  InstructionDecoder d(buffer, size, Dyninst::Arch_aarch64);

  std::deque<Instruction::Ptr> decodedInsns;
  Instruction::Ptr i;
  do
  {
    i = d.decode();
    cout<<i->format().c_str()<<endl;
    decodedInsns.push_back(i);
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

  setupRegisters();

  std::deque<registerSet> expectedRead, expectedWritten;
  registerSet tmpRead, tmpWritten;

  test_results_t retVal = PASSED;

  decodedInsns.pop_back();
  while(!decodedInsns.empty())
  {
      retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead.front(),
                                   expectedWritten.front()));
      // TEMP commented out
      /*
      if(decodedInsns.size() == 1)
      {
          if(!decodedInsns.front()->readsMemory())
          {
              logerror("**FAILED**: insn %s did not read memory, expected lhzux r5, r7, r9\n",
                       decodedInsns.front()->format().c_str());
              return FAILED;
          }
      }
      */
      decodedInsns.pop_front();

      expectedRead.pop_front();
      expectedWritten.pop_front();
  }

  return retVal;
}

