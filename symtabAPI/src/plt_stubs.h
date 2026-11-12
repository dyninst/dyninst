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

#ifndef SYMTABAPI_SRC_PLT_STUBS_H
#define SYMTABAPI_SRC_PLT_STUBS_H

#include "Architecture.h"
#include "dyninst_visibility.h"
#include "dyntypes.h"

#include <cstddef>

namespace Dyninst {
namespace SymtabAPI {

// The GOT slot that the code at addr reads a PLT stub's jump target from, or
// Dyninst::ADDRESS_INVALID if the code there does not read one.  buf holds the
// code, len bytes of it.  On aarch64 the code is
//
//     adrp x16, page of the slot
//     ldr  x17, [x16, #offset of the slot in that page]
//
// Other architectures give ADDRESS_INVALID.
DYNINST_EXPORT Address plt_stub_got_slot(const unsigned char *buf, std::size_t len,
                                         Address addr, Architecture arch);

}
}

#endif
