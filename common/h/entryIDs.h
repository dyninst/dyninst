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

#if !defined(ENTRYIDS_IA32_H)
#define ENTRYIDS_IA32_H

#include <string>
#include "dyntypes.h"
#include "util.h"

/* clang-format off */
enum entryID : unsigned int {
  #include "mnemonics/x86_entryIDs.h"
  #include "mnemonics/ppc_entryIDs.h"
  #include "mnemonics/aarch64_entryIDs.h"
  #include "mnemonics/AMDGPU/gfx908_entryIDs.h"
  #include "mnemonics/AMDGPU/gfx90a_entryIDs.h"
  #include "mnemonics/AMDGPU/gfx940_entryIDs.h"
  #include "mnemonics/NVIDIA/generic_entryIDs.h"
  #include "mnemonics/IntelGPU/generic_entryIDs.h"
  _entry_ids_max_
};
/* clang-format on */

enum prefixEntryID : unsigned int {
  prefix_none,
  prefix_rep,
  prefix_repnz
};

namespace NS_x86 {
COMMON_EXPORT extern dyn_hash_map<entryID, std::string> entryNames_IAPI;
COMMON_EXPORT extern dyn_hash_map<prefixEntryID, std::string> prefixEntryNames_IAPI;
}

#endif
