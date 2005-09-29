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

// $Id: test1_30.C,v 1.1 2005/09/29 20:38:36 bpellin Exp $
/*
 * #Name: test1_30
 * #Desc: Line Information
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,i386_unknown_solaris2_5,i386_unknown_linux2_0,ia64_unknown_linux2_4,i386_unknown_nt4_0,rs6000_ibm_aix4_1,alpha_dec_osf4_0,x86_64_unknown_linux2_4
 * #Notes:
 */

#include <iostream>

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

using namespace std;

int mutateeFortran;

//
// Start Test Case #30 - (line information)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) \
 || defined(i386_unknown_nt4_0) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(alpha_dec_osf4_0)

  unsigned long n;
  unsigned long baseAddr,lastAddr;
  unsigned int call30_1_line_no;
  unsigned short lineNo;
  char fileName[256];

	if (mutateeFortran) {
	    return 0;
	} 
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func30_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func30_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func30_1");
    }


    BPatch_Vector<BPatch_point *> *point30_1 = found_funcs[0]->findPoint(BPatch_entry);
	//instrument with the function that will set the line number

	if (!point30_1 || (point30_1->size() < 1)) {
		fprintf(stderr, "Unable to find point func30_1 - entry.\n");
		return -1;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	char *fn = "call30_1";
	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  fprintf(stderr, "    Unable to find function %s\n", fn);
	  return -1;
	}
	
	BPatch_function *call30_1func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> nullArgs;
	BPatch_funcCallExpr call30_1Expr(*call30_1func, nullArgs);

	checkCost(call30_1Expr);
    	appThread->insertSnippet(call30_1Expr, *point30_1);

	//get the line number of the function call30_1
	BPatch_variableExpr *expr30_7 = 
		appImage->findVariable("globalVariable30_7");
	if (expr30_7 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_7\n");
        	return -1;
    	}
	expr30_7->readValue(&n);
	call30_1_line_no = (unsigned)(n+1);

	//get the base addr and last addr of the function call30_1
	baseAddr = (unsigned long)(call30_1func->getBaseAddr());
	lastAddr = baseAddr + call30_1func->getSize();

	//now write the base address and last address of the function
	BPatch_variableExpr *expr30_8 = 
			appImage->findVariable("globalVariable30_8");
	if (expr30_8 == NULL) {
		fprintf(stderr, "**Failed** test #30 (line information)\n");
		fprintf(stderr, "    Unable to locate globalVariable30_8\n");
	}

	BPatch_variableExpr *expr30_9 = 
			appImage->findVariable("globalVariable30_9");
	if (expr30_9 == NULL) {
		fprintf(stderr, "**Failed** test #30 (line information)\n");
		fprintf(stderr, "    Unable to locate globalVariable30_9\n");
	}

	expr30_8->writeValue(&baseAddr);
	expr30_9->writeValue(&lastAddr);
	
	
	//check getLineAddr for appImage
	BPatch_variableExpr *expr30_3 =
			appImage->findVariable("globalVariable30_3");
	if (expr30_3 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_3\n");
        	return -1;
    	}

    	
        std::vector< std::pair< unsigned long, unsigned long > > ranges;
        if( appImage->getAddressRanges( "test1.mutatee.c", call30_1_line_no, ranges ) ) {
    	    n = ranges[0].first;
    	    expr30_3->writeValue( & n );
    	}
    	


	//check getLineAddr for module
	BPatch_variableExpr *expr30_4 =
			appImage->findVariable("globalVariable30_4");
	if (expr30_4 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_4\n");
        	return -1;
    	}
	BPatch_Vector<BPatch_module*>* appModules = appImage->getModules();
	for(unsigned int i=0;i<appModules->size();i++){
		char mname[256];
		(*appModules)[i]->getName(mname,255);mname[255] = '\0';
		if(!strncmp(mname,"test1.mutatee.c",15)){
			ranges.clear();
			if( (*appModules)[i]->getAddressRanges( NULL, call30_1_line_no, ranges ) ) {
				n = ranges[0].first;
				expr30_4->writeValue( & n );
			}
			else cerr << "BPatch_module->getLineToAddr returned false!" << endl;
			break;
		}
	}

	//check getLineAddr works for the function
	BPatch_variableExpr *expr30_5 =
		appImage->findVariable("globalVariable30_5");
	if (expr30_5 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_5\n");
        	return -1;
	}
	//check whether getLineFile works for appThread
	BPatch_variableExpr *expr30_6 =
		appImage->findVariable("globalVariable30_6");
	if (expr30_6 == NULL) {
        	fprintf(stderr, "**Failed** test #30 (line information)\n");
        	fprintf(stderr, "    Unable to locate globalVariable30_6\n");
        	return -1;
	}
	/* since the first line address of a function changes with the
	   compiler type (gcc,native) we need to check with next address
	   etc. Instead I use the last address of the function*/
	std::vector< std::pair< const char *, unsigned int > > lines;
	if( appThread->getSourceLines( lastAddr - 1, lines ) ) {
		n = lines[0].second;
		expr30_6->writeValue( & n );
		}
	else cerr << "appThread->getLineAndFile returned false!" << endl;
#endif

        return 0;
}

// External Interface
extern "C" int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    mutateeFortran = isMutateeFortran(appImage);

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
