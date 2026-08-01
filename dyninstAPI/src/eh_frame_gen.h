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

#ifndef DYNINST_EH_FRAME_GEN_H
#define DYNINST_EH_FRAME_GEN_H

#include <list>
#include <vector>

#include "dyntypes.h"        // Dyninst::Address
#include "eh_frame_arch.h"   // Dyninst::EHFrameArch

namespace Dyninst {

namespace SymtabAPI { class Symtab; }
namespace Relocation { class CodeTracker; }

// Synthesize .eh_frame (and, for functions with a C++ LSDA, .gcc_except_table)
// for the relocated code described by `relocatedCode`, and install them into
// `symObj` as loadable regions plus a DT_DYNINST_EH_FRAME dynamic tag that the
// runtime library reads and hands to __register_frame. This is what lets a C++
// exception be unwound through, and caught in, relocated code.
//
// `arch` supplies the per-architecture CFI parameters (see EHFrameArch);
// `regionHighWaterMark` is the end of the relocated-code section, used to place
// the new regions above it. Intended for dynamic rewriting only (a static
// binary links the runtime at emit time and would mis-relocate its reference to
// the region), and the caller is responsible for that gating.
void synthesizeRelocatedEHFrame(SymtabAPI::Symtab *symObj,
                                std::list<Relocation::CodeTracker *> &relocatedCode,
                                const EHFrameArch &arch,
                                Address regionHighWaterMark,
                                Address ownLo = 0, Address ownHi = 0,
                                Address loadBias = 0);

// Build the .eh_frame / .gcc_except_table bytes for the relocated code, placed
// at addresses derived from `regionBase`, WITHOUT delivering them. Pure and
// re-runnable: the byte lengths are independent of the placement addresses, so
// a caller that must allocate the region first can call once with a placeholder
// base to learn the sizes, allocate, then call again with the real base. Returns
// the number of FDEs emitted; the two blobs and the placement addresses chosen
// for them come back through the out-parameters. Used by both the rewriter
// (synthesizeRelocatedEHFrame) and the live-process delivery (PCProcess).
// `ownLo`/`ownHi`, when non-empty (ownHi > ownLo), restrict synthesis to code
// whose ORIGINAL address lies in [ownLo, ownHi) -- the instrumented
// application's own range. Foreign functions that dynamic instrumentation
// incidentally relocated (e.g. libc helpers pulled in by the iRPC/runtime
// machinery) are skipped: we have no source CFI for them, so an emitted FDE
// would carry only the CIE default rules -- a fabricated, wrong descriptor that
// can corrupt the whole __register_frame batch and defeat the application's own
// (correct) FDEs. Pass runtime addresses so the filter is PIE-safe.
unsigned buildRelocatedEHFrame(SymtabAPI::Symtab *symObj,
                               std::list<Relocation::CodeTracker *> &relocatedCode,
                               const EHFrameArch &arch,
                               Address regionBase,
                               std::vector<unsigned char> &ehOut,
                               std::vector<unsigned char> &exOut,
                               Address &ehVaddrOut,
                               Address &exVaddrOut,
                               Address ownLo = 0, Address ownHi = 0,
                               // Load bias (mapped_object::codeBase()): runtime = link + loadBias.
                               // The CodeTrackers carry RUNTIME original addresses, but the DWARF
                               // CFI (getCFALocations) and LSDA (getEHFrameInfo) are read at
                               // LINK-time addresses. For a PIE they differ; without the bias every
                               // CFA lookup and LSDA assignment misses (0 rules, 0 landing pads) and
                               // exceptions escape. 0 for non-PIE / the static rewriter.
                               Address loadBias = 0);

}  // namespace Dyninst

#endif  // DYNINST_EH_FRAME_GEN_H
