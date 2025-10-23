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

#include "Immediate.h"
#include "InstructionDecoderImpl.h"
#include "registers/MachRegister.h"

#include <array>
#include <iostream>
#include <stdint.h>
#include <string>

namespace Dyninst { namespace InstructionAPI {

  struct aarch64_insn_entry;
  struct aarch64_mask_entry;

  class InstructionDecoder_aarch64 : public InstructionDecoderImpl {
    friend struct aarch64_insn_entry;
    friend struct aarch64_mask_entry;

  public:
    InstructionDecoder_aarch64(Architecture a);

    virtual ~InstructionDecoder_aarch64();

    virtual Instruction decode(InstructionDecoder::buffer& b);

    virtual void setMode(bool) {}

    static const std::array<std::string, 16> condNames;
    static MachRegister sysRegMap(unsigned int);
    static const char* bitfieldInsnAliasMap(entryID);
    static const char* condInsnAliasMap(entryID);

  private:

    bool isPstateRead{}, isPstateWritten{};
    bool isFPInsn{}, isSIMDInsn{};
    bool skipRn{}, skipRm{};
    bool is64Bit{};
    bool isValid{};

    void mainDecode();

    int findInsnTableIndex(unsigned int);

    bool decodeOperands(const Instruction* insn_to_complete);

    /*members for handling operand re-ordering, will be removed later once a generic operand
     * ordering method is incorporated*/
    int oprRotateAmt{};
    bool hasb5{};

    void reorderOperands();

    unsigned int insn{};
    boost::shared_ptr<Instruction> insn_in_progress;

    template <int start, int end> int field(unsigned int raw) {
      return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
    }

    int32_t sign_extend32(int size_, int in) {
      int32_t val = 0 | in;

      return (val << (32 - size_)) >> (32 - size_);
    }

    int64_t sign_extend64(int size_, int in) {
      int64_t val = 0 | in;

      return (val << (64 - size_)) >> (64 - size_);
    }

    uint32_t unsign_extend32(int size_, int in) {
      uint32_t mask = ~0;

      return (mask >> (32 - size_)) & in;
    }

    uint64_t unsign_extend64(int size_, int in) {
      uint64_t mask = ~0;

      return (mask >> (64 - size_)) & in;
    }

    int highest_set_bit(int32_t val) {
      for(int bit_index = 31; bit_index >= 0; bit_index--)
        if(((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
          return bit_index + 1;

      return -1;
    }

    int lowest_set_bit(int32_t val) {
      for(int bit_index = 0; bit_index <= 31; bit_index++)
        if(((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
          return bit_index + 1;

      return -1;
    }

    int op1Field{}, op2Field{}, crmField{};

    void processSystemInsn();

    bool hasHw{};
    int hwField{};

    void processHwFieldInsn(int, int);

    bool hasShift{};
    int shiftField{};

    void processShiftFieldShiftedInsn(int, int);

    void processShiftFieldImmInsn(int, int);

    bool hasOption{};
    int optionField{};

    void processOptionFieldLSRegOffsetInsn();

    bool hasN{};
    int immr, immrLen{};
    int sField{}, nField{}, nLen{};

    int immlo{}, immloLen{};

    void makeBranchTarget(bool, bool, int, int);

    Expression::Ptr makeFallThroughExpr();

    int _szField{}, size{};
    int _typeField{};
    int cmode{};
    int op{};
    int simdAlphabetImm{};

    void processAlphabetImm();

    void NOTHING();

    void set32Mode();

    void setRegWidth();

    void setFPMode();

    void setSIMDMode();

    bool isSinglePrec();

    bool fix_bitfieldinsn_alias(int, int);
    void fix_condinsn_alias_and_cond(int&);
    void modify_mnemonic_simd_upperhalf_insns();
    bool pre_process_checks(const aarch64_insn_entry&);

    MachRegister makeAarch64RegID(MachRegister, unsigned int);

    MachRegister getLoadStoreSimdRegister(int encoding);

    Expression::Ptr makeRdExpr();
    Expression::Ptr makeRnExpr();
    Expression::Ptr makeRmExpr();
    Expression::Ptr makeRaExpr();
    Expression::Ptr makeRsExpr();
    Expression::Ptr makePstateExpr();
    Expression::Ptr makePCExpr();
    Expression::Ptr makeb40Expr();
    Expression::Ptr makeOptionExpression(int, int);

    template <typename T, Result_Type rT> Expression::Ptr fpExpand(int);

    Expression::Ptr makeRtExpr();
    Expression::Ptr makeRt2Expr();
    Expression::Ptr makeMemRefReg();
    Expression::Ptr makeMemRefReg_Rm();
    Expression::Ptr makeMemRefReg_ext();
    Expression::Ptr makeMemRefReg_amount();
    Expression::Ptr makeMemRefIndexLiteral();
    Expression::Ptr makeMemRefIndexUImm();
    Expression::Ptr makeMemRefIndexPre();
    Expression::Ptr makeMemRefIndexPost();
    Expression::Ptr makeMemRefIndex_addOffset9();
    Expression::Ptr makeMemRefIndex_offset9();
    Expression::Ptr makeMemRefPairPre();
    Expression::Ptr makeMemRefPairPost();
    Expression::Ptr makeMemRefPair_offset7();
    Expression::Ptr makeMemRefPair_addOffset7();
    Expression::Ptr makeMemRefEx();
    Expression::Ptr makeMemRefExPair();
    Expression::Ptr makeMemRefExPair2();
    Expression::Ptr makeMemRefSIMD_MULT();
    Expression::Ptr makeMemRefSIMD_SING();

    template <typename T>
    Expression::Ptr makeLogicalImm(int immr, int imms, int immsLen, Result_Type rT);

    void getMemRefIndexLiteral_OffsetLen(int&, int&);
    void getMemRefIndex_SizeSizelen(unsigned int&, unsigned int&);
    void getMemRefIndexPrePost_ImmImmlen(unsigned int&, unsigned int&);
    void getMemRefPair_ImmImmlen(unsigned int& immVal, unsigned int& immLen);
    void getMemRefEx_RT(Result_Type& rt);
    void getMemRefIndexLiteral_RT(Result_Type&);
    void getMemRefExPair_RT(Result_Type& rt);
    void getMemRefPair_RT(Result_Type& rt);
    void getMemRefIndex_RT(Result_Type&);
    void getMemRefIndexUImm_RT(Result_Type&);
    unsigned int getMemRefSIMD_MULT_T();
    unsigned int getMemRefSIMD_SING_T();
    void getMemRefSIMD_MULT_RT(Result_Type&);
    void getMemRefSIMD_SING_RT(Result_Type&);
    unsigned int get_SIMD_MULT_POST_imm();
    unsigned int get_SIMD_SING_POST_imm();
    void OPRRd();
    void OPRsf();

    template <unsigned int endBit, unsigned int startBit> void OPRoption();

    void OPRshift();
    void OPRhw();

    template <unsigned int endBit, unsigned int startBit> void OPRN();

    // for load store
    void LIndex();
    void STIndex();
    void OPRRn();
    void OPRRnL();
    void OPRRnLU();
    void OPRRnSU();
    void OPRRnS();
    void OPRRnU();
    void OPRRm();
    void OPRRt();
    void OPRRtL();
    void OPRRtS();
    void OPRRt2();
    void OPRRt2L();
    void OPRRt2S();
    void OPRop1();
    void OPRop2();

    template <unsigned int endBit, unsigned int startBit> void OPRcond();

    void OPRnzcv();
    void OPRCRm();
    void OPRCRn();

    template <unsigned int endBit, unsigned int startBit> void OPRS();

    void OPRRa();
    void OPRo0();
    void OPRb5();
    void OPRb40();

    template <unsigned int endBit, unsigned int startBit> void OPRsz();

    void OPRRs();

    template <unsigned int endBit, unsigned int startBit> void OPRimm();

    void OPRscale();

    template <unsigned int endBit, unsigned int startBit> void OPRtype();

    void OPRQ();
    void OPRL();

    // void OPRR();
    void OPRH() {}
    void OPRM() {}
    void OPRa();
    void OPRb();
    void OPRc();
    void OPRd();
    void OPRe();
    void OPRf();
    void OPRg();
    void OPRh();
    void OPRopc();
    void OPRopcode() {}
    void OPRlen();

    template <unsigned int endBit, unsigned int startBit> void OPRsize();

    void OPRcmode();
    void OPRrmode() {}
    void OPRop();
    void setFlags();

    unsigned int _Q{};
    unsigned int _L{};
    unsigned int _R{};

    void getSIMD_MULT_RptSelem(unsigned int& rpt, unsigned int& selem);
    unsigned int getSIMD_SING_selem();
  };
}}
