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

// $Id: test1_40.C,v 1.1 2008/10/30 19:19:35 legendre Exp $
/*
 * #Name: test1_40
 * #Desc: Verify that we can monitor call sites
 * #Dep: 
 * #Arch: !,alpha_dec_osf4_0_test,ia64_unknown_linux2_4_test,!mips_sgi_irix6_4_test,os_windows
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_40_Mutator : public DyninstMutator {
	virtual test_results_t setup(ParameterDict &param);
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_40_factory() 
{
	return new test1_40_Mutator();
}

//
//  Test case 40:  verify that we can monitor call sites
//

static BPatch_function *findFunction40(const char *fname, 
		BPatch_image *appImage)
{
	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction(fname, bpfv) || (bpfv.size() != 1)) 
	{

		logerror("**Failed test #40 (monitor call sites)\n");
		logerror("  Expected 1 functions matching %s, got %d\n",
				fname, bpfv.size());
		return NULL;
	}
	return bpfv[0];
}

static int setVar40(const char *vname, void *value, BPatch_image *appImage)
{
	BPatch_variableExpr *v;
	void *buf = value;

	if (NULL == (v = appImage->findVariable(vname))) 
	{
		logerror("**Failed test #40 (monitor call sites)\n");
		logerror("  cannot find variable %s\n", vname);
		return -1;
	}

	// Get around endianness on cross address-width mutators.
	// Note: Can't use reinterpret_cast here.  G++ produces an error:
	//   reinterpret_cast from `void*' to `unsigned int' loses precision

	unsigned long longAddr = (unsigned long)(value);
	unsigned int shortAddr = (unsigned  int)(unsigned long)(value);

	switch (v->getSize()) {
		case 4: buf = reinterpret_cast<void *>(&shortAddr); break;
		case 8: buf = reinterpret_cast<void *>(&longAddr);  break;
		default: assert(0 && "Invalid size of mutatee address variable");
	}

	// Done silly casting magic.  Write the value.
	if (! v->writeValue(buf, sizeof(unsigned int),false)) 
	{
		logerror("**Failed test #40 (monitor call sites)\n");
		logerror("  failed to write call site var to mutatee\n");
		return -1;
	}

	return 0;
}

#ifdef RETURNONFAIL
#undef RETURNONFAIL
#endif
#define RETURNONFAIL(expr) if((expr) < 0) return FAILED

test_results_t test1_40_Mutator::executeTest() 
{
	const char *monitorFuncName = "test1_40_monitorFunc";
	const char *callSiteAddrVarName = "test1_40_callsite5_addr";

	BPatch_function *monitorFunc = NULL;
	BPatch_Vector<BPatch_function *> bpfv;

	BPatch_function *call40_1 = findFunction40("test1_40_call1", appImage);
	RETURNONNULL(call40_1);
	RETURNONFAIL(setVar40("test1_40_addr_of_call1", call40_1->getBaseAddr(),appImage));

	BPatch_function *call40_2 = findFunction40("test1_40_call2", appImage);
	RETURNONNULL(call40_2);
	RETURNONFAIL(setVar40("test1_40_addr_of_call2", call40_2->getBaseAddr(),appImage));

	BPatch_function *call40_3 = findFunction40("test1_40_call3", appImage);
	RETURNONNULL(call40_3);
	RETURNONFAIL(setVar40("test1_40_addr_of_call3", call40_3->getBaseAddr(),appImage));

	//  call40_5 is the "dispatcher" of function pointers
	BPatch_function *targetFunc = findFunction40("test1_40_call5", appImage);
	RETURNONNULL(targetFunc);
	//RETURNONFAIL(setVar40("test1_40_addr_of_call5", call40_5->getBaseAddr(),appImage));

	monitorFunc = findFunction40(monitorFuncName, appImage);
	RETURNONNULL(monitorFunc);

	BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
	if (!calls) 
	{
		logerror("**Failed test #40 (monitor call sites)\n");
		logerror("  cannot find call points for test1_40_call5\n");
		return FAILED;
	}

	BPatch_Vector<BPatch_point *> dyncalls;
	for (unsigned int i = 0; i < calls->size(); ++i) 
	{
		BPatch_point *pt = (*calls)[i];
		if (pt->isDynamic())
			dyncalls.push_back(pt);
	}

	if (dyncalls.size() != 1) 
	{
		logerror("**Failed test #40 (monitor call sites)\n");
		logerror("  wrong number of dynamic points found (%d -- not 1)\n",
				dyncalls.size());
		logerror("  total number of calls found: %d\n", calls->size());
		return FAILED;
	}

	// write address of anticipated call site into mutatee var.
	void *callsite_address = dyncalls[0]->getAddress();
	RETURNONFAIL(setVar40(callSiteAddrVarName, callsite_address, appImage));

	//  issue command to monitor calls at this site, and we're done.
	if (! dyncalls[0]->monitorCalls(monitorFunc)) 
	{
		logerror("**Failed test #40 (monitor call sites)\n");
		logerror("  cannot monitor calls\n");
		return FAILED;
	}

	return PASSED;
}

// External Interface

test_results_t test1_40_Mutator::setup(ParameterDict &param) 
{
	bool createmode = param["createmode"]->getInt() == USEATTACH;
	appThread = (BPatch_thread *)(param["appThread"]->getPtr());
        appProc = appThread->getProcess();
	int mutateeXLC = param["mutateeXLC"]->getInt();

	// xlc does not produce the intended dynamic call points for this example
	if (mutateeXLC) 
	{
		return SKIPPED;
	}

	// Read the program's image and get an associated image object
        appImage = appProc->getImage();

	if (isMutateeFortran(appImage)) 
	{
		return SKIPPED;
	}

	if ( createmode == USEATTACH )
	{
		if ( ! signalAttached(appImage) )
			return FAILED;
	}

	// Run mutator code
	return PASSED;
}
