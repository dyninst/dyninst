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

#ifndef _BPATCH_PRIVATE_H_
#define _BPATCH_PRIVATE_H_

class BPatch_thread;
class BPatchSnippetHandle;
class BPatch_point;
class BPatch_snippet;

#include <vector>
#include "BPatch_snippet.h"

// TODO: this is bpatch-specific, move to BPatch_private.h?
struct batchInsertionRecord {
    // Thread-specific instru
    BPatch_thread *thread_;
    // For delayed insertion; vector because there is a vector-insert technique
    std::vector<BPatch_point *> points_;
    // This has to be vectorized to handle the multiple-point insertion + edges.
    std::vector<callWhen> when_;
    callOrder order_;
    BPatch_snippet snip; // Make a copy so that the user doesn't have to.
    BPatchSnippetHandle *handle_; // handle to fill in

    bool trampRecursive_;
};


#endif
