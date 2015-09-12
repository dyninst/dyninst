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

#include "InstructionDecoder-aarch64.h"
#include "Immediate.h"
#include <boost/assign/list_of.hpp>
#include "../../common/src/singleton_object_pool.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    typedef void (InstructionDecoder_aarch64::*operandFactory)();
    typedef std::vector< operandFactory > operandSpec;
    typedef const aarch64_entry&(InstructionDecoder_aarch64::*nextTableFunc)();
    typedef std::map<unsigned int, aarch64_entry> aarch64_table;

    // steve note: what are these used for?
    bool InstructionDecoder_aarch64::foundDoubleHummerInsn = false;
    bool InstructionDecoder_aarch64::foundQuadInsn = false;

    struct aarch64_entry
    {
        aarch64_entry(entryID o, const char* m, nextTableFunc next, operandSpec ops) :
            op(o), mnemonic(m), next_table(next), operands(ops)
        {
        }

        aarch64_entry(entryID o, const char* m, operandSpec ops):
            op(o), mnemonic(m), next_table(NULL), operands(ops)
        {
        }

        aarch64_entry() :
            op(aarch64_op_INVALID), mnemonic("INVALID"), next_table(NULL)
        {
            // TODO: why 5?
            operands.reserve(5);
        }

        aarch64_entry(const aarch64_entry& o) :
            op(o.op), mnemonic(o.mnemonic), next_table(o.next_table), operands(o.operands)
        {
        }

        const aarch64_entry& operator=(const aarch64_entry& rhs)
            {
                operands.reserve(rhs.operands.size());
                op = rhs.op;
                mnemonic = rhs.mnemonic;
                next_table = rhs.next_table;
                operands = rhs.operands;
                return *this;
            }

        entryID op;
        const char* mnemonic;
        nextTableFunc next_table;
        operandSpec operands;
        static void buildTables();
        static bool built_tables;

        // more tables for diff insn classes
        static aarch64_table main_opcode_table;
        static aarch64_table ext_op_GroupDiBSys;
        static std::vector<aarch64_entry> aarch64_insn_table;
    };

    InstructionDecoder_aarch64::InstructionDecoder_aarch64(Architecture a)
      : InstructionDecoderImpl(a),
        insn(0),
        insn_in_progress(NULL),
	    isRAWritten(false),
        invertBranchCondition(false),
        isFPInsn(false),
        bcIsConditional(false)
    {
        aarch64_entry::buildTables();
    }

    InstructionDecoder_aarch64::~InstructionDecoder_aarch64()
    {
    }

    void InstructionDecoder_aarch64::decodeOpcode(InstructionDecoder::buffer& b)
    {
      b.start += 4;
    }

    using namespace std;
    Instruction::Ptr InstructionDecoder_aarch64::decode(InstructionDecoder::buffer& b)
    {
        insn_printf("### decoding\n");
        if(b.start > b.end) return Instruction::Ptr();
        isRAWritten = false;
        isFPInsn = false;
        bcIsConditional = false;
        insn = b.start[0] << 24 | b.start[1] << 16 |
        b.start[2] << 8 | b.start[3];
#if defined(DEBUG_RAW_INSN)
        cout.width(0);
        cout << "0x";
        cout.width(8);
        cout.fill('0');
        cout << hex << insn << "\t";
#endif
        mainDecode();
        b.start += 4;
        return make_shared(insn_in_progress);
    }

    void InstructionDecoder_aarch64::setMode(bool )
    {
    }

    bool InstructionDecoder_aarch64::decodeOperands(const Instruction *)
    {
        return false;
    }

    void InstructionDecoder_aarch64::doDelayedDecode(const Instruction *)
    {
        assert(0); //not implemented yet
    }

    Result_Type InstructionDecoder_aarch64::makeSizeType(unsigned int)
    {
        assert(0); //not implemented
        return u32;
    }

    // *****************
    // decoding operands
    // *****************

    void InstructionDecoder_aarch64::Rd(){
        assert(0);
    }

    void InstructionDecoder_aarch64::Rn(){
        assert(0);
    }

    void InstructionDecoder_aarch64::Imm12(){
        assert(0);
    }

    // ****************
    // decoding opcodes
    // ****************

#define fn(x) (&InstructionDecoder_aarch64::x)

using namespace boost::assign;

#include "aarch64_opcode_tables.C"

    // TODO this is a tmp experiment
    const aarch64_entry& InstructionDecoder_aarch64::ext_op_DiBSys()
    {
        //insn_printf("dibsys field %d\n", field<30,30>(insn));
        return aarch64_entry::ext_op_GroupDiBSys[field<30,30>(insn)];
    }


    void InstructionDecoder_aarch64::mainDecode()
    {
        // the 27, 28 bit indicate which op class the insn is
        const aarch64_entry* current = &aarch64_entry::main_opcode_table[field<27, 28>(insn)];
        insn_printf("field %d 0x%x\n", field<27, 28>(insn), insn);
        while(current->next_table)
        {
            current = &(std::mem_fun(current->next_table)(this));
        }
        insn_in_progress = makeInstruction(current->op, current->mnemonic, 4, reinterpret_cast<unsigned char*>(&insn));
        insn_printf("ARM: %s\n", current->mnemonic);

        // control flow operations?
        /* tmp commented this part
        if(current->op == power_op_b ||
          current->op == power_op_bc ||
          current->op == power_op_bclr ||
          current->op == power_op_bcctr)
        {
            // decode control-flow operands immediately; we're all but guaranteed to need them
            doDelayedDecode(insn_in_progress);
        }
        */

        //insn_in_progress->arch_decoded_from = m_Arch;
        insn_in_progress->arch_decoded_from = Arch_aarch64;
        return;
    }
  };
};




