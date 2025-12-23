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
#include <assert.h>
#include <vector>
class AddressSpace;

namespace NS_amdgpu {
// AMDGPU GFX908 register enumeration. Note that THIS enumeration is for Dyninst's abstraction, and the enumerations are NOT meant to map to the architectural register number here.
// We map these registers to MachRegisters in RegisterConversion-amdgpu.C
// For information on architectural registers see MachRegister in dyn_regs.h
typedef enum {
sgpr0,
sgpr1,
sgpr2,
sgpr3,
sgpr4,
sgpr5,
sgpr6,
sgpr7,
sgpr8,
sgpr9,
sgpr10,
sgpr11,
sgpr12,
sgpr13,
sgpr14,
sgpr15,
sgpr16,
sgpr17,
sgpr18,
sgpr19,
sgpr20,
sgpr21,
sgpr22,
sgpr23,
sgpr24,
sgpr25,
sgpr26,
sgpr27,
sgpr28,
sgpr29,
sgpr30,
sgpr31,
sgpr32,
sgpr33,
sgpr34,
sgpr35,
sgpr36,
sgpr37,
sgpr38,
sgpr39,
sgpr40,
sgpr41,
sgpr42,
sgpr43,
sgpr44,
sgpr45,
sgpr46,
sgpr47,
sgpr48,
sgpr49,
sgpr50,
sgpr51,
sgpr52,
sgpr53,
sgpr54,
sgpr55,
sgpr56,
sgpr57,
sgpr58,
sgpr59,
sgpr60,
sgpr61,
sgpr62,
sgpr63,
sgpr64,
sgpr65,
sgpr66,
sgpr67,
sgpr68,
sgpr69,
sgpr70,
sgpr71,
sgpr72,
sgpr73,
sgpr74,
sgpr75,
sgpr76,
sgpr77,
sgpr78,
sgpr79,
sgpr80,
sgpr81,
sgpr82,
sgpr83,
sgpr84,
sgpr85,
sgpr86,
sgpr87,
sgpr88,
sgpr89,
sgpr90,
sgpr91,
sgpr92,
sgpr93,
sgpr94,
sgpr95,
sgpr96,
sgpr97,
sgpr98,
sgpr99,
sgpr100,
sgpr101,
flat_scratch_lo,
flat_scratch_hi,
xnack_mask_lo,
xnack_mask_hi,
vcc_lo,
vcc_hi,
ttmp0,
ttmp1,
ttmp2,
ttmp3,
ttmp4,
ttmp5,
ttmp6,
ttmp7,
ttmp8,
ttmp9,
ttmp10,
ttmp11,
ttmp12,
ttmp13,
ttmp14,
ttmp15,
m0,
exec_lo,
exec_hi,

// now do vector registers
v0,
v1,
v2,
v3,
v4,
v5,
v6,
v7,
v8,
v9,
v10,
v11,
v12,
v13,
v14,
v15,
v16,
v17,
v18,
v19,
v20,
v21,
v22,
v23,
v24,
v25,
v26,
v27,
v28,
v29,
v30,
v31,
v32,
v33,
v34,
v35,
v36,
v37,
v38,
v39,
v40,
v41,
v42,
v43,
v44,
v45,
v46,
v47,
v48,
v49,
v50,
v51,
v52,
v53,
v54,
v55,
v56,
v57,
v58,
v59,
v60,
v61,
v62,
v63,
v64,
v65,
v66,
v67,
v68,
v69,
v70,
v71,
v72,
v73,
v74,
v75,
v76,
v77,
v78,
v79,
v80,
v81,
v82,
v83,
v84,
v85,
v86,
v87,
v88,
v89,
v90,
v91,
v92,
v93,
v94,
v95,
v96,
v97,
v98,
v99,
v100,
v101,
v102,
v103,
v104,
v105,
v106,
v107,
v108,
v109,
v110,
v111,
v112,
v113,
v114,
v115,
v116,
v117,
v118,
v119,
v120,
v121,
v122,
v123,
v124,
v125,
v126,
v127,
v128,
v129,
v130,
v131,
v132,
v133,
v134,
v135,
v136,
v137,
v138,
v139,
v140,
v141,
v142,
v143,
v144,
v145,
v146,
v147,
v148,
v149,
v150,
v151,
v152,
v153,
v154,
v155,
v156,
v157,
v158,
v159,
v160,
v161,
v162,
v163,
v164,
v165,
v166,
v167,
v168,
v169,
v170,
v171,
v172,
v173,
v174,
v175,
v176,
v177,
v178,
v179,
v180,
v181,
v182,
v183,
v184,
v185,
v186,
v187,
v188,
v189,
v190,
v191,
v192,
v193,
v194,
v195,
v196,
v197,
v198,
v199,
v200,
v201,
v202,
v203,
v204,
v205,
v206,
v207,
v208,
v209,
v210,
v211,
v212,
v213,
v214,
v215,
v216,
v217,
v218,
v219,
v220,
v221,
v222,
v223,
v224,
v225,
v226,
v227,
v228,
v229,
v230,
v231,
v232,
v233,
v234,
v235,
v236,
v237,
v238,
v239,
v240,
v241,
v242,
v243,
v244,
v245,
v246,
v247,
v248,
v249,
v250,
v251,
v252,
v253,
v254,
v255,

ignored} amdgpuRegisters_t;


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
