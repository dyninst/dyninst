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
#ifndef DYNINST_DATAFLOW_ABI_BRIDGE_H
#define DYNINST_DATAFLOW_ABI_BRIDGE_H

#include "Architecture.h"
#include "dataflowAPI/h/bitArray.h"
#include "dataflowAPI/src/RegisterMap.h"

// Bridge between the architecture-independent common ABI (Dyninst::ABI,
// common/h/abi.h) and the bitArray representation used by the dataflow ABI
// class. Lives in its own translation unit so that including abi.h does not
// make Dyninst::ABI collide with the dataflowAPI ::ABI class under the
// file-scope `using namespace Dyninst` in ABI.C.
namespace Dyninst { namespace DataflowAPI {

  // Allocate (with new) and populate the seven ABI register bitArrays plus the
  // all-registers bitArray for `arch`, mapping registers through `idx`.
  // Supported: Arch_x86 and Arch_x86_64.
  void buildABIBitArrays(Dyninst::Architecture arch, RegisterMap& idx,
                         bitArray*& returnRegs, bitArray*& callParam,
                         bitArray*& returnRead, bitArray*& callRead,
                         bitArray*& callWritten, bitArray*& syscallRead,
                         bitArray*& syscallWritten, bitArray*& allRegs);

}}

#endif
