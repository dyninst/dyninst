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
#include <boost/assign/list_of.hpp>
#include <deque>
using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace boost::assign;

using namespace std;

class stack_mov_32_bit_Mutator : public InstructionMutator {
    public:
        stack_mov_32_bit_Mutator() { };
        virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* stack_mov_32_bit_factory()
{
    return new stack_mov_32_bit_Mutator();
}


test_results_t stack_mov_32_bit_Mutator::executeTest()
{
    const unsigned char buffer[] =
    {
      0x89, 0x44, 0x24, 0x03 // mov %eax, 0x3(%rsp)
    };
    unsigned int size = 4;
    unsigned int expectedInsns = 2;
    InstructionDecoder d(buffer, size, Dyninst::Arch_x86_64);
    
    std::deque<Instruction::Ptr> decodedInsns;
    Instruction::Ptr i;
    do
    {
        i = d.decode();
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
            if(*curInsn)
            {
                logerror("\t%s\n", (*curInsn)->format().c_str());
            }
            else
            {
                logerror("\t[NULL]\n");
            }
        }
    
        return FAILED;
    }
    if(decodedInsns.back() && decodedInsns.back()->isValid())
    {
        logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
        return FAILED;
    }
  
    Architecture curArch = Arch_x86_64;
    registerSet expectedRead, expectedWritten;
    test_results_t retVal = PASSED;
    {
        using namespace x86_64;
  
        RegisterAST::Ptr r_eax(new RegisterAST(eax));
        RegisterAST::Ptr r_rsp(new RegisterAST(rsp));

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
        expectedRead = { r_eax, r_rsp };
	// no registers written, moving reg->stack
#else
        expectedRead = list_of(r_eax)(r_rsp);
#endif  
  
        retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead, expectedWritten));
        decodedInsns.pop_front();
  
    }
    return retVal;
}

