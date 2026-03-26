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

#ifndef COMMON_IA32_MEMACC_H
#define COMMON_IA32_MEMACC_H

#include "dyntypes.h"

// VG(6/20/02): To support Paradyn without forcing it to include BPatch_memoryAccess, we
//              define this IA-32 "specific" class to encapsulate the same info - yuck

enum sizehacks {
  shREP = 1,
  shREPECMPS,
  shREPESCAS,
  shREPNECMPS,
  shREPNESCAS
};

struct ia32_memacc {
  bool is;
  bool read;
  bool write;
  bool nt; // non-temporal, e.g. movntq...
  bool prefetch;

  int addr_size; // size of address in 16-bit words
  long imm;
  int scale;
  int regs[2]; // register encodings (in ISA order): 0-7
               // (E)AX, (E)CX, (E)DX, (E)BX
               // (E)SP, (E)BP, (E)SI, (E)DI

  int size;
  int sizehack;    // register (E)CX or string based
  int prefetchlvl; // prefetch level
  int prefetchstt; // prefetch state (AMD)

  ia32_memacc()
      : is(false), read(false), write(false), nt(false), prefetch(false), addr_size(2), imm(0),
        scale(0), size(0), sizehack(0), prefetchlvl(-1), prefetchstt(-1) {
    regs[0] = -1;
    regs[1] = -1;
  }

  void set16(int reg0, int reg1, long disp) {
    is = true;
    addr_size = 1;
    regs[0] = reg0;
    regs[1] = reg1;
    imm = disp;
  }

  void set(int reg, long disp, int addr_sz) {
    is = true;
    addr_size = addr_sz;
    regs[0] = reg;
    imm = disp;
  }

  void set_sib(int base, int scal, int indx, long disp, int addr_sz) {
    is = true;
    addr_size = addr_sz;
    regs[0] = base;
    regs[1] = indx;
    scale = scal;
    imm = disp;
  }

  void setXY(int reg, int _size, int _addr_size) {
    is = true;
    regs[0] = reg;
    size = _size;
    addr_size = _addr_size;
  }

  void print();
};

ia32_memacc convert(char const *, Dyninst::Address, bool);

#endif
