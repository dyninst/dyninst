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

#include "BPatch.h"
#include "BPatch_function.h"

#include "ast.h"
#include "StackModExpr.h"

using namespace Dyninst;

using genericStackAST = Dyninst::DyninstAPI::genericStackAST;
using stackInsertionAST = Dyninst::DyninstAPI::stackInsertionAST;
using stackRemovalAST = Dyninst::DyninstAPI::stackRemovalAST;


BPatch_stackInsertExpr::BPatch_stackInsertExpr(int size)
{
    ast_wrapper = stackInsertionAST::generic(size);
    assert(BPatch::bpatch != NULL);
}

BPatch_stackRemoveExpr::BPatch_stackRemoveExpr(int size)
{
    ast_wrapper = stackRemovalAST::generic(size);
    assert(BPatch::bpatch != NULL);
}

BPatch_stackMoveExpr::BPatch_stackMoveExpr()
{
    ast_wrapper = genericStackAST::create();
    assert(BPatch::bpatch != NULL);
}

BPatch_canaryExpr::BPatch_canaryExpr()
{
    ast_wrapper = stackInsertionAST::canary(0);

    assert(BPatch::bpatch != NULL);
}

BPatch_canaryCheckExpr::BPatch_canaryCheckExpr(BPatch_function* failureFunc, bool canaryAfterPrologue, long canaryHeight)
{
    ast_wrapper = stackRemovalAST::canary(0, failureFunc->lowlevel_func(), canaryAfterPrologue, canaryHeight);

    assert(BPatch::bpatch != NULL);
}

BPatch_stackRandomizeExpr::BPatch_stackRandomizeExpr()
{
    ast_wrapper = genericStackAST::create();
    assert(BPatch::bpatch != NULL);
}
