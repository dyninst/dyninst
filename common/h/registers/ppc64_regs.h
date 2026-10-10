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

#ifndef DYNINST_PPC64_REGS_H
#define DYNINST_PPC64_REGS_H

//clang-format: off

#include "registers/reg_def.h"
#include <cstdint>

namespace Dyninst { namespace ppc64 {

  /**
   * Format of constants:
   *  [0x000000ff] Lower 8 bits are base register ID
   *  [0x0000ff00] Next 8 bits are unused.
   *  [0x00ff0000] Next 8 bits are the register category, GPR, VSR, etc.
   *  [0xff000000] Upper 8 bits are the architecture.
   **/

  const int32_t GPR = 0x00010000;
  const int32_t FPR = 0x00020000;
  const int32_t FSR = 0x00040000;
  const int32_t SPR = 0x00080000;
  const int32_t VSR = 0x00000000;

  //          (      name,  ID | cat |       arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_PPC64_REG_LIST(X) \
      X(r0, 0 | GPR | Arch_ppc64) \
      X(r1, 1 | GPR | Arch_ppc64) \
      X(r2, 2 | GPR | Arch_ppc64) \
      X(r3, 3 | GPR | Arch_ppc64) \
      X(r4, 4 | GPR | Arch_ppc64) \
      X(r5, 5 | GPR | Arch_ppc64) \
      X(r6, 6 | GPR | Arch_ppc64) \
      X(r7, 7 | GPR | Arch_ppc64) \
      X(r8, 8 | GPR | Arch_ppc64) \
      X(r9, 9 | GPR | Arch_ppc64) \
      X(r10, 10 | GPR | Arch_ppc64) \
      X(r11, 11 | GPR | Arch_ppc64) \
      X(r12, 12 | GPR | Arch_ppc64) \
      X(r13, 13 | GPR | Arch_ppc64) \
      X(r14, 14 | GPR | Arch_ppc64) \
      X(r15, 15 | GPR | Arch_ppc64) \
      X(r16, 16 | GPR | Arch_ppc64) \
      X(r17, 17 | GPR | Arch_ppc64) \
      X(r18, 18 | GPR | Arch_ppc64) \
      X(r19, 19 | GPR | Arch_ppc64) \
      X(r20, 20 | GPR | Arch_ppc64) \
      X(r21, 21 | GPR | Arch_ppc64) \
      X(r22, 22 | GPR | Arch_ppc64) \
      X(r23, 23 | GPR | Arch_ppc64) \
      X(r24, 24 | GPR | Arch_ppc64) \
      X(r25, 25 | GPR | Arch_ppc64) \
      X(r26, 26 | GPR | Arch_ppc64) \
      X(r27, 27 | GPR | Arch_ppc64) \
      X(r28, 28 | GPR | Arch_ppc64) \
      X(r29, 29 | GPR | Arch_ppc64) \
      X(r30, 30 | GPR | Arch_ppc64) \
      X(r31, 31 | GPR | Arch_ppc64) \
      X(fpr0, 0 | FPR | Arch_ppc64) \
      X(fpr1, 1 | FPR | Arch_ppc64) \
      X(fpr2, 2 | FPR | Arch_ppc64) \
      X(fpr3, 3 | FPR | Arch_ppc64) \
      X(fpr4, 4 | FPR | Arch_ppc64) \
      X(fpr5, 5 | FPR | Arch_ppc64) \
      X(fpr6, 6 | FPR | Arch_ppc64) \
      X(fpr7, 7 | FPR | Arch_ppc64) \
      X(fpr8, 8 | FPR | Arch_ppc64) \
      X(fpr9, 9 | FPR | Arch_ppc64) \
      X(fpr10, 10 | FPR | Arch_ppc64) \
      X(fpr11, 11 | FPR | Arch_ppc64) \
      X(fpr12, 12 | FPR | Arch_ppc64) \
      X(fpr13, 13 | FPR | Arch_ppc64) \
      X(fpr14, 14 | FPR | Arch_ppc64) \
      X(fpr15, 15 | FPR | Arch_ppc64) \
      X(fpr16, 16 | FPR | Arch_ppc64) \
      X(fpr17, 17 | FPR | Arch_ppc64) \
      X(fpr18, 18 | FPR | Arch_ppc64) \
      X(fpr19, 19 | FPR | Arch_ppc64) \
      X(fpr20, 20 | FPR | Arch_ppc64) \
      X(fpr21, 21 | FPR | Arch_ppc64) \
      X(fpr22, 22 | FPR | Arch_ppc64) \
      X(fpr23, 23 | FPR | Arch_ppc64) \
      X(fpr24, 24 | FPR | Arch_ppc64) \
      X(fpr25, 25 | FPR | Arch_ppc64) \
      X(fpr26, 26 | FPR | Arch_ppc64) \
      X(fpr27, 27 | FPR | Arch_ppc64) \
      X(fpr28, 28 | FPR | Arch_ppc64) \
      X(fpr29, 29 | FPR | Arch_ppc64) \
      X(fpr30, 30 | FPR | Arch_ppc64) \
      X(fpr31, 31 | FPR | Arch_ppc64) \
      X(fsr0, 0 | FSR | Arch_ppc64) \
      X(fsr1, 1 | FSR | Arch_ppc64) \
      X(fsr2, 2 | FSR | Arch_ppc64) \
      X(fsr3, 3 | FSR | Arch_ppc64) \
      X(fsr4, 4 | FSR | Arch_ppc64) \
      X(fsr5, 5 | FSR | Arch_ppc64) \
      X(fsr6, 6 | FSR | Arch_ppc64) \
      X(fsr7, 7 | FSR | Arch_ppc64) \
      X(fsr8, 8 | FSR | Arch_ppc64) \
      X(fsr9, 9 | FSR | Arch_ppc64) \
      X(fsr10, 10 | FSR | Arch_ppc64) \
      X(fsr11, 11 | FSR | Arch_ppc64) \
      X(fsr12, 12 | FSR | Arch_ppc64) \
      X(fsr13, 13 | FSR | Arch_ppc64) \
      X(fsr14, 14 | FSR | Arch_ppc64) \
      X(fsr15, 15 | FSR | Arch_ppc64) \
      X(fsr16, 16 | FSR | Arch_ppc64) \
      X(fsr17, 17 | FSR | Arch_ppc64) \
      X(fsr18, 18 | FSR | Arch_ppc64) \
      X(fsr19, 19 | FSR | Arch_ppc64) \
      X(fsr20, 20 | FSR | Arch_ppc64) \
      X(fsr21, 21 | FSR | Arch_ppc64) \
      X(fsr22, 22 | FSR | Arch_ppc64) \
      X(fsr23, 23 | FSR | Arch_ppc64) \
      X(fsr24, 24 | FSR | Arch_ppc64) \
      X(fsr25, 25 | FSR | Arch_ppc64) \
      X(fsr26, 26 | FSR | Arch_ppc64) \
      X(fsr27, 27 | FSR | Arch_ppc64) \
      X(fsr28, 28 | FSR | Arch_ppc64) \
      X(fsr29, 29 | FSR | Arch_ppc64) \
      X(fsr30, 30 | FSR | Arch_ppc64) \
      X(fsr31, 31 | FSR | Arch_ppc64) \
      X(mq, 0 | SPR | Arch_ppc64) \
      X(xer, 1 | SPR | Arch_ppc64) \
      X(lr, 8 | SPR | Arch_ppc64) \
      X(ctr, 9 | SPR | Arch_ppc64) \
      X(dsisr, 18 | SPR | Arch_ppc64) \
      X(dar, 19 | SPR | Arch_ppc64) \
      X(dec, 22 | SPR | Arch_ppc64) \
      X(sdr1, 25 | SPR | Arch_ppc64) \
      X(srr0, 26 | SPR | Arch_ppc64) \
      X(srr1, 27 | SPR | Arch_ppc64) \
      X(vrsave, 256 | SPR | Arch_ppc64) \
      X(sprg0, 272 | SPR | Arch_ppc64) \
      X(sprg1, 273 | SPR | Arch_ppc64) \
      X(sprg2, 274 | SPR | Arch_ppc64) \
      X(sprg3, 275 | SPR | Arch_ppc64) \
      X(sprg4, 276 | SPR | Arch_ppc64) \
      X(sprg5, 277 | SPR | Arch_ppc64) \
      X(sprg6, 278 | SPR | Arch_ppc64) \
      X(sprg7, 279 | SPR | Arch_ppc64) \
      X(sprg3_ro, 259 | SPR | Arch_ppc64) \
      X(sprg4_ro, 260 | SPR | Arch_ppc64) \
      X(sprg5_ro, 261 | SPR | Arch_ppc64) \
      X(sprg6_ro, 262 | SPR | Arch_ppc64) \
      X(sprg7_ro, 263 | SPR | Arch_ppc64) \
      X(ear, 282 | SPR | Arch_ppc64) \
      X(tbl_wo, 284 | SPR | Arch_ppc64) \
      X(tbl_ro, 268 | SPR | Arch_ppc64) \
      X(tbu_wo, 285 | SPR | Arch_ppc64) \
      X(tbu_ro, 269 | SPR | Arch_ppc64) \
      X(pvr, 287 | SPR | Arch_ppc64) \
      X(ibat0u, 528 | SPR | Arch_ppc64) \
      X(ibat0l, 529 | SPR | Arch_ppc64) \
      X(ibat1u, 530 | SPR | Arch_ppc64) \
      X(ibat1l, 531 | SPR | Arch_ppc64) \
      X(ibat2u, 532 | SPR | Arch_ppc64) \
      X(ibat2l, 533 | SPR | Arch_ppc64) \
      X(ibat3u, 534 | SPR | Arch_ppc64) \
      X(ibat3l, 535 | SPR | Arch_ppc64) \
      X(dbat0u, 536 | SPR | Arch_ppc64) \
      X(dbat0l, 537 | SPR | Arch_ppc64) \
      X(dbat1u, 538 | SPR | Arch_ppc64) \
      X(dbat1l, 539 | SPR | Arch_ppc64) \
      X(dbat2u, 540 | SPR | Arch_ppc64) \
      X(dbat2l, 541 | SPR | Arch_ppc64) \
      X(dbat3u, 542 | SPR | Arch_ppc64) \
      X(dbat3l, 543 | SPR | Arch_ppc64) \
      X(pc, 600 | SPR | Arch_ppc64) \
      X(fpscw, 601 | SPR | Arch_ppc64) \
      X(fpscw0, 602 | SPR | Arch_ppc64) \
      X(fpscw1, 603 | SPR | Arch_ppc64) \
      X(fpscw2, 604 | SPR | Arch_ppc64) \
      X(fpscw3, 605 | SPR | Arch_ppc64) \
      X(fpscw4, 606 | SPR | Arch_ppc64) \
      X(fpscw5, 607 | SPR | Arch_ppc64) \
      X(fpscw6, 608 | SPR | Arch_ppc64) \
      X(fpscw7, 609 | SPR | Arch_ppc64) \
      X(msr, 610 | SPR | Arch_ppc64) \
      X(ivpr, 611 | SPR | Arch_ppc64) \
      X(ivor8, 612 | SPR | Arch_ppc64) \
      X(seg0, 613 | SPR | Arch_ppc64) \
      X(seg1, 614 | SPR | Arch_ppc64) \
      X(seg2, 615 | SPR | Arch_ppc64) \
      X(seg3, 616 | SPR | Arch_ppc64) \
      X(seg4, 617 | SPR | Arch_ppc64) \
      X(seg5, 618 | SPR | Arch_ppc64) \
      X(seg6, 619 | SPR | Arch_ppc64) \
      X(seg7, 620 | SPR | Arch_ppc64) \
      X(cr0, 621 | SPR | Arch_ppc64) \
      X(cr1, 622 | SPR | Arch_ppc64) \
      X(cr2, 623 | SPR | Arch_ppc64) \
      X(cr3, 624 | SPR | Arch_ppc64) \
      X(cr4, 625 | SPR | Arch_ppc64) \
      X(cr5, 626 | SPR | Arch_ppc64) \
      X(cr6, 627 | SPR | Arch_ppc64) \
      X(cr7, 628 | SPR | Arch_ppc64) \
      X(cr, 629 | SPR | Arch_ppc64) \
      X(or3, 630 | SPR | Arch_ppc64) \
      X(trap, 631 | SPR | Arch_ppc64) \
      X(cr0l, 700 | SPR | Arch_ppc64) \
      X(cr0g, 701 | SPR | Arch_ppc64) \
      X(cr0e, 702 | SPR | Arch_ppc64) \
      X(cr0s, 703 | SPR | Arch_ppc64) \
      X(cr1l, 704 | SPR | Arch_ppc64) \
      X(cr1g, 705 | SPR | Arch_ppc64) \
      X(cr1e, 706 | SPR | Arch_ppc64) \
      X(cr1s, 707 | SPR | Arch_ppc64) \
      X(cr2l, 708 | SPR | Arch_ppc64) \
      X(cr2g, 709 | SPR | Arch_ppc64) \
      X(cr2e, 710 | SPR | Arch_ppc64) \
      X(cr2s, 711 | SPR | Arch_ppc64) \
      X(cr3l, 712 | SPR | Arch_ppc64) \
      X(cr3g, 713 | SPR | Arch_ppc64) \
      X(cr3e, 714 | SPR | Arch_ppc64) \
      X(cr3s, 715 | SPR | Arch_ppc64) \
      X(cr4l, 716 | SPR | Arch_ppc64) \
      X(cr4g, 717 | SPR | Arch_ppc64) \
      X(cr4e, 718 | SPR | Arch_ppc64) \
      X(cr4s, 719 | SPR | Arch_ppc64) \
      X(cr5l, 720 | SPR | Arch_ppc64) \
      X(cr5g, 721 | SPR | Arch_ppc64) \
      X(cr5e, 722 | SPR | Arch_ppc64) \
      X(cr5s, 723 | SPR | Arch_ppc64) \
      X(cr6l, 724 | SPR | Arch_ppc64) \
      X(cr6g, 725 | SPR | Arch_ppc64) \
      X(cr6e, 726 | SPR | Arch_ppc64) \
      X(cr6s, 727 | SPR | Arch_ppc64) \
      X(cr7l, 728 | SPR | Arch_ppc64) \
      X(cr7g, 729 | SPR | Arch_ppc64) \
      X(cr7e, 730 | SPR | Arch_ppc64) \
      X(cr7s, 731 | SPR | Arch_ppc64) \
      X(ppr, 896 | SPR | Arch_ppc64) \
      X(ppr32, 898 | SPR | Arch_ppc64) \
      X(vsr0, 0 | VSR | Arch_ppc64) \
      X(vsr1, 1 | VSR | Arch_ppc64) \
      X(vsr2, 2 | VSR | Arch_ppc64) \
      X(vsr3, 3 | VSR | Arch_ppc64) \
      X(vsr4, 4 | VSR | Arch_ppc64) \
      X(vsr5, 5 | VSR | Arch_ppc64) \
      X(vsr6, 6 | VSR | Arch_ppc64) \
      X(vsr7, 7 | VSR | Arch_ppc64) \
      X(vsr8, 8 | VSR | Arch_ppc64) \
      X(vsr9, 9 | VSR | Arch_ppc64) \
      X(vsr10, 10 | VSR | Arch_ppc64) \
      X(vsr11, 11 | VSR | Arch_ppc64) \
      X(vsr12, 12 | VSR | Arch_ppc64) \
      X(vsr13, 13 | VSR | Arch_ppc64) \
      X(vsr14, 14 | VSR | Arch_ppc64) \
      X(vsr15, 15 | VSR | Arch_ppc64) \
      X(vsr16, 16 | VSR | Arch_ppc64) \
      X(vsr17, 17 | VSR | Arch_ppc64) \
      X(vsr18, 18 | VSR | Arch_ppc64) \
      X(vsr19, 19 | VSR | Arch_ppc64) \
      X(vsr20, 20 | VSR | Arch_ppc64) \
      X(vsr21, 21 | VSR | Arch_ppc64) \
      X(vsr22, 22 | VSR | Arch_ppc64) \
      X(vsr23, 23 | VSR | Arch_ppc64) \
      X(vsr24, 24 | VSR | Arch_ppc64) \
      X(vsr25, 25 | VSR | Arch_ppc64) \
      X(vsr26, 26 | VSR | Arch_ppc64) \
      X(vsr27, 27 | VSR | Arch_ppc64) \
      X(vsr28, 28 | VSR | Arch_ppc64) \
      X(vsr29, 29 | VSR | Arch_ppc64) \
      X(vsr30, 30 | VSR | Arch_ppc64) \
      X(vsr31, 31 | VSR | Arch_ppc64) \
      X(vsr32, 32 | VSR | Arch_ppc64) \
      X(vsr33, 33 | VSR | Arch_ppc64) \
      X(vsr34, 34 | VSR | Arch_ppc64) \
      X(vsr35, 35 | VSR | Arch_ppc64) \
      X(vsr36, 36 | VSR | Arch_ppc64) \
      X(vsr37, 37 | VSR | Arch_ppc64) \
      X(vsr38, 38 | VSR | Arch_ppc64) \
      X(vsr39, 39 | VSR | Arch_ppc64) \
      X(vsr40, 40 | VSR | Arch_ppc64) \
      X(vsr41, 41 | VSR | Arch_ppc64) \
      X(vsr42, 42 | VSR | Arch_ppc64) \
      X(vsr43, 43 | VSR | Arch_ppc64) \
      X(vsr44, 44 | VSR | Arch_ppc64) \
      X(vsr45, 45 | VSR | Arch_ppc64) \
      X(vsr46, 46 | VSR | Arch_ppc64) \
      X(vsr47, 47 | VSR | Arch_ppc64) \
      X(vsr48, 48 | VSR | Arch_ppc64) \
      X(vsr49, 49 | VSR | Arch_ppc64) \
      X(vsr50, 50 | VSR | Arch_ppc64) \
      X(vsr51, 51 | VSR | Arch_ppc64) \
      X(vsr52, 52 | VSR | Arch_ppc64) \
      X(vsr53, 53 | VSR | Arch_ppc64) \
      X(vsr54, 54 | VSR | Arch_ppc64) \
      X(vsr55, 55 | VSR | Arch_ppc64) \
      X(vsr56, 56 | VSR | Arch_ppc64) \
      X(vsr57, 57 | VSR | Arch_ppc64) \
      X(vsr58, 58 | VSR | Arch_ppc64) \
      X(vsr59, 59 | VSR | Arch_ppc64) \
      X(vsr60, 60 | VSR | Arch_ppc64) \
      X(vsr61, 61 | VSR | Arch_ppc64) \
      X(vsr62, 62 | VSR | Arch_ppc64) \
      X(vsr63, 63 | VSR | Arch_ppc64)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  DYNINST_PPC64_REG_LIST(DEF_ONE)
  #undef DEF_ONE

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_PPC64_REG_LIST(NAME_ONE) };
  #undef NAME_ONE


}}

#endif
