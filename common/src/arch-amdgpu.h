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

#ifndef _ARCH_AMDGPU_H
#define _ARCH_AMDGPU_H

// We primarily use the emitter for AMDGPU.
// This is there to make the port build.

#include "dyntypes.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"
#include "dyn_register.h"
#include <assert.h>
#include <vector>
class AddressSpace;

namespace NS_amdgpu {
// AMDGPU GFX908 registers of interest for code generation. We will use liveness on these to
// allocate them for use in instrumentation.
// Note that this is for Dyninst's abstraction for code generation.
// The register ids are actual register numbers that are used in instructions. Since SGPRs and VGPRs have
// the same register numbers, they need to be distinguished.
//
// We map these 'Register's to MachRegisters in RegisterConversion-amdgpu.C
// For information on architectural registers see MachRegister in dyn_regs.h

using namespace Dyninst;

constexpr uint32_t MIN_SGPR_ID = 0;
constexpr uint32_t MAX_SGPR_ID = 101;

namespace RegisterConstants {
DYNINST_EXPORT extern const Register s0;
DYNINST_EXPORT extern const Register s1;
DYNINST_EXPORT extern const Register s2;
DYNINST_EXPORT extern const Register s3;
DYNINST_EXPORT extern const Register s4;
DYNINST_EXPORT extern const Register s5;
DYNINST_EXPORT extern const Register s6;
DYNINST_EXPORT extern const Register s7;
DYNINST_EXPORT extern const Register s8;
DYNINST_EXPORT extern const Register s9;
DYNINST_EXPORT extern const Register s10;
DYNINST_EXPORT extern const Register s11;
DYNINST_EXPORT extern const Register s12;
DYNINST_EXPORT extern const Register s13;
DYNINST_EXPORT extern const Register s14;
DYNINST_EXPORT extern const Register s15;
DYNINST_EXPORT extern const Register s16;
DYNINST_EXPORT extern const Register s17;
DYNINST_EXPORT extern const Register s18;
DYNINST_EXPORT extern const Register s19;
DYNINST_EXPORT extern const Register s20;
DYNINST_EXPORT extern const Register s21;
DYNINST_EXPORT extern const Register s22;
DYNINST_EXPORT extern const Register s23;
DYNINST_EXPORT extern const Register s24;
DYNINST_EXPORT extern const Register s25;
DYNINST_EXPORT extern const Register s26;
DYNINST_EXPORT extern const Register s27;
DYNINST_EXPORT extern const Register s28;
DYNINST_EXPORT extern const Register s29;
DYNINST_EXPORT extern const Register s30;
DYNINST_EXPORT extern const Register s31;
DYNINST_EXPORT extern const Register s32;
DYNINST_EXPORT extern const Register s33;
DYNINST_EXPORT extern const Register s34;
DYNINST_EXPORT extern const Register s35;
DYNINST_EXPORT extern const Register s36;
DYNINST_EXPORT extern const Register s37;
DYNINST_EXPORT extern const Register s38;
DYNINST_EXPORT extern const Register s39;
DYNINST_EXPORT extern const Register s40;
DYNINST_EXPORT extern const Register s41;
DYNINST_EXPORT extern const Register s42;
DYNINST_EXPORT extern const Register s43;
DYNINST_EXPORT extern const Register s44;
DYNINST_EXPORT extern const Register s45;
DYNINST_EXPORT extern const Register s46;
DYNINST_EXPORT extern const Register s47;
DYNINST_EXPORT extern const Register s48;
DYNINST_EXPORT extern const Register s49;
DYNINST_EXPORT extern const Register s50;
DYNINST_EXPORT extern const Register s51;
DYNINST_EXPORT extern const Register s52;
DYNINST_EXPORT extern const Register s53;
DYNINST_EXPORT extern const Register s54;
DYNINST_EXPORT extern const Register s55;
DYNINST_EXPORT extern const Register s56;
DYNINST_EXPORT extern const Register s57;
DYNINST_EXPORT extern const Register s58;
DYNINST_EXPORT extern const Register s59;
DYNINST_EXPORT extern const Register s60;
DYNINST_EXPORT extern const Register s61;
DYNINST_EXPORT extern const Register s62;
DYNINST_EXPORT extern const Register s63;
DYNINST_EXPORT extern const Register s64;
DYNINST_EXPORT extern const Register s65;
DYNINST_EXPORT extern const Register s66;
DYNINST_EXPORT extern const Register s67;
DYNINST_EXPORT extern const Register s68;
DYNINST_EXPORT extern const Register s69;
DYNINST_EXPORT extern const Register s70;
DYNINST_EXPORT extern const Register s71;
DYNINST_EXPORT extern const Register s72;
DYNINST_EXPORT extern const Register s73;
DYNINST_EXPORT extern const Register s74;
DYNINST_EXPORT extern const Register s75;
DYNINST_EXPORT extern const Register s76;
DYNINST_EXPORT extern const Register s77;
DYNINST_EXPORT extern const Register s78;
DYNINST_EXPORT extern const Register s79;
DYNINST_EXPORT extern const Register s80;
DYNINST_EXPORT extern const Register s81;
DYNINST_EXPORT extern const Register s82;
DYNINST_EXPORT extern const Register s83;
DYNINST_EXPORT extern const Register s84;
DYNINST_EXPORT extern const Register s85;
DYNINST_EXPORT extern const Register s86;
DYNINST_EXPORT extern const Register s87;
DYNINST_EXPORT extern const Register s88;
DYNINST_EXPORT extern const Register s89;
DYNINST_EXPORT extern const Register s90;
DYNINST_EXPORT extern const Register s91;
DYNINST_EXPORT extern const Register s92;
DYNINST_EXPORT extern const Register s93;
DYNINST_EXPORT extern const Register s94;
DYNINST_EXPORT extern const Register s95;
DYNINST_EXPORT extern const Register s96;
DYNINST_EXPORT extern const Register s97;
DYNINST_EXPORT extern const Register s98;
DYNINST_EXPORT extern const Register s99;
DYNINST_EXPORT extern const Register s100;
DYNINST_EXPORT extern const Register s101;
DYNINST_EXPORT extern const Register flat_scratch_lo;
DYNINST_EXPORT extern const Register flat_scratch_hi;
DYNINST_EXPORT extern const Register xnack_mask_lo;
DYNINST_EXPORT extern const Register xnack_mask_hi;
DYNINST_EXPORT extern const Register vcc_lo;
DYNINST_EXPORT extern const Register vcc_hi;
DYNINST_EXPORT extern const Register exec_lo;
DYNINST_EXPORT extern const Register exec_hi;

// Vector registers
DYNINST_EXPORT extern const Register v0;
DYNINST_EXPORT extern const Register v1;
DYNINST_EXPORT extern const Register v2;
DYNINST_EXPORT extern const Register v3;
DYNINST_EXPORT extern const Register v4;
DYNINST_EXPORT extern const Register v5;
DYNINST_EXPORT extern const Register v6;
DYNINST_EXPORT extern const Register v7;
DYNINST_EXPORT extern const Register v8;
DYNINST_EXPORT extern const Register v9;
DYNINST_EXPORT extern const Register v10;
DYNINST_EXPORT extern const Register v11;
DYNINST_EXPORT extern const Register v12;
DYNINST_EXPORT extern const Register v13;
DYNINST_EXPORT extern const Register v14;
DYNINST_EXPORT extern const Register v15;
DYNINST_EXPORT extern const Register v16;
DYNINST_EXPORT extern const Register v17;
DYNINST_EXPORT extern const Register v18;
DYNINST_EXPORT extern const Register v19;
DYNINST_EXPORT extern const Register v20;
DYNINST_EXPORT extern const Register v21;
DYNINST_EXPORT extern const Register v22;
DYNINST_EXPORT extern const Register v23;
DYNINST_EXPORT extern const Register v24;
DYNINST_EXPORT extern const Register v25;
DYNINST_EXPORT extern const Register v26;
DYNINST_EXPORT extern const Register v27;
DYNINST_EXPORT extern const Register v28;
DYNINST_EXPORT extern const Register v29;
DYNINST_EXPORT extern const Register v30;
DYNINST_EXPORT extern const Register v31;
DYNINST_EXPORT extern const Register v32;
DYNINST_EXPORT extern const Register v33;
DYNINST_EXPORT extern const Register v34;
DYNINST_EXPORT extern const Register v35;
DYNINST_EXPORT extern const Register v36;
DYNINST_EXPORT extern const Register v37;
DYNINST_EXPORT extern const Register v38;
DYNINST_EXPORT extern const Register v39;
DYNINST_EXPORT extern const Register v40;
DYNINST_EXPORT extern const Register v41;
DYNINST_EXPORT extern const Register v42;
DYNINST_EXPORT extern const Register v43;
DYNINST_EXPORT extern const Register v44;
DYNINST_EXPORT extern const Register v45;
DYNINST_EXPORT extern const Register v46;
DYNINST_EXPORT extern const Register v47;
DYNINST_EXPORT extern const Register v48;
DYNINST_EXPORT extern const Register v49;
DYNINST_EXPORT extern const Register v50;
DYNINST_EXPORT extern const Register v51;
DYNINST_EXPORT extern const Register v52;
DYNINST_EXPORT extern const Register v53;
DYNINST_EXPORT extern const Register v54;
DYNINST_EXPORT extern const Register v55;
DYNINST_EXPORT extern const Register v56;
DYNINST_EXPORT extern const Register v57;
DYNINST_EXPORT extern const Register v58;
DYNINST_EXPORT extern const Register v59;
DYNINST_EXPORT extern const Register v60;
DYNINST_EXPORT extern const Register v61;
DYNINST_EXPORT extern const Register v62;
DYNINST_EXPORT extern const Register v63;
DYNINST_EXPORT extern const Register v64;
DYNINST_EXPORT extern const Register v65;
DYNINST_EXPORT extern const Register v66;
DYNINST_EXPORT extern const Register v67;
DYNINST_EXPORT extern const Register v68;
DYNINST_EXPORT extern const Register v69;
DYNINST_EXPORT extern const Register v70;
DYNINST_EXPORT extern const Register v71;
DYNINST_EXPORT extern const Register v72;
DYNINST_EXPORT extern const Register v73;
DYNINST_EXPORT extern const Register v74;
DYNINST_EXPORT extern const Register v75;
DYNINST_EXPORT extern const Register v76;
DYNINST_EXPORT extern const Register v77;
DYNINST_EXPORT extern const Register v78;
DYNINST_EXPORT extern const Register v79;
DYNINST_EXPORT extern const Register v80;
DYNINST_EXPORT extern const Register v81;
DYNINST_EXPORT extern const Register v82;
DYNINST_EXPORT extern const Register v83;
DYNINST_EXPORT extern const Register v84;
DYNINST_EXPORT extern const Register v85;
DYNINST_EXPORT extern const Register v86;
DYNINST_EXPORT extern const Register v87;
DYNINST_EXPORT extern const Register v88;
DYNINST_EXPORT extern const Register v89;
DYNINST_EXPORT extern const Register v90;
DYNINST_EXPORT extern const Register v91;
DYNINST_EXPORT extern const Register v92;
DYNINST_EXPORT extern const Register v93;
DYNINST_EXPORT extern const Register v94;
DYNINST_EXPORT extern const Register v95;
DYNINST_EXPORT extern const Register v96;
DYNINST_EXPORT extern const Register v97;
DYNINST_EXPORT extern const Register v98;
DYNINST_EXPORT extern const Register v99;
DYNINST_EXPORT extern const Register v100;
DYNINST_EXPORT extern const Register v101;
DYNINST_EXPORT extern const Register v102;
DYNINST_EXPORT extern const Register v103;
DYNINST_EXPORT extern const Register v104;
DYNINST_EXPORT extern const Register v105;
DYNINST_EXPORT extern const Register v106;
DYNINST_EXPORT extern const Register v107;
DYNINST_EXPORT extern const Register v108;
DYNINST_EXPORT extern const Register v109;
DYNINST_EXPORT extern const Register v110;
DYNINST_EXPORT extern const Register v111;
DYNINST_EXPORT extern const Register v112;
DYNINST_EXPORT extern const Register v113;
DYNINST_EXPORT extern const Register v114;
DYNINST_EXPORT extern const Register v115;
DYNINST_EXPORT extern const Register v116;
DYNINST_EXPORT extern const Register v117;
DYNINST_EXPORT extern const Register v118;
DYNINST_EXPORT extern const Register v119;
DYNINST_EXPORT extern const Register v120;
DYNINST_EXPORT extern const Register v121;
DYNINST_EXPORT extern const Register v122;
DYNINST_EXPORT extern const Register v123;
DYNINST_EXPORT extern const Register v124;
DYNINST_EXPORT extern const Register v125;
DYNINST_EXPORT extern const Register v126;
DYNINST_EXPORT extern const Register v127;
DYNINST_EXPORT extern const Register v128;
DYNINST_EXPORT extern const Register v129;
DYNINST_EXPORT extern const Register v130;
DYNINST_EXPORT extern const Register v131;
DYNINST_EXPORT extern const Register v132;
DYNINST_EXPORT extern const Register v133;
DYNINST_EXPORT extern const Register v134;
DYNINST_EXPORT extern const Register v135;
DYNINST_EXPORT extern const Register v136;
DYNINST_EXPORT extern const Register v137;
DYNINST_EXPORT extern const Register v138;
DYNINST_EXPORT extern const Register v139;
DYNINST_EXPORT extern const Register v140;
DYNINST_EXPORT extern const Register v141;
DYNINST_EXPORT extern const Register v142;
DYNINST_EXPORT extern const Register v143;
DYNINST_EXPORT extern const Register v144;
DYNINST_EXPORT extern const Register v145;
DYNINST_EXPORT extern const Register v146;
DYNINST_EXPORT extern const Register v147;
DYNINST_EXPORT extern const Register v148;
DYNINST_EXPORT extern const Register v149;
DYNINST_EXPORT extern const Register v150;
DYNINST_EXPORT extern const Register v151;
DYNINST_EXPORT extern const Register v152;
DYNINST_EXPORT extern const Register v153;
DYNINST_EXPORT extern const Register v154;
DYNINST_EXPORT extern const Register v155;
DYNINST_EXPORT extern const Register v156;
DYNINST_EXPORT extern const Register v157;
DYNINST_EXPORT extern const Register v158;
DYNINST_EXPORT extern const Register v159;
DYNINST_EXPORT extern const Register v160;
DYNINST_EXPORT extern const Register v161;
DYNINST_EXPORT extern const Register v162;
DYNINST_EXPORT extern const Register v163;
DYNINST_EXPORT extern const Register v164;
DYNINST_EXPORT extern const Register v165;
DYNINST_EXPORT extern const Register v166;
DYNINST_EXPORT extern const Register v167;
DYNINST_EXPORT extern const Register v168;
DYNINST_EXPORT extern const Register v169;
DYNINST_EXPORT extern const Register v170;
DYNINST_EXPORT extern const Register v171;
DYNINST_EXPORT extern const Register v172;
DYNINST_EXPORT extern const Register v173;
DYNINST_EXPORT extern const Register v174;
DYNINST_EXPORT extern const Register v175;
DYNINST_EXPORT extern const Register v176;
DYNINST_EXPORT extern const Register v177;
DYNINST_EXPORT extern const Register v178;
DYNINST_EXPORT extern const Register v179;
DYNINST_EXPORT extern const Register v180;
DYNINST_EXPORT extern const Register v181;
DYNINST_EXPORT extern const Register v182;
DYNINST_EXPORT extern const Register v183;
DYNINST_EXPORT extern const Register v184;
DYNINST_EXPORT extern const Register v185;
DYNINST_EXPORT extern const Register v186;
DYNINST_EXPORT extern const Register v187;
DYNINST_EXPORT extern const Register v188;
DYNINST_EXPORT extern const Register v189;
DYNINST_EXPORT extern const Register v190;
DYNINST_EXPORT extern const Register v191;
DYNINST_EXPORT extern const Register v192;
DYNINST_EXPORT extern const Register v193;
DYNINST_EXPORT extern const Register v194;
DYNINST_EXPORT extern const Register v195;
DYNINST_EXPORT extern const Register v196;
DYNINST_EXPORT extern const Register v197;
DYNINST_EXPORT extern const Register v198;
DYNINST_EXPORT extern const Register v199;
DYNINST_EXPORT extern const Register v200;
DYNINST_EXPORT extern const Register v201;
DYNINST_EXPORT extern const Register v202;
DYNINST_EXPORT extern const Register v203;
DYNINST_EXPORT extern const Register v204;
DYNINST_EXPORT extern const Register v205;
DYNINST_EXPORT extern const Register v206;
DYNINST_EXPORT extern const Register v207;
DYNINST_EXPORT extern const Register v208;
DYNINST_EXPORT extern const Register v209;
DYNINST_EXPORT extern const Register v210;
DYNINST_EXPORT extern const Register v211;
DYNINST_EXPORT extern const Register v212;
DYNINST_EXPORT extern const Register v213;
DYNINST_EXPORT extern const Register v214;
DYNINST_EXPORT extern const Register v215;
DYNINST_EXPORT extern const Register v216;
DYNINST_EXPORT extern const Register v217;
DYNINST_EXPORT extern const Register v218;
DYNINST_EXPORT extern const Register v219;
DYNINST_EXPORT extern const Register v220;
DYNINST_EXPORT extern const Register v221;
DYNINST_EXPORT extern const Register v222;
DYNINST_EXPORT extern const Register v223;
DYNINST_EXPORT extern const Register v224;
DYNINST_EXPORT extern const Register v225;
DYNINST_EXPORT extern const Register v226;
DYNINST_EXPORT extern const Register v227;
DYNINST_EXPORT extern const Register v228;
DYNINST_EXPORT extern const Register v229;
DYNINST_EXPORT extern const Register v230;
DYNINST_EXPORT extern const Register v231;
DYNINST_EXPORT extern const Register v232;
DYNINST_EXPORT extern const Register v233;
DYNINST_EXPORT extern const Register v234;
DYNINST_EXPORT extern const Register v235;
DYNINST_EXPORT extern const Register v236;
DYNINST_EXPORT extern const Register v237;
DYNINST_EXPORT extern const Register v238;
DYNINST_EXPORT extern const Register v239;
DYNINST_EXPORT extern const Register v240;
DYNINST_EXPORT extern const Register v241;
DYNINST_EXPORT extern const Register v242;
DYNINST_EXPORT extern const Register v243;
DYNINST_EXPORT extern const Register v244;
DYNINST_EXPORT extern const Register v245;
DYNINST_EXPORT extern const Register v246;
DYNINST_EXPORT extern const Register v247;
DYNINST_EXPORT extern const Register v248;
DYNINST_EXPORT extern const Register v249;
DYNINST_EXPORT extern const Register v250;
DYNINST_EXPORT extern const Register v251;
DYNINST_EXPORT extern const Register v252;
DYNINST_EXPORT extern const Register v253;
DYNINST_EXPORT extern const Register v254;
DYNINST_EXPORT extern const Register v255;

DYNINST_EXPORT extern const Register ignored;

} // namespace RegisterConstants

typedef const unsigned int insn_mask;

typedef union {
  unsigned char byte[4];
  unsigned int raw;
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

// Helps to mitigate host/target endian mismatches
// unsigned int swapBytesIfNeeded(unsigned int i);

class DYNINST_EXPORT instruction {
private:
  instructUnion insn_;

public:
  // instruction() { insn_.raw = 0; }
  // instruction(unsigned int raw) {
  //   // Don't flip bits here.  Input is already in host byte order.
  //   insn_.raw = raw;
  // }
  // // Pointer creation method
  // instruction(const void *ptr) { insn_ = *((const instructUnion *)ptr); }
  instruction(const void *ptr, bool) { insn_ = *((const instructUnion *)ptr); }
  //
  // instruction(const instruction &insn) : insn_(insn.insn_) {}
  // instruction(instructUnion &insn) : insn_(insn) {}

  // instruction *copy() const;
  //
  // void clear() { insn_.raw = 0; }
  // void setInstruction(codeBuf_t *ptr, Dyninst::Address = 0);
  // void setBits(unsigned int pos, unsigned int len, unsigned int value) {
  //   unsigned int mask;
  //
  //   mask = ~((unsigned int)(~0) << len);
  //   value = value & mask;
  //
  //   mask = ~(mask << pos);
  //   value = value << pos;
  //
  //   insn_.raw = insn_.raw & mask;
  //   insn_.raw = insn_.raw | value;
  // }
  //
  unsigned int asInt() const { return insn_.raw; }
  // void setInstruction(unsigned char *ptr, Dyninst::Address = 0);
  //
  // // To solve host/target endian mismatches
  // static int signExtend(unsigned int i, unsigned int pos);
  // static instructUnion &swapBytes(instructUnion &i);
  //
  // // We need instruction::size() all _over_ the place.
  static unsigned size() { return sizeof(instructUnion); }
  //
  // Dyninst::Address getBranchOffset() const;
  // Dyninst::Address getBranchTargetAddress() const;
  // void setBranchOffset(Dyninst::Address newOffset);
  //
  // // Returns -1 if we can't do a branch due to architecture limitations
  // static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
  // static unsigned jumpSize(Dyninst::Address disp, unsigned addr_width);
  // static unsigned maxJumpSize(unsigned addr_width);
  //
  // static unsigned maxInterFunctionJumpSize(unsigned addr_width);
  //
  // // return the type of the instruction
  // unsigned type() const;
  //
  // // return a pointer to the instruction
  // const unsigned char *ptr() const { return (const unsigned char *)&insn_; }
  //
  // unsigned opcode() const;
  //
  // // Local version
  // bool isInsnType(const unsigned mask, const unsigned match) const {
  //   return ((insn_.raw & mask) == match);
  // }
  //
  // Dyninst::Address getTarget(Dyninst::Address insnAddr) const;
  //
  // unsigned spaceToRelocate() const;
  // bool getUsedRegs(std::vector<int> &regs);
  //
  // bool valid() const {
  //   assert(0);
  //   return false;
  // }
  //
  // bool isCall() const;
  //
  // static bool isAligned(Dyninst::Address addr) { return !(addr & 0x3); }
  //
  // bool isBranchReg() const;
  // bool isCondBranch() const;
  // bool isUncondBranch() const;
  // bool isThunk() const;
  //
  // bool isCleaningRet() const { return false; }
  //
  // bool isAtomicLoad() const;
  // bool isAtomicStore() const;
  //
  // // inferface for being called outside this class
  // unsigned getTargetReg() const;
  // unsigned getBranchTargetReg() const;
};

} // namespace NS_amdgpu
// end of NS_amdgpu

#endif
