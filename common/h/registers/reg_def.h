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

#ifndef DYNINST_REG_DEF_H
#define DYNINST_REG_DEF_H

#include "registers/MachRegister.h"

/**
 * DEF_REGISTER will define its first parameter as the name of the object
 * it's declaring, and 'i<name>' as the integer value representing that object.
 * As an example, the name of a register may be
 *  x86::EAX
 * with that register having a value of
 *  x86::iEAX
 *
 * The value is mostly useful in the 'case' part switch statements.
 **/
// Prototype (store-the-name / constexpr constants):
// The register constants are `constexpr MachRegister` objects carrying their
// name as a string literal. Plain `constexpr` (not `inline`, so no C++17
// requirement) gives each TU its own compile-time copy; MachRegister constants
// are only ever used by value (compared by the int handle), so per-TU internal
// linkage is fine. This makes the old DYN_DEFINE_REGS decl/def split (which
// existed only to get one runtime definition in libcommon) unnecessary.
#define DEF_REGISTER(name, value)                                                                  \
    constexpr signed int i##name = (value);                                                 \
    constexpr MachRegister name{i##name, #name}

#define DEF_REGISTER_ALIAS(name, target)                                                           \
    constexpr signed int i##name = i##target;                                               \
    constexpr MachRegister name{i##name, #name}

#endif
