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

#ifndef DYNINST_MACHREGISTER_h
#define DYNINST_MACHREGISTER_h

#include "Architecture.h"
#include "util.h"

#include <string>

namespace Dyninst {
  typedef unsigned long MachRegisterVal;

  class COMMON_EXPORT MachRegister {
  private:
    signed int reg;

  public:
    MachRegister();
    explicit MachRegister(signed int r);
    explicit MachRegister(signed int r, std::string n);

    MachRegister getBaseRegister() const;
    Architecture getArchitecture() const;
    bool isValid() const;

    std::string const& name() const;
    unsigned int size() const;
    bool operator<(const MachRegister& a) const;
    bool operator==(const MachRegister& a) const;
    operator signed int() const;
    signed int val() const;

    // Return the category of the MachRegister
    unsigned int regClass() const;

    static MachRegister getPC(Dyninst::Architecture arch);
    static MachRegister getReturnAddress(Dyninst::Architecture arch);
    static MachRegister getFramePointer(Dyninst::Architecture arch);
    static MachRegister getStackPointer(Dyninst::Architecture arch);
    static MachRegister getSyscallNumberReg(Dyninst::Architecture arch);
    static MachRegister getSyscallNumberOReg(Dyninst::Architecture arch);
    static MachRegister getSyscallReturnValueReg(Dyninst::Architecture arch);
    static MachRegister getZeroFlag(Dyninst::Architecture arch);

    bool isPC() const;
    bool isFramePointer() const;
    bool isStackPointer() const;
    bool isSyscallNumberReg() const;
    bool isSyscallReturnValueReg() const;
    bool isFlag() const;
    bool isZeroFlag() const;

    void getROSERegister(int& c, int& n, int& p);

    static MachRegister DwarfEncToReg(int encoding, Dyninst::Architecture arch);
    static MachRegister getArchRegFromAbstractReg(MachRegister abstract,
                                                  Dyninst::Architecture arch);
    int getDwarfEnc() const;

    static MachRegister getArchReg(unsigned int regNum, Dyninst::Architecture arch);
  };
}

#endif
