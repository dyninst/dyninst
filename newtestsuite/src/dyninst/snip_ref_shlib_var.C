/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id
/*
 * #Name: snip_ref_shlib_var
 * #Desc: use instrumentation to modify variable in shared library
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,i386_unknown_linux2_0_test,alpha_dec_osf4_0_test,ia64_unknown_linux2_4_test,x86_64_unknown_linux2_4_test
 * #Notes: This test uses libNameA/B 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

#ifdef os_windows_test
#define snprintf _snprintf
#endif


class snip_ref_shlib_var_Mutator : public DyninstMutator {
	const char *libNameAroot;
	const char *libNameBroot;
	char libNameA[128], libNameB[128];

	BPatch_snippet *doVarAssign(const char * to, const char *from);
	public:
	snip_ref_shlib_var_Mutator();
	virtual test_results_t executeTest();
	virtual test_results_t mutatorTest();
};

snip_ref_shlib_var_Mutator::snip_ref_shlib_var_Mutator() : libNameAroot("libtestA"),
									   libNameBroot("libtestB") 
{
}

extern "C" DLLEXPORT  TestMutator *snip_ref_shlib_var_factory() 
{
	return new snip_ref_shlib_var_Mutator();
}

BPatch_snippet * snip_ref_shlib_var_Mutator::doVarAssign(const char *to, const char *from) 
{
	BPatch_variableExpr *to_v = appImage->findVariable(to);

	if (!to_v)
	{
		logerror("%s[%d]:  failed to find var %s\n", FILE__, __LINE__, to);
		return NULL;
	}

	BPatch_variableExpr *from_v = appImage->findVariable(from);

	if (!from_v)
	{
		logerror("%s[%d]:  failed to find var %s\n", FILE__, __LINE__, from);
		return NULL;
	}

	BPatch_snippet *ret;
	ret = new BPatch_arithExpr(BPatch_assign, *to_v, *from_v);
	assert(ret);
	return ret;
}

test_results_t snip_ref_shlib_var_Mutator::mutatorTest() 
{
	//  The check function returns 1 on success (value changed as expected)
	//  or 0 on failure.
	const char *check_fname  = "check_snip_ref_shlib_var";
	const char *inst_func_name = "srsv1";
	BPatch_Vector<BPatch_function *> funcs;

	appImage->findFunction(inst_func_name, funcs);

	if (!funcs.size())
	{
		logerror("%s[%d]:  failed to find function %s\n", 
				FILE__, __LINE__, inst_func_name);
		return FAILED;
	}

	BPatch_function *inst_func = funcs[0];

	std::vector<BPatch_point *> *pts= inst_func->findPoint(BPatch_entry);

	if (!pts || !pts->size())
	{
		logerror("%s[%d]:   failed to find entry point to %s\n", 
				FILE__, __LINE__, inst_func_name);
		return FAILED;
	}

	BPatch_point *entry_point = (*pts)[0];

	BPatch_Vector<BPatch_snippet *> allInst;
	BPatch_snippet *snip;

	snip = doVarAssign("gv_srsv1", "snip_ref_shlib_var1");
	if (NULL == snip) return FAILED;
	allInst.push_back(snip);

	snip = doVarAssign("gv_srsv2", "snip_ref_shlib_var2");
	if (NULL == snip) return FAILED;
	allInst.push_back(snip);

	snip = doVarAssign("gv_srsv3", "snip_ref_shlib_var3");
	if (NULL == snip) return FAILED;
	allInst.push_back(snip);

	snip = doVarAssign("gv_srsv4", "snip_ref_shlib_var4");
	if (NULL == snip) return FAILED;
	allInst.push_back(snip);

	snip = doVarAssign("gv_srsv5", "snip_ref_shlib_var5");
	if (NULL == snip) return FAILED;
	allInst.push_back(snip);

#if 0
	snip = doVarAssign("gv_srsv6", "snip_ref_shlib_var6");
	if (NULL == snip) return FAILED;
	allInst.push_back(snip);
#endif

	BPatch_sequence my_ass(allInst);

	if (!appAddrSpace->insertSnippet(my_ass, *entry_point))
	{
		logerror("%s[%d]:  failed to insert snippet\n", FILE__, __LINE__);
		return FAILED;
	}


	return PASSED;
}

test_results_t snip_ref_shlib_var_Mutator::executeTest() 
{
	int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
	pointer_size = pointerSize(appImage);
#endif
	strncpy(libNameA, libNameAroot, 128);
	addLibArchExt(libNameA,128, pointer_size);
	strncpy(libNameB, libNameBroot, 128);
	addLibArchExt(libNameB,128, pointer_size);

	char libA[128], libB[128];
	snprintf(libA, 128, "./%s", libNameA);
	snprintf(libB, 128, "./%s", libNameB);

	if (! appAddrSpace->loadLibrary(libA)) 
	{
		logerror("**Failed snip_ref_shlib_var (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libNameA);
		return FAILED;
	}

	if (! appAddrSpace->loadLibrary(libB)) 
	{
		logerror("**Failed snip_ref_shlib_var (findFunction in module)\n");
		logerror("  Mutator couldn't load %s into mutatee\n", libNameB);
		return FAILED;
	}

	return mutatorTest();
} // snip_ref_shlib_var_Mutator::executeTest()

