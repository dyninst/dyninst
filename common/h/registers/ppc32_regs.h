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
#include <cstdint>

namespace Dyninst { namespace ppc32 {

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

  //          (      name,  ID | cat |       arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_PPC32_REG_LIST(X) \
      X(r0, 0 | GPR | Arch_ppc32) \
      X(r1, 1 | GPR | Arch_ppc32) \
      X(r2, 2 | GPR | Arch_ppc32) \
      X(r3, 3 | GPR | Arch_ppc32) \
      X(r4, 4 | GPR | Arch_ppc32) \
      X(r5, 5 | GPR | Arch_ppc32) \
      X(r6, 6 | GPR | Arch_ppc32) \
      X(r7, 7 | GPR | Arch_ppc32) \
      X(r8, 8 | GPR | Arch_ppc32) \
      X(r9, 9 | GPR | Arch_ppc32) \
      X(r10, 10 | GPR | Arch_ppc32) \
      X(r11, 11 | GPR | Arch_ppc32) \
      X(r12, 12 | GPR | Arch_ppc32) \
      X(r13, 13 | GPR | Arch_ppc32) \
      X(r14, 14 | GPR | Arch_ppc32) \
      X(r15, 15 | GPR | Arch_ppc32) \
      X(r16, 16 | GPR | Arch_ppc32) \
      X(r17, 17 | GPR | Arch_ppc32) \
      X(r18, 18 | GPR | Arch_ppc32) \
      X(r19, 19 | GPR | Arch_ppc32) \
      X(r20, 20 | GPR | Arch_ppc32) \
      X(r21, 21 | GPR | Arch_ppc32) \
      X(r22, 22 | GPR | Arch_ppc32) \
      X(r23, 23 | GPR | Arch_ppc32) \
      X(r24, 24 | GPR | Arch_ppc32) \
      X(r25, 25 | GPR | Arch_ppc32) \
      X(r26, 26 | GPR | Arch_ppc32) \
      X(r27, 27 | GPR | Arch_ppc32) \
      X(r28, 28 | GPR | Arch_ppc32) \
      X(r29, 29 | GPR | Arch_ppc32) \
      X(r30, 30 | GPR | Arch_ppc32) \
      X(r31, 31 | GPR | Arch_ppc32) \
      X(fpr0, 0 | FPR | Arch_ppc32) \
      X(fpr1, 1 | FPR | Arch_ppc32) \
      X(fpr2, 2 | FPR | Arch_ppc32) \
      X(fpr3, 3 | FPR | Arch_ppc32) \
      X(fpr4, 4 | FPR | Arch_ppc32) \
      X(fpr5, 5 | FPR | Arch_ppc32) \
      X(fpr6, 6 | FPR | Arch_ppc32) \
      X(fpr7, 7 | FPR | Arch_ppc32) \
      X(fpr8, 8 | FPR | Arch_ppc32) \
      X(fpr9, 9 | FPR | Arch_ppc32) \
      X(fpr10, 10 | FPR | Arch_ppc32) \
      X(fpr11, 11 | FPR | Arch_ppc32) \
      X(fpr12, 12 | FPR | Arch_ppc32) \
      X(fpr13, 13 | FPR | Arch_ppc32) \
      X(fpr14, 14 | FPR | Arch_ppc32) \
      X(fpr15, 15 | FPR | Arch_ppc32) \
      X(fpr16, 16 | FPR | Arch_ppc32) \
      X(fpr17, 17 | FPR | Arch_ppc32) \
      X(fpr18, 18 | FPR | Arch_ppc32) \
      X(fpr19, 19 | FPR | Arch_ppc32) \
      X(fpr20, 20 | FPR | Arch_ppc32) \
      X(fpr21, 21 | FPR | Arch_ppc32) \
      X(fpr22, 22 | FPR | Arch_ppc32) \
      X(fpr23, 23 | FPR | Arch_ppc32) \
      X(fpr24, 24 | FPR | Arch_ppc32) \
      X(fpr25, 25 | FPR | Arch_ppc32) \
      X(fpr26, 26 | FPR | Arch_ppc32) \
      X(fpr27, 27 | FPR | Arch_ppc32) \
      X(fpr28, 28 | FPR | Arch_ppc32) \
      X(fpr29, 29 | FPR | Arch_ppc32) \
      X(fpr30, 30 | FPR | Arch_ppc32) \
      X(fpr31, 31 | FPR | Arch_ppc32) \
      X(fsr0, 0 | FSR | Arch_ppc32) \
      X(fsr1, 1 | FSR | Arch_ppc32) \
      X(fsr2, 2 | FSR | Arch_ppc32) \
      X(fsr3, 3 | FSR | Arch_ppc32) \
      X(fsr4, 4 | FSR | Arch_ppc32) \
      X(fsr5, 5 | FSR | Arch_ppc32) \
      X(fsr6, 6 | FSR | Arch_ppc32) \
      X(fsr7, 7 | FSR | Arch_ppc32) \
      X(fsr8, 8 | FSR | Arch_ppc32) \
      X(fsr9, 9 | FSR | Arch_ppc32) \
      X(fsr10, 10 | FSR | Arch_ppc32) \
      X(fsr11, 11 | FSR | Arch_ppc32) \
      X(fsr12, 12 | FSR | Arch_ppc32) \
      X(fsr13, 13 | FSR | Arch_ppc32) \
      X(fsr14, 14 | FSR | Arch_ppc32) \
      X(fsr15, 15 | FSR | Arch_ppc32) \
      X(fsr16, 16 | FSR | Arch_ppc32) \
      X(fsr17, 17 | FSR | Arch_ppc32) \
      X(fsr18, 18 | FSR | Arch_ppc32) \
      X(fsr19, 19 | FSR | Arch_ppc32) \
      X(fsr20, 20 | FSR | Arch_ppc32) \
      X(fsr21, 21 | FSR | Arch_ppc32) \
      X(fsr22, 22 | FSR | Arch_ppc32) \
      X(fsr23, 23 | FSR | Arch_ppc32) \
      X(fsr24, 24 | FSR | Arch_ppc32) \
      X(fsr25, 25 | FSR | Arch_ppc32) \
      X(fsr26, 26 | FSR | Arch_ppc32) \
      X(fsr27, 27 | FSR | Arch_ppc32) \
      X(fsr28, 28 | FSR | Arch_ppc32) \
      X(fsr29, 29 | FSR | Arch_ppc32) \
      X(fsr30, 30 | FSR | Arch_ppc32) \
      X(fsr31, 31 | FSR | Arch_ppc32) \
      X(mq, 0 | SPR | Arch_ppc32) \
      X(xer, 1 | SPR | Arch_ppc32) \
      X(lr, 8 | SPR | Arch_ppc32) \
      X(ctr, 9 | SPR | Arch_ppc32) \
      X(amr, 13 | SPR | Arch_ppc32) \
      X(dscr, 17 | SPR | Arch_ppc32) \
      X(dsisr, 18 | SPR | Arch_ppc32) \
      X(dar, 19 | SPR | Arch_ppc32) \
      X(dec, 22 | SPR | Arch_ppc32) \
      X(sdr1, 25 | SPR | Arch_ppc32) \
      X(srr0, 26 | SPR | Arch_ppc32) \
      X(srr1, 27 | SPR | Arch_ppc32) \
      X(cfar, 28 | SPR | Arch_ppc32) \
      X(amr_pri, 29 | SPR | Arch_ppc32) \
      X(pid, 48 | SPR | Arch_ppc32) \
      X(gdecar, 53 | SPR | Arch_ppc32) \
      X(decar, 54 | SPR | Arch_ppc32) \
      X(mcivpr, 55 | SPR | Arch_ppc32) \
      X(lper, 56 | SPR | Arch_ppc32) \
      X(lperu, 57 | SPR | Arch_ppc32) \
      X(csrr0, 58 | SPR | Arch_ppc32) \
      X(csrr1, 59 | SPR | Arch_ppc32) \
      X(gtsrwr, 60 | SPR | Arch_ppc32) \
      X(esr, 62 | SPR | Arch_ppc32) \
      X(vrsave, 256 | SPR | Arch_ppc32) \
      X(sprg0, 272 | SPR | Arch_ppc32) \
      X(sprg1, 273 | SPR | Arch_ppc32) \
      X(sprg2, 274 | SPR | Arch_ppc32) \
      X(sprg3, 275 | SPR | Arch_ppc32) \
      X(sprg4, 276 | SPR | Arch_ppc32) \
      X(sprg5, 277 | SPR | Arch_ppc32) \
      X(sprg6, 278 | SPR | Arch_ppc32) \
      X(sprg7, 279 | SPR | Arch_ppc32) \
      X(sprg3_ro, 259 | SPR | Arch_ppc32) \
      X(sprg4_ro, 260 | SPR | Arch_ppc32) \
      X(sprg5_ro, 261 | SPR | Arch_ppc32) \
      X(sprg6_ro, 262 | SPR | Arch_ppc32) \
      X(sprg7_ro, 263 | SPR | Arch_ppc32) \
      X(ear, 282 | SPR | Arch_ppc32) \
      X(tbl_wo, 284 | SPR | Arch_ppc32) \
      X(tbl_ro, 268 | SPR | Arch_ppc32) \
      X(tbu_wo, 285 | SPR | Arch_ppc32) \
      X(tbu_ro, 269 | SPR | Arch_ppc32) \
      X(pvr, 287 | SPR | Arch_ppc32) \
      X(ibat0u, 528 | SPR | Arch_ppc32) \
      X(ibat0l, 529 | SPR | Arch_ppc32) \
      X(ibat1u, 530 | SPR | Arch_ppc32) \
      X(ibat1l, 531 | SPR | Arch_ppc32) \
      X(ibat2u, 532 | SPR | Arch_ppc32) \
      X(ibat2l, 533 | SPR | Arch_ppc32) \
      X(ibat3u, 534 | SPR | Arch_ppc32) \
      X(ibat3l, 535 | SPR | Arch_ppc32) \
      X(dbat0u, 536 | SPR | Arch_ppc32) \
      X(dbat0l, 537 | SPR | Arch_ppc32) \
      X(dbat1u, 538 | SPR | Arch_ppc32) \
      X(dbat1l, 539 | SPR | Arch_ppc32) \
      X(dbat2u, 540 | SPR | Arch_ppc32) \
      X(dbat2l, 541 | SPR | Arch_ppc32) \
      X(dbat3u, 542 | SPR | Arch_ppc32) \
      X(dbat3l, 543 | SPR | Arch_ppc32) \
      X(pc, 600 | SPR | Arch_ppc32) \
      X(fpscw, 601 | SPR | Arch_ppc32) \
      X(fpscw0, 602 | SPR | Arch_ppc32) \
      X(fpscw1, 603 | SPR | Arch_ppc32) \
      X(fpscw2, 604 | SPR | Arch_ppc32) \
      X(fpscw3, 605 | SPR | Arch_ppc32) \
      X(fpscw4, 606 | SPR | Arch_ppc32) \
      X(fpscw5, 607 | SPR | Arch_ppc32) \
      X(fpscw6, 608 | SPR | Arch_ppc32) \
      X(fpscw7, 609 | SPR | Arch_ppc32) \
      X(msr, 610 | SPR | Arch_ppc32) \
      X(ivpr, 611 | SPR | Arch_ppc32) \
      X(ivor8, 612 | SPR | Arch_ppc32) \
      X(seg0, 613 | SPR | Arch_ppc32) \
      X(seg1, 614 | SPR | Arch_ppc32) \
      X(seg2, 615 | SPR | Arch_ppc32) \
      X(seg3, 616 | SPR | Arch_ppc32) \
      X(seg4, 617 | SPR | Arch_ppc32) \
      X(seg5, 618 | SPR | Arch_ppc32) \
      X(seg6, 619 | SPR | Arch_ppc32) \
      X(seg7, 620 | SPR | Arch_ppc32) \
      X(cr0, 621 | SPR | Arch_ppc32) \
      X(cr1, 622 | SPR | Arch_ppc32) \
      X(cr2, 623 | SPR | Arch_ppc32) \
      X(cr3, 624 | SPR | Arch_ppc32) \
      X(cr4, 625 | SPR | Arch_ppc32) \
      X(cr5, 626 | SPR | Arch_ppc32) \
      X(cr6, 627 | SPR | Arch_ppc32) \
      X(cr7, 628 | SPR | Arch_ppc32) \
      X(cr, 629 | SPR | Arch_ppc32) \
      X(or3, 630 | SPR | Arch_ppc32) \
      X(trap, 631 | SPR | Arch_ppc32) \
      X(cr0l, 700 | SPR | Arch_ppc32) \
      X(cr0g, 701 | SPR | Arch_ppc32) \
      X(cr0e, 702 | SPR | Arch_ppc32) \
      X(cr0s, 703 | SPR | Arch_ppc32) \
      X(cr1l, 704 | SPR | Arch_ppc32) \
      X(cr1g, 705 | SPR | Arch_ppc32) \
      X(cr1e, 706 | SPR | Arch_ppc32) \
      X(cr1s, 707 | SPR | Arch_ppc32) \
      X(cr2l, 708 | SPR | Arch_ppc32) \
      X(cr2g, 709 | SPR | Arch_ppc32) \
      X(cr2e, 710 | SPR | Arch_ppc32) \
      X(cr2s, 711 | SPR | Arch_ppc32) \
      X(cr3l, 712 | SPR | Arch_ppc32) \
      X(cr3g, 713 | SPR | Arch_ppc32) \
      X(cr3e, 714 | SPR | Arch_ppc32) \
      X(cr3s, 715 | SPR | Arch_ppc32) \
      X(cr4l, 716 | SPR | Arch_ppc32) \
      X(cr4g, 717 | SPR | Arch_ppc32) \
      X(cr4e, 718 | SPR | Arch_ppc32) \
      X(cr4s, 719 | SPR | Arch_ppc32) \
      X(cr5l, 720 | SPR | Arch_ppc32) \
      X(cr5g, 721 | SPR | Arch_ppc32) \
      X(cr5e, 722 | SPR | Arch_ppc32) \
      X(cr5s, 723 | SPR | Arch_ppc32) \
      X(cr6l, 724 | SPR | Arch_ppc32) \
      X(cr6g, 725 | SPR | Arch_ppc32) \
      X(cr6e, 726 | SPR | Arch_ppc32) \
      X(cr6s, 727 | SPR | Arch_ppc32) \
      X(cr7l, 728 | SPR | Arch_ppc32) \
      X(cr7g, 729 | SPR | Arch_ppc32) \
      X(cr7e, 730 | SPR | Arch_ppc32) \
      X(cr7s, 731 | SPR | Arch_ppc32) \
      X(ppr, 896 | SPR | Arch_ppc32) \
      X(ppr32, 898 | SPR | Arch_ppc32)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  DYNINST_PPC32_REG_LIST(DEF_ONE)
  #undef DEF_ONE

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_PPC32_REG_LIST(NAME_ONE) };
  #undef NAME_ONE


}}

#endif
