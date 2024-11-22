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

#include "RegisterConversion.h"
#include "registerSpace.h"

#include <map>
#include <boost/assign/list_of.hpp>

#include "Register.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "registerSpace.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

// AMDGPU has 32-bit registers, and we use a pair of registers for 64-bit values.
// regToMachReg64 is here only to prevent linker error because parts of the x86
// code generator refer to it. This is a temporary hack until the conditional
// compilation and legacy cruft goes away.
multimap<Register, MachRegister> regToMachReg64 = map_list_of (0, 0);

multimap<Register, MachRegister> regToMachReg32 = map_list_of
  (registerSpace::s0,  amdgpu_gfx908::s0)
  (registerSpace::s1,  amdgpu_gfx908::s1)
  (registerSpace::s2,  amdgpu_gfx908::s2)
  (registerSpace::s3,  amdgpu_gfx908::s3)
  (registerSpace::s4,  amdgpu_gfx908::s4)
  (registerSpace::s5,  amdgpu_gfx908::s5)
  (registerSpace::s6,  amdgpu_gfx908::s6)
  (registerSpace::s7,  amdgpu_gfx908::s7)
  (registerSpace::s8,  amdgpu_gfx908::s8)
  (registerSpace::s9,  amdgpu_gfx908::s9)
  (registerSpace::s10, amdgpu_gfx908::s10)
  (registerSpace::s11, amdgpu_gfx908::s11)
  (registerSpace::s12, amdgpu_gfx908::s12)
  (registerSpace::s13, amdgpu_gfx908::s13)
  (registerSpace::s14, amdgpu_gfx908::s14)
  (registerSpace::s15, amdgpu_gfx908::s15)
  (registerSpace::s16, amdgpu_gfx908::s16)
  (registerSpace::s17, amdgpu_gfx908::s17)
  (registerSpace::s18, amdgpu_gfx908::s18)
  (registerSpace::s19, amdgpu_gfx908::s19)
  (registerSpace::s20, amdgpu_gfx908::s20)
  (registerSpace::s21, amdgpu_gfx908::s21)
  (registerSpace::s22, amdgpu_gfx908::s22)
  (registerSpace::s23, amdgpu_gfx908::s23)
  (registerSpace::s24, amdgpu_gfx908::s24)
  (registerSpace::s25, amdgpu_gfx908::s25)
  (registerSpace::s26, amdgpu_gfx908::s26)
  (registerSpace::s27, amdgpu_gfx908::s27)
  (registerSpace::s28, amdgpu_gfx908::s28)
  (registerSpace::s29, amdgpu_gfx908::s29)
  (registerSpace::s30, amdgpu_gfx908::s30)
  (registerSpace::s31, amdgpu_gfx908::s31)
  (registerSpace::s32, amdgpu_gfx908::s32)
  (registerSpace::s33, amdgpu_gfx908::s33)
  (registerSpace::s34, amdgpu_gfx908::s34)
  (registerSpace::s35, amdgpu_gfx908::s35)
  (registerSpace::s36, amdgpu_gfx908::s36)
  (registerSpace::s37, amdgpu_gfx908::s37)
  (registerSpace::s38, amdgpu_gfx908::s38)
  (registerSpace::s39, amdgpu_gfx908::s39)
  (registerSpace::s40, amdgpu_gfx908::s40)
  (registerSpace::s41, amdgpu_gfx908::s41)
  (registerSpace::s42, amdgpu_gfx908::s42)
  (registerSpace::s43, amdgpu_gfx908::s43)
  (registerSpace::s44, amdgpu_gfx908::s44)
  (registerSpace::s45, amdgpu_gfx908::s45)
  (registerSpace::s46, amdgpu_gfx908::s46)
  (registerSpace::s47, amdgpu_gfx908::s47)
  (registerSpace::s48, amdgpu_gfx908::s48)
  (registerSpace::s49, amdgpu_gfx908::s49)
  (registerSpace::s50, amdgpu_gfx908::s50)
  (registerSpace::s51, amdgpu_gfx908::s51)
  (registerSpace::s52, amdgpu_gfx908::s52)
  (registerSpace::s53, amdgpu_gfx908::s53)
  (registerSpace::s54, amdgpu_gfx908::s54)
  (registerSpace::s55, amdgpu_gfx908::s55)
  (registerSpace::s56, amdgpu_gfx908::s56)
  (registerSpace::s57, amdgpu_gfx908::s57)
  (registerSpace::s58, amdgpu_gfx908::s58)
  (registerSpace::s59, amdgpu_gfx908::s59)
  (registerSpace::s60, amdgpu_gfx908::s60)
  (registerSpace::s61, amdgpu_gfx908::s61)
  (registerSpace::s62, amdgpu_gfx908::s62)
  (registerSpace::s63, amdgpu_gfx908::s63)
  (registerSpace::s64, amdgpu_gfx908::s64)
  (registerSpace::s65, amdgpu_gfx908::s65)
  (registerSpace::s66, amdgpu_gfx908::s66)
  (registerSpace::s67, amdgpu_gfx908::s67)
  (registerSpace::s68, amdgpu_gfx908::s68)
  (registerSpace::s69, amdgpu_gfx908::s69)
  (registerSpace::s70, amdgpu_gfx908::s70)
  (registerSpace::s71, amdgpu_gfx908::s71)
  (registerSpace::s72, amdgpu_gfx908::s72)
  (registerSpace::s73, amdgpu_gfx908::s73)
  (registerSpace::s74, amdgpu_gfx908::s74)
  (registerSpace::s75, amdgpu_gfx908::s75)
  (registerSpace::s76, amdgpu_gfx908::s76)
  (registerSpace::s77, amdgpu_gfx908::s77)
  (registerSpace::s78, amdgpu_gfx908::s78)
  (registerSpace::s79, amdgpu_gfx908::s79)
  (registerSpace::s80, amdgpu_gfx908::s80)
  (registerSpace::s81, amdgpu_gfx908::s81)
  (registerSpace::s82, amdgpu_gfx908::s82)
  (registerSpace::s83, amdgpu_gfx908::s83)
  (registerSpace::s84, amdgpu_gfx908::s84)
  (registerSpace::s85, amdgpu_gfx908::s85)
  (registerSpace::s86, amdgpu_gfx908::s86)
  (registerSpace::s87, amdgpu_gfx908::s87)
  (registerSpace::s88, amdgpu_gfx908::s88)
  (registerSpace::s89, amdgpu_gfx908::s89)
  (registerSpace::s90, amdgpu_gfx908::s90)
  (registerSpace::s91, amdgpu_gfx908::s91)
  (registerSpace::s92, amdgpu_gfx908::s92)
  (registerSpace::s93, amdgpu_gfx908::s93)
  (registerSpace::s94, amdgpu_gfx908::s94)
  (registerSpace::s95, amdgpu_gfx908::s95)
  (registerSpace::s96, amdgpu_gfx908::s96)
  (registerSpace::s97, amdgpu_gfx908::s97)
  (registerSpace::s98, amdgpu_gfx908::s98)
  (registerSpace::s99, amdgpu_gfx908::s99)
  (registerSpace::s100, amdgpu_gfx908::s100)
  (registerSpace::s101, amdgpu_gfx908::s101)
  (registerSpace::flat_scratch_lo, amdgpu_gfx908::flat_scratch_lo)
  (registerSpace::flat_scratch_hi, amdgpu_gfx908::flat_scratch_hi)
  (registerSpace::xnack_mask_lo, amdgpu_gfx908::xnack_mask_lo)
  (registerSpace::xnack_mask_hi, amdgpu_gfx908::xnack_mask_hi)
  (registerSpace::vcc_lo, amdgpu_gfx908::vcc_lo)
  (registerSpace::vcc_hi, amdgpu_gfx908::vcc_hi)
  (registerSpace::ttmp0,  amdgpu_gfx908::ttmp0)
  (registerSpace::ttmp1,  amdgpu_gfx908::ttmp1)
  (registerSpace::ttmp2,  amdgpu_gfx908::ttmp2)
  (registerSpace::ttmp3,  amdgpu_gfx908::ttmp3)
  (registerSpace::ttmp4,  amdgpu_gfx908::ttmp4)
  (registerSpace::ttmp5,  amdgpu_gfx908::ttmp5)
  (registerSpace::ttmp6,  amdgpu_gfx908::ttmp6)
  (registerSpace::ttmp7,  amdgpu_gfx908::ttmp7)
  (registerSpace::ttmp8,  amdgpu_gfx908::ttmp8)
  (registerSpace::ttmp9,  amdgpu_gfx908::ttmp9)
  (registerSpace::ttmp10, amdgpu_gfx908::ttmp10)
  (registerSpace::ttmp11, amdgpu_gfx908::ttmp11)
  (registerSpace::ttmp12, amdgpu_gfx908::ttmp12)
  (registerSpace::ttmp13, amdgpu_gfx908::ttmp13)
  (registerSpace::ttmp14, amdgpu_gfx908::ttmp14)
  (registerSpace::ttmp15, amdgpu_gfx908::ttmp15)
  (registerSpace::m0,     amdgpu_gfx908::m0)
  (registerSpace::exec_lo, amdgpu_gfx908::exec_lo)
  (registerSpace::exec_hi, amdgpu_gfx908::exec_hi)

  // now do vector registers
  (registerSpace::v0,  amdgpu_gfx908::v0)
  (registerSpace::v1,  amdgpu_gfx908::v1)
  (registerSpace::v2,  amdgpu_gfx908::v2)
  (registerSpace::v3,  amdgpu_gfx908::v3)
  (registerSpace::v4,  amdgpu_gfx908::v4)
  (registerSpace::v5,  amdgpu_gfx908::v5)
  (registerSpace::v6,  amdgpu_gfx908::v6)
  (registerSpace::v7,  amdgpu_gfx908::v7)
  (registerSpace::v8,  amdgpu_gfx908::v8)
  (registerSpace::v9,  amdgpu_gfx908::v9)
  (registerSpace::v10, amdgpu_gfx908::v10)
  (registerSpace::v11, amdgpu_gfx908::v11)
  (registerSpace::v12, amdgpu_gfx908::v12)
  (registerSpace::v13, amdgpu_gfx908::v13)
  (registerSpace::v14, amdgpu_gfx908::v14)
  (registerSpace::v15, amdgpu_gfx908::v15)
  (registerSpace::v16, amdgpu_gfx908::v16)
  (registerSpace::v17, amdgpu_gfx908::v17)
  (registerSpace::v18, amdgpu_gfx908::v18)
  (registerSpace::v19, amdgpu_gfx908::v19)
  (registerSpace::v20, amdgpu_gfx908::v20)
  (registerSpace::v21, amdgpu_gfx908::v21)
  (registerSpace::v22, amdgpu_gfx908::v22)
  (registerSpace::v23, amdgpu_gfx908::v23)
  (registerSpace::v24, amdgpu_gfx908::v24)
  (registerSpace::v25, amdgpu_gfx908::v25)
  (registerSpace::v26, amdgpu_gfx908::v26)
  (registerSpace::v27, amdgpu_gfx908::v27)
  (registerSpace::v28, amdgpu_gfx908::v28)
  (registerSpace::v29, amdgpu_gfx908::v29)
  (registerSpace::v30, amdgpu_gfx908::v30)
  (registerSpace::v31, amdgpu_gfx908::v31)
  (registerSpace::v32, amdgpu_gfx908::v32)
  (registerSpace::v33, amdgpu_gfx908::v33)
  (registerSpace::v34, amdgpu_gfx908::v34)
  (registerSpace::v35, amdgpu_gfx908::v35)
  (registerSpace::v36, amdgpu_gfx908::v36)
  (registerSpace::v37, amdgpu_gfx908::v37)
  (registerSpace::v38, amdgpu_gfx908::v38)
  (registerSpace::v39, amdgpu_gfx908::v39)
  (registerSpace::v40, amdgpu_gfx908::v40)
  (registerSpace::v41, amdgpu_gfx908::v41)
  (registerSpace::v42, amdgpu_gfx908::v42)
  (registerSpace::v43, amdgpu_gfx908::v43)
  (registerSpace::v44, amdgpu_gfx908::v44)
  (registerSpace::v45, amdgpu_gfx908::v45)
  (registerSpace::v46, amdgpu_gfx908::v46)
  (registerSpace::v47, amdgpu_gfx908::v47)
  (registerSpace::v48, amdgpu_gfx908::v48)
  (registerSpace::v49, amdgpu_gfx908::v49)
  (registerSpace::v50, amdgpu_gfx908::v50)
  (registerSpace::v51, amdgpu_gfx908::v51)
  (registerSpace::v52, amdgpu_gfx908::v52)
  (registerSpace::v53, amdgpu_gfx908::v53)
  (registerSpace::v54, amdgpu_gfx908::v54)
  (registerSpace::v55, amdgpu_gfx908::v55)
  (registerSpace::v56, amdgpu_gfx908::v56)
  (registerSpace::v57, amdgpu_gfx908::v57)
  (registerSpace::v58, amdgpu_gfx908::v58)
  (registerSpace::v59, amdgpu_gfx908::v59)
  (registerSpace::v60, amdgpu_gfx908::v60)
  (registerSpace::v61, amdgpu_gfx908::v61)
  (registerSpace::v62, amdgpu_gfx908::v62)
  (registerSpace::v63, amdgpu_gfx908::v63)
  (registerSpace::v64, amdgpu_gfx908::v64)
  (registerSpace::v65, amdgpu_gfx908::v65)
  (registerSpace::v66, amdgpu_gfx908::v66)
  (registerSpace::v67, amdgpu_gfx908::v67)
  (registerSpace::v68, amdgpu_gfx908::v68)
  (registerSpace::v69, amdgpu_gfx908::v69)
  (registerSpace::v70, amdgpu_gfx908::v70)
  (registerSpace::v71, amdgpu_gfx908::v71)
  (registerSpace::v72, amdgpu_gfx908::v72)
  (registerSpace::v73, amdgpu_gfx908::v73)
  (registerSpace::v74, amdgpu_gfx908::v74)
  (registerSpace::v75, amdgpu_gfx908::v75)
  (registerSpace::v76, amdgpu_gfx908::v76)
  (registerSpace::v77, amdgpu_gfx908::v77)
  (registerSpace::v78, amdgpu_gfx908::v78)
  (registerSpace::v79, amdgpu_gfx908::v79)
  (registerSpace::v80, amdgpu_gfx908::v80)
  (registerSpace::v81, amdgpu_gfx908::v81)
  (registerSpace::v82, amdgpu_gfx908::v82)
  (registerSpace::v83, amdgpu_gfx908::v83)
  (registerSpace::v84, amdgpu_gfx908::v84)
  (registerSpace::v85, amdgpu_gfx908::v85)
  (registerSpace::v86, amdgpu_gfx908::v86)
  (registerSpace::v87, amdgpu_gfx908::v87)
  (registerSpace::v88, amdgpu_gfx908::v88)
  (registerSpace::v89, amdgpu_gfx908::v89)
  (registerSpace::v90, amdgpu_gfx908::v90)
  (registerSpace::v91, amdgpu_gfx908::v91)
  (registerSpace::v92, amdgpu_gfx908::v92)
  (registerSpace::v93, amdgpu_gfx908::v93)
  (registerSpace::v94, amdgpu_gfx908::v94)
  (registerSpace::v95, amdgpu_gfx908::v95)
  (registerSpace::v96, amdgpu_gfx908::v96)
  (registerSpace::v97, amdgpu_gfx908::v97)
  (registerSpace::v98, amdgpu_gfx908::v98)
  (registerSpace::v99, amdgpu_gfx908::v99)
  (registerSpace::v100, amdgpu_gfx908::v100)
  (registerSpace::v101, amdgpu_gfx908::v101)
  (registerSpace::v102, amdgpu_gfx908::v102)
  (registerSpace::v103, amdgpu_gfx908::v103)
  (registerSpace::v104, amdgpu_gfx908::v104)
  (registerSpace::v105, amdgpu_gfx908::v105)
  (registerSpace::v106, amdgpu_gfx908::v106)
  (registerSpace::v107, amdgpu_gfx908::v107)
  (registerSpace::v108, amdgpu_gfx908::v108)
  (registerSpace::v109, amdgpu_gfx908::v109)
  (registerSpace::v110, amdgpu_gfx908::v110)
  (registerSpace::v111, amdgpu_gfx908::v111)
  (registerSpace::v112, amdgpu_gfx908::v112)
  (registerSpace::v113, amdgpu_gfx908::v113)
  (registerSpace::v114, amdgpu_gfx908::v114)
  (registerSpace::v115, amdgpu_gfx908::v115)
  (registerSpace::v116, amdgpu_gfx908::v116)
  (registerSpace::v117, amdgpu_gfx908::v117)
  (registerSpace::v118, amdgpu_gfx908::v118)
  (registerSpace::v119, amdgpu_gfx908::v119)
  (registerSpace::v120, amdgpu_gfx908::v120)
  (registerSpace::v121, amdgpu_gfx908::v121)
  (registerSpace::v122, amdgpu_gfx908::v122)
  (registerSpace::v123, amdgpu_gfx908::v123)
  (registerSpace::v124, amdgpu_gfx908::v124)
  (registerSpace::v125, amdgpu_gfx908::v125)
  (registerSpace::v126, amdgpu_gfx908::v126)
  (registerSpace::v127, amdgpu_gfx908::v127)
  (registerSpace::v128, amdgpu_gfx908::v128)
  (registerSpace::v129, amdgpu_gfx908::v129)
  (registerSpace::v130, amdgpu_gfx908::v130)
  (registerSpace::v131, amdgpu_gfx908::v131)
  (registerSpace::v132, amdgpu_gfx908::v132)
  (registerSpace::v133, amdgpu_gfx908::v133)
  (registerSpace::v134, amdgpu_gfx908::v134)
  (registerSpace::v135, amdgpu_gfx908::v135)
  (registerSpace::v136, amdgpu_gfx908::v136)
  (registerSpace::v137, amdgpu_gfx908::v137)
  (registerSpace::v138, amdgpu_gfx908::v138)
  (registerSpace::v139, amdgpu_gfx908::v139)
  (registerSpace::v140, amdgpu_gfx908::v140)
  (registerSpace::v141, amdgpu_gfx908::v141)
  (registerSpace::v142, amdgpu_gfx908::v142)
  (registerSpace::v143, amdgpu_gfx908::v143)
  (registerSpace::v144, amdgpu_gfx908::v144)
  (registerSpace::v145, amdgpu_gfx908::v145)
  (registerSpace::v146, amdgpu_gfx908::v146)
  (registerSpace::v147, amdgpu_gfx908::v147)
  (registerSpace::v148, amdgpu_gfx908::v148)
  (registerSpace::v149, amdgpu_gfx908::v149)
  (registerSpace::v150, amdgpu_gfx908::v150)
  (registerSpace::v151, amdgpu_gfx908::v151)
  (registerSpace::v152, amdgpu_gfx908::v152)
  (registerSpace::v153, amdgpu_gfx908::v153)
  (registerSpace::v154, amdgpu_gfx908::v154)
  (registerSpace::v155, amdgpu_gfx908::v155)
  (registerSpace::v156, amdgpu_gfx908::v156)
  (registerSpace::v157, amdgpu_gfx908::v157)
  (registerSpace::v158, amdgpu_gfx908::v158)
  (registerSpace::v159, amdgpu_gfx908::v159)
  (registerSpace::v160, amdgpu_gfx908::v160)
  (registerSpace::v161, amdgpu_gfx908::v161)
  (registerSpace::v162, amdgpu_gfx908::v162)
  (registerSpace::v163, amdgpu_gfx908::v163)
  (registerSpace::v164, amdgpu_gfx908::v164)
  (registerSpace::v165, amdgpu_gfx908::v165)
  (registerSpace::v166, amdgpu_gfx908::v166)
  (registerSpace::v167, amdgpu_gfx908::v167)
  (registerSpace::v168, amdgpu_gfx908::v168)
  (registerSpace::v169, amdgpu_gfx908::v169)
  (registerSpace::v170, amdgpu_gfx908::v170)
  (registerSpace::v171, amdgpu_gfx908::v171)
  (registerSpace::v172, amdgpu_gfx908::v172)
  (registerSpace::v173, amdgpu_gfx908::v173)
  (registerSpace::v174, amdgpu_gfx908::v174)
  (registerSpace::v175, amdgpu_gfx908::v175)
  (registerSpace::v176, amdgpu_gfx908::v176)
  (registerSpace::v177, amdgpu_gfx908::v177)
  (registerSpace::v178, amdgpu_gfx908::v178)
  (registerSpace::v179, amdgpu_gfx908::v179)
  (registerSpace::v180, amdgpu_gfx908::v180)
  (registerSpace::v181, amdgpu_gfx908::v181)
  (registerSpace::v182, amdgpu_gfx908::v182)
  (registerSpace::v183, amdgpu_gfx908::v183)
  (registerSpace::v184, amdgpu_gfx908::v184)
  (registerSpace::v185, amdgpu_gfx908::v185)
  (registerSpace::v186, amdgpu_gfx908::v186)
  (registerSpace::v187, amdgpu_gfx908::v187)
  (registerSpace::v188, amdgpu_gfx908::v188)
  (registerSpace::v189, amdgpu_gfx908::v189)
  (registerSpace::v190, amdgpu_gfx908::v190)
  (registerSpace::v191, amdgpu_gfx908::v191)
  (registerSpace::v192, amdgpu_gfx908::v192)
  (registerSpace::v193, amdgpu_gfx908::v193)
  (registerSpace::v194, amdgpu_gfx908::v194)
  (registerSpace::v195, amdgpu_gfx908::v195)
  (registerSpace::v196, amdgpu_gfx908::v196)
  (registerSpace::v197, amdgpu_gfx908::v197)
  (registerSpace::v198, amdgpu_gfx908::v198)
  (registerSpace::v199, amdgpu_gfx908::v199)
  (registerSpace::v200, amdgpu_gfx908::v200)
  (registerSpace::v201, amdgpu_gfx908::v201)
  (registerSpace::v202, amdgpu_gfx908::v202)
  (registerSpace::v203, amdgpu_gfx908::v203)
  (registerSpace::v204, amdgpu_gfx908::v204)
  (registerSpace::v205, amdgpu_gfx908::v205)
  (registerSpace::v206, amdgpu_gfx908::v206)
  (registerSpace::v207, amdgpu_gfx908::v207)
  (registerSpace::v208, amdgpu_gfx908::v208)
  (registerSpace::v209, amdgpu_gfx908::v209)
  (registerSpace::v210, amdgpu_gfx908::v210)
  (registerSpace::v211, amdgpu_gfx908::v211)
  (registerSpace::v212, amdgpu_gfx908::v212)
  (registerSpace::v213, amdgpu_gfx908::v213)
  (registerSpace::v214, amdgpu_gfx908::v214)
  (registerSpace::v215, amdgpu_gfx908::v215)
  (registerSpace::v216, amdgpu_gfx908::v216)
  (registerSpace::v217, amdgpu_gfx908::v217)
  (registerSpace::v218, amdgpu_gfx908::v218)
  (registerSpace::v219, amdgpu_gfx908::v219)
  (registerSpace::v220, amdgpu_gfx908::v220)
  (registerSpace::v221, amdgpu_gfx908::v221)
  (registerSpace::v222, amdgpu_gfx908::v222)
  (registerSpace::v223, amdgpu_gfx908::v223)
  (registerSpace::v224, amdgpu_gfx908::v224)
  (registerSpace::v225, amdgpu_gfx908::v225)
  (registerSpace::v226, amdgpu_gfx908::v226)
  (registerSpace::v227, amdgpu_gfx908::v227)
  (registerSpace::v228, amdgpu_gfx908::v228)
  (registerSpace::v229, amdgpu_gfx908::v229)
  (registerSpace::v230, amdgpu_gfx908::v230)
  (registerSpace::v231, amdgpu_gfx908::v231)
  (registerSpace::v232, amdgpu_gfx908::v232)
  (registerSpace::v233, amdgpu_gfx908::v233)
  (registerSpace::v234, amdgpu_gfx908::v234)
  (registerSpace::v235, amdgpu_gfx908::v235)
  (registerSpace::v236, amdgpu_gfx908::v236)
  (registerSpace::v237, amdgpu_gfx908::v237)
  (registerSpace::v238, amdgpu_gfx908::v238)
  (registerSpace::v239, amdgpu_gfx908::v239)
  (registerSpace::v240, amdgpu_gfx908::v240)
  (registerSpace::v241, amdgpu_gfx908::v241)
  (registerSpace::v242, amdgpu_gfx908::v242)
  (registerSpace::v243, amdgpu_gfx908::v243)
  (registerSpace::v244, amdgpu_gfx908::v244)
  (registerSpace::v245, amdgpu_gfx908::v245)
  (registerSpace::v246, amdgpu_gfx908::v246)
  (registerSpace::v247, amdgpu_gfx908::v247)
  (registerSpace::v248, amdgpu_gfx908::v248)
  (registerSpace::v249, amdgpu_gfx908::v249)
  (registerSpace::v250, amdgpu_gfx908::v250)
  (registerSpace::v251, amdgpu_gfx908::v251)
  (registerSpace::v252, amdgpu_gfx908::v252)
  (registerSpace::v253, amdgpu_gfx908::v253)
  (registerSpace::v254, amdgpu_gfx908::v254)
  (registerSpace::v255, amdgpu_gfx908::v255)
  ;


map<MachRegister, Register> reverseRegisterMap = map_list_of
  (amdgpu_gfx908::s0,  registerSpace::s0)
  (amdgpu_gfx908::s1,  registerSpace::s1)
  (amdgpu_gfx908::s2,  registerSpace::s2)
  (amdgpu_gfx908::s3,  registerSpace::s3)
  (amdgpu_gfx908::s4,  registerSpace::s4)
  (amdgpu_gfx908::s5,  registerSpace::s5)
  (amdgpu_gfx908::s6,  registerSpace::s6)
  (amdgpu_gfx908::s7,  registerSpace::s7)
  (amdgpu_gfx908::s8,  registerSpace::s8)
  (amdgpu_gfx908::s9,  registerSpace::s9)
  (amdgpu_gfx908::s10, registerSpace::s10)
  (amdgpu_gfx908::s11, registerSpace::s11)
  (amdgpu_gfx908::s12, registerSpace::s12)
  (amdgpu_gfx908::s13, registerSpace::s13)
  (amdgpu_gfx908::s14, registerSpace::s14)
  (amdgpu_gfx908::s15, registerSpace::s15)
  (amdgpu_gfx908::s16, registerSpace::s16)
  (amdgpu_gfx908::s17, registerSpace::s17)
  (amdgpu_gfx908::s18, registerSpace::s18)
  (amdgpu_gfx908::s19, registerSpace::s19)
  (amdgpu_gfx908::s20, registerSpace::s20)
  (amdgpu_gfx908::s21, registerSpace::s21)
  (amdgpu_gfx908::s22, registerSpace::s22)
  (amdgpu_gfx908::s23, registerSpace::s23)
  (amdgpu_gfx908::s24, registerSpace::s24)
  (amdgpu_gfx908::s25, registerSpace::s25)
  (amdgpu_gfx908::s26, registerSpace::s26)
  (amdgpu_gfx908::s27, registerSpace::s27)
  (amdgpu_gfx908::s28, registerSpace::s28)
  (amdgpu_gfx908::s29, registerSpace::s29)
  (amdgpu_gfx908::s30, registerSpace::s30)
  (amdgpu_gfx908::s31, registerSpace::s31)
  (amdgpu_gfx908::s32, registerSpace::s32)
  (amdgpu_gfx908::s33, registerSpace::s33)
  (amdgpu_gfx908::s34, registerSpace::s34)
  (amdgpu_gfx908::s35, registerSpace::s35)
  (amdgpu_gfx908::s36, registerSpace::s36)
  (amdgpu_gfx908::s37, registerSpace::s37)
  (amdgpu_gfx908::s38, registerSpace::s38)
  (amdgpu_gfx908::s39, registerSpace::s39)
  (amdgpu_gfx908::s40, registerSpace::s40)
  (amdgpu_gfx908::s41, registerSpace::s41)
  (amdgpu_gfx908::s42, registerSpace::s42)
  (amdgpu_gfx908::s43, registerSpace::s43)
  (amdgpu_gfx908::s44, registerSpace::s44)
  (amdgpu_gfx908::s45, registerSpace::s45)
  (amdgpu_gfx908::s46, registerSpace::s46)
  (amdgpu_gfx908::s47, registerSpace::s47)
  (amdgpu_gfx908::s48, registerSpace::s48)
  (amdgpu_gfx908::s49, registerSpace::s49)
  (amdgpu_gfx908::s50, registerSpace::s50)
  (amdgpu_gfx908::s51, registerSpace::s51)
  (amdgpu_gfx908::s52, registerSpace::s52)
  (amdgpu_gfx908::s53, registerSpace::s53)
  (amdgpu_gfx908::s54, registerSpace::s54)
  (amdgpu_gfx908::s55, registerSpace::s55)
  (amdgpu_gfx908::s56, registerSpace::s56)
  (amdgpu_gfx908::s57, registerSpace::s57)
  (amdgpu_gfx908::s58, registerSpace::s58)
  (amdgpu_gfx908::s59, registerSpace::s59)
  (amdgpu_gfx908::s60, registerSpace::s60)
  (amdgpu_gfx908::s61, registerSpace::s61)
  (amdgpu_gfx908::s62, registerSpace::s62)
  (amdgpu_gfx908::s63, registerSpace::s63)
  (amdgpu_gfx908::s64, registerSpace::s64)
  (amdgpu_gfx908::s65, registerSpace::s65)
  (amdgpu_gfx908::s66, registerSpace::s66)
  (amdgpu_gfx908::s67, registerSpace::s67)
  (amdgpu_gfx908::s68, registerSpace::s68)
  (amdgpu_gfx908::s69, registerSpace::s69)
  (amdgpu_gfx908::s70, registerSpace::s70)
  (amdgpu_gfx908::s71, registerSpace::s71)
  (amdgpu_gfx908::s72, registerSpace::s72)
  (amdgpu_gfx908::s73, registerSpace::s73)
  (amdgpu_gfx908::s74, registerSpace::s74)
  (amdgpu_gfx908::s75, registerSpace::s75)
  (amdgpu_gfx908::s76, registerSpace::s76)
  (amdgpu_gfx908::s77, registerSpace::s77)
  (amdgpu_gfx908::s78, registerSpace::s78)
  (amdgpu_gfx908::s79, registerSpace::s79)
  (amdgpu_gfx908::s80, registerSpace::s80)
  (amdgpu_gfx908::s81, registerSpace::s81)
  (amdgpu_gfx908::s82, registerSpace::s82)
  (amdgpu_gfx908::s83, registerSpace::s83)
  (amdgpu_gfx908::s84, registerSpace::s84)
  (amdgpu_gfx908::s85, registerSpace::s85)
  (amdgpu_gfx908::s86, registerSpace::s86)
  (amdgpu_gfx908::s87, registerSpace::s87)
  (amdgpu_gfx908::s88, registerSpace::s88)
  (amdgpu_gfx908::s89, registerSpace::s89)
  (amdgpu_gfx908::s90, registerSpace::s90)
  (amdgpu_gfx908::s91, registerSpace::s91)
  (amdgpu_gfx908::s92, registerSpace::s92)
  (amdgpu_gfx908::s93, registerSpace::s93)
  (amdgpu_gfx908::s94, registerSpace::s94)
  (amdgpu_gfx908::s95, registerSpace::s95)
  (amdgpu_gfx908::s96, registerSpace::s96)
  (amdgpu_gfx908::s97, registerSpace::s97)
  (amdgpu_gfx908::s98, registerSpace::s98)
  (amdgpu_gfx908::s99, registerSpace::s99)
  (amdgpu_gfx908::s100, registerSpace::s100)
  (amdgpu_gfx908::s101, registerSpace::s101)
  (amdgpu_gfx908::flat_scratch_lo, registerSpace::flat_scratch_lo)
  (amdgpu_gfx908::flat_scratch_hi, registerSpace::flat_scratch_hi)
  (amdgpu_gfx908::xnack_mask_lo, registerSpace::xnack_mask_lo)
  (amdgpu_gfx908::xnack_mask_hi, registerSpace::xnack_mask_hi)
  (amdgpu_gfx908::vcc_lo, registerSpace::vcc_lo)
  (amdgpu_gfx908::vcc_hi, registerSpace::vcc_hi)
  (amdgpu_gfx908::ttmp0,  registerSpace::ttmp0)
  (amdgpu_gfx908::ttmp1,  registerSpace::ttmp1)
  (amdgpu_gfx908::ttmp2,  registerSpace::ttmp2)
  (amdgpu_gfx908::ttmp3,  registerSpace::ttmp3)
  (amdgpu_gfx908::ttmp4,  registerSpace::ttmp4)
  (amdgpu_gfx908::ttmp5,  registerSpace::ttmp5)
  (amdgpu_gfx908::ttmp6,  registerSpace::ttmp6)
  (amdgpu_gfx908::ttmp7,  registerSpace::ttmp7)
  (amdgpu_gfx908::ttmp8,  registerSpace::ttmp8)
  (amdgpu_gfx908::ttmp9,  registerSpace::ttmp9)
  (amdgpu_gfx908::ttmp10, registerSpace::ttmp10)
  (amdgpu_gfx908::ttmp11, registerSpace::ttmp11)
  (amdgpu_gfx908::ttmp12, registerSpace::ttmp12)
  (amdgpu_gfx908::ttmp13, registerSpace::ttmp13)
  (amdgpu_gfx908::ttmp14, registerSpace::ttmp14)
  (amdgpu_gfx908::ttmp15, registerSpace::ttmp15)
  (amdgpu_gfx908::m0,     registerSpace::m0)
  (amdgpu_gfx908::exec_lo, registerSpace::exec_lo)
  (amdgpu_gfx908::exec_hi, registerSpace::exec_hi)

  (amdgpu_gfx908::v0,  registerSpace::v0)
  (amdgpu_gfx908::v1,  registerSpace::v1)
  (amdgpu_gfx908::v2,  registerSpace::v2)
  (amdgpu_gfx908::v3,  registerSpace::v3)
  (amdgpu_gfx908::v4,  registerSpace::v4)
  (amdgpu_gfx908::v5,  registerSpace::v5)
  (amdgpu_gfx908::v6,  registerSpace::v6)
  (amdgpu_gfx908::v7,  registerSpace::v7)
  (amdgpu_gfx908::v8,  registerSpace::v8)
  (amdgpu_gfx908::v9,  registerSpace::v9)
  (amdgpu_gfx908::v10, registerSpace::v10)
  (amdgpu_gfx908::v11, registerSpace::v11)
  (amdgpu_gfx908::v12, registerSpace::v12)
  (amdgpu_gfx908::v13, registerSpace::v13)
  (amdgpu_gfx908::v14, registerSpace::v14)
  (amdgpu_gfx908::v15, registerSpace::v15)
  (amdgpu_gfx908::v16, registerSpace::v16)
  (amdgpu_gfx908::v17, registerSpace::v17)
  (amdgpu_gfx908::v18, registerSpace::v18)
  (amdgpu_gfx908::v19, registerSpace::v19)
  (amdgpu_gfx908::v20, registerSpace::v20)
  (amdgpu_gfx908::v21, registerSpace::v21)
  (amdgpu_gfx908::v22, registerSpace::v22)
  (amdgpu_gfx908::v23, registerSpace::v23)
  (amdgpu_gfx908::v24, registerSpace::v24)
  (amdgpu_gfx908::v25, registerSpace::v25)
  (amdgpu_gfx908::v26, registerSpace::v26)
  (amdgpu_gfx908::v27, registerSpace::v27)
  (amdgpu_gfx908::v28, registerSpace::v28)
  (amdgpu_gfx908::v29, registerSpace::v29)
  (amdgpu_gfx908::v30, registerSpace::v30)
  (amdgpu_gfx908::v31, registerSpace::v31)
  (amdgpu_gfx908::v32, registerSpace::v32)
  (amdgpu_gfx908::v33, registerSpace::v33)
  (amdgpu_gfx908::v34, registerSpace::v34)
  (amdgpu_gfx908::v35, registerSpace::v35)
  (amdgpu_gfx908::v36, registerSpace::v36)
  (amdgpu_gfx908::v37, registerSpace::v37)
  (amdgpu_gfx908::v38, registerSpace::v38)
  (amdgpu_gfx908::v39, registerSpace::v39)
  (amdgpu_gfx908::v40, registerSpace::v40)
  (amdgpu_gfx908::v41, registerSpace::v41)
  (amdgpu_gfx908::v42, registerSpace::v42)
  (amdgpu_gfx908::v43, registerSpace::v43)
  (amdgpu_gfx908::v44, registerSpace::v44)
  (amdgpu_gfx908::v45, registerSpace::v45)
  (amdgpu_gfx908::v46, registerSpace::v46)
  (amdgpu_gfx908::v47, registerSpace::v47)
  (amdgpu_gfx908::v48, registerSpace::v48)
  (amdgpu_gfx908::v49, registerSpace::v49)
  (amdgpu_gfx908::v50, registerSpace::v50)
  (amdgpu_gfx908::v51, registerSpace::v51)
  (amdgpu_gfx908::v52, registerSpace::v52)
  (amdgpu_gfx908::v53, registerSpace::v53)
  (amdgpu_gfx908::v54, registerSpace::v54)
  (amdgpu_gfx908::v55, registerSpace::v55)
  (amdgpu_gfx908::v56, registerSpace::v56)
  (amdgpu_gfx908::v57, registerSpace::v57)
  (amdgpu_gfx908::v58, registerSpace::v58)
  (amdgpu_gfx908::v59, registerSpace::v59)
  (amdgpu_gfx908::v60, registerSpace::v60)
  (amdgpu_gfx908::v61, registerSpace::v61)
  (amdgpu_gfx908::v62, registerSpace::v62)
  (amdgpu_gfx908::v63, registerSpace::v63)
  (amdgpu_gfx908::v64, registerSpace::v64)
  (amdgpu_gfx908::v65, registerSpace::v65)
  (amdgpu_gfx908::v66, registerSpace::v66)
  (amdgpu_gfx908::v67, registerSpace::v67)
  (amdgpu_gfx908::v68, registerSpace::v68)
  (amdgpu_gfx908::v69, registerSpace::v69)
  (amdgpu_gfx908::v70, registerSpace::v70)
  (amdgpu_gfx908::v71, registerSpace::v71)
  (amdgpu_gfx908::v72, registerSpace::v72)
  (amdgpu_gfx908::v73, registerSpace::v73)
  (amdgpu_gfx908::v74, registerSpace::v74)
  (amdgpu_gfx908::v75, registerSpace::v75)
  (amdgpu_gfx908::v76, registerSpace::v76)
  (amdgpu_gfx908::v77, registerSpace::v77)
  (amdgpu_gfx908::v78, registerSpace::v78)
  (amdgpu_gfx908::v79, registerSpace::v79)
  (amdgpu_gfx908::v80, registerSpace::v80)
  (amdgpu_gfx908::v81, registerSpace::v81)
  (amdgpu_gfx908::v82, registerSpace::v82)
  (amdgpu_gfx908::v83, registerSpace::v83)
  (amdgpu_gfx908::v84, registerSpace::v84)
  (amdgpu_gfx908::v85, registerSpace::v85)
  (amdgpu_gfx908::v86, registerSpace::v86)
  (amdgpu_gfx908::v87, registerSpace::v87)
  (amdgpu_gfx908::v88, registerSpace::v88)
  (amdgpu_gfx908::v89, registerSpace::v89)
  (amdgpu_gfx908::v90, registerSpace::v90)
  (amdgpu_gfx908::v91, registerSpace::v91)
  (amdgpu_gfx908::v92, registerSpace::v92)
  (amdgpu_gfx908::v93, registerSpace::v93)
  (amdgpu_gfx908::v94, registerSpace::v94)
  (amdgpu_gfx908::v95, registerSpace::v95)
  (amdgpu_gfx908::v96, registerSpace::v96)
  (amdgpu_gfx908::v97, registerSpace::v97)
  (amdgpu_gfx908::v98, registerSpace::v98)
  (amdgpu_gfx908::v99, registerSpace::v99)
  (amdgpu_gfx908::v100, registerSpace::v100)
  (amdgpu_gfx908::v101, registerSpace::v101)
  (amdgpu_gfx908::v102, registerSpace::v102)
  (amdgpu_gfx908::v103, registerSpace::v103)
  (amdgpu_gfx908::v104, registerSpace::v104)
  (amdgpu_gfx908::v105, registerSpace::v105)
  (amdgpu_gfx908::v106, registerSpace::v106)
  (amdgpu_gfx908::v107, registerSpace::v107)
  (amdgpu_gfx908::v108, registerSpace::v108)
  (amdgpu_gfx908::v109, registerSpace::v109)
  (amdgpu_gfx908::v110, registerSpace::v110)
  (amdgpu_gfx908::v111, registerSpace::v111)
  (amdgpu_gfx908::v112, registerSpace::v112)
  (amdgpu_gfx908::v113, registerSpace::v113)
  (amdgpu_gfx908::v114, registerSpace::v114)
  (amdgpu_gfx908::v115, registerSpace::v115)
  (amdgpu_gfx908::v116, registerSpace::v116)
  (amdgpu_gfx908::v117, registerSpace::v117)
  (amdgpu_gfx908::v118, registerSpace::v118)
  (amdgpu_gfx908::v119, registerSpace::v119)
  (amdgpu_gfx908::v120, registerSpace::v120)
  (amdgpu_gfx908::v121, registerSpace::v121)
  (amdgpu_gfx908::v122, registerSpace::v122)
  (amdgpu_gfx908::v123, registerSpace::v123)
  (amdgpu_gfx908::v124, registerSpace::v124)
  (amdgpu_gfx908::v125, registerSpace::v125)
  (amdgpu_gfx908::v126, registerSpace::v126)
  (amdgpu_gfx908::v127, registerSpace::v127)
  (amdgpu_gfx908::v128, registerSpace::v128)
  (amdgpu_gfx908::v129, registerSpace::v129)
  (amdgpu_gfx908::v130, registerSpace::v130)
  (amdgpu_gfx908::v131, registerSpace::v131)
  (amdgpu_gfx908::v132, registerSpace::v132)
  (amdgpu_gfx908::v133, registerSpace::v133)
  (amdgpu_gfx908::v134, registerSpace::v134)
  (amdgpu_gfx908::v135, registerSpace::v135)
  (amdgpu_gfx908::v136, registerSpace::v136)
  (amdgpu_gfx908::v137, registerSpace::v137)
  (amdgpu_gfx908::v138, registerSpace::v138)
  (amdgpu_gfx908::v139, registerSpace::v139)
  (amdgpu_gfx908::v140, registerSpace::v140)
  (amdgpu_gfx908::v141, registerSpace::v141)
  (amdgpu_gfx908::v142, registerSpace::v142)
  (amdgpu_gfx908::v143, registerSpace::v143)
  (amdgpu_gfx908::v144, registerSpace::v144)
  (amdgpu_gfx908::v145, registerSpace::v145)
  (amdgpu_gfx908::v146, registerSpace::v146)
  (amdgpu_gfx908::v147, registerSpace::v147)
  (amdgpu_gfx908::v148, registerSpace::v148)
  (amdgpu_gfx908::v149, registerSpace::v149)
  (amdgpu_gfx908::v150, registerSpace::v150)
  (amdgpu_gfx908::v151, registerSpace::v151)
  (amdgpu_gfx908::v152, registerSpace::v152)
  (amdgpu_gfx908::v153, registerSpace::v153)
  (amdgpu_gfx908::v154, registerSpace::v154)
  (amdgpu_gfx908::v155, registerSpace::v155)
  (amdgpu_gfx908::v156, registerSpace::v156)
  (amdgpu_gfx908::v157, registerSpace::v157)
  (amdgpu_gfx908::v158, registerSpace::v158)
  (amdgpu_gfx908::v159, registerSpace::v159)
  (amdgpu_gfx908::v160, registerSpace::v160)
  (amdgpu_gfx908::v161, registerSpace::v161)
  (amdgpu_gfx908::v162, registerSpace::v162)
  (amdgpu_gfx908::v163, registerSpace::v163)
  (amdgpu_gfx908::v164, registerSpace::v164)
  (amdgpu_gfx908::v165, registerSpace::v165)
  (amdgpu_gfx908::v166, registerSpace::v166)
  (amdgpu_gfx908::v167, registerSpace::v167)
  (amdgpu_gfx908::v168, registerSpace::v168)
  (amdgpu_gfx908::v169, registerSpace::v169)
  (amdgpu_gfx908::v170, registerSpace::v170)
  (amdgpu_gfx908::v171, registerSpace::v171)
  (amdgpu_gfx908::v172, registerSpace::v172)
  (amdgpu_gfx908::v173, registerSpace::v173)
  (amdgpu_gfx908::v174, registerSpace::v174)
  (amdgpu_gfx908::v175, registerSpace::v175)
  (amdgpu_gfx908::v176, registerSpace::v176)
  (amdgpu_gfx908::v177, registerSpace::v177)
  (amdgpu_gfx908::v178, registerSpace::v178)
  (amdgpu_gfx908::v179, registerSpace::v179)
  (amdgpu_gfx908::v180, registerSpace::v180)
  (amdgpu_gfx908::v181, registerSpace::v181)
  (amdgpu_gfx908::v182, registerSpace::v182)
  (amdgpu_gfx908::v183, registerSpace::v183)
  (amdgpu_gfx908::v184, registerSpace::v184)
  (amdgpu_gfx908::v185, registerSpace::v185)
  (amdgpu_gfx908::v186, registerSpace::v186)
  (amdgpu_gfx908::v187, registerSpace::v187)
  (amdgpu_gfx908::v188, registerSpace::v188)
  (amdgpu_gfx908::v189, registerSpace::v189)
  (amdgpu_gfx908::v190, registerSpace::v190)
  (amdgpu_gfx908::v191, registerSpace::v191)
  (amdgpu_gfx908::v192, registerSpace::v192)
  (amdgpu_gfx908::v193, registerSpace::v193)
  (amdgpu_gfx908::v194, registerSpace::v194)
  (amdgpu_gfx908::v195, registerSpace::v195)
  (amdgpu_gfx908::v196, registerSpace::v196)
  (amdgpu_gfx908::v197, registerSpace::v197)
  (amdgpu_gfx908::v198, registerSpace::v198)
  (amdgpu_gfx908::v199, registerSpace::v199)
  (amdgpu_gfx908::v200, registerSpace::v200)
  (amdgpu_gfx908::v201, registerSpace::v201)
  (amdgpu_gfx908::v202, registerSpace::v202)
  (amdgpu_gfx908::v203, registerSpace::v203)
  (amdgpu_gfx908::v204, registerSpace::v204)
  (amdgpu_gfx908::v205, registerSpace::v205)
  (amdgpu_gfx908::v206, registerSpace::v206)
  (amdgpu_gfx908::v207, registerSpace::v207)
  (amdgpu_gfx908::v208, registerSpace::v208)
  (amdgpu_gfx908::v209, registerSpace::v209)
  (amdgpu_gfx908::v210, registerSpace::v210)
  (amdgpu_gfx908::v211, registerSpace::v211)
  (amdgpu_gfx908::v212, registerSpace::v212)
  (amdgpu_gfx908::v213, registerSpace::v213)
  (amdgpu_gfx908::v214, registerSpace::v214)
  (amdgpu_gfx908::v215, registerSpace::v215)
  (amdgpu_gfx908::v216, registerSpace::v216)
  (amdgpu_gfx908::v217, registerSpace::v217)
  (amdgpu_gfx908::v218, registerSpace::v218)
  (amdgpu_gfx908::v219, registerSpace::v219)
  (amdgpu_gfx908::v220, registerSpace::v220)
  (amdgpu_gfx908::v221, registerSpace::v221)
  (amdgpu_gfx908::v222, registerSpace::v222)
  (amdgpu_gfx908::v223, registerSpace::v223)
  (amdgpu_gfx908::v224, registerSpace::v224)
  (amdgpu_gfx908::v225, registerSpace::v225)
  (amdgpu_gfx908::v226, registerSpace::v226)
  (amdgpu_gfx908::v227, registerSpace::v227)
  (amdgpu_gfx908::v228, registerSpace::v228)
  (amdgpu_gfx908::v229, registerSpace::v229)
  (amdgpu_gfx908::v230, registerSpace::v230)
  (amdgpu_gfx908::v231, registerSpace::v231)
  (amdgpu_gfx908::v232, registerSpace::v232)
  (amdgpu_gfx908::v233, registerSpace::v233)
  (amdgpu_gfx908::v234, registerSpace::v234)
  (amdgpu_gfx908::v235, registerSpace::v235)
  (amdgpu_gfx908::v236, registerSpace::v236)
  (amdgpu_gfx908::v237, registerSpace::v237)
  (amdgpu_gfx908::v238, registerSpace::v238)
  (amdgpu_gfx908::v239, registerSpace::v239)
  (amdgpu_gfx908::v240, registerSpace::v240)
  (amdgpu_gfx908::v241, registerSpace::v241)
  (amdgpu_gfx908::v242, registerSpace::v242)
  (amdgpu_gfx908::v243, registerSpace::v243)
  (amdgpu_gfx908::v244, registerSpace::v244)
  (amdgpu_gfx908::v245, registerSpace::v245)
  (amdgpu_gfx908::v246, registerSpace::v246)
  (amdgpu_gfx908::v247, registerSpace::v247)
  (amdgpu_gfx908::v248, registerSpace::v248)
  (amdgpu_gfx908::v249, registerSpace::v249)
  (amdgpu_gfx908::v250, registerSpace::v250)
  (amdgpu_gfx908::v251, registerSpace::v251)
  (amdgpu_gfx908::v252, registerSpace::v252)
  (amdgpu_gfx908::v253, registerSpace::v253)
  (amdgpu_gfx908::v254, registerSpace::v254)
  (amdgpu_gfx908::v255, registerSpace::v255)
  ;

Register convertRegID(MachRegister reg, bool &wasUpcast) {
    wasUpcast = false;

    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_amdgpu_gfx908);
//    RegisterAST::Ptr debug(new RegisterAST(baseReg));
//    fprintf(stderr, "DEBUG: converting %s", toBeConverted->format().c_str());
//    fprintf(stderr, " to %s\n", debug->format().c_str());
    map<MachRegister, Register>::const_iterator found =
      reverseRegisterMap.find(baseReg);
    if(found == reverseRegisterMap.end()) {
      // Yeah, this happens when we analyze trash code. Oops...
      return registerSpace::ignored;
    }

    return found->second;
}


Register convertRegID(RegisterAST::Ptr toBeConverted, bool& wasUpcast)
{
    return convertRegID(toBeConverted.get(), wasUpcast);
}

Register convertRegID(RegisterAST* toBeConverted, bool& wasUpcast)
{
     if(!toBeConverted) {
        //assert(0);
      return registerSpace::ignored;
    }
    return convertRegID(toBeConverted->getID(), wasUpcast);
}


MachRegister convertRegID(Register r, Dyninst::Architecture arch) {
    if( arch == Arch_amdgpu_gfx908) {
        switch(r) {
            case registerSpace::s0: return amdgpu_gfx908::s0;
            case registerSpace::s1: return amdgpu_gfx908::s1;
            case registerSpace::s2: return amdgpu_gfx908::s2;
            case registerSpace::s3: return amdgpu_gfx908::s3;
            case registerSpace::s4: return amdgpu_gfx908::s4;
            case registerSpace::s5: return amdgpu_gfx908::s5;
            case registerSpace::s6: return amdgpu_gfx908::s6;
            case registerSpace::s7: return amdgpu_gfx908::s7;
            case registerSpace::s8: return amdgpu_gfx908::s8;
            case registerSpace::s9: return amdgpu_gfx908::s9;
            case registerSpace::s10: return amdgpu_gfx908::s10;
            case registerSpace::s11: return amdgpu_gfx908::s11;
            case registerSpace::s12: return amdgpu_gfx908::s12;
            case registerSpace::s13: return amdgpu_gfx908::s13;
            case registerSpace::s14: return amdgpu_gfx908::s14;
            case registerSpace::s15: return amdgpu_gfx908::s15;
            case registerSpace::s16: return amdgpu_gfx908::s16;
            case registerSpace::s17: return amdgpu_gfx908::s17;
            case registerSpace::s18: return amdgpu_gfx908::s18;
            case registerSpace::s19: return amdgpu_gfx908::s19;
            case registerSpace::s20: return amdgpu_gfx908::s20;
            case registerSpace::s21: return amdgpu_gfx908::s21;
            case registerSpace::s22: return amdgpu_gfx908::s22;
            case registerSpace::s23: return amdgpu_gfx908::s23;
            case registerSpace::s24: return amdgpu_gfx908::s24;
            case registerSpace::s25: return amdgpu_gfx908::s25;
            case registerSpace::s26: return amdgpu_gfx908::s26;
            case registerSpace::s27: return amdgpu_gfx908::s27;
            case registerSpace::s28: return amdgpu_gfx908::s28;
            case registerSpace::s29: return amdgpu_gfx908::s29;
            case registerSpace::s30: return amdgpu_gfx908::s30;
            case registerSpace::s31: return amdgpu_gfx908::s31;
            case registerSpace::s32: return amdgpu_gfx908::s32;
            case registerSpace::s33: return amdgpu_gfx908::s33;
            case registerSpace::s34: return amdgpu_gfx908::s34;
            case registerSpace::s35: return amdgpu_gfx908::s35;
            case registerSpace::s36: return amdgpu_gfx908::s36;
            case registerSpace::s37: return amdgpu_gfx908::s37;
            case registerSpace::s38: return amdgpu_gfx908::s38;
            case registerSpace::s39: return amdgpu_gfx908::s39;
            case registerSpace::s40: return amdgpu_gfx908::s40;
            case registerSpace::s41: return amdgpu_gfx908::s41;
            case registerSpace::s42: return amdgpu_gfx908::s42;
            case registerSpace::s43: return amdgpu_gfx908::s43;
            case registerSpace::s44: return amdgpu_gfx908::s44;
            case registerSpace::s45: return amdgpu_gfx908::s45;
            case registerSpace::s46: return amdgpu_gfx908::s46;
            case registerSpace::s47: return amdgpu_gfx908::s47;
            case registerSpace::s48: return amdgpu_gfx908::s48;
            case registerSpace::s49: return amdgpu_gfx908::s49;
            case registerSpace::s50: return amdgpu_gfx908::s50;
            case registerSpace::s51: return amdgpu_gfx908::s51;
            case registerSpace::s52: return amdgpu_gfx908::s52;
            case registerSpace::s53: return amdgpu_gfx908::s53;
            case registerSpace::s54: return amdgpu_gfx908::s54;
            case registerSpace::s55: return amdgpu_gfx908::s55;
            case registerSpace::s56: return amdgpu_gfx908::s56;
            case registerSpace::s57: return amdgpu_gfx908::s57;
            case registerSpace::s58: return amdgpu_gfx908::s58;
            case registerSpace::s59: return amdgpu_gfx908::s59;
            case registerSpace::s60: return amdgpu_gfx908::s60;
            case registerSpace::s61: return amdgpu_gfx908::s61;
            case registerSpace::s62: return amdgpu_gfx908::s62;
            case registerSpace::s63: return amdgpu_gfx908::s63;
            case registerSpace::s64: return amdgpu_gfx908::s64;
            case registerSpace::s65: return amdgpu_gfx908::s65;
            case registerSpace::s66: return amdgpu_gfx908::s66;
            case registerSpace::s67: return amdgpu_gfx908::s67;
            case registerSpace::s68: return amdgpu_gfx908::s68;
            case registerSpace::s69: return amdgpu_gfx908::s69;
            case registerSpace::s70: return amdgpu_gfx908::s70;
            case registerSpace::s71: return amdgpu_gfx908::s71;
            case registerSpace::s72: return amdgpu_gfx908::s72;
            case registerSpace::s73: return amdgpu_gfx908::s73;
            case registerSpace::s74: return amdgpu_gfx908::s74;
            case registerSpace::s75: return amdgpu_gfx908::s75;
            case registerSpace::s76: return amdgpu_gfx908::s76;
            case registerSpace::s77: return amdgpu_gfx908::s77;
            case registerSpace::s78: return amdgpu_gfx908::s78;
            case registerSpace::s79: return amdgpu_gfx908::s79;
            case registerSpace::s80: return amdgpu_gfx908::s80;
            case registerSpace::s81: return amdgpu_gfx908::s81;
            case registerSpace::s82: return amdgpu_gfx908::s82;
            case registerSpace::s83: return amdgpu_gfx908::s83;
            case registerSpace::s84: return amdgpu_gfx908::s84;
            case registerSpace::s85: return amdgpu_gfx908::s85;
            case registerSpace::s86: return amdgpu_gfx908::s86;
            case registerSpace::s87: return amdgpu_gfx908::s87;
            case registerSpace::s88: return amdgpu_gfx908::s88;
            case registerSpace::s89: return amdgpu_gfx908::s89;
            case registerSpace::s90: return amdgpu_gfx908::s90;
            case registerSpace::s91: return amdgpu_gfx908::s91;
            case registerSpace::s92: return amdgpu_gfx908::s92;
            case registerSpace::s93: return amdgpu_gfx908::s93;
            case registerSpace::s94: return amdgpu_gfx908::s94;
            case registerSpace::s95: return amdgpu_gfx908::s95;
            case registerSpace::s96: return amdgpu_gfx908::s96;
            case registerSpace::s97: return amdgpu_gfx908::s97;
            case registerSpace::s98: return amdgpu_gfx908::s98;
            case registerSpace::s99: return amdgpu_gfx908::s99;
            case registerSpace::s100: return amdgpu_gfx908::s100;
            case registerSpace::s101: return amdgpu_gfx908::s101;
            case registerSpace::flat_scratch_lo: return amdgpu_gfx908::flat_scratch_lo;
            case registerSpace::flat_scratch_hi: return amdgpu_gfx908::flat_scratch_hi;
            case registerSpace::xnack_mask_lo: return amdgpu_gfx908::xnack_mask_lo;
            case registerSpace::xnack_mask_hi: return amdgpu_gfx908::xnack_mask_hi;
            case registerSpace::vcc_lo: return amdgpu_gfx908::vcc_lo;
            case registerSpace::vcc_hi: return amdgpu_gfx908::vcc_hi;
            case registerSpace::ttmp0: return amdgpu_gfx908::ttmp0;
            case registerSpace::ttmp1: return amdgpu_gfx908::ttmp1;
            case registerSpace::ttmp2: return amdgpu_gfx908::ttmp2;
            case registerSpace::ttmp3: return amdgpu_gfx908::ttmp3;
            case registerSpace::ttmp4: return amdgpu_gfx908::ttmp4;
            case registerSpace::ttmp5: return amdgpu_gfx908::ttmp5;
            case registerSpace::ttmp6: return amdgpu_gfx908::ttmp6;
            case registerSpace::ttmp7: return amdgpu_gfx908::ttmp7;
            case registerSpace::ttmp8: return amdgpu_gfx908::ttmp8;
            case registerSpace::ttmp9: return amdgpu_gfx908::ttmp9;
            case registerSpace::ttmp10: return amdgpu_gfx908::ttmp10;
            case registerSpace::ttmp11: return amdgpu_gfx908::ttmp11;
            case registerSpace::ttmp12: return amdgpu_gfx908::ttmp12;
            case registerSpace::ttmp13: return amdgpu_gfx908::ttmp13;
            case registerSpace::ttmp14: return amdgpu_gfx908::ttmp14;
            case registerSpace::ttmp15: return amdgpu_gfx908::ttmp15;
            case registerSpace::m0: return amdgpu_gfx908::m0;
            case registerSpace::exec_lo: return amdgpu_gfx908::exec_lo;
            case registerSpace::exec_hi: return amdgpu_gfx908::exec_hi;

            case registerSpace::v0: return amdgpu_gfx908::v0;
            case registerSpace::v1: return amdgpu_gfx908::v1;
            case registerSpace::v2: return amdgpu_gfx908::v2;
            case registerSpace::v3: return amdgpu_gfx908::v3;
            case registerSpace::v4: return amdgpu_gfx908::v4;
            case registerSpace::v5: return amdgpu_gfx908::v5;
            case registerSpace::v6: return amdgpu_gfx908::v6;
            case registerSpace::v7: return amdgpu_gfx908::v7;
            case registerSpace::v8: return amdgpu_gfx908::v8;
            case registerSpace::v9: return amdgpu_gfx908::v9;
            case registerSpace::v10: return amdgpu_gfx908::v10;
            case registerSpace::v11: return amdgpu_gfx908::v11;
            case registerSpace::v12: return amdgpu_gfx908::v12;
            case registerSpace::v13: return amdgpu_gfx908::v13;
            case registerSpace::v14: return amdgpu_gfx908::v14;
            case registerSpace::v15: return amdgpu_gfx908::v15;
            case registerSpace::v16: return amdgpu_gfx908::v16;
            case registerSpace::v17: return amdgpu_gfx908::v17;
            case registerSpace::v18: return amdgpu_gfx908::v18;
            case registerSpace::v19: return amdgpu_gfx908::v19;
            case registerSpace::v20: return amdgpu_gfx908::v20;
            case registerSpace::v21: return amdgpu_gfx908::v21;
            case registerSpace::v22: return amdgpu_gfx908::v22;
            case registerSpace::v23: return amdgpu_gfx908::v23;
            case registerSpace::v24: return amdgpu_gfx908::v24;
            case registerSpace::v25: return amdgpu_gfx908::v25;
            case registerSpace::v26: return amdgpu_gfx908::v26;
            case registerSpace::v27: return amdgpu_gfx908::v27;
            case registerSpace::v28: return amdgpu_gfx908::v28;
            case registerSpace::v29: return amdgpu_gfx908::v29;
            case registerSpace::v30: return amdgpu_gfx908::v30;
            case registerSpace::v31: return amdgpu_gfx908::v31;
            case registerSpace::v32: return amdgpu_gfx908::v32;
            case registerSpace::v33: return amdgpu_gfx908::v33;
            case registerSpace::v34: return amdgpu_gfx908::v34;
            case registerSpace::v35: return amdgpu_gfx908::v35;
            case registerSpace::v36: return amdgpu_gfx908::v36;
            case registerSpace::v37: return amdgpu_gfx908::v37;
            case registerSpace::v38: return amdgpu_gfx908::v38;
            case registerSpace::v39: return amdgpu_gfx908::v39;
            case registerSpace::v40: return amdgpu_gfx908::v40;
            case registerSpace::v41: return amdgpu_gfx908::v41;
            case registerSpace::v42: return amdgpu_gfx908::v42;
            case registerSpace::v43: return amdgpu_gfx908::v43;
            case registerSpace::v44: return amdgpu_gfx908::v44;
            case registerSpace::v45: return amdgpu_gfx908::v45;
            case registerSpace::v46: return amdgpu_gfx908::v46;
            case registerSpace::v47: return amdgpu_gfx908::v47;
            case registerSpace::v48: return amdgpu_gfx908::v48;
            case registerSpace::v49: return amdgpu_gfx908::v49;
            case registerSpace::v50: return amdgpu_gfx908::v50;
            case registerSpace::v51: return amdgpu_gfx908::v51;
            case registerSpace::v52: return amdgpu_gfx908::v52;
            case registerSpace::v53: return amdgpu_gfx908::v53;
            case registerSpace::v54: return amdgpu_gfx908::v54;
            case registerSpace::v55: return amdgpu_gfx908::v55;
            case registerSpace::v56: return amdgpu_gfx908::v56;
            case registerSpace::v57: return amdgpu_gfx908::v57;
            case registerSpace::v58: return amdgpu_gfx908::v58;
            case registerSpace::v59: return amdgpu_gfx908::v59;
            case registerSpace::v60: return amdgpu_gfx908::v60;
            case registerSpace::v61: return amdgpu_gfx908::v61;
            case registerSpace::v62: return amdgpu_gfx908::v62;
            case registerSpace::v63: return amdgpu_gfx908::v63;
            case registerSpace::v64: return amdgpu_gfx908::v64;
            case registerSpace::v65: return amdgpu_gfx908::v65;
            case registerSpace::v66: return amdgpu_gfx908::v66;
            case registerSpace::v67: return amdgpu_gfx908::v67;
            case registerSpace::v68: return amdgpu_gfx908::v68;
            case registerSpace::v69: return amdgpu_gfx908::v69;
            case registerSpace::v70: return amdgpu_gfx908::v70;
            case registerSpace::v71: return amdgpu_gfx908::v71;
            case registerSpace::v72: return amdgpu_gfx908::v72;
            case registerSpace::v73: return amdgpu_gfx908::v73;
            case registerSpace::v74: return amdgpu_gfx908::v74;
            case registerSpace::v75: return amdgpu_gfx908::v75;
            case registerSpace::v76: return amdgpu_gfx908::v76;
            case registerSpace::v77: return amdgpu_gfx908::v77;
            case registerSpace::v78: return amdgpu_gfx908::v78;
            case registerSpace::v79: return amdgpu_gfx908::v79;
            case registerSpace::v80: return amdgpu_gfx908::v80;
            case registerSpace::v81: return amdgpu_gfx908::v81;
            case registerSpace::v82: return amdgpu_gfx908::v82;
            case registerSpace::v83: return amdgpu_gfx908::v83;
            case registerSpace::v84: return amdgpu_gfx908::v84;
            case registerSpace::v85: return amdgpu_gfx908::v85;
            case registerSpace::v86: return amdgpu_gfx908::v86;
            case registerSpace::v87: return amdgpu_gfx908::v87;
            case registerSpace::v88: return amdgpu_gfx908::v88;
            case registerSpace::v89: return amdgpu_gfx908::v89;
            case registerSpace::v90: return amdgpu_gfx908::v90;
            case registerSpace::v91: return amdgpu_gfx908::v91;
            case registerSpace::v92: return amdgpu_gfx908::v92;
            case registerSpace::v93: return amdgpu_gfx908::v93;
            case registerSpace::v94: return amdgpu_gfx908::v94;
            case registerSpace::v95: return amdgpu_gfx908::v95;
            case registerSpace::v96: return amdgpu_gfx908::v96;
            case registerSpace::v97: return amdgpu_gfx908::v97;
            case registerSpace::v98: return amdgpu_gfx908::v98;
            case registerSpace::v99: return amdgpu_gfx908::v99;
            case registerSpace::v100: return amdgpu_gfx908::v100;
            case registerSpace::v101: return amdgpu_gfx908::v101;
            case registerSpace::v102: return amdgpu_gfx908::v102;
            case registerSpace::v103: return amdgpu_gfx908::v103;
            case registerSpace::v104: return amdgpu_gfx908::v104;
            case registerSpace::v105: return amdgpu_gfx908::v105;
            case registerSpace::v106: return amdgpu_gfx908::v106;
            case registerSpace::v107: return amdgpu_gfx908::v107;
            case registerSpace::v108: return amdgpu_gfx908::v108;
            case registerSpace::v109: return amdgpu_gfx908::v109;
            case registerSpace::v110: return amdgpu_gfx908::v110;
            case registerSpace::v111: return amdgpu_gfx908::v111;
            case registerSpace::v112: return amdgpu_gfx908::v112;
            case registerSpace::v113: return amdgpu_gfx908::v113;
            case registerSpace::v114: return amdgpu_gfx908::v114;
            case registerSpace::v115: return amdgpu_gfx908::v115;
            case registerSpace::v116: return amdgpu_gfx908::v116;
            case registerSpace::v117: return amdgpu_gfx908::v117;
            case registerSpace::v118: return amdgpu_gfx908::v118;
            case registerSpace::v119: return amdgpu_gfx908::v119;
            case registerSpace::v120: return amdgpu_gfx908::v120;
            case registerSpace::v121: return amdgpu_gfx908::v121;
            case registerSpace::v122: return amdgpu_gfx908::v122;
            case registerSpace::v123: return amdgpu_gfx908::v123;
            case registerSpace::v124: return amdgpu_gfx908::v124;
            case registerSpace::v125: return amdgpu_gfx908::v125;
            case registerSpace::v126: return amdgpu_gfx908::v126;
            case registerSpace::v127: return amdgpu_gfx908::v127;
            case registerSpace::v128: return amdgpu_gfx908::v128;
            case registerSpace::v129: return amdgpu_gfx908::v129;
            case registerSpace::v130: return amdgpu_gfx908::v130;
            case registerSpace::v131: return amdgpu_gfx908::v131;
            case registerSpace::v132: return amdgpu_gfx908::v132;
            case registerSpace::v133: return amdgpu_gfx908::v133;
            case registerSpace::v134: return amdgpu_gfx908::v134;
            case registerSpace::v135: return amdgpu_gfx908::v135;
            case registerSpace::v136: return amdgpu_gfx908::v136;
            case registerSpace::v137: return amdgpu_gfx908::v137;
            case registerSpace::v138: return amdgpu_gfx908::v138;
            case registerSpace::v139: return amdgpu_gfx908::v139;
            case registerSpace::v140: return amdgpu_gfx908::v140;
            case registerSpace::v141: return amdgpu_gfx908::v141;
            case registerSpace::v142: return amdgpu_gfx908::v142;
            case registerSpace::v143: return amdgpu_gfx908::v143;
            case registerSpace::v144: return amdgpu_gfx908::v144;
            case registerSpace::v145: return amdgpu_gfx908::v145;
            case registerSpace::v146: return amdgpu_gfx908::v146;
            case registerSpace::v147: return amdgpu_gfx908::v147;
            case registerSpace::v148: return amdgpu_gfx908::v148;
            case registerSpace::v149: return amdgpu_gfx908::v149;
            case registerSpace::v150: return amdgpu_gfx908::v150;
            case registerSpace::v151: return amdgpu_gfx908::v151;
            case registerSpace::v152: return amdgpu_gfx908::v152;
            case registerSpace::v153: return amdgpu_gfx908::v153;
            case registerSpace::v154: return amdgpu_gfx908::v154;
            case registerSpace::v155: return amdgpu_gfx908::v155;
            case registerSpace::v156: return amdgpu_gfx908::v156;
            case registerSpace::v157: return amdgpu_gfx908::v157;
            case registerSpace::v158: return amdgpu_gfx908::v158;
            case registerSpace::v159: return amdgpu_gfx908::v159;
            case registerSpace::v160: return amdgpu_gfx908::v160;
            case registerSpace::v161: return amdgpu_gfx908::v161;
            case registerSpace::v162: return amdgpu_gfx908::v162;
            case registerSpace::v163: return amdgpu_gfx908::v163;
            case registerSpace::v164: return amdgpu_gfx908::v164;
            case registerSpace::v165: return amdgpu_gfx908::v165;
            case registerSpace::v166: return amdgpu_gfx908::v166;
            case registerSpace::v167: return amdgpu_gfx908::v167;
            case registerSpace::v168: return amdgpu_gfx908::v168;
            case registerSpace::v169: return amdgpu_gfx908::v169;
            case registerSpace::v170: return amdgpu_gfx908::v170;
            case registerSpace::v171: return amdgpu_gfx908::v171;
            case registerSpace::v172: return amdgpu_gfx908::v172;
            case registerSpace::v173: return amdgpu_gfx908::v173;
            case registerSpace::v174: return amdgpu_gfx908::v174;
            case registerSpace::v175: return amdgpu_gfx908::v175;
            case registerSpace::v176: return amdgpu_gfx908::v176;
            case registerSpace::v177: return amdgpu_gfx908::v177;
            case registerSpace::v178: return amdgpu_gfx908::v178;
            case registerSpace::v179: return amdgpu_gfx908::v179;
            case registerSpace::v180: return amdgpu_gfx908::v180;
            case registerSpace::v181: return amdgpu_gfx908::v181;
            case registerSpace::v182: return amdgpu_gfx908::v182;
            case registerSpace::v183: return amdgpu_gfx908::v183;
            case registerSpace::v184: return amdgpu_gfx908::v184;
            case registerSpace::v185: return amdgpu_gfx908::v185;
            case registerSpace::v186: return amdgpu_gfx908::v186;
            case registerSpace::v187: return amdgpu_gfx908::v187;
            case registerSpace::v188: return amdgpu_gfx908::v188;
            case registerSpace::v189: return amdgpu_gfx908::v189;
            case registerSpace::v190: return amdgpu_gfx908::v190;
            case registerSpace::v191: return amdgpu_gfx908::v191;
            case registerSpace::v192: return amdgpu_gfx908::v192;
            case registerSpace::v193: return amdgpu_gfx908::v193;
            case registerSpace::v194: return amdgpu_gfx908::v194;
            case registerSpace::v195: return amdgpu_gfx908::v195;
            case registerSpace::v196: return amdgpu_gfx908::v196;
            case registerSpace::v197: return amdgpu_gfx908::v197;
            case registerSpace::v198: return amdgpu_gfx908::v198;
            case registerSpace::v199: return amdgpu_gfx908::v199;
            case registerSpace::v200: return amdgpu_gfx908::v200;
            case registerSpace::v201: return amdgpu_gfx908::v201;
            case registerSpace::v202: return amdgpu_gfx908::v202;
            case registerSpace::v203: return amdgpu_gfx908::v203;
            case registerSpace::v204: return amdgpu_gfx908::v204;
            case registerSpace::v205: return amdgpu_gfx908::v205;
            case registerSpace::v206: return amdgpu_gfx908::v206;
            case registerSpace::v207: return amdgpu_gfx908::v207;
            case registerSpace::v208: return amdgpu_gfx908::v208;
            case registerSpace::v209: return amdgpu_gfx908::v209;
            case registerSpace::v210: return amdgpu_gfx908::v210;
            case registerSpace::v211: return amdgpu_gfx908::v211;
            case registerSpace::v212: return amdgpu_gfx908::v212;
            case registerSpace::v213: return amdgpu_gfx908::v213;
            case registerSpace::v214: return amdgpu_gfx908::v214;
            case registerSpace::v215: return amdgpu_gfx908::v215;
            case registerSpace::v216: return amdgpu_gfx908::v216;
            case registerSpace::v217: return amdgpu_gfx908::v217;
            case registerSpace::v218: return amdgpu_gfx908::v218;
            case registerSpace::v219: return amdgpu_gfx908::v219;
            case registerSpace::v220: return amdgpu_gfx908::v220;
            case registerSpace::v221: return amdgpu_gfx908::v221;
            case registerSpace::v222: return amdgpu_gfx908::v222;
            case registerSpace::v223: return amdgpu_gfx908::v223;
            case registerSpace::v224: return amdgpu_gfx908::v224;
            case registerSpace::v225: return amdgpu_gfx908::v225;
            case registerSpace::v226: return amdgpu_gfx908::v226;
            case registerSpace::v227: return amdgpu_gfx908::v227;
            case registerSpace::v228: return amdgpu_gfx908::v228;
            case registerSpace::v229: return amdgpu_gfx908::v229;
            case registerSpace::v230: return amdgpu_gfx908::v230;
            case registerSpace::v231: return amdgpu_gfx908::v231;
            case registerSpace::v232: return amdgpu_gfx908::v232;
            case registerSpace::v233: return amdgpu_gfx908::v233;
            case registerSpace::v234: return amdgpu_gfx908::v234;
            case registerSpace::v235: return amdgpu_gfx908::v235;
            case registerSpace::v236: return amdgpu_gfx908::v236;
            case registerSpace::v237: return amdgpu_gfx908::v237;
            case registerSpace::v238: return amdgpu_gfx908::v238;
            case registerSpace::v239: return amdgpu_gfx908::v239;
            case registerSpace::v240: return amdgpu_gfx908::v240;
            case registerSpace::v241: return amdgpu_gfx908::v241;
            case registerSpace::v242: return amdgpu_gfx908::v242;
            case registerSpace::v243: return amdgpu_gfx908::v243;
            case registerSpace::v244: return amdgpu_gfx908::v244;
            case registerSpace::v245: return amdgpu_gfx908::v245;
            case registerSpace::v246: return amdgpu_gfx908::v246;
            case registerSpace::v247: return amdgpu_gfx908::v247;
            case registerSpace::v248: return amdgpu_gfx908::v248;
            case registerSpace::v249: return amdgpu_gfx908::v249;
            case registerSpace::v250: return amdgpu_gfx908::v250;
            case registerSpace::v251: return amdgpu_gfx908::v251;
            case registerSpace::v252: return amdgpu_gfx908::v252;
            case registerSpace::v253: return amdgpu_gfx908::v253;
            case registerSpace::v254: return amdgpu_gfx908::v254;
            case registerSpace::v255: return amdgpu_gfx908::v255;
            default:
                break;
        }
    } else {
        assert(!"Invalid architecture");
    }

    assert(!"Register not handled");
    return InvalidReg;
}

