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

#ifndef DYNINST_COMMON_ABI_DEFINITIONS_H
#define DYNINST_COMMON_ABI_DEFINITIONS_H

#include "registers/register_set.h"

namespace Dyninst { namespace abi {

  // The register usage rules of one calling sequence (function call or
  // system call), as written in an architecture's ABI document.
  struct convention {
    // Registers used to pass arguments.
    register_set params;

    // Registers used to return values. For the system calling sequence this
    // only documents the ABI: no liveness set is derived from it.
    register_set returns;

    // Registers whose value survives the call ("callee-saved", "nonvolatile").
    register_set preserved;

    // Registers with an ABI-wide role that any function may read and that are
    // still meaningful after it returns (e.g. the aarch64 link register, a
    // thread pointer).
    register_set globals;
  };

  struct definition {
    convention function;
    convention syscall;
  };

}}

#endif
