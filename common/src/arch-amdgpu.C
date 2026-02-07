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

#include "common/src/arch-amdgpu.h"

namespace NS_amdgpu {

namespace RegisterConstants {
DYNINST_EXPORT extern const Register s0(OperandRegId(0), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s1(OperandRegId(1), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s2(OperandRegId(2), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s3(OperandRegId(3), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s4(OperandRegId(4), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s5(OperandRegId(5), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s6(OperandRegId(6), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s7(OperandRegId(7), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s8(OperandRegId(8), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s9(OperandRegId(9), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s10(OperandRegId(10), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s11(OperandRegId(11), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s12(OperandRegId(12), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s13(OperandRegId(13), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s14(OperandRegId(14), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s15(OperandRegId(15), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s16(OperandRegId(16), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s17(OperandRegId(17), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s18(OperandRegId(18), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s19(OperandRegId(19), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s20(OperandRegId(20), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s21(OperandRegId(21), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s22(OperandRegId(22), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s23(OperandRegId(23), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s24(OperandRegId(24), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s25(OperandRegId(25), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s26(OperandRegId(26), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s27(OperandRegId(27), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s28(OperandRegId(28), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s29(OperandRegId(29), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s30(OperandRegId(30), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s31(OperandRegId(31), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s32(OperandRegId(32), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s33(OperandRegId(33), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s34(OperandRegId(34), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s35(OperandRegId(35), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s36(OperandRegId(36), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s37(OperandRegId(37), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s38(OperandRegId(38), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s39(OperandRegId(39), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s40(OperandRegId(40), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s41(OperandRegId(41), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s42(OperandRegId(42), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s43(OperandRegId(43), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s44(OperandRegId(44), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s45(OperandRegId(45), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s46(OperandRegId(46), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s47(OperandRegId(47), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s48(OperandRegId(48), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s49(OperandRegId(49), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s50(OperandRegId(50), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s51(OperandRegId(51), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s52(OperandRegId(52), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s53(OperandRegId(53), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s54(OperandRegId(54), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s55(OperandRegId(55), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s56(OperandRegId(56), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s57(OperandRegId(57), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s58(OperandRegId(58), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s59(OperandRegId(59), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s60(OperandRegId(60), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s61(OperandRegId(61), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s62(OperandRegId(62), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s63(OperandRegId(63), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s64(OperandRegId(64), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s65(OperandRegId(65), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s66(OperandRegId(66), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s67(OperandRegId(67), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s68(OperandRegId(68), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s69(OperandRegId(69), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s70(OperandRegId(70), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s71(OperandRegId(71), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s72(OperandRegId(72), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s73(OperandRegId(73), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s74(OperandRegId(74), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s75(OperandRegId(75), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s76(OperandRegId(76), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s77(OperandRegId(77), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s78(OperandRegId(78), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s79(OperandRegId(79), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s80(OperandRegId(80), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s81(OperandRegId(81), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s82(OperandRegId(82), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s83(OperandRegId(83), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s84(OperandRegId(84), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s85(OperandRegId(85), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s86(OperandRegId(86), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s87(OperandRegId(87), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s88(OperandRegId(88), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s89(OperandRegId(89), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s90(OperandRegId(90), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s91(OperandRegId(91), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s92(OperandRegId(92), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s93(OperandRegId(93), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s94(OperandRegId(94), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s95(OperandRegId(95), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s96(OperandRegId(96), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s97(OperandRegId(97), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s98(OperandRegId(98), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s99(OperandRegId(99), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s100(OperandRegId(100), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register s101(OperandRegId(101), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register flat_scratch_lo(OperandRegId(102), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register flat_scratch_hi(OperandRegId(103), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register xnack_mask_lo(OperandRegId(104), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register xnack_mask_hi(OperandRegId(105), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register vcc_lo(OperandRegId(106), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register vcc_hi(OperandRegId(107), RegKind::SCALAR, BlockSize(1));
DYNINST_EXPORT extern const Register exec_lo(OperandRegId(126), RegKind::SCALAR_PREDICATE, BlockSize(1));
DYNINST_EXPORT extern const Register exec_hi(OperandRegId(127), RegKind::SCALAR_PREDICATE, BlockSize(1));

// Vector registers
DYNINST_EXPORT extern const Register v0(OperandRegId(0), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v1(OperandRegId(1), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v2(OperandRegId(2), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v3(OperandRegId(3), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v4(OperandRegId(4), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v5(OperandRegId(5), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v6(OperandRegId(6), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v7(OperandRegId(7), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v8(OperandRegId(8), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v9(OperandRegId(9), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v10(OperandRegId(10), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v11(OperandRegId(11), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v12(OperandRegId(12), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v13(OperandRegId(13), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v14(OperandRegId(14), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v15(OperandRegId(15), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v16(OperandRegId(16), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v17(OperandRegId(17), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v18(OperandRegId(18), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v19(OperandRegId(19), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v20(OperandRegId(20), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v21(OperandRegId(21), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v22(OperandRegId(22), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v23(OperandRegId(23), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v24(OperandRegId(24), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v25(OperandRegId(25), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v26(OperandRegId(26), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v27(OperandRegId(27), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v28(OperandRegId(28), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v29(OperandRegId(29), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v30(OperandRegId(30), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v31(OperandRegId(31), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v32(OperandRegId(32), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v33(OperandRegId(33), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v34(OperandRegId(34), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v35(OperandRegId(35), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v36(OperandRegId(36), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v37(OperandRegId(37), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v38(OperandRegId(38), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v39(OperandRegId(39), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v40(OperandRegId(40), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v41(OperandRegId(41), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v42(OperandRegId(42), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v43(OperandRegId(43), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v44(OperandRegId(44), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v45(OperandRegId(45), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v46(OperandRegId(46), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v47(OperandRegId(47), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v48(OperandRegId(48), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v49(OperandRegId(49), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v50(OperandRegId(50), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v51(OperandRegId(51), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v52(OperandRegId(52), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v53(OperandRegId(53), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v54(OperandRegId(54), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v55(OperandRegId(55), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v56(OperandRegId(56), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v57(OperandRegId(57), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v58(OperandRegId(58), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v59(OperandRegId(59), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v60(OperandRegId(60), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v61(OperandRegId(61), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v62(OperandRegId(62), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v63(OperandRegId(63), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v64(OperandRegId(64), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v65(OperandRegId(65), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v66(OperandRegId(66), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v67(OperandRegId(67), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v68(OperandRegId(68), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v69(OperandRegId(69), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v70(OperandRegId(70), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v71(OperandRegId(71), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v72(OperandRegId(72), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v73(OperandRegId(73), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v74(OperandRegId(74), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v75(OperandRegId(75), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v76(OperandRegId(76), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v77(OperandRegId(77), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v78(OperandRegId(78), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v79(OperandRegId(79), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v80(OperandRegId(80), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v81(OperandRegId(81), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v82(OperandRegId(82), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v83(OperandRegId(83), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v84(OperandRegId(84), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v85(OperandRegId(85), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v86(OperandRegId(86), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v87(OperandRegId(87), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v88(OperandRegId(88), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v89(OperandRegId(89), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v90(OperandRegId(90), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v91(OperandRegId(91), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v92(OperandRegId(92), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v93(OperandRegId(93), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v94(OperandRegId(94), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v95(OperandRegId(95), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v96(OperandRegId(96), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v97(OperandRegId(97), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v98(OperandRegId(98), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v99(OperandRegId(99), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v100(OperandRegId(100), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v101(OperandRegId(101), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v102(OperandRegId(102), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v103(OperandRegId(103), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v104(OperandRegId(104), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v105(OperandRegId(105), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v106(OperandRegId(106), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v107(OperandRegId(107), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v108(OperandRegId(108), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v109(OperandRegId(109), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v110(OperandRegId(110), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v111(OperandRegId(111), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v112(OperandRegId(112), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v113(OperandRegId(113), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v114(OperandRegId(114), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v115(OperandRegId(115), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v116(OperandRegId(116), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v117(OperandRegId(117), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v118(OperandRegId(118), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v119(OperandRegId(119), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v120(OperandRegId(120), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v121(OperandRegId(121), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v122(OperandRegId(122), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v123(OperandRegId(123), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v124(OperandRegId(124), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v125(OperandRegId(125), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v126(OperandRegId(126), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v127(OperandRegId(127), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v128(OperandRegId(128), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v129(OperandRegId(129), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v130(OperandRegId(130), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v131(OperandRegId(131), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v132(OperandRegId(132), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v133(OperandRegId(133), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v134(OperandRegId(134), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v135(OperandRegId(135), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v136(OperandRegId(136), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v137(OperandRegId(137), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v138(OperandRegId(138), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v139(OperandRegId(139), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v140(OperandRegId(140), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v141(OperandRegId(141), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v142(OperandRegId(142), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v143(OperandRegId(143), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v144(OperandRegId(144), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v145(OperandRegId(145), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v146(OperandRegId(146), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v147(OperandRegId(147), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v148(OperandRegId(148), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v149(OperandRegId(149), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v150(OperandRegId(150), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v151(OperandRegId(151), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v152(OperandRegId(152), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v153(OperandRegId(153), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v154(OperandRegId(154), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v155(OperandRegId(155), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v156(OperandRegId(156), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v157(OperandRegId(157), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v158(OperandRegId(158), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v159(OperandRegId(159), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v160(OperandRegId(160), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v161(OperandRegId(161), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v162(OperandRegId(162), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v163(OperandRegId(163), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v164(OperandRegId(164), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v165(OperandRegId(165), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v166(OperandRegId(166), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v167(OperandRegId(167), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v168(OperandRegId(168), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v169(OperandRegId(169), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v170(OperandRegId(170), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v171(OperandRegId(171), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v172(OperandRegId(172), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v173(OperandRegId(173), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v174(OperandRegId(174), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v175(OperandRegId(175), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v176(OperandRegId(176), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v177(OperandRegId(177), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v178(OperandRegId(178), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v179(OperandRegId(179), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v180(OperandRegId(180), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v181(OperandRegId(181), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v182(OperandRegId(182), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v183(OperandRegId(183), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v184(OperandRegId(184), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v185(OperandRegId(185), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v186(OperandRegId(186), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v187(OperandRegId(187), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v188(OperandRegId(188), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v189(OperandRegId(189), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v190(OperandRegId(190), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v191(OperandRegId(191), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v192(OperandRegId(192), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v193(OperandRegId(193), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v194(OperandRegId(194), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v195(OperandRegId(195), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v196(OperandRegId(196), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v197(OperandRegId(197), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v198(OperandRegId(198), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v199(OperandRegId(199), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v200(OperandRegId(200), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v201(OperandRegId(201), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v202(OperandRegId(202), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v203(OperandRegId(203), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v204(OperandRegId(204), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v205(OperandRegId(205), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v206(OperandRegId(206), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v207(OperandRegId(207), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v208(OperandRegId(208), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v209(OperandRegId(209), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v210(OperandRegId(210), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v211(OperandRegId(211), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v212(OperandRegId(212), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v213(OperandRegId(213), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v214(OperandRegId(214), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v215(OperandRegId(215), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v216(OperandRegId(216), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v217(OperandRegId(217), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v218(OperandRegId(218), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v219(OperandRegId(219), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v220(OperandRegId(220), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v221(OperandRegId(221), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v222(OperandRegId(222), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v223(OperandRegId(223), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v224(OperandRegId(224), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v225(OperandRegId(225), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v226(OperandRegId(226), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v227(OperandRegId(227), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v228(OperandRegId(228), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v229(OperandRegId(229), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v230(OperandRegId(230), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v231(OperandRegId(231), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v232(OperandRegId(232), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v233(OperandRegId(233), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v234(OperandRegId(234), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v235(OperandRegId(235), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v236(OperandRegId(236), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v237(OperandRegId(237), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v238(OperandRegId(238), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v239(OperandRegId(239), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v240(OperandRegId(240), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v241(OperandRegId(241), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v242(OperandRegId(242), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v243(OperandRegId(243), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v244(OperandRegId(244), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v245(OperandRegId(245), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v246(OperandRegId(246), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v247(OperandRegId(247), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v248(OperandRegId(248), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v249(OperandRegId(249), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v250(OperandRegId(250), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v251(OperandRegId(251), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v252(OperandRegId(252), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v253(OperandRegId(253), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v254(OperandRegId(254), RegKind::VECTOR, BlockSize(1));
DYNINST_EXPORT extern const Register v255(OperandRegId(255), RegKind::VECTOR, BlockSize(1));

DYNINST_EXPORT extern const Register ignored{};

} // namespace RegisterConstants

// unsigned int swapBytesIfNeeded(unsigned int [> i <]) {
//   assert(!"Not implemented for AMDGPU");
//   return 0;
// }
//
// int instruction::signExtend(unsigned int [> i */, unsigned int /* pos <]) {
//   assert(!"Not implemented for AMDGPU");
//   return 0;
// }
//
// instructUnion &instruction::swapBytes(instructUnion &i) {
//   assert(!"Not implemented for AMDGPU");
//   return i;
// }
//
// instruction *instruction::copy() const {
//   assert(!"Not implemented for AMDGPU");
//   return nullptr;
// }
//
// unsigned instruction::getTargetReg() const {
//   assert(!"Not implemented for AMDGPU");
//   return 0;
// }
//
// Dyninst::Address instruction::getTarget(Dyninst::Address [> addr <]) const {
//   assert(!"Not implemented for AMDGPU");
//   return 0;
// }
//
// void instruction::setBranchOffset(Dyninst::Address [>newOffset<]) {
//   assert(!"Not implemented for AMDGPU");
// }
//
// bool instruction::isCall() const {
//   assert(!"Not implemented for AMDGPU");
//   return false;
// }
//
// void instruction::setInstruction(codeBuf_t * [>ptr<], Dyninst::Address) {
//   assert(!"Not implemented for AMDGPU");
// }
//
// void instruction::setInstruction(unsigned char * [> ptr <], Dyninst::Address) {
//   assert(!"Not implemented for AMDGPU");
// }
//
// bool instruction::isBranchReg() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// bool instruction::isUncondBranch() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// bool instruction::isCondBranch() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// unsigned instruction::jumpSize(Dyninst::Address [>from*/, Dyninst::Address /*to<],
//                                unsigned [>addr_width<]) {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// unsigned instruction::jumpSize(Dyninst::Address [>disp*/, unsigned /*addr_width<]) {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// unsigned instruction::maxJumpSize(unsigned [> addr_width <]) {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// unsigned instruction::maxInterFunctionJumpSize(unsigned [> addr_width <]) {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// unsigned instruction::spaceToRelocate() const {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// bool instruction::getUsedRegs(std::vector<int> &) {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// bool instruction::isThunk() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// unsigned instruction::getBranchTargetReg() const {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// Dyninst::Address instruction::getBranchOffset() const {
//   assert(!"Not implmented for AMDGPU");
//   return 0;
// }
//
// unsigned instruction::opcode() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// bool instruction::isAtomicLoad() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }
//
// bool instruction::isAtomicStore() const {
//   assert(!"Not implmented for AMDGPU");
//   return false;
// }

} // namespace NS_amdgpu
