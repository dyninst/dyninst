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

// $Id: test1_31.C,v 1.1 2005/09/29 20:38:37 bpellin Exp $
/*
 * #Name: test1_31
 * #Desc: Mutator Side - Non-Recursive Base Tramp
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//
// Start Test Case #31 - (non-recursive base tramp)
//
int mutatorTest( BPatch_thread * appThread, BPatch_image * appImage )
{
   const char * func31_2_name = "func31_2";
   const char * func31_3_name = "func31_3";
   const char * func31_4_name = "func31_4";

   BPatch_image * app_image = appImage;
   BPatch_thread * app_thread = appThread;
  
   BPatch_Vector<BPatch_function *> bpfv;
   if (NULL == appImage->findFunction(func31_2_name, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     fprintf(stderr, "    Unable to find function %s\n", func31_2_name);
     return -1;
   }
	
   BPatch_function *func31_2_function = bpfv[0];

   bpfv.clear();

   if (NULL == appImage->findFunction(func31_3_name, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     fprintf(stderr, "    Unable to find function %s\n", func31_3_name);
     return -1;
   }
   
   BPatch_function *func31_3_function = bpfv[0];

   bpfv.clear();
   
   if (NULL == appImage->findFunction(func31_4_name, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     fprintf(stderr, "    Unable to find function %s\n", func31_4_name);
     return -1;
   }
   
   BPatch_function *func31_4_function = bpfv[0];
   
   bool old_value = BPatch::bpatch->isTrampRecursive();
   BPatch::bpatch->setTrampRecursive( false );
   
   BPatch_Vector<BPatch_snippet *> func31_2_args;
   BPatch_snippet * func31_2_snippet =
      new BPatch_funcCallExpr( * func31_3_function,
                               func31_2_args );
   instrument_entry_points( app_thread, app_image, func31_2_function, func31_2_snippet );
   
   BPatch_Vector<BPatch_snippet *> func31_3_args_1;
   func31_3_args_1.push_back( new BPatch_constExpr( 1 ) );
   BPatch_snippet * func31_3_snippet_1 =
      new BPatch_funcCallExpr( * func31_4_function,
                               func31_3_args_1 );
   instrument_entry_points(app_thread, app_image, func31_3_function, func31_3_snippet_1);

   BPatch_Vector<BPatch_snippet *> bar_args_2;
   bar_args_2.push_back( new BPatch_constExpr( 2 ) );
   BPatch_snippet * bar_snippet_2 =
      new BPatch_funcCallExpr( * func31_4_function,
                               bar_args_2 );
   instrument_exit_points(app_thread, app_image, func31_3_function, bar_snippet_2);
   
   BPatch::bpatch->setTrampRecursive( old_value );

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
