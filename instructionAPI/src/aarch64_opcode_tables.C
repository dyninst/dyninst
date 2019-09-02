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
#define INVALID_ENTRY aarch64_insn_entry::main_insn_table[0]

#define fn(...) (&InstructionDecoder_aarch64::__VA_ARGS__)

const operandFactory aarch64_insn_entry::operandTable[] = {
    // INVALID
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // abs
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // abs
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // adc
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // adcs
    fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd), // add
    fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd), // add
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // add
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // add
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // add
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // addhn
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // addp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // addp
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd), // adds
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd), // adds
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // adds
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // addv
    fn(OPRimm<30,29>),fn(OPRimm<23,5>),fn(OPRRd), // adr
    fn(OPRimm<30,29>),fn(OPRimm<23,5>),fn(OPRRd), // adrp
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // aesd
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // aese
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // aesimc
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // aesmc
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // and
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // and
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // and
    fn(setFlags),fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // ands
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // ands
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // asr
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // asr
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // asrv
    fn(OPRop1),fn(OPRop2),fn(OPRRt), // at
    fn(OPRimm<23,5>),fn(OPRcond<3,0>), // b
    fn(OPRimm<25,0>), // b
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // bfi
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // bfm
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // bfxil
    fn(setSIMDMode),fn(OPRQ),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd), // bic
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // bic
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // bic
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // bics
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // bif
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // bit
    fn(OPRimm<25,0>), // bl
    fn(OPRRn), // blr
    fn(OPRRn), // br
    fn(OPRimm<20,5>), // brk
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // bsl
    fn(OPRsf),fn(OPRimm<23,5>),fn(OPRRt), // cbnz
    fn(OPRsf),fn(OPRimm<23,5>),fn(OPRRt), // cbz
    fn(OPRsf),fn(OPRimm<20,16>),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv), // ccmn
    fn(OPRsf),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv), // ccmn
    fn(OPRsf),fn(OPRimm<20,16>),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv), // ccmp
    fn(OPRsf),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv), // ccmp
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd), // cinc
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd), // cinv
    fn(OPRCRm), // clrex
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cls
    fn(OPRsf),fn(OPRRn),fn(OPRRd), // cls
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // clz
    fn(OPRsf),fn(OPRRn),fn(OPRRd), // clz
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmeq
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmeq
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmeq
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmeq
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmge
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmge
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmge
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmge
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmgt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmgt
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmgt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmgt
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmhi
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmhi
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmhs
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmhs
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmle
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmle
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmlt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cmlt
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn), // cmn
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn), // cmn
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn), // cmn
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn), // cmp
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn), // cmp
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn), // cmp
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmtst
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // cmtst
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd), // cneg
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // cnt
    fn(OPRsf),fn(OPRRm),fn(OPRsz<11,10>),fn(OPRRn),fn(OPRRd), // crc32
    fn(OPRsf),fn(OPRRm),fn(OPRsz<11,10>),fn(OPRRn),fn(OPRRd), // crc32c
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // csel
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRd), // cset
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRd), // csetm
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // csinc
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // csinv
    fn(OPRsf),fn(OPRcond<15,12>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // csneg
    fn(OPRop1),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // dc
    fn(OPRimm<20,5>), // dcps1
    fn(OPRimm<20,5>), // dcps2
    fn(OPRimm<20,5>), // dcps3
    fn(OPRCRm), // dmb
    // drps
    fn(OPRCRm), // dsb
    fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // dup
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // dup
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // dup
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // eon
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // eor
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // eor
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // eor
    // eret
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRimm<14,11>),fn(OPRRn),fn(OPRRd), // ext
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // extr
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fabd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fabd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fabs
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fabs
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // facge
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // facge
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // facgt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // facgt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fadd
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fadd
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // faddp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // faddp
    fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv), // fccmp
    fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRnzcv), // fccmpe
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fcmeq
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fcmeq
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmeq
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmeq
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fcmge
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fcmge
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmge
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmge
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fcmgt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fcmgt
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmgt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmgt
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmle
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmle
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmlt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcmlt
    fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRopc), // fcmp
    fn(setFPMode),fn(setFlags),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRopc), // fcmpe
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRcond<15,12>),fn(OPRRn),fn(OPRRd), // fcsel
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRopc),fn(OPRRn),fn(OPRRd), // fcvt
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtas
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtas
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtas
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtau
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtau
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtau
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtl
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtms
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtms
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtms
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtmu
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtmu
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtmu
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtn
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtns
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtns
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtns
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtnu
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtnu
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtnu
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtps
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtps
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtps
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtpu
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtpu
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtpu
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtxn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtxn
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // fcvtzs
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // fcvtzs
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtzs
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtzs
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd), // fcvtzs
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtzs
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // fcvtzu
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // fcvtzu
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtzu
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fcvtzu
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd), // fcvtzu
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fcvtzu
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fdiv
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fdiv
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // fmadd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmax
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmax
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmaxnm
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmaxnm
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fmaxnmp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmaxnmp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fmaxnmv
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fmaxp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmaxp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fmaxv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmin
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmin
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fminnm
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fminnm
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fminnmp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fminnmp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fminnmv
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fminp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fminp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fminv
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmla
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmla
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmla
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmls
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmls
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmls
    fn(setSIMDMode),fn(OPRQ),fn(OPRop),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd), // fmov
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fmov
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRrmode),fn(OPRopcode),fn(OPRRn),fn(OPRRd), // fmov
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRimm<20,13>),fn(OPRRd), // fmov
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // fmsub
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmul
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmul
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmul
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmul
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmulx
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // fmulx
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmulx
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fmulx
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fneg
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fneg
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // fnmadd
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // fnmsub
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fnmul
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frecpe
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frecpe
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // frecps
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // frecps
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frecpx
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frinta
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frinta
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frinti
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frinti
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frintm
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frintm
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frintn
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frintn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frintp
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frintp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frintx
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frintx
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frintz
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // frintz
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frsqrte
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // frsqrte
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // frsqrts
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // frsqrts
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // fsqrt
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // fsqrt
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fsub
    fn(setFPMode),fn(OPRtype<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // fsub
    fn(OPRCRm),fn(OPRop2), // hint
    fn(OPRimm<20,5>), // hlt
    fn(OPRimm<20,5>), // hvc
    fn(OPRop1),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // ic
    fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRimm<14,11>),fn(OPRRn),fn(OPRRd), // ins
    fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // ins
    fn(OPRCRm), // isb
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld1
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld1
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld1
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld1
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld1r
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld1r
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld2
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld2
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld2
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld2
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld2r
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld2r
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld3
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld3
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld3
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld3
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld3r
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld3r
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld4
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld4
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld4
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld4
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld4r
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnL),fn(OPRRtL), // ld4r
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnL),fn(OPRRtL), // ldar
    fn(setRegWidth),fn(OPRRnL),fn(OPRRtL), // ldarb
    fn(setRegWidth),fn(OPRRnL),fn(OPRRtL), // ldarh
    fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldaxp
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnL),fn(OPRRtL), // ldaxr
    fn(setRegWidth),fn(OPRRnL),fn(OPRRtL), // ldaxrb
    fn(setRegWidth),fn(OPRRnL),fn(OPRRtL), // ldaxrh
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldnp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldnp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL), // ldp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL), // ldp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL), // ldp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL), // ldp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldp
    fn(setRegWidth),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL), // ldpsw
    fn(setRegWidth),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnLU),fn(OPRRtL), // ldpsw
    fn(setRegWidth),fn(OPRimm<21,15>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldpsw
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<23,5>),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<23,5>),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldr
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrb
    fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldrb
    fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldrb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrh
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrh
    fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldrh
    fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldrh
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrsb
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrsb
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldrsb
    fn(setRegWidth),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldrsb
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrsh
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrsh
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldrsh
    fn(setRegWidth),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldrsh
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrsw
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnLU),fn(OPRRtL), // ldrsw
    fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRtL), // ldrsw
    fn(setRegWidth),fn(OPRimm<23,5>),fn(OPRRtL), // ldrsw
    fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRtL), // ldrsw
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldtr
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldtrb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldtrh
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldtrsb
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldtrsh
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldtrsw
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldur
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldur
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldurb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldurh
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldursb
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldursh
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRtL), // ldursw
    fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRt2L),fn(OPRRnL),fn(OPRRtL), // ldxp
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnL),fn(OPRRtL), // ldxr
    fn(setRegWidth),fn(OPRRnL),fn(OPRRtL), // ldxrb
    fn(setRegWidth),fn(OPRRnL),fn(OPRRtL), // ldxrh
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // lsl
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // lsl
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // lslv
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // lsr
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // lsr
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // lsrv
    fn(OPRsf),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // madd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // mla
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // mla
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // mls
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // mls
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // mneg
    fn(OPRsf),fn(OPRRn),fn(OPRRd), // mov
    fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // mov
    fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRimm<14,11>),fn(OPRRn),fn(OPRRd), // mov
    fn(setSIMDMode),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // mov
    fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd), // mov
    fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd), // mov
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // mov
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRd), // mov
    fn(OPRsf),fn(OPRRm),fn(OPRRd), // mov
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // mov
    fn(setSIMDMode),fn(OPRQ),fn(OPRop),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd), // movi
    fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd), // movk
    fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd), // movn
    fn(OPRsf),fn(OPRhw),fn(OPRimm<20,5>),fn(OPRRd), // movz
    fn(OPRo0),fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // mrs
    fn(OPRop1),fn(OPRCRm),fn(OPRop2), // msr
    fn(OPRo0),fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // msr
    fn(OPRsf),fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // msub
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // mul
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // mul
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // mul
    fn(setSIMDMode),fn(OPRQ),fn(OPRRn),fn(OPRRd), // mvn
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRd), // mvn
    fn(setSIMDMode),fn(OPRQ),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd), // mvni
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // neg
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // neg
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRd), // neg
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRd), // negs
    fn(OPRsf),fn(OPRRm),fn(OPRRd), // ngc
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRRd), // ngcs
    // nop
    fn(setSIMDMode),fn(OPRQ),fn(OPRRn),fn(OPRRd), // not
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // orn
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // orn
    fn(setSIMDMode),fn(OPRQ),fn(OPRa),fn(OPRb),fn(OPRc),fn(OPRcmode),fn(OPRd),fn(OPRe),fn(OPRf),fn(OPRg),fn(OPRh),fn(OPRRd), // orr
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRRn),fn(OPRRd), // orr
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // orr
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // orr
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // pmul
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // pmull
    fn(OPRimm<21,10>),fn(OPRRnL),fn(OPRRt), // prfm
    fn(OPRimm<23,5>),fn(OPRRt), // prfm
    fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnL),fn(OPRRt), // prfm
    fn(OPRimm<20,12>),fn(OPRRnL),fn(OPRRt), // prfum
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // raddhn
    fn(setSIMDMode),fn(OPRQ),fn(OPRRn),fn(OPRRd), // rbit
    fn(OPRsf),fn(OPRRn),fn(OPRRd), // rbit
    fn(OPRRn), // ret
    fn(OPRsf),fn(OPRopc),fn(OPRRn),fn(OPRRd), // rev
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // rev16
    fn(OPRsf),fn(OPRRn),fn(OPRRd), // rev16
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // rev32
    fn(OPRRn),fn(OPRRd), // rev32
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // rev64
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // ror
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // ror
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // rorv
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // rshrn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // rsubhn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // saba
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sabal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sabd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sabdl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sadalp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // saddl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // saddlp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // saddlv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // saddw
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sbc
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sbcs
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // sbfiz
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // sbfm
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // sbfx
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // scvtf
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // scvtf
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // scvtf
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // scvtf
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd), // scvtf
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // scvtf
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sdiv
    // sev
    // sevl
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha1c
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // sha1h
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha1m
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha1p
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha1su0
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // sha1su1
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha256h2
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha256h
    fn(setSIMDMode),fn(OPRRn),fn(OPRRd), // sha256su0
    fn(setSIMDMode),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sha256su1
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // shadd
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // shl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // shl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // shll
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // shrn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // shsub
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sli
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sli
    fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // smaddl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // smax
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // smaxp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // smaxv
    fn(OPRimm<20,5>), // smc
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // smin
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sminp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sminv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // smlal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // smlal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // smlsl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // smlsl
    fn(OPRRm),fn(OPRRn),fn(OPRRd), // smnegl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // smov
    fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // smsubl
    fn(OPRRm),fn(OPRRn),fn(OPRRd), // smulh
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // smull
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // smull
    fn(OPRRm),fn(OPRRn),fn(OPRRd), // smull
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqabs
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqabs
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqadd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqadd
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmlal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmlal
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmlal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmlal
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmlsl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmlsl
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmlsl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmlsl
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmulh
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmulh
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmulh
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmulh
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmull
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqdmull
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmull
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqdmull
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqneg
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqneg
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqrdmulh
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // sqrdmulh
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqrdmulh
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqrdmulh
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqrshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqrshl
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqrshrn
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqrshrn
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqrshrun
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqrshrun
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshl
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqshl
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshlu
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshlu
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshrn
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshrn
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshrun
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sqshrun
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqsub
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sqsub
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqxtn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqxtn
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqxtun
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // sqxtun
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // srhadd
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sri
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sri
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // srshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // srshl
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // srshr
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // srshr
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // srsra
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // srsra
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sshll
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sshr
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // sshr
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ssra
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ssra
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // ssubl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // ssubw
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st1
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st1
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st1
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st1
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st2
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st2
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st2
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st2
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st3
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st3
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st3
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st3
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st4
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st4
    fn(setSIMDMode),fn(OPRQ),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st4
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRopcode),fn(OPRS<12,12>),fn(OPRsize<11,10>),fn(OPRRnS),fn(OPRRtS), // st4
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRnS),fn(OPRRtS), // stlr
    fn(setRegWidth),fn(OPRRnS),fn(OPRRtS), // stlrb
    fn(setRegWidth),fn(OPRRnS),fn(OPRRtS), // stlrh
    fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRs),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS), // stlxp
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRs),fn(OPRRnS),fn(OPRRtS), // stlxr
    fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS), // stlxrb
    fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS), // stlxrh
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS), // stnp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS), // stnp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS), // stp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS), // stp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS), // stp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS), // stp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnSU),fn(OPRRtS), // stp
    fn(setRegWidth),fn(OPRopc),fn(OPRimm<21,15>),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS), // stp
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // str
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // str
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS), // str
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // str
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // str
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS), // str
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS), // str
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS), // str
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // strb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // strb
    fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS), // strb
    fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS), // strb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // strh
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnSU),fn(OPRRtS), // strh
    fn(setRegWidth),fn(OPRimm<21,10>),fn(OPRRnS),fn(OPRRtS), // strh
    fn(setRegWidth),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRS<12,12>),fn(OPRRnS),fn(OPRRtS), // strh
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // sttr
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // sttrb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // sttrh
    fn(setRegWidth),fn(setSIMDMode),fn(OPRsize<31,30>),fn(OPRopc),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // stur
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // stur
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // sturb
    fn(setRegWidth),fn(OPRimm<20,12>),fn(OPRRnS),fn(OPRRtS), // sturh
    fn(setRegWidth),fn(OPRsz<30,30>),fn(OPRRs),fn(OPRRt2S),fn(OPRRnS),fn(OPRRtS), // stxp
    fn(setRegWidth),fn(OPRsize<31,30>),fn(OPRRs),fn(OPRRnS),fn(OPRRtS), // stxr
    fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS), // stxrb
    fn(setRegWidth),fn(OPRRs),fn(OPRRnS),fn(OPRRtS), // stxrh
    fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd), // sub
    fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd), // sub
    fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // sub
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sub
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // sub
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // subhn
    fn(setFlags),fn(OPRsf),fn(OPRRm),fn(OPRoption<15,13>),fn(OPRimm<12,10>),fn(OPRRn),fn(OPRRd), // subs
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRimm<21,10>),fn(OPRRn),fn(OPRRd), // subs
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // subs
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // suqadd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // suqadd
    fn(OPRimm<20,5>), // svc
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRRn),fn(OPRRd), // sxtb
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRRn),fn(OPRRd), // sxth
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRRn),fn(OPRRd), // sxtl
    fn(OPRRn),fn(OPRRd), // sxtw
    fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // sys
    fn(OPRop1),fn(OPRCRn),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // sysl
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRlen),fn(OPRRn),fn(OPRRd), // tbl
    fn(OPRb5),fn(OPRb40),fn(OPRimm<18,5>),fn(OPRRt), // tbnz
    fn(setSIMDMode),fn(OPRQ),fn(OPRRm),fn(OPRlen),fn(OPRRn),fn(OPRRd), // tbx
    fn(OPRb5),fn(OPRb40),fn(OPRimm<18,5>),fn(OPRRt), // tbz
    fn(OPRop1),fn(OPRCRm),fn(OPRop2),fn(OPRRt), // tlbi
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // trn1
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // trn2
    fn(setFlags),fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn), // tst
    fn(setFlags),fn(OPRsf),fn(OPRshift),fn(OPRRm),fn(OPRimm<15,10>),fn(OPRRn), // tst
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uaba
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uabal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uabd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uabdl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // uadalp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uaddl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // uaddlp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // uaddlv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uaddw
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // ubfiz
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // ubfm
    fn(OPRsf),fn(OPRN<22,22>),fn(OPRimm<21,16>),fn(OPRimm<15,10>),fn(OPRRn),fn(OPRRd), // ubfx
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ucvtf
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ucvtf
    fn(setSIMDMode),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // ucvtf
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // ucvtf
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRscale),fn(OPRRn),fn(OPRRd), // ucvtf
    fn(setFPMode),fn(OPRsf),fn(OPRtype<23,22>),fn(OPRRn),fn(OPRRd), // ucvtf
    fn(OPRsf),fn(OPRRm),fn(OPRRn),fn(OPRRd), // udiv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uhadd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uhsub
    fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // umaddl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // umax
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // umaxp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // umaxv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // umin
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uminp
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // uminv
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // umlal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // umlal
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // umlsl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // umlsl
    fn(OPRRm),fn(OPRRn),fn(OPRRd), // umnegl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<20,16>),fn(OPRRn),fn(OPRRd), // umov
    fn(OPRRm),fn(OPRRa),fn(OPRRn),fn(OPRRd), // umsubl
    fn(OPRRm),fn(OPRRn),fn(OPRRd), // umulh
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRL),fn(OPRM),fn(OPRRm),fn(OPRH),fn(OPRRn),fn(OPRRd), // umull
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // umull
    fn(OPRRm),fn(OPRRn),fn(OPRRd), // umull
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqadd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqadd
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqrshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqrshl
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // uqrshrn
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // uqrshrn
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // uqshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // uqshl
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqshl
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // uqshrn
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // uqshrn
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqsub
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uqsub
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // uqxtn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // uqxtn
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // urecpe
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // urhadd
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // urshl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // urshl
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // urshr
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // urshr
    fn(setSIMDMode),fn(OPRQ),fn(OPRsz<22,22>),fn(OPRRn),fn(OPRRd), // ursqrte
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ursra
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ursra
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // ushl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // ushl
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ushll
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ushr
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // ushr
    fn(setSIMDMode),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // usqadd
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // usqadd
    fn(setSIMDMode),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // usra
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRimm<18,16>),fn(OPRRn),fn(OPRRd), // usra
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // usubl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // usubw
    fn(OPRRn),fn(OPRRd), // uxtb
    fn(OPRRn),fn(OPRRd), // uxth
    fn(setSIMDMode),fn(OPRQ),fn(OPRimm<22,19>),fn(OPRRn),fn(OPRRd), // uxtl
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uzp1
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // uzp2
    // wfe
    // wfi
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRn),fn(OPRRd), // xtn
    // yield
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // zip1
    fn(setSIMDMode),fn(OPRQ),fn(OPRsize<23,22>),fn(OPRRm),fn(OPRRn),fn(OPRRd), // zip2  
};  // end operandTable

const aarch64_insn_table aarch64_insn_entry::main_insn_table = {
    {aarch64_op_INVALID,"INVALID",0,&operandTable[0]},
    {aarch64_op_abs_advsimd,"abs",4,&operandTable[0]},
    {aarch64_op_abs_advsimd,"abs",5,&operandTable[4]},
    {aarch64_op_adc,"adc",4,&operandTable[9]},
    {aarch64_op_adcs,"adcs",5,&operandTable[13]},
    {aarch64_op_add_addsub_ext,"add",6,&operandTable[18]},
    {aarch64_op_add_addsub_imm,"add",5,&operandTable[24]},
    {aarch64_op_add_addsub_shift,"add",6,&operandTable[29]},
    {aarch64_op_add_advsimd,"add",5,&operandTable[35]},
    {aarch64_op_add_advsimd,"add",6,&operandTable[40]},
    {aarch64_op_addhn_advsimd,"addhn",6,&operandTable[46]},
    {aarch64_op_addp_advsimd_pair,"addp",4,&operandTable[52]},
    {aarch64_op_addp_advsimd_vec,"addp",6,&operandTable[56]},
    {aarch64_op_adds_addsub_ext,"adds",7,&operandTable[62]},
    {aarch64_op_adds_addsub_imm,"adds",6,&operandTable[69]},
    {aarch64_op_adds_addsub_shift,"adds",7,&operandTable[75]},
    {aarch64_op_addv_advsimd,"addv",5,&operandTable[82]},
    {aarch64_op_adr,"adr",3,&operandTable[87]},
    {aarch64_op_adrp,"adrp",3,&operandTable[90]},
    {aarch64_op_aesd_advsimd,"aesd",3,&operandTable[93]},
    {aarch64_op_aese_advsimd,"aese",3,&operandTable[96]},
    {aarch64_op_aesimc_advsimd,"aesimc",3,&operandTable[99]},
    {aarch64_op_aesmc_advsimd,"aesmc",3,&operandTable[102]},
    {aarch64_op_and_advsimd,"and",5,&operandTable[105]},
    {aarch64_op_and_log_imm,"and",6,&operandTable[110]},
    {aarch64_op_and_log_shift,"and",6,&operandTable[116]},
    {aarch64_op_ands_log_imm,"ands",7,&operandTable[122]},
    {aarch64_op_ands_log_shift,"ands",7,&operandTable[129]},
    {aarch64_op_asr_asrv,"asr",4,&operandTable[136]},
    {aarch64_op_asr_sbfm,"asr",6,&operandTable[140]},
    {aarch64_op_asrv,"asrv",4,&operandTable[146]},
    {aarch64_op_at_sys,"at",3,&operandTable[150]},
    {aarch64_op_b_cond,"b",2,&operandTable[153]},
    {aarch64_op_b_uncond,"b",1,&operandTable[155]},
    {aarch64_op_bfi_bfm,"bfi",6,&operandTable[156]},
    {aarch64_op_bfm,"bfm",6,&operandTable[162]},
    {aarch64_op_bfxil_bfm,"bfxil",6,&operandTable[168]},
    {aarch64_op_bic_advsimd_imm,"bic",12,&operandTable[174]},
    {aarch64_op_bic_advsimd_reg,"bic",5,&operandTable[186]},
    {aarch64_op_bic_log_shift,"bic",6,&operandTable[191]},
    {aarch64_op_bics,"bics",7,&operandTable[197]},
    {aarch64_op_bif_advsimd,"bif",5,&operandTable[204]},
    {aarch64_op_bit_advsimd,"bit",5,&operandTable[209]},
    {aarch64_op_bl,"bl",1,&operandTable[214]},
    {aarch64_op_blr,"blr",1,&operandTable[215]},
    {aarch64_op_br,"br",1,&operandTable[216]},
    {aarch64_op_brk,"brk",1,&operandTable[217]},
    {aarch64_op_bsl_advsimd,"bsl",5,&operandTable[218]},
    {aarch64_op_cbnz,"cbnz",3,&operandTable[223]},
    {aarch64_op_cbz,"cbz",3,&operandTable[226]},
    {aarch64_op_ccmn_imm,"ccmn",5,&operandTable[229]},
    {aarch64_op_ccmn_reg,"ccmn",5,&operandTable[234]},
    {aarch64_op_ccmp_imm,"ccmp",5,&operandTable[239]},
    {aarch64_op_ccmp_reg,"ccmp",5,&operandTable[244]},
    {aarch64_op_cinc_csinc,"cinc",4,&operandTable[249]},
    {aarch64_op_cinv_csinv,"cinv",4,&operandTable[253]},
    {aarch64_op_clrex,"clrex",1,&operandTable[257]},
    {aarch64_op_cls_advsimd,"cls",5,&operandTable[258]},
    {aarch64_op_cls_int,"cls",3,&operandTable[263]},
    {aarch64_op_clz_advsimd,"clz",5,&operandTable[266]},
    {aarch64_op_clz_int,"clz",3,&operandTable[271]},
    {aarch64_op_cmeq_advsimd_reg,"cmeq",5,&operandTable[274]},
    {aarch64_op_cmeq_advsimd_reg,"cmeq",6,&operandTable[279]},
    {aarch64_op_cmeq_advsimd_zero,"cmeq",4,&operandTable[285]},
    {aarch64_op_cmeq_advsimd_zero,"cmeq",5,&operandTable[289]},
    {aarch64_op_cmge_advsimd_reg,"cmge",5,&operandTable[294]},
    {aarch64_op_cmge_advsimd_reg,"cmge",6,&operandTable[299]},
    {aarch64_op_cmge_advsimd_zero,"cmge",4,&operandTable[305]},
    {aarch64_op_cmge_advsimd_zero,"cmge",5,&operandTable[309]},
    {aarch64_op_cmgt_advsimd_reg,"cmgt",5,&operandTable[314]},
    {aarch64_op_cmgt_advsimd_reg,"cmgt",6,&operandTable[319]},
    {aarch64_op_cmgt_advsimd_zero,"cmgt",4,&operandTable[325]},
    {aarch64_op_cmgt_advsimd_zero,"cmgt",5,&operandTable[329]},
    {aarch64_op_cmhi_advsimd,"cmhi",5,&operandTable[334]},
    {aarch64_op_cmhi_advsimd,"cmhi",6,&operandTable[339]},
    {aarch64_op_cmhs_advsimd,"cmhs",5,&operandTable[345]},
    {aarch64_op_cmhs_advsimd,"cmhs",6,&operandTable[350]},
    {aarch64_op_cmle_advsimd,"cmle",4,&operandTable[356]},
    {aarch64_op_cmle_advsimd,"cmle",5,&operandTable[360]},
    {aarch64_op_cmlt_advsimd,"cmlt",4,&operandTable[365]},
    {aarch64_op_cmlt_advsimd,"cmlt",5,&operandTable[369]},
    {aarch64_op_cmn_adds_addsub_ext,"cmn",6,&operandTable[374]},
    {aarch64_op_cmn_adds_addsub_imm,"cmn",5,&operandTable[380]},
    {aarch64_op_cmn_adds_addsub_shift,"cmn",6,&operandTable[385]},
    {aarch64_op_cmp_subs_addsub_ext,"cmp",6,&operandTable[391]},
    {aarch64_op_cmp_subs_addsub_imm,"cmp",5,&operandTable[397]},
    {aarch64_op_cmp_subs_addsub_shift,"cmp",6,&operandTable[402]},
    {aarch64_op_cmtst_advsimd,"cmtst",5,&operandTable[408]},
    {aarch64_op_cmtst_advsimd,"cmtst",6,&operandTable[413]},
    {aarch64_op_cneg_csneg,"cneg",4,&operandTable[419]},
    {aarch64_op_cnt_advsimd,"cnt",5,&operandTable[423]},
    {aarch64_op_crc32,"crc32",5,&operandTable[428]},
    {aarch64_op_crc32c,"crc32c",5,&operandTable[433]},
    {aarch64_op_csel,"csel",5,&operandTable[438]},
    {aarch64_op_cset_csinc,"cset",3,&operandTable[443]},
    {aarch64_op_csetm_csinv,"csetm",3,&operandTable[446]},
    {aarch64_op_csinc,"csinc",5,&operandTable[449]},
    {aarch64_op_csinv,"csinv",5,&operandTable[454]},
    {aarch64_op_csneg,"csneg",5,&operandTable[459]},
    {aarch64_op_dc_sys,"dc",4,&operandTable[464]},
    {aarch64_op_dcps1,"dcps1",1,&operandTable[468]},
    {aarch64_op_dcps2,"dcps2",1,&operandTable[469]},
    {aarch64_op_dcps3,"dcps3",1,&operandTable[470]},
    {aarch64_op_dmb,"dmb",1,&operandTable[471]},
    {aarch64_op_drps,"drps",0,&operandTable[472]},
    {aarch64_op_dsb,"dsb",1,&operandTable[472]},
    {aarch64_op_dup_advsimd_elt,"dup",4,&operandTable[473]},
    {aarch64_op_dup_advsimd_elt,"dup",5,&operandTable[477]},
    {aarch64_op_dup_advsimd_gen,"dup",5,&operandTable[482]},
    {aarch64_op_eon,"eon",6,&operandTable[487]},
    {aarch64_op_eor_advsimd,"eor",5,&operandTable[493]},
    {aarch64_op_eor_log_imm,"eor",6,&operandTable[498]},
    {aarch64_op_eor_log_shift,"eor",6,&operandTable[504]},
    {aarch64_op_eret,"eret",0,&operandTable[510]},
    {aarch64_op_ext_advsimd,"ext",6,&operandTable[510]},
    {aarch64_op_extr,"extr",6,&operandTable[516]},
    {aarch64_op_fabd_advsimd,"fabd",5,&operandTable[522]},
    {aarch64_op_fabd_advsimd,"fabd",6,&operandTable[527]},
    {aarch64_op_fabs_advsimd,"fabs",5,&operandTable[533]},
    {aarch64_op_fabs_float,"fabs",4,&operandTable[538]},
    {aarch64_op_facge_advsimd,"facge",5,&operandTable[542]},
    {aarch64_op_facge_advsimd,"facge",6,&operandTable[547]},
    {aarch64_op_facgt_advsimd,"facgt",5,&operandTable[553]},
    {aarch64_op_facgt_advsimd,"facgt",6,&operandTable[558]},
    {aarch64_op_fadd_advsimd,"fadd",6,&operandTable[564]},
    {aarch64_op_fadd_float,"fadd",5,&operandTable[570]},
    {aarch64_op_faddp_advsimd_pair,"faddp",4,&operandTable[575]},
    {aarch64_op_faddp_advsimd_vec,"faddp",6,&operandTable[579]},
    {aarch64_op_fccmp_float,"fccmp",7,&operandTable[585]},
    {aarch64_op_fccmpe_float,"fccmpe",7,&operandTable[592]},
    {aarch64_op_fcmeq_advsimd_reg,"fcmeq",5,&operandTable[599]},
    {aarch64_op_fcmeq_advsimd_reg,"fcmeq",6,&operandTable[604]},
    {aarch64_op_fcmeq_advsimd_zero,"fcmeq",4,&operandTable[610]},
    {aarch64_op_fcmeq_advsimd_zero,"fcmeq",5,&operandTable[614]},
    {aarch64_op_fcmge_advsimd_reg,"fcmge",5,&operandTable[619]},
    {aarch64_op_fcmge_advsimd_reg,"fcmge",6,&operandTable[624]},
    {aarch64_op_fcmge_advsimd_zero,"fcmge",4,&operandTable[630]},
    {aarch64_op_fcmge_advsimd_zero,"fcmge",5,&operandTable[634]},
    {aarch64_op_fcmgt_advsimd_reg,"fcmgt",5,&operandTable[639]},
    {aarch64_op_fcmgt_advsimd_reg,"fcmgt",6,&operandTable[644]},
    {aarch64_op_fcmgt_advsimd_zero,"fcmgt",4,&operandTable[650]},
    {aarch64_op_fcmgt_advsimd_zero,"fcmgt",5,&operandTable[654]},
    {aarch64_op_fcmle_advsimd,"fcmle",4,&operandTable[659]},
    {aarch64_op_fcmle_advsimd,"fcmle",5,&operandTable[663]},
    {aarch64_op_fcmlt_advsimd,"fcmlt",4,&operandTable[668]},
    {aarch64_op_fcmlt_advsimd,"fcmlt",5,&operandTable[672]},
    {aarch64_op_fcmp_float,"fcmp",6,&operandTable[677]},
    {aarch64_op_fcmpe_float,"fcmpe",6,&operandTable[683]},
    {aarch64_op_fcsel_float,"fcsel",6,&operandTable[689]},
    {aarch64_op_fcvt_float,"fcvt",5,&operandTable[695]},
    {aarch64_op_fcvtas_advsimd,"fcvtas",4,&operandTable[700]},
    {aarch64_op_fcvtas_advsimd,"fcvtas",5,&operandTable[704]},
    {aarch64_op_fcvtas_float,"fcvtas",5,&operandTable[709]},
    {aarch64_op_fcvtau_advsimd,"fcvtau",4,&operandTable[714]},
    {aarch64_op_fcvtau_advsimd,"fcvtau",5,&operandTable[718]},
    {aarch64_op_fcvtau_float,"fcvtau",5,&operandTable[723]},
    {aarch64_op_fcvtl_advsimd,"fcvtl",5,&operandTable[728]},
    {aarch64_op_fcvtms_advsimd,"fcvtms",4,&operandTable[733]},
    {aarch64_op_fcvtms_advsimd,"fcvtms",5,&operandTable[737]},
    {aarch64_op_fcvtms_float,"fcvtms",5,&operandTable[742]},
    {aarch64_op_fcvtmu_advsimd,"fcvtmu",4,&operandTable[747]},
    {aarch64_op_fcvtmu_advsimd,"fcvtmu",5,&operandTable[751]},
    {aarch64_op_fcvtmu_float,"fcvtmu",5,&operandTable[756]},
    {aarch64_op_fcvtn_advsimd,"fcvtn",5,&operandTable[761]},
    {aarch64_op_fcvtns_advsimd,"fcvtns",4,&operandTable[766]},
    {aarch64_op_fcvtns_advsimd,"fcvtns",5,&operandTable[770]},
    {aarch64_op_fcvtns_float,"fcvtns",5,&operandTable[775]},
    {aarch64_op_fcvtnu_advsimd,"fcvtnu",4,&operandTable[780]},
    {aarch64_op_fcvtnu_advsimd,"fcvtnu",5,&operandTable[784]},
    {aarch64_op_fcvtnu_float,"fcvtnu",5,&operandTable[789]},
    {aarch64_op_fcvtps_advsimd,"fcvtps",4,&operandTable[794]},
    {aarch64_op_fcvtps_advsimd,"fcvtps",5,&operandTable[798]},
    {aarch64_op_fcvtps_float,"fcvtps",5,&operandTable[803]},
    {aarch64_op_fcvtpu_advsimd,"fcvtpu",4,&operandTable[808]},
    {aarch64_op_fcvtpu_advsimd,"fcvtpu",5,&operandTable[812]},
    {aarch64_op_fcvtpu_float,"fcvtpu",5,&operandTable[817]},
    {aarch64_op_fcvtxn_advsimd,"fcvtxn",4,&operandTable[822]},
    {aarch64_op_fcvtxn_advsimd,"fcvtxn",5,&operandTable[826]},
    {aarch64_op_fcvtzs_advsimd_fix,"fcvtzs",5,&operandTable[831]},
    {aarch64_op_fcvtzs_advsimd_fix,"fcvtzs",6,&operandTable[836]},
    {aarch64_op_fcvtzs_advsimd_int,"fcvtzs",4,&operandTable[842]},
    {aarch64_op_fcvtzs_advsimd_int,"fcvtzs",5,&operandTable[846]},
    {aarch64_op_fcvtzs_float_fix,"fcvtzs",6,&operandTable[851]},
    {aarch64_op_fcvtzs_float_int,"fcvtzs",5,&operandTable[857]},
    {aarch64_op_fcvtzu_advsimd_fix,"fcvtzu",5,&operandTable[862]},
    {aarch64_op_fcvtzu_advsimd_fix,"fcvtzu",6,&operandTable[867]},
    {aarch64_op_fcvtzu_advsimd_int,"fcvtzu",4,&operandTable[873]},
    {aarch64_op_fcvtzu_advsimd_int,"fcvtzu",5,&operandTable[877]},
    {aarch64_op_fcvtzu_float_fix,"fcvtzu",6,&operandTable[882]},
    {aarch64_op_fcvtzu_float_int,"fcvtzu",5,&operandTable[888]},
    {aarch64_op_fdiv_advsimd,"fdiv",6,&operandTable[893]},
    {aarch64_op_fdiv_float,"fdiv",5,&operandTable[899]},
    {aarch64_op_fmadd_float,"fmadd",6,&operandTable[904]},
    {aarch64_op_fmax_advsimd,"fmax",6,&operandTable[910]},
    {aarch64_op_fmax_float,"fmax",5,&operandTable[916]},
    {aarch64_op_fmaxnm_advsimd,"fmaxnm",6,&operandTable[921]},
    {aarch64_op_fmaxnm_float,"fmaxnm",5,&operandTable[927]},
    {aarch64_op_fmaxnmp_advsimd_pair,"fmaxnmp",4,&operandTable[932]},
    {aarch64_op_fmaxnmp_advsimd_vec,"fmaxnmp",6,&operandTable[936]},
    {aarch64_op_fmaxnmv_advsimd,"fmaxnmv",5,&operandTable[942]},
    {aarch64_op_fmaxp_advsimd_pair,"fmaxp",4,&operandTable[947]},
    {aarch64_op_fmaxp_advsimd_vec,"fmaxp",6,&operandTable[951]},
    {aarch64_op_fmaxv_advsimd,"fmaxv",5,&operandTable[957]},
    {aarch64_op_fmin_advsimd,"fmin",6,&operandTable[962]},
    {aarch64_op_fmin_float,"fmin",5,&operandTable[968]},
    {aarch64_op_fminnm_advsimd,"fminnm",6,&operandTable[973]},
    {aarch64_op_fminnm_float,"fminnm",5,&operandTable[979]},
    {aarch64_op_fminnmp_advsimd_pair,"fminnmp",4,&operandTable[984]},
    {aarch64_op_fminnmp_advsimd_vec,"fminnmp",6,&operandTable[988]},
    {aarch64_op_fminnmv_advsimd,"fminnmv",5,&operandTable[994]},
    {aarch64_op_fminp_advsimd_pair,"fminp",4,&operandTable[999]},
    {aarch64_op_fminp_advsimd_vec,"fminp",6,&operandTable[1003]},
    {aarch64_op_fminv_advsimd,"fminv",5,&operandTable[1009]},
    {aarch64_op_fmla_advsimd_elt,"fmla",8,&operandTable[1014]},
    {aarch64_op_fmla_advsimd_elt,"fmla",9,&operandTable[1022]},
    {aarch64_op_fmla_advsimd_vec,"fmla",6,&operandTable[1031]},
    {aarch64_op_fmls_advsimd_elt,"fmls",8,&operandTable[1037]},
    {aarch64_op_fmls_advsimd_elt,"fmls",9,&operandTable[1045]},
    {aarch64_op_fmls_advsimd_vec,"fmls",6,&operandTable[1054]},
    {aarch64_op_fmov_advsimd,"fmov",12,&operandTable[1060]},
    {aarch64_op_fmov_float,"fmov",4,&operandTable[1072]},
    {aarch64_op_fmov_float_gen,"fmov",7,&operandTable[1076]},
    {aarch64_op_fmov_float_imm,"fmov",4,&operandTable[1083]},
    {aarch64_op_fmsub_float,"fmsub",6,&operandTable[1087]},
    {aarch64_op_fmul_advsimd_elt,"fmul",8,&operandTable[1093]},
    {aarch64_op_fmul_advsimd_elt,"fmul",9,&operandTable[1101]},
    {aarch64_op_fmul_advsimd_vec,"fmul",6,&operandTable[1110]},
    {aarch64_op_fmul_float,"fmul",5,&operandTable[1116]},
    {aarch64_op_fmulx_advsimd_elt,"fmulx",8,&operandTable[1121]},
    {aarch64_op_fmulx_advsimd_elt,"fmulx",9,&operandTable[1129]},
    {aarch64_op_fmulx_advsimd_vec,"fmulx",5,&operandTable[1138]},
    {aarch64_op_fmulx_advsimd_vec,"fmulx",6,&operandTable[1143]},
    {aarch64_op_fneg_advsimd,"fneg",5,&operandTable[1149]},
    {aarch64_op_fneg_float,"fneg",4,&operandTable[1154]},
    {aarch64_op_fnmadd_float,"fnmadd",6,&operandTable[1158]},
    {aarch64_op_fnmsub_float,"fnmsub",6,&operandTable[1164]},
    {aarch64_op_fnmul_float,"fnmul",5,&operandTable[1170]},
    {aarch64_op_frecpe_advsimd,"frecpe",4,&operandTable[1175]},
    {aarch64_op_frecpe_advsimd,"frecpe",5,&operandTable[1179]},
    {aarch64_op_frecps_advsimd,"frecps",5,&operandTable[1184]},
    {aarch64_op_frecps_advsimd,"frecps",6,&operandTable[1189]},
    {aarch64_op_frecpx_advsimd,"frecpx",4,&operandTable[1195]},
    {aarch64_op_frinta_advsimd,"frinta",5,&operandTable[1199]},
    {aarch64_op_frinta_float,"frinta",4,&operandTable[1204]},
    {aarch64_op_frinti_advsimd,"frinti",5,&operandTable[1208]},
    {aarch64_op_frinti_float,"frinti",4,&operandTable[1213]},
    {aarch64_op_frintm_advsimd,"frintm",5,&operandTable[1217]},
    {aarch64_op_frintm_float,"frintm",4,&operandTable[1222]},
    {aarch64_op_frintn_advsimd,"frintn",5,&operandTable[1226]},
    {aarch64_op_frintn_float,"frintn",4,&operandTable[1231]},
    {aarch64_op_frintp_advsimd,"frintp",5,&operandTable[1235]},
    {aarch64_op_frintp_float,"frintp",4,&operandTable[1240]},
    {aarch64_op_frintx_advsimd,"frintx",5,&operandTable[1244]},
    {aarch64_op_frintx_float,"frintx",4,&operandTable[1249]},
    {aarch64_op_frintz_advsimd,"frintz",5,&operandTable[1253]},
    {aarch64_op_frintz_float,"frintz",4,&operandTable[1258]},
    {aarch64_op_frsqrte_advsimd,"frsqrte",4,&operandTable[1262]},
    {aarch64_op_frsqrte_advsimd,"frsqrte",5,&operandTable[1266]},
    {aarch64_op_frsqrts_advsimd,"frsqrts",5,&operandTable[1271]},
    {aarch64_op_frsqrts_advsimd,"frsqrts",6,&operandTable[1276]},
    {aarch64_op_fsqrt_advsimd,"fsqrt",5,&operandTable[1282]},
    {aarch64_op_fsqrt_float,"fsqrt",4,&operandTable[1287]},
    {aarch64_op_fsub_advsimd,"fsub",6,&operandTable[1291]},
    {aarch64_op_fsub_float,"fsub",5,&operandTable[1297]},
    {aarch64_op_hint,"hint",2,&operandTable[1302]},
    {aarch64_op_hlt,"hlt",1,&operandTable[1304]},
    {aarch64_op_hvc,"hvc",1,&operandTable[1305]},
    {aarch64_op_ic_sys,"ic",4,&operandTable[1306]},
    {aarch64_op_ins_advsimd_elt,"ins",5,&operandTable[1310]},
    {aarch64_op_ins_advsimd_gen,"ins",4,&operandTable[1315]},
    {aarch64_op_isb,"isb",1,&operandTable[1319]},
    {aarch64_op_ld1_advsimd_mult,"ld1",6,&operandTable[1320]},
    {aarch64_op_ld1_advsimd_mult,"ld1",7,&operandTable[1326]},
    {aarch64_op_ld1_advsimd_sngl,"ld1",7,&operandTable[1333]},
    {aarch64_op_ld1_advsimd_sngl,"ld1",8,&operandTable[1340]},
    {aarch64_op_ld1r_advsimd,"ld1r",5,&operandTable[1348]},
    {aarch64_op_ld1r_advsimd,"ld1r",6,&operandTable[1353]},
    {aarch64_op_ld2_advsimd_mult,"ld2",5,&operandTable[1359]},
    {aarch64_op_ld2_advsimd_mult,"ld2",6,&operandTable[1364]},
    {aarch64_op_ld2_advsimd_sngl,"ld2",7,&operandTable[1370]},
    {aarch64_op_ld2_advsimd_sngl,"ld2",8,&operandTable[1377]},
    {aarch64_op_ld2r_advsimd,"ld2r",5,&operandTable[1385]},
    {aarch64_op_ld2r_advsimd,"ld2r",6,&operandTable[1390]},
    {aarch64_op_ld3_advsimd_mult,"ld3",5,&operandTable[1396]},
    {aarch64_op_ld3_advsimd_mult,"ld3",6,&operandTable[1401]},
    {aarch64_op_ld3_advsimd_sngl,"ld3",7,&operandTable[1407]},
    {aarch64_op_ld3_advsimd_sngl,"ld3",8,&operandTable[1414]},
    {aarch64_op_ld3r_advsimd,"ld3r",5,&operandTable[1422]},
    {aarch64_op_ld3r_advsimd,"ld3r",6,&operandTable[1427]},
    {aarch64_op_ld4_advsimd_mult,"ld4",5,&operandTable[1433]},
    {aarch64_op_ld4_advsimd_mult,"ld4",6,&operandTable[1438]},
    {aarch64_op_ld4_advsimd_sngl,"ld4",7,&operandTable[1444]},
    {aarch64_op_ld4_advsimd_sngl,"ld4",8,&operandTable[1451]},
    {aarch64_op_ld4r_advsimd,"ld4r",5,&operandTable[1459]},
    {aarch64_op_ld4r_advsimd,"ld4r",6,&operandTable[1464]},
    {aarch64_op_ldar,"ldar",4,&operandTable[1470]},
    {aarch64_op_ldarb,"ldarb",3,&operandTable[1474]},
    {aarch64_op_ldarh,"ldarh",3,&operandTable[1477]},
    {aarch64_op_ldaxp,"ldaxp",5,&operandTable[1480]},
    {aarch64_op_ldaxr,"ldaxr",4,&operandTable[1485]},
    {aarch64_op_ldaxrb,"ldaxrb",3,&operandTable[1489]},
    {aarch64_op_ldaxrh,"ldaxrh",3,&operandTable[1492]},
    {aarch64_op_ldnp_fpsimd,"ldnp",7,&operandTable[1495]},
    {aarch64_op_ldnp_gen,"ldnp",6,&operandTable[1502]},
    {aarch64_op_ldp_fpsimd,"ldp",7,&operandTable[1508]},
    {aarch64_op_ldp_fpsimd,"ldp",7,&operandTable[1515]},
    {aarch64_op_ldp_fpsimd,"ldp",7,&operandTable[1522]},
    {aarch64_op_ldp_gen,"ldp",6,&operandTable[1529]},
    {aarch64_op_ldp_gen,"ldp",6,&operandTable[1535]},
    {aarch64_op_ldp_gen,"ldp",6,&operandTable[1541]},
    {aarch64_op_ldpsw,"ldpsw",5,&operandTable[1547]},
    {aarch64_op_ldpsw,"ldpsw",5,&operandTable[1552]},
    {aarch64_op_ldpsw,"ldpsw",5,&operandTable[1557]},
    {aarch64_op_ldr_imm_fpsimd,"ldr",7,&operandTable[1562]},
    {aarch64_op_ldr_imm_fpsimd,"ldr",7,&operandTable[1569]},
    {aarch64_op_ldr_imm_fpsimd,"ldr",7,&operandTable[1576]},
    {aarch64_op_ldr_imm_gen,"ldr",5,&operandTable[1583]},
    {aarch64_op_ldr_imm_gen,"ldr",5,&operandTable[1588]},
    {aarch64_op_ldr_imm_gen,"ldr",5,&operandTable[1593]},
    {aarch64_op_ldr_lit_fpsimd,"ldr",5,&operandTable[1598]},
    {aarch64_op_ldr_lit_gen,"ldr",4,&operandTable[1603]},
    {aarch64_op_ldr_reg_fpsimd,"ldr",9,&operandTable[1607]},
    {aarch64_op_ldr_reg_gen,"ldr",7,&operandTable[1616]},
    {aarch64_op_ldrb_imm,"ldrb",4,&operandTable[1623]},
    {aarch64_op_ldrb_imm,"ldrb",4,&operandTable[1627]},
    {aarch64_op_ldrb_imm,"ldrb",4,&operandTable[1631]},
    {aarch64_op_ldrb_reg,"ldrb",6,&operandTable[1635]},
    {aarch64_op_ldrh_imm,"ldrh",4,&operandTable[1641]},
    {aarch64_op_ldrh_imm,"ldrh",4,&operandTable[1645]},
    {aarch64_op_ldrh_imm,"ldrh",4,&operandTable[1649]},
    {aarch64_op_ldrh_reg,"ldrh",6,&operandTable[1653]},
    {aarch64_op_ldrsb_imm,"ldrsb",5,&operandTable[1659]},
    {aarch64_op_ldrsb_imm,"ldrsb",5,&operandTable[1664]},
    {aarch64_op_ldrsb_imm,"ldrsb",5,&operandTable[1669]},
    {aarch64_op_ldrsb_reg,"ldrsb",7,&operandTable[1674]},
    {aarch64_op_ldrsh_imm,"ldrsh",5,&operandTable[1681]},
    {aarch64_op_ldrsh_imm,"ldrsh",5,&operandTable[1686]},
    {aarch64_op_ldrsh_imm,"ldrsh",5,&operandTable[1691]},
    {aarch64_op_ldrsh_reg,"ldrsh",7,&operandTable[1696]},
    {aarch64_op_ldrsw_imm,"ldrsw",4,&operandTable[1703]},
    {aarch64_op_ldrsw_imm,"ldrsw",4,&operandTable[1707]},
    {aarch64_op_ldrsw_imm,"ldrsw",4,&operandTable[1711]},
    {aarch64_op_ldrsw_lit,"ldrsw",3,&operandTable[1715]},
    {aarch64_op_ldrsw_reg,"ldrsw",6,&operandTable[1718]},
    {aarch64_op_ldtr,"ldtr",5,&operandTable[1724]},
    {aarch64_op_ldtrb,"ldtrb",4,&operandTable[1729]},
    {aarch64_op_ldtrh,"ldtrh",4,&operandTable[1733]},
    {aarch64_op_ldtrsb,"ldtrsb",5,&operandTable[1737]},
    {aarch64_op_ldtrsh,"ldtrsh",5,&operandTable[1742]},
    {aarch64_op_ldtrsw,"ldtrsw",4,&operandTable[1747]},
    {aarch64_op_ldur_fpsimd,"ldur",7,&operandTable[1751]},
    {aarch64_op_ldur_gen,"ldur",5,&operandTable[1758]},
    {aarch64_op_ldurb,"ldurb",4,&operandTable[1763]},
    {aarch64_op_ldurh,"ldurh",4,&operandTable[1767]},
    {aarch64_op_ldursb,"ldursb",5,&operandTable[1771]},
    {aarch64_op_ldursh,"ldursh",5,&operandTable[1776]},
    {aarch64_op_ldursw,"ldursw",4,&operandTable[1781]},
    {aarch64_op_ldxp,"ldxp",5,&operandTable[1785]},
    {aarch64_op_ldxr,"ldxr",4,&operandTable[1790]},
    {aarch64_op_ldxrb,"ldxrb",3,&operandTable[1794]},
    {aarch64_op_ldxrh,"ldxrh",3,&operandTable[1797]},
    {aarch64_op_lsl_lslv,"lsl",4,&operandTable[1800]},
    {aarch64_op_lsl_ubfm,"lsl",6,&operandTable[1804]},
    {aarch64_op_lslv,"lslv",4,&operandTable[1810]},
    {aarch64_op_lsr_lsrv,"lsr",4,&operandTable[1814]},
    {aarch64_op_lsr_ubfm,"lsr",6,&operandTable[1818]},
    {aarch64_op_lsrv,"lsrv",4,&operandTable[1824]},
    {aarch64_op_madd,"madd",5,&operandTable[1828]},
    {aarch64_op_mla_advsimd_elt,"mla",9,&operandTable[1833]},
    {aarch64_op_mla_advsimd_vec,"mla",6,&operandTable[1842]},
    {aarch64_op_mls_advsimd_elt,"mls",9,&operandTable[1848]},
    {aarch64_op_mls_advsimd_vec,"mls",6,&operandTable[1857]},
    {aarch64_op_mneg_msub,"mneg",4,&operandTable[1863]},
    {aarch64_op_mov_add_addsub_imm,"mov",3,&operandTable[1867]},
    {aarch64_op_mov_dup_advsimd_elt,"mov",4,&operandTable[1870]},
    {aarch64_op_mov_ins_advsimd_elt,"mov",5,&operandTable[1874]},
    {aarch64_op_mov_ins_advsimd_gen,"mov",4,&operandTable[1879]},
    {aarch64_op_mov_movn,"mov",4,&operandTable[1883]},
    {aarch64_op_mov_movz,"mov",4,&operandTable[1887]},
    {aarch64_op_mov_orr_advsimd_reg,"mov",5,&operandTable[1891]},
    {aarch64_op_mov_orr_log_imm,"mov",5,&operandTable[1896]},
    {aarch64_op_mov_orr_log_shift,"mov",3,&operandTable[1901]},
    {aarch64_op_mov_umov_advsimd,"mov",5,&operandTable[1904]},
    {aarch64_op_movi_advsimd,"movi",13,&operandTable[1909]},
    {aarch64_op_movk,"movk",4,&operandTable[1922]},
    {aarch64_op_movn,"movn",4,&operandTable[1926]},
    {aarch64_op_movz,"movz",4,&operandTable[1930]},
    {aarch64_op_mrs,"mrs",6,&operandTable[1934]},
    {aarch64_op_msr_imm,"msr",3,&operandTable[1940]},
    {aarch64_op_msr_reg,"msr",6,&operandTable[1943]},
    {aarch64_op_msub,"msub",5,&operandTable[1949]},
    {aarch64_op_mul_advsimd_elt,"mul",9,&operandTable[1954]},
    {aarch64_op_mul_advsimd_vec,"mul",6,&operandTable[1963]},
    {aarch64_op_mul_madd,"mul",4,&operandTable[1969]},
    {aarch64_op_mvn_not_advsimd,"mvn",4,&operandTable[1973]},
    {aarch64_op_mvn_orn_log_shift,"mvn",5,&operandTable[1977]},
    {aarch64_op_mvni_advsimd,"mvni",12,&operandTable[1982]},
    {aarch64_op_neg_advsimd,"neg",4,&operandTable[1994]},
    {aarch64_op_neg_advsimd,"neg",5,&operandTable[1998]},
    {aarch64_op_neg_sub_addsub_shift,"neg",5,&operandTable[2003]},
    {aarch64_op_negs_subs_addsub_shift,"negs",6,&operandTable[2008]},
    {aarch64_op_ngc_sbc,"ngc",3,&operandTable[2014]},
    {aarch64_op_ngcs_sbcs,"ngcs",4,&operandTable[2017]},
    {aarch64_op_nop_hint,"nop",0,&operandTable[2021]},
    {aarch64_op_not_advsimd,"not",4,&operandTable[2021]},
    {aarch64_op_orn_advsimd,"orn",5,&operandTable[2025]},
    {aarch64_op_orn_log_shift,"orn",6,&operandTable[2030]},
    {aarch64_op_orr_advsimd_imm,"orr",12,&operandTable[2036]},
    {aarch64_op_orr_advsimd_reg,"orr",5,&operandTable[2048]},
    {aarch64_op_orr_log_imm,"orr",6,&operandTable[2053]},
    {aarch64_op_orr_log_shift,"orr",6,&operandTable[2059]},
    {aarch64_op_pmul_advsimd,"pmul",6,&operandTable[2065]},
    {aarch64_op_pmull_advsimd,"pmull",6,&operandTable[2071]},
    {aarch64_op_prfm_imm,"prfm",3,&operandTable[2077]},
    {aarch64_op_prfm_lit,"prfm",2,&operandTable[2080]},
    {aarch64_op_prfm_reg,"prfm",5,&operandTable[2082]},
    {aarch64_op_prfum,"prfum",3,&operandTable[2087]},
    {aarch64_op_raddhn_advsimd,"raddhn",6,&operandTable[2090]},
    {aarch64_op_rbit_advsimd,"rbit",4,&operandTable[2096]},
    {aarch64_op_rbit_int,"rbit",3,&operandTable[2100]},
    {aarch64_op_ret,"ret",1,&operandTable[2103]},
    {aarch64_op_rev,"rev",4,&operandTable[2104]},
    {aarch64_op_rev16_advsimd,"rev16",5,&operandTable[2108]},
    {aarch64_op_rev16_int,"rev16",3,&operandTable[2113]},
    {aarch64_op_rev32_advsimd,"rev32",5,&operandTable[2116]},
    {aarch64_op_rev32_int,"rev32",2,&operandTable[2121]},
    {aarch64_op_rev64_advsimd,"rev64",5,&operandTable[2123]},
    {aarch64_op_ror_extr,"ror",6,&operandTable[2128]},
    {aarch64_op_ror_rorv,"ror",4,&operandTable[2134]},
    {aarch64_op_rorv,"rorv",4,&operandTable[2138]},
    {aarch64_op_rshrn_advsimd,"rshrn",6,&operandTable[2142]},
    {aarch64_op_rsubhn_advsimd,"rsubhn",6,&operandTable[2148]},
    {aarch64_op_saba_advsimd,"saba",6,&operandTable[2154]},
    {aarch64_op_sabal_advsimd,"sabal",6,&operandTable[2160]},
    {aarch64_op_sabd_advsimd,"sabd",6,&operandTable[2166]},
    {aarch64_op_sabdl_advsimd,"sabdl",6,&operandTable[2172]},
    {aarch64_op_sadalp_advsimd,"sadalp",5,&operandTable[2178]},
    {aarch64_op_saddl_advsimd,"saddl",6,&operandTable[2183]},
    {aarch64_op_saddlp_advsimd,"saddlp",5,&operandTable[2189]},
    {aarch64_op_saddlv_advsimd,"saddlv",5,&operandTable[2194]},
    {aarch64_op_saddw_advsimd,"saddw",6,&operandTable[2199]},
    {aarch64_op_sbc,"sbc",4,&operandTable[2205]},
    {aarch64_op_sbcs,"sbcs",5,&operandTable[2209]},
    {aarch64_op_sbfiz_sbfm,"sbfiz",6,&operandTable[2214]},
    {aarch64_op_sbfm,"sbfm",6,&operandTable[2220]},
    {aarch64_op_sbfx_sbfm,"sbfx",6,&operandTable[2226]},
    {aarch64_op_scvtf_advsimd_fix,"scvtf",5,&operandTable[2232]},
    {aarch64_op_scvtf_advsimd_fix,"scvtf",6,&operandTable[2237]},
    {aarch64_op_scvtf_advsimd_int,"scvtf",4,&operandTable[2243]},
    {aarch64_op_scvtf_advsimd_int,"scvtf",5,&operandTable[2247]},
    {aarch64_op_scvtf_float_fix,"scvtf",6,&operandTable[2252]},
    {aarch64_op_scvtf_float_int,"scvtf",5,&operandTable[2258]},
    {aarch64_op_sdiv,"sdiv",4,&operandTable[2263]},
    {aarch64_op_sev_hint,"sev",0,&operandTable[2267]},
    {aarch64_op_sevl_hint,"sevl",0,&operandTable[2267]},
    {aarch64_op_sha1c_advsimd,"sha1c",4,&operandTable[2267]},
    {aarch64_op_sha1h_advsimd,"sha1h",3,&operandTable[2271]},
    {aarch64_op_sha1m_advsimd,"sha1m",4,&operandTable[2274]},
    {aarch64_op_sha1p_advsimd,"sha1p",4,&operandTable[2278]},
    {aarch64_op_sha1su0_advsimd,"sha1su0",4,&operandTable[2282]},
    {aarch64_op_sha1su1_advsimd,"sha1su1",3,&operandTable[2286]},
    {aarch64_op_sha256h2_advsimd,"sha256h2",4,&operandTable[2289]},
    {aarch64_op_sha256h_advsimd,"sha256h",4,&operandTable[2293]},
    {aarch64_op_sha256su0_advsimd,"sha256su0",3,&operandTable[2297]},
    {aarch64_op_sha256su1_advsimd,"sha256su1",4,&operandTable[2300]},
    {aarch64_op_shadd_advsimd,"shadd",6,&operandTable[2304]},
    {aarch64_op_shl_advsimd,"shl",5,&operandTable[2310]},
    {aarch64_op_shl_advsimd,"shl",6,&operandTable[2315]},
    {aarch64_op_shll_advsimd,"shll",5,&operandTable[2321]},
    {aarch64_op_shrn_advsimd,"shrn",6,&operandTable[2326]},
    {aarch64_op_shsub_advsimd,"shsub",6,&operandTable[2332]},
    {aarch64_op_sli_advsimd,"sli",5,&operandTable[2338]},
    {aarch64_op_sli_advsimd,"sli",6,&operandTable[2343]},
    {aarch64_op_smaddl,"smaddl",4,&operandTable[2349]},
    {aarch64_op_smax_advsimd,"smax",6,&operandTable[2353]},
    {aarch64_op_smaxp_advsimd,"smaxp",6,&operandTable[2359]},
    {aarch64_op_smaxv_advsimd,"smaxv",5,&operandTable[2365]},
    {aarch64_op_smc,"smc",1,&operandTable[2370]},
    {aarch64_op_smin_advsimd,"smin",6,&operandTable[2371]},
    {aarch64_op_sminp_advsimd,"sminp",6,&operandTable[2377]},
    {aarch64_op_sminv_advsimd,"sminv",5,&operandTable[2383]},
    {aarch64_op_smlal_advsimd_elt,"smlal",9,&operandTable[2388]},
    {aarch64_op_smlal_advsimd_vec,"smlal",6,&operandTable[2397]},
    {aarch64_op_smlsl_advsimd_elt,"smlsl",9,&operandTable[2403]},
    {aarch64_op_smlsl_advsimd_vec,"smlsl",6,&operandTable[2412]},
    {aarch64_op_smnegl_smsubl,"smnegl",3,&operandTable[2418]},
    {aarch64_op_smov_advsimd,"smov",5,&operandTable[2421]},
    {aarch64_op_smsubl,"smsubl",4,&operandTable[2426]},
    {aarch64_op_smulh,"smulh",3,&operandTable[2430]},
    {aarch64_op_smull_advsimd_elt,"smull",9,&operandTable[2433]},
    {aarch64_op_smull_advsimd_vec,"smull",6,&operandTable[2442]},
    {aarch64_op_smull_smaddl,"smull",3,&operandTable[2448]},
    {aarch64_op_sqabs_advsimd,"sqabs",4,&operandTable[2451]},
    {aarch64_op_sqabs_advsimd,"sqabs",5,&operandTable[2455]},
    {aarch64_op_sqadd_advsimd,"sqadd",5,&operandTable[2460]},
    {aarch64_op_sqadd_advsimd,"sqadd",6,&operandTable[2465]},
    {aarch64_op_sqdmlal_advsimd_elt,"sqdmlal",8,&operandTable[2471]},
    {aarch64_op_sqdmlal_advsimd_elt,"sqdmlal",9,&operandTable[2479]},
    {aarch64_op_sqdmlal_advsimd_vec,"sqdmlal",5,&operandTable[2488]},
    {aarch64_op_sqdmlal_advsimd_vec,"sqdmlal",6,&operandTable[2493]},
    {aarch64_op_sqdmlsl_advsimd_elt,"sqdmlsl",8,&operandTable[2499]},
    {aarch64_op_sqdmlsl_advsimd_elt,"sqdmlsl",9,&operandTable[2507]},
    {aarch64_op_sqdmlsl_advsimd_vec,"sqdmlsl",5,&operandTable[2516]},
    {aarch64_op_sqdmlsl_advsimd_vec,"sqdmlsl",6,&operandTable[2521]},
    {aarch64_op_sqdmulh_advsimd_elt,"sqdmulh",8,&operandTable[2527]},
    {aarch64_op_sqdmulh_advsimd_elt,"sqdmulh",9,&operandTable[2535]},
    {aarch64_op_sqdmulh_advsimd_vec,"sqdmulh",5,&operandTable[2544]},
    {aarch64_op_sqdmulh_advsimd_vec,"sqdmulh",6,&operandTable[2549]},
    {aarch64_op_sqdmull_advsimd_elt,"sqdmull",8,&operandTable[2555]},
    {aarch64_op_sqdmull_advsimd_elt,"sqdmull",9,&operandTable[2563]},
    {aarch64_op_sqdmull_advsimd_vec,"sqdmull",5,&operandTable[2572]},
    {aarch64_op_sqdmull_advsimd_vec,"sqdmull",6,&operandTable[2577]},
    {aarch64_op_sqneg_advsimd,"sqneg",4,&operandTable[2583]},
    {aarch64_op_sqneg_advsimd,"sqneg",5,&operandTable[2587]},
    {aarch64_op_sqrdmulh_advsimd_elt,"sqrdmulh",8,&operandTable[2592]},
    {aarch64_op_sqrdmulh_advsimd_elt,"sqrdmulh",9,&operandTable[2600]},
    {aarch64_op_sqrdmulh_advsimd_vec,"sqrdmulh",5,&operandTable[2609]},
    {aarch64_op_sqrdmulh_advsimd_vec,"sqrdmulh",6,&operandTable[2614]},
    {aarch64_op_sqrshl_advsimd,"sqrshl",5,&operandTable[2620]},
    {aarch64_op_sqrshl_advsimd,"sqrshl",6,&operandTable[2625]},
    {aarch64_op_sqrshrn_advsimd,"sqrshrn",5,&operandTable[2631]},
    {aarch64_op_sqrshrn_advsimd,"sqrshrn",6,&operandTable[2636]},
    {aarch64_op_sqrshrun_advsimd,"sqrshrun",5,&operandTable[2642]},
    {aarch64_op_sqrshrun_advsimd,"sqrshrun",6,&operandTable[2647]},
    {aarch64_op_sqshl_advsimd_imm,"sqshl",5,&operandTable[2653]},
    {aarch64_op_sqshl_advsimd_imm,"sqshl",6,&operandTable[2658]},
    {aarch64_op_sqshl_advsimd_reg,"sqshl",5,&operandTable[2664]},
    {aarch64_op_sqshl_advsimd_reg,"sqshl",6,&operandTable[2669]},
    {aarch64_op_sqshlu_advsimd,"sqshlu",5,&operandTable[2675]},
    {aarch64_op_sqshlu_advsimd,"sqshlu",6,&operandTable[2680]},
    {aarch64_op_sqshrn_advsimd,"sqshrn",5,&operandTable[2686]},
    {aarch64_op_sqshrn_advsimd,"sqshrn",6,&operandTable[2691]},
    {aarch64_op_sqshrun_advsimd,"sqshrun",5,&operandTable[2697]},
    {aarch64_op_sqshrun_advsimd,"sqshrun",6,&operandTable[2702]},
    {aarch64_op_sqsub_advsimd,"sqsub",5,&operandTable[2708]},
    {aarch64_op_sqsub_advsimd,"sqsub",6,&operandTable[2713]},
    {aarch64_op_sqxtn_advsimd,"sqxtn",4,&operandTable[2719]},
    {aarch64_op_sqxtn_advsimd,"sqxtn",5,&operandTable[2723]},
    {aarch64_op_sqxtun_advsimd,"sqxtun",4,&operandTable[2728]},
    {aarch64_op_sqxtun_advsimd,"sqxtun",5,&operandTable[2732]},
    {aarch64_op_srhadd_advsimd,"srhadd",6,&operandTable[2737]},
    {aarch64_op_sri_advsimd,"sri",5,&operandTable[2743]},
    {aarch64_op_sri_advsimd,"sri",6,&operandTable[2748]},
    {aarch64_op_srshl_advsimd,"srshl",5,&operandTable[2754]},
    {aarch64_op_srshl_advsimd,"srshl",6,&operandTable[2759]},
    {aarch64_op_srshr_advsimd,"srshr",5,&operandTable[2765]},
    {aarch64_op_srshr_advsimd,"srshr",6,&operandTable[2770]},
    {aarch64_op_srsra_advsimd,"srsra",5,&operandTable[2776]},
    {aarch64_op_srsra_advsimd,"srsra",6,&operandTable[2781]},
    {aarch64_op_sshl_advsimd,"sshl",5,&operandTable[2787]},
    {aarch64_op_sshl_advsimd,"sshl",6,&operandTable[2792]},
    {aarch64_op_sshll_advsimd,"sshll",6,&operandTable[2798]},
    {aarch64_op_sshr_advsimd,"sshr",5,&operandTable[2804]},
    {aarch64_op_sshr_advsimd,"sshr",6,&operandTable[2809]},
    {aarch64_op_ssra_advsimd,"ssra",5,&operandTable[2815]},
    {aarch64_op_ssra_advsimd,"ssra",6,&operandTable[2820]},
    {aarch64_op_ssubl_advsimd,"ssubl",6,&operandTable[2826]},
    {aarch64_op_ssubw_advsimd,"ssubw",6,&operandTable[2832]},
    {aarch64_op_st1_advsimd_mult,"st1",6,&operandTable[2838]},
    {aarch64_op_st1_advsimd_mult,"st1",7,&operandTable[2844]},
    {aarch64_op_st1_advsimd_sngl,"st1",7,&operandTable[2851]},
    {aarch64_op_st1_advsimd_sngl,"st1",8,&operandTable[2858]},
    {aarch64_op_st2_advsimd_mult,"st2",5,&operandTable[2866]},
    {aarch64_op_st2_advsimd_mult,"st2",6,&operandTable[2871]},
    {aarch64_op_st2_advsimd_sngl,"st2",7,&operandTable[2877]},
    {aarch64_op_st2_advsimd_sngl,"st2",8,&operandTable[2884]},
    {aarch64_op_st3_advsimd_mult,"st3",5,&operandTable[2892]},
    {aarch64_op_st3_advsimd_mult,"st3",6,&operandTable[2897]},
    {aarch64_op_st3_advsimd_sngl,"st3",7,&operandTable[2903]},
    {aarch64_op_st3_advsimd_sngl,"st3",8,&operandTable[2910]},
    {aarch64_op_st4_advsimd_mult,"st4",5,&operandTable[2918]},
    {aarch64_op_st4_advsimd_mult,"st4",6,&operandTable[2923]},
    {aarch64_op_st4_advsimd_sngl,"st4",7,&operandTable[2929]},
    {aarch64_op_st4_advsimd_sngl,"st4",8,&operandTable[2936]},
    {aarch64_op_stlr,"stlr",4,&operandTable[2944]},
    {aarch64_op_stlrb,"stlrb",3,&operandTable[2948]},
    {aarch64_op_stlrh,"stlrh",3,&operandTable[2951]},
    {aarch64_op_stlxp,"stlxp",6,&operandTable[2954]},
    {aarch64_op_stlxr,"stlxr",5,&operandTable[2960]},
    {aarch64_op_stlxrb,"stlxrb",4,&operandTable[2965]},
    {aarch64_op_stlxrh,"stlxrh",4,&operandTable[2969]},
    {aarch64_op_stnp_fpsimd,"stnp",7,&operandTable[2973]},
    {aarch64_op_stnp_gen,"stnp",6,&operandTable[2980]},
    {aarch64_op_stp_fpsimd,"stp",7,&operandTable[2986]},
    {aarch64_op_stp_fpsimd,"stp",7,&operandTable[2993]},
    {aarch64_op_stp_fpsimd,"stp",7,&operandTable[3000]},
    {aarch64_op_stp_gen,"stp",6,&operandTable[3007]},
    {aarch64_op_stp_gen,"stp",6,&operandTable[3013]},
    {aarch64_op_stp_gen,"stp",6,&operandTable[3019]},
    {aarch64_op_str_imm_fpsimd,"str",7,&operandTable[3025]},
    {aarch64_op_str_imm_fpsimd,"str",7,&operandTable[3032]},
    {aarch64_op_str_imm_fpsimd,"str",7,&operandTable[3039]},
    {aarch64_op_str_imm_gen,"str",5,&operandTable[3046]},
    {aarch64_op_str_imm_gen,"str",5,&operandTable[3051]},
    {aarch64_op_str_imm_gen,"str",5,&operandTable[3056]},
    {aarch64_op_str_reg_fpsimd,"str",9,&operandTable[3061]},
    {aarch64_op_str_reg_gen,"str",7,&operandTable[3070]},
    {aarch64_op_strb_imm,"strb",4,&operandTable[3077]},
    {aarch64_op_strb_imm,"strb",4,&operandTable[3081]},
    {aarch64_op_strb_imm,"strb",4,&operandTable[3085]},
    {aarch64_op_strb_reg,"strb",6,&operandTable[3089]},
    {aarch64_op_strh_imm,"strh",4,&operandTable[3095]},
    {aarch64_op_strh_imm,"strh",4,&operandTable[3099]},
    {aarch64_op_strh_imm,"strh",4,&operandTable[3103]},
    {aarch64_op_strh_reg,"strh",6,&operandTable[3107]},
    {aarch64_op_sttr,"sttr",5,&operandTable[3113]},
    {aarch64_op_sttrb,"sttrb",4,&operandTable[3118]},
    {aarch64_op_sttrh,"sttrh",4,&operandTable[3122]},
    {aarch64_op_stur_fpsimd,"stur",7,&operandTable[3126]},
    {aarch64_op_stur_gen,"stur",5,&operandTable[3133]},
    {aarch64_op_sturb,"sturb",4,&operandTable[3138]},
    {aarch64_op_sturh,"sturh",4,&operandTable[3142]},
    {aarch64_op_stxp,"stxp",6,&operandTable[3146]},
    {aarch64_op_stxr,"stxr",5,&operandTable[3152]},
    {aarch64_op_stxrb,"stxrb",4,&operandTable[3157]},
    {aarch64_op_stxrh,"stxrh",4,&operandTable[3161]},
    {aarch64_op_sub_addsub_ext,"sub",6,&operandTable[3165]},
    {aarch64_op_sub_addsub_imm,"sub",5,&operandTable[3171]},
    {aarch64_op_sub_addsub_shift,"sub",6,&operandTable[3176]},
    {aarch64_op_sub_advsimd,"sub",5,&operandTable[3182]},
    {aarch64_op_sub_advsimd,"sub",6,&operandTable[3187]},
    {aarch64_op_subhn_advsimd,"subhn",6,&operandTable[3193]},
    {aarch64_op_subs_addsub_ext,"subs",7,&operandTable[3199]},
    {aarch64_op_subs_addsub_imm,"subs",6,&operandTable[3206]},
    {aarch64_op_subs_addsub_shift,"subs",7,&operandTable[3212]},
    {aarch64_op_suqadd_advsimd,"suqadd",4,&operandTable[3219]},
    {aarch64_op_suqadd_advsimd,"suqadd",5,&operandTable[3223]},
    {aarch64_op_svc,"svc",1,&operandTable[3228]},
    {aarch64_op_sxtb_sbfm,"sxtb",4,&operandTable[3229]},
    {aarch64_op_sxth_sbfm,"sxth",4,&operandTable[3233]},
    {aarch64_op_sxtl_sshll_advsimd,"sxtl",5,&operandTable[3237]},
    {aarch64_op_sxtw_sbfm,"sxtw",2,&operandTable[3242]},
    {aarch64_op_sys,"sys",5,&operandTable[3244]},
    {aarch64_op_sysl,"sysl",5,&operandTable[3249]},
    {aarch64_op_tbl_advsimd,"tbl",6,&operandTable[3254]},
    {aarch64_op_tbnz,"tbnz",4,&operandTable[3260]},
    {aarch64_op_tbx_advsimd,"tbx",6,&operandTable[3264]},
    {aarch64_op_tbz,"tbz",4,&operandTable[3270]},
    {aarch64_op_tlbi_sys,"tlbi",4,&operandTable[3274]},
    {aarch64_op_trn1_advsimd,"trn1",6,&operandTable[3278]},
    {aarch64_op_trn2_advsimd,"trn2",6,&operandTable[3284]},
    {aarch64_op_tst_ands_log_imm,"tst",6,&operandTable[3290]},
    {aarch64_op_tst_ands_log_shift,"tst",6,&operandTable[3296]},
    {aarch64_op_uaba_advsimd,"uaba",6,&operandTable[3302]},
    {aarch64_op_uabal_advsimd,"uabal",6,&operandTable[3308]},
    {aarch64_op_uabd_advsimd,"uabd",6,&operandTable[3314]},
    {aarch64_op_uabdl_advsimd,"uabdl",6,&operandTable[3320]},
    {aarch64_op_uadalp_advsimd,"uadalp",5,&operandTable[3326]},
    {aarch64_op_uaddl_advsimd,"uaddl",6,&operandTable[3331]},
    {aarch64_op_uaddlp_advsimd,"uaddlp",5,&operandTable[3337]},
    {aarch64_op_uaddlv_advsimd,"uaddlv",5,&operandTable[3342]},
    {aarch64_op_uaddw_advsimd,"uaddw",6,&operandTable[3347]},
    {aarch64_op_ubfiz_ubfm,"ubfiz",6,&operandTable[3353]},
    {aarch64_op_ubfm,"ubfm",6,&operandTable[3359]},
    {aarch64_op_ubfx_ubfm,"ubfx",6,&operandTable[3365]},
    {aarch64_op_ucvtf_advsimd_fix,"ucvtf",5,&operandTable[3371]},
    {aarch64_op_ucvtf_advsimd_fix,"ucvtf",6,&operandTable[3376]},
    {aarch64_op_ucvtf_advsimd_int,"ucvtf",4,&operandTable[3382]},
    {aarch64_op_ucvtf_advsimd_int,"ucvtf",5,&operandTable[3386]},
    {aarch64_op_ucvtf_float_fix,"ucvtf",6,&operandTable[3391]},
    {aarch64_op_ucvtf_float_int,"ucvtf",5,&operandTable[3397]},
    {aarch64_op_udiv,"udiv",4,&operandTable[3402]},
    {aarch64_op_uhadd_advsimd,"uhadd",6,&operandTable[3406]},
    {aarch64_op_uhsub_advsimd,"uhsub",6,&operandTable[3412]},
    {aarch64_op_umaddl,"umaddl",4,&operandTable[3418]},
    {aarch64_op_umax_advsimd,"umax",6,&operandTable[3422]},
    {aarch64_op_umaxp_advsimd,"umaxp",6,&operandTable[3428]},
    {aarch64_op_umaxv_advsimd,"umaxv",5,&operandTable[3434]},
    {aarch64_op_umin_advsimd,"umin",6,&operandTable[3439]},
    {aarch64_op_uminp_advsimd,"uminp",6,&operandTable[3445]},
    {aarch64_op_uminv_advsimd,"uminv",5,&operandTable[3451]},
    {aarch64_op_umlal_advsimd_elt,"umlal",9,&operandTable[3456]},
    {aarch64_op_umlal_advsimd_vec,"umlal",6,&operandTable[3465]},
    {aarch64_op_umlsl_advsimd_elt,"umlsl",9,&operandTable[3471]},
    {aarch64_op_umlsl_advsimd_vec,"umlsl",6,&operandTable[3480]},
    {aarch64_op_umnegl_umsubl,"umnegl",3,&operandTable[3486]},
    {aarch64_op_umov_advsimd,"umov",5,&operandTable[3489]},
    {aarch64_op_umsubl,"umsubl",4,&operandTable[3494]},
    {aarch64_op_umulh,"umulh",3,&operandTable[3498]},
    {aarch64_op_umull_advsimd_elt,"umull",9,&operandTable[3501]},
    {aarch64_op_umull_advsimd_vec,"umull",6,&operandTable[3510]},
    {aarch64_op_umull_umaddl,"umull",3,&operandTable[3516]},
    {aarch64_op_uqadd_advsimd,"uqadd",5,&operandTable[3519]},
    {aarch64_op_uqadd_advsimd,"uqadd",6,&operandTable[3524]},
    {aarch64_op_uqrshl_advsimd,"uqrshl",5,&operandTable[3530]},
    {aarch64_op_uqrshl_advsimd,"uqrshl",6,&operandTable[3535]},
    {aarch64_op_uqrshrn_advsimd,"uqrshrn",5,&operandTable[3541]},
    {aarch64_op_uqrshrn_advsimd,"uqrshrn",6,&operandTable[3546]},
    {aarch64_op_uqshl_advsimd_imm,"uqshl",5,&operandTable[3552]},
    {aarch64_op_uqshl_advsimd_imm,"uqshl",6,&operandTable[3557]},
    {aarch64_op_uqshl_advsimd_reg,"uqshl",5,&operandTable[3563]},
    {aarch64_op_uqshl_advsimd_reg,"uqshl",6,&operandTable[3568]},
    {aarch64_op_uqshrn_advsimd,"uqshrn",5,&operandTable[3574]},
    {aarch64_op_uqshrn_advsimd,"uqshrn",6,&operandTable[3579]},
    {aarch64_op_uqsub_advsimd,"uqsub",5,&operandTable[3585]},
    {aarch64_op_uqsub_advsimd,"uqsub",6,&operandTable[3590]},
    {aarch64_op_uqxtn_advsimd,"uqxtn",4,&operandTable[3596]},
    {aarch64_op_uqxtn_advsimd,"uqxtn",5,&operandTable[3600]},
    {aarch64_op_urecpe_advsimd,"urecpe",5,&operandTable[3605]},
    {aarch64_op_urhadd_advsimd,"urhadd",6,&operandTable[3610]},
    {aarch64_op_urshl_advsimd,"urshl",5,&operandTable[3616]},
    {aarch64_op_urshl_advsimd,"urshl",6,&operandTable[3621]},
    {aarch64_op_urshr_advsimd,"urshr",5,&operandTable[3627]},
    {aarch64_op_urshr_advsimd,"urshr",6,&operandTable[3632]},
    {aarch64_op_ursqrte_advsimd,"ursqrte",5,&operandTable[3638]},
    {aarch64_op_ursra_advsimd,"ursra",5,&operandTable[3643]},
    {aarch64_op_ursra_advsimd,"ursra",6,&operandTable[3648]},
    {aarch64_op_ushl_advsimd,"ushl",5,&operandTable[3654]},
    {aarch64_op_ushl_advsimd,"ushl",6,&operandTable[3659]},
    {aarch64_op_ushll_advsimd,"ushll",6,&operandTable[3665]},
    {aarch64_op_ushr_advsimd,"ushr",5,&operandTable[3671]},
    {aarch64_op_ushr_advsimd,"ushr",6,&operandTable[3676]},
    {aarch64_op_usqadd_advsimd,"usqadd",4,&operandTable[3682]},
    {aarch64_op_usqadd_advsimd,"usqadd",5,&operandTable[3686]},
    {aarch64_op_usra_advsimd,"usra",5,&operandTable[3691]},
    {aarch64_op_usra_advsimd,"usra",6,&operandTable[3696]},
    {aarch64_op_usubl_advsimd,"usubl",6,&operandTable[3702]},
    {aarch64_op_usubw_advsimd,"usubw",6,&operandTable[3708]},
    {aarch64_op_uxtb_ubfm,"uxtb",2,&operandTable[3714]},
    {aarch64_op_uxth_ubfm,"uxth",2,&operandTable[3716]},
    {aarch64_op_uxtl_ushll_advsimd,"uxtl",5,&operandTable[3718]},
    {aarch64_op_uzp1_advsimd,"uzp1",6,&operandTable[3723]},
    {aarch64_op_uzp2_advsimd,"uzp2",6,&operandTable[3729]},
    {aarch64_op_wfe_hint,"wfe",0,&operandTable[3735]},
    {aarch64_op_wfi_hint,"wfi",0,&operandTable[3735]},
    {aarch64_op_xtn_advsimd,"xtn",5,&operandTable[3735]},
    {aarch64_op_yield_hint,"yield",0,&operandTable[3740]},
    {aarch64_op_zip1_advsimd,"zip1",6,&operandTable[3740]},
    {aarch64_op_zip2_advsimd,"zip2",6,&operandTable[3746]},
};  // end main_insn_table

const std::pair<unsigned int,unsigned int> aarch64_mask_entry::branchTable[] = {
    {0,1},{1,2},{2,3},{3,4},
    {0,5},{1,6},{2,7},{3,8},{4,9},{5,10},{6,11},{7,12},
    {0,430},{1,431},
    {0,494},{2,495},{3,496},{4,497},{6,498},{7,499},{8,500},{9,501},{10,502},{12,503},{13,504},{14,505},{15,506},
    {0,13},{1,14},{2,15},{3,16},{4,17},{5,18},{6,19},{7,20},
    {4,51},{5,52},{6,53},{7,54},{13,55},{15,56},
    {0,57},{1,58},{2,59},{3,60},{4,61},{5,62},{6,63},{7,64},
    {0,65},{1,66},{2,67},{3,68},{4,69},{5,70},{6,71},{7,72},
    {0,75},{1,76},{2,77},{3,78},{4,79},{5,80},{6,81},{7,82},
    {0,103},{1,104},{2,105},{3,106},{4,107},{5,108},{6,109},{7,110},
    {0,127},{1,128},{4,129},{5,130},{6,131},{7,132},{8,133},{9,134},{12,135},{13,136},{14,137},{15,138},
    {0,379},{1,380},
    {0,21},{1,22},{4,23},{5,24},{6,25},{7,26},
    {62,31},{63,32},{190,33},{191,34},{254,35},{255,36},
    {2047,41},{6143,42},
    {2047,45},{6143,46},
    {0,49},{1,50},
    {31,27},{63,28},
    {31,29},{63,30},
    {31,37},{63,38},
    {31,39},{63,40},
    {0,43},{1,44},
    {0,47},{1,48},
    {0,73},
    {0,74},
    {0,83},{1,84},
    {0,88},{1,89},
    {0,93},{1,94},
    {0,98},{1,99},
    {0,85},{2,86},{4,87},
    {0,90},{2,91},{4,92},
    {0,95},{2,96},{4,97},
    {0,100},{2,101},{4,102},
    {0,111},{1,112},{64,113},{65,114},
    {0,115},{1,116},{64,117},{65,118},
    {0,119},{1,120},{2,121},{3,122},
    {0,123},{1,124},{2,125},{3,126},
    {0,860},{1,860},{2,860},{3,861},
    {0,868},{1,868},{2,868},{3,869},
    {0,864},{1,864},{2,864},{3,865},
    {0,872},{1,872},{2,872},{3,873},
    {0,862},{1,862},{2,862},{3,863},
    {0,870},{1,870},{2,870},{3,871},
    {0,866},{1,866},{2,866},{3,867},
    {0,874},{1,874},{2,874},{3,875},
    {0,139},{1,140},{2,141},{3,142},
    {0,149},{1,150},{3,151},{5,152},{7,153},
    {0,155},{1,156},{2,157},{3,158},{4,159},{5,160},{6,161},{7,162},{8,163},{9,164},{10,165},{11,166},{12,167},{13,168},{14,169},{15,170},
    {0,186},{1,187},{2,188},{3,189},{4,190},{5,191},{6,192},{7,193},{8,194},{9,195},{10,196},{11,197},{12,198},{13,199},{14,200},{15,201},
    {0,206},{1,207},{2,208},{3,209},{4,210},{5,211},{6,212},{7,213},{8,214},{9,215},{10,216},{11,217},{12,218},{13,219},{15,220},
    {0,246},{1,247},{2,248},{3,249},{4,250},{5,251},{6,252},{7,253},{8,254},{9,255},{10,256},{11,257},{12,258},{14,259},{15,260},
    {4,271},
    {0,272},{1,273},{2,274},{4,275},{5,276},{6,277},{7,278},{8,279},{9,280},{10,281},{11,282},{12,283},{13,284},{14,285},{15,286},
    {0,298},{1,299},{2,300},{3,301},{4,302},{5,303},{6,304},{7,305},{8,306},{9,307},{10,308},{11,309},{12,310},{13,311},{14,312},{15,313},
    {0,318},{1,319},{3,320},{4,321},{5,322},{7,323},{8,324},{9,325},{11,326},{15,327},
    {0,355},{1,356},{2,357},{3,358},{4,359},{5,360},{6,361},{8,362},{10,363},{11,364},{12,365},{13,366},{14,367},{15,368},
    {1,143},{3,144},
    {0,145},{1,146},{2,147},{3,148},
    {1,154},
    {0,171},{1,172},
    {0,173},{16,174},
    {0,175},{1,176},{8,177},
    {0,178},{8,179},
    {0,180},{1,181},{8,182},
    {0,183},{1,184},{8,185},
    {0,202},{1,203},{2,204},{3,205},
    {0,221},{1,222},
    {0,225},{1,226},
    {0,229},{1,230},{16,231},{17,232},
    {0,235},{1,236},{17,237},
    {1,240},{32,241},{33,242},
    {1,243},{32,244},{33,245},
    {0,223},{1,224},
    {0,227},{1,228},
    {0,233},{1,234},
    {0,238},{1,239},
    {0,261},{1,262},
    {0,263},{1,264},
    {0,265},{1,266},
    {0,267},{1,268},
    {0,269},{1,270},
    {0,287},{1,288},
    {0,289},{1,290},{16,291},
    {0,292},{1,293},
    {0,294},{32,295},
    {0,296},{1,297},
    {0,314},{1,315},{2,316},{3,317},
    {0,328},{1,329},
    {0,330},{1,331},
    {1,334},{16,335},{17,336},
    {0,339},{1,340},
    {1,343},{16,344},{32,345},{33,346},{48,347},
    {1,348},{32,349},{33,350},
    {16,351},{32,352},{33,353},{48,354},
    {0,332},{1,333},
    {0,337},{1,338},
    {0,341},{1,342},
    {0,369},{1,370},
    {0,371},{1,372},
    {0,373},{1,374},
    {0,375},{1,376},
    {0,377},{1,378},
    {1,381},{2,382},{3,383},{5,384},{6,385},{7,386},{8,387},{9,388},{10,389},{11,390},{12,391},{13,392},{16,393},{18,394},{20,395},{22,396},{25,397},{26,398},
    {0,399},{2,400},{4,400},{6,400},{8,400},{10,400},{12,400},{14,400},{16,400},{18,400},{20,400},{22,400},{24,400},{26,400},{28,400},{30,400},{3,401},{5,401},{7,401},{9,401},{11,401},{13,401},{15,401},{17,401},{19,401},{21,401},{23,401},{25,401},{27,401},{29,401},{31,401},
    {0,853},{2,853},{4,853},{6,853},{8,853},{10,853},{12,853},{13,853},{14,855},{1,854},{3,854},{5,854},{7,854},{9,854},{11,854},{15,856},
    {0,402},{1,403},{2,404},{3,405},{5,406},{7,407},{8,408},{9,409},{10,410},{14,411},{16,412},{17,413},{18,414},{19,415},{20,416},{21,417},{22,418},{23,419},{24,420},{25,421},{26,422},{30,423},
    {8,424},{9,425},{15,426},{24,427},{25,428},{31,429},
    {0,432},{1,433},{2,434},{3,435},
    {0,454},{1,455},{2,456},
    {0,436},{1,437},
    {0,438},{1,439},{2,440},{3,441},
    {0,442},{1,443},{2,444},{4,445},{5,446},{6,447},{7,448},
    {0,449},{1,450},{2,451},{4,452},
    {0,453},
    {0,457},{1,458},
    {0,459},{1,460},{2,461},{3,462},
    {0,463},{4,464},{5,465},{6,466},
    {1,467},{2,468},{3,469},{32,470},{64,471},{161,472},{162,473},{163,474},
    {0,475},{1,476},{2,477},{3,478},
    {63488,489},{129024,490},{194560,491},{325632,492},{391168,493},
    {0,479},{1,480},
    {95,481},{127,482},{159,483},
    {3,484},
    {26,485},{28,486},{29,487},{30,488},
    {0,507},{1,508},
    {0,511},{4,512},{6,513},{8,514},{12,515},{14,516},
    {0,539},{1,540},{2,541},{3,542},{4,543},{10,544},{11,545},{12,546},
    {0,551},{1,552},{2,553},{3,554},
    {0,668},{1,669},{2,670},{3,671},
    {0,695},{1,696},{2,697},{3,698},{6,699},{8,700},{9,701},{10,702},{11,703},{14,704},{16,705},{17,706},{18,707},{19,708},{22,709},{24,710},{25,711},{26,712},{27,713},{30,714},
    {0,759},{1,760},{2,761},{3,762},
    {0,773},{8,774},{10,775},{32,776},{40,777},{42,778},
    {0,780},{1,781},{3,782},{6,783},{8,784},{9,785},{11,786},{14,787},
    {0,788},{1,789},
    {195,790},{202,791},{203,792},{205,793},{206,794},{207,795},{209,796},{210,797},{211,798},{213,799},{215,800},{218,801},{222,802},{225,803},{226,804},{227,805},{230,806},{234,807},{237,808},{238,809},{242,810},{245,811},{246,812},{249,813},{251,814},{254,815},
    {65,836},{67,837},{69,838},{71,839},{73,840},{75,841},{77,842},{79,843},{81,844},{83,845},{93,846},{95,847},{114,848},
    {0,509},{1,510},
    {0,517},{1,518},
    {0,519},{2,520},{4,521},{5,522},
    {0,529},
    {0,530},{1,531},
    {0,532},{1,533},{2,534},
    {2,523},{3,524},
    {0,525},{1,526},{2,527},{3,528},
    {0,535},{1,536},
    {0,537},{1,538},
    {1,547},
    {1,548},
    {1,549},
    {1,550},
    {2,555},{3,556},{24,557},{25,558},
    {0,559},{1,560},{2,561},{3,562},
    {0,613},{1,614},{4,615},{8,616},{12,617},{16,618},{20,619},{24,620},
    {2,621},{3,622},{6,623},{10,624},{11,625},{13,626},{14,627},{15,628},{17,629},{18,630},{19,631},{21,632},{23,633},{30,634},{33,635},{34,636},{35,637},{36,638},{38,639},{42,640},{44,641},{45,642},{46,643},{50,644},{52,645},{54,646},{55,647},{57,648},{58,649},{62,650},{63,651},
    {0,563},{1,564},
    {0,602},{1,603},
    {0,604},{1,605},{2,606},{3,607},{4,608},{5,609},{6,610},{7,611},{8,612},
    {0,565},{1,566},{2,567},
    {0,568},{2,569},{4,570},{6,571},{8,572},
    {0,585},{8,586},
    {0,587},{1,588},{2,589},{3,590},
    {0,573},{1,574},{2,575},{3,576},
    {0,577},{1,578},
    {0,579},{1,580},
    {0,581},{1,582},{2,583},{3,584},
    {0,591},{1,592},{2,593},{3,594},
    {0,595},{1,596},{2,597},{3,598},
    {0,599},{2,600},{3,601},
    {0,652},{1,653},
    {0,656},{1,657},{17,658},
    {1,661},{32,662},
    {1,663},{32,664},{33,665},
    {0,666},{1,667},
    {0,654},{1,655},
    {0,659},{1,660},
    {0,672},{1,673},
    {0,674},{1,675},
    {1,676},{2,677},{3,678},{5,679},{6,680},{7,681},{10,682},{11,683},{14,684},{15,685},
    {2,686},{3,687},{6,688},{8,689},{10,690},{13,691},{15,692},
    {0,693},{1,694},
    {0,715},{1,716},{2,717},{3,718},
    {0,719},{1,720},{2,721},{3,722},
    {0,723},{1,724},{2,725},{3,726},
    {0,727},{1,728},{2,729},{3,730},
    {0,731},{1,732},{2,733},{3,734},
    {0,735},{1,736},
    {0,737},{1,738},
    {0,739},{1,740},
    {0,741},{1,742},
    {0,743},{1,744},
    {0,745},{1,746},
    {0,747},{1,748},
    {0,749},{1,750},
    {0,751},{1,752},
    {0,753},{1,754},
    {0,755},{2,756},
    {0,757},{2,758},
    {0,763},{1,764},{2,765},{3,766},
    {0,767},{1,768},
    {0,769},{1,770},
    {0,771},{2,772},
    {0,779},
    {1,816},{33,817},
    {0,818},{1,819},
    {1,822},{16,823},{32,824},{48,825},
    {1,826},{16,827},{32,828},{33,829},
    {0,830},{1,831},
    {0,832},{1,833},
    {16,834},{48,835},
    {0,820},{1,821},
    {0,849},{1,850},
    {0,851},{1,852},
    {0,855},{1,859},
    {0,857},{1,858},
};  // end branchTable

const aarch64_decoder_table aarch64_mask_entry::main_decoder_table = {
	{0x18000000, 4,&branchTable[0], -1},
	{0x0, 0,&branchTable[4], 0},
	{0x7000000, 8,&branchTable[4], -1},
	{0x4000000, 2,&branchTable[12], -1},
	{0x27000000, 13,&branchTable[14], -1},
	{0x20c00000, 8,&branchTable[27], -1},
	{0x60c00000, 6,&branchTable[35], -1},
	{0x60200000, 8,&branchTable[41], -1},
	{0x60200000, 8,&branchTable[49], -1},
	{0x20c00000, 8,&branchTable[57], -1},
	{0x20c00000, 8,&branchTable[65], -1},
	{0xa0208400, 12,&branchTable[73], -1},
	{0x80000400, 2,&branchTable[85], -1},
	{0x80208000, 6,&branchTable[87], -1},
	{0x803f8000, 6,&branchTable[93], -1},
	{0x803ffc00, 2,&branchTable[99], -1},
	{0x803ffc00, 2,&branchTable[101], -1},
	{0x0, 0,&branchTable[103], 582},
	{0x0, 0,&branchTable[103], 303},
	{0x0, 0,&branchTable[103], 586},
	{0x40000000, 2,&branchTable[103], -1},
	{0x40007c00, 2,&branchTable[105], -1},
	{0x40007c00, 2,&branchTable[107], -1},
	{0x0, 0,&branchTable[109], 613},
	{0x0, 0,&branchTable[109], 578},
	{0x0, 0,&branchTable[109], 612},
	{0x0, 0,&branchTable[109], 577},
	{0x0, 0,&branchTable[109], 614},
	{0x0, 0,&branchTable[109], 615},
	{0x0, 0,&branchTable[109], 579},
	{0x0, 0,&branchTable[109], 580},
	{0x40007c00, 2,&branchTable[109], -1},
	{0x40007c00, 2,&branchTable[111], -1},
	{0x0, 0,&branchTable[113], 358},
	{0x0, 0,&branchTable[113], 299},
	{0x0, 0,&branchTable[113], 357},
	{0x0, 0,&branchTable[113], 298},
	{0x0, 0,&branchTable[113], 359},
	{0x0, 0,&branchTable[113], 360},
	{0x0, 0,&branchTable[113], 300},
	{0x0, 0,&branchTable[113], 301},
	{0x40000000, 2,&branchTable[113], -1},
	{0x0, 0,&branchTable[115], 574},
	{0x0, 0,&branchTable[115], 575},
	{0x0, 0,&branchTable[115], 576},
	{0x40000000, 2,&branchTable[115], -1},
	{0x0, 0,&branchTable[117], 295},
	{0x0, 0,&branchTable[117], 296},
	{0x0, 0,&branchTable[117], 297},
	{0x0, 0,&branchTable[117], 307},
	{0x0, 0,&branchTable[117], 310},
	{0x0, 0,&branchTable[117], 588},
	{0x0, 0,&branchTable[117], 309},
	{0x0, 0,&branchTable[117], 587},
	{0x0, 0,&branchTable[117], 308},
	{0x0, 0,&branchTable[117], 312},
	{0x0, 0,&branchTable[117], 311},
	{0x0, 0,&branchTable[117], 25},
	{0x0, 0,&branchTable[117], 39},
	{0x0, 0,&branchTable[117], 410},
	{0x0, 0,&branchTable[117], 406},
	{0x0, 0,&branchTable[117], 112},
	{0x0, 0,&branchTable[117], 109},
	{0x0, 0,&branchTable[117], 27},
	{0x0, 0,&branchTable[117], 40},
	{0x0, 0,&branchTable[117], 7},
	{0x0, 0,&branchTable[117], 5},
	{0x0, 0,&branchTable[117], 15},
	{0xc00000, 1,&branchTable[117], -1},
	{0x0, 0,&branchTable[118], 618},
	{0x0, 0,&branchTable[118], 616},
	{0x0, 0,&branchTable[118], 624},
	{0xc00000, 1,&branchTable[118], -1},
	{0x0, 0,&branchTable[119], 13},
	{0x0, 0,&branchTable[119], 622},
	{0x803f2000, 2,&branchTable[119], -1},
	{0x803f2000, 2,&branchTable[121], -1},
	{0x80202000, 2,&branchTable[123], -1},
	{0x80202000, 2,&branchTable[125], -1},
	{0x0, 0,&branchTable[127], 581},
	{0x0, 0,&branchTable[127], 302},
	{0x0, 0,&branchTable[127], 583},
	{0x0, 0,&branchTable[127], 304},
	{0xd000, 3,&branchTable[127], -1},
	{0x0, 0,&branchTable[130], 558},
	{0x0, 0,&branchTable[130], 570},
	{0x0, 0,&branchTable[130], 566},
	{0x0, 0,&branchTable[130], 562},
	{0xd000, 3,&branchTable[130], -1},
	{0x0, 0,&branchTable[133], 271},
	{0x0, 0,&branchTable[133], 289},
	{0x0, 0,&branchTable[133], 283},
	{0x0, 0,&branchTable[133], 277},
	{0xd000, 3,&branchTable[133], -1},
	{0x0, 0,&branchTable[136], 559},
	{0x0, 0,&branchTable[136], 571},
	{0x0, 0,&branchTable[136], 567},
	{0x0, 0,&branchTable[136], 563},
	{0xd000, 3,&branchTable[136], -1},
	{0x0, 0,&branchTable[139], 272},
	{0x0, 0,&branchTable[139], 290},
	{0x0, 0,&branchTable[139], 284},
	{0x0, 0,&branchTable[139], 278},
	{0x803f2000, 4,&branchTable[139], -1},
	{0x803f2000, 4,&branchTable[143], -1},
	{0x80202000, 4,&branchTable[147], -1},
	{0x80202000, 4,&branchTable[151], -1},
	{0x0, 0,&branchTable[155], 585},
	{0x0, 0,&branchTable[155], 306},
	{0x0, 0,&branchTable[155], 584},
	{0x0, 0,&branchTable[155], 305},
	{0x0, 0,&branchTable[155], 560},
	{0x0, 0,&branchTable[155], 568},
	{0x0, 0,&branchTable[155], 564},
	{0x0, 0,&branchTable[155], 572},
	{0xC000, 4,&branchTable[155], -1},
	{0xC000, 4,&branchTable[159], -1},
	{0xC000, 4,&branchTable[163], -1},
	{0xC000, 4,&branchTable[167], -1},
	{0x0, 0,&branchTable[171], 561},
	{0x0, 0,&branchTable[171], 569},
	{0x0, 0,&branchTable[171], 565},
	{0x0, 0,&branchTable[171], 573},
	{0xC000, 4,&branchTable[171], -1},
	{0xC000, 4,&branchTable[175], -1},
	{0xC000, 4,&branchTable[179], -1},
	{0xC000, 4,&branchTable[183], -1},
	{0x1800, 4,&branchTable[187], -1},
	{0xc07800, 5,&branchTable[191], -1},
	{0x7800, 16,&branchTable[196], -1},
	{0x7800, 16,&branchTable[212], -1},
	{0x7800, 15,&branchTable[228], -1},
	{0x7800, 15,&branchTable[243], -1},
	{0x0, 0,&branchTable[258], 114},
	{0x40c00000, 1,&branchTable[258], -1},
	{0x7800, 15,&branchTable[259], -1},
	{0x7800, 16,&branchTable[274], -1},
	{0x7800, 10,&branchTable[290], -1},
	{0x7800, 14,&branchTable[300], -1},
	{0x0, 0,&branchTable[314], 634},
	{0x6000, 2,&branchTable[314], -1},
	{0x0, 0,&branchTable[316], 636},
	{0x6000, 4,&branchTable[316], -1},
	{0x0, 0,&branchTable[320], 639},
	{0x0, 0,&branchTable[320], 640},
	{0x0, 0,&branchTable[320], 721},
	{0x0, 0,&branchTable[320], 727},
	{0x0, 0,&branchTable[320], 722},
	{0x0, 0,&branchTable[320], 728},
	{0x0, 0,&branchTable[320], 107},
	{0x0, 0,&branchTable[320], 108},
	{0x40000000, 1,&branchTable[320], -1},
	{0x0, 0,&branchTable[321], 486},
	{0x0, 0,&branchTable[321], 382},
	{0x0, 0,&branchTable[321], 269},
	{0x0, 0,&branchTable[321], 437},
	{0x0, 0,&branchTable[321], 426},
	{0x0, 0,&branchTable[321], 440},
	{0x0, 0,&branchTable[321], 422},
	{0x0, 0,&branchTable[321], 556},
	{0x1f0000, 2,&branchTable[321], -1},
	{0x0, 0,&branchTable[323], 557},
	{0x1f0000, 2,&branchTable[323], -1},
	{0x0, 0,&branchTable[325], 10},
	{0x1f0000, 3,&branchTable[325], -1},
	{0x0, 0,&branchTable[328], 433},
	{0x1f0000, 2,&branchTable[328], -1},
	{0x0, 0,&branchTable[330], 621},
	{0x1f0000, 3,&branchTable[330], -1},
	{0x0, 0,&branchTable[333], 435},
	{0x1f0000, 3,&branchTable[333], -1},
	{0x0, 0,&branchTable[336], 438},
	{0x0, 0,&branchTable[336], 725},
	{0x0, 0,&branchTable[336], 626},
	{0x0, 0,&branchTable[336], 439},
	{0x0, 0,&branchTable[336], 57},
	{0x0, 0,&branchTable[336], 537},
	{0x0, 0,&branchTable[336], 20},
	{0x0, 0,&branchTable[336], 90},
	{0x0, 0,&branchTable[336], 19},
	{0x0, 0,&branchTable[336], 436},
	{0x0, 0,&branchTable[336], 163},
	{0x0, 0,&branchTable[336], 22},
	{0x0, 0,&branchTable[336], 493},
	{0x0, 0,&branchTable[336], 156},
	{0x0, 0,&branchTable[336], 21},
	{0x0, 0,&branchTable[336], 465},
	{0x0, 0,&branchTable[336], 495},
	{0x0, 0,&branchTable[336], 540},
	{0xc00000, 4,&branchTable[336], -1},
	{0x0, 0,&branchTable[340], 470},
	{0x0, 0,&branchTable[340], 535},
	{0x0, 0,&branchTable[340], 70},
	{0x0, 0,&branchTable[340], 66},
	{0x0, 0,&branchTable[340], 550},
	{0x0, 0,&branchTable[340], 527},
	{0x0, 0,&branchTable[340], 544},
	{0x0, 0,&branchTable[340], 519},
	{0x0, 0,&branchTable[340], 474},
	{0x0, 0,&branchTable[340], 478},
	{0x0, 0,&branchTable[340], 434},
	{0x0, 0,&branchTable[340], 432},
	{0x0, 0,&branchTable[340], 23},
	{0x0, 0,&branchTable[340], 38},
	{0x0, 0,&branchTable[340], 379},
	{0x0, 0,&branchTable[340], 405},
	{0x0, 0,&branchTable[340], 482},
	{0x1f0000, 2,&branchTable[340], -1},
	{0x0, 0,&branchTable[342], 499},
	{0x1f0000, 2,&branchTable[342], -1},
	{0x0, 0,&branchTable[344], 484},
	{0x1f0000, 4,&branchTable[344], -1},
	{0x0, 0,&branchTable[348], 503},
	{0x1f0000, 3,&branchTable[348], -1},
	{0x0, 0,&branchTable[351], 490},
	{0x9f0000, 3,&branchTable[351], -1},
	{0x0, 0,&branchTable[354], 511},
	{0x9f0000, 3,&branchTable[354], -1},
	{0x0, 0,&branchTable[357], 412},
	{0x0, 0,&branchTable[357], 145},
	{0x0, 0,&branchTable[357], 118},
	{0x0, 0,&branchTable[357], 72},
	{0x800000, 2,&branchTable[357], -1},
	{0x0, 0,&branchTable[359], 248},
	{0x0, 0,&branchTable[359], 250},
	{0x0, 0,&branchTable[359], 64},
	{0x800000, 2,&branchTable[359], -1},
	{0x0, 0,&branchTable[361], 246},
	{0x0, 0,&branchTable[361], 254},
	{0x0, 0,&branchTable[361], 80},
	{0x800000, 2,&branchTable[361], -1},
	{0x0, 0,&branchTable[363], 476},
	{0x0, 0,&branchTable[363], 480},
	{0x0, 0,&branchTable[363], 165},
	{0x0, 0,&branchTable[363], 171},
	{0x0, 0,&branchTable[363], 2},
	{0x800000, 2,&branchTable[363], -1},
	{0x0, 0,&branchTable[365], 16},
	{0x0, 0,&branchTable[365], 158},
	{0x0, 0,&branchTable[365], 181},
	{0x0, 0,&branchTable[365], 151},
	{0x0, 0,&branchTable[365], 141},
	{0x0, 0,&branchTable[365], 698},
	{0x0, 0,&branchTable[365], 449},
	{0x0, 0,&branchTable[365], 133},
	{0x0, 0,&branchTable[365], 238},
	{0x0, 0,&branchTable[365], 9},
	{0x0, 0,&branchTable[365], 88},
	{0x0, 0,&branchTable[365], 369},
	{0x0, 0,&branchTable[365], 392},
	{0x0, 0,&branchTable[365], 475},
	{0x0, 0,&branchTable[365], 479},
	{0x0, 0,&branchTable[365], 507},
	{0x0, 0,&branchTable[365], 12},
	{0x800000, 2,&branchTable[365], -1},
	{0x800000, 2,&branchTable[367], -1},
	{0x800000, 2,&branchTable[369], -1},
	{0x0, 0,&branchTable[371], 231},
	{0x0, 0,&branchTable[371], 131},
	{0x800000, 2,&branchTable[371], -1},
	{0x800000, 2,&branchTable[373], -1},
	{0x0, 0,&branchTable[375], 195},
	{0x0, 0,&branchTable[375], 205},
	{0x0, 0,&branchTable[375], 215},
	{0x0, 0,&branchTable[375], 218},
	{0x0, 0,&branchTable[375], 124},
	{0x0, 0,&branchTable[375], 262},
	{0x0, 0,&branchTable[375], 193},
	{0x0, 0,&branchTable[375], 203},
	{0x0, 0,&branchTable[375], 240},
	{0x0, 0,&branchTable[375], 259},
	{0x0, 0,&branchTable[375], 268},
	{0x0, 0,&branchTable[375], 648},
	{0x0, 0,&branchTable[375], 424},
	{0x0, 0,&branchTable[375], 651},
	{0x0, 0,&branchTable[375], 716},
	{0x1f0000, 2,&branchTable[375], -1},
	{0x0, 0,&branchTable[377], 717},
	{0x1f0000, 3,&branchTable[377], -1},
	{0x0, 0,&branchTable[380], 417},
	{0x1f0000, 2,&branchTable[380], -1},
	{0x0, 0,&branchTable[382], 644},
	{0xdf0000, 2,&branchTable[382], -1},
	{0x0, 0,&branchTable[384], 431},
	{0x1f0000, 2,&branchTable[384], -1},
	{0x0, 0,&branchTable[386], 646},
	{0x0, 0,&branchTable[386], 513},
	{0x0, 0,&branchTable[386], 649},
	{0x0, 0,&branchTable[386], 539},
	{0x0, 0,&branchTable[386], 713},
	{0x0, 0,&branchTable[386], 468},
	{0x0, 0,&branchTable[386], 650},
	{0x0, 0,&branchTable[386], 59},
	{0x0, 0,&branchTable[386], 697},
	{0x0, 0,&branchTable[386], 394},
	{0x0, 0,&branchTable[386], 418},
	{0x0, 0,&branchTable[386], 647},
	{0x0, 0,&branchTable[386], 177},
	{0x0, 0,&branchTable[386], 662},
	{0x0, 0,&branchTable[386], 683},
	{0x0, 0,&branchTable[386], 699},
	{0xc00000, 4,&branchTable[386], -1},
	{0x0, 0,&branchTable[390], 663},
	{0x0, 0,&branchTable[390], 695},
	{0x0, 0,&branchTable[390], 74},
	{0x0, 0,&branchTable[390], 76},
	{0x0, 0,&branchTable[390], 708},
	{0x0, 0,&branchTable[390], 691},
	{0x0, 0,&branchTable[390], 701},
	{0x0, 0,&branchTable[390], 685},
	{0x0, 0,&branchTable[390], 665},
	{0x0, 0,&branchTable[390], 668},
	{0x0, 0,&branchTable[390], 645},
	{0x0, 0,&branchTable[390], 643},
	{0x0, 0,&branchTable[390], 110},
	{0x0, 0,&branchTable[390], 47},
	{0x0, 0,&branchTable[390], 42},
	{0x0, 0,&branchTable[390], 41},
	{0x0, 0,&branchTable[390], 672},
	{0x1f0000, 2,&branchTable[390], -1},
	{0x1f0000, 2,&branchTable[392], -1},
	{0x0, 0,&branchTable[394], 674},
	{0x1f0000, 3,&branchTable[394], -1},
	{0x1f0000, 2,&branchTable[397], -1},
	{0x0, 0,&branchTable[399], 680},
	{0x9f0000, 5,&branchTable[399], -1},
	{0x9f0000, 3,&branchTable[404], -1},
	{0x9f0000, 4,&branchTable[407], -1},
	{0x0, 0,&branchTable[411], 68},
	{0x0, 0,&branchTable[411], 242},
	{0x0, 0,&branchTable[411], 78},
	{0x800000, 2,&branchTable[411], -1},
	{0x0, 0,&branchTable[413], 252},
	{0x0, 0,&branchTable[413], 244},
	{0x800000, 2,&branchTable[413], -1},
	{0x0, 0,&branchTable[415], 667},
	{0x0, 0,&branchTable[415], 670},
	{0x0, 0,&branchTable[415], 168},
	{0x0, 0,&branchTable[415], 174},
	{0x0, 0,&branchTable[415], 398},
	{0x800000, 2,&branchTable[415], -1},
	{0x0, 0,&branchTable[417], 161},
	{0x0, 0,&branchTable[417], 187},
	{0x0, 0,&branchTable[417], 154},
	{0x0, 0,&branchTable[417], 199},
	{0x0, 0,&branchTable[417], 137},
	{0x0, 0,&branchTable[417], 704},
	{0x0, 0,&branchTable[417], 209},
	{0x0, 0,&branchTable[417], 658},
	{0x0, 0,&branchTable[417], 143},
	{0x0, 0,&branchTable[417], 257},
	{0x0, 0,&branchTable[417], 202},
	{0x0, 0,&branchTable[417], 232},
	{0x0, 0,&branchTable[417], 260},
	{0x0, 0,&branchTable[417], 212},
	{0x0, 0,&branchTable[417], 620},
	{0x0, 0,&branchTable[417], 62},
	{0x0, 0,&branchTable[417], 371},
	{0x0, 0,&branchTable[417], 411},
	{0x0, 0,&branchTable[417], 666},
	{0x0, 0,&branchTable[417], 669},
	{0x0, 0,&branchTable[417], 517},
	{0x800000, 2,&branchTable[417], -1},
	{0x800000, 2,&branchTable[419], -1},
	{0x0, 0,&branchTable[421], 226},
	{0x800000, 2,&branchTable[421], -1},
	{0x800000, 2,&branchTable[423], -1},
	{0x800000, 2,&branchTable[425], -1},
	{0x0, 0,&branchTable[427], 190},
	{0x0, 0,&branchTable[427], 198},
	{0x0, 0,&branchTable[427], 208},
	{0x0, 0,&branchTable[427], 127},
	{0x0, 0,&branchTable[427], 117},
	{0x0, 0,&branchTable[427], 135},
	{0x0, 0,&branchTable[427], 139},
	{0x0, 0,&branchTable[427], 121},
	{0x0, 0,&branchTable[427], 123},
	{0x0, 0,&branchTable[427], 201},
	{0x0, 0,&branchTable[427], 211},
	{0x2000f000, 18,&branchTable[427], -1},
	{0xf80800, 31,&branchTable[445], -1},
	{0x0, 0,&branchTable[476], 214},
	{0x0, 0,&branchTable[476], 481},
	{0x0, 0,&branchTable[476], 497},
	{0x0, 0,&branchTable[476], 217},
	{0x0, 0,&branchTable[476], 483},
	{0x0, 0,&branchTable[476], 501},
	{0x0, 0,&branchTable[476], 391},
	{0x0, 0,&branchTable[476], 225},
	{0x0, 0,&branchTable[476], 489},
	{0x0, 0,&branchTable[476], 509},
	{0x0, 0,&branchTable[476], 505},
	{0x0, 0,&branchTable[476], 515},
	{0x0, 0,&branchTable[476], 368},
	{0x0, 0,&branchTable[476], 671},
	{0x0, 0,&branchTable[476], 370},
	{0x0, 0,&branchTable[476], 673},
	{0x0, 0,&branchTable[476], 229},
	{0x0, 0,&branchTable[476], 679},
	{0x0000f000, 16,&branchTable[476], -1},
	{0x2000f000, 22,&branchTable[492], -1},
	{0x2000f000, 6,&branchTable[514], -1},
	{0x0, 0,&branchTable[520], 553},
	{0x0, 0,&branchTable[520], 555},
	{0x0, 0,&branchTable[520], 546},
	{0x0, 0,&branchTable[520], 548},
	{0x0, 0,&branchTable[520], 467},
	{0x0, 0,&branchTable[520], 525},
	{0x0, 0,&branchTable[520], 469},
	{0x0, 0,&branchTable[520], 531},
	{0x0, 0,&branchTable[520], 551},
	{0x0, 0,&branchTable[520], 447},
	{0x0, 0,&branchTable[520], 711},
	{0x0, 0,&branchTable[520], 715},
	{0x0, 0,&branchTable[520], 703},
	{0x0, 0,&branchTable[520], 706},
	{0x0, 0,&branchTable[520], 542},
	{0x0, 0,&branchTable[520], 472},
	{0x0, 0,&branchTable[520], 529},
	{0x0, 0,&branchTable[520], 689},
	{0x0, 0,&branchTable[520], 533},
	{0x0, 0,&branchTable[520], 693},
	{0x0, 0,&branchTable[520], 709},
	{0x0, 0,&branchTable[520], 656},
	{0x0, 0,&branchTable[520], 430},
	{0x0, 0,&branchTable[520], 521},
	{0x0, 0,&branchTable[520], 179},
	{0x0, 0,&branchTable[520], 523},
	{0x0, 0,&branchTable[520], 687},
	{0x0, 0,&branchTable[520], 185},
	{0x3000000, 4,&branchTable[520], -1},
	{0x60000000, 3,&branchTable[524], -1},
	{0x80000000, 2,&branchTable[527], -1},
	{0x60000000, 4,&branchTable[529], -1},
	{0x60800000, 7,&branchTable[533], -1},
	{0x60800000, 4,&branchTable[540], -1},
	{0x0, 0,&branchTable[544], 17},
	{0x0, 0,&branchTable[544], 18},
	{0x0, 0,&branchTable[544], 6},
	{0x0, 0,&branchTable[544], 14},
	{0x0, 0,&branchTable[544], 617},
	{0x0, 0,&branchTable[544], 623},
	{0x0, 0,&branchTable[544], 24},
	{0x0, 0,&branchTable[544], 377},
	{0x0, 0,&branchTable[544], 409},
	{0x0, 0,&branchTable[544], 111},
	{0x0, 0,&branchTable[544], 378},
	{0x0, 0,&branchTable[544], 26},
	{0x0, 0,&branchTable[544], 384},
	{0x0, 0,&branchTable[544], 443},
	{0x200000, 1,&branchTable[544], -1},
	{0x0, 0,&branchTable[545], 34},
	{0x0, 0,&branchTable[545], 362},
	{0x0, 0,&branchTable[545], 115},
	{0x80000000, 2,&branchTable[545], -1},
	{0x3000000, 4,&branchTable[547], -1},
	{0x83000000, 4,&branchTable[551], -1},
	{0x0, 0,&branchTable[555], 33},
	{0x0, 0,&branchTable[555], 43},
	{0x0, 0,&branchTable[555], 49},
	{0x0, 0,&branchTable[555], 48},
	{0x0, 0,&branchTable[555], 637},
	{0x0, 0,&branchTable[555], 635},
	{0x0, 0,&branchTable[555], 32},
	{0xe0001f, 8,&branchTable[555], -1},
	{0xf00000, 4,&branchTable[563], -1},
	{0xfffc1f, 5,&branchTable[567], -1},
	{0x0, 0,&branchTable[572], 627},
	{0x0, 0,&branchTable[572], 266},
	{0x0, 0,&branchTable[572], 477},
	{0x0, 0,&branchTable[572], 46},
	{0x0, 0,&branchTable[572], 265},
	{0x0, 0,&branchTable[572], 100},
	{0x0, 0,&branchTable[572], 101},
	{0x0, 0,&branchTable[572], 102},
	{0x80000, 2,&branchTable[572], -1},
	{0x0, 0,&branchTable[574], 389},
	{0x0, 0,&branchTable[574], 633},
	{0x0, 0,&branchTable[574], 387},
	{0xf01f, 3,&branchTable[574], -1},
	{0x0, 0,&branchTable[577], 632},
	{0x70000, 1,&branchTable[577], -1},
	{0x700e0, 4,&branchTable[578], -1},
	{0x0, 0,&branchTable[582], 388},
	{0x0, 0,&branchTable[582], 264},
	{0x0, 0,&branchTable[582], 56},
	{0x0, 0,&branchTable[582], 105},
	{0x0, 0,&branchTable[582], 103},
	{0x0, 0,&branchTable[582], 270},
	{0x0, 0,&branchTable[582], 45},
	{0x0, 0,&branchTable[582], 44},
	{0x0, 0,&branchTable[582], 420},
	{0x0, 0,&branchTable[582], 113},
	{0x0, 0,&branchTable[582], 104},
	{0x80000000, 2,&branchTable[582], -1},
	{0x40e00000, 6,&branchTable[584], -1},
	{0x40e08000, 8,&branchTable[590], -1},
	{0x0, 0,&branchTable[598], 319},
	{0x40200000, 4,&branchTable[598], -1},
	{0xc0008000, 4,&branchTable[602], -1},
	{0x80a00c00, 20,&branchTable[606], -1},
	{0x80800000, 4,&branchTable[626], -1},
	{0x40e00c00, 6,&branchTable[630], -1},
	{0x600c00, 8,&branchTable[636], -1},
	{0x400000, 2,&branchTable[644], -1},
	{0xc020fc00, 26,&branchTable[646], -1},
	{0xc080f400, 13,&branchTable[672], -1},
	{0x0, 0,&branchTable[685], 320},
	{0x40000000, 2,&branchTable[685], -1},
	{0x0, 0,&branchTable[687], 342},
	{0x0, 0,&branchTable[687], 414},
	{0x0, 0,&branchTable[687], 3},
	{0xc00, 2,&branchTable[687], -1},
	{0xf000, 4,&branchTable[689], -1},
	{0xfc00, 1,&branchTable[693], -1},
	{0xc00, 2,&branchTable[694], -1},
	{0x1ff800, 3,&branchTable[696], -1},
	{0x0, 0,&branchTable[699], 93},
	{0x0, 0,&branchTable[699], 96},
	{0xc00, 2,&branchTable[699], -1},
	{0xc00, 4,&branchTable[701], -1},
	{0x0, 0,&branchTable[705], 91},
	{0x0, 0,&branchTable[705], 92},
	{0x0, 0,&branchTable[705], 661},
	{0x0, 0,&branchTable[705], 452},
	{0x0, 0,&branchTable[705], 361},
	{0x0, 0,&branchTable[705], 364},
	{0x0, 0,&branchTable[705], 28},
	{0x0, 0,&branchTable[705], 428},
	{0x0, 0,&branchTable[705], 441},
	{0x0, 0,&branchTable[705], 97},
	{0x0, 0,&branchTable[705], 98},
	{0x400, 2,&branchTable[705], -1},
	{0x0, 0,&branchTable[707], 421},
	{0x400, 2,&branchTable[707], -1},
	{0x0, 0,&branchTable[709], 419},
	{0x0, 0,&branchTable[709], 423},
	{0x0, 0,&branchTable[709], 60},
	{0x0, 0,&branchTable[709], 58},
	{0x0, 0,&branchTable[709], 367},
	{0x0, 0,&branchTable[709], 390},
	{0x80000000, 1,&branchTable[709], -1},
	{0x80000000, 1,&branchTable[710], -1},
	{0x0, 0,&branchTable[711], 488},
	{0x80000000, 1,&branchTable[711], -1},
	{0x80000000, 1,&branchTable[712], -1},
	{0x0, 0,&branchTable[713], 678},
	{0x0, 0,&branchTable[713], 473},
	{0x0, 0,&branchTable[713], 487},
	{0x0, 0,&branchTable[713], 664},
	{0x0, 0,&branchTable[713], 677},
	{0x9f0000, 4,&branchTable[713], -1},
	{0xc00, 4,&branchTable[717], -1},
	{0x80c0fc00, 8,&branchTable[721], -1},
	{0x8000fc00, 31,&branchTable[729], -1},
	{0x0, 0,&branchTable[760], 450},
	{0x0, 0,&branchTable[760], 659},
	{0x0, 0,&branchTable[760], 182},
	{0x0, 0,&branchTable[760], 188},
	{0x1000, 2,&branchTable[760], -1},
	{0x80800010, 2,&branchTable[762], -1},
	{0x8080f000, 9,&branchTable[764], -1},
	{0x0, 0,&branchTable[773], 148},
	{0x6000, 3,&branchTable[773], -1},
	{0x0, 0,&branchTable[776], 222},
	{0x168000, 5,&branchTable[776], -1},
	{0x80808017, 2,&branchTable[781], -1},
	{0x801e0000, 4,&branchTable[783], -1},
	{0x890000, 4,&branchTable[787], -1},
	{0x890000, 2,&branchTable[791], -1},
	{0x890000, 2,&branchTable[793], -1},
	{0x0, 0,&branchTable[795], 221},
	{0x890000, 4,&branchTable[795], -1},
	{0x0, 0,&branchTable[799], 166},
	{0x0, 0,&branchTable[799], 169},
	{0x0, 0,&branchTable[799], 172},
	{0x0, 0,&branchTable[799], 175},
	{0x0, 0,&branchTable[799], 451},
	{0x0, 0,&branchTable[799], 660},
	{0x0, 0,&branchTable[799], 152},
	{0x0, 0,&branchTable[799], 155},
	{0x0, 0,&branchTable[799], 159},
	{0x0, 0,&branchTable[799], 162},
	{0x0, 0,&branchTable[799], 183},
	{0x0, 0,&branchTable[799], 189},
	{0x0, 0,&branchTable[799], 146},
	{0x0, 0,&branchTable[799], 147},
	{0x818000, 4,&branchTable[799], -1},
	{0x0, 0,&branchTable[803], 149},
	{0x818000, 4,&branchTable[803], -1},
	{0x818000, 3,&branchTable[807], -1},
	{0x0, 0,&branchTable[810], 220},
	{0x0, 0,&branchTable[810], 119},
	{0x0, 0,&branchTable[810], 233},
	{0x0, 0,&branchTable[810], 261},
	{0x0, 0,&branchTable[810], 249},
	{0x0, 0,&branchTable[810], 251},
	{0x0, 0,&branchTable[810], 247},
	{0x0, 0,&branchTable[810], 255},
	{0x0, 0,&branchTable[810], 243},
	{0x0, 0,&branchTable[810], 253},
	{0x0, 0,&branchTable[810], 245},
	{0x0, 0,&branchTable[810], 128},
	{0x0, 0,&branchTable[810], 129},
	{0x0, 0,&branchTable[810], 227},
	{0x0, 0,&branchTable[810], 191},
	{0x0, 0,&branchTable[810], 125},
	{0x0, 0,&branchTable[810], 263},
	{0x0, 0,&branchTable[810], 194},
	{0x0, 0,&branchTable[810], 204},
	{0x0, 0,&branchTable[810], 196},
	{0x0, 0,&branchTable[810], 206},
	{0x0, 0,&branchTable[810], 236},
	{0x0, 0,&branchTable[810], 455},
	{0x0, 0,&branchTable[810], 106},
	{0x0, 0,&branchTable[810], 458},
	{0x0, 0,&branchTable[810], 457},
	{0x0, 0,&branchTable[810], 459},
	{0x0, 0,&branchTable[810], 462},
	{0x0, 0,&branchTable[810], 461},
	{0x0, 0,&branchTable[810], 464},
	{0x0, 0,&branchTable[810], 456},
	{0x0, 0,&branchTable[810], 494},
	{0x0, 0,&branchTable[810], 460},
	{0x0, 0,&branchTable[810], 463},
	{0x0, 0,&branchTable[810], 534},
	{0x0, 0,&branchTable[810], 69},
	{0x0, 0,&branchTable[810], 625},
	{0x0, 0,&branchTable[810], 65},
	{0x0, 0,&branchTable[810], 549},
	{0x0, 0,&branchTable[810], 536},
	{0x0, 0,&branchTable[810], 526},
	{0x0, 0,&branchTable[810], 543},
	{0x0, 0,&branchTable[810], 518},
	{0x0, 0,&branchTable[810], 492},
	{0x0, 0,&branchTable[810], 8},
	{0x0, 0,&branchTable[810], 71},
	{0x0, 0,&branchTable[810], 87},
	{0x0, 0,&branchTable[810], 498},
	{0x0, 0,&branchTable[810], 63},
	{0x1f0000, 2,&branchTable[810], -1},
	{0x0, 0,&branchTable[812], 502},
	{0x0, 0,&branchTable[812], 506},
	{0x1f0000, 3,&branchTable[812], -1},
	{0x9f0000, 2,&branchTable[815], -1},
	{0x0, 0,&branchTable[817], 510},
	{0x9f0000, 3,&branchTable[817], -1},
	{0x0, 0,&branchTable[820], 230},
	{0x0, 0,&branchTable[820], 130},
	{0x0, 0,&branchTable[820], 144},
	{0x0, 0,&branchTable[820], 241},
	{0x800000, 2,&branchTable[820], -1},
	{0x0, 0,&branchTable[822], 79},
	{0x800000, 2,&branchTable[822], -1},
	{0x0, 0,&branchTable[824], 164},
	{0x0, 0,&branchTable[824], 170},
	{0x0, 0,&branchTable[824], 1},
	{0x800000, 2,&branchTable[824], -1},
	{0x0, 0,&branchTable[826], 11},
	{0x0, 0,&branchTable[826], 157},
	{0x0, 0,&branchTable[826], 180},
	{0x0, 0,&branchTable[826], 150},
	{0x0, 0,&branchTable[826], 140},
	{0x0, 0,&branchTable[826], 448},
	{0x0, 0,&branchTable[826], 132},
	{0x0, 0,&branchTable[826], 237},
	{0x0, 0,&branchTable[826], 239},
	{0x0, 0,&branchTable[826], 258},
	{0xa00000, 2,&branchTable[826], -1},
	{0xa00000, 2,&branchTable[828], -1},
	{0x7400, 10,&branchTable[830], -1},
	{0x7400, 7,&branchTable[840], -1},
	{0x0, 0,&branchTable[847], 192},
	{0x0, 0,&branchTable[847], 234},
	{0x0, 0,&branchTable[847], 223},
	{0x0, 0,&branchTable[847], 235},
	{0x0, 0,&branchTable[847], 552},
	{0x0, 0,&branchTable[847], 213},
	{0x0, 0,&branchTable[847], 554},
	{0x0, 0,&branchTable[847], 545},
	{0x0, 0,&branchTable[847], 496},
	{0x0, 0,&branchTable[847], 547},
	{0x0, 0,&branchTable[847], 216},
	{0x0, 0,&branchTable[847], 466},
	{0x0, 0,&branchTable[847], 500},
	{0x0, 0,&branchTable[847], 524},
	{0x0, 0,&branchTable[847], 224},
	{0x800800, 2,&branchTable[847], -1},
	{0x0, 0,&branchTable[849], 508},
	{0x0, 0,&branchTable[849], 504},
	{0x0, 0,&branchTable[849], 514},
	{0x0, 0,&branchTable[849], 446},
	{0x0, 0,&branchTable[849], 178},
	{0x0, 0,&branchTable[849], 530},
	{0x0, 0,&branchTable[849], 520},
	{0x40400000, 4,&branchTable[849], -1},
	{0x40400000, 4,&branchTable[853], -1},
	{0x40400000, 4,&branchTable[857], -1},
	{0x40400000, 4,&branchTable[861], -1},
	{0x40400000, 4,&branchTable[865], -1},
	{0x40000000, 2,&branchTable[869], -1},
	{0x40000000, 2,&branchTable[871], -1},
	{0x40000000, 2,&branchTable[873], -1},
	{0x40000000, 2,&branchTable[875], -1},
	{0x40000000, 2,&branchTable[877], -1},
	{0x400000, 2,&branchTable[879], -1},
	{0x400000, 2,&branchTable[881], -1},
	{0x400000, 2,&branchTable[883], -1},
	{0x400000, 2,&branchTable[885], -1},
	{0x400000, 2,&branchTable[887], -1},
	{0x40400000, 2,&branchTable[889], -1},
	{0x0, 0,&branchTable[891], 339},
	{0x0, 0,&branchTable[891], 349},
	{0x0, 0,&branchTable[891], 340},
	{0x40400000, 2,&branchTable[891], -1},
	{0x0, 0,&branchTable[893], 610},
	{0x0, 0,&branchTable[893], 352},
	{0x0, 0,&branchTable[893], 611},
	{0x0, 0,&branchTable[893], 353},
	{0x0, 0,&branchTable[893], 597},
	{0x0, 0,&branchTable[893], 323},
	{0x0, 0,&branchTable[893], 601},
	{0x0, 0,&branchTable[893], 327},
	{0x0, 0,&branchTable[893], 606},
	{0x0, 0,&branchTable[893], 345},
	{0x0, 0,&branchTable[893], 607},
	{0x0, 0,&branchTable[893], 346},
	{0x0, 0,&branchTable[893], 598},
	{0x0, 0,&branchTable[893], 324},
	{0x0, 0,&branchTable[893], 602},
	{0x0, 0,&branchTable[893], 328},
	{0x0, 0,&branchTable[893], 600},
	{0x0, 0,&branchTable[893], 326},
	{0x0, 0,&branchTable[893], 604},
	{0x0, 0,&branchTable[893], 330},
	{0x0, 0,&branchTable[893], 354},
	{0x0, 0,&branchTable[893], 355},
	{0x0, 0,&branchTable[893], 331},
	{0x0, 0,&branchTable[893], 335},
	{0x0, 0,&branchTable[893], 347},
	{0x0, 0,&branchTable[893], 348},
	{0x0, 0,&branchTable[893], 332},
	{0x0, 0,&branchTable[893], 336},
	{0x0, 0,&branchTable[893], 334},
	{0x0, 0,&branchTable[893], 338},
	{0x0, 0,&branchTable[893], 609},
	{0x0, 0,&branchTable[893], 351},
	{0x0, 0,&branchTable[893], 592},
	{0x0, 0,&branchTable[893], 316},
	{0x0, 0,&branchTable[893], 605},
	{0x0, 0,&branchTable[893], 344},
	{0x0, 0,&branchTable[893], 593},
	{0x0, 0,&branchTable[893], 317},
	{0x0, 0,&branchTable[893], 596},
	{0x0, 0,&branchTable[893], 322},
	{0x0, 0,&branchTable[893], 356},
	{0x0, 0,&branchTable[893], 416},
	{0x0, 0,&branchTable[893], 343},
	{0x0, 0,&branchTable[893], 415},
	{0x40400000, 4,&branchTable[893], -1},
	{0x40000000, 2,&branchTable[897], -1},
	{0x400000, 2,&branchTable[899], -1},
	{0x40400000, 2,&branchTable[901], -1},
	{0x0, 0,&branchTable[903], 599},
	{0x0, 0,&branchTable[903], 325},
	{0x0, 0,&branchTable[903], 603},
	{0x0, 0,&branchTable[903], 329},
	{0x0, 0,&branchTable[903], 333},
	{0x0, 0,&branchTable[903], 337},
	{0x0, 0,&branchTable[903], 594},
	{0x0, 0,&branchTable[903], 318},
	{0x0, 0,&branchTable[903], 341},
	{0x0, 0,&branchTable[903], 413},
	{0x0, 0,&branchTable[903], 4},
	{0x0, 0,&branchTable[903], 51},
	{0x0, 0,&branchTable[903], 50},
	{0xf000, 1,&branchTable[903], -1},
	{0x0, 0,&branchTable[904], 53},
	{0x0, 0,&branchTable[904], 52},
	{0x0, 0,&branchTable[904], 442},
	{0x0, 0,&branchTable[904], 608},
	{0x0, 0,&branchTable[904], 589},
	{0x0, 0,&branchTable[904], 590},
	{0x0, 0,&branchTable[904], 595},
	{0x0, 0,&branchTable[904], 350},
	{0x0, 0,&branchTable[904], 313},
	{0x0, 0,&branchTable[904], 314},
	{0x0, 0,&branchTable[904], 321},
	{0x0, 0,&branchTable[904], 591},
	{0x0, 0,&branchTable[904], 315},
	{0x0, 0,&branchTable[904], 682},
	{0x0, 0,&branchTable[904], 538},
	{0x0, 0,&branchTable[904], 694},
	{0x0, 0,&branchTable[904], 73},
	{0x0, 0,&branchTable[904], 712},
	{0x0, 0,&branchTable[904], 75},
	{0x0, 0,&branchTable[904], 707},
	{0x0, 0,&branchTable[904], 696},
	{0x0, 0,&branchTable[904], 690},
	{0x0, 0,&branchTable[904], 700},
	{0x0, 0,&branchTable[904], 684},
	{0x0, 0,&branchTable[904], 176},
	{0x0, 0,&branchTable[904], 512},
	{0x0, 0,&branchTable[904], 619},
	{0x0, 0,&branchTable[904], 67},
	{0x0, 0,&branchTable[904], 61},
	{0x0, 0,&branchTable[904], 77},
	{0x9f0000, 2,&branchTable[904], -1},
	{0x0, 0,&branchTable[906], 516},
	{0x1f0000, 2,&branchTable[906], -1},
	{0x9f0000, 4,&branchTable[908], -1},
	{0x0, 0,&branchTable[912], 116},
	{0x9f0000, 4,&branchTable[912], -1},
	{0x800000, 2,&branchTable[916], -1},
	{0x800000, 2,&branchTable[918], -1},
	{0x9f0000, 2,&branchTable[920], -1},
	{0x0, 0,&branchTable[922], 167},
	{0x0, 0,&branchTable[922], 173},
	{0x0, 0,&branchTable[922], 397},
	{0x800000, 2,&branchTable[922], -1},
	{0x0, 0,&branchTable[924], 160},
	{0x0, 0,&branchTable[924], 186},
	{0x0, 0,&branchTable[924], 153},
	{0x0, 0,&branchTable[924], 197},
	{0x0, 0,&branchTable[924], 136},
	{0x0, 0,&branchTable[924], 207},
	{0x0, 0,&branchTable[924], 657},
	{0x0, 0,&branchTable[924], 126},
	{0x0, 0,&branchTable[924], 142},
	{0x0, 0,&branchTable[924], 256},
	{0x0, 0,&branchTable[924], 134},
	{0x0, 0,&branchTable[924], 138},
	{0x0, 0,&branchTable[924], 120},
	{0x0, 0,&branchTable[924], 122},
	{0x0, 0,&branchTable[924], 200},
	{0x0, 0,&branchTable[924], 210},
	{0x0, 0,&branchTable[924], 710},
	{0x0, 0,&branchTable[924], 714},
	{0x0, 0,&branchTable[924], 702},
	{0x0, 0,&branchTable[924], 705},
	{0x0, 0,&branchTable[924], 541},
	{0x0, 0,&branchTable[924], 471},
	{0x0, 0,&branchTable[924], 528},
	{0x0, 0,&branchTable[924], 688},
	{0x800, 2,&branchTable[924], -1},
	{0x800, 2,&branchTable[926], -1},
	{0x0, 0,&branchTable[928], 655},
	{0x0, 0,&branchTable[928], 184},
	{0x0, 0,&branchTable[928], 228},
	{0x0, 0,&branchTable[928], 532},
	{0x0, 0,&branchTable[928], 522},
	{0x0, 0,&branchTable[928], 692},
	{0x0, 0,&branchTable[928], 686},
	{0x20000000, 2,&branchTable[928], -1},
	{0x20000000, 2,&branchTable[930], -1},
	{0x0, 0,&branchTable[932], 383},
	{0x0, 0,&branchTable[932], 219},
	{0x0, 0,&branchTable[932], 407},
	{0x0, 0,&branchTable[932], 37},
	{0x0, 0,&branchTable[932], 396},
	{0x0, 0,&branchTable[932], 273},
	{0x0, 0,&branchTable[932], 275},
	{0x0, 0,&branchTable[932], 274},
	{0x0, 0,&branchTable[932], 276},
	{0x0, 0,&branchTable[932], 279},
	{0x0, 0,&branchTable[932], 281},
	{0x0, 0,&branchTable[932], 280},
	{0x0, 0,&branchTable[932], 282},
	{0x0, 0,&branchTable[932], 285},
	{0x0, 0,&branchTable[932], 287},
	{0x0, 0,&branchTable[932], 286},
	{0x0, 0,&branchTable[932], 288},
	{0x0, 0,&branchTable[932], 291},
	{0x0, 0,&branchTable[932], 293},
	{0x0, 0,&branchTable[932], 292},
	{0x0, 0,&branchTable[932], 294},
	{0x18000000, 4,&branchTable[0], -1},
	{0x0, 0,&branchTable[4], 0},
	{0x7000000, 8,&branchTable[4], -1},
	{0x4000000, 2,&branchTable[12], -1},
	{0x27000000, 13,&branchTable[14], -1},
	{0x20c00000, 8,&branchTable[27], -1},
	{0x60c00000, 6,&branchTable[35], -1},
	{0x60200000, 8,&branchTable[41], -1},
	{0x60200000, 8,&branchTable[49], -1},
	{0x20c00000, 8,&branchTable[57], -1},
	{0x20c00000, 8,&branchTable[65], -1},
	{0xa0208400, 12,&branchTable[73], -1},
	{0x80000400, 2,&branchTable[85], -1},
	{0x80208000, 6,&branchTable[87], -1},
	{0x803f8000, 6,&branchTable[93], -1},
	{0x803ffc00, 2,&branchTable[99], -1},
	{0x803ffc00, 2,&branchTable[101], -1},
	{0x0, 0,&branchTable[103], 582},
	{0x0, 0,&branchTable[103], 303},
	{0x0, 0,&branchTable[103], 586},
	{0x40000000, 2,&branchTable[103], -1},
	{0x40007c00, 2,&branchTable[105], -1},
	{0x40007c00, 2,&branchTable[107], -1},
	{0x0, 0,&branchTable[109], 613},
	{0x0, 0,&branchTable[109], 578},
	{0x0, 0,&branchTable[109], 612},
	{0x0, 0,&branchTable[109], 577},
	{0x0, 0,&branchTable[109], 614},
	{0x0, 0,&branchTable[109], 615},
	{0x0, 0,&branchTable[109], 579},
	{0x0, 0,&branchTable[109], 580},
	{0x40007c00, 2,&branchTable[109], -1},
	{0x40007c00, 2,&branchTable[111], -1},
	{0x0, 0,&branchTable[113], 358},
	{0x0, 0,&branchTable[113], 299},
	{0x0, 0,&branchTable[113], 357},
	{0x0, 0,&branchTable[113], 298},
	{0x0, 0,&branchTable[113], 359},
	{0x0, 0,&branchTable[113], 360},
	{0x0, 0,&branchTable[113], 300},
	{0x0, 0,&branchTable[113], 301},
	{0x40000000, 2,&branchTable[113], -1},
	{0x0, 0,&branchTable[115], 574},
	{0x0, 0,&branchTable[115], 575},
	{0x0, 0,&branchTable[115], 576},
	{0x40000000, 2,&branchTable[115], -1},
	{0x0, 0,&branchTable[117], 295},
	{0x0, 0,&branchTable[117], 296},
	{0x0, 0,&branchTable[117], 297},
	{0x0, 0,&branchTable[117], 307},
	{0x0, 0,&branchTable[117], 310},
	{0x0, 0,&branchTable[117], 588},
	{0x0, 0,&branchTable[117], 309},
	{0x0, 0,&branchTable[117], 587},
	{0x0, 0,&branchTable[117], 308},
	{0x0, 0,&branchTable[117], 312},
	{0x0, 0,&branchTable[117], 311},
	{0x0, 0,&branchTable[117], 25},
	{0x0, 0,&branchTable[117], 39},
	{0x0, 0,&branchTable[117], 410},
	{0x0, 0,&branchTable[117], 406},
	{0x0, 0,&branchTable[117], 112},
	{0x0, 0,&branchTable[117], 109},
	{0x0, 0,&branchTable[117], 27},
	{0x0, 0,&branchTable[117], 40},
	{0x0, 0,&branchTable[117], 7},
	{0x0, 0,&branchTable[117], 5},
	{0x0, 0,&branchTable[117], 15},
	{0xc00000, 1,&branchTable[117], -1},
	{0x0, 0,&branchTable[118], 618},
	{0x0, 0,&branchTable[118], 616},
	{0x0, 0,&branchTable[118], 624},
	{0xc00000, 1,&branchTable[118], -1},
	{0x0, 0,&branchTable[119], 13},
	{0x0, 0,&branchTable[119], 622},
	{0x803f2000, 2,&branchTable[119], -1},
	{0x803f2000, 2,&branchTable[121], -1},
	{0x80202000, 2,&branchTable[123], -1},
	{0x80202000, 2,&branchTable[125], -1},
	{0x0, 0,&branchTable[127], 581},
	{0x0, 0,&branchTable[127], 302},
	{0x0, 0,&branchTable[127], 583},
	{0x0, 0,&branchTable[127], 304},
	{0xd000, 3,&branchTable[127], -1},
	{0x0, 0,&branchTable[130], 558},
	{0x0, 0,&branchTable[130], 570},
	{0x0, 0,&branchTable[130], 566},
	{0x0, 0,&branchTable[130], 562},
	{0xd000, 3,&branchTable[130], -1},
	{0x0, 0,&branchTable[133], 271},
	{0x0, 0,&branchTable[133], 289},
	{0x0, 0,&branchTable[133], 283},
	{0x0, 0,&branchTable[133], 277},
	{0xd000, 3,&branchTable[133], -1},
	{0x0, 0,&branchTable[136], 559},
	{0x0, 0,&branchTable[136], 571},
	{0x0, 0,&branchTable[136], 567},
	{0x0, 0,&branchTable[136], 563},
	{0xd000, 3,&branchTable[136], -1},
	{0x0, 0,&branchTable[139], 272},
	{0x0, 0,&branchTable[139], 290},
	{0x0, 0,&branchTable[139], 284},
	{0x0, 0,&branchTable[139], 278},
	{0x803f2000, 4,&branchTable[139], -1},
	{0x803f2000, 4,&branchTable[143], -1},
	{0x80202000, 4,&branchTable[147], -1},
	{0x80202000, 4,&branchTable[151], -1},
	{0x0, 0,&branchTable[155], 585},
	{0x0, 0,&branchTable[155], 306},
	{0x0, 0,&branchTable[155], 584},
	{0x0, 0,&branchTable[155], 305},
	{0x0, 0,&branchTable[155], 560},
	{0x0, 0,&branchTable[155], 568},
	{0x0, 0,&branchTable[155], 564},
	{0x0, 0,&branchTable[155], 572},
	{0xC000, 4,&branchTable[155], -1},
	{0xC000, 4,&branchTable[159], -1},
	{0xC000, 4,&branchTable[163], -1},
	{0xC000, 4,&branchTable[167], -1},
	{0x0, 0,&branchTable[171], 561},
	{0x0, 0,&branchTable[171], 569},
	{0x0, 0,&branchTable[171], 565},
	{0x0, 0,&branchTable[171], 573},
	{0xC000, 4,&branchTable[171], -1},
	{0xC000, 4,&branchTable[175], -1},
	{0xC000, 4,&branchTable[179], -1},
	{0xC000, 4,&branchTable[183], -1},
	{0x1800, 4,&branchTable[187], -1},
	{0xc07800, 5,&branchTable[191], -1},
	{0x7800, 16,&branchTable[196], -1},
	{0x7800, 16,&branchTable[212], -1},
	{0x7800, 15,&branchTable[228], -1},
	{0x7800, 15,&branchTable[243], -1},
	{0x0, 0,&branchTable[258], 114},
	{0x40c00000, 1,&branchTable[258], -1},
	{0x7800, 15,&branchTable[259], -1},
	{0x7800, 16,&branchTable[274], -1},
	{0x7800, 10,&branchTable[290], -1},
	{0x7800, 14,&branchTable[300], -1},
	{0x0, 0,&branchTable[314], 634},
	{0x6000, 2,&branchTable[314], -1},
	{0x0, 0,&branchTable[316], 636},
	{0x6000, 4,&branchTable[316], -1},
	{0x0, 0,&branchTable[320], 639},
	{0x0, 0,&branchTable[320], 640},
	{0x0, 0,&branchTable[320], 721},
	{0x0, 0,&branchTable[320], 727},
	{0x0, 0,&branchTable[320], 722},
	{0x0, 0,&branchTable[320], 728},
	{0x0, 0,&branchTable[320], 107},
	{0x0, 0,&branchTable[320], 108},
	{0x40000000, 1,&branchTable[320], -1},
	{0x0, 0,&branchTable[321], 486},
	{0x0, 0,&branchTable[321], 382},
	{0x0, 0,&branchTable[321], 269},
	{0x0, 0,&branchTable[321], 437},
	{0x0, 0,&branchTable[321], 426},
	{0x0, 0,&branchTable[321], 440},
	{0x0, 0,&branchTable[321], 422},
	{0x0, 0,&branchTable[321], 556},
	{0x1f0000, 2,&branchTable[321], -1},
	{0x0, 0,&branchTable[323], 557},
	{0x1f0000, 2,&branchTable[323], -1},
	{0x0, 0,&branchTable[325], 10},
	{0x1f0000, 3,&branchTable[325], -1},
	{0x0, 0,&branchTable[328], 433},
	{0x1f0000, 2,&branchTable[328], -1},
	{0x0, 0,&branchTable[330], 621},
	{0x1f0000, 3,&branchTable[330], -1},
	{0x0, 0,&branchTable[333], 435},
	{0x1f0000, 3,&branchTable[333], -1},
	{0x0, 0,&branchTable[336], 438},
	{0x0, 0,&branchTable[336], 725},
	{0x0, 0,&branchTable[336], 626},
	{0x0, 0,&branchTable[336], 439},
	{0x0, 0,&branchTable[336], 57},
	{0x0, 0,&branchTable[336], 537},
	{0x0, 0,&branchTable[336], 20},
	{0x0, 0,&branchTable[336], 90},
	{0x0, 0,&branchTable[336], 19},
	{0x0, 0,&branchTable[336], 436},
	{0x0, 0,&branchTable[336], 163},
	{0x0, 0,&branchTable[336], 22},
	{0x0, 0,&branchTable[336], 493},
	{0x0, 0,&branchTable[336], 156},
	{0x0, 0,&branchTable[336], 21},
	{0x0, 0,&branchTable[336], 465},
	{0x0, 0,&branchTable[336], 495},
	{0x0, 0,&branchTable[336], 540},
	{0xc00000, 4,&branchTable[336], -1},
	{0x0, 0,&branchTable[340], 470},
	{0x0, 0,&branchTable[340], 535},
	{0x0, 0,&branchTable[340], 70},
	{0x0, 0,&branchTable[340], 66},
	{0x0, 0,&branchTable[340], 550},
	{0x0, 0,&branchTable[340], 527},
	{0x0, 0,&branchTable[340], 544},
	{0x0, 0,&branchTable[340], 519},
	{0x0, 0,&branchTable[340], 474},
	{0x0, 0,&branchTable[340], 478},
	{0x0, 0,&branchTable[340], 434},
	{0x0, 0,&branchTable[340], 432},
	{0x0, 0,&branchTable[340], 23},
	{0x0, 0,&branchTable[340], 38},
	{0x0, 0,&branchTable[340], 379},
	{0x0, 0,&branchTable[340], 405},
	{0x0, 0,&branchTable[340], 482},
	{0x1f0000, 2,&branchTable[340], -1},
	{0x0, 0,&branchTable[342], 499},
	{0x1f0000, 2,&branchTable[342], -1},
	{0x0, 0,&branchTable[344], 484},
	{0x1f0000, 4,&branchTable[344], -1},
	{0x0, 0,&branchTable[348], 503},
	{0x1f0000, 3,&branchTable[348], -1},
	{0x0, 0,&branchTable[351], 490},
	{0x9f0000, 3,&branchTable[351], -1},
	{0x0, 0,&branchTable[354], 511},
	{0x9f0000, 3,&branchTable[354], -1},
	{0x0, 0,&branchTable[357], 412},
	{0x0, 0,&branchTable[357], 145},
	{0x0, 0,&branchTable[357], 118},
	{0x0, 0,&branchTable[357], 72},
	{0x800000, 2,&branchTable[357], -1},
	{0x0, 0,&branchTable[359], 248},
	{0x0, 0,&branchTable[359], 250},
	{0x0, 0,&branchTable[359], 64},
	{0x800000, 2,&branchTable[359], -1},
	{0x0, 0,&branchTable[361], 246},
	{0x0, 0,&branchTable[361], 254},
	{0x0, 0,&branchTable[361], 80},
	{0x800000, 2,&branchTable[361], -1},
	{0x0, 0,&branchTable[363], 476},
	{0x0, 0,&branchTable[363], 480},
	{0x0, 0,&branchTable[363], 165},
	{0x0, 0,&branchTable[363], 171},
	{0x0, 0,&branchTable[363], 2},
	{0x800000, 2,&branchTable[363], -1},
	{0x0, 0,&branchTable[365], 16},
	{0x0, 0,&branchTable[365], 158},
	{0x0, 0,&branchTable[365], 181},
	{0x0, 0,&branchTable[365], 151},
	{0x0, 0,&branchTable[365], 141},
	{0x0, 0,&branchTable[365], 698},
	{0x0, 0,&branchTable[365], 449},
	{0x0, 0,&branchTable[365], 133},
	{0x0, 0,&branchTable[365], 238},
	{0x0, 0,&branchTable[365], 9},
	{0x0, 0,&branchTable[365], 88},
	{0x0, 0,&branchTable[365], 369},
	{0x0, 0,&branchTable[365], 392},
	{0x0, 0,&branchTable[365], 475},
	{0x0, 0,&branchTable[365], 479},
	{0x0, 0,&branchTable[365], 507},
	{0x0, 0,&branchTable[365], 12},
	{0x800000, 2,&branchTable[365], -1},
	{0x800000, 2,&branchTable[367], -1},
	{0x800000, 2,&branchTable[369], -1},
	{0x0, 0,&branchTable[371], 231},
	{0x0, 0,&branchTable[371], 131},
	{0x800000, 2,&branchTable[371], -1},
	{0x800000, 2,&branchTable[373], -1},
	{0x0, 0,&branchTable[375], 195},
	{0x0, 0,&branchTable[375], 205},
	{0x0, 0,&branchTable[375], 215},
	{0x0, 0,&branchTable[375], 218},
	{0x0, 0,&branchTable[375], 124},
	{0x0, 0,&branchTable[375], 262},
	{0x0, 0,&branchTable[375], 193},
	{0x0, 0,&branchTable[375], 203},
	{0x0, 0,&branchTable[375], 240},
	{0x0, 0,&branchTable[375], 259},
	{0x0, 0,&branchTable[375], 268},
	{0x0, 0,&branchTable[375], 648},
	{0x0, 0,&branchTable[375], 424},
	{0x0, 0,&branchTable[375], 651},
	{0x0, 0,&branchTable[375], 716},
	{0x1f0000, 2,&branchTable[375], -1},
	{0x0, 0,&branchTable[377], 717},
	{0x1f0000, 3,&branchTable[377], -1},
	{0x0, 0,&branchTable[380], 417},
	{0x1f0000, 2,&branchTable[380], -1},
	{0x0, 0,&branchTable[382], 644},
	{0xdf0000, 2,&branchTable[382], -1},
	{0x0, 0,&branchTable[384], 431},
	{0x1f0000, 2,&branchTable[384], -1},
	{0x0, 0,&branchTable[386], 646},
	{0x0, 0,&branchTable[386], 513},
	{0x0, 0,&branchTable[386], 649},
	{0x0, 0,&branchTable[386], 539},
	{0x0, 0,&branchTable[386], 713},
	{0x0, 0,&branchTable[386], 468},
	{0x0, 0,&branchTable[386], 650},
	{0x0, 0,&branchTable[386], 59},
	{0x0, 0,&branchTable[386], 697},
	{0x0, 0,&branchTable[386], 394},
	{0x0, 0,&branchTable[386], 418},
	{0x0, 0,&branchTable[386], 647},
	{0x0, 0,&branchTable[386], 177},
	{0x0, 0,&branchTable[386], 662},
	{0x0, 0,&branchTable[386], 683},
	{0x0, 0,&branchTable[386], 699},
	{0xc00000, 4,&branchTable[386], -1},
	{0x0, 0,&branchTable[390], 663},
	{0x0, 0,&branchTable[390], 695},
	{0x0, 0,&branchTable[390], 74},
	{0x0, 0,&branchTable[390], 76},
	{0x0, 0,&branchTable[390], 708},
	{0x0, 0,&branchTable[390], 691},
	{0x0, 0,&branchTable[390], 701},
	{0x0, 0,&branchTable[390], 685},
	{0x0, 0,&branchTable[390], 665},
	{0x0, 0,&branchTable[390], 668},
	{0x0, 0,&branchTable[390], 645},
	{0x0, 0,&branchTable[390], 643},
	{0x0, 0,&branchTable[390], 110},
	{0x0, 0,&branchTable[390], 47},
	{0x0, 0,&branchTable[390], 42},
	{0x0, 0,&branchTable[390], 41},
	{0x0, 0,&branchTable[390], 672},
	{0x1f0000, 2,&branchTable[390], -1},
	{0x1f0000, 2,&branchTable[392], -1},
	{0x0, 0,&branchTable[394], 674},
	{0x1f0000, 3,&branchTable[394], -1},
	{0x1f0000, 2,&branchTable[397], -1},
	{0x0, 0,&branchTable[399], 680},
	{0x9f0000, 5,&branchTable[399], -1},
	{0x9f0000, 3,&branchTable[404], -1},
	{0x9f0000, 4,&branchTable[407], -1},
	{0x0, 0,&branchTable[411], 68},
	{0x0, 0,&branchTable[411], 242},
	{0x0, 0,&branchTable[411], 78},
	{0x800000, 2,&branchTable[411], -1},
	{0x0, 0,&branchTable[413], 252},
	{0x0, 0,&branchTable[413], 244},
	{0x800000, 2,&branchTable[413], -1},
	{0x0, 0,&branchTable[415], 667},
	{0x0, 0,&branchTable[415], 670},
	{0x0, 0,&branchTable[415], 168},
	{0x0, 0,&branchTable[415], 174},
	{0x0, 0,&branchTable[415], 398},
	{0x800000, 2,&branchTable[415], -1},
	{0x0, 0,&branchTable[417], 161},
	{0x0, 0,&branchTable[417], 187},
	{0x0, 0,&branchTable[417], 154},
	{0x0, 0,&branchTable[417], 199},
	{0x0, 0,&branchTable[417], 137},
	{0x0, 0,&branchTable[417], 704},
	{0x0, 0,&branchTable[417], 209},
	{0x0, 0,&branchTable[417], 658},
	{0x0, 0,&branchTable[417], 143},
	{0x0, 0,&branchTable[417], 257},
	{0x0, 0,&branchTable[417], 202},
	{0x0, 0,&branchTable[417], 232},
	{0x0, 0,&branchTable[417], 260},
	{0x0, 0,&branchTable[417], 212},
	{0x0, 0,&branchTable[417], 620},
	{0x0, 0,&branchTable[417], 62},
	{0x0, 0,&branchTable[417], 371},
	{0x0, 0,&branchTable[417], 411},
	{0x0, 0,&branchTable[417], 666},
	{0x0, 0,&branchTable[417], 669},
	{0x0, 0,&branchTable[417], 517},
	{0x800000, 2,&branchTable[417], -1},
	{0x800000, 2,&branchTable[419], -1},
	{0x0, 0,&branchTable[421], 226},
	{0x800000, 2,&branchTable[421], -1},
	{0x800000, 2,&branchTable[423], -1},
	{0x800000, 2,&branchTable[425], -1},
	{0x0, 0,&branchTable[427], 190},
	{0x0, 0,&branchTable[427], 198},
	{0x0, 0,&branchTable[427], 208},
	{0x0, 0,&branchTable[427], 127},
	{0x0, 0,&branchTable[427], 117},
	{0x0, 0,&branchTable[427], 135},
	{0x0, 0,&branchTable[427], 139},
	{0x0, 0,&branchTable[427], 121},
	{0x0, 0,&branchTable[427], 123},
	{0x0, 0,&branchTable[427], 201},
	{0x0, 0,&branchTable[427], 211},
	{0x2000f000, 18,&branchTable[427], -1},
	{0xf80800, 31,&branchTable[445], -1},
	{0x0, 0,&branchTable[476], 214},
	{0x0, 0,&branchTable[476], 481},
	{0x0, 0,&branchTable[476], 497},
	{0x0, 0,&branchTable[476], 217},
	{0x0, 0,&branchTable[476], 483},
	{0x0, 0,&branchTable[476], 501},
	{0x0, 0,&branchTable[476], 391},
	{0x0, 0,&branchTable[476], 225},
	{0x0, 0,&branchTable[476], 489},
	{0x0, 0,&branchTable[476], 509},
	{0x0, 0,&branchTable[476], 505},
	{0x0, 0,&branchTable[476], 515},
	{0x0, 0,&branchTable[476], 368},
	{0x0, 0,&branchTable[476], 671},
	{0x0, 0,&branchTable[476], 370},
	{0x0, 0,&branchTable[476], 673},
	{0x0, 0,&branchTable[476], 229},
	{0x0, 0,&branchTable[476], 679},
	{0x0000f000, 16,&branchTable[476], -1},
	{0x2000f000, 22,&branchTable[492], -1},
	{0x2000f000, 6,&branchTable[514], -1},
	{0x0, 0,&branchTable[520], 553},
	{0x0, 0,&branchTable[520], 555},
	{0x0, 0,&branchTable[520], 546},
	{0x0, 0,&branchTable[520], 548},
	{0x0, 0,&branchTable[520], 467},
	{0x0, 0,&branchTable[520], 525},
	{0x0, 0,&branchTable[520], 469},
	{0x0, 0,&branchTable[520], 531},
	{0x0, 0,&branchTable[520], 551},
	{0x0, 0,&branchTable[520], 447},
	{0x0, 0,&branchTable[520], 711},
	{0x0, 0,&branchTable[520], 715},
	{0x0, 0,&branchTable[520], 703},
	{0x0, 0,&branchTable[520], 706},
	{0x0, 0,&branchTable[520], 542},
	{0x0, 0,&branchTable[520], 472},
	{0x0, 0,&branchTable[520], 529},
	{0x0, 0,&branchTable[520], 689},
	{0x0, 0,&branchTable[520], 533},
	{0x0, 0,&branchTable[520], 693},
	{0x0, 0,&branchTable[520], 709},
	{0x0, 0,&branchTable[520], 656},
	{0x0, 0,&branchTable[520], 430},
	{0x0, 0,&branchTable[520], 521},
	{0x0, 0,&branchTable[520], 179},
	{0x0, 0,&branchTable[520], 523},
	{0x0, 0,&branchTable[520], 687},
	{0x0, 0,&branchTable[520], 185},
	{0x3000000, 4,&branchTable[520], -1},
	{0x60000000, 3,&branchTable[524], -1},
	{0x80000000, 2,&branchTable[527], -1},
	{0x60000000, 4,&branchTable[529], -1},
	{0x60800000, 7,&branchTable[533], -1},
	{0x60800000, 4,&branchTable[540], -1},
	{0x0, 0,&branchTable[544], 17},
	{0x0, 0,&branchTable[544], 18},
	{0x0, 0,&branchTable[544], 6},
	{0x0, 0,&branchTable[544], 14},
	{0x0, 0,&branchTable[544], 617},
	{0x0, 0,&branchTable[544], 623},
	{0x0, 0,&branchTable[544], 24},
	{0x0, 0,&branchTable[544], 377},
	{0x0, 0,&branchTable[544], 409},
	{0x0, 0,&branchTable[544], 111},
	{0x0, 0,&branchTable[544], 378},
	{0x0, 0,&branchTable[544], 26},
	{0x0, 0,&branchTable[544], 384},
	{0x0, 0,&branchTable[544], 443},
	{0x200000, 1,&branchTable[544], -1},
	{0x0, 0,&branchTable[545], 34},
	{0x0, 0,&branchTable[545], 362},
	{0x0, 0,&branchTable[545], 115},
	{0x80000000, 2,&branchTable[545], -1},
	{0x3000000, 4,&branchTable[547], -1},
	{0x83000000, 4,&branchTable[551], -1},
	{0x0, 0,&branchTable[555], 33},
	{0x0, 0,&branchTable[555], 43},
	{0x0, 0,&branchTable[555], 49},
	{0x0, 0,&branchTable[555], 48},
	{0x0, 0,&branchTable[555], 637},
	{0x0, 0,&branchTable[555], 635},
	{0x0, 0,&branchTable[555], 32},
	{0xe0001f, 8,&branchTable[555], -1},
	{0xf00000, 4,&branchTable[563], -1},
	{0xfffc1f, 5,&branchTable[567], -1},
	{0x0, 0,&branchTable[572], 627},
	{0x0, 0,&branchTable[572], 266},
	{0x0, 0,&branchTable[572], 477},
	{0x0, 0,&branchTable[572], 46},
	{0x0, 0,&branchTable[572], 265},
	{0x0, 0,&branchTable[572], 100},
	{0x0, 0,&branchTable[572], 101},
	{0x0, 0,&branchTable[572], 102},
	{0x80000, 2,&branchTable[572], -1},
	{0x0, 0,&branchTable[574], 389},
	{0x0, 0,&branchTable[574], 633},
	{0x0, 0,&branchTable[574], 387},
	{0xf01f, 3,&branchTable[574], -1},
	{0x0, 0,&branchTable[577], 632},
	{0x70000, 1,&branchTable[577], -1},
	{0x700e0, 4,&branchTable[578], -1},
	{0x0, 0,&branchTable[582], 388},
	{0x0, 0,&branchTable[582], 264},
	{0x0, 0,&branchTable[582], 56},
	{0x0, 0,&branchTable[582], 105},
	{0x0, 0,&branchTable[582], 103},
	{0x0, 0,&branchTable[582], 270},
	{0x0, 0,&branchTable[582], 45},
	{0x0, 0,&branchTable[582], 44},
	{0x0, 0,&branchTable[582], 420},
	{0x0, 0,&branchTable[582], 113},
	{0x0, 0,&branchTable[582], 104},
	{0x80000000, 2,&branchTable[582], -1},
	{0x40e00000, 6,&branchTable[584], -1},
	{0x40e08000, 8,&branchTable[590], -1},
	{0x0, 0,&branchTable[598], 319},
	{0x40200000, 4,&branchTable[598], -1},
	{0xc0008000, 4,&branchTable[602], -1},
	{0x80a00c00, 20,&branchTable[606], -1},
	{0x80800000, 4,&branchTable[626], -1},
	{0x40e00c00, 6,&branchTable[630], -1},
	{0x600c00, 8,&branchTable[636], -1},
	{0x400000, 2,&branchTable[644], -1},
	{0xc020fc00, 26,&branchTable[646], -1},
	{0xc080f400, 13,&branchTable[672], -1},
	{0x0, 0,&branchTable[685], 320},
	{0x40000000, 2,&branchTable[685], -1},
	{0x0, 0,&branchTable[687], 342},
	{0x0, 0,&branchTable[687], 414},
	{0x0, 0,&branchTable[687], 3},
	{0xc00, 2,&branchTable[687], -1},
	{0xf000, 4,&branchTable[689], -1},
	{0xfc00, 1,&branchTable[693], -1},
	{0xc00, 2,&branchTable[694], -1},
	{0x1ff800, 3,&branchTable[696], -1},
	{0x0, 0,&branchTable[699], 93},
	{0x0, 0,&branchTable[699], 96},
	{0xc00, 2,&branchTable[699], -1},
	{0xc00, 4,&branchTable[701], -1},
	{0x0, 0,&branchTable[705], 91},
	{0x0, 0,&branchTable[705], 92},
	{0x0, 0,&branchTable[705], 661},
	{0x0, 0,&branchTable[705], 452},
	{0x0, 0,&branchTable[705], 361},
	{0x0, 0,&branchTable[705], 364},
	{0x0, 0,&branchTable[705], 28},
	{0x0, 0,&branchTable[705], 428},
	{0x0, 0,&branchTable[705], 441},
	{0x0, 0,&branchTable[705], 97},
	{0x0, 0,&branchTable[705], 98},
	{0x400, 2,&branchTable[705], -1},
	{0x0, 0,&branchTable[707], 421},
	{0x400, 2,&branchTable[707], -1},
	{0x0, 0,&branchTable[709], 419},
	{0x0, 0,&branchTable[709], 423},
	{0x0, 0,&branchTable[709], 60},
	{0x0, 0,&branchTable[709], 58},
	{0x0, 0,&branchTable[709], 367},
	{0x0, 0,&branchTable[709], 390},
	{0x80000000, 1,&branchTable[709], -1},
	{0x80000000, 1,&branchTable[710], -1},
	{0x0, 0,&branchTable[711], 488},
	{0x80000000, 1,&branchTable[711], -1},
	{0x80000000, 1,&branchTable[712], -1},
	{0x0, 0,&branchTable[713], 678},
	{0x0, 0,&branchTable[713], 473},
	{0x0, 0,&branchTable[713], 487},
	{0x0, 0,&branchTable[713], 664},
	{0x0, 0,&branchTable[713], 677},
	{0x9f0000, 4,&branchTable[713], -1},
	{0xc00, 4,&branchTable[717], -1},
	{0x80c0fc00, 8,&branchTable[721], -1},
	{0x8000fc00, 31,&branchTable[729], -1},
	{0x0, 0,&branchTable[760], 450},
	{0x0, 0,&branchTable[760], 659},
	{0x0, 0,&branchTable[760], 182},
	{0x0, 0,&branchTable[760], 188},
	{0x1000, 2,&branchTable[760], -1},
	{0x80800010, 2,&branchTable[762], -1},
	{0x8080f000, 9,&branchTable[764], -1},
	{0x0, 0,&branchTable[773], 148},
	{0x6000, 3,&branchTable[773], -1},
	{0x0, 0,&branchTable[776], 222},
	{0x168000, 5,&branchTable[776], -1},
	{0x80808017, 2,&branchTable[781], -1},
	{0x801e0000, 4,&branchTable[783], -1},
	{0x890000, 4,&branchTable[787], -1},
	{0x890000, 2,&branchTable[791], -1},
	{0x890000, 2,&branchTable[793], -1},
	{0x0, 0,&branchTable[795], 221},
	{0x890000, 4,&branchTable[795], -1},
	{0x0, 0,&branchTable[799], 166},
	{0x0, 0,&branchTable[799], 169},
	{0x0, 0,&branchTable[799], 172},
	{0x0, 0,&branchTable[799], 175},
	{0x0, 0,&branchTable[799], 451},
	{0x0, 0,&branchTable[799], 660},
	{0x0, 0,&branchTable[799], 152},
	{0x0, 0,&branchTable[799], 155},
	{0x0, 0,&branchTable[799], 159},
	{0x0, 0,&branchTable[799], 162},
	{0x0, 0,&branchTable[799], 183},
	{0x0, 0,&branchTable[799], 189},
	{0x0, 0,&branchTable[799], 146},
	{0x0, 0,&branchTable[799], 147},
	{0x818000, 4,&branchTable[799], -1},
	{0x0, 0,&branchTable[803], 149},
	{0x818000, 4,&branchTable[803], -1},
	{0x818000, 3,&branchTable[807], -1},
	{0x0, 0,&branchTable[810], 220},
	{0x0, 0,&branchTable[810], 119},
	{0x0, 0,&branchTable[810], 233},
	{0x0, 0,&branchTable[810], 261},
	{0x0, 0,&branchTable[810], 249},
	{0x0, 0,&branchTable[810], 251},
	{0x0, 0,&branchTable[810], 247},
	{0x0, 0,&branchTable[810], 255},
	{0x0, 0,&branchTable[810], 243},
	{0x0, 0,&branchTable[810], 253},
	{0x0, 0,&branchTable[810], 245},
	{0x0, 0,&branchTable[810], 128},
	{0x0, 0,&branchTable[810], 129},
	{0x0, 0,&branchTable[810], 227},
	{0x0, 0,&branchTable[810], 191},
	{0x0, 0,&branchTable[810], 125},
	{0x0, 0,&branchTable[810], 263},
	{0x0, 0,&branchTable[810], 194},
	{0x0, 0,&branchTable[810], 204},
	{0x0, 0,&branchTable[810], 196},
	{0x0, 0,&branchTable[810], 206},
	{0x0, 0,&branchTable[810], 236},
	{0x0, 0,&branchTable[810], 455},
	{0x0, 0,&branchTable[810], 106},
	{0x0, 0,&branchTable[810], 458},
	{0x0, 0,&branchTable[810], 457},
	{0x0, 0,&branchTable[810], 459},
	{0x0, 0,&branchTable[810], 462},
	{0x0, 0,&branchTable[810], 461},
	{0x0, 0,&branchTable[810], 464},
	{0x0, 0,&branchTable[810], 456},
	{0x0, 0,&branchTable[810], 494},
	{0x0, 0,&branchTable[810], 460},
	{0x0, 0,&branchTable[810], 463},
	{0x0, 0,&branchTable[810], 534},
	{0x0, 0,&branchTable[810], 69},
	{0x0, 0,&branchTable[810], 625},
	{0x0, 0,&branchTable[810], 65},
	{0x0, 0,&branchTable[810], 549},
	{0x0, 0,&branchTable[810], 536},
	{0x0, 0,&branchTable[810], 526},
	{0x0, 0,&branchTable[810], 543},
	{0x0, 0,&branchTable[810], 518},
	{0x0, 0,&branchTable[810], 492},
	{0x0, 0,&branchTable[810], 8},
	{0x0, 0,&branchTable[810], 71},
	{0x0, 0,&branchTable[810], 87},
	{0x0, 0,&branchTable[810], 498},
	{0x0, 0,&branchTable[810], 63},
	{0x1f0000, 2,&branchTable[810], -1},
	{0x0, 0,&branchTable[812], 502},
	{0x0, 0,&branchTable[812], 506},
	{0x1f0000, 3,&branchTable[812], -1},
	{0x9f0000, 2,&branchTable[815], -1},
	{0x0, 0,&branchTable[817], 510},
	{0x9f0000, 3,&branchTable[817], -1},
	{0x0, 0,&branchTable[820], 230},
	{0x0, 0,&branchTable[820], 130},
	{0x0, 0,&branchTable[820], 144},
	{0x0, 0,&branchTable[820], 241},
	{0x800000, 2,&branchTable[820], -1},
	{0x0, 0,&branchTable[822], 79},
	{0x800000, 2,&branchTable[822], -1},
	{0x0, 0,&branchTable[824], 164},
	{0x0, 0,&branchTable[824], 170},
	{0x0, 0,&branchTable[824], 1},
	{0x800000, 2,&branchTable[824], -1},
	{0x0, 0,&branchTable[826], 11},
	{0x0, 0,&branchTable[826], 157},
	{0x0, 0,&branchTable[826], 180},
	{0x0, 0,&branchTable[826], 150},
	{0x0, 0,&branchTable[826], 140},
	{0x0, 0,&branchTable[826], 448},
	{0x0, 0,&branchTable[826], 132},
	{0x0, 0,&branchTable[826], 237},
	{0x0, 0,&branchTable[826], 239},
	{0x0, 0,&branchTable[826], 258},
	{0xa00000, 2,&branchTable[826], -1},
	{0xa00000, 2,&branchTable[828], -1},
	{0x7400, 10,&branchTable[830], -1},
	{0x7400, 7,&branchTable[840], -1},
	{0x0, 0,&branchTable[847], 192},
	{0x0, 0,&branchTable[847], 234},
	{0x0, 0,&branchTable[847], 223},
	{0x0, 0,&branchTable[847], 235},
	{0x0, 0,&branchTable[847], 552},
	{0x0, 0,&branchTable[847], 213},
	{0x0, 0,&branchTable[847], 554},
	{0x0, 0,&branchTable[847], 545},
	{0x0, 0,&branchTable[847], 496},
	{0x0, 0,&branchTable[847], 547},
	{0x0, 0,&branchTable[847], 216},
	{0x0, 0,&branchTable[847], 466},
	{0x0, 0,&branchTable[847], 500},
	{0x0, 0,&branchTable[847], 524},
	{0x0, 0,&branchTable[847], 224},
	{0x800800, 2,&branchTable[847], -1},
	{0x0, 0,&branchTable[849], 508},
	{0x0, 0,&branchTable[849], 504},
	{0x0, 0,&branchTable[849], 514},
	{0x0, 0,&branchTable[849], 446},
	{0x0, 0,&branchTable[849], 178},
	{0x0, 0,&branchTable[849], 530},
	{0x0, 0,&branchTable[849], 520},
	{0x40400000, 4,&branchTable[849], -1},
	{0x40400000, 4,&branchTable[853], -1},
	{0x40400000, 4,&branchTable[857], -1},
	{0x40400000, 4,&branchTable[861], -1},
	{0x40400000, 4,&branchTable[865], -1},
	{0x40000000, 2,&branchTable[869], -1},
	{0x40000000, 2,&branchTable[871], -1},
	{0x40000000, 2,&branchTable[873], -1},
	{0x40000000, 2,&branchTable[875], -1},
	{0x40000000, 2,&branchTable[877], -1},
	{0x400000, 2,&branchTable[879], -1},
	{0x400000, 2,&branchTable[881], -1},
	{0x400000, 2,&branchTable[883], -1},
	{0x400000, 2,&branchTable[885], -1},
	{0x400000, 2,&branchTable[887], -1},
	{0x40400000, 2,&branchTable[889], -1},
	{0x0, 0,&branchTable[891], 339},
	{0x0, 0,&branchTable[891], 349},
	{0x0, 0,&branchTable[891], 340},
	{0x40400000, 2,&branchTable[891], -1},
	{0x0, 0,&branchTable[893], 610},
	{0x0, 0,&branchTable[893], 352},
	{0x0, 0,&branchTable[893], 611},
	{0x0, 0,&branchTable[893], 353},
	{0x0, 0,&branchTable[893], 597},
	{0x0, 0,&branchTable[893], 323},
	{0x0, 0,&branchTable[893], 601},
	{0x0, 0,&branchTable[893], 327},
	{0x0, 0,&branchTable[893], 606},
	{0x0, 0,&branchTable[893], 345},
	{0x0, 0,&branchTable[893], 607},
	{0x0, 0,&branchTable[893], 346},
	{0x0, 0,&branchTable[893], 598},
	{0x0, 0,&branchTable[893], 324},
	{0x0, 0,&branchTable[893], 602},
	{0x0, 0,&branchTable[893], 328},
	{0x0, 0,&branchTable[893], 600},
	{0x0, 0,&branchTable[893], 326},
	{0x0, 0,&branchTable[893], 604},
	{0x0, 0,&branchTable[893], 330},
	{0x0, 0,&branchTable[893], 354},
	{0x0, 0,&branchTable[893], 355},
	{0x0, 0,&branchTable[893], 331},
	{0x0, 0,&branchTable[893], 335},
	{0x0, 0,&branchTable[893], 347},
	{0x0, 0,&branchTable[893], 348},
	{0x0, 0,&branchTable[893], 332},
	{0x0, 0,&branchTable[893], 336},
	{0x0, 0,&branchTable[893], 334},
	{0x0, 0,&branchTable[893], 338},
	{0x0, 0,&branchTable[893], 609},
	{0x0, 0,&branchTable[893], 351},
	{0x0, 0,&branchTable[893], 592},
	{0x0, 0,&branchTable[893], 316},
	{0x0, 0,&branchTable[893], 605},
	{0x0, 0,&branchTable[893], 344},
	{0x0, 0,&branchTable[893], 593},
	{0x0, 0,&branchTable[893], 317},
	{0x0, 0,&branchTable[893], 596},
	{0x0, 0,&branchTable[893], 322},
	{0x0, 0,&branchTable[893], 356},
	{0x0, 0,&branchTable[893], 416},
	{0x0, 0,&branchTable[893], 343},
	{0x0, 0,&branchTable[893], 415},
	{0x40400000, 4,&branchTable[893], -1},
	{0x40000000, 2,&branchTable[897], -1},
	{0x400000, 2,&branchTable[899], -1},
	{0x40400000, 2,&branchTable[901], -1},
	{0x0, 0,&branchTable[903], 599},
	{0x0, 0,&branchTable[903], 325},
	{0x0, 0,&branchTable[903], 603},
	{0x0, 0,&branchTable[903], 329},
	{0x0, 0,&branchTable[903], 333},
	{0x0, 0,&branchTable[903], 337},
	{0x0, 0,&branchTable[903], 594},
	{0x0, 0,&branchTable[903], 318},
	{0x0, 0,&branchTable[903], 341},
	{0x0, 0,&branchTable[903], 413},
	{0x0, 0,&branchTable[903], 4},
	{0x0, 0,&branchTable[903], 51},
	{0x0, 0,&branchTable[903], 50},
	{0xf000, 1,&branchTable[903], -1},
	{0x0, 0,&branchTable[904], 53},
	{0x0, 0,&branchTable[904], 52},
	{0x0, 0,&branchTable[904], 442},
	{0x0, 0,&branchTable[904], 608},
	{0x0, 0,&branchTable[904], 589},
	{0x0, 0,&branchTable[904], 590},
	{0x0, 0,&branchTable[904], 595},
	{0x0, 0,&branchTable[904], 350},
	{0x0, 0,&branchTable[904], 313},
	{0x0, 0,&branchTable[904], 314},
	{0x0, 0,&branchTable[904], 321},
	{0x0, 0,&branchTable[904], 591},
	{0x0, 0,&branchTable[904], 315},
	{0x0, 0,&branchTable[904], 682},
	{0x0, 0,&branchTable[904], 538},
	{0x0, 0,&branchTable[904], 694},
	{0x0, 0,&branchTable[904], 73},
	{0x0, 0,&branchTable[904], 712},
	{0x0, 0,&branchTable[904], 75},
	{0x0, 0,&branchTable[904], 707},
	{0x0, 0,&branchTable[904], 696},
	{0x0, 0,&branchTable[904], 690},
	{0x0, 0,&branchTable[904], 700},
	{0x0, 0,&branchTable[904], 684},
	{0x0, 0,&branchTable[904], 176},
	{0x0, 0,&branchTable[904], 512},
	{0x0, 0,&branchTable[904], 619},
	{0x0, 0,&branchTable[904], 67},
	{0x0, 0,&branchTable[904], 61},
	{0x0, 0,&branchTable[904], 77},
	{0x9f0000, 2,&branchTable[904], -1},
	{0x0, 0,&branchTable[906], 516},
	{0x1f0000, 2,&branchTable[906], -1},
	{0x9f0000, 4,&branchTable[908], -1},
	{0x0, 0,&branchTable[912], 116},
	{0x9f0000, 4,&branchTable[912], -1},
	{0x800000, 2,&branchTable[916], -1},
	{0x800000, 2,&branchTable[918], -1},
	{0x9f0000, 2,&branchTable[920], -1},
	{0x0, 0,&branchTable[922], 167},
	{0x0, 0,&branchTable[922], 173},
	{0x0, 0,&branchTable[922], 397},
	{0x800000, 2,&branchTable[922], -1},
	{0x0, 0,&branchTable[924], 160},
	{0x0, 0,&branchTable[924], 186},
	{0x0, 0,&branchTable[924], 153},
	{0x0, 0,&branchTable[924], 197},
	{0x0, 0,&branchTable[924], 136},
	{0x0, 0,&branchTable[924], 207},
	{0x0, 0,&branchTable[924], 657},
	{0x0, 0,&branchTable[924], 126},
	{0x0, 0,&branchTable[924], 142},
	{0x0, 0,&branchTable[924], 256},
	{0x0, 0,&branchTable[924], 134},
	{0x0, 0,&branchTable[924], 138},
	{0x0, 0,&branchTable[924], 120},
	{0x0, 0,&branchTable[924], 122},
	{0x0, 0,&branchTable[924], 200},
	{0x0, 0,&branchTable[924], 210},
	{0x0, 0,&branchTable[924], 710},
	{0x0, 0,&branchTable[924], 714},
	{0x0, 0,&branchTable[924], 702},
	{0x0, 0,&branchTable[924], 705},
	{0x0, 0,&branchTable[924], 541},
	{0x0, 0,&branchTable[924], 471},
	{0x0, 0,&branchTable[924], 528},
	{0x0, 0,&branchTable[924], 688},
	{0x800, 2,&branchTable[924], -1},
	{0x800, 2,&branchTable[926], -1},
	{0x0, 0,&branchTable[928], 655},
	{0x0, 0,&branchTable[928], 184},
	{0x0, 0,&branchTable[928], 228},
	{0x0, 0,&branchTable[928], 532},
	{0x0, 0,&branchTable[928], 522},
	{0x0, 0,&branchTable[928], 692},
	{0x0, 0,&branchTable[928], 686},
	{0x20000000, 2,&branchTable[928], -1},
	{0x20000000, 2,&branchTable[930], -1},
	{0x0, 0,&branchTable[932], 383},
	{0x0, 0,&branchTable[932], 219},
	{0x0, 0,&branchTable[932], 407},
	{0x0, 0,&branchTable[932], 37},
	{0x0, 0,&branchTable[932], 396},
	{0x0, 0,&branchTable[932], 273},
	{0x0, 0,&branchTable[932], 275},
	{0x0, 0,&branchTable[932], 274},
	{0x0, 0,&branchTable[932], 276},
	{0x0, 0,&branchTable[932], 279},
	{0x0, 0,&branchTable[932], 281},
	{0x0, 0,&branchTable[932], 280},
	{0x0, 0,&branchTable[932], 282},
	{0x0, 0,&branchTable[932], 285},
	{0x0, 0,&branchTable[932], 287},
	{0x0, 0,&branchTable[932], 286},
	{0x0, 0,&branchTable[932], 288},
	{0x0, 0,&branchTable[932], 291},
	{0x0, 0,&branchTable[932], 293},
	{0x0, 0,&branchTable[932], 292},
	{0x0, 0,&branchTable[932], 294},
};  // end main_decoder_table

MachRegister InstructionDecoder_aarch64::sysRegMap(unsigned int m) {
    switch(m) {
    case 0x741d: return aarch64::tlbi_vale3is;
    case 0xc021: return aarch64::id_aa64pfr1_el1;
    case 0xf089: return aarch64::sder32_el3;
    case 0xf664: return aarch64::icc_ctlr_el3;
    case 0xe180: return aarch64::dacr32_el2;
    case 0xe659: return aarch64::ich_vtr_el2;
    case 0xc659: return aarch64::icc_dir_el1;
    case 0x6425: return aarch64::tlbi_ipas2le1;
    case 0xda20: return aarch64::fpcr;
    case 0xf288: return aarch64::afsr0_el3;
    case 0xd807: return aarch64::dczid_el0;
    case 0xc081: return aarch64::actlr_el1;
    case 0xf080: return aarch64::sctlr_el3;
    case 0xe080: return aarch64::sctlr_el2;
    case 0x63c0: return aarch64::at_s1e2r;
    case 0x43a8: return aarch64::ic_iallu;
    case 0xf667: return aarch64::icc_igrpen1_el3;
    case 0xdce6: return aarch64::pmceid0_el0;
    case 0xde82: return aarch64::tpidr_el0;
    case 0x6405: return aarch64::tlbi_ipas2le1is;
    case 0xc681: return aarch64::contextidr_el1;
    case 0xc00e: return aarch64::id_mmfr2_el1;
    case 0xdf60: return aarch64::pmevtyper0_el0;
    case 0xdf61: return aarch64::pmevtyper1_el0;
    case 0xdf62: return aarch64::pmevtyper2_el0;
    case 0xdf63: return aarch64::pmevtyper3_el0;
    case 0xdf64: return aarch64::pmevtyper4_el0;
    case 0xdf65: return aarch64::pmevtyper5_el0;
    case 0xdf66: return aarch64::pmevtyper6_el0;
    case 0xdf67: return aarch64::pmevtyper7_el0;
    case 0xdf68: return aarch64::pmevtyper8_el0;
    case 0xdf69: return aarch64::pmevtyper9_el0;
    case 0xdf6a: return aarch64::pmevtyper10_el0;
    case 0xdf6b: return aarch64::pmevtyper11_el0;
    case 0xdf6c: return aarch64::pmevtyper12_el0;
    case 0xdf6d: return aarch64::pmevtyper13_el0;
    case 0xdf6e: return aarch64::pmevtyper14_el0;
    case 0xdf6f: return aarch64::pmevtyper15_el0;
    case 0xdf70: return aarch64::pmevtyper16_el0;
    case 0xdf71: return aarch64::pmevtyper17_el0;
    case 0xdf72: return aarch64::pmevtyper18_el0;
    case 0xdf73: return aarch64::pmevtyper19_el0;
    case 0xdf74: return aarch64::pmevtyper20_el0;
    case 0xdf75: return aarch64::pmevtyper21_el0;
    case 0xdf76: return aarch64::pmevtyper22_el0;
    case 0xdf77: return aarch64::pmevtyper23_el0;
    case 0xdf78: return aarch64::pmevtyper24_el0;
    case 0xdf79: return aarch64::pmevtyper25_el0;
    case 0xdf7a: return aarch64::pmevtyper26_el0;
    case 0xdf7b: return aarch64::pmevtyper27_el0;
    case 0xdf7c: return aarch64::pmevtyper28_el0;
    case 0xdf7d: return aarch64::pmevtyper29_el0;
    case 0xdf7e: return aarch64::pmevtyper30_el0;
    case 0xc510: return aarch64::mair_el1;
    case 0x641d: return aarch64::tlbi_vale2is;
    case 0xc800: return aarch64::ccsidr_el1;
    case 0xe218: return aarch64::spsr_irq;
    case 0xf201: return aarch64::elr_el3;
    case 0x7418: return aarch64::tlbi_alle3is;
    case 0xc208: return aarch64::sp_el0;
    case 0xc00d: return aarch64::id_mmfr1_el1;
    case 0x5ba9: return aarch64::ic_ivau;
    case 0xc641: return aarch64::icc_eoir0_el1;
    case 0xc65f: return aarch64::icc_sgi0r_el1;
    case 0xc65d: return aarch64::icc_sgi1r_el1;
    case 0x6439: return aarch64::tlbi_vae2;
    case 0xdf11: return aarch64::cntp_ctl_el0;
    case 0xc684: return aarch64::tpidr_el1;
    case 0x5bf1: return aarch64::dc_civac;
    case 0x6401: return aarch64::tlbi_ipas2e1is;
    case 0xdf1a: return aarch64::cntv_cval_el0;
    case 0xc643: return aarch64::icc_bpr0_el1;
    case 0x9808: return aarch64::mdccsr_el0;
    case 0x83c6: return aarch64::dbgclaimset_el1;
    case 0xe640: return aarch64::ich_ap0r0_el2;
    case 0xe641: return aarch64::ich_ap0r1_el2;
    case 0xe642: return aarch64::ich_ap0r2_el2;
    case 0xe643: return aarch64::ich_ap0r3_el2;
    case 0x43b1: return aarch64::dc_ivac;
    case 0xe65d: return aarch64::ich_elrsr_el2;
    case 0xe658: return aarch64::ich_hcr_el2;
    case 0xe600: return aarch64::vbar_el2;
    case 0xc65e: return aarch64::icc_asgi1r_el1;
    case 0xc02d: return aarch64::id_aa64afr1_el1;
    case 0xda10: return aarch64::nzcv;
    case 0xc100: return aarch64::ttbr0_el1;
    case 0xf081: return aarch64::actlr_el3;
    case 0x8080: return aarch64::mdrar_el1;
    case 0xf602: return aarch64::rmr_el3;
    case 0xc708: return aarch64::cntkctl_el1;
    case 0xf601: return aarch64::rvbar_el3;
    case 0xc00a: return aarch64::id_dfr0_el1;
    case 0x43c0: return aarch64::at_s1e1r;
    case 0xc648: return aarch64::icc_ap1r0_el1;
    case 0xc649: return aarch64::icc_ap1r1_el1;
    case 0xc64a: return aarch64::icc_ap1r2_el1;
    case 0xc64b: return aarch64::icc_ap1r3_el1;
    case 0xc230: return aarch64::icc_pmr_el1;
    case 0xdce7: return aarch64::pmceid1_el0;
    case 0x43d2: return aarch64::dc_csw;
    case 0xc009: return aarch64::id_pfr1_el1;
    case 0xc00b: return aarch64::id_afr0_el1;
    case 0xe518: return aarch64::amair_el2;
    case 0xdcf3: return aarch64::pmovsset_el0;
    case 0x4438: return aarch64::tlbi_vmalle1;
    case 0x643d: return aarch64::tlbi_vale2;
    case 0x63c7: return aarch64::at_s12e0w;
    case 0xc015: return aarch64::id_isar5_el1;
    case 0xc102: return aarch64::tcr_el1;
    case 0xdf40: return aarch64::pmevcntr0_el0;
    case 0xdf41: return aarch64::pmevcntr1_el0;
    case 0xdf42: return aarch64::pmevcntr2_el0;
    case 0xdf43: return aarch64::pmevcntr3_el0;
    case 0xdf44: return aarch64::pmevcntr4_el0;
    case 0xdf45: return aarch64::pmevcntr5_el0;
    case 0xdf46: return aarch64::pmevcntr6_el0;
    case 0xdf47: return aarch64::pmevcntr7_el0;
    case 0xdf48: return aarch64::pmevcntr8_el0;
    case 0xdf49: return aarch64::pmevcntr9_el0;
    case 0xdf4a: return aarch64::pmevcntr10_el0;
    case 0xdf4b: return aarch64::pmevcntr11_el0;
    case 0xdf4c: return aarch64::pmevcntr12_el0;
    case 0xdf4d: return aarch64::pmevcntr13_el0;
    case 0xdf4e: return aarch64::pmevcntr14_el0;
    case 0xdf4f: return aarch64::pmevcntr15_el0;
    case 0xdf50: return aarch64::pmevcntr16_el0;
    case 0xdf51: return aarch64::pmevcntr17_el0;
    case 0xdf52: return aarch64::pmevcntr18_el0;
    case 0xdf53: return aarch64::pmevcntr19_el0;
    case 0xdf54: return aarch64::pmevcntr20_el0;
    case 0xdf55: return aarch64::pmevcntr21_el0;
    case 0xdf56: return aarch64::pmevcntr22_el0;
    case 0xdf57: return aarch64::pmevcntr23_el0;
    case 0xdf58: return aarch64::pmevcntr24_el0;
    case 0xdf59: return aarch64::pmevcntr25_el0;
    case 0xdf5a: return aarch64::pmevcntr26_el0;
    case 0xdf5b: return aarch64::pmevcntr27_el0;
    case 0xdf5c: return aarch64::pmevcntr28_el0;
    case 0xdf5d: return aarch64::pmevcntr29_el0;
    case 0xdf5e: return aarch64::pmevcntr30_el0;
    case 0x9820: return aarch64::dbgdtr_el0;
    case 0xf518: return aarch64::amair_el3;
    case 0xc601: return aarch64::rvbar_el1;
    case 0x5bd9: return aarch64::dc_cvau;
    case 0xe288: return aarch64::afsr0_el2;
    case 0xc640: return aarch64::icc_iar0_el1;
    case 0xc663: return aarch64::icc_bpr1_el1;
    case 0x43f2: return aarch64::dc_cisw;
    case 0xe304: return aarch64::hpfar_el2;
    case 0xe219: return aarch64::spsr_abt;
    case 0xd000: return aarch64::csselr_el1;
    case 0xc4f2: return aarch64::pmintenclr_el1;
    case 0x443b: return aarch64::tlbi_vaae1;
    case 0xe300: return aarch64::far_el2;
    case 0xc212: return aarch64::currentel;
    case 0x80a4: return aarch64::dbgprcr_el1;
    case 0x6421: return aarch64::tlbi_ipas2e1;
    case 0xf510: return aarch64::mair_el3;
    case 0xc101: return aarch64::ttbr1_el1;
    case 0xdf02: return aarch64::cntvct_el0;
    case 0x441a: return aarch64::tlbi_aside1is;
    case 0xf088: return aarch64::scr_el3;
    case 0xc018: return aarch64::mvfr0_el1;
    case 0x643c: return aarch64::tlbi_alle1;
    case 0xdce3: return aarch64::pmovsclr_el0;
    case 0xc019: return aarch64::mvfr1_el1;
    case 0x443f: return aarch64::tlbi_vaale1;
    case 0xc201: return aarch64::elr_el1;
    case 0xe601: return aarch64::rvbar_el2;
    case 0xc290: return aarch64::esr_el1;
    case 0x443a: return aarch64::tlbi_aside1;
    case 0xdce1: return aarch64::pmcntenset_el0;
    case 0xda28: return aarch64::dspsr_el0;
    case 0x5ba1: return aarch64::dc_zva;
    case 0xe65a: return aarch64::ich_misr_el2;
    case 0xc600: return aarch64::vbar_el1;
    case 0xc011: return aarch64::id_isar1_el1;
    case 0xc082: return aarch64::cpacr_el1;
    case 0xc3a0: return aarch64::par_el1;
    case 0xe711: return aarch64::cnthp_ctl_el2;
    case 0xe081: return aarch64::actlr_el2;
    case 0xe21b: return aarch64::spsr_fiq;
    case 0xc020: return aarch64::id_aa64pfr0_el1;
    case 0xf682: return aarch64::tpidr_el3;
    case 0x63c1: return aarch64::at_s1e2w;
    case 0x6438: return aarch64::tlbi_alle2;
    case 0xc014: return aarch64::id_isar4_el1;
    case 0xa038: return aarch64::dbgvcr32_el2;
    case 0xe088: return aarch64::hcr_el2;
    case 0xe65b: return aarch64::ich_eisr_el2;
    case 0x8010: return aarch64::mdccint_el1;
    case 0xda29: return aarch64::dlr_el0;
    case 0xe682: return aarch64::tpidr_el2;
    case 0xdcf0: return aarch64::pmuserenr_el0;
    case 0x8032: return aarch64::oseccr_el1;
    case 0x641e: return aarch64::tlbi_vmalls12e1is;
    case 0x8012: return aarch64::mdscr_el1;
    case 0xe65f: return aarch64::ich_vmcr_el2;
    case 0xf300: return aarch64::far_el3;
    case 0xe200: return aarch64::spsr_el2;
    case 0x808c: return aarch64::oslsr_el1;
    case 0x4439: return aarch64::tlbi_vae1;
    case 0xda11: return aarch64::daif;
    case 0xc300: return aarch64::far_el1;
    case 0xe08b: return aarch64::hstr_el2;
    case 0xc801: return aarch64::clidr_el1;
    case 0xe108: return aarch64::vttbr_el2;
    case 0xc02c: return aarch64::id_aa64afr0_el1;
    case 0xc65b: return aarch64::icc_rpr_el1;
    case 0xff11: return aarch64::cntps_ctl_el1;
    case 0xc289: return aarch64::afsr1_el1;
    case 0xc210: return aarch64::spsel;
    case 0xda21: return aarch64::fpsr;
    case 0xdce0: return aarch64::pmcr_el0;
    case 0x441b: return aarch64::tlbi_vaae1is;
    case 0xc665: return aarch64::icc_sre_el1;
    case 0xdf01: return aarch64::cntpct_el0;
    case 0x6418: return aarch64::tlbi_alle2is;
    case 0xc667: return aarch64::icc_igrpen1_el1;
    case 0xe281: return aarch64::ifsr32_el2;
    case 0xc666: return aarch64::icc_igrpen0_el1;
    case 0xde83: return aarch64::tpidrro_el0;
    case 0xe08f: return aarch64::hacr_el2;
    case 0x43c3: return aarch64::at_s1e0w;
    case 0xdf7f: return aarch64::pmccfiltr_el0;
    case 0xe710: return aarch64::cnthp_tval_el2;
    case 0x63c6: return aarch64::at_s12e0r;
    case 0xe703: return aarch64::cntvoff_el2;
    case 0xe64d: return aarch64::icc_sre_el2;
    case 0xc200: return aarch64::spsr_el1;
    case 0xc608: return aarch64::isr_el1;
    case 0xe708: return aarch64::cnthctl_el2;
    case 0xe10a: return aarch64::vtcr_el2;
    case 0xe005: return aarch64::vmpidr_el2;
    case 0xdce4: return aarch64::pmswinc_el0;
    case 0xc00c: return aarch64::id_mmfr0_el1;
    case 0xdf12: return aarch64::cntp_cval_el0;
    case 0xc642: return aarch64::icc_hppir0_el1;
    case 0xf099: return aarch64::mdcr_el3;
    case 0x83f6: return aarch64::dbgauthstatus_el1;
    case 0xc661: return aarch64::icc_eoir1_el1;
    case 0xe201: return aarch64::elr_el2;
    case 0x4388: return aarch64::ic_ialluis;
    case 0xf102: return aarch64::tcr_el3;
    case 0xc807: return aarch64::aidr_el1;
    case 0xdf00: return aarch64::cntfrq_el0;
    case 0x7439: return aarch64::tlbi_vae3;
    case 0x643e: return aarch64::tlbi_vmalls12e1;
    case 0xe290: return aarch64::esr_el2;
    case 0xf600: return aarch64::vbar_el3;
    case 0xc602: return aarch64::rmr_el1;
    case 0xc000: return aarch64::midr_el1;
    case 0x43b2: return aarch64::dc_isw;
    case 0xc010: return aarch64::id_isar0_el1;
    case 0xd801: return aarch64::ctr_el0;
    case 0xe208: return aarch64::sp_el1;
    case 0xdf18: return aarch64::cntv_tval_el0;
    case 0x7419: return aarch64::tlbi_vae3is;
    case 0xf289: return aarch64::afsr1_el3;
    case 0xc016: return aarch64::id_mmfr4_el1;
    case 0xf100: return aarch64::ttbr0_el3;
    case 0xe21a: return aarch64::spsr_und;
    case 0x6419: return aarch64::tlbi_vae2is;
    case 0x73c1: return aarch64::at_s1e3w;
    case 0x809c: return aarch64::osdlr_el1;
    case 0xe660: return aarch64::ich_lr0_el2;
    case 0xe661: return aarch64::ich_lr1_el2;
    case 0xe662: return aarch64::ich_lr2_el2;
    case 0xe663: return aarch64::ich_lr3_el2;
    case 0xe664: return aarch64::ich_lr4_el2;
    case 0xe665: return aarch64::ich_lr5_el2;
    case 0xe666: return aarch64::ich_lr6_el2;
    case 0xe667: return aarch64::ich_lr7_el2;
    case 0xe668: return aarch64::ich_lr8_el2;
    case 0xe669: return aarch64::ich_lr9_el2;
    case 0xe66a: return aarch64::ich_lr10_el2;
    case 0xe66b: return aarch64::ich_lr11_el2;
    case 0xe66c: return aarch64::ich_lr12_el2;
    case 0xe66d: return aarch64::ich_lr13_el2;
    case 0xe66e: return aarch64::ich_lr14_el2;
    case 0xe66f: return aarch64::ich_lr15_el2;
    case 0x83ce: return aarch64::dbgclaimclr_el1;
    case 0x4418: return aarch64::tlbi_vmalle1is;
    case 0xc039: return aarch64::id_aa64mmfr1_el1;
    case 0xdce8: return aarch64::pmccntr_el0;
    case 0xf290: return aarch64::esr_el3;
    case 0xf08a: return aarch64::cptr_el3;
    case 0xdf10: return aarch64::cntp_tval_el0;
    case 0xe102: return aarch64::tcr_el2;
    case 0x5bd1: return aarch64::dc_cvac;
    case 0x441f: return aarch64::tlbi_vaale1is;
    case 0xc4f1: return aarch64::pmintenset_el1;
    case 0x8000: return aarch64::dbgbcr0_el1;
    case 0x8001: return aarch64::dbgbcr1_el1;
    case 0x8002: return aarch64::dbgbcr2_el1;
    case 0x8003: return aarch64::dbgbcr3_el1;
    case 0x8004: return aarch64::dbgbcr4_el1;
    case 0x8005: return aarch64::dbgbcr5_el1;
    case 0x8006: return aarch64::dbgbcr6_el1;
    case 0x8007: return aarch64::dbgbcr7_el1;
    case 0x8008: return aarch64::dbgbcr8_el1;
    case 0x8009: return aarch64::dbgbcr9_el1;
    case 0x800a: return aarch64::dbgbcr10_el1;
    case 0x800b: return aarch64::dbgbcr11_el1;
    case 0x800c: return aarch64::dbgbcr12_el1;
    case 0x800d: return aarch64::dbgbcr13_el1;
    case 0x800e: return aarch64::dbgbcr14_el1;
    case 0x800f: return aarch64::dbgbcr15_el1;
    case 0xdce5: return aarch64::pmselr_el0;
    case 0x641c: return aarch64::tlbi_alle1is;
    case 0xc662: return aarch64::icc_hppir1_el1;
    case 0x4419: return aarch64::tlbi_vae1is;
    case 0xe089: return aarch64::mdcr_el2;
    case 0x63c5: return aarch64::at_s12e1w;
    case 0xe000: return aarch64::vpidr_el2;
    case 0xc028: return aarch64::id_aa64dfr0_el1;
    case 0xc012: return aarch64::id_isar2_el1;
    case 0xe100: return aarch64::ttbr0_el2;
    case 0xc644: return aarch64::icc_ap0r0_el1;
    case 0xc645: return aarch64::icc_ap0r1_el1;
    case 0xc646: return aarch64::icc_ap0r2_el1;
    case 0xc647: return aarch64::icc_ap0r3_el1;
    case 0x9828: return aarch64::dbgdtrrx_el0;
    case 0xe298: return aarch64::fpexc32_el2;
    case 0xdce9: return aarch64::pmxevtyper_el0;
    case 0x8084: return aarch64::oslar_el1;
    case 0xc008: return aarch64::id_pfr0_el1;
    case 0xc00f: return aarch64::id_mmfr3_el1;
    case 0x801a: return aarch64::osdtrtx_el1;
    case 0xc030: return aarch64::id_aa64isar0_el1;
    case 0xc01a: return aarch64::mvfr2_el1;
    case 0xdce2: return aarch64::pmcntenclr_el0;
    case 0xe648: return aarch64::ich_ap1r0_el2;
    case 0xe649: return aarch64::ich_ap1r1_el2;
    case 0xe64a: return aarch64::ich_ap1r2_el2;
    case 0xe64b: return aarch64::ich_ap1r3_el2;
    case 0xc038: return aarch64::id_aa64mmfr0_el1;
    case 0x743d: return aarch64::tlbi_vale3;
    case 0xc006: return aarch64::revidr_el1;
    case 0xe510: return aarch64::mair_el2;
    case 0xc080: return aarch64::sctlr_el1;
    case 0x43c2: return aarch64::at_s1e0r;
    case 0xe08a: return aarch64::cptr_el2;
    case 0x63c4: return aarch64::at_s12e1r;
    case 0xdcea: return aarch64::pmxevcntr_el0;
    case 0xc288: return aarch64::afsr0_el1;
    case 0x43c1: return aarch64::at_s1e1w;
    case 0xc013: return aarch64::id_isar3_el1;
    case 0xe289: return aarch64::afsr1_el2;
    case 0x441d: return aarch64::tlbi_vale1is;
    case 0xf208: return aarch64::sp_el2;
    case 0xff12: return aarch64::cntps_cval_el1;
    case 0xff10: return aarch64::cntps_tval_el1;
    case 0xc664: return aarch64::icc_ctlr_el1;
    case 0xdf19: return aarch64::cntv_ctl_el0;
    case 0xe602: return aarch64::rmr_el2;
    case 0xc029: return aarch64::id_aa64dfr1_el1;
    case 0xe712: return aarch64::cnthp_cval_el2;
    case 0x73c0: return aarch64::at_s1e3r;
    case 0xc031: return aarch64::id_aa64isar1_el1;
    case 0xc005: return aarch64::mpidr_el1;
    case 0xc518: return aarch64::amair_el1;
    case 0x7438: return aarch64::tlbi_alle3;
    case 0xf200: return aarch64::spsr_el3;
    case 0x443d: return aarch64::tlbi_vale1;
    case 0xf665: return aarch64::icc_sre_el3;
    case 0xc660: return aarch64::icc_iar1_el1;
    default: assert(!"tried to access system register not accessible in EL0");
    };
}  // end sysRegMap
