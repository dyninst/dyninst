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
#include "dyninst_visibility.h"

#include <string>
#include <vector>
#include <cstdint>

namespace Dyninst {
  typedef unsigned long MachRegisterVal;

  class DYNINST_EXPORT MachRegister {
  private:
    int32_t reg;
    // Prototype (store-the-name): the register name is stored directly in the
    // object (pointing at a string literal from DEF_REGISTER), which removes the
    // dependency on the runtime registers::names map and lets the register
    // constants be constexpr.
    const char* reg_name = nullptr;

  public:
    // Prototype (constexpr MachRegister): the integer handle makes MachRegister
    // a literal type -- usable in constant expressions, so register constants
    // can eventually be constexpr with no static-init/name-map dependency.
    // Only the int-handle path is constexpr here; the name-populating
    // constructor and name()/getAllRegistersForArch caches are unchanged (their
    // migration to a static, generated table is the follow-up -- see PR notes).
    constexpr MachRegister() : reg(0) {}
    constexpr explicit MachRegister(int32_t r) : reg(r) {}
    constexpr MachRegister(int32_t r, const char* n) : reg(r), reg_name(n) {}

    MachRegister getBaseRegister() const;
    Architecture getArchitecture() const;
    bool isValid() const;

    // Returns by value (was `std::string const&`): the name now lives in the
    // object as a `const char*`, so there is no long-lived std::string to
    // reference. Returning by value keeps `const std::string& x = r.name();`
    // working via lifetime extension.
    std::string name() const { return reg_name ? std::string(reg_name) : std::string("<INVALID_REG>"); }
    unsigned int size() const;
    constexpr bool operator<(const MachRegister& a) const { return reg < a.reg; }
    constexpr bool operator==(const MachRegister& a) const { return reg == a.reg; }
    constexpr operator int32_t() const { return reg; }
    constexpr int32_t val() const { return reg; }

    // Return the category of the MachRegister
    unsigned int regClass() const;

    /* Returns the architecture-specific ID for the register's length
     *
     *  This is _not_ the number of bytes the register contains. For
     *  that, use `size()`.
     */
    int32_t getLengthID() const;

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

    /* Checks if this is a general-purpose register
     *
     *  General-purpose registers do not have a specific data
     *  type or use. For example, they might be used for both
     *  integer arithmetic and memory addressing.
     */
    bool isGeneralPurpose() const;

    /* Checks if this is a vector register
     *
     *  Vector registers are capable of performing operations
     *  on multiple data values simultaneously.
     *
     *  NOTE: This includes any vector status/control registers.
     */
    bool isVector() const;

    /* Checks if this is a control/status register
     *
     *  Control registers influence the execution behavior of the Processing
     *  Unit. For example, enabling/disabling floating-point exceptions.
     *
     *  Status registers report behaviors of executed instructions.
     *  For example, if a floating-point exception occurred.
     *
     *  NOTE: FLAG registers are not considered to be status registers.
     *        Use `isFlag()` for that.
     */
    bool isControlStatus() const;

    /* Checks if this register operates on floating-point data
     *
     *  NOTE: This includes any floating-point status/control registers.
     */
    bool isFloatingPoint() const;

    static std::vector<MachRegister> const& getAllRegistersForArch(Dyninst::Architecture);
  };
}

#endif
