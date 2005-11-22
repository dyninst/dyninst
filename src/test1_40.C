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

// $Id: test1_40.C,v 1.2 2005/11/22 19:41:58 bpellin Exp $
/*
 * #Name: test1_40
 * #Desc: Verify that we can monitor call sites
 * #Dep: 
 * #Arch: !,alpha_dec_osf4_0,ia64_unknown_linux2_4,!mips_sgi_irix6_4,os_windows
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

int mutateeFortran;
// TODO: Fix mutateeXLC so that it is set
int mutateeXLC = 0;


//
//  Test case 40:  verify that we can monitor call sites
//

BPatch_function *findFunction40(const char *fname, 
                                BPatch_image *appImage)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

      fprintf(stderr, "**Failed test #40 (monitor call sites)\n");
      fprintf(stderr, "  Expected 1 functions matching call40_1, got %d\n",
              bpfv.size());
         return NULL;
  }
  return bpfv[0];
}

int setVar40(const char *vname, void *addr, BPatch_image *appImage)
{
   BPatch_variableExpr *v;
   void *buf = addr;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #40 (monitor call sites)\n");
      fprintf(stderr, "  cannot find variable %s\n", vname);
      return -1;
   }

   if (! v->writeValue(&buf, sizeof(unsigned int),false)) {
      fprintf(stderr, "**Failed test #40 (monitor call sites)\n");
      fprintf(stderr, "  failed to write call site var to mutatee\n");
      return -1;
   }

   return 0;
}

int mutatorTest(BPatch_thread * /*appThread*/, BPatch_image *appImage)
{

#if !defined(alpha_dec_osf4_0) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(mips_sgi_irix6_4) \
 && !defined(os_windows)

   if (mutateeFortran) return 0;
   // xlc does not produce the intended dynamic call points for this example
   if (mutateeXLC) return 0;

   const char *monitorFuncName = "func_40_monitorFunc";
   const char *callSiteAddrVarName = "callsite40_5_addr";

   BPatch_function *monitorFunc = NULL;
   BPatch_Vector<BPatch_function *> bpfv;

  BPatch_function *call40_1 = findFunction40("call40_1", appImage);
  RETURNONNULL(call40_1);
  RETURNONFAIL(setVar40("gv_addr_of_call40_1", call40_1->getBaseAddr(),appImage));

  BPatch_function *call40_2 = findFunction40("call40_2", appImage);
  RETURNONNULL(call40_2);
  RETURNONFAIL(setVar40("gv_addr_of_call40_2", call40_2->getBaseAddr(),appImage));

  BPatch_function *call40_3 = findFunction40("call40_3", appImage);
  RETURNONNULL(call40_3);
  RETURNONFAIL(setVar40("gv_addr_of_call40_3", call40_3->getBaseAddr(),appImage));

  //  call40_5 is the "dispatcher" of function pointers
  BPatch_function *targetFunc = findFunction40("call40_5", appImage);
  RETURNONNULL(targetFunc);
  //RETURNONFAIL(setVar40("gv_addr_of_call40_5", call40_5->getBaseAddr(),appImage));

  monitorFunc = findFunction40(monitorFuncName, appImage);

   BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
   if (!calls) {
      fprintf(stderr, "**Failed test #40 (monitor call sites)\n");
      fprintf(stderr, "  cannot find call points for call40_5\n");
         return -1;
   }

   BPatch_Vector<BPatch_point *> dyncalls;
   for (unsigned int i = 0; i < calls->size(); ++i) {
     BPatch_point *pt = (*calls)[i];
     if (pt->isDynamic())
       dyncalls.push_back(pt);
   }

   if (dyncalls.size() != 1) {
      fprintf(stderr, "**Failed test #40 (monitor call sites)\n");
      fprintf(stderr, "  wrong number of dynamic points found (%d -- not 1)\n",
              dyncalls.size());
      fprintf(stderr, "  total number of calls found: %d\n", calls->size());
         return -1;
   }

   // write address of anticipated call site into mutatee var.
   void *callsite_address = dyncalls[0]->getAddress();
   RETURNONFAIL(setVar40(callSiteAddrVarName, callsite_address, appImage));

   //  issue command to monitor calls at this site, and we're done.
   if (! dyncalls[0]->monitorCalls(monitorFunc)) {
      fprintf(stderr, "**Failed test #40 (monitor call sites)\n");
      fprintf(stderr, "  cannot monitor calls\n");
      return -1;
   }
#endif

   return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
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
