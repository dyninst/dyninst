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
#ifndef ABI_H
#define ABI_H

#include "dyn_regs.h"
#include "bitArray.h"
#include <map>

using namespace Dyninst;
class ABI {
  static ABI *globalABI_;
  static ABI *globalABI64_;
  std::map<MachRegister, int> *index;
  int addr_width;

 public:
  DATAFLOW_EXPORT const bitArray &getCallReadRegisters() const;
  DATAFLOW_EXPORT const bitArray &getCallWrittenRegisters() const;
  DATAFLOW_EXPORT const bitArray &getReturnReadRegisters() const;
  DATAFLOW_EXPORT const bitArray &getReturnRegisters() const;
  // No such thing as return written...

  // Syscall!
  DATAFLOW_EXPORT const bitArray &getSyscallReadRegisters() const;
  DATAFLOW_EXPORT const bitArray &getSyscallWrittenRegisters() const;

  DATAFLOW_EXPORT const bitArray &getAllRegs() const;

  DATAFLOW_EXPORT int getIndex(MachRegister machReg);
  DATAFLOW_EXPORT std::map<MachRegister, int> *getIndexMap();

  DATAFLOW_EXPORT static void initialize32();
  DATAFLOW_EXPORT static void initialize64();

  DATAFLOW_EXPORT static ABI *getABI(int addr_width);
  DATAFLOW_EXPORT bitArray getBitArray();

 private:
  static bitArray callRead_;
  static bitArray callRead64_;

  static bitArray callWritten_;
  static bitArray callWritten64_;

  static bitArray returnRead_;
  static bitArray returnRead64_;

  static bitArray returnRegs_;
  static bitArray returnRegs64_;

  static bitArray syscallRead_;
  static bitArray syscallRead64_;

  static bitArray syscallWritten_;
  static bitArray syscallWritten64_;

  static bitArray allRegs_;
  static bitArray allRegs64_;

  static bitArray getBitArray(int size) { return bitArray(size); }

  ABI() : index(NULL), addr_width(0) {}
};

#endif  // ABI_H
