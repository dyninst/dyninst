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

// $Id: test_lib_soExecution.C,v 1.5 2006/12/01 01:33:34 legendre Exp $

#include <stdio.h>

#include "test_lib.h"
#include "ParameterDict.h"
#include "dlfcn.h"


int loadLibRunTest(test_data_t &testLib, ParameterDict &param)
{
   //printf("Loading test: %s\n", testLib.soname);
   char *fullSoPath;
#if defined(os_aix)
   fullSoPath = searchPath(getenv("LIBPATH"), testLib.soname);
#else
   fullSoPath = searchPath(getenv("LD_LIBRARY_PATH"), testLib.soname);
#endif
   if (!fullSoPath) {
      printf("Error finding lib %s in LD_LIBRARY_PATH/LIBPATH\n");
      return -1;
   }
   void *handle = dlopen(fullSoPath, RTLD_NOW);
   ::free(fullSoPath);

   if (!handle)
   {
      printf("Error opening lib: %s\n", testLib.soname);
      printf("%s\n", dlerror());
      return -1;
   }

   typedef int (*mutatorM_t)(ParameterDict &);
   char mutator_name[256];
   snprintf(mutator_name, 256, "%s_mutatorMAIN", testLib.name); 
   mutatorM_t mutTest = (mutatorM_t) dlsym(handle, mutator_name);
   if ( !mutTest )
   {
      printf("Error finding function: %s, in %s\n", mutator_name, 
             testLib.soname);
      printf("%s\n", dlerror());
      dlclose(handle);
      return -1;
   }

   // Call function
   int result = mutTest(param);

   dlclose(handle);

   //printf("Ran test: %s\n", testLib.soname);
   return result;
}
