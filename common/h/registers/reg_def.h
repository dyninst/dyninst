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
#if defined(DYN_DEFINE_REGS)
// DYN_DEFINE_REGS Should only be defined in libcommon.
// We want one definition, which will be in libcommon, and declarations
// for everyone else.
//
// I wanted these to be const MachRegister objects, but that changes the
// linker scope.  Instead they're non-const.  Every accessor function is
// const anyways, so we'll just close our eyes and pretend they're declared
// const.
#  define DEF_REGISTER(name, value, Arch)                                                          \
    const signed int i##name = (value);                                                            \
    COMMON_EXPORT MachRegister name(i##name, Arch "::" #name)
#else
#  define DEF_REGISTER(name, value, Arch)                                                          \
    const signed int i##name = (value);                                                            \
    COMMON_EXPORT extern MachRegister name

#endif

#endif
