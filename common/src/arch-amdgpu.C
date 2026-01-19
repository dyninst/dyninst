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
DYNINST_EXPORT extern const Register s0(0, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s1(1, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s2(2, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s3(3, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s4(4, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s5(5, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s6(6, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s7(7, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s8(8, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s9(9, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s10(10, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s11(11, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s12(12, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s13(13, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s14(14, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s15(15, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s16(16, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s17(17, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s18(18, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s19(19, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s20(20, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s21(21, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s22(22, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s23(23, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s24(24, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s25(25, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s26(26, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s27(27, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s28(28, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s29(29, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s30(30, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s31(31, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s32(32, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s33(33, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s34(34, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s35(35, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s36(36, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s37(37, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s38(38, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s39(39, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s40(40, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s41(41, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s42(42, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s43(43, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s44(44, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s45(45, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s46(46, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s47(47, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s48(48, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s49(49, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s50(50, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s51(51, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s52(52, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s53(53, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s54(54, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s55(55, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s56(56, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s57(57, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s58(58, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s59(59, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s60(60, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s61(61, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s62(62, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s63(63, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s64(64, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s65(65, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s66(66, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s67(67, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s68(68, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s69(69, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s70(70, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s71(71, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s72(72, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s73(73, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s74(74, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s75(75, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s76(76, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s77(77, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s78(78, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s79(79, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s80(80, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s81(81, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s82(82, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s83(83, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s84(84, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s85(85, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s86(86, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s87(87, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s88(88, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s89(89, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s90(90, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s91(91, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s92(92, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s93(93, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s94(94, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s95(95, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s96(96, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s97(97, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s98(98, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s99(99, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s100(100, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register s101(101, SCALAR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register flat_scratch_lo(102, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register flat_scratch_hi(103, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register xnack_mask_lo(104, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register xnack_mask_hi(105, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register vcc_lo(106, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register vcc_hi(107, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register m0(124, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register exec_lo(126, SCALAR, SPECIAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register exec_hi(127, SCALAR, SPECIAL_PURPOSE, 0);

// Vector registers
DYNINST_EXPORT extern const Register v0(0, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v1(1, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v2(2, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v3(3, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v4(4, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v5(5, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v6(6, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v7(7, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v8(8, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v9(9, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v10(10, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v11(11, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v12(12, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v13(13, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v14(14, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v15(15, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v16(16, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v17(17, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v18(18, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v19(19, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v20(20, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v21(21, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v22(22, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v23(23, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v24(24, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v25(25, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v26(26, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v27(27, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v28(28, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v29(29, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v30(30, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v31(31, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v32(32, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v33(33, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v34(34, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v35(35, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v36(36, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v37(37, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v38(38, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v39(39, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v40(40, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v41(41, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v42(42, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v43(43, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v44(44, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v45(45, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v46(46, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v47(47, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v48(48, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v49(49, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v50(50, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v51(51, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v52(52, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v53(53, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v54(54, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v55(55, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v56(56, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v57(57, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v58(58, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v59(59, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v60(60, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v61(61, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v62(62, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v63(63, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v64(64, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v65(65, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v66(66, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v67(67, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v68(68, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v69(69, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v70(70, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v71(71, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v72(72, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v73(73, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v74(74, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v75(75, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v76(76, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v77(77, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v78(78, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v79(79, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v80(80, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v81(81, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v82(82, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v83(83, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v84(84, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v85(85, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v86(86, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v87(87, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v88(88, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v89(89, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v90(90, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v91(91, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v92(92, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v93(93, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v94(94, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v95(95, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v96(96, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v97(97, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v98(98, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v99(99, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v100(100, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v101(101, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v102(102, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v103(103, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v104(104, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v105(105, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v106(106, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v107(107, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v108(108, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v109(109, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v110(110, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v111(111, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v112(112, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v113(113, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v114(114, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v115(115, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v116(116, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v117(117, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v118(118, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v119(119, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v120(120, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v121(121, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v122(122, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v123(123, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v124(124, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v125(125, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v126(126, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v127(127, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v128(128, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v129(129, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v130(130, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v131(131, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v132(132, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v133(133, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v134(134, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v135(135, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v136(136, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v137(137, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v138(138, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v139(139, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v140(140, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v141(141, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v142(142, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v143(143, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v144(144, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v145(145, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v146(146, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v147(147, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v148(148, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v149(149, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v150(150, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v151(151, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v152(152, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v153(153, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v154(154, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v155(155, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v156(156, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v157(157, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v158(158, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v159(159, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v160(160, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v161(161, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v162(162, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v163(163, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v164(164, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v165(165, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v166(166, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v167(167, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v168(168, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v169(169, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v170(170, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v171(171, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v172(172, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v173(173, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v174(174, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v175(175, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v176(176, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v177(177, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v178(178, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v179(179, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v180(180, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v181(181, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v182(182, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v183(183, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v184(184, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v185(185, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v186(186, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v187(187, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v188(188, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v189(189, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v190(190, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v191(191, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v192(192, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v193(193, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v194(194, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v195(195, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v196(196, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v197(197, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v198(198, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v199(199, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v200(200, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v201(201, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v202(202, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v203(203, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v204(204, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v205(205, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v206(206, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v207(207, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v208(208, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v209(209, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v210(210, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v211(211, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v212(212, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v213(213, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v214(214, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v215(215, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v216(216, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v217(217, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v218(218, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v219(219, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v220(220, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v221(221, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v222(222, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v223(223, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v224(224, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v225(225, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v226(226, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v227(227, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v228(228, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v229(229, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v230(230, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v231(231, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v232(232, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v233(233, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v234(234, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v235(235, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v236(236, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v237(237, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v238(238, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v239(239, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v240(240, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v241(241, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v242(242, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v243(243, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v244(244, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v245(245, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v246(246, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v247(247, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v248(248, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v249(249, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v250(250, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v251(251, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v252(252, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v253(253, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v254(254, VECTOR, GENERAL_PURPOSE, 0);
DYNINST_EXPORT extern const Register v255(255, VECTOR, GENERAL_PURPOSE, 0);

DYNINST_EXPORT extern const Register ignored(-1);

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
