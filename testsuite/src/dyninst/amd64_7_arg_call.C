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

// $Id: amd64_7_arg_call.C,v 1.1 2008/10/30 19:18:17 legendre Exp $
/*
 * #Name: amd64_7_arg_call
 * #Desc: Mutator Side (call a seven argument function; this will exercise parameter passing via stack)
 * #Arch: AMD64
 * #Dep: 
 * #Notes: Uses mutateeFortran variable
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class amd64_7_arg_call_Mutator : public DyninstMutator {
        virtual DLLEXPORT test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator *amd64_7_arg_call_factory()
{
    return new amd64_7_arg_call_Mutator();
}

//
// mutator side (call a seven argument function)
//

test_results_t DLLEXPORT amd64_7_arg_call_Mutator::executeTest()
{
    const char *funcName = "amd64_7_arg_call_func";
    const char* testName = "seven parameter function";

        // Find the entry point to the procedure "amd64_7_arg_call_func"

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(funcName, found_funcs))
         || !found_funcs.size())
    {
        logerror("\tUnable to find function %s\n",
                 funcName);
        return FAILED;
    }

    if (1 < found_funcs.size())
    {
        logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n",
                 __FILE__, __LINE__, found_funcs.size(), funcName);
    }

    BPatch_Vector<BPatch_point *> *point = found_funcs[0]->findPoint(BPatch_entry);

    if (!point || ((*point).size() == 0))
    {
        logerror("**Failed** test for %s\n", testName);
        logerror("    Unable to find entry point to \"%s.\"\n", funcName);
        return FAILED;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    const char *fn = "amd64_7_arg_call";

    if (NULL == appImage->findFunction(fn, bpfv)
        || !bpfv.size()
        || NULL == bpfv[0])
    {
        logerror("**Failed** test for %s\n", testName);
        logerror("    Unable to find function %s\n", fn);
        return FAILED;
    }

    BPatch_function *call_func = bpfv[0];


        BPatch_Vector<BPatch_snippet *> call_args;

        BPatch_constExpr expr2_1 (0), expr2_2 (1), expr2_3 (2), expr2_4 (3);
        BPatch_constExpr expr2_5 (4), expr2_6 (5), expr2_7 (6);


        call_args.push_back(&expr2_1);
        call_args.push_back(&expr2_2);
        call_args.push_back(&expr2_3);
        call_args.push_back(&expr2_4);
        call_args.push_back(&expr2_5);
        call_args.push_back(&expr2_6);
        call_args.push_back(&expr2_7);

        BPatch_funcCallExpr callExpr(*call_func, call_args);

        appAddrSpace->insertSnippet(callExpr, *point, BPatch_callBefore, BPatch_lastSnippet);

        return PASSED;
}
