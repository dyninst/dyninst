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

// $Id: test6_2.C,v 1.1 2005/09/29 20:39:34 bpellin Exp $
/*
 * #Name: test6_2
 * #Desc: Store Instrumentation
 * #Dep: 
 * #Arch: !(sparc_sun_solaris2_4,,rs6000_ibm_aix4_1,i386_unknown_linux2_0,x86_64_unknown_linux2_4,i386_unknown_nt4_0,ia64_unknown_linux2_4)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test6.h"


#ifdef sparc_sun_solaris2_4
const unsigned int nstores = 13;
BPatch_memoryAccess* storeList[nstores];

void init_test_data()
{
  int k=-1;

  storeList[++k] = MK_LS(3,17,-1,1); // ldstub
  storeList[++k] = MK_LS(0,17,-1,4); // cas
  storeList[++k] = MK_LS(0,17,-1,8); // casx
  storeList[++k] = MK_LS(0,17,0,4);  // swap

  storeList[++k] = MK_ST(7,21,-1,1);
  storeList[++k] = MK_ST(6,21,-1,2);
  storeList[++k] = MK_ST(4,21,-1,4);
  storeList[++k] = MK_ST(0,21,0,8);
  storeList[++k] = MK_ST(0,17,0,4);
  storeList[++k] = MK_ST(0,17,0,8);
  storeList[++k] = MK_ST(0,18,0,16);
  storeList[++k] = MK_ST(4,21,-1,4);
  storeList[++k] = MK_ST(0,21,0,8);
}
#endif

#ifdef rs6000_ibm_aix4_1
const unsigned int nstores = 32;
BPatch_memoryAccess* storeList[nstores];

void init_test_data()
{
  int k=-1;

  storeList[++k] = MK_ST(3, 7, -1, 1);
  storeList[++k] = MK_ST(1, 7, -1, 1);
  storeList[++k] = MK_ST(0, 3, 8, 1);
  storeList[++k] = MK_ST(0, 8, 3, 1);

  storeList[++k] = MK_ST(2, 7, -1, 2);
  storeList[++k] = MK_ST(6, 7, -1, 2);
  storeList[++k] = MK_ST(0, 3, 9, 2);
  storeList[++k] = MK_ST(0, 9, 3, 2);

  storeList[++k] = MK_ST(0, 7, -1, 4);
  storeList[++k] = MK_ST(4, 7, -1, 4);
  storeList[++k] = MK_ST(0, 3, 9, 4);
  storeList[++k] = MK_ST(0, 9, 3, 4);

  storeList[++k] = MK_ST(0, 7, -1, 8);
  storeList[++k] = MK_ST(0, 7, -1, 8);
  storeList[++k] = MK_ST(0, 7, 8, 8);
  storeList[++k] = MK_ST(0, 8, 7, 8);

  storeList[++k] = MK_ST(0, 8, 7, 2);
  storeList[++k] = MK_ST(0, 9, 7, 4);

  storeList[++k] = MK_ST(-76, 1, -1, 76);
  storeList[++k] = MK_ST(0, 4, -1, 20);

  storeList[++k] = new BPatch_memoryAccess("", 0, 
				    false, true,
				    0, 1, 9,
				    0, POWER_XER2531, -1);

  storeList[++k] = MK_ST(0, -1, 3, 4); // 0 means -1 (no register) in ra
  storeList[++k] = MK_ST(0, -1, 7, 8);

  storeList[++k] = MK_ST(4, 4, -1, 4); // s24
  storeList[++k] = MK_ST(0, 4, 0, 4);
  storeList[++k] = MK_ST(0, 4, -1, 4);
  storeList[++k] = MK_ST(0, -1, 4, 4);  // 0 means -1 (no register) in ra
  storeList[++k] = MK_ST(0, 6, -1, 8);
  storeList[++k] = MK_ST(0, 6, 9, 8);
  storeList[++k] = MK_ST(8, 6, -1, 8);
  storeList[++k] = MK_ST(0, 9, 7, 8);

  storeList[++k] = MK_ST(0, -1, 4, 4);  // 0 means -1 (no register) in ra
}
#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
const unsigned int nstores = 23;
BPatch_memoryAccess* storeList[nstores];

void *divarwp, *dfvarsp, *dfvardp, *dfvartp, *dlargep;

void get_vars_addrs(BPatch_image* bip) // from mutatee
{
#ifdef i386_unknown_nt4_0
  // FIXME: With or without leading _ dyninst cannot find these variables.
  // VC++6 debugger has no such problems...
  BPatch_variableExpr* bpvep_diwarw = bip->findVariable("_divarw");
  BPatch_variableExpr* bpvep_diwars = bip->findVariable("_dfvars");
  BPatch_variableExpr* bpvep_diward = bip->findVariable("_dfvard");
  BPatch_variableExpr* bpvep_diwart = bip->findVariable("_dfvart");
  BPatch_variableExpr* bpvep_dlarge = bip->findVariable("_dlarge");
#else
  BPatch_variableExpr* bpvep_diwarw = bip->findVariable("divarw");
  BPatch_variableExpr* bpvep_diwars = bip->findVariable("dfvars");
  BPatch_variableExpr* bpvep_diward = bip->findVariable("dfvard");
  BPatch_variableExpr* bpvep_diwart = bip->findVariable("dfvart");
  BPatch_variableExpr* bpvep_dlarge = bip->findVariable("dlarge");
#endif
  
  divarwp = bpvep_diwarw->getBaseAddr();
  dfvarsp = bpvep_diwars->getBaseAddr();
  dfvardp = bpvep_diward->getBaseAddr();
  dfvartp = bpvep_diwart->getBaseAddr();
  dlargep = bpvep_dlarge->getBaseAddr();
}

void init_test_data()
{
  int k=-1;

  storeList[++k] = MK_ST(-4,4,-1,4);
  storeList[++k] = MK_ST(-4,4,-1,4);
  storeList[++k] = MK_ST(-4,4,-1,4);
  storeList[++k] = MK_ST(-4,4,-1,4);
  storeList[++k] = MK_ST(-4,4,-1,4);

  storeList[++k] = MK_LS((long)divarwp+4,-1,-1,4); // s6
  storeList[++k] = MK_ST((long)divarwp+4,-1,-1,4);
  storeList[++k] = MK_LS((long)divarwp,-1,-1,4);
  storeList[++k] = MK_LS((long)divarwp+4,-1,-1,4);
  storeList[++k] = MK_ST((long)divarwp,-1,-1,4);   // s10
  storeList[++k] = MK_LS((long)divarwp+4,-1,-1,4);

  storeList[++k] = MK_STnt((long)divarwp,-1,-1,8); // s12
  //storeList[++k] = MK_ST(0,7,-1,4);
  storeList[++k] = new BPatch_memoryAccess("", 0, 
					   false, true,
                                           0, 7, -1, 0,
                                           0, -1, 1, 2,
                                           -1, false);
  storeList[++k] = MK_ST(0,7,-1,4);
  //storeList[++k] = MK_SL2(0,7,-1,4,0,6,-1,4); // s15
  storeList[++k] = MK_SL2vECX(0,7,-1,0,6,-1,2);
  
  storeList[++k] = MK_ST((long)dfvarsp,-1,-1,4);
  storeList[++k] = MK_ST((long)dfvardp,-1,-1,8);
  storeList[++k] = MK_ST((long)dfvartp,-1,-1,10);
  storeList[++k] = MK_ST((long)divarwp+2,-1,-1,2);
  storeList[++k] = MK_ST((long)divarwp+4,-1,-1,4);
  storeList[++k] = MK_ST((long)divarwp+8,-1,-1,8);

  storeList[++k] = MK_ST((long)divarwp,-1,-1,2);
  storeList[++k] = MK_ST((long)dlargep,-1,-1,28);
}
#endif

#ifdef ia64_unknown_linux2_4
const unsigned int nstores = 3;
BPatch_memoryAccess* storeList[nstores];
void init_test_data() {
	storeList[0] = MK_ST( 0, 14, -1, 8 );
	storeList[1] = MK_ST( 0, 14, -1, 8 );
	storeList[2] = MK_ST( 0, 14, -1, 8 );
}
#endif

#ifdef mips_sgi_irix6_4
void init_test_data()
{
}
#endif

#ifdef alpha_dec_osf4_0
void init_test_data()
{
}
#endif


// Find and instrument loads with a simple function call snippet
int mutatorTest(BPatch_thread *bpthr, BPatch_image *bpimg)
{
  int testnum = 2;
  const char* testdesc = "store instrumentation";
#if !defined(sparc_sun_solaris2_4) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(i386_unknown_nt4_0) \
 && !defined(ia64_unknown_linux2_4)
  skiptest(testnum, testdesc);
#else
  init_test_data();

  BPatch_Set<BPatch_opCode> stores;
  stores.insert(BPatch_opStore);

  BPatch_Vector<BPatch_function *> found_funcs;
  const char *inFunction = "loadsnstores";
  if ((NULL == bpimg->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    inFunction);
    return -1;
  }
       
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), inFunction);
  }
       
  BPatch_Vector<BPatch_point *> *res1 = found_funcs[0]->findPoint(stores);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores\".\n");

  dumpvect(res1, "Stores");

  if((*res1).size() != nstores)
    failtest(testnum, testdesc, "Number of stores seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, storeList, "store"))
    failtest(testnum, testdesc, "Store sequence failed validation.\n");

  RETURNONFAIL(instCall(bpthr, "Store", res1));
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

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
