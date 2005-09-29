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

// $Id: test6_3.C,v 1.1 2005/09/29 20:39:35 bpellin Exp $
/*
 * #Name: test6_3
 * #Desc: Prefetch Instrumentation
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
const unsigned int nprefes = 2;
BPatch_memoryAccess* prefeList[nprefes];

void init_test_data()
{
  int k=-1;
  prefeList[++k] = MK_PF(0,17,0,2);
  prefeList[++k] = MK_PF(0,20,0,0);
}
#endif

#ifdef rs6000_ibm_aix4_1
const unsigned int nprefes = 0;
#if defined(__XLC__) || defined(__xlC__)
BPatch_memoryAccess* *prefeList;
#else
BPatch_memoryAccess* prefeList[nprefes];
#endif

void init_test_data()
{
}

#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
const unsigned int nprefes = 2;
BPatch_memoryAccess* prefeList[nprefes + 1]; // for NT

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

  prefeList[++k] = MK_PF((long)divarwp,-1,-1,IA32prefetchT0);
  prefeList[++k] = MK_PF((long)divarwp,-1,-1,IA32AMDprefetch);
}
#endif

#ifdef ia64_unknown_linux2_4
const unsigned int nprefes = 3;
BPatch_memoryAccess* prefeList[nprefes];
void init_test_data() {
	prefeList[0] = MK_PF( 0, 14, -1, 0 );
	prefeList[1] = MK_PF( 0, 14, -1, 0 );
	prefeList[2] = MK_PF( 0, 14, -1, 0 );
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
  int testnum = 3;
  const char* testdesc = "prefetch instrumentation";
#if !defined(sparc_sun_solaris2_4) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(i386_unknown_nt4_0) \
 && !defined(ia64_unknown_linux2_4)
  skiptest(testnum, testdesc);
#else
  init_test_data();
  
  BPatch_Set<BPatch_opCode> prefes;
  prefes.insert(BPatch_opPrefetch);

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
       
  BPatch_Vector<BPatch_point *> *res1 = found_funcs[0]->findPoint(prefes);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores\".\n");

  dumpvect(res1, "Prefetches");

  if((*res1).size() != nprefes)
    failtest(testnum, testdesc,
             "Number of prefetches seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, prefeList, "prefetch"))
    failtest(testnum, testdesc, "Prefetch sequence failed validation.\n");

  RETURNONFAIL(instCall(bpthr, "Prefetch", res1));
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
