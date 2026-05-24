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
#include <stdio.h>

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "image.h"
#include "parsing/parse_block.h"
#include "parsing/parse_func.h"
#include "patching/instPoint.h"
#include "parsing/Parsing.h"
#include "debug.h"
#include "BPatch.h"
#include "mapped_object.h"
#include "dynproc/dynProcess.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

// windows.h pollutes namespace with min/max macros
#undef min
#undef max

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif


void
DynParseCallback::abruptEnd_cf(Address /*addr*/,ParseAPI::Block *b,default_details*)
{
  static_cast<parse_block*>(b)->setAbruptEnd(true);
}

void
DynParseCallback::newfunction_retstatus(Function *func)
{
    dynamic_cast<parse_func*>(func)->setinit_retstatus( func->retstatus() );
}

void DynParseCallback::destroy_cb(Block *b) {
   _img->destroy(b);
}

void DynParseCallback::destroy_cb(Edge *e) {
   _img->destroy(e);
}

void DynParseCallback::destroy_cb(Function *f) {
   _img->destroy(f);
}

void DynParseCallback::remove_edge_cb(ParseAPI::Block *, ParseAPI::Edge *, edge_type_t) {
   //cerr << "Warning: edge removal callback unimplemented" << endl;
}

void DynParseCallback::remove_block_cb(ParseAPI::Function *, ParseAPI::Block *) {
   // we currently do all necessary cleanup during destroy
   //cerr << "Warning: block removal callback unimplemented" << endl;
}

void
DynParseCallback::patch_nop_jump(Address addr)
{
    Architecture arch = _img->codeObject()->cs()->getArch();
    assert( Arch_x86 == arch || Arch_x86_64 == arch );

    unsigned char * ptr = (unsigned char *) 
        _img->codeObject()->cs()->getPtrToInstruction(addr);
    ptr[0] = 0x90;
}

void
DynParseCallback::interproc_cf(Function*f,Block *b,Address /*addr*/,interproc_details*det)
{
    (void) f; // compiler warning
    if (det->type == ParseCallback::interproc_details::unresolved) {
        static_cast<parse_block*>(b)->setUnresolvedCF(true);
    }
}

void
DynParseCallback::overlapping_blocks(Block*b1,Block*b2)
{
    parsing_printf("[%s:%d] blocks [%lx,%lx) and [%lx,%lx) overlap"
                   "inconsistently\n",
        FILE__,__LINE__,b1->start(),b1->end(),b2->start(),b2->end());
    static_cast<parse_block*>(b1)->markAsNeedingRelocation();
    static_cast<parse_block*>(b2)->markAsNeedingRelocation();
}

extern bool codeBytesUpdateCB(void *objCB, Address targ);
bool 
DynParseCallback::updateCodeBytes(Address target_)
{   // calls function that updates bytes if needed
    assert(BPatch_normalMode != _img->hybridMode());
    return codeBytesUpdateCB( _img->cb_arg0(), 
                              target_ + _img->desc().code() );
}

bool
DynParseCallback::hasWeirdInsns(const ParseAPI::Function* func) const
{
    return static_cast<parse_func*>
        (const_cast<ParseAPI::Function*>
            (func))->hasWeirdInsns();
}

void 
DynParseCallback::foundWeirdInsns(ParseAPI::Function* func)
{
    static_cast<parse_func*>(func)->setHasWeirdInsns(true);
}

void DynParseCallback::split_block_cb(ParseAPI::Block *first, ParseAPI::Block *second)
{
    if (SCAST_PB(first)->abruptEnd()) {
        SCAST_PB(first)->setAbruptEnd(false);
        SCAST_PB(second)->setAbruptEnd(true);
    }
    if (SCAST_PB(first)->unresolvedCF()) {
        SCAST_PB(first)->setUnresolvedCF(false);
        SCAST_PB(second)->setUnresolvedCF(true);
    }
    if (SCAST_PB(first)->needsRelocation()) {
        SCAST_PB(second)->markAsNeedingRelocation();
    }
    //parse_block::canBeRelocated_ doesn't need to be set, it's only ever
    // true for the sink block, which is never split
}
