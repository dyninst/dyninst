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

// $Id: test1_30.C,v 1.1 2008/10/30 19:18:55 legendre Exp $
/*
 * #Name: test1_30
 * #Desc: Line Information
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,i386_unknown_solaris2_5_test,i386_unknown_linux2_0_test,ia64_unknown_linux2_4_test,i386_unknown_nt4_0_test,rs6000_ibm_aix4_1_test,alpha_dec_osf4_0_test,x86_64_unknown_linux2_4_test
 * #Notes:
 */

#include <iostream>

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_statement.h"

#include "test_lib.h"

using namespace std;

#include "dyninst_comp.h"
class test1_30_Mutator : public DyninstMutator {
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test1_30_factory() {
  return new test1_30_Mutator();
}

//
// Start Test Case #30 - (line information)
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_30_Mutator::executeTest() {
#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0_test) \
 || defined(rs6000_ibm_aix4_1_test) \
 || defined(os_linux_test) /* Use OS #define instead of platform - Greg */ \
 || defined(os_freebsd_test)
  unsigned long n;
  unsigned long baseAddr,lastAddr;
  unsigned int call30_1_line_no;
  unsigned short lineNo;
  char fileName[256];

  if (isMutateeFortran(appImage)) {
    return SKIPPED;
  } 

  const char *funcName = "test1_30_mutatee";
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) {
      logerror("    Unable to find function %s\n", funcName);
      return FAILED;
    }

    if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), funcName);
    }


    BPatch_Vector<BPatch_point *> *point30_1 = found_funcs[0]->findPoint(BPatch_entry);
	//instrument with the function that will set the line number

	if (!point30_1 || (point30_1->size() < 1)) {
		logerror("Unable to find point %s - entry.\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	const char *fn = "test1_30_call1";
	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  logerror("    Unable to find function %s\n", fn);
	  return FAILED;
	}
	
	BPatch_function *call30_1func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> nullArgs;
	BPatch_funcCallExpr call30_1Expr(*call30_1func, nullArgs);

	checkCost(call30_1Expr);
    	appAddrSpace->insertSnippet(call30_1Expr, *point30_1);

	//get the line number of the function call30_1
	BPatch_variableExpr *expr30_7 = 
		appImage->findVariable("test1_30_globalVariable7");
	if (expr30_7 == NULL) {
        	logerror("**Failed** test #30 (line information)\n");
        	logerror("    Unable to locate test1_30_globalVariable7\n");
        	return FAILED;
    	}
	expr30_7->readValue(&n);
	call30_1_line_no = (unsigned)(n+1);

        call30_1func->getAddressRange(baseAddr, lastAddr);

	//now write the base address and last address of the function
	BPatch_variableExpr *expr30_8 = 
			appImage->findVariable("test1_30_globalVariable8");
	if (expr30_8 == NULL) {
		logerror("**Failed** test #30 (line information)\n");
		logerror("    Unable to locate test1_30_globalVariable8\n");
	}

	BPatch_variableExpr *expr30_9 = 
			appImage->findVariable("test1_30_globalVariable9");
	if (expr30_9 == NULL) {
		logerror("**Failed** test #30 (line information)\n");
		logerror("    Unable to locate test1_30_globalVariable9\n");
	}

	expr30_8->writeValue(&baseAddr);
	expr30_9->writeValue(&lastAddr);
	
	
	//check getLineAddr for appImage
	BPatch_variableExpr *expr30_3 =
			appImage->findVariable("test1_30_globalVariable3");
	if (expr30_3 == NULL) {
        	logerror("**Failed** test #30 (line information)\n");
        	logerror("    Unable to locate test1_30_globalVariable3\n");
        	return FAILED;
    	}

    	
        std::vector< std::pair< unsigned long, unsigned long > > ranges;
        if( appImage->getAddressRanges( "test1_30_mutatee.c", call30_1_line_no, ranges ) ) {
    	    n = ranges[0].first;
    	    expr30_3->writeValue( & n );
    	}
    	


	//check getLineAddr for module
	BPatch_variableExpr *expr30_4 =
			appImage->findVariable("test1_30_globalVariable4");
	if (expr30_4 == NULL) {
        	logerror("**Failed** test #30 (line information)\n");
        	logerror("    Unable to locate test1_30_globalVariable4\n");
        	return FAILED;
    	}
	BPatch_Vector<BPatch_module*>* appModules = appImage->getModules();
	for(unsigned int i=0;i<appModules->size();i++){
	  char mname[256];
	  (*appModules)[i]->getName(mname,255);mname[255] = '\0';
	  if(!strncmp(mname,"test1_30_mutatee.c",15)){
	    ranges.clear();
	    if( (*appModules)[i]->getAddressRanges( NULL, call30_1_line_no, ranges ) ) {
	      n = ranges[0].first;
	      expr30_4->writeValue( & n );
	    }
	    else
	      logerror("BPatch_module->getAddressRanges returned false!\n");
	    break;
	  }
	}

	//check getLineAddr works for the function
	BPatch_variableExpr *expr30_5 =
		appImage->findVariable("test1_30_globalVariable5");
	if (expr30_5 == NULL) {
        	logerror("**Failed** test #30 (line information)\n");
        	logerror("    Unable to locate test1_30_globalVariable5\n");
        	return FAILED;
	}
	//check whether getLineFile works for appThread
	BPatch_variableExpr *expr30_6 =
		appImage->findVariable("test1_30_globalVariable6");
	if (expr30_6 == NULL) {
        	logerror("**Failed** test #30 (line information)\n");
        	logerror("    Unable to locate test1_30_globalVariable6\n");
        	return FAILED;
	}
	/* since the first line address of a function changes with the
	   compiler type (gcc,native) we need to check with next address
	   etc. Instead I use the last address of the function*/

	//std::vector< std::pair< const char *, unsigned int > > lines;
        BPatch_Vector<BPatch_statement> lines;
	if (appImage->getSourceLines( lastAddr - 1, lines)) {
		//n = lines[0].second;
		n = lines[0].lineNumber();
		expr30_6->writeValue( & n );
	}
	else {
	  logerror("appThread->getLineAndFile returned false!\n");
	}
        return PASSED;
#else
	return SKIPPED;
#endif
}
