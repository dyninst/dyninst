
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
#ifndef APPEND_OPERANDS_GFX908_H
#define APPEND_OPERANDS_GFX908_H
#include <stdint.h>

void appendOPR_SIMM4(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SIMM8(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SIMM16(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SIMM32(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_WAITCNT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_ATTR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_DSMEM(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_FLAT_SCRATCH(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_PARAM(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_PC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SDST(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SDST_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SDST_M0(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void processOPR_SMEM_OFFSET(layout_ENC_SMEM & layout  );
void appendOPR_SRC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_ACCVGPR_OR_CONST(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_NOLDS(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_SIMPLE(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SRC_VGPR_OR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SREG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SREG_NOVCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SSRC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SSRC_LANESEL(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SSRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_SSRC_SPECIAL_SCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_TGT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_VCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
void appendOPR_VGPR_OR_LDS(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1 , bool isImplicit = false );
#endif /* APPEND_OPERANS_GFX908_H*/
