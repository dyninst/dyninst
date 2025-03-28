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

#include "InstructionDecoderImpl.h"
#include <iostream>
#include "Immediate.h"
#include "dyn_regs.h"

#include <array>
#include <iostream>
#include <stdint.h>
#include <string>

namespace Dyninst
{
namespace InstructionAPI
{
#if defined(__GNUC__)
#    define insn_printf(format, ...)                                                     \
        do                                                                               \
        {                                                                                \
            printf("[%s:%u]insn_debug " format, FILE__, __LINE__, ##__VA_ARGS__);        \
        } while(0)
#endif

struct amdgpu_insn_entry;
struct amdgpu_mask_entry;

class InstructionDecoder_amdgpu_vega : public InstructionDecoderImpl
{
    friend struct amdgpu_insn_entry;
    friend struct amdgpu_mask_entry;
    enum DecodeFamily
    {
        sopp
    };

public:
    InstructionDecoder_amdgpu_vega(Architecture a);

    virtual ~InstructionDecoder_amdgpu_vega();

    virtual void decodeOpcode(InstructionDecoder::buffer& b);

    // decode one instruction starting from b.start
    // will advance b.start whenver a instruction is successfully decoded
    virtual Instruction decode(InstructionDecoder::buffer& b);

    virtual void setMode(bool) {}

    virtual bool decodeOperands(const Instruction* insn_to_complete);

    bool decodeOperands(const amdgpu_insn_entry& insn_entry);

    virtual void doDelayedDecode(const Instruction* insn_to_complete);

    static const std::array<std::string, 16> condNames;
    static MachRegister                      sysRegMap(unsigned int);
    static const char*                       bitfieldInsnAliasMap(entryID);
    static const char*                       condInsnAliasMap(entryID);

private:
    virtual Result_Type makeSizeType(unsigned int opType);

    bool is64Bit;

    unsigned int insn_size;  // size of the instruction that we are currently working on
    unsigned int insn;       // the first 32 bit
    unsigned int insn_high;  // the second 32 bit
    unsigned long long int insn_long;  // the combined 64 bit: insn_high << 32 | insn

    // the main process of decoding an instruciton, won't advance buffer
    void mainDecode(InstructionDecoder::buffer& b);

    void mainDecodeOpcode(InstructionDecoder::buffer& b);

    void setupInsnWord(InstructionDecoder::buffer& b);
    // pointer to the instruction that we are currently working on
    boost::shared_ptr<Instruction> insn_in_progress;

    template <int start, int end>
    int field(unsigned int raw)
    {
#if defined DEBUG_FIELD
        std::cerr << start << "-" << end << ":" << std::dec
                  << (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
        return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
    }

    template <int start, int end>
    int longfield(unsigned long long int raw)
    {
#if defined DEBUG_FIELD
        std::cerr << start << "-" << end << ":" << std::dec
                  << (raw >> (start) & (0xFFFFFFFFFFFFFFFF >> (63 - (end - start))))
                  << " ";
#endif
        return ((raw >> (start)) & (0xFFFFFFFFFFFFFFFF >> (63 - (end - start))));
    }

    template <int start, int end>
    int rev_field(unsigned int raw)
    {
#if defined DEBUG_FIELD
        std::cerr << start << "-" << end << ":" << std::dec
                  << (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
        unsigned int le = (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));

        std::cerr << "operating on le " << std::hex << le << std::endl;
        unsigned int be = __builtin_bswap32(le);

        std::cerr << "operating on be " << std::hex << be << std::endl;
        return be >> (31 - (end - start));
    }

    int32_t sign_extend32(int size_, int in)
    {
        int32_t val = 0 | in;

        return (val << (32 - size_)) >> (32 - size_);
    }

    int64_t sign_extend64(int size_, int in)
    {
        int64_t val = 0 | in;

        return (val << (64 - size_)) >> (64 - size_);
    }

    uint32_t unsign_extend32(int size_, int in)
    {
        uint32_t mask = ~0;

        return (mask >> (32 - size_)) & in;
    }

    uint64_t unsign_extend64(int size_, int in)
    {
        uint64_t mask = ~0;

        return (mask >> (64 - size_)) & in;
    }

    int highest_set_bit(int32_t val)
    {
        for(int bit_index = 31; bit_index >= 0; bit_index--)
            if(((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
                return bit_index + 1;

        return -1;
    }

    int lowest_set_bit(int32_t val)
    {
        for(int bit_index = 0; bit_index <= 31; bit_index++)
            if(((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
                return bit_index + 1;

        return -1;
    }

    bool hasHw;
    int  hwField;

    void processHwFieldInsn(int, int);

    bool hasShift;
    int  shiftField;

    void makeBranchTarget(bool, bool, int, int);

    Expression::Ptr makeFallThroughExpr();

    int _szField, size;
    int _typeField;
    int cmode;
    int op;
    int simdAlphabetImm;

    void processAlphabetImm();

    void NOTHING();
    bool fix_bitfieldinsn_alias(int, int);
    void fix_condinsn_alias_and_cond(int&);
    void modify_mnemonic_simd_upperhalf_insns();

    MachRegister makeAmdgpuRegID(MachRegister, unsigned int, unsigned int len = 1);

    MachRegister getLoadStoreSimdRegister(int encoding);

    Expression::Ptr makePCExpr();

    template <typename T>
    Expression::Ptr makeLogicalImm(int immr, int imms, int immsLen, Result_Type rT);

    // for load store
    void insnSize(unsigned int insn_size);

    Expression::Ptr decodeSSRC(unsigned int index);
    Expression::Ptr decodeVSRC(unsigned int index);
    Expression::Ptr decodeVDST(unsigned int index);

    Expression::Ptr decodeSGPRorM0(unsigned int offset);

    void finalizeFLATOperands();
    void finalizeSMEMOperands();

    void finalizeSOPKOperands();
    void finalizeSOPPOperands();
    void finalizeSOP2Operands();
    void finalizeSOP1Operands();
    void finalizeSOPCOperands();

    void finalizeVOP1Operands();
    void finalizeVOP2Operands();
    void finalizeVOPCOperands();
    void finalizeVINTRPOperands();
    void finalizeDSOperands();
    void finalizeMTBUFOperands();
    void finalizeMUBUFOperands();
    void finalizeVOP3ABOperands();
    void finalizeVOP3POperands();

    bool         useImm;
    unsigned int immLen;
    unsigned int immLiteral;
    bool         setSCC;

#define IS_LD_ST() (isLoad || isStore)

    unsigned int num_elements;  // the number of elements that will be load or store by
                                // each instruction
    bool isSMEM;                // this is set when using smem instruction
    bool isLoad;    // this is set when a smem instruction is load, will set number of
                    // elements that are loaded at the same time
    bool isStore;   // similar to isLoad, but for store instructions
    bool isBuffer;  //
    bool isScratch;

    bool isBranch;       // this is set for all branch instructions,
    bool isConditional;  // this is set for all conditional branch instruction, will set
                         // branchCond
    bool isCall;         // this is a call function

    // this is set for instructions that directly modify pc
    // namely s_setpc and s_swappc
    bool isModifyPC;

    // reset the decoder internal state for decoding the next instruction
    void reset();

    Expression::Ptr branchCond;
    Expression::Ptr branchTarget;

    void setBranch() { isBranch = true; }

    void setConditionalBranch()
    {
        isConditional = true;
        // TODO : set conditional branch
    }
    void setModifyPC() { isModifyPC = true; }

    void setCall() { isCall = true; }

    inline unsigned int get32bit(InstructionDecoder::buffer& b, unsigned int offset)
    {
        assert(offset % 4 == 0);
        return b.start[offset + 3] << 24 | b.start[offset + 2] << 16 |
               b.start[offset + 1] << 8 | b.start[offset];
    }

    template <unsigned int start, unsigned int end, unsigned int candidate>
    void setUseImm(InstructionDecoder::buffer& b, unsigned int offset)
    {
        if(longfield<start, end>(insn_long) == candidate)
        {
            useImm     = true;
            immLen     = 4;
            immLiteral = get32bit(b, offset);
        }
    }

    void setSMEM() { isSMEM = true; }

    template <unsigned int num_elements>
    void setLoad()
    {
        isLoad             = true;
        this->num_elements = num_elements;
    }

    template <unsigned int num_elements>
    void setStore()
    {
        isStore            = true;
        this->num_elements = num_elements;
    }

    void setScratch() { isScratch = true; }

    void setBuffer() { isBuffer = true; }

    typedef struct buffer_resource_desc
    {
        unsigned long long base_address;
        unsigned           stride;
        unsigned           cache_swizzle;
        unsigned           swizzle_enable;
        unsigned           num_records;
        unsigned           dst_sel_x;
        unsigned           dst_sel_y;
        unsigned           dst_sel_z;
        unsigned           dst_sel_w;
        unsigned           num_format;
        unsigned           data_format;
        unsigned           user_vm_enable;
        unsigned           user_vm_mode;
        unsigned           index_stride;
        unsigned           add_tid_enable;
        unsigned           non_volatile;
        unsigned           type;
    } buffer_resource_desc;

    void debug_instr();

#include "amdgpu_decoder_impl_vega.h"
};
}  // namespace InstructionAPI
}  // namespace Dyninst
