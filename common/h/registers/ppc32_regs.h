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

#ifndef DYNINST_PPC32_REGS_H
#define DYNINST_PPC32_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"

namespace Dyninst { namespace ppc32 {
  /**
   * For interpreting constants:
   *  Lowest 16 bits (0x000000ff) is base register ID
   *  Next 16 bits (0x0000ff00) is the aliasing and subrange ID-
   *    used on x86/x86_64 to distinguish between things like EAX and AH
   *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/MMX/...
   *  Top 16 bits (0xff000000) are the architecture.
   *
   *  These values/layout are not guaranteed to remain the same as part of the
   *  public interface, and may change.
   **/

  const signed int GPR = 0x00010000;
  const signed int FPR = 0x00020000;
  const signed int FSR = 0x00040000;
  const signed int SPR = 0x00080000;

  //          (      name,  ID | cat |       arch, arch   )
  DEF_REGISTER(        r0,   0 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r1,   1 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r2,   2 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r3,   3 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r4,   4 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r5,   5 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r6,   6 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r7,   7 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r8,   8 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        r9,   9 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r10,  10 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r11,  11 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r12,  12 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r13,  13 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r14,  14 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r15,  15 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r16,  16 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r17,  17 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r18,  18 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r19,  19 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r20,  20 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r21,  21 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r22,  22 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r23,  23 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r24,  24 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r25,  25 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r26,  26 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r27,  27 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r28,  28 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r29,  29 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r30,  30 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       r31,  31 | GPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr0,   0 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr1,   1 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr2,   2 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr3,   3 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr4,   4 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr5,   5 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr6,   6 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr7,   7 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr8,   8 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fpr9,   9 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr10,  10 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr11,  11 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr12,  12 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr13,  13 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr14,  14 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr15,  15 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr16,  16 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr17,  17 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr18,  18 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr19,  19 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr20,  20 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr21,  21 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr22,  22 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr23,  23 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr24,  24 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr25,  25 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr26,  26 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr27,  27 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr28,  28 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr29,  29 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr30,  30 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpr31,  31 | FPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr0,   0 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr1,   1 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr2,   2 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr3,   3 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr4,   4 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr5,   5 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr6,   6 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr7,   7 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr8,   8 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      fsr9,   9 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr10,  10 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr11,  11 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr12,  12 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr13,  13 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr14,  14 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr15,  15 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr16,  16 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr17,  17 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr18,  18 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr19,  19 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr20,  20 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr21,  21 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr22,  22 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr23,  23 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr24,  24 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr25,  25 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr26,  26 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr27,  27 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr28,  28 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr29,  29 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr30,  30 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fsr31,  31 | FSR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        mq,   0 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       xer,   1 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        lr,   8 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       ctr,   9 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       amr,  13 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      dscr,  17 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     dsisr,  18 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       dar,  19 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       dec,  22 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      sdr1,  25 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      srr0,  26 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      srr1,  27 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cfar,  28 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(   amr_pri,  29 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       pid,  48 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    gdecar,  53 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     decar,  54 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    mcivpr,  55 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      lper,  56 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     lperu,  57 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     csrr0,  58 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     csrr1,  59 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    gtsrwr,  60 | SPR | Arch_ppc32, "ppc32");
  //DEF_REGISTER(      iamr,  61 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       esr,  62 | SPR | Arch_ppc32, "ppc32");
  //DEF_REGISTER(      ivpr,  66 | SPR | Arch_ppc32, "ppc32");

  DEF_REGISTER(    vrsave, 256 | SPR | Arch_ppc32, "ppc32");

  DEF_REGISTER(     sprg0, 272 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg1, 273 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg2, 274 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg3, 275 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg4, 276 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg5, 277 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg6, 278 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     sprg7, 279 | SPR | Arch_ppc32, "ppc32");

  DEF_REGISTER(  sprg3_ro, 259 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(  sprg4_ro, 260 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(  sprg5_ro, 261 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(  sprg6_ro, 262 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(  sprg7_ro, 263 | SPR | Arch_ppc32, "ppc32");

  DEF_REGISTER(       ear, 282 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    tbl_wo, 284 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    tbl_ro, 268 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    tbu_wo, 285 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    tbu_ro, 269 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       pvr, 287 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat0u, 528 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat0l, 529 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat1u, 530 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat1l, 531 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat2u, 532 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat2l, 533 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat3u, 534 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    ibat3l, 535 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat0u, 536 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat0l, 537 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat1u, 538 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat1l, 539 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat2u, 540 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat2l, 541 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat3u, 542 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    dbat3l, 543 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        pc, 600 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     fpscw, 601 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw0, 602 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw1, 603 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw2, 604 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw3, 605 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw4, 606 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw5, 607 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw6, 608 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(    fpscw7, 609 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       msr, 610 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      ivpr, 611 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     ivor8, 612 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg0, 613 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg1, 614 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg2, 615 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg3, 616 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg4, 617 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg5, 618 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg6, 619 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      seg7, 620 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr0, 621 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr1, 622 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr2, 623 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr3, 624 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr4, 625 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr5, 626 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr6, 627 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       cr7, 628 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(        cr, 629 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       or3, 630 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      trap, 631 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr0l, 700 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr0g, 701 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr0e, 702 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr0s, 703 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr1l, 704 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr1g, 705 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr1e, 706 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr1s, 707 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr2l, 708 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr2g, 709 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr2e, 710 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr2s, 711 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr3l, 712 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr3g, 713 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr3e, 714 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr3s, 715 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr4l, 716 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr4g, 717 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr4e, 718 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr4s, 719 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr5l, 720 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr5g, 721 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr5e, 722 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr5s, 723 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr6l, 724 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr6g, 725 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr6e, 726 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr6s, 727 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr7l, 728 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr7g, 729 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr7e, 730 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(      cr7s, 731 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(       ppr, 896 | SPR | Arch_ppc32, "ppc32");
  DEF_REGISTER(     ppr32, 898 | SPR | Arch_ppc32, "ppc32");

}}

#endif
